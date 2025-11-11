/*   Local MQTT module in AI IoT . */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"

#include "zhengan.h"
#include "za_info.h"

#include "za_aiot.h"
#include "za_aiot_mqttlocal.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"

static const char *TAG = "ZA_AIOT_MQTTLOCAL";

typedef struct
{
    za_aiot_mqttlocal_type_t  tMqttLocalOpType;
    esp_mqtt_client_handle_t tMqttClientHandle;
} za_aiot_mqttlocal_data_t;

static za_aiot_mqttlocal_data_t tCurrentDataContext;

static const char *sZA_MQTTLOCAL_URL[] = {
    "null",                                 /* ZA_AIOT_COM_NULL */
    "/zdmedia/command/",                    /* ZA_AIOT_COM_MAIN_SERVICE */
    "null",                                 /* ZA_AIOT_COM_UPDATE_TOUCH_KEY */
    "/zdmedia/event/",                      /* ZA_AIOT_COM_UPDATE_EVENT */
    "/zdmedia/information/",                /* ZA_AIOT_COM_UPDATE_SYSINFO */
    "/zdmedia/bindresult/",                 /* ZA_AIOT_COM_BIND_SERVICE */
    "/zdmedia/bindcode/",                   /* ZA_AIOT_COM_UPDATA_BINDCODE */
    "/zdmedia/message/",                    /* ZA_AIOT_COM_MAIN_SERVICE_MESSAGE */
    "null",                                 /* ZA_AIOT_COM_TBD_DATA */
    "null",                                 /* ZA_AIOT_COM_TEST */
    "null"                                  /* ZA_AIOT_COM_END */
    };  // must be same as the za_aiot_com_type_t 

/** Private  **/

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ZA_ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static ZA_BOOL _za_aiot_mqttlocal_parse_callback(ZA_CHAR *sTopic, ZA_INT32 nTopicLength, ZA_CHAR *sData, ZA_INT32 nDataLength)
{
    za_aiot_com_type_t zaAiotComTyp = ZA_AIOT_COM_END;

    ZA_INT8 nStrCmpRet = zaAiotComTyp;

    ZA_CHAR aTopic[150] = {0};
    ZA_CHAR aTopicLocal[50] = {0};
    
    ZA_CHAR aMacAddr[30] = {0};

    ZA_CHAR *pTopicName = ZA_NULL;

    //ZA_ESP_LOGI(TAG, "_za_aiot_mqtt_parse debug --> topic: %.*s , topiclen: %d, data: %.*s , datalen: %d", nTopicLength, sTopic, nTopicLength, nDataLength, sData, nDataLength);

    if (nTopicLength <= 0)
    {
        ZA_ESP_LOGE(TAG, "_za_aiot_mqttlocal_parse_callback , nTopicLength must be more than 0");

        return ZA_FALSE;
    }

    pTopicName = aTopicLocal;
    za_info_get_mac_wifi_sta(aMacAddr, 20);


    //ZA_CHAR *pTopicStr = za_malloc(nTopicLength+1);
    ZA_CHAR *pTopicStr = aTopic;
    strncpy(pTopicStr, sTopic, nTopicLength);
    pTopicStr[nTopicLength] = '\0';

    for (ZA_UINT32 nCount = 0; nCount < zaAiotComTyp; nCount ++)
    {
        za_memset(pTopicName, 0, 50);
        strcpy(pTopicName, sZA_MQTTLOCAL_URL[nCount]);
        strcat(pTopicName, aMacAddr);
        if (za_strcmp(pTopicName, pTopicStr) == 0)
        {
            nStrCmpRet = nCount;

            //ZA_ESP_LOGI(TAG, "nStrCmpRet %d ", nStrCmpRet);

            break;
        }
    }

    //za_free(pTopicStr);

    if (nStrCmpRet == ZA_AIOT_COM_END)
    {
        ZA_ESP_LOGE(TAG, "_za_aiot_mqtts_parse_callback , can not find topic in array.");

        return ZA_FALSE;
    }

    // when use this callback please make a new function
    //za_aiot_callback_data((za_aiot_com_type_t)nStrCmpRet, sData, nDataLength);

    return ZA_TRUE;
}


