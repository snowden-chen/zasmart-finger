#pragma once
#include <vector>
#include <functional>

#include <esp_timer.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>

class PowerManager {
private:
    //esp_timer_handle_t timer_handle_;
    std::function<void(bool)> on_charging_status_changed_;
    std::function<void(bool)> on_low_battery_status_changed_;

    std::function<void(bool)> on_monitor_battery_status_process_;

    gpio_num_t charging_pin_ = CHARGING_DETECT_GPIO;
    std::vector<uint16_t> adc_values_;
    uint32_t battery_level_ = 0;
    int adc_value = 0;
    bool is_charging_ = false;
    bool is_low_battery_ = false;
    int ticks_ = 0;
    const int kBatteryAdcInterval = 60;
    const int kBatteryAdcDataCount = 3;
    const int kLowBatteryLevel = 20;

    adc_oneshot_unit_handle_t adc_handle_;

    void CheckBatteryStatus() {
        // Get charging status
        //bool new_charging_status = gpio_get_level(charging_pin_) == 1;
        //bool new_charging_status = false; // by zach chen
        //bool new_charging_status = (adc_value >= 2400); // by zach chen// old-pen
        //bool new_charging_status = ((gpio_get_level(charging_pin_) == 0) || (adc_value > 2500)); // by zach chen
        bool new_charging_status = (gpio_get_level(charging_pin_) == 0); // by zach chen //new-pen
        if (new_charging_status != is_charging_) {
            is_charging_ = new_charging_status;
            if (on_charging_status_changed_) {
                on_charging_status_changed_(is_charging_);
            }
            ReadBatteryAdcData();
            return;
        }

        // 如果电池电量数据不足，则读取电池电量数据
        if (adc_values_.size() < kBatteryAdcDataCount) {
            ReadBatteryAdcData();
            return;
        }

        // 如果电池电量数据充足，则每 kBatteryAdcInterval 个 tick 读取一次电池电量数据
        ticks_++;
        if (ticks_ % kBatteryAdcInterval == 0) {
            ReadBatteryAdcData();
        }
    }

