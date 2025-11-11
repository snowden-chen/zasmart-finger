/*  zhengan framework base file . */

/*
   NOTE: Writed By ZachSnowdenChen
*/



#include "za_common.h"
#include "za_command.h"

#include "zhengan.h"

#include "za_upgrade.h"
#include "za_storage.h"
#include "za_appext.h"
#include "za_aiot.h"
#include "za_system.h"
#include "za_innersor.h"

static const char *TAG = "ZHENGAN";

#define ZA_SYS_MSG_QUEUE_DEEPTH (64)
#define ZA_APP_MSG_QUEUE_DEEPTH (64)

#define ZA_TIMER_ONE_SHOT (0)
#define ZA_TIMER_ACTION_TIMEOUT (100) // ms

#define ZA_APP_MSG_TASK_STACKSIZE ZA_BUFFER_8K
#define ZA_APP_MSG_TASK_PRIORITY 5

#define ZA_SYS_MSG_TASK_STACKSIZE ZA_BUFFER_8K
#define ZA_SYS_MSG_TASK_PRIORITY 3

#define ZA_WIFI_STA_MAXIMUM_RETRY  CONFIG_WIFISTA_MAXIMUM_RETRY

typedef enum
{
   ZA_TIMER_START = 0,
   ZA_TIMER_WORKING,
   ZA_TIMER_CHECK_STATUS,
   ZA_TIMER_CHECK_KEY,
   ZA_TIMER_MODULE_ACTION,
   ZA_TIMER_GAME_CHECK,
   ZA_TIMER_TEST_MODE,
   ZA_TIMER_END
} za_timer_mode_t;

typedef struct
{
   ZA_QueueHandle_t h_app_msg;
} za_app_context_t;

typedef struct
{
   ZA_BOOL b_factory_mode;
   ZA_BOOL b_Ble_Wifi_allConnect;
   ZA_BOOL b_Ble_Connect;
   ZA_BOOL b_Wifi_Connect;
   ZA_QueueHandle_t h_sys_msg;
   ZA_SemaphoreHandle_t h_sys_SemphStatus;
   ZA_TimerHandle_t nh_timer_list[ZA_TIMER_END];
} za_system_context_t;

typedef struct
{
   za_app_context_t zhengan_app_context;
   za_system_context_t zhengan_sys_context;
} za_main_context_t;

typedef ZA_VOID (*za_timer_notify)(ZA_TimerHandle_t hTimerHandle);

static za_main_context_t zhengan_main_context;

static ZA_INT32 nWifiRetryNum = 0;

static ZA_VOID _za_sys_set_wifi_ifconnect(ZA_BOOL bIfConnect);
static ZA_VOID _za_sys_set_ble_ifconnect(ZA_BOOL bIfConnect);
static ZA_VOID _za_sys_set_blewifiall_ifconnect(ZA_VOID *arg);
static ZA_BOOL _za_sys_get_blewifiall_ifconnect(ZA_VOID *arg);

static wifi_config_t tWifiConfigTemp;
static wifi_config_t tWifiConfigCurrent;

static ZA_UINT32 nBatteryPercent = 101;

static ZA_INT32 nAiotErrorCount = -1;
static ZA_INT32 nAiotDisconnectCount = -1;

static ZA_CHAR MagicBoxMsgBuffer[ZHENGAN_APP_MAG_DATABUF_LEN] = {0};

static ZA_BOOL bAutoUpgradeFlag = ZA_FALSE;

/*** private ***/

/*
#define CHECK_ERROR_CODE(returned, expected) ({                        \
            if(returned != expected){                                  \
                printf("TWDT ERROR\n");                                \
                abort();                                               \
            }                                                          \
})
*/

static ZA_BOOL _za_timer_create(za_timer_mode_t stModule, ZA_INT32 nPeriod_ms, za_timer_notify notifyFunc)
{
   ZA_CHAR timerName[16] = {0};
   ZA_TimerHandle_t hTmpHandle = ZA_NULL;
   za_memset(timerName, 0, za_sizeof(timerName));
   za_sprintf(timerName, "za_timer_%d", stModule);

   ZA_ESP_LOGI(TAG, "za_timer_list[%d] = %p\n", stModule, zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule]);

   if (zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule] == ZA_NULL)
   {
      zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule] = 0;

      hTmpHandle = za_timer_create(timerName, nPeriod_ms / ZA_portTICK_PERIOD_MS, ZA_TIMER_ONE_SHOT, (void *)stModule, notifyFunc);
      if (hTmpHandle == ZA_NULL)
      {
         ZA_ESP_LOGE(TAG, "Timer %s create failed\n", timerName);
         return ZA_FALSE;
      }
      zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule] = hTmpHandle;

      if (za_timer_start(hTmpHandle, ZA_TIMER_ACTION_TIMEOUT) != pdPASS)
      {
         ZA_ESP_LOGE(TAG, "Timer %s start failed,need to start manual\n", timerName);
         return ZA_FALSE;
      }
   }
   else
   {
      za_timer_change_period(zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule], nPeriod_ms / ZA_portTICK_PERIOD_MS, ZA_TIMER_ACTION_TIMEOUT);

      if (stModule == ZA_TIMER_WORKING)
      {
         ZA_ESP_LOGI(TAG, "timer change timer %d ms", nPeriod_ms);
      }
   }

   ZA_ESP_LOGI(TAG, "za_timer_list[%d] = %p\n", stModule, zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule]);
   return !!(zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule]);
}

/*
static ZA_BOOL _za_timer_change_period(za_timer_mode_t stModule, ZA_UINT32 nPeriod_ms) //may be has not working well.
{
   ZA_ESP_LOGI(TAG, "_za_timer_change_period");

   if (zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule] != ZA_NULL)
   {
      za_timer_change_period(zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule], nPeriod_ms / ZA_portTICK_PERIOD_MS, ZA_TIMER_ACTION_TIMEOUT);

      if (stModule == ZA_TIMER_WORKING)
      {
         ZA_ESP_LOGI(TAG, "timer change timer %d ms", nPeriod_ms);
      }
   }
   else
   {
      return ZA_FALSE;
   }

   return ZA_TRUE;
}
*/

