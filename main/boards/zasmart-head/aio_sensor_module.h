#ifndef AIO_SENSOR_MODULE_H
#define AIO_SENSOR_MODULE_H

#include <stdint.h>
#include <stdbool.h>

#include <vector>
#include <functional>

#include <cstring>
#include <string>

#include <mutex>

#include "esp_attr.h"

class AioSensorModule {
public:
    using AioSensorCallback = std::function<void(const std::string& OidVer, const std::string& oid)>; //

    struct AioData {
        uint8_t header;     // 7-bit header
        uint8_t reserver;   // 6-bit reserved
        uint32_t data;      // 30-bit index/data
        uint32_t index;     // mapped index
        bool stat;          // 数据是否有效
    };

    static AioSensorModule& GetInstance();

    bool Init();
    bool StartCalculation();
    void StartAIOTask();
    void SaveCalibrationData();

    const AioData& GetAioData() const;
    const int& GetAioIndex() const;
    void ClearAioIndex();



private:
    AioSensorModule();
    ~AioSensorModule();

    void SetIoDirection(int gpio, bool is_output);
    void SetIoLevel(int gpio, bool level);
    int ReadIoLevel(int gpio);

    void AioGpioInit();
    int ReadSda();
    int ReadAio(AioData* buf);
    void WriteCmd(int cmd);
    void WriteData(int* buffer, int len);
    int AioInit();
    void AioToIndex(AioData* data);
    static void OidTask(void* arg);
    void DelayMs(uint16_t ms);

    AioData aio_data_;
    int aio_index_;

    //AioSensorCallback callback_;
    //std::mutex callback_mutex_;
    //std::mutex sendoid_mutex_;

public:
    void onOidDetected(AioSensorCallback callback);
};

#endif // AIO_SENSOR_MODULE_H