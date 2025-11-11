/*   AI IoT to server by wifi or ble mesh use MQTT or others. */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"

#include "za_storage.h"

#include "za_aiot.h"

#include "za_aiot_mqtt.h"
#include "za_aiot_mqtts.h"
#include "za_aiot_mqttlocal.h"
#include "za_aiot_udp.h"

static const char *TAG = "ZA_AIOT";

//static const char *MQTT_TEST_URL = "mqtt://mqtt.eclipseprojects.io";
static const char *MQTT_SER_URL = "ws://172.23.1.14:18830";
//static const char *MQTT_SER_URL = "ws://192.168.1.113:18830";
static const char *MQTTS_SER_URL = "wss://mqtt.zdmedia.com.cn";

//#define ZA_AIOT_MSG_QUEUE_DEEPTH (32)
#define ZA_AIOT_MSG_TASK_STACKSIZE ZA_BUFFER_6K
#define ZA_AIOT_MSG_TASK_PRIORITY 5

typedef ZA_BOOL (* za_aiot_com_init_func_t)(const ZA_CHAR * sMqttServUrl);
typedef ZA_BOOL (* za_aiot_local_init_func_t)(const ZA_CHAR * sServUrl, const ZA_CHAR * sUserName, const ZA_CHAR * sPassWord);
typedef ZA_BOOL (* za_aiot_com_term_func_t)(ZA_VOID *pvParameter);
typedef ZA_BOOL (* za_aiot_com_reconnect_func_t)(ZA_VOID *pvParameter);
typedef ZA_BOOL (* za_aiot_com_push_func_t)(za_aiot_com_data_t tAiotComData);
typedef ZA_BOOL (* za_aiot_com_pushtop_func_t)(za_aiot_com_data_t tAiotComData);
typedef ZA_BOOL (* za_aiot_com_pushclient_func_t)(const ZA_CHAR * sClientIdata, za_aiot_com_data_t tAiotComData);
typedef ZA_BOOL (* za_aiot_com_pull_func_t)(za_aiot_com_data_t tAiotComData);
typedef ZA_BOOL (* za_aiot_com_refuse_func_t)(za_aiot_com_data_t tAiotComData);

typedef struct
{
   ZA_BOOL b_IfInited;

   ZA_BOOL b_IfConnected;
   za_aiot_protocol_type_t tProtocolCur;

   ZA_BOOL b_IfLoSubConnected;
   za_aiot_protocol_type_t tLoSubProtocolCur;

   //ZA_QueueHandle_t h_AiotQueueMsg;
   ZA_SemaphoreHandle_t h_AiotSemphMutex;
   ZA_SemaphoreHandle_t h_AiotSemphMutexCallback;

   za_aiot_com_init_func_t       tfunc_ComInit;
   za_aiot_com_term_func_t       tfunc_ComTerm;
   za_aiot_com_reconnect_func_t  tfunc_ComReconnect;
   za_aiot_com_push_func_t       tfunc_ComPush;
   za_aiot_com_pushtop_func_t    tfunc_ComPushTopData;
   za_aiot_com_pushclient_func_t tfunc_ComPushClientData;
   za_aiot_com_pull_func_t       tfunc_ComPull;
   za_aiot_com_refuse_func_t     tfunc_ComRefuse;
   za_aiot_data_callback         tfunc_CallBack[ZA_AIOT_COM_END];

   za_aiot_local_init_func_t        tfunc_LocalSubInit;
   za_aiot_com_term_func_t          tfunc_LocalSubTerm;
   za_aiot_com_push_func_t          tfunc_LocalSubPush;
   za_aiot_com_pushtop_func_t       tfunc_LocalSubPushTopData;
   za_aiot_com_pull_func_t          tfunc_LocalSubPull;
   za_aiot_com_refuse_func_t        tfunc_LocalSubRefuse;
   za_aiot_data_callback            tfunc_LSCallBack[ZA_AIOT_COM_END];
} za_aiot_context_t;

typedef struct
{
    /* data */
    za_aiot_com_type_t tAiotComType;
    ZA_CHAR aDatabuf[ZA_APPEXT_MAX_DATA_LEN];
    ZA_INT32 nDataLength;
    ZA_BOOL bIfBufing;
} za_aiot_com_databuf_t;