static ZA_BOOL _za_timer_activate(za_timer_mode_t stModule)
{
   //ZA_ESP_LOGI(TAG, "za_timer_activate");

   if (zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule] != ZA_NULL)
   {
      za_timer_reset(zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule], ZA_TIMER_ACTION_TIMEOUT);
   }

   return ZA_TRUE;
}

/*
static ZA_BOOL _za_timer_deactivate(za_timer_mode_t stModule)
{
   //ZA_ESP_LOGI(TAG, "za_timer_deactivate");

   if (zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule] != ZA_NULL)
   {
      za_timer_stop(zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule], ZA_TIMER_ACTION_TIMEOUT);
   }

   return ZA_TRUE;
}
*/

/*
static ZA_BOOL _za_timer_delete(za_timer_mode_t stModule)
{
   ZA_ESP_LOGI(TAG, "za_timer_deactivate");

   if (zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule] != ZA_NULL)
   {
      za_timer_delete(zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule], ZA_TIMER_ACTION_TIMEOUT);

      zhengan_main_context.zhengan_sys_context.nh_timer_list[stModule] = ZA_NULL;
   }

   return ZA_TRUE;
}
*/

static ZA_UINT32 nCheckStatusCnt = 0;
static void _za_timer_check_status_callback(ZA_TimerHandle_t hTimerHandle)
{
   //ZA_ESP_LOGI(TAG, "_za_timer check_status callback.");

   nCheckStatusCnt ++;

   if(nCheckStatusCnt > 4294967290)
   {
      nCheckStatusCnt = 0;
   }
/*
   if (nCheckStatusCnt > 20)
   {
      nCheckStatusCnt = 0;
      za_command_excute("SystmInr Y PowerUpdate");
   }
*/

   if (nCheckStatusCnt == 5)
   {
      za_storage_sys_mode_t tSystemMode = za_storage_get_system_mode(NULL);

      /*  fast boot 
      if (tSystemMode.nFastBootCount >= 3)
      {
         //tSystemMode.tWirelessMode = ZA_STORAGE_BLE_HOST_MODE;
         //za_command_excute("SystmInr Y Wifi-DisConNull");
         //za_command_excute("SystmInr Y RebootDelay");
      }*/

      tSystemMode.nFastBootCount = 0;
      za_storage_set_system_mode(tSystemMode);
   }

   za_configASSERT(hTimerHandle);

   if (_za_sys_get_blewifiall_ifconnect(NULL))
   {
      za_innersor_wifi_led_ctrl(ZA_TRUE);
      za_innersor_ble_led_ctrl(ZA_TRUE);
   }
   else
   {
      if (zhengan_main_context.zhengan_sys_context.b_Ble_Connect)
      {
         za_innersor_ble_led_ctrl(ZA_TRUE);
      }
      else
      {
         za_innersor_ble_led_ctrl(ZA_FALSE);
      }

      if (zhengan_main_context.zhengan_sys_context.b_Wifi_Connect)
      {
         za_innersor_wifi_led_ctrl(ZA_TRUE);
      }
      else
      {
         za_innersor_wifi_led_ctrl(ZA_FALSE);
      }
   }

   _za_timer_activate(ZA_TIMER_CHECK_STATUS);

   return;
}

static void _za_timer_working_callback(ZA_TimerHandle_t hTimerHandle)
{
   //ZA_ESP_LOGI(TAG, "_za_timer working callback.");

   za_configASSERT(hTimerHandle);

   if (zhengan_main_context.zhengan_sys_context.b_Wifi_Connect)
   {
      if (za_aiot_connect_status(ZA_NULL))
      {
         za_app_send_sysinfo_to_serv();
      }
   }

   _za_timer_activate(ZA_TIMER_WORKING);

   //LongMore_Wdt_DoFeeding(NULL);
   //za_command_excute("SystmInr Y wdtfeed");

   return;
}

static ZA_UINT32 nCheckPowerKeyCnt = 0; // 50 ms/check
static void _za_timer_check_key_callback(ZA_TimerHandle_t hTimerHandle)
{
   //ZA_ESP_LOGI(TAG, "_za_timer_check_key_callback");

   za_configASSERT(hTimerHandle);

   za_app_msg_data_t za_appmsg_data;
   za_sys_msg_data_t za_sysmsg_data;

   if (za_innersor_power_key_check(ZANULL, 0) == ZA_TRUE)
   {
      nCheckPowerKeyCnt++;
   }
   else
   {
      if ((nCheckPowerKeyCnt > 1) && (nCheckPowerKeyCnt <= 40))
      {
         ZA_ESP_LOGI(TAG, "za_innersor_power_key_check - ZA_SYS_EVENT_KEY_SHORT_UP");

         za_sysmsg_data.za_sysmsg_type = ZA_SYS_EVENT_KEY_SHORT_UP;

         if(ZA_ESP_PASS != za_msg_queue_send(za_get_sys_msg_handle(ZA_NULL), &za_sysmsg_data, portMAX_DELAY))
         {
            ZA_ESP_LOGE(TAG,"SEND za_sysmsg_data : %d failure",za_sysmsg_data.za_sysmsg_type);
         }
      }
      else if (nCheckPowerKeyCnt > 40)// 2000 ms, long time key check
      {
         ZA_ESP_LOGI(TAG, "nCheckPowerKeyCnt > 40");

         za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_BATTERY_LOW_TO_STANDBY;
         za_appmsg_data.n_param1 = 0;

         if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(ZANULL), &za_appmsg_data, portMAX_DELAY))
         {
            ZA_ESP_LOGI(TAG, "start send ZA_APP_EVENT_BATTERY_LOW_TO_STANDBY failure. ---- > %d", za_appmsg_data.za_appmsg_type);
         }
      }

      nCheckPowerKeyCnt = 0;
   }
   
   _za_timer_activate(ZA_TIMER_CHECK_KEY);

   return;
}

