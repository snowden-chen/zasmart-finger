#include "wifi_board.h"
#include "codecs/box_audio_codec.h"
#include "display/oled_display.h"
#include "application.h"
#include "button.h"
#include "led/circular_strip.h"
//#include "led/single_led.h"
//#include "led/gpio_led.h"
#include "assets/lang_config.h"
#include "config.h"
#include "settings.h"

#include "power_save_timer.h"
#include "power_manager.h"

#include <esp_log.h>
#include <esp_efuse_table.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/i2c_master.h>
#include <wifi_station.h>

#include "zasmart_shock_manager.h"
#include "aio_sensor_module.h" 

#define TAG "ZasmartHeadBoard"

//LV_FONT_DECLARE(font_puhui_14_1);
//LV_FONT_DECLARE(font_awesome_14_1);

class ZasmartHeadBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t i2c_bus_;
    i2c_master_bus_handle_t codec_i2c_bus_;
    //Button boot_button_;
    Button vol_up_button_;
    Button vol_down_button_;
    Button main_button_;
    //esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    //esp_lcd_panel_handle_t panel_ = nullptr;
    Display* display_ = nullptr;
    PowerSaveTimer* power_save_timer_;
    PowerManager* power_manager_;
    ZasmartShockManager* shockManager_;
    bool is_netconfig_up_ = false;
    bool is_netconfig_down_ = false;
    bool is_netconfig_doing_ = false;

    int dev_status_count_ = 0;

    void InitializeShockManager(){
        shockManager_ = new ZasmartShockManager(SHOCK_DETECT_GPIO);

        shockManager_->begin();
        shockManager_->onShockDetected([this](bool shocked) {
            auto& app = Application::GetInstance();
            if (shocked) {
                //ESP_LOGI(TAG, "Device is being shocked!");
                
                if (power_save_timer_) {
                    power_save_timer_->WakeUp();
                    dev_status_count_ = 0;
                }
                
                if (app.GetDeviceState() == kDeviceStateIdle)
                {   
                    //app.StartListening();
                    app.ToggleChatState();
                }
                

                // 在这里可以触发播放音效、LED动画、关屏等操作
                /*
                if(Application::GetInstance().GetDeviceState() == kDeviceStateSpeaking)
                {
                    Application::GetInstance().AbortSpeaking(kAbortReasonNone);
                    Application::GetInstance().ToggleChatState();
                }
                else if (Application::GetInstance().GetDeviceState() == kDeviceStateIdle)
                {
                    Application::GetInstance().ToggleChatState();
                }
                */
                
            }
            else {
                //ESP_LOGI(TAG, "Device shock is released!");
            }
        });
    }

    
    void InitializePowerManager() {
        power_manager_ = new PowerManager(CHARGING_DETECT_GPIO);
        power_manager_->OnChargingStatusChanged([this](bool is_charging) {
            auto& app = Application::GetInstance();
            if (is_charging) {
                power_save_timer_->SetEnabled(false);
                
                std::string powerVer= "";
                char powerStr[64];
                snprintf(powerStr, sizeof(powerStr), "tyzpower:CHARGE %u", power_manager_->GetBatteryLevel());
                app.SendPressKeyToServer(powerVer,powerStr);

                //app.PlaySound(Lang::Sounds::OGG_POPUP);
            } else {
                power_save_timer_->SetEnabled(true);

                std::string powerVer= "";
                char powerStr[64];
                snprintf(powerStr, sizeof(powerStr), "tyzpower:BATTERY %u", power_manager_->GetBatteryLevel());
                app.SendPressKeyToServer(powerVer,powerStr);
            }
        });
        power_manager_->OnLowBatteryStatusChanged([this](bool is_low_battery) {
            auto& app = Application::GetInstance();
            if (is_low_battery) 
            {
                std::string powerVer= "";
                char powerStr[64];
                snprintf(powerStr, sizeof(powerStr), "tyzpower:LOW %u", power_manager_->GetBatteryLevel());
                app.SendPressKeyToServer(powerVer,powerStr);

                ESP_LOGI(TAG, "Device is low_battery mode!");
                app.PlaySound(Lang::Sounds::OGG_LOW_BATTERY);
            }
        });

        power_manager_->onMonitorBatteryStatusProcess([this](bool is_charging) {
            //ESP_LOGI(TAG, "onMonitorBatteryStatusProcess ------ by zach ------!");
            if (is_charging) {
                //ESP_LOGI(TAG, "Charging......, process none.");
            } else {
                auto& app = Application::GetInstance();
                if ((app.GetDeviceState() != kDeviceStateIdle) && (app.GetDeviceState() != kDeviceStateSpeaking))
                {
                    dev_status_count_++;
                    if (dev_status_count_ > 3)
                    {
                        //ESP_LOGI(TAG, "No Charging! Try change state to idle.");
                        app.ToggleChatState();
                        dev_status_count_ = 0;
                    }
                }
                else
                {
                    dev_status_count_ = 0;
                }

                if (power_manager_->GetBatteryLevel() == 0)
                {
                    //ESP_LOGI(TAG, "Battery level is 0, try to shutdown.");

                    std::string powerVer= "";
                    char powerStr[64];
                    snprintf(powerStr, sizeof(powerStr), "tyzpower:SHUTDOWN %u", power_manager_->GetBatteryLevel());
                    app.SendPressKeyToServer(powerVer,powerStr);

                    AioSensorModule& aio_module_ = AioSensorModule::GetInstance();
                    aio_module_.SaveCalibrationData();
                    app.PlaySound(Lang::Sounds::OGG_LOW_BATTERY);

                    gpio_set_level(POWER_CONTROL_GPIO, 0);
                }
            }
        });
    }


    void InitializePowerSaveTimer() {
        power_save_timer_ = new PowerSaveTimer(-1, 60, 300);
        power_save_timer_->OnEnterSleepMode([this]() {
            ESP_LOGI(TAG, "Enabling sleep mode");
            //auto display = GetDisplay();
            //display->SetChatMessage("system", "");
            //display->SetEmotion("sleepy");
            auto& app = Application::GetInstance();
            std::string powerVer= "";
            char powerStr[64];
            snprintf(powerStr, sizeof(powerStr), "tyzpower:SLEEP %u", power_manager_->GetBatteryLevel());
            app.SendPressKeyToServer(powerVer,powerStr);
        });
        power_save_timer_->OnExitSleepMode([this]() {
            ESP_LOGI(TAG, "Exiting sleep mode");
            //auto display = GetDisplay();
            //display->SetChatMessage("system", "");
            //display->SetEmotion("neutral");
        });
        power_save_timer_->OnShutdownRequest([this]() {
            ESP_LOGI(TAG, "Shutting down");

            auto& app = Application::GetInstance();

            std::string powerVer= "";
            char powerStr[64];
            snprintf(powerStr, sizeof(powerStr), "tyzpower:SHUTDOWN %u", power_manager_->GetBatteryLevel());
            app.SendPressKeyToServer(powerVer,powerStr);

            AioSensorModule& aio_module_ = AioSensorModule::GetInstance();
            aio_module_.SaveCalibrationData();
            app.PlaySound(Lang::Sounds::OGG_LOW_BATTERY);

            gpio_set_level(POWER_CONTROL_GPIO, 0);
        });
        
        //ESP_LOGI(TAG, "save timer SetEnabled -------------------->!");
        power_save_timer_->SetEnabled(true);
    }
    void InitializeI2c() {
        // Initialize I2C peripheral
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = (i2c_port_t)0,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_));
        
        /*
        i2c_master_bus_config_t i2c_bus_cfg_display = {
            .i2c_port = (i2c_port_t)1,
            .sda_io_num = DISPLAY_I2C_SDA_PIN,
            .scl_io_num = DISPLAY_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg_display, &codec_i2c_bus_));
        */
    }


    void InitializeButtons() {

        main_button_.OnClick([this]() {
            
            auto& app = Application::GetInstance();
            /*       
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }

    
            if (app.GetDeviceState() == kDeviceStateSpeaking){
                app.AbortSpeaking(kAbortReasonNone);
            }
            app.ToggleChatState();
            */

            if (power_save_timer_) {
                power_save_timer_->WakeUp();
                dev_status_count_ = 0;
            }

            /*
            if (app.GetDeviceState() == kDeviceStateSpeaking)
            {
                app.AbortSpeaking(kAbortReasonNone);
                app.StartListening();
            }
            */

            std::string pressVer = "";
            char presskeyStr[64];
            snprintf(presskeyStr, sizeof(presskeyStr), "tyzbtn:M_KEY %u", power_manager_->GetBatteryLevel());
            app.SendPressKeyToServer(pressVer,presskeyStr);

            if (app.GetDeviceState() == kDeviceStateSpeaking)
            {   
                //app.AbortSpeaking(kAbortReasonWakeWordDetected);
                app.ToggleChatState();
            }
            else if (app.GetDeviceState() == kDeviceStateIdle)
            {
                //app.StartListening();
                app.ToggleChatState();
            }
        });

        main_button_.OnDoubleClick([this]() {
//#if CONFIG_USE_DEVICE_AEC
            //if (app.GetDeviceState() == kDeviceStateIdle) {
                //app.SetAecMode(app.GetAecMode() == kAecOff ? kAecOnDeviceSide : kAecOff);
            //}
//#endif
            auto& app = Application::GetInstance();
            std::string pressVer = "";
            char presskeyStr[64];
            snprintf(presskeyStr, sizeof(presskeyStr), "tyzbtn:D_M_KEY %u", power_manager_->GetBatteryLevel());
            app.SendPressKeyToServer(pressVer,presskeyStr);

            auto codec = GetAudioCodec();
            auto volume = codec->output_volume();
            if (volume < 92)
            {
                volume = 92;
                codec->SetOutputVolume(volume);
            }
        });

        vol_up_button_.OnClick([this]() {
            //auto& app = Application::GetInstance();
            //ESP_LOGI(TAG, "up_button_ OnClick volume");
            //power_save_timer_->WakeUp();
            if (power_save_timer_) {
                power_save_timer_->WakeUp();
                dev_status_count_ = 0;
            }
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume();
            if (volume <= 42)
            {
                volume = 43;
            }
            else
            {
                volume += 9;
            }

            if (volume >= 92) {
                volume = 92;
            }

            codec->SetOutputVolume(volume);
            //app.PlaySound(Lang::Sounds::OGG_EXCLAMATION);

            GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        });

        vol_down_button_.OnClick([this]() {
            //auto& app = Application::GetInstance();
            //ESP_LOGI(TAG, "down_button_ OnClick volume");
            //power_save_timer_->WakeUp();
            if (power_save_timer_) {
                power_save_timer_->WakeUp();
                dev_status_count_ = 0;
            }
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume() - 9;
            if (volume < 42) 
            {
                volume = 0;
            }

            codec->SetOutputVolume(volume);
            //app.PlaySound(Lang::Sounds::OGG_EXCLAMATION);
            
            GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        });

        main_button_.OnLongPress([this]() {
            auto& app = Application::GetInstance();

            AioSensorModule& aio_module_ = AioSensorModule::GetInstance();
            ESP_LOGI(TAG, "main_button_ OnLongPress poweroff");

            std::string powerVer= "";
            char powerStr[64];
            snprintf(powerStr, sizeof(powerStr), "tyzpower:OFF %u", power_manager_->GetBatteryLevel());
            app.SendPressKeyToServer(powerVer,powerStr);

            aio_module_.SaveCalibrationData();

            app.PlaySound(Lang::Sounds::OGG_LOW_BATTERY);
            gpio_set_level(POWER_CONTROL_GPIO, 0);
        });

        vol_up_button_.OnLongPress([this]() {
            auto& app = Application::GetInstance();
            ESP_LOGI(TAG, "vol_up_button_ OnLongPress to ready for config network.");
            if (power_save_timer_) {
                power_save_timer_->WakeUp();
                dev_status_count_ = 0;
            }

            is_netconfig_up_ = true;

            if (is_netconfig_up_ == true && is_netconfig_down_ == true && is_netconfig_doing_ == false
                && app.GetDeviceState() == kDeviceStateStarting
                && !WifiStation::GetInstance().IsConnected())
            {
                is_netconfig_doing_ = true;
                ResetWifiConfiguration();
                is_netconfig_up_ = false;
            }
        });

        vol_down_button_.OnLongPress([this]() {
            auto& app = Application::GetInstance();
            ESP_LOGI(TAG, "vol_down_button_ OnLongPress to config network.");
            if (power_save_timer_) {
                power_save_timer_->WakeUp();
                dev_status_count_ = 0;
            }

            is_netconfig_down_ = true;
            
            if (is_netconfig_up_ == true && is_netconfig_down_ == true && is_netconfig_doing_ == false
                && app.GetDeviceState() == kDeviceStateStarting
                && !WifiStation::GetInstance().IsConnected())
            {
                is_netconfig_doing_ = true;
                ResetWifiConfiguration();
                is_netconfig_down_ = false;
            }
        });
    }

    void InitializeSsd1306Display() {

        /*
        // SSD1306 config
        esp_lcd_panel_io_i2c_config_t io_config = {
            .dev_addr = 0x3C,
            .on_color_trans_done = nullptr,
            .user_ctx = nullptr,
            .control_phase_bytes = 1,
            .dc_bit_offset = 6,
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 8,
            .flags = {
                .dc_low_on_data = 0,
                .disable_control_phase = 0,
            },
            .scl_speed_hz = 400 * 1000,
        };

        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c_v2(codec_i2c_bus_, &io_config, &panel_io_));

        ESP_LOGI(TAG, "Install SSD1306 driver");
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = -1;
        panel_config.bits_per_pixel = 1;

        esp_lcd_panel_ssd1306_config_t ssd1306_config = {
            .height = static_cast<uint8_t>(DISPLAY_HEIGHT),
        };
        panel_config.vendor_config = &ssd1306_config;

        ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(panel_io_, &panel_config, &panel_));
        ESP_LOGI(TAG, "SSD1306 driver installed");

        // Reset the display
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_));
        if (esp_lcd_panel_init(panel_) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize display");
            display_ = new NoDisplay();
            return;
        }

        // Set the display to on
        ESP_LOGI(TAG, "Turning display on");
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));

        display_ = new OledDisplay(panel_io_, panel_, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y,
            {&font_puhui_14_1, &font_awesome_14_1});
        */

        ESP_LOGI(TAG, "with no display!");
        display_ = new NoDisplay();
        return;
    }


    void InitializeOid() 
    {
        
        AioSensorModule& aio_module_ = AioSensorModule::GetInstance();
        
        /*
        // 启动 AIO 模块
        //aio_module_.Init();
           
        if (!aio_module_.StartCalculation()) 
        {
            ESP_LOGE(TAG, "AIO calculation start failed.");

            //return;
        } 
        
        else 
        {
            ESP_LOGI(TAG, "AIO module started.");
            aio_module_.StartAIOTask();
        }
        */

        aio_module_.StartAIOTask();
       // vTaskDelay(pdMS_TO_TICKS(10));


        /*
        aio_module_.onOidDetected([this](const std::string& OidVer,const std::string& oid) 
        {
            auto& app = Application::GetInstance();

            if ((app.GetDeviceState() != kDeviceStateListening) && (app.GetDeviceState() != kDeviceStateConnecting))
            {
                app.StartListening();
            }

            //vTaskDelay(pdMS_TO_TICKS(10));

            ESP_LOGI(TAG, "oid detected: %s ; oid value: %s", OidVer.c_str(),oid.c_str());

            if (app.GetDeviceState() == kDeviceStateSpeaking)
            {
                app.AbortSpeaking(kAbortReasonNone);
                app.StartListening();
            }

            if (power_save_timer_) 
            {
                power_save_timer_->WakeUp();
            }

            //vTaskDelay(pdMS_TO_TICKS(10));

            app.SendOidToServer(OidVer,oid);

            //vTaskDelay(pdMS_TO_TICKS(5));
            
            //app.StopListening();
        });
        */
    }

