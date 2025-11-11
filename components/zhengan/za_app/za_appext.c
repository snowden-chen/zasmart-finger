/*   Application of zhengan framework to deal all businesses. */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"

#include "zhengan.h"
//#include "longmore.h"

#include "za_appext.h"
#include "za_aiot.h"
#include "za_json.h"
#include "za_storage.h"
#include "za_system.h"

static const char *TAG = "ZA_APPEXT";

typedef struct
{
   ZA_BOOL b_IfInited;
   ZA_SemaphoreHandle_t h_AppExtSemphMutex;
} za_app_ext_context_t;

static za_app_ext_context_t tAppExtContext;

//static ZA_CHAR SendBuf[500] = {0};
static ZA_CHAR SendBuf[300] = {0};

static za_aiot_com_data_t tAiotComData;

static ZA_CHAR strBindFlagID[50] = {0};

/** Private **/

ZA_VOID za_appexit_sysinfo_callback(za_aiot_com_type_t tAiotComType, ZA_CHAR *pData, ZA_INT32 nLength)
{
   if (tAiotComType == ZA_AIOT_COM_UPDATE_SYSINFO)
   {
      ZA_ESP_LOGI(TAG, "za_appexit_sysinfo_callback debug --> data: %.*s , datalen: %d", nLength, pData, nLength);
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za_appexit_sysinfo_callback Error.");
   }

   return;
}

ZA_VOID za_appexit_mainservice_callback(za_aiot_com_type_t tAiotComType, ZA_CHAR *pData, ZA_INT32 nLength)
{
   ZA_CHAR aDataBuf[ZA_APPEXT_MAX_DATA_LEN] = {0};

   za_sys_msg_data_t za_sysmsg_data;
   za_app_msg_data_t za_appmsg_data;

   if (nLength >= ZA_APPEXT_MAX_DATA_LEN)
   {
      return;
   }

   if (tAiotComType == ZA_AIOT_COM_MAIN_SERVICE)
   {
      ZA_ESP_LOGI(TAG, "za_appexit_mainservice_callback debug --> data: %.*s , datalen: %d", nLength, pData, nLength);

      za_memcpy(aDataBuf,pData,nLength);
                          
      if (za_json_start_to_unpack((const ZA_CHAR *)aDataBuf))
      {
         if (za_app_check_bind_flag_id(za_json_unpack_bind_flag_id(ZA_NULL)))
         {
            if (za_json_unpack_mainservice_type(ZA_NULL) == ZA_AIOT_MAINSERVICE_DOWNLOAD)
            {
               za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_CMD_DO_IT_NOW;

               za_memset(za_sysmsg_data.aDataBuffer,0,ZA_SYS_DATA_BUF_LEN);

               za_strcpy((ZA_CHAR *)(za_sysmsg_data.aDataBuffer), za_json_unpack_mainservice_download_cmd(ZA_NULL));

               if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
               {
                  ZA_ESP_LOGE(TAG,"SEND za_sysmsg_data : %d failure",za_sysmsg_data.za_sysmsg_type);
               }
               
            }
            else if(za_json_unpack_mainservice_type(ZA_NULL) == ZA_AIOT_MAINSERVICE_UPLOAD)
            {
               za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_AIOT_REPORT_SYSINFO;

               if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(ZA_NULL), &za_appmsg_data, portMAX_DELAY))
               {
                  ZA_ESP_LOGE(TAG,"SEND za_appmsg_data : %d failure",za_appmsg_data.za_appmsg_type);
               }
            }
         }

      }

      za_json_end_to_unpack(ZA_NULL);
   }
   else
   {
      ZA_ESP_LOGE(TAG, "za_appexit_sysinfo_callback Error.");
   }

   return;
}