/*
static void _za_timer_test_mode_callback(ZA_TimerHandle_t hTimerHandle)
{
   ZA_ESP_LOGI(TAG, "_za_timer test mode callback.");

   za_configASSERT(hTimerHandle);

   _za_timer_activate(ZA_TIMER_TEST_MODE);

   za_sys_msg_data_t st_cb_data =
   {
      .za_sysmsg_type = ZA_SYS_EVENT_TEST_MODE,
      .n_param1 = 1,
      .n_param2 = 2,
      .n_param3 = 3,
   };

   if(ZA_ESP_PASS != za_msg_queue_send(zhengan_main_context.zhengan_sys_context.h_sys_msg, &st_cb_data,portMAX_DELAY))
   {
      ZA_ESP_LOGE(TAG,"MAIN SEND msg :%d failure",st_cb_data.za_sysmsg_type);
   }

   _za_timer_activate(ZA_TIMER_TEST_MODE);

   return;
}
*/

static ZA_UINT32 nGameCallbackTimeCount = 0; // 10ms/per for control the interval time of data.
static ZA_UINT32 nGameCallbackCounter = 0;

static void _za_timer_check_game_callback(ZA_TimerHandle_t hTimerHandle)
{
   za_configASSERT(hTimerHandle);

   if (nGameCallbackTimeCount == 0) // pause this timer, run null
   {
      //nGameCallbackTimeCount = 1;

      _za_timer_activate(ZA_TIMER_GAME_CHECK);

      return;
   }

   nGameCallbackCounter ++;
   if (nGameCallbackCounter < nGameCallbackTimeCount)
   {
      _za_timer_activate(ZA_TIMER_GAME_CHECK);

      return;
   }

   nGameCallbackCounter = 0;

   // goto run the well task:

   switch (za_system_gamemode_get(ZANULL))
   {
      case ZA_STORAGE_GAME_MAGIC_BOX_UPDATE_REPORT:
      {
         za_innersor_game_do_msg_magic_box_update_report(MagicBoxMsgBuffer);
      }
      break;

      default:
      {
         ;
      }
      break;
   }

   _za_timer_activate(ZA_TIMER_GAME_CHECK);

   return;
}



static ZA_VOID _za_sys_test_mode_do(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   //LongMore_ExcuteCmd("APDS9960 Y OpenColor");

   //vTaskDelay(ZA_pdMS_TO_TICKS(2000));

   //LongMore_ExcuteCmd("APDS9960 Y CloseColor");

   return;
}

static ZA_VOID _za_sys_set_wifi_ifconnect(ZA_BOOL bIfConnect)
{
   za_semaphore_request(zhengan_main_context.zhengan_sys_context.h_sys_SemphStatus, ZA_portMAX_DELAY);

   zhengan_main_context.zhengan_sys_context.b_Wifi_Connect = bIfConnect;

   za_semaphore_release(zhengan_main_context.zhengan_sys_context.h_sys_SemphStatus);

   return;
}

static ZA_VOID _za_sys_set_ble_ifconnect(ZA_BOOL bIfConnect)
{
   za_semaphore_request(zhengan_main_context.zhengan_sys_context.h_sys_SemphStatus, ZA_portMAX_DELAY);

   zhengan_main_context.zhengan_sys_context.b_Ble_Connect = bIfConnect;

   za_semaphore_release(zhengan_main_context.zhengan_sys_context.h_sys_SemphStatus);

   return;
}

static ZA_VOID _za_sys_set_blewifiall_ifconnect(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   za_semaphore_request(zhengan_main_context.zhengan_sys_context.h_sys_SemphStatus, ZA_portMAX_DELAY);

   if (zhengan_main_context.zhengan_sys_context.b_Ble_Connect && zhengan_main_context.zhengan_sys_context.b_Wifi_Connect)
   {
      zhengan_main_context.zhengan_sys_context.b_Ble_Wifi_allConnect = ZA_TRUE;
   }
   else
   {
      zhengan_main_context.zhengan_sys_context.b_Ble_Wifi_allConnect = ZA_FALSE;
   }

   za_semaphore_release(zhengan_main_context.zhengan_sys_context.h_sys_SemphStatus);

   return;
}

static ZA_BOOL _za_sys_get_blewifiall_ifconnect(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   za_semaphore_request(zhengan_main_context.zhengan_sys_context.h_sys_SemphStatus, ZA_portMAX_DELAY);

   if (zhengan_main_context.zhengan_sys_context.b_Ble_Connect && zhengan_main_context.zhengan_sys_context.b_Wifi_Connect)
   {
      zhengan_main_context.zhengan_sys_context.b_Ble_Wifi_allConnect = ZA_TRUE;
   }
   else
   {
      zhengan_main_context.zhengan_sys_context.b_Ble_Wifi_allConnect = ZA_FALSE;
   }

   za_semaphore_release(zhengan_main_context.zhengan_sys_context.h_sys_SemphStatus);

   return zhengan_main_context.zhengan_sys_context.b_Ble_Wifi_allConnect;
}