/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqttlocal_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ZA_ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    //esp_mqtt_client_handle_t client = event->client;
    //int msg_id;

    za_app_msg_data_t za_appmsg_data;

    //za_aiot_msg_data_t t_AiotMsgData;

    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:
        {
            ZA_ESP_LOGI(TAG, "MQTTS_EVENT_CONNECTED");

            tCurrentDataContext.tMqttLocalOpType = ZA_AIOT_MQTTLOCAL_TYPE_RUNNING;

            za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_AIOTLOCAL_CONNECTED;

            if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(NULL), &za_appmsg_data, portMAX_DELAY))
            {
                ZA_ESP_LOGE(TAG, "za_msg_queue_send failure: %d", za_appmsg_data.za_appmsg_type);
            }
        }
        break;

        case MQTT_EVENT_DISCONNECTED:
        {
            ZA_ESP_LOGI(TAG, "MQTTS_EVENT_DISCONNECTED");

            tCurrentDataContext.tMqttLocalOpType = ZA_AIOT_MQTTLOCAL_TYPE_DISCONNECTED;

            za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_AIOTLOCAL_DISCONNECTED;

            if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(NULL), &za_appmsg_data, portMAX_DELAY))
            {
                ZA_ESP_LOGE(TAG, "za_msg_queue_send failure: %d", za_appmsg_data.za_appmsg_type);
            }

        }
        break;

        case MQTT_EVENT_SUBSCRIBED:
        {
            ZA_ESP_LOGI(TAG, "MQTTS_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);

        }
        break;

        case MQTT_EVENT_UNSUBSCRIBED:
        {
            ZA_ESP_LOGI(TAG, "MQTTS_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        }
        break;

        case MQTT_EVENT_PUBLISHED:
        {
            ZA_ESP_LOGI(TAG, "MQTTS_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        }
        break;

        case MQTT_EVENT_DATA:
        {
            ZA_ESP_LOGI(TAG, "MQTTS_EVENT_DATA");


            _za_aiot_mqttlocal_parse_callback(event->topic, event->topic_len, event->data, event->data_len); //for tmp debug
        }
        break;

        case MQTT_EVENT_ERROR:
        {
            ZA_ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
            {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ZA_ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
        }
        break;

        default:
        {
            ZA_ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        }
        break;
    }
}


/** Public **/

ZA_BOOL za_aiot_mqttlocal_init(const ZA_CHAR * sMqttServUrl, const ZA_CHAR * sUserName, const ZA_CHAR * sPassWord)
{
    esp_log_level_set("ZA_AIOT_MQTTLOCAL", ESP_LOG_VERBOSE);

    za_memset(&tCurrentDataContext, 0, za_sizeof(za_aiot_mqttlocal_data_t));

    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = sMqttServUrl
        },
        .credentials = {
            .username = (const char *)sUserName,
            .authentication.password = (const char *)sPassWord,
        },
        .session = {
            .keepalive = 10
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqttlocal_event_handler, NULL);

    esp_mqtt_client_start(client);

    tCurrentDataContext.tMqttLocalOpType = ZA_AIOT_MQTTLOCAL_TYPE_INITED;
    tCurrentDataContext.tMqttClientHandle = client;

    return ZA_TRUE;
}

ZA_BOOL za_aiot_mqttlocal_term(ZA_VOID *pvParameter)
{
    (void) pvParameter;

    if ((tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_INITED) || (tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_RUNNING))
    {
        if (tCurrentDataContext.tMqttClientHandle != ZA_NULL)
        {
            esp_mqtt_client_stop(tCurrentDataContext.tMqttClientHandle);
            
            if (esp_mqtt_client_destroy(tCurrentDataContext.tMqttClientHandle) != ESP_OK)
            {
                ZA_ESP_LOGE(TAG, "esp_mqtt_client_destroy failed in za_aiot_mqttlocal_term");

                return ZA_FALSE;
            }
        }
        else
        {
            ZA_ESP_LOGE(TAG, "za_aiot_mqttlocal_term failed , the handle of mqttlocal is null");

            return ZA_FALSE;
        }

        tCurrentDataContext.tMqttLocalOpType = ZA_AIOT_MQTTLOCAL_TYPE_TERMED;
        tCurrentDataContext.tMqttClientHandle = ZA_NULL;
    }

    return ZA_TRUE;
}

ZA_BOOL za_aiot_mqttlocal_push_data(za_aiot_com_data_t tAiotComData)
{
    ZA_INT32 msg_id;

    ZA_BOOL bRetVal = ZA_FALSE;

    ZA_CHAR aMacAddr[30] = {0};
    ZA_CHAR aTopic[150] = {0};
    ZA_CHAR *pTopicName = ZA_NULL;

    pTopicName = aTopic;

    if ((tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_INITED) || (tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_RUNNING))
    {
        if (tCurrentDataContext.tMqttClientHandle != ZA_NULL)
        {
            za_info_get_mac_wifi_sta(aMacAddr, 20);

            strcpy(pTopicName, sZA_MQTTLOCAL_URL[tAiotComData.tZaAiotComType]);
            strcat(pTopicName, aMacAddr);

            switch(tAiotComData.tZaComlevel)
            {
                case ZA_AIOT_COMLEVEL_L1:
                {
                    msg_id = esp_mqtt_client_publish(tCurrentDataContext.tMqttClientHandle, pTopicName, tAiotComData.p_data, 0, 0, 0);
                }
                break;

                default:
                {
                    msg_id = esp_mqtt_client_publish(tCurrentDataContext.tMqttClientHandle, pTopicName, tAiotComData.p_data, 0, 1, 0);
                }
                break;
            }

            ZA_ESP_LOGI(TAG, "za_aiot_mqttlocal_push_data > %s msg_id:%d", sZA_MQTTLOCAL_URL[tAiotComData.tZaAiotComType], msg_id);

            bRetVal = ZA_TRUE;
        }
        else
        {
            ZA_ESP_LOGE(TAG, "za_aiot_mqttlocal_push_data failed , the handle of mqttlocal is null");
        }
    }

    return bRetVal;
}

ZA_BOOL za_aiot_mqttlocal_push_topic_data(za_aiot_com_data_t tAiotComData)
{
    ZA_INT32 msg_id;

    ZA_BOOL bRetVal = ZA_FALSE;

    ZA_CHAR aMacAddr[30] = {0};
    ZA_CHAR aTopic[150] = {0};
    ZA_CHAR *pTopicName = ZA_NULL;

    pTopicName = aTopic;

    if ((tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_INITED) || (tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_RUNNING))
    {
        if (tCurrentDataContext.tMqttClientHandle != ZA_NULL)
        {
            za_info_get_mac_wifi_sta(aMacAddr, 20);

            strcpy(pTopicName, sZA_MQTTLOCAL_URL[tAiotComData.tZaAiotComType]);
            strcat(pTopicName, tAiotComData.p_data);

            switch(tAiotComData.tZaComlevel)
            {
                case ZA_AIOT_COMLEVEL_L1:
                {
                    msg_id = esp_mqtt_client_publish(tCurrentDataContext.tMqttClientHandle, pTopicName, aMacAddr, 0, 0, 0);
                }
                break;

                default:
                {
                    msg_id = esp_mqtt_client_publish(tCurrentDataContext.tMqttClientHandle, pTopicName, aMacAddr, 0, 1, 0);
                }
                break;
            }

            ZA_ESP_LOGI(TAG, "za_aiot_mqttlocal_push_topic_data > %s msg_id:%d",sZA_MQTTLOCAL_URL[tAiotComData.tZaAiotComType], msg_id);

            bRetVal = ZA_TRUE;
        }
        else
        {
            ZA_ESP_LOGE(TAG, "za_aiot_mqttlocal_push_topic_data failed , the handle of mqttlocal is null");
        }
    }

    return bRetVal;
}

ZA_BOOL za_aiot_mqttlocal_push_client_data(const ZA_CHAR * sClientIdata, za_aiot_com_data_t tAiotComData)
{
    ZA_INT32 msg_id;

    ZA_BOOL bRetVal = ZA_FALSE;

    ZA_CHAR aTopic[150] = {0};
    ZA_CHAR *pTopicName = ZA_NULL;

    pTopicName = aTopic;

    if ((tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_INITED) || (tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_RUNNING))
    {
        if (tCurrentDataContext.tMqttClientHandle != ZA_NULL)
        {
            strcpy(pTopicName, sZA_MQTTLOCAL_URL[tAiotComData.tZaAiotComType]);
            strcat(pTopicName, sClientIdata);

            switch(tAiotComData.tZaComlevel)
            {
                case ZA_AIOT_COMLEVEL_L1:
                {
                    msg_id = esp_mqtt_client_publish(tCurrentDataContext.tMqttClientHandle, pTopicName, tAiotComData.p_data, 0, 0, 0);
                }
                break;

                default:
                {
                    msg_id = esp_mqtt_client_publish(tCurrentDataContext.tMqttClientHandle, pTopicName, tAiotComData.p_data, 0, 1, 0);
                }
                break;
            }

            ZA_ESP_LOGI(TAG, "za_aiot_mqttlocal_push_client_data > %s msg_id:%d",sZA_MQTTLOCAL_URL[tAiotComData.tZaAiotComType], msg_id);

            bRetVal = ZA_TRUE;
        }
        else
        {
            ZA_ESP_LOGE(TAG, "za_aiot_mqttlocal_push_client_data failed , the handle of mqttlocal is null");
        }

    }

    return bRetVal;
}

ZA_BOOL za_aiot_mqttlocal_pull_data(za_aiot_com_data_t tAiotComData)
{
    ZA_INT32 msg_id;

    ZA_BOOL bRetVal = ZA_FALSE;

    ZA_CHAR aMacAddr[30] = {0};
    ZA_CHAR aTopic[150] = {0};
    ZA_CHAR *pTopicName = ZA_NULL;

    pTopicName = aTopic;

    if ((tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_INITED) || (tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_RUNNING))
    {
        if (tCurrentDataContext.tMqttClientHandle != ZA_NULL)
        {
            za_info_get_mac_wifi_sta(aMacAddr, 20);

            strcpy(pTopicName, sZA_MQTTLOCAL_URL[tAiotComData.tZaAiotComType]);
            strcat(pTopicName, aMacAddr);

            switch(tAiotComData.tZaComlevel)
            {
                case ZA_AIOT_COMLEVEL_L1:
                {
                    msg_id = esp_mqtt_client_subscribe(tCurrentDataContext.tMqttClientHandle, pTopicName, 0);
                }
                break;

                default:
                {
                    msg_id = esp_mqtt_client_subscribe(tCurrentDataContext.tMqttClientHandle, pTopicName, 1);
                }
                break;
            }

            ZA_ESP_LOGI(TAG, "za_aiot_mqttlocal_pull_data > %s msg_id:%d", sZA_MQTTLOCAL_URL[tAiotComData.tZaAiotComType], msg_id);

            bRetVal = ZA_TRUE;
        }
        else
        {
            ZA_ESP_LOGE(TAG, "za_aiot_mqttlocal_pull_data failed , the handle of mqttlocal is null");
        }
    }
    
    return bRetVal;
}

ZA_BOOL za_aiot_mqttlocal_refuse_data(za_aiot_com_data_t tAiotComData)
{
    ZA_INT32 msg_id;

    ZA_BOOL bRetVal = ZA_FALSE;

    ZA_CHAR aMacAddr[30] = {0};
    ZA_CHAR aTopic[150] = {0};
    ZA_CHAR *pTopicName = ZA_NULL;

    pTopicName = aTopic;

    if ((tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_INITED) || (tCurrentDataContext.tMqttLocalOpType == ZA_AIOT_MQTTLOCAL_TYPE_RUNNING))
    {
        if (tCurrentDataContext.tMqttClientHandle != ZA_NULL)
        {
            za_info_get_mac_wifi_sta(aMacAddr, 20);

            strcpy(pTopicName, sZA_MQTTLOCAL_URL[tAiotComData.tZaAiotComType]);
            strcat(pTopicName, aMacAddr);

            msg_id = esp_mqtt_client_unsubscribe(tCurrentDataContext.tMqttClientHandle, pTopicName);

            ZA_ESP_LOGI(TAG, "za_aiot_mqttlocal_refuse_data > %s msg_id:%d",sZA_MQTTLOCAL_URL[tAiotComData.tZaAiotComType], msg_id);

            bRetVal = ZA_TRUE;
        }
        else
        {
            ZA_ESP_LOGE(TAG, "za_aiot_mqttlocal_refuse_data failed , the handle of mqttlocal is null");
        }
    }
    
    return bRetVal;
}
