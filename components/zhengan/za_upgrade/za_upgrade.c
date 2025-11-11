/* Used for zadiy factory upgrade data , files , ota ... . */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"
#include "za_command.h"

#include "zhengan.h"

#include "za_storage.h"
#include "za_upgrade.h"
#include "za_system.h"
#include "za_innersor.h"

#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

#include <sys/socket.h>

#define HASH_LEN 32

#define ZA_UPGRADE_TASK_STACKSIZE ZA_BUFFER_8K
#define ZA_UPGRADE_TASK_PRIORITY 5

static const char *TAG = "ZA_UPGRADE";

static const char *ZASMART_URL = "https://ff3nod.zdmedia.com.cn/cza/zadiy/factory/ZADIYFACTORY.bin";
static const char *ZASMART_DEBUG_URL = "https://ff3nod.zdmedia.com.cn/cza/zadiy/factory/debug/ZADIYFACTORY.bin";

static const char *ZASMART_URL_OPEN = "http://ff3nod.zdmedia.com.cn/cza/zadiy/forfactory/ZADIYFACTORY.bin";

extern const ZA_UINT8 server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const ZA_UINT8 server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

static esp_app_desc_t running_app_info;

static ZA_UINT32 nCount = 0;
static ZA_BOOL bStatusFlag = ZA_TRUE;


static ZA_VOID print_sha256(const ZA_UINT8 *image_hash, const ZA_CHAR *label)
{
    ZA_CHAR hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (ZA_INT32 i = 0; i < HASH_LEN; ++i) {
        za_sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ZA_ESP_LOGI(TAG, "%s %s", label, hash_print);
}

static ZA_VOID get_sha256_of_partitions(ZA_VOID)
{
    ZA_UINT8 sha_256[HASH_LEN] = { 0 };

    esp_partition_t partition;

    // get sha256 digest for bootloader
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    const esp_partition_t *running = esp_ota_get_running_partition();

    // get sha256 digest for running partition
    esp_partition_get_sha256(running, sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");

    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        ZA_ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }
}

za_esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {

      case HTTP_EVENT_ERROR: // if can not connect to the server
         ZA_ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
         //za_command_excute("CaidengI Y sigredf-1");
         break;

      case HTTP_EVENT_ON_CONNECTED: // connect to the server
         ZA_ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
         nCount = 0;
         break;

      case HTTP_EVENT_HEADER_SENT: // Be Ready to Found the SSL / HTTPS 
         ZA_ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
         break;

      case HTTP_EVENT_ON_HEADER: // Be Ready to Found the SSL / HTTPS
         ZA_ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
         break;

      case HTTP_EVENT_ON_DATA: // Start to Update data 
        {
            ZA_ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            nCount ++;

            if (! (nCount % 13) )
            {
                if (bStatusFlag)
                {
                    bStatusFlag = ZA_FALSE;

                    //za_command_excute("CaidengI Y sigpurple-1");
                }
                else
                {
                    bStatusFlag = ZA_TRUE;

                    //za_command_excute("CaidengI Y siggreenf-1");
                }
            }
        }
        break;

      case HTTP_EVENT_ON_FINISH:
         ZA_ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
         //za_command_excute("CaidengI Y sigblue-1"); 
         nCount = 0;
         break;

      case HTTP_EVENT_DISCONNECTED: // if update finish or err happened
         ZA_ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
         nCount = 0;
         break;

      case HTTP_EVENT_REDIRECT: // 新增的 case
         ZA_ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
         break;

      default:
         break;
    }

    return ESP_OK;
}

static esp_err_t validate_image_header(esp_app_desc_t *new_app_info)
{
    if (new_app_info == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;

    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
    {
        ZA_ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }

    if (memcmp(new_app_info->version, running_app_info.version, sizeof(new_app_info->version)) == 0)
    {
        ZA_ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");

        return ESP_FAIL;
    }

    return ESP_OK;
}

/* Public */

ZA_BOOL za_upgrade_goto_factory(ZA_VOID *pvParameter)
{
    (void) pvParameter;

    const esp_partition_t *find_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_OTA, NULL);

    if (find_partition != NULL) 
    {
        if(esp_partition_erase_range(find_partition, 0, find_partition->size) == ESP_OK)
        {
            return ZA_TRUE;
        }
    }

    return ZA_FALSE;
}