static ZA_VOID _za_app_msg_task(ZA_VOID_PARA *arg)
{
   (void)arg;

   ZA_BOOL appmsg_run_flag = ZA_TRUE;

   za_app_msg_data_t za_msg_data;

   while (appmsg_run_flag)
   {
      // ZA_ESP_LOGI(TAG, "_za_app_msg_task : thread task test... ");
      za_memset(&za_msg_data, 0x00, za_sizeof(za_app_msg_data_t));

      if (ZA_ESP_PASS != za_msg_queue_recv(zhengan_main_context.zhengan_app_context.h_app_msg, &za_msg_data, ZA_portMAX_DELAY))
      {
         ZA_ESP_LOGE(TAG, "za_msg_queue_recv : zhengan_app_context.h_app_msg.");

         continue;
      }

      switch (za_msg_data.za_appmsg_type)
      {
         case ZA_APP_EVENT_TEST_MODE:
         {

         }
         break;

         case ZA_APP_EVENT_BATTERY_CHARGE:
         {
            nBatteryPercent = za_msg_data.n_param1;
         }
         break;

         case ZA_APP_EVENT_BATTERY_NORMAL:
         {
            nBatteryPercent = za_msg_data.n_param1;
         }
         break;

         case ZA_APP_EVENT_BATTERY_LOW_POWER:
         {
            nBatteryPercent = za_msg_data.n_param1;
         }
         break;

         case ZA_APP_EVENT_BATTERY_TOO_LOW_POWER:
         {
            nBatteryPercent = za_msg_data.n_param1;
         }
         break;

         case ZA_APP_EVENT_BATTERY_LOW_TO_STANDBY:
         {
            nBatteryPercent = za_msg_data.n_param1;

            za_command_excute("SystmInr Y Standby");
         }
         break;        

         case ZA_APP_EVENT_DO_CONNECT_MQTTS:
         {
            if (ZA_TRUE == za_aiot_init())
            {
               ZA_vTaskDelay(1500 / ZA_portTICK_PERIOD_MS); // 15ms for test // 2000ms default 

               ZA_ESP_LOGI(TAG, "_ - ZA_APP_EVENT_DO_CONNECT_MQTTS.");

              if(ZA_TRUE != za_aiot_connect_server(ZA_AIOT_MQTTS_PROTOCOL))
              {
                 za_aiot_disconnect_server(ZA_NULL);
                 za_aiot_connect_server(ZA_AIOT_MQTTS_PROTOCOL);
              }
            }
         }
         break;

         case ZA_APP_EVENT_DO_CONNECT_MQTT:
         {
            if (ZA_TRUE == za_aiot_init())
            {
               ZA_vTaskDelay(2000 / ZA_portTICK_PERIOD_MS); // 15ms for test // 2000ms default 

               ZA_ESP_LOGI(TAG, "_ - ZA_APP_EVENT_DO_CONNECT_MQTT.");

              if(ZA_TRUE != za_aiot_connect_server(ZA_AIOT_MQTT_PROTOCOL))
              {
                 za_aiot_disconnect_server(ZA_NULL);
              }
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_CONNECTED:
         {
            if (! za_aiot_connect_set_status(ZA_TRUE))
            {
               ZA_ESP_LOGE(TAG, "AIOT Set Connect ERROR, MUST Check.");
               break;
            }

            nAiotErrorCount = 0;
            nAiotDisconnectCount = 0;

            /*call init out devices interface ... */;
            za_app_require_mainservice_from_serv();

            za_msg_data.za_appmsg_type = ZA_APP_EVENT_AIOT_REPORT_SYSINFO;

            if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(ZA_NULL), &za_msg_data, portMAX_DELAY))
            {
               ZA_ESP_LOGE(TAG,"SEND za_appmsg_data : %d failure",za_msg_data.za_appmsg_type);
            }

            za_storage_sys_mode_t tSystemMode = za_storage_get_system_mode(NULL);

            za_app_memory_load_bind_flag_id(tSystemMode.cBindFlagID);

            if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_ZHENDONG_SENSOR)
            {
               ZA_ESP_LOGI(TAG, "Shake out device module start.");

               //za_command_excute("SystmInr Y ShakeInit"); // for test
            }
            else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_RENTIGANYING_SENSOR)
            {
               ZA_ESP_LOGI(TAG, "Hongwai Renti ganying out device module start.");

               //za_command_excute("SystmInr Y BodysenInit"); // for test
            }
            else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_HONGWAIDUIGUAN_SENSOR)
            {
               ZA_ESP_LOGI(TAG, "Hongwai duiguan ganying out device module start.");

               //za_command_excute("SystmInr Y IrpartyInit"); // for test
            }
            else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_LIUZHOUTUOLUOYI_SENSOR)
            {
               ZA_ESP_LOGI(TAG, "liuzhou tuoluoyi out device module start.");

               //za_command_excute("SystmInr Y MPU6050Init"); // for test
            }
            else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_DIANZILUOPAN_SENSOR)
            {
               ZA_ESP_LOGI(TAG, "dian zi luo pan out device module start.");

               //za_command_excute("SystmInr Y CompassInit"); // for test
            }
            else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_SERVO_DRIVER)
            {
               ZA_ESP_LOGI(TAG, "servo driver module start.");
            }
            else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_ROBOT_T1)
            {
               ZA_ESP_LOGI(TAG, "Robot T1 Now start.");
            }
            else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_MARK_ACTION)
            {
               ZA_ESP_LOGI(TAG, "Action Mark module start.");

               //za_command_excute("SystmInr Y ActionInit"); // for test

               //za_app_module_action(ZANULL);
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_DISCONNECTED:
         {
            ZA_ESP_LOGE(TAG, "ZA_APP_EVENT_AIOT_DISCONNECTED.");

            if (! za_aiot_connect_set_status(ZA_FALSE))
            {
               ZA_ESP_LOGE(TAG, "AIOT Set Connect ERROR, MUST Check.");

               break;
            }

            if (nAiotDisconnectCount >= 0) // means has been connected first
            {
               nAiotDisconnectCount ++;

               if (nAiotDisconnectCount > 3)
               {
                  za_aiot_reconnect_server(ZANULL);

                  nAiotDisconnectCount = 0;
               }
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_ERROR_COM:
         {
            ZA_ESP_LOGE(TAG, "ZA_APP_EVENT_AIOT_ERROR_COM.");

            if (nAiotErrorCount >= 0) // means has been connected first
            {
               nAiotErrorCount ++;

               if (nAiotErrorCount > 7)
               {
                  // reboot system 
                  ZA_ESP_LOGE(TAG, "System will reboot.");

                  za_command_excute("SystmInr Y RebootDelay");
               }
            }
         }
         break;

         case ZA_APP_EVENT_AIOTLOCAL_SET_PARAMS:
         {
            if (! za_aiot_local_set_server_params(za_aiot_local_get_server_url(ZANULL), 
            za_aiot_local_get_server_uname(ZANULL), 
            za_aiot_local_get_server_upasswd(ZANULL)) )
            {
               ZA_ESP_LOGE(TAG, "ZA_APP_EVENT_AIOTLOCAL_SET_PARAMS ERROR.");
            }
         }
         break;

         case ZA_APP_EVENT_AIOTLOCAL_DO_CONNECT:
         {

         }
         break;

         case ZA_APP_EVENT_AIOTLOCAL_CONNECTED:
         {

         }
         break;

         case ZA_APP_EVENT_AIOTLOCAL_DISCONNECTED:
         {

         }
         break;

         case ZA_APP_EVENT_LOCAL_COMMAND_STORE_CTRL:
         {

         }
         break;

         case ZA_APP_EVENT_LOCAL_COMMAND_DELAY_MS:
         {

         }
         break;

         case ZA_APP_EVENT_UPDATE_AFTER_AMOMENT:
         {
            if (zhengan_main_context.zhengan_sys_context.b_Wifi_Connect)
            {
               //if (za_storage_if_file_path_exist(STORAGE_AUTO_UPDATE_APP_PATH))
               //{
                  //ZA_vTaskDelay(2000 / ZA_portTICK_PERIOD_MS);

                  //za_command_excute("UpgradeI y openzasmart");

                  //za_command_excute("UpgradeI y updatebypath");
                  //ZA_vTaskDelay(1000 / ZA_portTICK_PERIOD_MS);
               //}


               za_storage_sys_mode_t tStoreSysModeReadW = za_storage_get_system_mode(ZA_NULL);

               if (tStoreSysModeReadW.tUpgradeMode == ZA_STORAGE_UPGRADE_FORCE_MODE)
               {
                  ZA_vTaskDelay(2000 / ZA_portTICK_PERIOD_MS);
                  //za_command_excute("UpgradeI y updateopen");
                  za_command_excute("upgrade");
               }
            }
         }
         break;


         case ZA_APP_EVENT_AIOT_SEND_SHAKE:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               //za_app_send_shake_event_to_serv();
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_SEND_INFRARED_NEW:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               //za_app_send_infrared_event_to_serv("1");
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_SEND_INFRARED:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               //za_app_send_infrared_event_to_serv("2");
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_SEND_INFRARED_END:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               //za_app_send_infrared_event_to_serv("3");
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_SEND_IRPARTY:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               //za_app_send_irparty_event_to_serv(za_msg_data.cBuffer);
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_SEND_ACCGYRO:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               //za_app_send_accgyro_event_to_serv(za_msg_data.cBuffer);
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_SEND_COMPASS:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               //za_app_send_compass_event_to_serv(za_msg_data.cBuffer);
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_SEND_OID:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               za_app_send_oid_event_to_serv(za_msg_data.cBuffer);
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_START_BIND:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               za_app_require_bindresult_from_serv();

               //za_command_excute("CaidengI Y purple");
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_SEND_BIND_CODE:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               za_app_send_bind_code_to_serv(za_msg_data.cBuffer);
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_REPORT_SYSINFO:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               za_app_send_sysinfo_to_serv();
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_MARK_ACTION_BASE:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               //za_app_send_action_event_to_serv(za_msg_data.cBuffer);
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_SEND_MAIN_COMMAND:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               za_app_send_main_command_to_serv(za_msg_data.cTagBuf, za_msg_data.cBuffer);
            }
         }
         break;

         case ZA_APP_EVENT_AIOT_SEND_MAIN_MESSAGE:
         {
            if (za_aiot_connect_status(ZA_NULL))
            {
               za_app_send_main_message_to_serv(za_msg_data.cTagBuf, za_msg_data.cBuffer);
            }
         }
         break;

         default:
            break;
      }

      ZA_vTaskDelay(10 / ZA_portTICK_PERIOD_MS); // 15ms for test
   }

   return;
}

