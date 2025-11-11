#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>

#include "za_common.h"
#include "za_command.h"

#include "zhengan.h"

#include "za_blufi.h"
#include "za_storage.h"
#include "za_wifi.h"
#include "za_upgrade.h"
#include "za_console.h"
#include "za_aiot.h"
#include "za_json.h"
#include "za_appext.h"
#include "za_innersor.h"
#include "za_system.h"
#include "za_aiosm.h"

#include "application.h"
#include "system_info.h"

#define TAG "main"

extern "C" void app_main(void)
{
    // Initialize the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS flash for WiFi configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    
    za_debug();

    ZA_ESP_LOGI(TAG, "ZASMART MAIN ENTER.");

    /*
    if (za_app_ext_init() != ZA_TRUE)
    {
        ZA_ESP_LOGE(TAG, "za_app_ext_init error, so power off / done / game over.");
        vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
        goto ZA_MACHINE_DONE;
    }
    */   

    za_innersor_init();

    /*
    if (za_storage_nvs_init() != ZA_TRUE)
    {
        ZA_ESP_LOGE(TAG, "za_storage_nvs_init error, so power off / done / game over.");
        vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
        goto ZA_MACHINE_DONE;
    }
    */

    /*
    if (za_storage_spiffs_init() != ZA_TRUE)
    {
        ZA_ESP_LOGE(TAG, "za_storage_spiffs_init error, so power off / done / game over.");
        vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
        goto ZA_MACHINE_DONE;
    }
    */

    /*
    if (za_storage_init() != ZA_TRUE)
    {
        ZA_ESP_LOGE(TAG, "za_storage_init error, so power off / done / game over.");
        vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
        goto ZA_MACHINE_DONE;
    }
    */
    
    /*
    za_system_init();
    

    if ( za_app_init() != ZA_TRUE )
    {
        ZA_ESP_LOGE(TAG, "za_app_init error, so power off / done / game over.");

        vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
        goto ZA_MACHINE_DONE;
    }
    */

    //for test by zach snowden chen
    //za_storage_get_calibra_param1(ZANULL);
    //za_storage_get_calibra_param2(ZANULL);


    if ( za_aiosm_init() != ZA_TRUE )
    {
         vTaskDelay(ZA_pdMS_TO_TICKS(20));

        if (za_aiosm_init() != ZA_TRUE )
        {
            ZA_ESP_LOGE(TAG, "za_aiosm_init error, so power off / done / game over.");

            vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
            //goto ZA_MACHINE_DONE;
            za_inner_LM_PowerCtrlALL(ZA_FALSE);
        }
    }

    vTaskDelay(ZA_pdMS_TO_TICKS(50));

    //Application::GetInstance().Start();

    // The main thread will exit and release the stack memory

    za_debug();

    //vTaskDelay(ZA_pdMS_TO_TICKS(100));

    auto& app = Application::GetInstance();
    app.Start();

    while(ZA_TRUE)
    {
        //vTaskDelay(ZA_pdMS_TO_TICKS(1000));
        vTaskDelay(ZA_pdMS_TO_TICKS(1000000000));
    }


/*
ZA_MACHINE_DONE:

    // Launch the application
    //Application::GetInstance().Start();

    while(ZA_TRUE)
    {
        ZA_ESP_LOGE(TAG, "ZA_MACHINE_DONE, so power off / done / game over.");

        vTaskDelay(ZA_pdMS_TO_TICKS(500));

        //esp_restart();
        za_inner_LM_PowerCtrlALL(ZA_FALSE);
    }
*/
    return;
}