typedef struct
{
   /* data */
   ZA_BOOL bIfInited;

   ZA_BOOL bIfConnected;

   ZA_CHAR aCurLinkUrl[ZA_SHORT_URL_LEN];
   ZA_CHAR aCurServIPv4[ZA_LOCALSERVER_IP];
   ZA_UINT16 nCurServPort;
   ZA_CHAR aCurUName[ZA_USER_LEN];
   ZA_CHAR aCurUPassWD[ZA_PASSWORD_LEN];

   ZA_CHAR aSetLinkUrl[ZA_SHORT_URL_LEN];
   ZA_CHAR aSetServIPv4[ZA_LOCALSERVER_IP];
   ZA_UINT16 nSetServPort;
   ZA_CHAR aSetUName[ZA_USER_LEN];
   ZA_CHAR aSetUPassWD[ZA_PASSWORD_LEN];

} za_aiot_local_status;


static za_aiot_com_databuf_t tAiotCurDataBuf;

static za_aiot_context_t tAiotContext = {0};

static za_aiot_local_status tAiotLocalCurStatus;

void _za_aiot_task(void *arg);

/** Public **/

ZA_BOOL za_aiot_init()
{
   ZA_ESP_LOGI(TAG, "ZA AIOT ENTER");

   za_memset(&tAiotContext, 0, sizeof(za_aiot_context_t));
   za_memset(&tAiotLocalCurStatus, 0, sizeof(za_aiot_local_status));

   tAiotContext.h_AiotSemphMutex = za_semaphore_create(1,1);
   tAiotContext.h_AiotSemphMutexCallback = za_semaphore_create(1,1);

   za_memset(&tAiotCurDataBuf, 0, za_sizeof(za_aiot_com_databuf_t));

   //tAiotContext.h_AiotQueueMsg = za_msg_queue_create(ZA_AIOT_MSG_QUEUE_DEEPTH, za_sizeof(za_aiot_msg_data_t));

   za_task_create(_za_aiot_task, "za_aiot_task", ZA_AIOT_MSG_TASK_STACKSIZE, NULL, ZA_AIOT_MSG_TASK_PRIORITY, NULL);

   tAiotContext.b_IfInited = ZA_TRUE;

   return ZA_TRUE;
}

ZA_BOOL za_aiot_init_status(ZA_VOID *pvParameter)
{
   (void) pvParameter;

   return tAiotContext.b_IfInited;
}

ZA_BOOL za_aiot_term(ZA_VOID *pvParameter)
{
   (void) pvParameter;

   //za_aiot_msg_data_t t_AiotMsgData;

   //t_AiotMsgData.za_aiot_msg_type = ZA_AIOT_EVENT_TERM_TASK;

   //za_aiot_dealwith(t_AiotMsgData);

   //za_msg_queue_delete(tAiotContext.h_AiotQueueMsg);

   za_semaphore_delete(tAiotContext.h_AiotSemphMutex);
   za_semaphore_delete(tAiotContext.h_AiotSemphMutexCallback);

   za_memset(&tAiotContext, 0, sizeof(za_aiot_context_t));

   return ZA_TRUE;
}