#define POWER_START_TIMEOUT_SECOND (8)
static ZA_VOID _za_sys_msg_task(ZA_VOID_PARA *arg)
{
   ZA_BOOL sysmsg_run_flag = ZA_TRUE;

   za_sys_msg_data_t za_msg_data;
   za_app_msg_data_t za_appmsg_data;

   ZA_UINT32 n_powerstart_tick = xTaskGetTickCount();
   ZA_BOOL b_powerstart_flag = ZA_TRUE;
   ZA_UINT32 n_key_count  = 0;
   ZA_UINT32  nLastKeyPressTick = xTaskGetTickCount();

   while (sysmsg_run_flag)
   {
      za_memset(&za_msg_data, 0x00, za_sizeof(za_sys_msg_data_t));

      if (ZA_ESP_PASS != za_msg_queue_recv(zhengan_main_context.zhengan_sys_context.h_sys_msg, &za_msg_data, ZA_portMAX_DELAY))
      {
         ZA_ESP_LOGE(TAG, "za_msg_queue_recv : zhengan_sys_context.h_sys_msg");

         return;
      }

      ZA_ESP_LOGI(TAG, "SYS RECV MSG: %d", za_msg_data.za_sysmsg_type);

      if (zhengan_main_context.zhengan_sys_context.b_factory_mode)
      {
         /* process */

         return;
      }

      switch (za_msg_data.za_sysmsg_type)
      {
         case ZA_SYS_EVENT_TEST_MODE:
         {
            ZA_ESP_LOGI(TAG, "SYS EVENT TEST MODE - n_p-1 %d n_p-2 %d n_p-3 %d", za_msg_data.n_param1, za_msg_data.n_param2, za_msg_data.n_param3);

            _za_sys_test_mode_do(NULL);

            /**/
         }
         break;

         case ZA_SYS_EVENT_KEY_SHORT_UP:
         {
            /*
            ZA_vTaskDelay( 100 / ZA_portTICK_PERIOD_MS );

            if (!b_powerstart_flag)
            {
               n_key_count ++;

               if (n_key_count >= 5)
               {
                  n_key_count = 0;
                  za_command_excute("SystmInr Y Wifi-DisConNull");
               }

            }*/

            if (b_powerstart_flag)
            {
               ZA_UINT32 currentTick = xTaskGetTickCount();

               if ((currentTick - n_powerstart_tick)  > pdMS_TO_TICKS(POWER_START_TIMEOUT_SECOND * 1000))
               {
                  b_powerstart_flag = ZA_FALSE;
               }
               else
               {
                  // 判断按键间隔是否合理（如小于 1 秒）
                  if ((currentTick - nLastKeyPressTick) < pdMS_TO_TICKS(1000))
                  {
                        n_key_count++;
                  }
                  else
                  {
                        n_key_count = 1; // 重新开始计数
                  }

                  nLastKeyPressTick = currentTick;

                  if (n_key_count >= 5)
                  {
                        n_key_count = 0;
                        za_command_excute("SystmInr Y Wifi-DisConNull");
                  }
               }
            }

         }
         break;

         case ZA_SYS_EVENT_WIFI_SET_OP_MODE:
         {
            nWifiRetryNum = ZA_WIFI_STA_MAXIMUM_RETRY;

            esp_wifi_disconnect();

            ZA_ESPERROR_CHECK( esp_wifi_set_mode((wifi_mode_t)za_msg_data.n_param1) );

            //za_command_excute("CaidengI Y sigbluef-0");

            _za_timer_activate(ZA_TIMER_CHECK_STATUS);
         }
         break;

         case ZA_SYS_EVENT_WIFI_CONFIG_SSID:
         {
            wifi_config_t *pWifiConfigData = ZA_NULL;
            pWifiConfigData = (wifi_config_t *)za_msg_data.p_data;

            za_memset(&tWifiConfigTemp, ZA_NULL, za_sizeof(wifi_config_t));
         
            za_strcpy((ZA_CHAR *)tWifiConfigTemp.sta.ssid, (ZA_CHAR *)pWifiConfigData->sta.ssid);

            za_free((ZA_VOID *)za_msg_data.p_data);
         }
         break;

         case ZA_SYS_EVENT_WIFI_CONFIG_PASSWD:
         {
            wifi_config_t *pWifiConfigData = ZA_NULL;
            pWifiConfigData = (wifi_config_t *)za_msg_data.p_data;

            za_strcpy((ZA_CHAR *)tWifiConfigTemp.sta.password, (ZA_CHAR *)pWifiConfigData->sta.password);

            za_free((ZA_VOID *)za_msg_data.p_data);
         }
         break;

         case ZA_SYS_EVENT_WIFI_CONNECT_TO_AP:
         {
            ZA_ESP_LOGI(TAG, "Ready to connect : SSID-  %s, PASSWD- %s\n", tWifiConfigTemp.sta.ssid, tWifiConfigTemp.sta.password);
         
            esp_wifi_set_config(WIFI_IF_STA, &tWifiConfigTemp);

            nWifiRetryNum = 0;

            /* there is no wifi callback when the device has already connected to this wifi
            so disconnect wifi before connection.
            */
            esp_wifi_connect();

            za_memcpy(&tWifiConfigCurrent, &tWifiConfigTemp, za_sizeof(wifi_config_t));
            
            za_memset(&tWifiConfigTemp, ZA_NULL, za_sizeof(wifi_config_t));
         }
         break;

         case ZA_SYS_EVENT_WIFI_REQ_DISCONNECT_TO_AP:
         {
            ZA_ESP_LOGI(TAG, "SYS Request Disconnect to AP \n");

            esp_wifi_disconnect();

            nWifiRetryNum = ZA_WIFI_STA_MAXIMUM_RETRY;
         }
         break;

         case ZA_SYS_EVENT_WIFI_REQ_DISCONNECT_AND_SET_NULL:
         {
            ZA_ESP_LOGI(TAG, "SYS Request Disconnect to AP and Set SSID && PASSWD null .\n");

            za_strcpy((ZA_CHAR *)tWifiConfigTemp.sta.ssid, "*#null@!$");
            za_strcpy((ZA_CHAR *)tWifiConfigTemp.sta.password, "@#&$!null*#*!$(^)");

            esp_wifi_set_config(WIFI_IF_STA, &tWifiConfigTemp);

            esp_wifi_disconnect();

            nWifiRetryNum = ZA_WIFI_STA_MAXIMUM_RETRY;
         }
         break;

         case ZA_SYS_EVENT_WIFI_STATUS_CONNECTED:
         {
            ZA_ESP_LOGI(TAG, "SYS EVENT WIFI STATUS Connected. \n");

            nWifiRetryNum = ZA_WIFI_STA_MAXIMUM_RETRY - 2;

            za_innersor_wifi_led_shink_ctrl(ZA_FALSE);
            za_innersor_ble_led_shink_ctrl(ZA_FALSE);

            _za_sys_set_wifi_ifconnect(ZA_TRUE);

            _za_sys_set_blewifiall_ifconnect(NULL);

            _za_timer_activate(ZA_TIMER_CHECK_STATUS);

            za_storage_sys_mode_t tSystemMode = za_storage_get_system_mode(NULL);
            if (tSystemMode.tWirelessMode == ZA_STORAGE_BLE_HOST_MODE)
            {
               ZA_ESP_LOGI(TAG, "Change to WiFi Client Mode.");

               za_command_excute("SystmInr Y WL-WifiClient");
               za_command_excute("SystmInr Y RebootDelay");
            }
            else
            {
               if (!bAutoUpgradeFlag)
               {
                  za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_UPDATE_AFTER_AMOMENT;

                  if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(NULL), &za_appmsg_data, portMAX_DELAY))
                  {
                     ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_appmsg_data.za_appmsg_type);
                  }

                  bAutoUpgradeFlag = ZA_TRUE;
               }

               if (za_aiot_init_status(ZANULL) || za_aiot_connect_status(ZANULL))
               {
                  break;
               }

               if (tSystemMode.tAiotServerMode == ZA_STORAGE_AIOT_SERV_INTERNET)
               {
                  za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_DO_CONNECT_MQTTS;
               }
               else if (tSystemMode.tAiotServerMode == ZA_STORAGE_AIOT_SERV_LOCALSERV)
               {
                  za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_DO_CONNECT_MQTT;
               }
               else
               {
                  ZA_ESP_LOGE(TAG, "AIOT Server Mode Error when wifi connected. \n");
                  break;
               }

               if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(NULL), &za_appmsg_data, portMAX_DELAY))
               {
                  ZA_ESP_LOGE(TAG,"SEND msg : %d failure",za_appmsg_data.za_appmsg_type);
               }
            }
         }
         break;

         case ZA_SYS_EVENT_WIFI_STATUS_UNCONNECTED:
         {
            ZA_ESP_LOGI(TAG, "SYS EVENT WIFI STATUS UnConnected. \n");

            if (nWifiRetryNum < ZA_WIFI_STA_MAXIMUM_RETRY)
            {
               nWifiRetryNum ++;

               esp_wifi_connect();
            }
            else
            {
               /* report the wifi status to application. */
               //za_command_excute("CaidengI Y random-1"); // 1号灯 开关灯;

               _za_sys_set_wifi_ifconnect(ZA_FALSE);

               _za_sys_set_blewifiall_ifconnect(NULL);

               _za_timer_activate(ZA_TIMER_CHECK_STATUS);

               za_storage_sys_mode_t tSystemMode = za_storage_get_system_mode(NULL);

               if (tSystemMode.tWirelessMode == ZA_STORAGE_WIFI_CLIENT_MODE)
               {
                  ZA_ESP_LOGI(TAG, "Change to Bluetooth Host Mode.");

                  za_command_excute("SystmInr Y WL-BleHost");
                  za_command_excute("SystmInr Y RebootDelay");
               }
               else if (tSystemMode.tWirelessMode == ZA_STORAGE_BLEHOST_WIFICLIENT_MODE)
               {
                  ZA_ESP_LOGI(TAG, "Wifi disconnected, just reboot.");

                  //za_command_excute("CaidengI Y breathxx"); 

                  //za_command_excute("SystmInr Y RebootDelay");
               }
            }
         }
         break;

         case ZA_SYS_EVENT_BLUFI_STATUS_CONNECTED:
         {
            //za_command_excute("CaidengI Y sigblue-0");

            za_innersor_wifi_led_shink_ctrl(ZA_FALSE);
            za_innersor_ble_led_shink_ctrl(ZA_FALSE);

            _za_sys_set_ble_ifconnect(ZA_TRUE);

            _za_sys_set_blewifiall_ifconnect(NULL);

            _za_timer_activate(ZA_TIMER_CHECK_STATUS);
         }
         break;

         case ZA_SYS_EVENT_BLUFI_STATUS_UNCONNECTED:
         {
            //za_command_excute("CaidengI Y random-0"); // 0号灯;

            _za_sys_set_ble_ifconnect(ZA_FALSE);

            _za_sys_set_blewifiall_ifconnect(NULL);

            _za_timer_activate(ZA_TIMER_CHECK_STATUS);
         }
         break;

         case ZA_SYS_EVENT_BLEWIFI_ALL_CONNECTED:
         {
            //za_command_excute("CaidengI Y sigblue-0");
            //za_command_excute("CaidengI Y siggreen-1"); 
            //za_command_excute("CaidengI Y breathto"); // breath on;
         }
         break;

         case ZA_SYS_EVENT_BLE_BLUFI_CUSTOM_DATA_RECV:
         {
            //za_command_excute("CaidengI Y sigbluef-0");
            
            //_za_timer_change_period(ZA_TIMER_CHECK_STATUS, 1);
            _za_timer_activate(ZA_TIMER_CHECK_STATUS);

            ZA_ESP_LOGI(TAG, "SYS RECV MSG: %d - cmd: %s", za_msg_data.za_sysmsg_type, za_msg_data.aDataBuffer);

            za_command_excute((char *)za_msg_data.aDataBuffer);
         }
         break;

         case ZA_SYS_EVENT_CMD_DO_IT_NOW:
         {
            za_command_excute((char *)za_msg_data.aDataBuffer);
         }
         break;

         case ZA_SYS_EVENT_UPDATE_BIND_FLAG_ID:
         {
            ZA_ESP_LOGI(TAG, "SYS RECV MSG: %d - cmd: %s", za_msg_data.za_sysmsg_type, za_msg_data.aDataBuffer);

            za_app_memory_load_bind_flag_id((const ZA_CHAR *)(za_msg_data.aDataBuffer));

            za_storage_sys_mode_t tSysMode = za_storage_get_system_mode(NULL);

            za_strcpy((ZA_CHAR *)(tSysMode.cBindFlagID), (const ZA_CHAR *)za_msg_data.aDataBuffer);

            if (!za_storage_set_system_mode(tSysMode))
            {
               ZA_ESP_LOGI(TAG, " ZA_SYS_EVENT_UPDATE_BIND_FLAG_ID : ERROR. ");
            }

            if (! za_aiot_local_set_server_params(za_aiot_local_get_server_url(ZANULL), 
            za_aiot_local_get_server_uname(ZANULL), 
            za_aiot_local_get_server_upasswd(ZANULL)) )
            {
               ZA_ESP_LOGE(TAG, "ZA_APP_EVENT_AIOTLOCAL_SET_PARAMS ERROR.");
            }

            if (tSysMode.tAiotServerMode != ZA_STORAGE_AIOT_SERV_INTERNET)
            {
               tSysMode.tAiotServerMode = ZA_STORAGE_AIOT_SERV_INTERNET;

               if (!za_storage_set_system_mode(tSysMode))
               {
                  ZA_ESP_LOGI(TAG, " updata flag: ZA_STORAGE_AIOT_SERV_INTERNET : ERROR. ");
               }

               za_command_excute("SystmInr Y RebootDelay");
            }
         }
         break;

         default:
         {
            ;
         }
         break;
      }

      ZA_vTaskDelay(10 / ZA_portTICK_PERIOD_MS); // 15ms for test
   }

   return;
}