public:
    ZasmartHeadBoard() : 
        vol_up_button_(VOLUME_UP_BUTTON_GPIO),
        vol_down_button_(VOLUME_DOWN_BUTTON_GPIO),
        main_button_(POWER_BUTTON_GPIO) 
    {
        gpio_reset_pin(POWER_CONTROL_GPIO);
        gpio_set_direction(POWER_CONTROL_GPIO, GPIO_MODE_OUTPUT);
        gpio_set_level(POWER_CONTROL_GPIO, 1);

        //gpio_reset_pin(BUILTIN_LED_GPIO);
        //gpio_set_direction(BUILTIN_LED_GPIO, GPIO_MODE_OUTPUT);
        //gpio_set_level(BUILTIN_LED_GPIO, 1);

        InitializeShockManager();
        
        InitializeI2c();
        InitializeButtons();

        InitializePowerSaveTimer();

        InitializePowerManager();

        InitializeSsd1306Display();
        
        InitializeOid();
    }

    virtual Led* GetLed() override 
    {
        //static SingleLed led(BUILTIN_LED_GPIO);
        //static GpioLed led(BUILTIN_LED_GPIO);

        //static CircularStrip led(BUILTIN_LED_GPIO, 1);
        //return &led;

        static NoLed led;
        return &led;
    }

    virtual AudioCodec* GetAudioCodec() override 
    {
        static BoxAudioCodec audio_codec(
            i2c_bus_, 
            AUDIO_INPUT_SAMPLE_RATE, 
            AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, 
            AUDIO_I2S_GPIO_BCLK, 
            AUDIO_I2S_GPIO_WS, 
            AUDIO_I2S_GPIO_DOUT, 
            AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, 
            AUDIO_CODEC_ES8311_ADDR, 
            AUDIO_CODEC_ES7210_ADDR, 
            AUDIO_INPUT_REFERENCE);
        return &audio_codec;
    }

    virtual Display* GetDisplay() override 
    {
        return display_;
    }

    virtual bool GetBatteryLevel(int& level, bool& charging, bool& discharging) override 
    {
        static bool last_discharging = false;
        charging = power_manager_->IsCharging();
        discharging = power_manager_->IsDischarging();

        if (discharging != last_discharging) 
        {
            power_save_timer_->SetEnabled(discharging);

            last_discharging = discharging;
        }

        level = power_manager_->GetBatteryLevel();

        ESP_LOGI(TAG, "GetBatteryLevel charging=%d discharging=%d", charging, discharging);

        return true; 

    }

    virtual void SetPowerSaveMode(bool enabled) override 
    {
        if (!enabled) 
        {
            power_save_timer_->WakeUp();
        }

        WifiBoard::SetPowerSaveMode(enabled);
    }
};

DECLARE_BOARD(ZasmartHeadBoard);
