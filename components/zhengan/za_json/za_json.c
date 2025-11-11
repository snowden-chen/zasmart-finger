/*  Json process for all components with some certain datas. */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"
#include "zhengan.h"

#include "za_json.h"
#include "za_info.h"
#include "za_aiot.h"
#include "za_storage.h"
#include "za_upgrade.h"
#include "za_system.h"

#include "cJSON.h"


static const char *TAG = "ZA_JSON";

typedef struct
{
   ZA_BOOL b_IfInited;

   //ZA_SemaphoreHandle_t h_PackSemphMutex;
   //ZA_SemaphoreHandle_t h_UnpackSemphMutex;

   cJSON *pPackRoot;
   ZA_CHAR *pPackDataStr;
   cJSON *pUnpackRoot;
   ZA_CHAR *pUnPackDataStr;
} za_json_context_t;

static za_json_context_t tJsonContext;

/** Public **/


ZA_BOOL za_json_init()
{
   ZA_ESP_LOGI(TAG, "ZA JSON ENTER");

   za_memset(&tJsonContext, 0, sizeof(za_json_context_t));

   //tJsonContext.h_PackSemphMutex = za_semaphore_create(1,1);
   //tJsonContext.h_UnpackSemphMutex = za_semaphore_create(1,1);

   tJsonContext.b_IfInited = ZA_TRUE;

   return ZA_TRUE;
}

ZA_BOOL za_json_show_version()
{
   ZA_ESP_LOGI(TAG, "JSON Library Version: %s", cJSON_Version());

   return ZA_TRUE;
}

// --- pack part

ZA_BOOL za_json_start_to_pack(ZA_VOID *pvParameter)
{
   (void) pvParameter;

   if (! tJsonContext.b_IfInited)
   {
      return ZA_FALSE;
   }

   //za_semaphore_request(tJsonContext.h_PackSemphMutex, ZA_portMAX_DELAY);

   tJsonContext.pPackRoot = cJSON_CreateObject();

   //za_semaphore_release(tJsonContext.h_PackSemphMutex);

   return ZA_TRUE;
}

ZA_CHAR * za_json_pack_format(ZA_VOID *pvParameter)
{
   (void) pvParameter;

   if ((! tJsonContext.b_IfInited) || (tJsonContext.pPackRoot == ZA_NULL))
   {
      return ZA_NULL;
   }

   tJsonContext.pPackDataStr = cJSON_Print(tJsonContext.pPackRoot);

   ZA_ESP_LOGI(TAG, "format ---> %s", tJsonContext.pPackDataStr);

   return tJsonContext.pPackDataStr;
}

ZA_CHAR * za_json_pack_unformat(ZA_VOID *pvParameter)
{
   (void) pvParameter;

   if ((! tJsonContext.b_IfInited) || (tJsonContext.pPackRoot == ZA_NULL))
   {
      return ZA_NULL;
   }

   tJsonContext.pPackDataStr = cJSON_PrintUnformatted(tJsonContext.pPackRoot);

   ZA_ESP_LOGI(TAG, "format ---> %s", tJsonContext.pPackDataStr);

   return tJsonContext.pPackDataStr;
}

ZA_BOOL za_json_end_to_pack(ZA_VOID *pvParameter)
{
   (void) pvParameter;

   if ((! tJsonContext.b_IfInited) 
   || (tJsonContext.pPackDataStr == ZA_NULL)
   || (tJsonContext.pPackRoot == ZA_NULL))
   {
      return ZA_FALSE;
   }

   //za_semaphore_request(tJsonContext.h_PackSemphMutex, ZA_portMAX_DELAY);

   cJSON_free(tJsonContext.pPackDataStr);
   cJSON_Delete(tJsonContext.pPackRoot);

   tJsonContext.pPackDataStr = ZA_NULL;
   tJsonContext.pPackRoot = ZA_NULL;

   //za_semaphore_release(tJsonContext.h_PackSemphMutex);

   return ZA_TRUE;
}

