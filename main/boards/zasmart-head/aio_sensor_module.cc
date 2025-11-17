#include "aio_sensor_module.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>
//#include "za_aiot.h"
//#include "za_app_msg.h"

#include <cstring> // for memset
#include <string>

#include "application.h"
#include "settings.h"

//#include "za_common.h"

// 定义 GPIO 引脚
#define AIO_SCL GPIO_NUM_7
#define AIO_SDA GPIO_NUM_6

// 宏定义
#define GPIO_HIGH 1
#define GPIO_LOW 0
#define TAG "AioSensorModule"

AioSensorModule::AioSensorModule() {
    memset(&aio_data_, 0, sizeof(aio_data_));
}

AioSensorModule::~AioSensorModule() {
    // 可在此添加资源释放逻辑（如关闭GPIO等）
    //std::lock_guard<std::mutex> lock(callback_mutex_);
    //callback_ = nullptr;  // 清理回调
}

AioSensorModule& AioSensorModule::GetInstance() {
    static AioSensorModule instance;
    return instance;
}

bool AioSensorModule::Init() {
    ESP_LOGI(TAG, "Initializing AIO Sensor Module...");

    AioInit();

    return true;
}

bool AioSensorModule::StartCalculation() 
{
    ESP_LOGI(TAG, "Starting AIO calculation...");

    uint16_t delay_times = 1000;

    //AioGpioInit();
    //DelayMs(10);
    SetIoLevel(AIO_SCL, GPIO_HIGH);
    DelayMs(50);
    SetIoLevel(AIO_SCL, GPIO_LOW);
    //DelayMs(200);

	while(delay_times)
	{
		delay_times--;
		
		DelayMs(10);
		
		if(ReadSda() == 0)
		{
			ESP_LOGI(TAG,"aio_init:aio read!");
			
			ReadAio(&aio_data_);
			
			break;
		}

		if(delay_times == 0)
		{
			ESP_LOGE(TAG,"aio init failed!!!---");
			DelayMs(200);//by zach.snowden.chen
		}
	}

    if (aio_data_.header == 0x60) 
    {
        if (aio_data_.data == 0xFFF8)
        {
            ESP_LOGI(TAG, "AIO Pen INIT SUCCESS!");

            int cali_cmd_415[3] = {0x0e30, 0xc80, 0xc80};
            WriteData(cali_cmd_415, 3);
            ESP_LOGI(TAG, "Calibration command sent. Ack: 0x%x", aio_data_.header);

            DelayMs(2);

            int cali_restart_415[3] = {0x0e33, 0x0000, 0x0000};
            WriteData(cali_restart_415, 3);
            ESP_LOGI(TAG, "Restart command sent. Ack: 0x%x", aio_data_.header);

            DelayMs(2);

            return true;
        }
        else
        {
            ESP_LOGE(TAG, "AIO Pen INIT FAILED!");
            return false;
        }
    }

	if (aio_data_.header == 0x7F) //by zach snowden chen
	{
		if(aio_data_.data== 0xFFFF)
		{
			ESP_LOGI(TAG,"AIO pen INIT SUC!!!");
			
            int cali_cmd_415[3] = {0x0e30, 0xc80, 0xc80};
			WriteData(cali_cmd_415,3);
			ESP_LOGI(TAG,"输出ack的值0x%x",aio_data_.header);

			DelayMs(2);

            int cali_restart_415[3] = {0x0e33, 0x0000, 0x0000};
			WriteData(cali_restart_415,3);
			ESP_LOGI(TAG,"restart 输出ack的值0x%x",aio_data_.header);

			DelayMs(2);
			

			//aio_write_data(dpr_open,3);	
			
			return true;
		}
		else
		{
			ESP_LOGE(TAG,"AIO Pen INIT FAILED!!!");
			
			return false;
		}
	}

    return true;
}

void AioSensorModule::StartAIOTask() {
    xTaskCreate(OidTask, "AIO_OID_Task", 4096, this, 2, nullptr);
}