ZA_VOID za_appexit_bindservice_callback(za_aiot_com_type_t tAiotComType, ZA_CHAR *pData, ZA_INT32 nLength)
{
   ZA_CHAR aDataBuf[ZA_APPEXT_NORMAL_DATA_LEN] = {0};

   za_sys_msg_data_t za_sysmsg_data;

   if (nLength >= ZA_APPEXT_NORMAL_DATA_LEN)
   {
      return;
   }

   if (tAiotComType == ZA_AIOT_COM_BIND_SERVICE)
   {
      ZA_ESP_LOGI(TAG, "za_appexit_bindservice_callback debug --> data: %.*s , datalen: %d", nLength, pData, nLength);

      za_memcpy(aDataBuf,pData,nLength);
                          
      za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_CMD_DO_IT_NOW;

      za_memset(za_sysmsg_data.aDataBuffer,0,ZA_SYS_DATA_BUF_LEN);

      za_strcpy((ZA_CHAR *)(za_sysmsg_data.aDataBuffer), "CaidengI Y white");

      if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
      {
         ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_sysmsg_data.za_sysmsg_type);
      }

      if (za_json_start_to_unpack((const ZA_CHAR *)aDataBuf))
      {
         za_memset(za_sysmsg_data.aDataBuffer,0,ZA_SYS_DATA_BUF_LEN);
         za_strcpy((ZA_CHAR *)(za_sysmsg_data.aDataBuffer), za_json_unpack_bind_flag_id(ZA_NULL));

         za_aiot_local_set_server_url(za_json_unpack_localserver_ip(ZANULL));
         za_aiot_local_set_server_uname("zdmedia");
         za_aiot_local_set_server_upasswd("123456");

         za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_UPDATE_BIND_FLAG_ID;

         if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
         {
            ZA_ESP_LOGE(TAG,"SEND za_sysmsg_data : %d failure",za_sysmsg_data.za_sysmsg_type);
         }

         za_json_end_to_unpack(ZA_NULL);
      }

      /* may be other status display or be drv go to work...  */

   }


   return;
}




/** Public **/

ZA_BOOL za_app_ext_init()
{
   ZA_ESP_LOGI(TAG, "ZA APP ENTER");

   za_memset(&tAppExtContext, 0, sizeof(za_app_ext_context_t));

   tAppExtContext.h_AppExtSemphMutex = za_semaphore_create(1,1);

   tAppExtContext.b_IfInited = ZA_TRUE;

   return ZA_TRUE;
}

ZA_BOOL za_app_send_sysinfo_to_serv()
{
   ZA_CHAR *strJson = ZA_NULL;
   ZA_UINT32 nStrLength = ZA_NULL;
   ZA_VOID *pStrData = ZA_NULL;

   ZA_BOOL bIfSuccess = ZA_FALSE;

   if(! tAppExtContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za app is not inited.");

      return ZA_FALSE;
   }

   if (! za_aiot_connect_status(ZA_NULL))
   {
      ZA_ESP_LOGE(TAG, "za aiot is not connected.");

      return ZA_FALSE;
   }

   za_semaphore_request(tAppExtContext.h_AppExtSemphMutex, ZA_portMAX_DELAY);

   za_memset(&tAiotComData, ZA_NULL, za_sizeof(za_aiot_com_data_t));

   za_json_start_to_pack(ZA_NULL);
   za_json_pack_system_info(NULL);

   strJson = za_json_pack_unformat(NULL);

   //nStrLength = strlen(strJson);
   //pStrData = strJson;

   pStrData = SendBuf;

   nStrLength = strlen(strJson)+1; // ['\0']
   //pStrData = za_malloc(nStrLength);
   
   za_memset(pStrData,ZA_NULL, nStrLength);
   za_memcpy(pStrData, (ZA_VOID *)strJson, strlen(strJson));

   tAiotComData.tZaAiotComType = ZA_AIOT_COM_UPDATE_SYSINFO;
   tAiotComData.p_data = pStrData;
   tAiotComData.n_length = nStrLength;
   tAiotComData.tZaComlevel = ZA_AIOT_COMLEVEL_L1;

   za_json_end_to_pack(NULL);

   za_semaphore_release(tAppExtContext.h_AppExtSemphMutex);

   bIfSuccess = za_aiot_push_data(tAiotComData);

   //ZA_vTaskDelay(5 / ZA_portTICK_PERIOD_MS); // 15ms for test

   //if (tAiotComData.p_data != ZA_NULL)
   //{
      //za_free(tAiotComData.p_data);
   //}

   //za_free((ZA_VOID *)pStrData);



   //bIfSuccess = za_aiot_push_data(tAiotComData);

   return bIfSuccess;
}