ZA_BOOL za_json_pack_system_info(ZA_VOID *pvParameter)
{
   (void) pvParameter;

   cJSON *pPackSysInfo = ZA_NULL;
   cJSON *pPackSysInfo2 = ZA_NULL;
   cJSON *pPackSysInfo3 = ZA_NULL;

   //ZA_UINT32 nArrayNum = 0;

   //ZA_CHAR aMacAddr[20] = {0};

   if ((! tJsonContext.b_IfInited) || (tJsonContext.pPackRoot == ZA_NULL))
   {
      return ZA_FALSE;
   }

   pPackSysInfo = cJSON_CreateObject();

   /*
   cJSON_AddItemToObject(tJsonContext.pPackRoot, "sysinfo", pPackSysInfo);
   cJSON_AddNumberToObject(pPackSysInfo, "free", za_info_get_free_memory(ZA_NULL));

   za_info_get_mac_wifi_sta(aMacAddr, 20);
   cJSON_AddStringToObject(pPackSysInfo, "wifimac", aMacAddr);

   za_info_get_mac_bt(aMacAddr, 20);
   cJSON_AddStringToObject(pPackSysInfo, "btmac", aMacAddr);
   */

   cJSON_AddItemToObject(tJsonContext.pPackRoot, "wifi", pPackSysInfo);

   cJSON_AddStringToObject(pPackSysInfo, "ssid", (const ZA_CHAR *)(za_app_get_current_wifi(ZANULL).sta.ssid));
   cJSON_AddNumberToObject(pPackSysInfo, "quality", 0);

   pPackSysInfo2 = cJSON_CreateObject();

   cJSON_AddItemToObject(tJsonContext.pPackRoot, "sys", pPackSysInfo2);
   //cJSON_AddStringToObject(pPackSysInfo2, "firmver", "0.0.12");
   cJSON_AddStringToObject(pPackSysInfo2, "firmver", za_upgrade_firmware_ver(ZANULL));
   cJSON_AddNumberToObject(pPackSysInfo2, "battery", (const double)(za_app_get_current_battery_percent(ZANULL)));

   za_storage_sys_mode_t tSystemMode = za_storage_get_system_mode(NULL);

   if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_ZHENDONG_SENSOR)
   {
      const char *strings[1] = { "1-shake" };

      pPackSysInfo3 = cJSON_CreateStringArray(strings,1);

      cJSON_AddItemToObject(tJsonContext.pPackRoot, "sub", pPackSysInfo3);
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_RENTIGANYING_SENSOR)
   {
      const char *strings[1] = { "2-infrared" };

      pPackSysInfo3 = cJSON_CreateStringArray(strings,1);

      cJSON_AddItemToObject(tJsonContext.pPackRoot, "sub", pPackSysInfo3);
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_HONGWAIDUIGUAN_SENSOR)
   {
      const char *strings[1] = { "3-irparty" };

      pPackSysInfo3 = cJSON_CreateStringArray(strings,1);

      cJSON_AddItemToObject(tJsonContext.pPackRoot, "sub", pPackSysInfo3);
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_LIUZHOUTUOLUOYI_SENSOR)
   {
      const char *strings[1] = { "4-accgyro" };

      pPackSysInfo3 = cJSON_CreateStringArray(strings,1);

      cJSON_AddItemToObject(tJsonContext.pPackRoot, "sub", pPackSysInfo3);
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_DIANZILUOPAN_SENSOR)
   {
      const char *strings[1] = { "5-compass" };

      pPackSysInfo3 = cJSON_CreateStringArray(strings,1);

      cJSON_AddItemToObject(tJsonContext.pPackRoot, "sub", pPackSysInfo3);
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_SERVO_DRIVER)
   {
      const char *strings[1] = { "6-servo" };

      pPackSysInfo3 = cJSON_CreateStringArray(strings,1);

      cJSON_AddItemToObject(tJsonContext.pPackRoot, "sub", pPackSysInfo3);
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_ROBOT_T1)
   {
      const char *strings[1] = { "7-robott1" };

      pPackSysInfo3 = cJSON_CreateStringArray(strings,1);

      cJSON_AddItemToObject(tJsonContext.pPackRoot, "sub", pPackSysInfo3);
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_MARK_ACTION)
   {
      const char *strings[1] = { "8-action" };

      pPackSysInfo3 = cJSON_CreateStringArray(strings,1);

      cJSON_AddItemToObject(tJsonContext.pPackRoot, "sub", pPackSysInfo3);
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_MARK_DIY)
   {
      const char *strings[1] = { "B0-DIY" };

      pPackSysInfo3 = cJSON_CreateStringArray(strings,1);

      cJSON_AddItemToObject(tJsonContext.pPackRoot, "sub", pPackSysInfo3);
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_MARK_DIY_LIGHT)
   {
      const char *strings[1] = { "B1-DIYLight" };

      pPackSysInfo3 = cJSON_CreateStringArray(strings,1);

      cJSON_AddItemToObject(tJsonContext.pPackRoot, "sub", pPackSysInfo3);
   }

   cJSON_AddStringToObject(tJsonContext.pPackRoot, "bind", tSystemMode.cBindFlagID);

   if (tSystemMode.tAiotServerMode == ZA_STORAGE_AIOT_SERV_INTERNET)
   {
      cJSON_AddStringToObject(tJsonContext.pPackRoot, "link", "0");
   }
   else if (tSystemMode.tAiotServerMode == ZA_STORAGE_AIOT_SERV_LOCALSERV)
   {
      cJSON_AddStringToObject(tJsonContext.pPackRoot, "link", "1");
   }

   cJSON_AddStringToObject(tJsonContext.pPackRoot, "serv", za_aiot_local_get_server_url(ZANULL));

   return ZA_TRUE;
}