ZA_BOOL za_aiot_connect_server(za_aiot_protocol_type_t tProtocolType)
{
   ZA_BOOL errCode_b = ZA_FALSE;

   if(!tAiotContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited.");

      return ZA_FALSE;
   }

   switch(tProtocolType)
   {
      case ZA_AIOT_MQTT_PROTOCOL:
      {
         za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

         tAiotContext.tProtocolCur = ZA_AIOT_MQTT_PROTOCOL;
         tAiotContext.tLoSubProtocolCur = ZA_AIOT_MQTT_PROTOCOL;

         tAiotContext.tfunc_ComInit = za_aiot_mqtt_init;
         tAiotContext.tfunc_ComTerm = za_aiot_mqtt_term;
         tAiotContext.tfunc_ComReconnect = za_aiot_mqtt_reconnect;
         tAiotContext.tfunc_ComPush = za_aiot_mqtt_push_data;
         tAiotContext.tfunc_ComPushTopData = za_aiot_mqtt_push_topic_data;
         tAiotContext.tfunc_ComPushClientData = za_aiot_mqtt_push_client_data;
         tAiotContext.tfunc_ComPull = za_aiot_mqtt_pull_data;

/*
         tAiotContext.tfunc_LocalSubInit = za_aiot_mqttlocal_init;
         tAiotContext.tfunc_LocalSubTerm = za_aiot_mqttlocal_term;
         tAiotContext.tfunc_LocalSubPush = za_aiot_mqttlocal_push_data;
         tAiotContext.tfunc_LocalSubPushTopData = za_aiot_mqttlocal_push_topic_data;
         tAiotContext.tfunc_ComPull = za_aiot_mqttlocal_pull_data;
*/

/*
         tAiotContext.tfunc_LocalSubInit = za_aiot_udp_init;
         tAiotContext.tfunc_LocalSubTerm = za_aiot_udp_term;
         tAiotContext.tfunc_LocalSubPush = za_aiot_udp_push_data;
*/

         za_semaphore_release(tAiotContext.h_AiotSemphMutex);
      }
      break;

      case ZA_AIOT_MQTTS_PROTOCOL:
      {
         za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

         tAiotContext.tProtocolCur = ZA_AIOT_MQTTS_PROTOCOL;
         tAiotContext.tLoSubProtocolCur = ZA_AIOT_MQTT_PROTOCOL;

         tAiotContext.tfunc_ComInit = za_aiot_mqtts_init;
         tAiotContext.tfunc_ComTerm = za_aiot_mqtts_term;
         tAiotContext.tfunc_ComReconnect = za_aiot_mqtts_reconnect;
         tAiotContext.tfunc_ComPush = za_aiot_mqtts_push_data;
         tAiotContext.tfunc_ComPushTopData = za_aiot_mqtts_push_topic_data;
         tAiotContext.tfunc_ComPushClientData = za_aiot_mqtts_push_client_data;
         tAiotContext.tfunc_ComPull = za_aiot_mqtts_pull_data;

/*
         tAiotContext.tfunc_LocalSubInit = za_aiot_mqttlocal_init;
         tAiotContext.tfunc_LocalSubTerm = za_aiot_mqttlocal_term;
         tAiotContext.tfunc_LocalSubPush = za_aiot_mqttlocal_push_data;
         tAiotContext.tfunc_LocalSubPushTopData = za_aiot_mqttlocal_push_topic_data;
         tAiotContext.tfunc_ComPull = za_aiot_mqttlocal_pull_data;
*/

/*
         tAiotContext.tfunc_LocalSubInit = za_aiot_udp_init;
         tAiotContext.tfunc_LocalSubTerm = za_aiot_udp_term;
         tAiotContext.tfunc_LocalSubPush = za_aiot_udp_push_data;
*/

         za_semaphore_release(tAiotContext.h_AiotSemphMutex);
      }
      break;

      default:
      break;
   }

   if (tAiotContext.tfunc_ComInit != ZA_NULL)
   {
      switch (tAiotContext.tProtocolCur)
      {
         case ZA_AIOT_MQTT_PROTOCOL:
         {
            za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

            errCode_b = tAiotContext.tfunc_ComInit(MQTT_SER_URL);

            za_semaphore_release(tAiotContext.h_AiotSemphMutex);
         }
         break;

         case ZA_AIOT_MQTTS_PROTOCOL:
         {
            za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);
            
            errCode_b = tAiotContext.tfunc_ComInit(MQTTS_SER_URL);

            za_semaphore_release(tAiotContext.h_AiotSemphMutex);
         }
         break;

         default:
         break;
      }

      za_storage_aiot_local_server_t tStoreLoServReadW = za_storage_get_local_server(ZA_NULL);

      za_aiot_local_set_server_url(tStoreLoServReadW.aSetLinkUrl);

      tAiotContext.b_IfConnected = ZA_TRUE;
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot tfunc_ComInit is null.");
   }

   return errCode_b;
}

ZA_BOOL za_aiot_disconnect_server(ZA_VOID *pvParameter)
{
   ZA_BOOL errCode_b = ZA_FALSE;

   if(!tAiotContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited.");

      return ZA_FALSE;
   }

   if (tAiotContext.tfunc_ComTerm != ZA_NULL)
   {
      errCode_b = tAiotContext.tfunc_ComTerm(ZA_NULL);
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot tfunc_ComTerm is null.");
   }

   tAiotContext.b_IfConnected = ZA_FALSE;
   
   tAiotContext.tfunc_ComInit = ZA_NULL;
   tAiotContext.tfunc_ComTerm = ZA_NULL;
   tAiotContext.tfunc_ComPush = ZA_NULL;
   tAiotContext.tfunc_ComPushTopData = ZA_NULL;
   tAiotContext.tfunc_ComPull = ZA_NULL;

   tAiotContext.b_IfLoSubConnected = ZA_FALSE;


   return errCode_b;
}