ZA_BOOL za_app_send_oid_event_to_serv(ZA_CHAR *pData)
{
   ZA_CHAR *strJson = ZA_NULL;
   ZA_UINT32 nStrLength = ZA_NULL;
   ZA_VOID *pStrData = ZA_NULL;

   ZA_BOOL bIfSuccess = ZA_FALSE;

   if(! tAppExtContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za app is not inited.");

      return ZA_FALSE;
   }

   if (! za_aiot_connect_status(ZA_NULL))
   {
      ZA_ESP_LOGE(TAG, "za aiot is not connected.");

      return ZA_FALSE;
   }

   za_semaphore_request(tAppExtContext.h_AppExtSemphMutex, ZA_portMAX_DELAY);

   za_memset(&tAiotComData, ZA_NULL, za_sizeof(za_aiot_com_data_t));

   za_json_start_to_pack(ZA_NULL);
   za_json_pack_oid_event(pData);

   strJson = za_json_pack_unformat(NULL);

   pStrData = SendBuf;

   nStrLength = strlen(strJson)+1; // ['\0']

   za_memset(pStrData,ZA_NULL, nStrLength);
   za_memcpy(pStrData, (ZA_VOID *)strJson, strlen(strJson));

   tAiotComData.tZaAiotComType = ZA_AIOT_COM_UPDATE_EVENT;
   tAiotComData.p_data = pStrData;
   tAiotComData.n_length = nStrLength;
   tAiotComData.tZaComlevel = ZA_AIOT_COMLEVEL_L1;

   za_json_end_to_pack(NULL);

   za_semaphore_release(tAppExtContext.h_AppExtSemphMutex);

   bIfSuccess = za_aiot_push_data(tAiotComData);

/*
   if (! za_aiot_local_get_status(ZANULL))
   {
      //bIfSuccess = za_aiot_push_data(tAiotComData);
   }
   else
   {
      bIfSuccess = za_aiot_local_push_data(tAiotComData);
   }
*/
   return bIfSuccess;
}

ZA_BOOL za_app_require_from_serv()
{
   if(! tAppExtContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za app is not inited.");

      return ZA_FALSE;
   }

   za_semaphore_request(tAppExtContext.h_AppExtSemphMutex, ZA_portMAX_DELAY);

   za_memset(&tAiotComData, ZA_NULL, za_sizeof(za_aiot_com_data_t));

   tAiotComData.tZaAiotComType = ZA_AIOT_COM_UPDATE_SYSINFO;

   tAiotComData.tZaComlevel = ZA_AIOT_COMLEVEL_L1;

   za_aiot_regist_callback(ZA_AIOT_COM_UPDATE_SYSINFO, za_appexit_sysinfo_callback);

   za_semaphore_release(tAppExtContext.h_AppExtSemphMutex);

   return za_aiot_pull_data(tAiotComData);
}

ZA_BOOL za_app_require_mainservice_from_serv()
{
   if(! tAppExtContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za app is not inited.");

      return ZA_FALSE;
   }

   if (! za_aiot_connect_status(ZA_NULL))
   {
      ZA_ESP_LOGE(TAG, "za aiot is not connected.");

      return ZA_FALSE;
   }

   za_semaphore_request(tAppExtContext.h_AppExtSemphMutex, ZA_portMAX_DELAY);

   za_memset(&tAiotComData, ZA_NULL, za_sizeof(za_aiot_com_data_t));

   tAiotComData.tZaAiotComType = ZA_AIOT_COM_MAIN_SERVICE;

   tAiotComData.tZaComlevel = ZA_AIOT_COMLEVEL_L1;

   za_aiot_regist_callback(tAiotComData.tZaAiotComType, za_appexit_mainservice_callback);

   za_semaphore_release(tAppExtContext.h_AppExtSemphMutex);

   return za_aiot_pull_data(tAiotComData);
}

ZA_BOOL za_app_require_bindresult_from_serv()
{
   if(! tAppExtContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za app is not inited.");

      return ZA_FALSE;
   }

   if (! za_aiot_connect_status(ZA_NULL))
   {
      ZA_ESP_LOGE(TAG, "za aiot is not connected.");

      return ZA_FALSE;
   }

   za_semaphore_request(tAppExtContext.h_AppExtSemphMutex, ZA_portMAX_DELAY);

   tAiotComData.tZaAiotComType = ZA_AIOT_COM_BIND_SERVICE;

   tAiotComData.tZaComlevel = ZA_AIOT_COMLEVEL_L1;

   za_aiot_regist_callback(tAiotComData.tZaAiotComType, za_appexit_bindservice_callback);

   za_semaphore_release(tAppExtContext.h_AppExtSemphMutex);

   return za_aiot_pull_data(tAiotComData);

}

