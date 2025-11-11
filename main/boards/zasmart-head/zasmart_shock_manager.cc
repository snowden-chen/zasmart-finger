#include "zasmart_shock_manager.h"
#include "esp_log.h"

static const char* TAG = "ZasmartShockManager";

// 静态成员函数 - GPIO 中断处理
void IRAM_ATTR ZasmartShockManager::gpio_isr_handler(void* arg) {
    ZasmartShockManager* self = static_cast<ZasmartShockManager*>(arg);
    // 启动去抖定时器
    //esp_timer_start_once(self->debounce_timer_, 50000); // 50ms 去抖动
    //esp_timer_start_once(self->debounce_timer_, 200000); // 200ms 去抖动
    //esp_timer_start_once(self->debounce_timer_, 500000); // 500ms 去抖动
    //esp_timer_start_once(self->debounce_timer_, 300000); // 300ms 去抖动
    esp_timer_start_once(self->debounce_timer_, 100000); // 100ms 去抖动
    //esp_timer_start_once(self->debounce_timer_, 80000); // 80ms 去抖动
    //esp_timer_start_once(self->debounce_timer_, 150000); // 150ms 去抖动
    //ESP_LOGI(TAG, "gpio_isr_handler ------>..");
}

// 定时器回调用于确认震动事件
void ZasmartShockManager::debounce_timer_callback(void* arg) {
    ZasmartShockManager* self = static_cast<ZasmartShockManager*>(arg);
    bool current_state = gpio_get_level(self->shock_pin_) == 0;

    if (current_state != self->last_state_) {
        self->last_state_ = current_state;

        // 使用信号量保护回调执行
        if (self->callback_ && xSemaphoreTake(self->callback_semaphore_, pdMS_TO_TICKS(100)) == pdTRUE) 
        {
            if (self->last_state_) {
                //ESP_LOGI(TAG, "Shock detected!");
                if (self->callback_) {
                    self->callback_(true);
                }
            } else {
                //ESP_LOGI(TAG, "Shock released.");
                if (self->callback_) {
                    self->callback_(false);
                }
            }

            xSemaphoreGive(self->callback_semaphore_);
        }
    }

    //ESP_LOGI(TAG, "Shock debounce_timer_callback.");
    /*
    if (current_state != self->last_state_) {
        self->last_state_ = current_state;
        if (self->callback_) {
            self->callback_(current_state);
        }
    }*/
}

// 构造函数
ZasmartShockManager::ZasmartShockManager(gpio_num_t pin)
    : shock_pin_(pin), last_state_(true) {

    // 初始化 GPIO

    //isr
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // 下降沿触发 // org
    //io_conf.intr_type = GPIO_INTR_LOW_LEVEL;
    io_conf.mode = GPIO_MODE_INPUT; // org
    //io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << shock_pin_);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    // no isr
    /*
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE; // 下降沿触发
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << shock_pin_);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    */

    gpio_config(&io_conf);

    //gpio_set_level(shock_pin_, 1);

    // 创建去抖 timer
    esp_timer_create_args_t timer_args = {};
    timer_args.callback = debounce_timer_callback;
    timer_args.arg = this;
    timer_args.name = "debounce_timer";
    esp_timer_create(&timer_args, &debounce_timer_);

    // 创建信号量用于保护回调执行
    callback_semaphore_ = xSemaphoreCreateBinary();
    if (callback_semaphore_ != NULL) {
        // 初始化时给予信号量
        xSemaphoreGive(callback_semaphore_);
    }

    // 设置中断服务
    //gpio_install_isr_service(0);
    gpio_install_isr_service(1);
    gpio_isr_handler_add(shock_pin_, gpio_isr_handler, this);
}

// 析构函数
ZasmartShockManager::~ZasmartShockManager() {
    gpio_isr_handler_remove(shock_pin_);
    esp_timer_stop(debounce_timer_);
    esp_timer_delete(debounce_timer_);

    // 删除信号量
    if (callback_semaphore_ != NULL) {
        vSemaphoreDelete(callback_semaphore_);
    }
}

// 启动检测
void ZasmartShockManager::begin() {
    ESP_LOGI(TAG, "Shock manager started on GPIO %d", shock_pin_);
}

// 注册回调函数
void ZasmartShockManager::onShockDetected(ShockCallback callback) {
    // 使用信号量保护回调注册过程
    if (callback_semaphore_ != NULL && xSemaphoreTake(callback_semaphore_, pdMS_TO_TICKS(100)) == pdTRUE) {
        callback_ = std::move(callback);
        xSemaphoreGive(callback_semaphore_);
    } else {
        callback_ = std::move(callback);
    }
}