ZA_BOOL za_aiot_reconnect_server(ZA_VOID *pvParameter)
{
   (void) pvParameter;

   ZA_BOOL errCode_b = ZA_FALSE;

   if(!tAiotContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited.");

      return ZA_FALSE;
   }

   if (tAiotContext.tfunc_ComReconnect != ZA_NULL)
   {
      errCode_b = tAiotContext.tfunc_ComReconnect(ZA_NULL);
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot tfunc_ComTerm is null.");
   }

   return errCode_b;
}

ZA_BOOL za_aiot_connect_set_status(ZA_BOOL bIfConnected)
{
   if(!tAiotContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited.");

      return ZA_FALSE;
   }

   if ((tAiotContext.tfunc_ComInit != ZA_NULL) && (tAiotContext.tfunc_ComTerm != ZA_NULL))
   {
      tAiotContext.b_IfConnected = bIfConnected;
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot tfunc_ComInit or tfunc_ComTerm is null.");

      tAiotContext.b_IfConnected = ZA_FALSE;

      return ZA_FALSE;
   }

   return ZA_TRUE;
}

ZA_BOOL za_aiot_connect_status(ZA_VOID *pvParameter)
{
   (void) pvParameter;

   return tAiotContext.b_IfConnected;
}

ZA_BOOL za_aiot_report(za_aiot_report_type_t tReportType)
{
   //za_aiot_msg_data_t t_AiotMsgData;

   switch(tReportType)
   {
      case ZA_AIOT_REPORT_SYSTEM_INFO_ONE_TIME:
      {
         //t_AiotMsgData.za_aiot_msg_type = ZA_AIOT_EVENT_REPORT;

         //t_AiotMsgData.p_data = (ZA_VOID *)tReportType;
      }
      break;

      default:
      break;
   }

   //za_aiot_dealwith(t_AiotMsgData);

   return ZA_TRUE;
}

ZA_BOOL za_aiot_push_data(za_aiot_com_data_t tAiotComData)
{
   ZA_BOOL errCode_b = ZA_FALSE;

   if(tAiotContext.b_IfInited && tAiotContext.tfunc_ComPush != ZA_NULL)
   {
      za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

      errCode_b = tAiotContext.tfunc_ComPush(tAiotComData);
      
      za_semaphore_release(tAiotContext.h_AiotSemphMutex);
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited or za_aiot_push_data function is null.");

      errCode_b = ZA_FALSE;
   }

   return errCode_b;
}

ZA_BOOL za_aiot_push_topic_data(za_aiot_com_data_t tAiotComData)
{
   ZA_BOOL errCode_b = ZA_FALSE;

   if(tAiotContext.b_IfInited && tAiotContext.tfunc_ComPushTopData != ZA_NULL)
   {
      za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

      errCode_b = tAiotContext.tfunc_ComPushTopData(tAiotComData);
      
      za_semaphore_release(tAiotContext.h_AiotSemphMutex);
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited or za_aiot_push_topic_data function is null.");

      errCode_b = ZA_FALSE;
   }

   return errCode_b;
}

ZA_BOOL za_aiot_push_client_data(const ZA_CHAR * sClientIdata, za_aiot_com_data_t tAiotComData)
{
   ZA_BOOL errCode_b = ZA_FALSE;

   if(tAiotContext.b_IfInited && tAiotContext.tfunc_ComPushClientData != ZA_NULL)
   {
      za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

      errCode_b = tAiotContext.tfunc_ComPushClientData(sClientIdata, tAiotComData);
      
      za_semaphore_release(tAiotContext.h_AiotSemphMutex);
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited or za_aiot_push_client_data function is null.");

      errCode_b = ZA_FALSE;
   }

   return errCode_b;
}

ZA_BOOL za_aiot_pull_data(za_aiot_com_data_t tAiotComData)
{
   ZA_BOOL errCode_b = ZA_FALSE;

   if (tAiotContext.b_IfInited && tAiotContext.tfunc_ComPull != ZA_NULL)
   {
      za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

      errCode_b = tAiotContext.tfunc_ComPull(tAiotComData);

      za_semaphore_release(tAiotContext.h_AiotSemphMutex);
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited or za_aiot_pull_data function is null.");

      return ZA_FALSE;
   }

   return errCode_b;
}