/*** public ***/

ZA_BOOL za_app_init()
{
   ZA_ESP_LOGI(TAG, "ZHENGAN APP INIT ENTER");

   za_memset(&zhengan_main_context, 0x00, za_sizeof(za_main_context_t));

   zhengan_main_context.zhengan_sys_context.h_sys_msg = za_msg_queue_create(ZA_SYS_MSG_QUEUE_DEEPTH, za_sizeof(za_sys_msg_data_t));

   zhengan_main_context.zhengan_sys_context.h_sys_SemphStatus = za_semaphore_create(1,1);

   _za_timer_create(ZA_TIMER_CHECK_STATUS, 1000, _za_timer_check_status_callback); // one Seconds Timer
   _za_timer_create(ZA_TIMER_WORKING, 3000, _za_timer_working_callback); // Three Seconds Timer
   _za_timer_create(ZA_TIMER_CHECK_KEY, 50, _za_timer_check_key_callback); // 50 mili-seconds Timer
   _za_timer_create(ZA_TIMER_GAME_CHECK, 10, _za_timer_check_game_callback); // 10 mili-seconds timer
   

   zhengan_main_context.zhengan_app_context.h_app_msg = za_msg_queue_create(ZA_APP_MSG_QUEUE_DEEPTH, za_sizeof(za_app_msg_data_t));

   za_task_create(_za_app_msg_task, "za_appmsg_task", ZA_APP_MSG_TASK_STACKSIZE, NULL, ZA_APP_MSG_TASK_PRIORITY, NULL);
   za_task_create(_za_sys_msg_task, "za_sysmsg_task", ZA_SYS_MSG_TASK_STACKSIZE, NULL, ZA_SYS_MSG_TASK_PRIORITY, NULL);

   za_command_excute("SystmInr Y PowerUpdate");

   return ZA_TRUE;
}

