#pragma once
#include <vector>
#include <functional>

#include <esp_timer.h>
#include <driver/gpio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "esp_attr.h"



class ZasmartShockManager {
public:
    using ShockCallback = std::function<void(bool shock_detected)>; //

private:
    gpio_num_t shock_pin_;
    esp_timer_handle_t debounce_timer_;
    bool last_state_;
    ShockCallback callback_;
    SemaphoreHandle_t callback_semaphore_; // 信号量用于保护回调执行

    static void IRAM_ATTR gpio_isr_handler(void* arg);
    static void debounce_timer_callback(void* arg);

public:
    explicit ZasmartShockManager(gpio_num_t pin);
    ~ZasmartShockManager();

    void begin();
    void onShockDetected(ShockCallback callback);
};