ZA_BOOL za_aiot_refuse_data(za_aiot_com_data_t tAiotComData)
{
   ZA_BOOL errCode_b = ZA_FALSE;

   if (tAiotContext.b_IfInited && tAiotContext.tfunc_ComRefuse != ZA_NULL)
   {
      za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

      errCode_b = tAiotContext.tfunc_ComRefuse(tAiotComData);

      za_semaphore_release(tAiotContext.h_AiotSemphMutex);
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited or za_aiot_refuse_data function is null.");

      return ZA_FALSE;
   }

   return errCode_b;
}

/*
ZA_BOOL za_aiot_dealwith(za_aiot_msg_data_t tAiotMsgData)
{
   if (tAiotContext.h_AiotQueueMsg == NULL)
   {
      ZA_ESP_LOGE(TAG, "The Handle: tAiotContext.h_AiotQueueMsg is null .");

      return ZA_FALSE;
   }

   if(ZA_ESP_PASS != za_msg_queue_send(tAiotContext.h_AiotQueueMsg, &tAiotMsgData, portMAX_DELAY))
   {
      ZA_ESP_LOGE(TAG,"SEND msg : %d failure", tAiotMsgData.za_aiot_msg_type);

      return ZA_FALSE;
   }

   return ZA_TRUE;
}
*/

ZA_VOID za_aiot_callback_data(za_aiot_com_type_t tAiotComType, ZA_CHAR *pData, ZA_INT32 nLength)
{
   if (tAiotContext.b_IfInited && tAiotContext.tfunc_CallBack[tAiotComType] != ZA_NULL)
   {
      za_semaphore_request(tAiotContext.h_AiotSemphMutexCallback, ZA_portMAX_DELAY);

      //tAiotContext.tfunc_CallBack[tAiotComType](tAiotComType, pData, nLength);

      if(! tAiotCurDataBuf.bIfBufing)
      {
        tAiotCurDataBuf.tAiotComType = (za_aiot_com_type_t)tAiotComType;
        za_memcpy(tAiotCurDataBuf.aDatabuf, pData, nLength);
        tAiotCurDataBuf.nDataLength = nLength;
        tAiotCurDataBuf.bIfBufing = ZA_TRUE;
      }

      za_semaphore_release(tAiotContext.h_AiotSemphMutexCallback);
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited or za_aiot_callback_data function is null.");
   }

   return;
}

ZA_BOOL za_aiot_regist_callback(za_aiot_com_type_t callType, za_aiot_data_callback Callback)
{
   za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

   tAiotContext.tfunc_CallBack[callType] = Callback;

   za_semaphore_release(tAiotContext.h_AiotSemphMutex);

   return ZA_TRUE;
}


/**local sub com func*/

ZA_BOOL za_aiot_local_set_server_params(const ZA_CHAR * sMqttLocalServUrl, const ZA_CHAR * sUserName, const ZA_CHAR * sPassWord)
{
   ZA_BOOL errCode_b = ZA_FALSE;

   za_storage_aiot_local_server_t tStoreLoServReadW = za_storage_get_local_server(ZA_NULL);

   za_strcpy((ZA_CHAR *)(tStoreLoServReadW.aSetLinkUrl), (const ZA_CHAR *)sMqttLocalServUrl);
   za_strcpy((ZA_CHAR *)(tStoreLoServReadW.aSetUName), (const ZA_CHAR *)sUserName);
   za_strcpy((ZA_CHAR *)(tStoreLoServReadW.aSetUPassWD), (const ZA_CHAR *)sPassWord);

	tStoreLoServReadW.tLocalServMode = ZA_LOCALSERV_IP_MODE_CERTAIN;

	errCode_b = za_storage_set_local_server(tStoreLoServReadW);

   return errCode_b;
}