ZA_BOOL za_app_longmore_pre()
{
   /* 设定灯的初始化指示状态 */

   za_command_excute("CaidengI Y briset 28"); // open light && set , briset include brion
   //za_command_excute("CaidengI Y random-0"); // 0号灯 插脚灯
   //za_command_excute("CaidengI Y random-1"); // 1号灯 开关灯
   za_command_excute("CaidengI Y firestatus"); // 1号灯 开关灯


   return ZA_TRUE;
}

ZA_QueueHandle_t za_get_sys_msg_handle(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   return zhengan_main_context.zhengan_sys_context.h_sys_msg;
}

ZA_QueueHandle_t za_get_app_msg_handle(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   return zhengan_main_context.zhengan_app_context.h_app_msg;
}

ZA_VOID za_status_start(ZA_VOID *arg)
{
   (ZA_VOID) arg;

/*
   za_command_excute("MotorInr Y ever");

   za_command_excute("BuzzIner Y notec-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
   za_command_excute("BuzzIner Y notecs-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
   za_command_excute("BuzzIner Y noted-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
   za_command_excute("BuzzIner Y noteeb-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
   za_command_excute("BuzzIner Y notee-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
   za_command_excute("BuzzIner Y notef-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
   za_command_excute("BuzzIner Y notefs-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
   za_command_excute("BuzzIner Y noteg-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
   za_command_excute("BuzzIner Y notegs-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
   za_command_excute("BuzzIner Y notea-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
   za_command_excute("BuzzIner Y notebb-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
   za_command_excute("BuzzIner Y noteb-5");
   ZA_vTaskDelay( 10 / ZA_portTICK_PERIOD_MS );
*/

   return;
}