ZA_BOOL za_app_send_bind_code_to_serv(const ZA_CHAR *strBindCode)
{
   ZA_VOID *pStrData = ZA_NULL;
   ZA_BOOL bIfSuccess = ZA_FALSE;
   ZA_UINT32 nStrLength = ZA_NULL;

   pStrData = SendBuf;

   if(! tAppExtContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za app is not inited.");

      return ZA_FALSE;
   }

   if (! za_aiot_connect_status(ZA_NULL))
   {
      ZA_ESP_LOGE(TAG, "za aiot is not connected.");

      return ZA_FALSE;
   }

   if (strBindCode == NULL)
   {
      ZA_ESP_LOGE(TAG, "za_app_send_bind_code_to_serv data is not valid.");

      return ZA_FALSE;
   }

   za_semaphore_request(tAppExtContext.h_AppExtSemphMutex, ZA_portMAX_DELAY);

   za_memset(&tAiotComData, ZA_NULL, za_sizeof(za_aiot_com_data_t));

   nStrLength = strlen(strBindCode) + 1;
   za_memset(pStrData,ZA_NULL, nStrLength);
   za_strcpy(pStrData, strBindCode);

   tAiotComData.tZaAiotComType = ZA_AIOT_COM_UPDATA_BINDCODE;
   tAiotComData.p_data = pStrData;
   tAiotComData.n_length = nStrLength;
   tAiotComData.tZaComlevel = ZA_AIOT_COMLEVEL_L1;

   za_semaphore_release(tAppExtContext.h_AppExtSemphMutex);

   bIfSuccess = za_aiot_push_topic_data(tAiotComData);

   return bIfSuccess;
}

ZA_BOOL za_app_memory_load_bind_flag_id(const ZA_CHAR *strBindFlagIdNew)
{
   za_semaphore_request(tAppExtContext.h_AppExtSemphMutex, ZA_portMAX_DELAY);

   if (strBindFlagIdNew != ZA_NULL)
   {
      za_strcpy((ZA_CHAR *)(strBindFlagID), strBindFlagIdNew);

      //ZA_ESP_LOGI(TAG, "debug : za_app_memory_load_bind_flag_id --> %s.", strBindFlagID);
   }

   za_semaphore_release(tAppExtContext.h_AppExtSemphMutex);

   return ZA_TRUE;
}

ZA_BOOL za_app_check_bind_flag_id(const ZA_CHAR *strBindFlagIdRev)
{
   ZA_BOOL bIfSuccess = ZA_FALSE;

   if (strBindFlagIdRev != ZA_NULL)
   {
      za_semaphore_request(tAppExtContext.h_AppExtSemphMutex, ZA_portMAX_DELAY);

      if (za_strcmp(strBindFlagID, strBindFlagIdRev) == 0)
      {
         bIfSuccess = ZA_TRUE;
      }

      za_semaphore_release(tAppExtContext.h_AppExtSemphMutex);
   }

   return bIfSuccess;
}

ZA_CHAR * za_app_memory_read_bind_flag_id(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   return strBindFlagID;
}


ZA_CHAR * za_app_udp_package(ZA_CHAR *pTopicName, ZA_CHAR  *pPayloadInput, ZA_UINT32 *pLengthOut)
{
   ZA_CHAR *strJson = ZA_NULL;
   ZA_UINT32 nStrLength = ZA_NULL;
   ZA_VOID *pStrData = ZA_NULL;

   if (! za_aiot_local_get_status(ZA_NULL))
   {
      ZA_ESP_LOGE(TAG, "za aiot is not connected.");

      return ZA_NULL;
   }

   za_semaphore_request(tAppExtContext.h_AppExtSemphMutex, ZA_portMAX_DELAY);

   za_json_start_to_pack(ZA_NULL);
   za_json_pack_udp_data_event(pTopicName, pPayloadInput);

   strJson = za_json_pack_unformat(NULL);

   pStrData = SendBuf;

   nStrLength = strlen(strJson)+1; // ['\0']

   za_memset(pStrData,ZA_NULL, nStrLength);
   za_memcpy(pStrData, (ZA_VOID *)strJson, strlen(strJson));

   za_json_end_to_pack(NULL);

   za_semaphore_release(tAppExtContext.h_AppExtSemphMutex);

   *pLengthOut = nStrLength;

   return pStrData;
}