ZA_BOOL za_upgrade_goto_factory_reboot(ZA_VOID *pvParameter)
{
    (void) pvParameter;

    za_storage_sys_mode_t tStoreSysModeReadW = za_storage_get_system_mode(ZA_NULL);

    const esp_partition_t *find_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_OTA, NULL);

    if (find_partition != NULL) 
    {
        if(esp_partition_erase_range(find_partition, 0, find_partition->size) == ESP_OK)
        {
            tStoreSysModeReadW.tUpgradeMode = ZA_STORAGE_UPGRADE_IGNORE_MODE;

            za_storage_set_system_mode(tStoreSysModeReadW);

            vTaskDelay(100 / portTICK_PERIOD_MS);
            esp_restart();

            return ZA_TRUE;
        }
    }

    return ZA_FALSE;
}

ZA_BOOL za_upgrade_set_force_reboot()
{
    za_storage_sys_mode_t tStoreSysModeReadW = za_storage_get_system_mode(ZA_NULL);

    tStoreSysModeReadW.tUpgradeMode = ZA_STORAGE_UPGRADE_FORCE_MODE;

    za_storage_set_system_mode(tStoreSysModeReadW);

    za_upgrade_goto_factory(ZA_NULL);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    esp_restart();

    return ZA_TRUE;
}

ZA_BOOL za_upgrade_set_version_reboot()
{
    za_storage_sys_mode_t tStoreSysModeReadW = za_storage_get_system_mode(ZA_NULL);

    tStoreSysModeReadW.tUpgradeMode = ZA_STORAGE_UPGRADE_VERSION_MODE;

    za_storage_set_system_mode(tStoreSysModeReadW);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    esp_restart();

    return ZA_TRUE;
}

ZA_BOOL za_upgrade_set_factory_reboot()
{
    za_storage_sys_mode_t tStoreSysModeReadW = za_storage_get_system_mode(ZA_NULL);

    tStoreSysModeReadW.tUpgradeMode = ZA_STORAGE_UPGRADE_FACTORY_MODE;

    za_storage_set_system_mode(tStoreSysModeReadW);

    za_upgrade_goto_factory(ZA_NULL);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    esp_restart();
    
    return ZA_TRUE;
}

ZA_BOOL za_upgrade_set_force_mode()
{
    za_storage_sys_mode_t tStoreSysModeReadW = za_storage_get_system_mode(ZA_NULL);

    tStoreSysModeReadW.tUpgradeMode = ZA_STORAGE_UPGRADE_FORCE_MODE;

    za_storage_set_system_mode(tStoreSysModeReadW);

    return ZA_TRUE;
}

ZA_BOOL za_upgrade_set_ignore_mode()
{
    za_storage_sys_mode_t tStoreSysModeReadW = za_storage_get_system_mode(ZA_NULL);

    tStoreSysModeReadW.tUpgradeMode = ZA_STORAGE_UPGRADE_IGNORE_MODE;

    za_storage_set_system_mode(tStoreSysModeReadW);

    return ZA_TRUE;
}

ZA_BOOL za_upgrade_init()
{
    za_storage_sys_mode_t tStoreSysModeReadW = za_storage_get_system_mode(ZA_NULL);

    get_sha256_of_partitions();

    if (tStoreSysModeReadW.tUpgradeMode != ZA_STORAGE_UPGRADE_NULL)
    {
        switch (tStoreSysModeReadW.tUpgradeMode)
        {
            case ZA_STORAGE_UPGRADE_IGNORE_MODE:
            {
                /*ignore mode*/
            }
            break;

            case ZA_STORAGE_UPGRADE_FORCE_MODE:
            {
                //za_upgrade_start(ZASMART_URL);

                //tStoreSysModeReadW.tUpgradeMode = ZA_STORAGE_UPGRADE_IGNORE_MODE;

                //za_storage_set_system_mode(tStoreSysModeReadW);

            }
            break;

            case ZA_STORAGE_UPGRADE_VERSION_MODE:
            {
                //za_upgrade_start_ver(ZASMART_URL);

                tStoreSysModeReadW.tUpgradeMode = ZA_STORAGE_UPGRADE_IGNORE_MODE;

                za_storage_set_system_mode(tStoreSysModeReadW);
            }
            break;

            case ZA_STORAGE_UPGRADE_FACTORY_MODE:
            {
                //za_upgrade_goto_factory(ZA_NULL);

                tStoreSysModeReadW.tUpgradeMode = ZA_STORAGE_UPGRADE_IGNORE_MODE;

                za_storage_set_system_mode(tStoreSysModeReadW);
            }
            break;
        
            default:
                break;
        }
    }

    return ZA_TRUE;
}