void AioSensorModule::SaveCalibrationData() {
    int cali_cmd_read[3] = {0x0e37, 0x0000, 0x0000};
    int param1 = 0x00;
    int param2 = 0x00;

    WriteData(cali_cmd_read, 3);

    if ((aio_data_.header & 0x07) != 0)
    {
        if (aio_data_.data > 0x12c0)
        {
            param1 = 0x12c0;
        }
        else if (aio_data_.data < 0x640)
        {
            param1 = 0x640;
        }
        else
        {
            param1 = aio_data_.data;
        }
    }

    DelayMs(2);

    WriteData(cali_cmd_read, 3);

    if ((aio_data_.header & 0x07) != 0)
    {
        if (aio_data_.data > 0x12c0)
        {
            param2 = 0x12c0;
        }
        else if (aio_data_.data < 0x640)
        {
            param2 = 0x640;
        }
        else
        {
            param2 = aio_data_.data;
        }
    }

    DelayMs(2);

    ESP_LOGI(TAG, "Calibration poweroff. param1: 0x%x , param2: 0x%x", (unsigned int)(param1), (unsigned int)(param2));

    Settings settings("oidcalibration", true);
    settings.SetInt("param1", param1);
    settings.SetInt("param2", param2);

    return;
}

const AioSensorModule::AioData& AioSensorModule::GetAioData() const {
    return aio_data_;
}

const int& AioSensorModule::GetAioIndex() const {
    return aio_index_;
}

void AioSensorModule::ClearAioIndex() {
    aio_index_ = 0;
}

