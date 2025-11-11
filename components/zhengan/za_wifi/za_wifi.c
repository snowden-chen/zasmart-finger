/*  wifi station & wifi mesh , maybe. */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"

#include "zhengan.h"

#include "za_wifi.h"

#include "esp_wifi.h"

#include "lwip/err.h"
#include "lwip/sys.h"

static const char *TAG = "ZA_WIFI";

//static int s_retry_num = 0;

/* FreeRTOS event group to signal when we are connected*/
//static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
//#define WIFI_CONNECTED_BIT BIT0
//#define WIFI_FAIL_BIT      BIT1



/*** private ***/

static void sta_wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    //wifi_event_sta_connected_t *event;

    /*
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (s_retry_num < ZA_WIFI_STA_MAXIMUM_RETRY) 
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }

        ESP_LOGI(TAG,"connect to the AP fail");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    */

    switch (event_id)
    {
        case WIFI_EVENT_STA_START:
        {
            ZA_ESP_LOGI(TAG, "WIFI_EVENT_STA_START . \n");

            //esp_wifi_disconnect();
            esp_wifi_connect();
        }
        break;

        case WIFI_EVENT_STA_CONNECTED:
        {
            ZA_ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED . \n");
            //esp_wifi_connect();

            za_sys_msg_data_t za_sysmsg_data;
            za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_WIFI_STATUS_CONNECTED;

            if (za_get_sys_msg_handle(ZA_NULL) != ZA_NULL)
            {
                if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
                {
                    ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_sysmsg_data.za_sysmsg_type);
                }
            }
        }
        break;

        case WIFI_EVENT_STA_DISCONNECTED:
        {
            ZA_ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED . \n");
            //esp_wifi_connect();

            za_sys_msg_data_t za_sysmsg_data;
            za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_WIFI_STATUS_UNCONNECTED;

            if (za_get_sys_msg_handle(ZA_NULL) != ZA_NULL)
            {
                if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
                {
                    ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_sysmsg_data.za_sysmsg_type);
                }
            }            
        }
        break;

        case WIFI_EVENT_AP_START:
        {

        }
        break;

        case WIFI_EVENT_SCAN_DONE:
        {
            ZA_ESP_LOGI(TAG, "WIFI_EVENT_SCAN_DONE .");

            ZA_UINT16 apCount = 0;
            esp_wifi_scan_get_ap_num(&apCount);
            if (apCount == 0)
            {
                ZA_ESP_LOGI(TAG, "Nothing AP found.");

                break;
            }

            wifi_ap_record_t *ap_list = (wifi_ap_record_t *)za_malloc(sizeof(wifi_ap_record_t) * apCount);
            if (!ap_list)
            {
                ZA_ESP_LOGE(TAG, "malloc error, ap_list is NULL");
                break;
            }

            ZA_ESPERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, ap_list));

            /*

            esp_blufi_ap_record_t * blufi_ap_list = (esp_blufi_ap_record_t *)za_malloc(apCount * za_sizeof(esp_blufi_ap_record_t));
            if (!blufi_ap_list)
            {
                if (ap_list)
                {
                    za_free(ap_list);
                }

                ZA_ESP_LOGE(TAG, "malloc error, blufi_ap_list is NULL");

                break;
            }

            for (int i = 0; i < apCount; ++i)
            {
                blufi_ap_list[i].rssi = ap_list[i].rssi;
                za_memcpy(blufi_ap_list[i].ssid, ap_list[i].ssid, sizeof(ap_list[i].ssid));
            }

            if (ble_is_connected == true)
            {
                //esp_blufi_send_wifi_list(apCount, blufi_ap_list);
                ZA_ESP_LOGI(TAG, "Could send wifi list to Ble Host.");
            }
            else
            {
                ZA_ESP_LOGI(TAG, "BLUFI BLE is not connected yet\n");
            }
            */

            esp_wifi_scan_stop();

            za_free(ap_list);
            //free(blufi_ap_list);

        }
        break;

        default:
            break;
    }

    return;
}

static ip_event_got_ip_t tIpEventData;

static void sta_ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    switch (event_id)
    {
        case IP_EVENT_STA_GOT_IP: 
        {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ZA_ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));

            za_memcpy(&tIpEventData, event, sizeof(ip_event_got_ip_t));
        }
        break;

        default:
            break;
    }

    return;
}

/*** public ***/

ZA_BOOL za_wifi_init()
{
    /* Init Wifi Station Mode */
    ZA_ESP_LOGI(TAG, "ZA WIFI INIT ENTER");

    //s_wifi_event_group = xEventGroupCreate();

    ZA_ESPERROR_CHECK(esp_netif_init());

    ZA_ESPERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    za_configASSERT(sta_netif);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &sta_wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));


    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &sta_ip_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ZA_ESPERROR_CHECK(esp_wifi_init(&cfg));

    //ZA_ESPERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ZA_ESPERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_FLASH) );
    ZA_ESPERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ZA_ESPERROR_CHECK(esp_wifi_start());

    /*
    // config wifi default if wifi params is null 
    */
    wifi_config_t tWifiConfigParams = za_app_get_current_wifi(ZANULL);
    ZA_ESP_LOGI(TAG, "wifi init -> ssid:%s, pwd:%s.", (const ZA_CHAR *)(tWifiConfigParams.sta.ssid), (const ZA_CHAR *)(tWifiConfigParams.sta.password));

    return ZA_TRUE;
}

ZA_VOID za_wifi_disconnect_from_ap_set_null(ZA_VOID *pvParameter)
{
    (void) pvParameter;

    za_sys_msg_data_t za_sysmsg_data;
    za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_WIFI_REQ_DISCONNECT_AND_SET_NULL;

    if (za_get_sys_msg_handle(ZA_NULL) != ZA_NULL)
    {
        if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
        {
            ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_sysmsg_data.za_sysmsg_type);
        }
    }    

    return;
}

ZA_VOID za_wifi_print_local_ip_addr(ZA_VOID *pvParameter)
{
    (void) pvParameter;

    printf("WiFi Ip Addr : ");
    printf(IPSTR,IP2STR(&tIpEventData.ip_info.ip));
    printf("\n");

    return;
}