ZA_CHAR * za_upgrade_firmware_ver(ZA_VOID *pvParameter)
{
   (ZA_VOID) pvParameter;

   return running_app_info.version;
}


ZA_VOID zasmart_force_ota_task(ZA_VOID *pvParameter)
{
    ZA_ESP_LOGI(TAG, "Starting OTA force update...");

    esp_http_client_config_t config = {
        //.url = CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL,
        //.url = "https://ff3nod.zdmedia.com.cn/cza/v2/ZASMART.bin",
        .url = (const char *)pvParameter,
        .cert_pem = (char *)server_cert_pem_start,
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,

        //.disable_auto_redirect = true,
    };

    nCount = 0;
    bStatusFlag = ZA_TRUE;

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    za_esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK)
    {
        ZA_ESP_LOGI(TAG, "Firmware upgrade success!");
        //za_command_excute("CaidengI Y white"); 
        ZA_vTaskDelay(100 / ZA_portTICK_PERIOD_MS);
        esp_restart();
    } 
    else 
    {
        ZA_ESP_LOGE(TAG, "Firmware upgrade failed!");
        //za_command_excute("CaidengI Y sigredf-1");
    }

    vTaskDelete(NULL);
}

ZA_UCHAR za_system_Zasmart(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    //esp_wifi_set_ps(WIFI_PS_NONE);
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    za_task_create(zasmart_force_ota_task, "za_upgrade_task", ZA_UPGRADE_TASK_STACKSIZE, (void * const)(ZASMART_URL), ZA_UPGRADE_TASK_PRIORITY, NULL);

    za_system_msg_magic_box_update_report(ZANULL);

    za_innersor_wifi_led_shink_ctrl(ZA_TRUE);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_zasmart_open(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    //esp_wifi_set_ps(WIFI_PS_NONE);
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    za_task_create(zasmart_force_ota_task, "za_open_upgrade_task", ZA_UPGRADE_TASK_STACKSIZE, (void * const)(ZASMART_URL_OPEN), ZA_UPGRADE_TASK_PRIORITY, NULL);

    za_system_msg_magic_box_update_report(ZANULL);

    za_innersor_wifi_led_shink_ctrl(ZA_TRUE);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_update_from_path(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_CHAR cTempChar[255] = {0};

    (ZA_VOID) p;

    if (za_storage_get_autoupdate_path(cTempChar) == ZA_FALSE)
    {
        return ZA_CMD_ERR_UNINVAIL;
    }

    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    za_task_create(zasmart_force_ota_task, "za_update_from_path_task", ZA_UPGRADE_TASK_STACKSIZE, (void * const)(cTempChar), ZA_UPGRADE_TASK_PRIORITY, NULL);

    za_system_msg_magic_box_update_report(ZANULL);

    za_innersor_wifi_led_shink_ctrl(ZA_TRUE);

    return ZA_CMD_SUCCESS;
}

static esp_err_t _http_client_init_cb(esp_http_client_handle_t http_client)
{
    esp_err_t err = ESP_OK;
    /* Uncomment to add custom headers to HTTP request */
    // err = esp_http_client_set_header(http_client, "Custom-Header", "Value");
    return err;
}

ZA_VOID zasmart_advanced_ota_task(ZA_VOID *pvParameter)
{
    ZA_ESP_LOGI(TAG, "Starting Advanced OTA example");

    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    za_esp_err_t ota_finish_err = ESP_OK;

    esp_http_client_config_t config = {
        .url = (const char *)pvParameter,
        .cert_pem = (char *)server_cert_pem_start,
        .timeout_ms = 5000, // 5 second
        .keep_alive_enable = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
        .http_client_init_cb = _http_client_init_cb, // Register a callback to be invoked after esp_http_client is initialized
#ifdef CONFIG_EXAMPLE_ENABLE_PARTIAL_HTTP_DOWNLOAD
        .partial_http_download = true,
        .max_http_request_size = CONFIG_EXAMPLE_HTTP_REQUEST_SIZE,
#endif
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    za_esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK)
    {
        ZA_ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
        //za_command_excute("CaidengI Y sigredf-1");
        
	esp_wifi_set_ps(WIFI_PS_NONE);

	vTaskDelete(NULL);
    }

    esp_app_desc_t app_desc;
    err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
    if (err != ESP_OK)
    {
        ZA_ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");
        //za_command_excute("CaidengI Y sigredf-1");
        goto ota_end;
    }

    err = validate_image_header(&app_desc);
    if (err != ESP_OK)
    {
        ZA_ESP_LOGW(TAG, "image header verification - stop here.");
        //za_command_excute("CaidengI Y sigredf-1");
        //za_command_excute("CaidengI Y orange"); 
        goto ota_end;
    }

    //za_command_excute("CaidengI Y siggreenf-1");

    za_system_msg_magic_box_update_report(ZANULL);

    za_innersor_wifi_led_shink_ctrl(ZA_TRUE);

    while (1)
    {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
        {
            break;
        }

        // esp_https_ota_perform returns after every read operation which gives user the ability to
        // monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of image
        // data read so far.
        ZA_ESP_LOGI(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
    }

    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true)
    {
        // the OTA image was not completely received and user can customise the response to this situation.
        ZA_ESP_LOGE(TAG, "Complete data was not received.");
        //za_command_excute("CaidengI Y sigredf-1");
    }
    else
    {
        ota_finish_err = esp_https_ota_finish(https_ota_handle);

        if ((err == ESP_OK) && (ota_finish_err == ESP_OK))
        {
            ZA_ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");

            //za_command_excute("CaidengI Y white"); 

            vTaskDelay(100 / portTICK_PERIOD_MS);

            esp_restart();
        }
        else
        {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED)
            {
                ZA_ESP_LOGE(TAG, "Image validation failed, image is corrupted");
            }

            ZA_ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);

            //za_command_excute("CaidengI Y sigredf-1");

            vTaskDelete(NULL);
        }
    }

ota_end:
    esp_https_ota_abort(https_ota_handle);
    ZA_ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed");

    esp_wifi_set_ps(WIFI_PS_NONE);

    vTaskDelete(NULL);
}

