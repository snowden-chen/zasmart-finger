/*   Bluetooth config wifi connection to ap in 2.4GHZ . */

/*
   NOTE: Writed By ZachSnowdenChen
*/

#include "za_common.h"

#include "zhengan.h"

#include "za_info.h"

#include "esp_bt.h"
#include "esp_blufi_api.h"
#include "esp_blufi.h"

#include "esp_gap_ble_api.h"

#include "blufi_example.h"

#include "za_blufi.h"

#include "za_storage.h"

static const char *TAG = "ZA_BLUFI";

static wifi_config_t sta_config;
static wifi_config_t ap_config;

static bool ble_is_connected = false;

static ZA_VOID _za_blufi_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);
static esp_blufi_callbacks_t za_blufi_callbacks = {
    .event_cb = _za_blufi_event_callback,
    .negotiate_data_handler = blufi_dh_negotiate_data_handler,
    .encrypt_func = blufi_aes_encrypt,
    .decrypt_func = blufi_aes_decrypt,
    .checksum_func = blufi_crc_checksum,
};

static uint8_t blufi_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
};

static esp_ble_adv_data_t za_blufi_adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 16,
    .p_service_uuid = blufi_service_uuid128,
    .flag = 0x6,
};

/*** private ***/

static ZA_VOID _za_blufi_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
{
    switch(event)
    {
        case ESP_BLUFI_EVENT_INIT_FINISH:
        {
            ZA_ESP_LOGI(TAG, "BLUFI init finish\n");

            esp_ble_gap_set_device_name("ZADIY");
            esp_ble_gap_config_adv_data(&za_blufi_adv_data);
            //esp_blufi_adv_start();
        }
        break;

        case ESP_BLUFI_EVENT_DEINIT_FINISH:
        {
            ZA_ESP_LOGI(TAG, "BLUFI deinit finish\n");
        }
        break;

        case ESP_BLUFI_EVENT_BLE_CONNECT:
        {
            ZA_ESP_LOGI(TAG, "BLUFI ble connect\n");

            ble_is_connected = true;

            esp_blufi_adv_stop();

            blufi_security_init();

            za_sys_msg_data_t za_sysmsg_data;
            za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_BLUFI_STATUS_CONNECTED;

            if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
            {
                ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_sysmsg_data.za_sysmsg_type);
            }
        }
        break;

        case ESP_BLUFI_EVENT_BLE_DISCONNECT:
        {
            ZA_ESP_LOGI(TAG, "BLUFI ble disconnect\n");

            ble_is_connected = false;

            blufi_security_deinit();

            esp_ble_gap_set_device_name("ZADIY");
            esp_ble_gap_config_adv_data(&za_blufi_adv_data);
            //esp_blufi_adv_start();

            za_sys_msg_data_t za_sysmsg_data;
            za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_BLUFI_STATUS_UNCONNECTED;

            if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
            {
                ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_sysmsg_data.za_sysmsg_type);
            }
        }
        break;

        case ESP_BLUFI_EVENT_SET_WIFI_OPMODE:
        {
            ZA_ESP_LOGI(TAG, "BLUFI Set WIFI opmode %d\n", param->wifi_mode.op_mode);
            //ESP_ERROR_CHECK( esp_wifi_set_mode(param->wifi_mode.op_mode) );

            za_sys_msg_data_t za_sysmsg_data;
            za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_WIFI_SET_OP_MODE;
            za_sysmsg_data.n_param1 = (ZA_UINT32)(param->wifi_mode.op_mode);

            if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
            {
                ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_sysmsg_data.za_sysmsg_type);
            }
        }
        break;

        case ESP_BLUFI_EVENT_REQ_CONNECT_TO_AP:
        {
            ZA_ESP_LOGI(TAG, "BLUFI requset wifi connect to AP\n");
            /* there is no wifi callback when the device has already connected to this wifi
            so disconnect wifi before connection.
            */
            //esp_wifi_disconnect();
            //esp_wifi_connect();

            za_sys_msg_data_t za_sysmsg_data;
            za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_WIFI_CONNECT_TO_AP;

            if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
            {
                ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_sysmsg_data.za_sysmsg_type);
            }
        }   
        break;

        case ESP_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
        {
            ZA_ESP_LOGI(TAG, "BLUFI requset wifi disconnect from AP\n");
            //esp_wifi_disconnect();
        }
        break;

        case ESP_BLUFI_EVENT_REPORT_ERROR:
        {
            ZA_ESP_LOGI(TAG, "BLUFI report error, error code %d\n", param->report_error.state);
            esp_blufi_send_error_info(param->report_error.state);
        }
        break;

        case ESP_BLUFI_EVENT_GET_WIFI_STATUS: 
        {
            /*
            wifi_mode_t mode;
            esp_blufi_extra_info_t info;

            esp_wifi_get_mode(&mode);

            if (gl_sta_connected) 
            {
                memset(&info, 0, sizeof(esp_blufi_extra_info_t));
                memcpy(info.sta_bssid, gl_sta_bssid, 6);
                info.sta_bssid_set = true;
                info.sta_ssid = gl_sta_ssid;
                info.sta_ssid_len = gl_sta_ssid_len;
                esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
            } 
            else 
            {
                esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, 0, NULL);
            }
            */

            ZA_ESP_LOGI(TAG, "BLUFI get wifi status from AP\n");
        }
        break;

        case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
        {
            ZA_ESP_LOGI(TAG, "blufi close a gatt connection");
            esp_blufi_disconnect();
        }
        break;

        case ESP_BLUFI_EVENT_DEAUTHENTICATE_STA:
        {
            /* TODO */
        }
        break;

	    case ESP_BLUFI_EVENT_RECV_STA_BSSID:
        {
            za_memcpy(sta_config.sta.bssid, param->sta_bssid.bssid, 6);

            sta_config.sta.bssid_set = 1;

            //esp_wifi_set_config(WIFI_IF_STA, &sta_config);

            ZA_ESP_LOGI(TAG, "Recv STA BSSID %s\n", sta_config.sta.ssid);
        }
        break;

	    case ESP_BLUFI_EVENT_RECV_STA_SSID:
        {
            za_strncpy((ZA_CHAR *)sta_config.sta.ssid, (ZA_CHAR *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
            
            sta_config.sta.ssid[param->sta_ssid.ssid_len] = '\0';

            //esp_wifi_set_config(WIFI_IF_STA, &sta_config);

            ZA_VOID *pWifiConfig = za_malloc(za_sizeof(wifi_config_t));
            za_sys_msg_data_t za_sysmsg_data;

            za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_WIFI_CONFIG_SSID;
            za_memcpy(pWifiConfig, &sta_config, za_sizeof(wifi_config_t));
            za_sysmsg_data.p_data = pWifiConfig;

            if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(NULL), &za_sysmsg_data, portMAX_DELAY))
            {
                ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_sysmsg_data.za_sysmsg_type);
            }

            ZA_ESP_LOGI(TAG, "Recv STA SSID %s\n", sta_config.sta.ssid);
        }
        break;

        case ESP_BLUFI_EVENT_RECV_STA_PASSWD:
        {
            za_strncpy((ZA_CHAR *)sta_config.sta.password, (ZA_CHAR *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
            
            sta_config.sta.password[param->sta_passwd.passwd_len] = '\0';
            
            //esp_wifi_set_config(WIFI_IF_STA, &sta_config);

            ZA_VOID *pWifiConfig = za_malloc(za_sizeof(wifi_config_t));
            za_sys_msg_data_t za_sysmsg_data;

            za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_WIFI_CONFIG_PASSWD;
            za_memcpy(pWifiConfig, &sta_config, za_sizeof(wifi_config_t));
            za_sysmsg_data.p_data = pWifiConfig;

            if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(NULL), &za_sysmsg_data, portMAX_DELAY))
            {
                ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_sysmsg_data.za_sysmsg_type);
            }
            
            ZA_ESP_LOGI(TAG, "Recv STA PASSWORD %s\n", sta_config.sta.password);
        }
        break;

        case ESP_BLUFI_EVENT_RECV_SOFTAP_SSID:
        {
            za_strncpy((ZA_CHAR *)ap_config.ap.ssid, (ZA_CHAR *)param->softap_ssid.ssid, param->softap_ssid.ssid_len);
            ap_config.ap.ssid[param->softap_ssid.ssid_len] = '\0';
            ap_config.ap.ssid_len = param->softap_ssid.ssid_len;
            
            //esp_wifi_set_config(WIFI_IF_AP, &ap_config);
            
            ZA_ESP_LOGI(TAG, "Recv SOFTAP SSID %s, ssid len %d\n", ap_config.ap.ssid, ap_config.ap.ssid_len);
        }
        break;

        case ESP_BLUFI_EVENT_RECV_SOFTAP_PASSWD:
        {
            za_strncpy((ZA_CHAR *)ap_config.ap.password, (ZA_CHAR *)param->softap_passwd.passwd, param->softap_passwd.passwd_len);
            
            ap_config.ap.password[param->softap_passwd.passwd_len] = '\0';
            
            //esp_wifi_set_config(WIFI_IF_AP, &ap_config);
            
            ZA_ESP_LOGI(TAG, "Recv SOFTAP PASSWORD %s len = %d\n", ap_config.ap.password, param->softap_passwd.passwd_len);
        }
        break;

        case ESP_BLUFI_EVENT_RECV_SOFTAP_MAX_CONN_NUM:
        {
            if (param->softap_max_conn_num.max_conn_num > 4)
            {
                return;
            }

            ap_config.ap.max_connection = param->softap_max_conn_num.max_conn_num;

            //esp_wifi_set_config(WIFI_IF_AP, &ap_config);

            ZA_ESP_LOGI(TAG, "Recv SOFTAP MAX CONN NUM %d\n", ap_config.ap.max_connection);
        }
        break;

        case ESP_BLUFI_EVENT_RECV_SOFTAP_AUTH_MODE:
        {
            if (param->softap_auth_mode.auth_mode >= WIFI_AUTH_MAX) 
            {
                return;
            }

            ap_config.ap.authmode = param->softap_auth_mode.auth_mode;

            //esp_wifi_set_config(WIFI_IF_AP, &ap_config);

            ZA_ESP_LOGI(TAG, "Recv SOFTAP AUTH MODE %d\n", ap_config.ap.authmode);
        }
        break;

        case ESP_BLUFI_EVENT_RECV_SOFTAP_CHANNEL:
        {
            if (param->softap_channel.channel > 13)
            {
                return;
            }

            ap_config.ap.channel = param->softap_channel.channel;

            //esp_wifi_set_config(WIFI_IF_AP, &ap_config);

            ZA_ESP_LOGI(TAG, "Recv SOFTAP CHANNEL %d\n", ap_config.ap.channel);
        }
        break;

        case ESP_BLUFI_EVENT_GET_WIFI_LIST:
        {
            /*
            wifi_scan_config_t scanConf = {
                .ssid = NULL,
                .bssid = NULL,
                .channel = 0,
                .show_hidden = false
            };
            esp_wifi_scan_start(&scanConf, true);
            */

            ZA_ESP_LOGI(TAG, "Start scan wifi to get wifi list.");
        }
        break;

        case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:
        {
            ZA_ESP_LOGI(TAG, "Recv Custom Data %lu\n", param->custom_data.data_len);
            //za_esp_log_buffer_hex(TAG, param->custom_data.data, param->custom_data.data_len);
            za_esp_log_buffer_char(TAG, param->custom_data.data, param->custom_data.data_len);

            if ((param->custom_data.data_len >= ZA_LONGMORE_CMD_HEAD_LEN) && (param->custom_data.data_len < ZA_SYS_DATA_BUF_LEN))
            {
                za_sys_msg_data_t za_sysmsg_data;
                za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_BLE_BLUFI_CUSTOM_DATA_RECV;

                za_memset(za_sysmsg_data.aDataBuffer,0,ZA_SYS_DATA_BUF_LEN);
                za_memcpy(za_sysmsg_data.aDataBuffer, param->custom_data.data, param->custom_data.data_len);

                if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
                {
                    ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_sysmsg_data.za_sysmsg_type);
                }

                ZA_CHAR aMacAddr[30] = {0};

                za_info_get_mac_wifi_sta(aMacAddr, 20);

                esp_blufi_send_custom_data((uint8_t *)aMacAddr, za_sizeof(aMacAddr)); 
            }

        }
        break;

        case ESP_BLUFI_EVENT_RECV_USERNAME:
        {
            /* Not handle currently */
            ZA_ESP_LOGI(TAG, "ESP_BLUFI_EVENT_RECV_USERNAME.");
        }
        break;

        case ESP_BLUFI_EVENT_RECV_CA_CERT:
        {
            /* Not handle currently */
            ZA_ESP_LOGI(TAG, "ESP_BLUFI_EVENT_RECV_CA_CERT.");
        }
        break;

        case ESP_BLUFI_EVENT_RECV_CLIENT_CERT:
        {
            /* Not handle currently */
            ZA_ESP_LOGI(TAG, "ESP_BLUFI_EVENT_RECV_CLIENT_CERT.");
        }
        break;

        case ESP_BLUFI_EVENT_RECV_SERVER_CERT:
        {
            /* Not handle currently */
            ZA_ESP_LOGI(TAG, "ESP_BLUFI_EVENT_RECV_SERVER_CERT.");
        }
        break;

        case ESP_BLUFI_EVENT_RECV_CLIENT_PRIV_KEY:
        {
            /* Not handle currently */
            ZA_ESP_LOGI(TAG, "ESP_BLUFI_EVENT_RECV_CLIENT_PRIV_KEY.");
        }
        break;

        case ESP_BLUFI_EVENT_RECV_SERVER_PRIV_KEY:
        {
            /* Not handle currently */
            ZA_ESP_LOGI(TAG, "ESP_BLUFI_EVENT_RECV_SERVER_PRIV_KEY.");
        }
        break;

        default:
            break;
    }

    return;
}