    void ReadBatteryAdcData() {
        //int adc_value;
        uint32_t temp_val = 0;

        //ESP_ERROR_CHECK(adc_oneshot_read(adc_handle_, ADC_CHANNEL_2, &adc_value)); //no use by zach
        //ESP_ERROR_CHECK(adc_oneshot_read(adc_handle_, ADC_CHANNEL_0, &adc_value));
        //ESP_ERROR_CHECK(adc_oneshot_read(adc_handle_, ADC_CHANNEL_4, &adc_value)); //maybe GPIO5

        for(int t = 0; t < 10; t ++) {
            ESP_ERROR_CHECK(adc_oneshot_read(adc_handle_, ADC_CHANNEL_4, &adc_value));
            temp_val += adc_value;
        }

        adc_value = temp_val / 10;
        
        // 将 ADC 值添加到队列中
        adc_values_.push_back(adc_value);
        if (adc_values_.size() > kBatteryAdcDataCount) {
            adc_values_.erase(adc_values_.begin());
        }
        uint32_t average_adc = 0;
        for (auto value : adc_values_) {
            average_adc += value;
        }
        average_adc /= adc_values_.size();

        // 定义电池电量区间
      /*
        const struct {
            uint16_t adc;
            uint8_t level;
        } levels[] = {
            {1970, 0},
            {2062, 20},
            {2154, 40},
            {2246, 60},
            {2338, 80},
            {2430, 100}
        };
*/

/*
        const struct {
            uint16_t adc;
            uint8_t level;
        } levels[] = {
            {1750, 0},
            {1835, 20},
            {1960, 40},
            {2070, 60},
            {2155, 80},
            {2239, 100}
        };
*/

/* //last try
        const struct {
            uint16_t adc;
            uint8_t level;
        } levels[] = {
            {1700, 0},
            {1800, 20},
            {1960, 40},
            {2070, 60},
            {2155, 80},
            {2450, 100}
        };
*/
/* // wanted status
        const struct {
            uint16_t adc;
            uint8_t level;
        } levels[] = {
            {2030, 0},     //3.45v
            {2134, 20},    //3.63v
            {2252, 40},    //3.78v 
            {2370, 60},    //3.9v
            {2488, 80},    //4.0v
            {2606, 100}    //4.2v
        };
*/
        const struct {
            uint16_t adc;
            uint8_t level;
        } levels[] = {
            {2030, 0},     //3.45v
            {2080, 20},    //3.5v
            {2130, 40},    //3.6v 
            {2250, 60},    //3.8v
            {2300, 80},    //3.9v
            {2450, 100}    //4.05v
        };
/*
        const struct {
            uint16_t adc;
            uint8_t level;
        } levels[] = {
            {200, 0},
            {280, 20},
            {360, 40},
            {440, 60},
            {520, 80},
            {600, 100}
        };
*/

/*
        const struct {
            uint16_t adc;
            uint8_t level;
        } levels[] = {
            {2250, 0},
            {2550, 20},
            {2850, 40},
            {3150, 60},
            {3450, 80},
            {3750, 100}
        };
*/

/*
        // 定义电池电量区间
        const struct {
            uint16_t adc;
            uint8_t level;
        } levels[] = {
            {2951, 0},  // 3.80V 
            {3019, 20},
            {3037, 40},
            {3091, 60}, // 3.88 
            {3124, 80},
            {3231, 100}
        };
*/

        // 定义电池电量区间
//        const struct {
//            uint16_t adc;
//            uint8_t level;
//        } levels[] = {
//            {2696, 0},      /*  3.48V -屏幕闪屏 */
//            {2724, 20},     /*  3.53V */
//            {2861, 40},     /*  3.7V */
//            {3038, 60},     /*  3.90V */
//            {3150, 80},     /*  4.02V */
//            {3280, 100}     /*  4.14V */
//        };

        // 低于最低值时
        if (average_adc < levels[0].adc) {
            battery_level_ = 0;
        }
        // 高于最高值时
        else if (average_adc >= levels[5].adc) {
            battery_level_ = 100;
        } else {
            // 线性插值计算中间值
            for (int i = 0; i < 5; i++) {
                if (average_adc >= levels[i].adc && average_adc < levels[i+1].adc) {
                    float ratio = static_cast<float>(average_adc - levels[i].adc) / (levels[i+1].adc - levels[i].adc);
                    battery_level_ = levels[i].level + ratio * (levels[i+1].level - levels[i].level);
                    break;
                }
            }
        }

        // Check low battery status
        if (adc_values_.size() >= kBatteryAdcDataCount) {
            bool new_low_battery_status = battery_level_ <= kLowBatteryLevel;
            if (new_low_battery_status != is_low_battery_) {
                is_low_battery_ = new_low_battery_status;
                if (on_low_battery_status_changed_) {
                    on_low_battery_status_changed_(is_low_battery_);
                }
            }
        }

        if (on_monitor_battery_status_process_) {
            on_monitor_battery_status_process_(is_charging_);
        }

        //low_voltage_ = adc_value;

        ESP_LOGI("PowerManager", "ADC value: %d average: %ld level: %ld", adc_value, average_adc, battery_level_);
    }

public:
    esp_timer_handle_t timer_handle_;
    //uint16_t low_voltage_ = 1750;
    PowerManager(gpio_num_t pin) : charging_pin_(pin) {
        // 初始化充电引脚
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << charging_pin_);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; 
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;     
        gpio_config(&io_conf);

        // 创建电池电量检查定时器
        esp_timer_create_args_t timer_args = {
            .callback = [](void* arg) {
                PowerManager* self = static_cast<PowerManager*>(arg);
                self->CheckBatteryStatus();
            },
            .arg = this,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "battery_check_timer",
            .skip_unhandled_events = true,
        };
        ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer_handle_));
        ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle_, 1000000));

        // 初始化 ADC
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT_1,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
        };
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle_));
        
        adc_oneshot_chan_cfg_t chan_config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12,
        };
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle_, ADC_CHANNEL_4, &chan_config));
    }

    ~PowerManager() {
        if (timer_handle_) {
            esp_timer_stop(timer_handle_);
            esp_timer_delete(timer_handle_);
        }
        if (adc_handle_) {
            adc_oneshot_del_unit(adc_handle_);
        }
    }

    bool IsCharging() {
        
        // 如果电量已经满了，则不再显示充电中
        /*  if (battery_level_ == 100) {
            is_charging_ = false;
            return false;
        }
        */

        //is_charging_ = false; // for temp by zach

        return is_charging_;
    }

    bool IsDischarging() {
        // 没有区分充电和放电，所以直接返回相反状态
        return !is_charging_;
    }

    uint8_t GetBatteryLevel() {
        return battery_level_;
    }

    void OnLowBatteryStatusChanged(std::function<void(bool)> callback) {
        on_low_battery_status_changed_ = callback;
    }

    void OnChargingStatusChanged(std::function<void(bool)> callback) {
        on_charging_status_changed_ = callback;
    }

    void onMonitorBatteryStatusProcess(std::function<void(bool)> callback) {
        on_monitor_battery_status_process_ = callback;
    }
};