ZA_UCHAR za_system_ZasmartVer(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    //esp_wifi_set_ps(WIFI_PS_NONE);
    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM); // for upgrade

    za_task_create(zasmart_advanced_ota_task, "za_upgrade_task_ver", ZA_UPGRADE_TASK_STACKSIZE, (void * const)(ZASMART_URL), ZA_UPGRADE_TASK_PRIORITY, NULL);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_GoFactory(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    za_upgrade_goto_factory_reboot(ZA_NULL);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_update_debug(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    za_task_create(zasmart_force_ota_task, "za_upgrade_debug_task", ZA_UPGRADE_TASK_STACKSIZE, (void * const)(ZASMART_DEBUG_URL), ZA_UPGRADE_TASK_PRIORITY, NULL);

    za_system_msg_magic_box_update_report(ZANULL);

    za_innersor_wifi_led_shink_ctrl(ZA_TRUE);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_zachange_force(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_CHAR aZachangeUrl[255] = {0};

    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    if (za_common_get_zadiy_bin_path(p,aZachangeUrl, 255) == 0)
    {
        return ZA_CMD_ERR;
    }

    za_task_create(zasmart_force_ota_task, "za_zachange_task", ZA_UPGRADE_TASK_STACKSIZE, (void * const)(aZachangeUrl), ZA_UPGRADE_TASK_PRIORITY, NULL);
    
    za_system_msg_magic_box_update_report(ZANULL);

    za_innersor_wifi_led_shink_ctrl(ZA_TRUE);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_update_auto_default(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    if (za_storage_set_autoupdate_path(ZASMART_URL) != ZA_TRUE)
    {
        return ZA_CMD_ERR;
    }

    za_upgrade_set_force_mode();

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_update_auto_open(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    if (za_storage_set_autoupdate_path(ZASMART_URL_OPEN) != ZA_TRUE)
    {
        return ZA_CMD_ERR;
    }

    za_upgrade_set_force_mode();

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_update_manual_default(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    za_storage_delete_file_path(STORAGE_AUTO_UPDATE_APP_PATH);

    za_upgrade_set_ignore_mode();

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_update_set_path(ZA_CHAR *p, ZA_UCHAR len)
{
    if (za_storage_set_autoupdate_path(p) != ZA_TRUE)
    {
        return ZA_CMD_ERR;
    }

    za_upgrade_set_force_mode();

    return ZA_CMD_SUCCESS;
}