/*** public ***/

ZA_BOOL za_blufi_init()
{
    za_esp_err_t ret;

    ZA_ESP_LOGI(TAG, "ZA BLUFI INIT ENTER");

    za_storage_sys_mode_t tSystemMode = za_storage_get_system_mode(NULL);

    if ((tSystemMode.tWirelessMode == ZA_STORAGE_BLE_HOST_MODE) 
    || (tSystemMode.tWirelessMode == ZA_STORAGE_BLEHOST_WIFICLIENT_MODE))
    {
        ZA_ESPERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

        ret = esp_bt_controller_init(&bt_cfg);
        if (ret) 
        {
            ZA_ESP_LOGE(TAG, "%s initialize bt controller failed: %s\n", __func__, za_esp_err_to_name(ret));
        }
        
        ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
        if (ret) 
        {
            ZA_ESP_LOGE(TAG, "%s enable bt controller failed: %s\n", __func__, za_esp_err_to_name(ret));
            
            return ZA_FALSE;
        }

        ret = esp_blufi_host_and_cb_init(&za_blufi_callbacks);
        if (ret) 
        {
            ZA_ESP_LOGE(TAG, "%s initialise failed: %s\n", __func__, za_esp_err_to_name(ret));
            
            return ZA_FALSE;
        }

        ZA_ESP_LOGI(TAG, "BLUFI VERSION %04x\n", esp_blufi_get_version());

        return ZA_TRUE;
    }

    ZA_ESP_LOGI(TAG, "ZA BLUFI NOT INITIALIZED.");

    return ZA_TRUE;
}