ZA_VOID za_status_end(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   //za_command_excute("MotorInr Y stop");
   //za_command_excute("BuzzIner Y stop");

   return;
}

wifi_config_t za_app_get_current_wifi(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   wifi_config_t tWifiConfigNow;

   esp_wifi_get_config(WIFI_IF_STA, &tWifiConfigNow);

   return tWifiConfigNow;
}

ZA_UINT32 za_app_get_current_battery_percent(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   return nBatteryPercent;
}


//--------------------------------------- out module init call when init on zasmart

ZA_VOID za_app_out_module_init(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   za_storage_sys_mode_t tSystemMode = za_storage_get_system_mode(NULL);

   if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_ZHENDONG_SENSOR)
   {
      ZA_ESP_LOGI(TAG, "Shake out device module INITIALIZED.");

      //za_command_excute("SystmInr Y ShakeInit"); // for test
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_RENTIGANYING_SENSOR)
   {
      ZA_ESP_LOGI(TAG, "Hongwai Renti ganying out device module INITIALIZED.");

      //za_command_excute("SystmInr Y BodysenInit"); // for test
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_HONGWAIDUIGUAN_SENSOR)
   {
      ZA_ESP_LOGI(TAG, "Hongwai duiguan ganying out device module INITIALIZED.");

      //za_command_excute("SystmInr Y IrpartyInit"); // for test
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_LIUZHOUTUOLUOYI_SENSOR)
   {
      ZA_ESP_LOGI(TAG, "liuzhou tuoluoyi out device module INITIALIZED.");

      //za_command_excute("SystmInr Y MPU6050Init"); // for test
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_DIANZILUOPAN_SENSOR)
   {
      ZA_ESP_LOGI(TAG, "dian zi luo pan out device module INITIALIZED.");

      //za_command_excute("SystmInr Y CompassInit"); // for test
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_SERVO_DRIVER)
   {
      ZA_ESP_LOGI(TAG, "servo driver module INITIALIZED.");
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_ROBOT_T1)
   {
      ZA_ESP_LOGI(TAG, "Robot T1 Now INITIALIZED.");
   }
   else if (tSystemMode.tOutmoduleType == ZA_STORAGE_OUT_MARK_ACTION)
   {
      ZA_ESP_LOGI(TAG, "Action Mark module INITIALIZED.");

      //za_command_excute("SystmInr Y ActionInit");
   }
   
   return;
}

// nTimeInterval : ms , if < 10ms just be 10ms,
ZA_VOID za_app_game_set_datacheck_time_interval(ZA_UINT32 nTimeInterval)
{
   ZA_UINT32 nIntervalReal = nTimeInterval / 10; // count = nTimeInterval /  10 ms = n

   nGameCallbackTimeCount = nIntervalReal;

   return;
}

ZA_VOID za_app_game_set_magic_box_msg_report(ZA_CHAR *pcMsgReport)
{
   memset(MagicBoxMsgBuffer, 0, sizeof(ZA_CHAR)*ZHENGAN_APP_MAG_DATABUF_LEN);
   za_sprintf(MagicBoxMsgBuffer, pcMsgReport);

   return;
}

ZA_CHAR * za_app_game_get_magic_box_msg_report(ZA_VOID *arg)
{
   (ZA_VOID) arg;

   return MagicBoxMsgBuffer;
}
