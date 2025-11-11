/*   Inner system everything . */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"
#include "za_command.h"

#include "zhengan.h"
#include "za_system.h"
#include "za_storage.h"
#include "za_wifi.h"
#include "za_info.h"
#include "za_json.h"
#include "za_innersor.h"

const static char *TAG = "ZA_SYSTEM";

typedef struct
{
    ZA_SemaphoreHandle_t h_innersor_system_semph;
} za_system_context_t;

static za_system_context_t za_system_context;

static za_storage_game_ctrl_mode_t za_system_game_ctrl_mode = ZA_STORAGE_GAME_OPEN_EVENT;
static za_storage_game_msg_ctrl_mode_t za_system_game_msg_ctrl_mode = ZA_STORAGE_GAME_MSG_CTRLMODE_JSON;
static za_storage_game_cmd_ctrl_mode_t za_system_game_cmd_ctrl_mode = ZA_STORAGE_GAME_CMD_CTRLMODE_JSON;
static za_storage_game_msg_cmd_mode_t za_system_game_msgcmd_mode_switch = ZA_STORAGE_GAME_CMD_MODE_ON;

ZA_BOOL za_system_init()
{
   za_memset(&za_system_context, 0x00, za_sizeof(za_system_context_t));

   za_system_context.h_innersor_system_semph = za_semaphore_create(1,1);

   return ZA_TRUE;
}