ZA_BOOL za_json_pack_oid_event(ZA_CHAR * strCode)
{
   if ((! tJsonContext.b_IfInited) || (tJsonContext.pPackRoot == ZA_NULL))
   {
      return ZA_FALSE;
   }

   cJSON_AddStringToObject(tJsonContext.pPackRoot, "type", "oid");
   cJSON_AddStringToObject(tJsonContext.pPackRoot, "code", (const ZA_CHAR *)strCode);

   return ZA_TRUE;
}

ZA_BOOL za_json_pack_udp_data_event(ZA_CHAR * strCodeTopic, ZA_CHAR  * strCodePayload)
{
   if ((! tJsonContext.b_IfInited) || (tJsonContext.pPackRoot == ZA_NULL))
   {
      return ZA_FALSE;
   }

   cJSON_AddStringToObject(tJsonContext.pPackRoot, "topic", (const ZA_CHAR *)strCodeTopic);
   cJSON_AddStringToObject(tJsonContext.pPackRoot, "payload", (const ZA_CHAR *)strCodePayload);

   return ZA_TRUE;
}

ZA_BOOL za_json_pack_pushmain_message(ZA_CHAR  * strTag, ZA_CHAR * strParams)
{
   ZA_CHAR aMacAddr[30] = {0};
   ZA_CHAR aIdStr[30] = {0};

   za_info_get_mac_wifi_sta(aMacAddr, 20);

   sprintf(aIdStr,"%d",za_system_get_time_now(ZANULL));

   if ((! tJsonContext.b_IfInited) || (tJsonContext.pPackRoot == ZA_NULL))
   {
      return ZA_FALSE;
   }

   cJSON_AddStringToObject(tJsonContext.pPackRoot, "from", (const ZA_CHAR *)aMacAddr);
   cJSON_AddStringToObject(tJsonContext.pPackRoot, "id", (const ZA_CHAR *)aIdStr);
   cJSON_AddStringToObject(tJsonContext.pPackRoot, "tag", (const ZA_CHAR *)strTag);
   cJSON_AddStringToObject(tJsonContext.pPackRoot, "params", (const ZA_CHAR *)strParams);

   return ZA_TRUE;
}

ZA_BOOL za_json_pack_pushmain_command(ZA_CHAR  * strTag, ZA_CHAR * strParams)
{
   ZA_CHAR aMacAddr[30] = {0};
   ZA_CHAR aIdStr[30] = {0};

   za_info_get_mac_wifi_sta(aMacAddr, 20);

   sprintf(aIdStr,"%d",za_system_get_time_now(ZANULL));

   if ((! tJsonContext.b_IfInited) || (tJsonContext.pPackRoot == ZA_NULL))
   {
      return ZA_FALSE;
   }

   cJSON_AddStringToObject(tJsonContext.pPackRoot, "from", (const ZA_CHAR *)aMacAddr);
   cJSON_AddStringToObject(tJsonContext.pPackRoot, "id", (const ZA_CHAR *)aIdStr);
   cJSON_AddStringToObject(tJsonContext.pPackRoot, "tag", (const ZA_CHAR *)strTag);
   cJSON_AddStringToObject(tJsonContext.pPackRoot, "params", (const ZA_CHAR *)strParams);

   return ZA_TRUE;
}

// unpack part

ZA_BOOL za_json_start_to_unpack(const ZA_CHAR *UnpackStr)
{
   if ((UnpackStr == ZA_NULL) || (! tJsonContext.b_IfInited))
   {
      return ZA_FALSE;
   }

   //za_semaphore_request(tJsonContext.h_UnpackSemphMutex, ZA_portMAX_DELAY);

   tJsonContext.pUnpackRoot = cJSON_Parse(UnpackStr);
   if (tJsonContext.pUnpackRoot == ZA_NULL)
   {
      //za_semaphore_release(tJsonContext.h_UnpackSemphMutex);

      return ZA_FALSE;
   }

   //za_semaphore_release(tJsonContext.h_UnpackSemphMutex);

   return ZA_TRUE;
}