ZA_BOOL za_aiot_local_connect_server(const ZA_CHAR * sMqttLocalServUrl, const ZA_CHAR * sUserName, const ZA_CHAR * sPassWord)
{
   ZA_BOOL errCode_b = ZA_FALSE;

   if(!tAiotContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited.");

      return ZA_FALSE;
   }

   if (tAiotContext.tfunc_LocalSubInit != ZA_NULL)
   {
      za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

      errCode_b = tAiotContext.tfunc_LocalSubInit(sMqttLocalServUrl, sUserName, sPassWord);

      za_semaphore_release(tAiotContext.h_AiotSemphMutex);

      //tAiotContext.b_IfLoSubConnected = ZA_TRUE;

      tAiotLocalCurStatus.bIfInited = ZA_TRUE;

      za_strcpy((ZA_CHAR *)(tAiotLocalCurStatus.aCurLinkUrl), (const ZA_CHAR *)sMqttLocalServUrl);
      za_strcpy((ZA_CHAR *)(tAiotLocalCurStatus.aCurUName), (const ZA_CHAR *)sUserName);
      za_strcpy((ZA_CHAR *)(tAiotLocalCurStatus.aCurUPassWD), (const ZA_CHAR *)sPassWord);

   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot tfunc_LocalSubInit is null.");
   }

   return errCode_b;
}

ZA_BOOL za_aiot_local_disconnect_server(ZA_VOID *pvParameter)
{
   (void) pvParameter;

   ZA_BOOL errCode_b = ZA_FALSE;

   if(!tAiotContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited.");

      return ZA_FALSE;
   }

   if (tAiotContext.tfunc_LocalSubTerm != ZA_NULL)
   {
      errCode_b = tAiotContext.tfunc_LocalSubTerm(ZA_NULL);

      tAiotLocalCurStatus.bIfInited = ZA_FALSE;
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot tfunc_ComTerm is null.");
   }

   tAiotContext.b_IfLoSubConnected = ZA_FALSE;

   tAiotContext.tfunc_LocalSubInit = ZA_NULL;
   tAiotContext.tfunc_LocalSubTerm = ZA_NULL;
   tAiotContext.tfunc_LocalSubPush = ZA_NULL;
   tAiotContext.tfunc_LocalSubPushTopData = ZA_NULL;
   tAiotContext.tfunc_LocalSubPull = ZA_NULL;


   return errCode_b;
}

ZA_BOOL za_aiot_local_set_status(ZA_BOOL bIfConnected)
{
   if(!tAiotContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited.");

      return ZA_FALSE;
   }

   if ((tAiotContext.tfunc_LocalSubInit != ZA_NULL) && (tAiotContext.tfunc_LocalSubTerm != ZA_NULL))
   {
      tAiotContext.b_IfLoSubConnected = bIfConnected;
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot tfunc_LocalSubInit or tfunc_ComTerm is null.");

      tAiotContext.b_IfLoSubConnected = ZA_FALSE;

      return ZA_FALSE;
   }

   return ZA_TRUE;
}

ZA_BOOL za_aiot_local_get_status(ZA_VOID *pvParameter)
{
   (void) pvParameter;
   
   return tAiotContext.b_IfLoSubConnected;
}

ZA_BOOL za_aiot_local_set_server_url(const ZA_CHAR * const pcSrcParams)
{
   if (pcSrcParams != NULL)
   {
      za_strcpy((ZA_CHAR *)(tAiotLocalCurStatus.aSetLinkUrl), (const ZA_CHAR *)pcSrcParams);
   }
   else
   {
      return ZA_FALSE;
   }

   return ZA_TRUE;
}

ZA_CHAR * za_aiot_local_get_server_url(ZA_VOID *pvParameter)
{
   (ZA_VOID) pvParameter;

   return tAiotLocalCurStatus.aSetLinkUrl;
}

ZA_BOOL za_aiot_local_set_server_uname(const ZA_CHAR * const pcSrcParams)
{
   if (pcSrcParams != NULL)
   {
      za_strcpy((ZA_CHAR *)(tAiotLocalCurStatus.aSetUName), (const ZA_CHAR *)pcSrcParams);
   }
   else
   {
      return ZA_FALSE;
   }

   return ZA_TRUE;
}

ZA_CHAR * za_aiot_local_get_server_uname(ZA_VOID *pvParameter)
{
   (ZA_VOID) pvParameter;

   return tAiotLocalCurStatus.aSetUName;
}

ZA_BOOL za_aiot_local_set_server_upasswd(const ZA_CHAR * const pcSrcParams)
{
   if (pcSrcParams != NULL)
   {
      za_strcpy((ZA_CHAR *)(tAiotLocalCurStatus.aSetUPassWD), (const ZA_CHAR *)pcSrcParams);
   }
   else
   {
      return ZA_FALSE;
   }

   return ZA_TRUE;
}