ZA_BOOL za_app_send_main_message_to_serv(ZA_CHAR  * strTag, ZA_CHAR *pData)
{
   ZA_CHAR *strJson = ZA_NULL;
   ZA_UINT32 nStrLength = ZA_NULL;
   ZA_VOID *pStrData = ZA_NULL;

   ZA_BOOL bIfSuccess = ZA_FALSE;

   if(! tAppExtContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za app is not inited.");

      return ZA_FALSE;
   }

   if (! za_aiot_connect_status(ZA_NULL))
   {
      ZA_ESP_LOGE(TAG, "za aiot is not connected.");

      return ZA_FALSE;
   }

   za_semaphore_request(tAppExtContext.h_AppExtSemphMutex, ZA_portMAX_DELAY);

   za_memset(&tAiotComData, ZA_NULL, za_sizeof(za_aiot_com_data_t));

   pStrData = SendBuf;

   if (za_system_game_msg_ctrlmode_get(ZANULL) == ZA_STORAGE_GAME_MSG_CTRLMODE_JSON)
   {
      za_json_start_to_pack(ZA_NULL);

      za_json_pack_pushmain_message(strTag, pData);

      strJson = za_json_pack_unformat(NULL);

      nStrLength = strlen(strJson)+1; // ['\0']

      za_memset(pStrData,ZA_NULL, nStrLength);
      za_memcpy(pStrData, (ZA_VOID *)strJson, strlen(strJson));

      tAiotComData.tZaAiotComType = ZA_AIOT_COM_MAIN_SERVICE_MESSAGE;
      tAiotComData.p_data = pStrData;
      tAiotComData.n_length = nStrLength;
      tAiotComData.tZaComlevel = ZA_AIOT_COMLEVEL_L1;

      za_json_end_to_pack(NULL);
   }
   else if (za_system_game_msg_ctrlmode_get(ZANULL) == ZA_STORAGE_GAME_MSG_CTRLMODE_SIMPLE)
   {
      nStrLength = strlen(strTag) + strlen(pData) + 2; // [':']['\0']
      za_memset(pStrData,ZA_NULL, nStrLength);
      
      strcpy(pStrData, strTag);
      strcat(pStrData, ":");
      strcat(pStrData, pData);

      tAiotComData.tZaAiotComType = ZA_AIOT_COM_MAIN_SERVICE_MESSAGE;
      tAiotComData.p_data = pStrData;
      tAiotComData.n_length = nStrLength;
      tAiotComData.tZaComlevel = ZA_AIOT_COMLEVEL_L1;
   }

   za_semaphore_release(tAppExtContext.h_AppExtSemphMutex);

   //bIfSuccess = za_aiot_push_data(tAiotComData);
   bIfSuccess = za_aiot_push_client_data(za_app_memory_read_bind_flag_id(ZANULL), tAiotComData);

   return bIfSuccess;
}

ZA_BOOL za_app_send_main_command_to_serv(ZA_CHAR  * strTag, ZA_CHAR *pData)
{
   ZA_CHAR *strJson = ZA_NULL;
   ZA_UINT32 nStrLength = ZA_NULL;
   ZA_VOID *pStrData = ZA_NULL;

   ZA_BOOL bIfSuccess = ZA_FALSE;

   if(! tAppExtContext.b_IfInited)
   {
      ZA_ESP_LOGE(TAG, "za app is not inited.");

      return ZA_FALSE;
   }

   if (! za_aiot_connect_status(ZA_NULL))
   {
      ZA_ESP_LOGE(TAG, "za aiot is not connected.");

      return ZA_FALSE;
   }

   za_semaphore_request(tAppExtContext.h_AppExtSemphMutex, ZA_portMAX_DELAY);

   za_memset(&tAiotComData, ZA_NULL, za_sizeof(za_aiot_com_data_t));

   za_json_start_to_pack(ZA_NULL);
   
   za_json_pack_pushmain_command(strTag, pData);

   strJson = za_json_pack_unformat(NULL);

   pStrData = SendBuf;

   nStrLength = strlen(strJson)+1; // ['\0']

   za_memset(pStrData,ZA_NULL, nStrLength);
   za_memcpy(pStrData, (ZA_VOID *)strJson, strlen(strJson));

   tAiotComData.tZaAiotComType = ZA_AIOT_COM_MAIN_SERVICE;
   tAiotComData.p_data = pStrData;
   tAiotComData.n_length = nStrLength;
   tAiotComData.tZaComlevel = ZA_AIOT_COMLEVEL_L2;

   za_json_end_to_pack(NULL);

   za_semaphore_release(tAppExtContext.h_AppExtSemphMutex);

   //bIfSuccess = za_aiot_push_data(tAiotComData);
   bIfSuccess = za_aiot_push_client_data(za_app_memory_read_bind_flag_id(ZANULL), tAiotComData);

   return bIfSuccess;
}