void AioSensorModule::SetIoDirection(int gpio, bool is_output) {
    gpio_set_direction(static_cast<gpio_num_t>(gpio),
                       is_output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
}

void AioSensorModule::SetIoLevel(int gpio, bool level) {
    gpio_set_level(static_cast<gpio_num_t>(gpio), level ? GPIO_HIGH : GPIO_LOW);
}

int AioSensorModule::ReadIoLevel(int gpio) {
    return gpio_get_level(static_cast<gpio_num_t>(gpio));
}

void AioSensorModule::AioGpioInit() {
    gpio_reset_pin(AIO_SCL);
    gpio_reset_pin(AIO_SDA);

    SetIoDirection(AIO_SCL, true);  // 输出模式
    SetIoDirection(AIO_SDA, false); // 输入输出模式（用于读取）

    SetIoLevel(AIO_SCL, GPIO_LOW); // 默认低电平
}

int AioSensorModule::ReadSda() {
    return ReadIoLevel(AIO_SDA);
}

int AioSensorModule::ReadAio(AioData* buf) {
    uint8_t i = 0, j = 0;
    uint8_t aio_len = 0;
    int len = 0;

    if (ReadSda() == 1)
    {
        return 0;
    }

    buf->stat = true;
    buf->header = 0;
    buf->reserver = 0;
    buf->data = 0;
    buf->index = 0;

    SetIoDirection(AIO_SCL, true);
    SetIoDirection(AIO_SDA, true);
    SetIoLevel(AIO_SCL, GPIO_HIGH);
    SetIoLevel(AIO_SDA, GPIO_LOW);

    //vTaskDelay(pdMS_TO_TICKS(3));
    usleep(3);

    SetIoLevel(AIO_SCL, GPIO_LOW);

    //vTaskDelay(pdMS_TO_TICKS(3));
    usleep(3);

    SetIoLevel(AIO_SCL, GPIO_HIGH);
    SetIoDirection(AIO_SDA, false);

    for (i = 0; i < 7; ++i) 
    {
        SetIoLevel(AIO_SCL, GPIO_HIGH);

        buf->header <<= 1;

        //vTaskDelay(pdMS_TO_TICKS(5));
        usleep(5);

        SetIoLevel(AIO_SCL, GPIO_LOW);

        if (ReadIoLevel(AIO_SDA))
        {
            buf->header++;
        }
            
        //vTaskDelay(pdMS_TO_TICKS(5));
        usleep(5);
    }

    if (buf->header >= 0x61 && buf->header <= 0x63) 
    {
        for (i = 0; i < 6; ++i) 
        {
            SetIoLevel(AIO_SCL, GPIO_HIGH);

            buf->reserver <<= 1;

            //vTaskDelay(pdMS_TO_TICKS(5));
            usleep(5);

            SetIoLevel(AIO_SCL, GPIO_LOW);

            if (ReadIoLevel(AIO_SDA))
            {
                buf->reserver++;
            }
                
            //vTaskDelay(pdMS_TO_TICKS(5));
            usleep(5);
        }

        len = 43;
        aio_len = 30;
    } 
    else if (buf->header == 0x69) 
    {
        aio_len = 9;
        len = 16;
    } 
    else if (buf->header == 0x70) 
    {
        aio_len = 16;
        len = 23;
    } 
    else 
    {
        aio_len = 16;
        len = 23;
    }

    i = j = 0;

    while (aio_len--) 
    {
        SetIoLevel(AIO_SCL, GPIO_HIGH);

        buf->data <<= 1;

        //vTaskDelay(pdMS_TO_TICKS(5));
        usleep(5);

        SetIoLevel(AIO_SCL, GPIO_LOW);

        if (ReadIoLevel(AIO_SDA))
        {
            buf->data++;
        }
            
        //vTaskDelay(pdMS_TO_TICKS(5));
        usleep(5);

        if (++i >= 8) 
        {
            i = 0;
            j++;
        }
    }

    AioToIndex(buf);

    //vTaskDelay(pdMS_TO_TICKS(80));
    usleep(80);

    return len;
}

void AioSensorModule::WriteCmd(int cmd) 
{
    int j = 0;
    int iTemp = cmd;

    if (ReadSda() == 0)
        ReadAio(&aio_data_);

    SetIoDirection(AIO_SDA, true);
    SetIoLevel(AIO_SCL, GPIO_HIGH);
    SetIoLevel(AIO_SDA, GPIO_HIGH);

    //vTaskDelay(pdMS_TO_TICKS(5));
    usleep(5);

    SetIoLevel(AIO_SCL, GPIO_LOW);

    usleep(5);

    //vTaskDelay(pdMS_TO_TICKS(5));

    for (j = 0; j < 8; ++j) 
    {
        SetIoLevel(AIO_SCL, GPIO_HIGH);
        SetIoLevel(AIO_SDA, (iTemp & 0x80) ? GPIO_HIGH : GPIO_LOW);

        iTemp <<= 1;

        //vTaskDelay(pdMS_TO_TICKS(5));
        usleep(5);

        SetIoLevel(AIO_SCL, GPIO_LOW);

        //vTaskDelay(pdMS_TO_TICKS(5));
        usleep(5);
    }

    SetIoDirection(AIO_SDA, false);
    //vTaskDelay(pdMS_TO_TICKS(78));
    usleep(78);
    ESP_LOGI(TAG, "Command 0x%x written successfully.", cmd);
}

void AioSensorModule::WriteData(int* buffer, int len) 
{
    int i = 0;
    //ESP_LOGI(TAG,"aio init end,write cmd buf is 0x%04x   0x%04x   0x%04x",(unsigned short)(buffer[0]),(unsigned short)(buffer[1]),(unsigned short)(buffer[2]));
    
    if (ReadIoLevel(AIO_SDA) == 0)
    {
        ReadAio(&aio_data_);
    }

    SetIoDirection(AIO_SDA, true);
    SetIoLevel(AIO_SCL, GPIO_HIGH);
    SetIoLevel(AIO_SDA, GPIO_HIGH);
    //vTaskDelay(pdMS_TO_TICKS(5));
    usleep(5);
    SetIoLevel(AIO_SCL, GPIO_LOW);
    usleep(5);


    for (int i = 0; i < len; ++i) 
    {
        int iTemp = buffer[i];
        for (int j = 0; j < 16; ++j) 
        {
            SetIoLevel(AIO_SCL, GPIO_HIGH);
            SetIoLevel(AIO_SDA, (iTemp & 0x8000) ? GPIO_HIGH : GPIO_LOW);
            iTemp <<= 1;
            //vTaskDelay(pdMS_TO_TICKS(5));
            usleep(5);
            SetIoLevel(AIO_SCL, GPIO_LOW);
            usleep(5);
            //vTaskDelay(pdMS_TO_TICKS(5));
        }
    }

    SetIoDirection(AIO_SDA, false);
    //vTaskDelay(pdMS_TO_TICKS(78));
    usleep(78);

    i = 200;

	while(i)
	{
		DelayMs(10);

		if (ReadSda() == 0) 
		{
			ReadAio(&aio_data_);
			break;
		}

		i--;
	}

    return;
}

int AioSensorModule::AioInit() {
    AioGpioInit();
    uint16_t delay_times = 1000;
    DelayMs(10);
    SetIoLevel(AIO_SCL, GPIO_HIGH);
    DelayMs(50);
    SetIoLevel(AIO_SCL, GPIO_LOW);

    while (delay_times--) 
    {
        DelayMs(10);

        if (ReadSda() == 0) 
        {
            ESP_LOGI(TAG,"aio_init:aio read!");
            ReadAio(&aio_data_);
            break;
        }

        if (delay_times == 0) {
            ESP_LOGE(TAG, "AIO init failed!");
            //return 1;
        }
    }

    ESP_LOGI(TAG, "AIO read! Header: 0x%x, Data: 0x%x", (unsigned int)(aio_data_.header), (unsigned int)(aio_data_.data));

    if (aio_data_.header == 0x60 ) 
    {
        if(aio_data_.data == 0xFFF8)
        {
            ESP_LOGW(TAG, "AIO Pen INIT SUCCESSFUL!");
            int dpr_open[3] = {0xE2F, 0x07, 0x02};
            WriteData(dpr_open, 3);

            return 0;
        }
        else 
        {
            ESP_LOGE(TAG, "AIO Pen INIT FAILED!");
            return 1;
        }
    }

    return 2;
}

void AioSensorModule::AioToIndex(AioData* data) {
    if (data->header == 0x61) 
    {
        //ESP_LOGE(TAG, "AIO to index error: unsupported data %d", (int)(data->data));
        data->index = data->data;
    } 
    else if (data->header == 0x62 || data->header == 0x63) 
    {
        data->index = data->data;
    }
}

void AioSensorModule::OidTask(void* arg) {
    AioSensorModule* self = static_cast<AioSensorModule*>(arg);
    auto& app = Application::GetInstance();
    //za_app_msg_data_t msg_data;

    while (true) 
    {
        self->ReadAio(&self->aio_data_);
        if (self->aio_data_.stat) {
            self->aio_data_.stat = false;

            std::string oidVer = "";
            char oidStr[64];

            //snprintf(oidStr, sizeof(oidStr), "%d", self->aio_data_.index);
            snprintf(oidStr, sizeof(oidStr), "tyzoid:%" PRIu32, self->aio_data_.index);
            //std::string oid = std::to_string(self->aio_data_.index);

            if (self->aio_data_.header == 0x62) {
                ESP_LOGI(TAG, "OID3 code: %d; 0x%x", (int)(self->aio_data_.index), (unsigned int)(self->aio_data_.index));

                /*
                if (self->aio_data_.index != 0 && za_aiot_connect_status(nullptr)) {
                    msg_data.za_appmsg_type = ZA_APP_EVENT_AIOT_SEND_OID;
                    snprintf(msg_data.cBuffer, sizeof(msg_data.cBuffer), "%d", self->aio_data_.index);
                    za_msg_queue_send(za_get_app_msg_handle(nullptr), &msg_data, portMAX_DELAY);
                    memset(msg_data.cBuffer, 0, sizeof(msg_data.cBuffer));
                }
                */

                
/*
                if (app.GetDeviceState() == kDeviceStateSpeaking)
                {
                    app.AbortSpeaking(kAbortReasonNone);
                    
                    //app.StopListening();
                    //app.StartListening();

                    app.ToggleChatState();
                }

                vTaskDelay(pdMS_TO_TICKS(50));
*/

/*
                if ((app.GetDeviceState() != kDeviceStateListening) && (app.GetDeviceState() != kDeviceStateConnecting))
                {
                    //app.StartListening();
                    app.ToggleChatState();
                    vTaskDelay(pdMS_TO_TICKS(80));
                    
                }
*/

                //std::lock_guard<std::mutex> lock(self->sendoid_mutex_);
                //if ((app.GetDeviceState() == kDeviceStateSpeaking) || (app.GetDeviceState() == kDeviceStateIdle))
                //{   
                //    app.ToggleChatState();
                //    vTaskDelay(pdMS_TO_TICKS(100));
                //}

                //std::lock_guard<std::mutex> lock(self->sendoid_mutex_);
                app.SendOidToServer(oidVer,oidStr);
                
                if ((int)(self->aio_data_.index) == 518888)
                {
                    if (app.GetDeviceState() == kDeviceStateSpeaking)
                    {   
                        app.ToggleChatState();
                    }
                    else if (app.GetDeviceState() == kDeviceStateIdle)
                    {
                        app.ToggleChatState();
                    }
                    else
                    {
                        self->aio_index_= (int)(self->aio_data_.index);
                    }
                }

                /*
                std::lock_guard<std::mutex> lock(self->callback_mutex_);
                if (self->callback_) 
                {
                    //self->callback_("OID3",std::to_string(self->aio_data_.index));
                    //self->callback_("",std::to_string(self->aio_data_.index));

                    auto callback = self->callback_;
                    //callback(oidVer,oid);
                    callback(oidVer,oidStr);
                }*/
            }
			else if(self->aio_data_.header == 0x61)
			{
				ESP_LOGI(TAG,"get the oid2 code:%d\n",(int)(self->aio_data_.index));
				//OID2 Index aio_data.index

/*
                if (app.GetDeviceState() == kDeviceStateSpeaking)
                {
                    app.AbortSpeaking(kAbortReasonNone);
                    
                    //app.StopListening();
                    //app.StartListening();

                    //app.ToggleChatState();
                }

                vTaskDelay(pdMS_TO_TICKS(50));
*/

/*
                if ((app.GetDeviceState() != kDeviceStateListening) && (app.GetDeviceState() != kDeviceStateConnecting))
                {
                    //app.StartListening();
                    app.ToggleChatState();
                    vTaskDelay(pdMS_TO_TICKS(80));
                    
                }
*/
                //std::lock_guard<std::mutex> lock(self->sendoid_mutex_);
                //if ((app.GetDeviceState() == kDeviceStateSpeaking) || (app.GetDeviceState() == kDeviceStateIdle))
                //{   
                //    app.ToggleChatState();
                //    vTaskDelay(pdMS_TO_TICKS(100));
                //}

                //std::lock_guard<std::mutex> lock(self->sendoid_mutex_);
                app.SendOidToServer(oidVer,oidStr);

                //for test by zach snowden chen
                //int cali_cmd_poweroff[3] = {0x0e37, 0x0000, 0x0000};
                //self->WriteData(cali_cmd_poweroff, 3);
                //ESP_LOGI(TAG, "Calibration poweroff. Ack: 0x%x , Data: 0x%x", (unsigned int)(self->aio_data_.header), (unsigned int)(self->aio_data_.data));

			}
			else if(self->aio_data_.header == 0x63)
			{
				ESP_LOGI(TAG,"get the drp code:%d\n",(int)(self->aio_data_.index));
				//dpr index aio_data.index
			}
			else if(self->aio_data_.header == 0x60)		
			{
				//cmd 
				if(self->aio_data_.data == 0xFFF1)		//reset cmd
				{
                    int dpr_close[3] = {0xE2F, 0x02,0x01};
                    self->WriteData(dpr_close, 3);		
				}
			}

        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void AioSensorModule::DelayMs(uint16_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

// 注册回调函数
void AioSensorModule::onOidDetected(AioSensorCallback callback) {
    //std::lock_guard<std::mutex> lock(callback_mutex_);
    //callback_ = std::move(callback);
}