ZA_BOOL za_json_end_to_unpack(ZA_VOID *pvParameter)
{
   (ZA_VOID) pvParameter;

   if ((! tJsonContext.b_IfInited) || (tJsonContext.pUnpackRoot == ZA_NULL))
   {
      return ZA_FALSE;
   }

   //za_semaphore_request(tJsonContext.h_UnpackSemphMutex, ZA_portMAX_DELAY);

   cJSON_Delete(tJsonContext.pUnpackRoot);

   //za_semaphore_release(tJsonContext.h_UnpackSemphMutex);

   return ZA_TRUE;
}

za_aiot_mainservice_type_t za_json_unpack_mainservice_type(ZA_VOID *pvParameter)
{
   //ZA_CHAR aTag[20] = {0};

   za_aiot_mainservice_type_t tTypeRet = ZA_AIOT_MAINSERVICE_NULL;

   (ZA_VOID) pvParameter;

   if ((! tJsonContext.b_IfInited) || (tJsonContext.pUnpackRoot == ZA_NULL))
   {
      return tTypeRet;
   }

   //za_semaphore_request(tJsonContext.h_UnpackSemphMutex, ZA_portMAX_DELAY);

   cJSON *pTagCmd = cJSON_GetObjectItem(tJsonContext.pUnpackRoot, "tag");

   if (pTagCmd != ZA_NULL)
   {
      if (cJSON_IsString(pTagCmd))
      {
         if (! za_strcmp(pTagCmd->valuestring, "upload"))
         {
            tTypeRet = ZA_AIOT_MAINSERVICE_UPLOAD;
         }
         else if (! za_strcmp(pTagCmd->valuestring, "download"))
         {
            tTypeRet = ZA_AIOT_MAINSERVICE_DOWNLOAD;
         }
         else if (! za_strcmp(pTagCmd->valuestring, "upgrade"))
         {
            tTypeRet = ZA_AIOT_MAINSERVICE_UPGRADE;
         }
         else
         {
            tTypeRet = ZA_AIOT_MAINSERVICE_END;
         }
      }
   }

   //za_semaphore_release(tJsonContext.h_UnpackSemphMutex);

   return tTypeRet;
}

ZA_CHAR * za_json_unpack_mainservice_download_cmd(ZA_VOID *pvParameter)
{
   (ZA_VOID) pvParameter;

   tJsonContext.pUnPackDataStr = ZA_NULL;

   if ((! tJsonContext.b_IfInited) || (tJsonContext.pUnpackRoot == ZA_NULL))
   {
      return tJsonContext.pUnPackDataStr;
   }

   ///za_semaphore_request(tJsonContext.h_UnpackSemphMutex, ZA_portMAX_DELAY);

   cJSON *pTagCmd = cJSON_GetObjectItem(tJsonContext.pUnpackRoot, "params");

   if (pTagCmd != ZA_NULL)
   {
      if (cJSON_IsString(pTagCmd))
      {
         tJsonContext.pUnPackDataStr = pTagCmd->valuestring;
      }
   }

   //za_semaphore_release(tJsonContext.h_UnpackSemphMutex);

   return tJsonContext.pUnPackDataStr;
}

ZA_CHAR * za_json_unpack_bind_flag_id(ZA_VOID *pvParameter)
{
   (ZA_VOID) pvParameter;

   tJsonContext.pUnPackDataStr = ZA_NULL;

   if ((! tJsonContext.b_IfInited) || (tJsonContext.pUnpackRoot == ZA_NULL))
   {
      return tJsonContext.pUnPackDataStr;
   }

   //za_semaphore_request(tJsonContext.h_UnpackSemphMutex, ZA_portMAX_DELAY);

   cJSON *pTagCmd = cJSON_GetObjectItem(tJsonContext.pUnpackRoot, "from");

   if (pTagCmd != ZA_NULL)
   {
      if (cJSON_IsString(pTagCmd))
      {
         tJsonContext.pUnPackDataStr = pTagCmd->valuestring;
      }
   }

   //za_semaphore_release(tJsonContext.h_UnpackSemphMutex);

   return tJsonContext.pUnPackDataStr;
}

ZA_CHAR * za_json_unpack_localserver_ip(ZA_VOID *pvParameter)
{
   (ZA_VOID) pvParameter;

   tJsonContext.pUnPackDataStr = ZA_NULL;

   if ((! tJsonContext.b_IfInited) || (tJsonContext.pUnpackRoot == ZA_NULL))
   {
      return tJsonContext.pUnPackDataStr;
   }

   cJSON *pTagCmd = cJSON_GetObjectItem(tJsonContext.pUnpackRoot, "params");

   if (pTagCmd != ZA_NULL)
   {
      if (cJSON_IsString(pTagCmd))
      {
         tJsonContext.pUnPackDataStr = pTagCmd->valuestring;
      }
   }

   return tJsonContext.pUnPackDataStr;
}