ZA_CHAR * za_aiot_local_get_server_upasswd(ZA_VOID *pvParameter)
{
   (ZA_VOID) pvParameter;

   return tAiotLocalCurStatus.aSetUPassWD;
}

ZA_BOOL za_aiot_local_set_server_ipv4(const ZA_CHAR * const pcSrcParams)
{
   if (pcSrcParams != NULL)
   {
      za_strcpy((ZA_CHAR *)(tAiotLocalCurStatus.aSetServIPv4), (const ZA_CHAR *)pcSrcParams);
   }
   else
   {
      return ZA_FALSE;
   }

   return ZA_TRUE;
}

ZA_CHAR * za_aiot_local_get_server_ipv4(ZA_VOID *pvParameter)
{
   (ZA_VOID) pvParameter;

   return tAiotLocalCurStatus.aSetServIPv4;
}

ZA_BOOL za_aiot_local_set_server_port(ZA_UINT16 nServPort)
{
   tAiotLocalCurStatus.nSetServPort = nServPort;

   return ZA_TRUE;
}

ZA_UINT16 za_aiot_local_get_server_port(ZA_VOID *pvParameter)
{
   (ZA_VOID) pvParameter;

   return tAiotLocalCurStatus.nSetServPort;
}

ZA_BOOL za_aiot_local_push_data(za_aiot_com_data_t tAiotComData)
{
   ZA_BOOL errCode_b = ZA_FALSE;

   if(tAiotContext.b_IfInited && tAiotContext.tfunc_LocalSubPush != ZA_NULL)
   {
      za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

      errCode_b = tAiotContext.tfunc_LocalSubPush(tAiotComData);
      
      za_semaphore_release(tAiotContext.h_AiotSemphMutex);
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za aiot is not inited or za_aiot_localpush_data function is null.");

      errCode_b = ZA_FALSE;
   }

   return errCode_b;
}


/** Private **/


void _za_aiot_task(void *arg)
{
   (void)arg;

   //za_aiot_msg_data_t za_aiot_msg_data;

   //za_memset(&za_aiot_msg_data, 0x00, za_sizeof(za_aiot_msg_data_t));

   ZA_BOOL b_aiot_task_runflag = ZA_TRUE;

   while (b_aiot_task_runflag)
   {
      //za_memset(&za_aiot_msg_data, 0x00, za_sizeof(za_aiot_msg_data_t));

      /*
      if (ZA_ESP_PASS != za_msg_queue_recv(tAiotContext.h_AiotQueueMsg, &za_aiot_msg_data, ZA_portMAX_DELAY))
      {
         ZA_ESP_LOGE(TAG, "za_msg_queue_recv : tAiotContext.h_AiotQueueMsg.");

         continue;
      }

      switch(za_aiot_msg_data.za_aiot_msg_type)
      {
         case ZA_AIOT_EVENT_REPORT:
         {
            
         }
         break;

         case ZA_AIOT_EVENT_CONNECTED:
         {

         }
         break;

         case ZA_AIOT_EVENT_DISCONNECTED:
         {

         }
         break;

         case ZA_AIOT_EVENT_SET_CHECKDATA:
         {

         }
         break;

         case ZA_AIOT_EVENT_CHECKDATA:
         {

         }
         break;

         default:
            break;
      }
      */

      if (tAiotCurDataBuf.bIfBufing)
      {
         //za_semaphore_request(tAiotContext.h_AiotSemphMutex, ZA_portMAX_DELAY);

         tAiotContext.tfunc_CallBack[tAiotCurDataBuf.tAiotComType](tAiotCurDataBuf.tAiotComType, tAiotCurDataBuf.aDatabuf, tAiotCurDataBuf.nDataLength);

         //za_semaphore_release(tAiotContext.h_AiotSemphMutex);

         za_semaphore_request(tAiotContext.h_AiotSemphMutexCallback, ZA_portMAX_DELAY);

         za_memset(&tAiotCurDataBuf, 0, za_sizeof(za_aiot_com_databuf_t));
         tAiotCurDataBuf.bIfBufing = ZA_FALSE;

         za_semaphore_release(tAiotContext.h_AiotSemphMutexCallback);         

      } 

      ZA_vTaskDelay(20 / ZA_portTICK_PERIOD_MS); // 1ms for switch
   }

   ZA_ESP_LOGI(TAG, "ZA AIOT TASK Delete. ");

   vTaskDelete(NULL);

   return;
}