ZA_UCHAR za_system_Reboot(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_semaphore_request(za_system_context.h_innersor_system_semph, ZA_portMAX_DELAY);

   esp_restart();

   za_semaphore_release(za_system_context.h_innersor_system_semph);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_RebootDelay(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_semaphore_request(za_system_context.h_innersor_system_semph, ZA_portMAX_DELAY);

   vTaskDelay( 1500 / ZA_portTICK_PERIOD_MS );
   esp_restart();

   za_semaphore_release(za_system_context.h_innersor_system_semph);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_Standby(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_semaphore_request(za_system_context.h_innersor_system_semph, ZA_portMAX_DELAY);

   vTaskDelay( 500 / ZA_portTICK_PERIOD_MS );
   za_innersor_power_all_ctrl(ZA_FALSE);
   vTaskDelay( 500 / ZA_portTICK_PERIOD_MS );
   esp_deep_sleep_start();

   za_semaphore_release(za_system_context.h_innersor_system_semph);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_PowerEC(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_PowerFL(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_powercheck_update(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   za_app_msg_data_t za_appmsg_data;

   ZA_UINT32 nPercent = 101;

   if (nPercent > 100)
   {
      za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_BATTERY_CHARGE;
      za_appmsg_data.n_param1 = nPercent;

      if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(ZANULL), &za_appmsg_data, portMAX_DELAY))
      {
         ZA_ESP_LOGI(TAG, "start send ZA_APP_EVENT_BATTERY_CHARGE failure. ---- > %d", za_appmsg_data.za_appmsg_type);
      }
   }
   else if ((nPercent > 20) && (nPercent <= 100))
   {
      za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_BATTERY_NORMAL;
      za_appmsg_data.n_param1 = nPercent;

      if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(ZANULL), &za_appmsg_data, portMAX_DELAY))
      {
         ZA_ESP_LOGI(TAG, "start send ZA_APP_EVENT_BATTERY_NORMAL failure. ---- > %d", za_appmsg_data.za_appmsg_type);
      }
   }
   else if ((nPercent >= 10) && (nPercent <= 20))
   {
      za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_BATTERY_LOW_POWER;
      za_appmsg_data.n_param1 = nPercent;

      if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(ZANULL), &za_appmsg_data, portMAX_DELAY))
      {
         ZA_ESP_LOGI(TAG, "start send ZA_APP_EVENT_BATTERY_LOW_POWER failure. ---- > %d", za_appmsg_data.za_appmsg_type);
      }
   }
   else if ((nPercent > 0) && (nPercent < 10))
   {
      za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_BATTERY_TOO_LOW_POWER;
      za_appmsg_data.n_param1 = nPercent;

      if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(ZANULL), &za_appmsg_data, portMAX_DELAY))
      {
         ZA_ESP_LOGI(TAG, "start send ZA_APP_EVENT_BATTERY_TOO_LOW_POWER failure. ---- > %d", za_appmsg_data.za_appmsg_type);
      }
   }
   else
   {
      za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_BATTERY_LOW_TO_STANDBY;
      za_appmsg_data.n_param1 = nPercent;

      if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(ZANULL), &za_appmsg_data, portMAX_DELAY))
      {
         ZA_ESP_LOGI(TAG, "start send ZA_APP_EVENT_BATTERY_LOW_TO_STANDBY failure. ---- > %d", za_appmsg_data.za_appmsg_type);
      }
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_CMD_StaStore(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_CMD_EndStore(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_CMD_BoOrder(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_CMD_BoCircle(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_CMD_BoRandom(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_CMD_BoOff(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_CMD_BoType(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_CMD_Delay(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_AIOT_Internet(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tAiotServerMode != ZA_STORAGE_AIOT_SERV_INTERNET)
   {
      tSysMode.tAiotServerMode = ZA_STORAGE_AIOT_SERV_INTERNET;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_AIOT_Internet error! ");
      }
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_AIOT_Localserv(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tAiotServerMode != ZA_STORAGE_AIOT_SERV_LOCALSERV)
   {
      tSysMode.tAiotServerMode = ZA_STORAGE_AIOT_SERV_LOCALSERV;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_AIOT_Internet error! ");
      }
   }
   
   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_BindID(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   //char sLocalParam[ZA_SYS_DATA_BUF_LEN] = {0};

   za_sys_msg_data_t za_sysmsg_data;

   za_memset(za_sysmsg_data.aDataBuffer, ZA_NULL, ZA_SYS_DATA_BUF_LEN);
   //za_sprintf((ZA_CHAR *)sLocalParam, "%s", p);

   //lm_SysInnerCheckBindId((ZA_CHAR *)(za_sysmsg_data.aDataBuffer));
   if(za_common_get_bind_id_from_string(p, (ZA_CHAR *)(za_sysmsg_data.aDataBuffer)) == 0)
   {
      return ZA_CMD_ERR;
   }

   za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_UPDATE_BIND_FLAG_ID;

   if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
   {
      ZA_ESP_LOGI(TAG, "za_system_BindID error! ");
   }
   
   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_MQTT_Server(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_app_msg_data_t za_appmsg_data;

   char sLocalParam[ZA_SYS_DATA_BUF_LEN] = {0};

   //za_sprintf((ZA_CHAR *)sLocalParam, "%s", p);

   if (za_common_get_localmqtt_url_from_string_only(p, sLocalParam) == 0)
   {
      return ZA_CMD_ERR;
   }

   za_aiot_local_set_server_url(sLocalParam);

   za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_AIOTLOCAL_SET_PARAMS;

   if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(NULL), &za_appmsg_data, portMAX_DELAY))
   {
      ZA_ESP_LOGI(TAG, "za_system_MQTT_Server error! ");
   }

   za_aiot_local_set_server_uname("zdmedia");
   za_aiot_local_set_server_upasswd("123456");
   
   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_WL_BleHost(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tWirelessMode != ZA_STORAGE_BLE_HOST_MODE)
   {
      tSysMode.tWirelessMode = ZA_STORAGE_BLE_HOST_MODE;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_WL_BleHost error! ");
      }
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_WL_WifiClient(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tWirelessMode != ZA_STORAGE_WIFI_CLIENT_MODE)
   {
      tSysMode.tWirelessMode = ZA_STORAGE_WIFI_CLIENT_MODE;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_WL_BleHost error! ");
      }
   }
   
   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_WL_BleHostWifiClient(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tWirelessMode != ZA_STORAGE_BLEHOST_WIFICLIENT_MODE)
   {
      tSysMode.tWirelessMode = ZA_STORAGE_BLEHOST_WIFICLIENT_MODE;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_WL_BleHostWifiClient error! ");
      }
   }
   
   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_WL_GetMode(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   ZA_ESP_LOGI(TAG, "za_system_WL_GetMode : %d . \n", tSysMode.tWirelessMode);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_OM_Zhendong(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tOutmoduleType != ZA_STORAGE_OUT_ZHENDONG_SENSOR)
   {
      tSysMode.tOutmoduleType = ZA_STORAGE_OUT_ZHENDONG_SENSOR;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_OM_Zhendong error! ");
      }
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_OM_RTGanying(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tOutmoduleType != ZA_STORAGE_OUT_RENTIGANYING_SENSOR)
   {
      tSysMode.tOutmoduleType = ZA_STORAGE_OUT_RENTIGANYING_SENSOR;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_OM_RTGanying error! ");
      }
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_OM_HWDGGanying(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tOutmoduleType != ZA_STORAGE_OUT_HONGWAIDUIGUAN_SENSOR)
   {
      tSysMode.tOutmoduleType = ZA_STORAGE_OUT_HONGWAIDUIGUAN_SENSOR;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_OM_Accgyro error! ");
      }
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_OM_Accgyro(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tOutmoduleType != ZA_STORAGE_OUT_LIUZHOUTUOLUOYI_SENSOR)
   {
      tSysMode.tOutmoduleType = ZA_STORAGE_OUT_LIUZHOUTUOLUOYI_SENSOR;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_OM_Accgyro error! ");
      }
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_OM_Compass(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tOutmoduleType != ZA_STORAGE_OUT_DIANZILUOPAN_SENSOR)
   {
      tSysMode.tOutmoduleType = ZA_STORAGE_OUT_DIANZILUOPAN_SENSOR;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_OM_Compass error! ");
      }
   }
   
   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_OM_Servo(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tOutmoduleType != ZA_STORAGE_OUT_SERVO_DRIVER)
   {
      tSysMode.tOutmoduleType = ZA_STORAGE_OUT_SERVO_DRIVER;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_OM_Servo error! ");
      }
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_OM_RobotT1(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tOutmoduleType != ZA_STORAGE_OUT_ROBOT_T1)
   {
      tSysMode.tOutmoduleType = ZA_STORAGE_OUT_ROBOT_T1;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_OM_RobotT1 error! ");
      }
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_OM_Action(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   if (tSysMode.tOutmoduleType != ZA_STORAGE_OUT_MARK_ACTION)
   {
      tSysMode.tOutmoduleType = ZA_STORAGE_OUT_MARK_ACTION;

      if (!za_storage_set_system_mode(tSysMode))
      {
         ZA_ESP_LOGI(TAG, "za_system_OM_Action error! ");
      }
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_OM_GetType(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

   ZA_ESP_LOGI(TAG, "za_system_OM_GetType : %d . \n", tSysMode.tOutmoduleType);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_EraseLS(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   if (!za_storage_erase_path_space(ZA_STORAGE_LOCALSTORAGE_PATH))
   {
      ZA_ESP_LOGI(TAG, "za_system_EraseLS error! ");
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_EraseLS_SysMode(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   if (!za_storage_erase_key_space(ZA_STORAGE_LOCALSTORAGE_PATH, ZA_STORAGE_SYSTEM_MODE_KEY))
   {
      ZA_ESP_LOGI(TAG, "za_system_EraseLS_SysMode error! ");
   }

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_Wifi_DisConNull(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_wifi_disconnect_from_ap_set_null(NULL);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_Monitor_Sys(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   za_info_monitor_sys(NULL);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_JSON(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;

   char *strJson = NULL;

   za_json_start_to_pack(NULL);
   za_json_pack_system_info(NULL);

   strJson = za_json_pack_format(NULL);
   ZA_ESP_LOGI(TAG, "%s", strJson);
   ZA_ESP_LOGI(TAG, "\n len: %d \n", za_strlen(strJson));

   strJson = za_json_pack_unformat(NULL);
   ZA_ESP_LOGI(TAG, "%s", strJson);
   ZA_ESP_LOGI(TAG, "\n len: %d \n", za_strlen(strJson));

   za_json_end_to_pack(NULL);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_GetMac(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_APDS9960Init(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   //lm_SysInnerDriversInitAPDS9960(NULL);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_MPU6050Init(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   //lm_SysInnerDriversInitMPU6050(NULL);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_ShakeInit(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   //lm_SysOuterDriversInitShake(NULL);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_BodysenInit(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   //lm_SysOuterDriversInitBodysen(NULL);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_IrpartyInit(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   //lm_SysOuterDriversInitIrparty(NULL);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_AccgyroInit(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_CompassInit(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   //lm_SysOuterDriversInitHMC5883(NULL);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_ActionInit(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   
   //lm_SysInnerDriversInitMarkAction(NULL);

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_VisibleInit(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_system_RFIDInit(ZA_CHAR *p, ZA_UCHAR len)
{
   (ZA_VOID) p;
   

   return ZA_CMD_SUCCESS;
}

//------------------- not for cmds here---------------


ZA_UINT32 za_system_get_time_now(ZA_VOID_PARA *arg)
{
   (ZA_VOID) arg;

   //struct timeval tv_now;
   //gettimeofday(&tv_now, NULL);
   //int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
   
   int64_t time_us = esp_system_get_time();

   return (ZA_UINT32)time_us;
}

ZA_VOID za_system_gamemode_set(za_storage_game_ctrl_mode_t eGameCtrlMode)
{
   if ((eGameCtrlMode == ZA_STORAGE_GAME_MODE_NULL) || (eGameCtrlMode == ZA_STORAGE_GAME_MODE_END))
   {
      za_system_game_ctrl_mode = ZA_STORAGE_GAME_OPEN_EVENT;
   }
   else
   {
      za_system_game_ctrl_mode = eGameCtrlMode;
   }

   return;
}

za_storage_game_ctrl_mode_t za_system_gamemode_get(ZA_VOID_PARA *arg)
{
   (ZA_VOID) arg;

   return za_system_game_ctrl_mode;
}

ZA_VOID za_system_game_msg_ctrlmode_set(za_storage_game_msg_ctrl_mode_t eGameMsgCtrlMode)
{
   if ((eGameMsgCtrlMode == ZA_STORAGE_GAME_MSG_CTRLMODE_NULL) || (eGameMsgCtrlMode == ZA_STORAGE_GAME_MSG_CTRLMODE_END))
   {
      za_system_game_msg_ctrl_mode = ZA_STORAGE_GAME_MSG_CTRLMODE_SIMPLE;
   }
   else
   {
      za_system_game_msg_ctrl_mode = eGameMsgCtrlMode;
   }

   return;
}

za_storage_game_msg_ctrl_mode_t za_system_game_msg_ctrlmode_get(ZA_VOID_PARA *arg)
{
   (ZA_VOID) arg;

   return za_system_game_msg_ctrl_mode;
}

ZA_VOID za_system_game_cmd_ctrlmode_set(za_storage_game_cmd_ctrl_mode_t eGameCmdCtrlMode)
{
   if ((eGameCmdCtrlMode == ZA_STORAGE_GAME_CMD_CTRLMODE_NULL) || (eGameCmdCtrlMode == ZA_STORAGE_GAME_CMD_CTRLMODE_END))
   {
      za_system_game_cmd_ctrl_mode = ZA_STORAGE_GAME_CMD_CTRLMODE_JSON;
   }
   else
   {
      za_system_game_cmd_ctrl_mode = eGameCmdCtrlMode;
   }

   return;
}

za_storage_game_cmd_ctrl_mode_t za_system_game_cmd_ctrlmode_get(ZA_VOID_PARA *arg)
{
   (ZA_VOID) arg;

   return za_system_game_cmd_ctrl_mode;
}

ZA_VOID za_system_game_msgcmd_mode_switch_set(za_storage_game_msg_cmd_mode_t eGameMsgCmdCtrlMode)
{
   if ((eGameMsgCmdCtrlMode == ZA_STORAGE_GAME_MSG_CMD_CTRL_NULL) || (eGameMsgCmdCtrlMode == ZA_STORAGE_GAME_MSG_CMD_CTRL_END))
   {
      za_system_game_msgcmd_mode_switch = ZA_STORAGE_GAME_CMD_MODE_ON;
   }
   else
   {
      za_system_game_msgcmd_mode_switch = eGameMsgCmdCtrlMode;
   }

   return;
}

za_storage_game_msg_cmd_mode_t za_system_game_msgcmd_mode_switch_get(ZA_VOID_PARA *arg)
{
   (ZA_VOID) arg;

   return za_system_game_msgcmd_mode_switch;
}


// certain info report for circle
ZA_VOID za_system_msg_magic_box_update_report(ZA_VOID_PARA *arg)
{
   (ZA_VOID) arg;

   za_system_game_msg_ctrlmode_set(ZA_STORAGE_GAME_MSG_CTRLMODE_SIMPLE);
   za_system_gamemode_set(ZA_STORAGE_GAME_MAGIC_BOX_UPDATE_REPORT);
   za_app_game_set_datacheck_time_interval(atoi("1000"));

   za_app_game_set_magic_box_msg_report("  升级中，请稍后 ......");

   return;
}

ZA_VOID za_system_msg_magic_box_report_ontime(ZA_CHAR *pcMsgReport)
{
   za_system_game_msg_ctrlmode_set(ZA_STORAGE_GAME_MSG_CTRLMODE_SIMPLE);
   za_system_gamemode_set(ZA_STORAGE_GAME_MODE_NULL);

   za_innersor_game_do_msg_magic_box_update_report(pcMsgReport);
}
