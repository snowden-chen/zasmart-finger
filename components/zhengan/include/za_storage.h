/* za_storage.h , nvs and spiffs and others, maybe. */

/* Version : V0.1 */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_STORAGE_H
#define ZA_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_CMD_STORE_SPIFFS_PATH "/spiffs/cmdrecord.txt"
#define STORAGE_AUTO_UPDATE_APP_PATH  "/spiffs/autoupdate.txt"

/** Defines the enumeration of the operational wireless working mode type WIFI Mode or Bluetooth Ble Mode. */
typedef enum
{
	ZA_STORAGE_OPRATE_NULL = 0,						/**< Means This Item is not avalible. */
	ZA_STORAGE_BLE_HOST_MODE,                    	/**< Bluetooth Ble Host Mode : Ble name: ZADIY. */
	ZA_STORAGE_WIFI_CLIENT_MODE,					/**< Wifi Client Mode . */
	ZA_STORAGE_BLEHOST_WIFICLIENT_MODE,				/**< *Ble Host & Wifi Client as the same time.*/
	ZA_STORAGE_WIRELESS_MODE_END                    /**< End. */
} za_storage_wireless_mode_t;

/** Defines the enumeration of the operational upgrade working mode type . */
typedef enum
{
	ZA_STORAGE_UPGRADE_NULL = 0,					/**< Means This Item is not avalible. */
	ZA_STORAGE_UPGRADE_IGNORE_MODE,					/**< ignore upgrade event, go  */
	ZA_STORAGE_UPGRADE_FORCE_MODE,                  /**< to upgrade the firmware forcely. */
	ZA_STORAGE_UPGRADE_VERSION_MODE,				/**< to upgrade the firmware by version . */
	ZA_STORAGE_UPGRADE_FACTORY_MODE,				/**< erease the ota area, change to factory. */
	ZA_STORAGE_UPGRADE_MODE_END                    	/**< End. */
} za_storage_upgrade_mode_t;

/** Defines the enumeration of the operational out devices (module) type . */
typedef enum
{
	ZA_STORAGE_OUT_OPRATE_NULL = 0,						/**< Means This Item is not avalible. */
	ZA_STORAGE_OUT_ZHENDONG_SENSOR,                    	/**< 灵敏的震动传感器器   --- 1 # */
	ZA_STORAGE_OUT_RENTIGANYING_SENSOR,					/**< 人体红外传感器. 每检测一次有1秒间隔 .  --- 2 # */
	ZA_STORAGE_OUT_HONGWAIDUIGUAN_SENSOR,				/**< 红外对管传感器， 上传持续检测到红外反射的时间单位计数*/
	ZA_STORAGE_OUT_LIUZHOUTUOLUOYI_SENSOR,				/**< 六轴陀螺仪， 上传持续检测到的六轴陀螺仪数据 - ACCGYRO Data*/
	ZA_STORAGE_OUT_DIANZILUOPAN_SENSOR,					/**< 电子罗盘， 上传持续检测到的电子罗盘数据 - COMPASS Data*/
	ZA_STORAGE_OUT_SERVO_DRIVER,						/**< 直流减速马达， 接受上位机的控制 - Left / Right --- UP / DOWN --> 电压，时间*/
	ZA_STORAGE_OUT_ROBOT_T1,							/**< 组合式科创机器人 - T1 型 [双马达/语音播报/电子罗盘] */
	ZA_STORAGE_OUT_MARK_ACTION,							/**< 动作感应式数据底座 -> 震动传感器 + 六轴陀螺仪 */
	ZA_STORAGE_OUT_MARK_VISIBLE,						/**< 可见光感应式数据底座 -> 光传感器 / 颜色 + 手势 + 光亮度 + 接近  */
	ZA_STORAGE_OUT_MARK_RFID,							/**< 射频识别式数据底座 -> RFID  */
	ZA_STORAGE_OUT_MARK_DIY,							/**< zasmart diy - zadiy  */
	ZA_STORAGE_OUT_MARK_DIY_LIGHT,						/**< zasmart diy light - zadiy led strip  */
	ZA_STORAGE_OUTMODULE_MODE_END                       /**< End. */
} za_storage_outmodule_type_t;

/** Defines the Aiot Server Mode */
typedef enum
{
	ZA_STORAGE_AIOT_SERVER_NULL = 0,
	ZA_STORAGE_AIOT_SERV_INTERNET,						/**< mqtts internet default*/
	ZA_STORAGE_AIOT_SERV_LOCALSERV,						/**< local mqtt inner net */
	ZA_STORAGE_AIOT_SERVER_MODE_END
} za_storage_aiotserver_type_t;

typedef struct
{
	za_storage_wireless_mode_t tWirelessMode;
	za_storage_upgrade_mode_t tUpgradeMode;			
	za_storage_outmodule_type_t tOutmoduleType;			
	za_storage_aiotserver_type_t tAiotServerMode;		/**< Aiot 服务模式*/
	ZA_UINT32 nFastBootCount;
	ZA_CHAR cBindFlagID[50]; // store the string of bing id from server when binding...
}za_storage_sys_mode_t;

typedef enum
{
	ZA_CMDSTORE_BOOTUP_OPRATE_NULL = 0,						
	ZA_CMDSTORE_BOOTUP_ORDER,         					/**< doing cmd order one by one , but once*/           	
	ZA_CMDSTORE_BOOTUP_CIRCLE,							/**< doing cmd order one by one , circle when ending.*/
	ZA_CMDSTORE_BOOTUP_RANDOM,							/**< doing cmd random .*/
	ZA_CMDSTORE_BOOTUP_OFF,								/**< do not run store cmd when bootup .*/
	ZA_CMDSTORE_BOOTUP_MODE_END                    		/**< End. */
} za_storage_cmdstore_bootup_mode_t;

typedef enum
{
	ZA_LOCALSERV_IP_MODE_NULL = 0,
	ZA_LOCALSERV_IP_MODE_DEFAULT,
	ZA_LOCALSERV_IP_MODE_CERTAIN,
	ZA_LOCALSERV_IP_MODE_END
} za_storage_localserv_ipmode_t;

typedef enum
{
	ZA_STORAGE_GAME_MODE_NULL = 0,

	ZA_STORAGE_GAME_CLOSE_EVENT,
	ZA_STORAGE_GAME_OPEN_EVENT,
	ZA_STORAGE_GAME_MAGIC_BOX_UPDATE_REPORT,

	ZA_STORAGE_GAME_MODE_END
} za_storage_game_ctrl_mode_t;

typedef enum
{
	ZA_STORAGE_GAME_MSG_CTRLMODE_NULL = 0,
	ZA_STORAGE_GAME_MSG_CTRLMODE_JSON,
	ZA_STORAGE_GAME_MSG_CTRLMODE_SIMPLE,
	ZA_STORAGE_GAME_MSG_CTRLMODE_END
} za_storage_game_msg_ctrl_mode_t;

typedef enum
{
	ZA_STORAGE_GAME_CMD_CTRLMODE_NULL = 0,
	ZA_STORAGE_GAME_CMD_CTRLMODE_JSON,
	ZA_STORAGE_GAME_CMD_CTRLMODE_SIMPLE,
	ZA_STORAGE_GAME_CMD_CTRLMODE_END
} za_storage_game_cmd_ctrl_mode_t;

typedef enum
{
	ZA_STORAGE_GAME_MSG_CMD_CTRL_NULL = 0,
	ZA_STORAGE_GAME_MSG_MODE_ON,
	ZA_STORAGE_GAME_CMD_MODE_ON,
	ZA_STORAGE_GAME_MSG_CMD_CTRL_END
} za_storage_game_msg_cmd_mode_t;

typedef struct
{
	za_storage_localserv_ipmode_t tLocalServMode;
   	ZA_CHAR aSetLinkUrl[ZA_SHORT_URL_LEN];
   	ZA_CHAR aSetUName[ZA_USER_LEN];
   	ZA_CHAR aSetUPassWD[ZA_PASSWORD_LEN];
}za_storage_aiot_local_server_t;

/** Defines the enumeration of the cmd store config data. */
typedef struct
{
	za_storage_cmdstore_bootup_mode_t tCmdstoreBtpMode;
	ZA_UINT32 nCmdNumber;
	ZA_CHAR cCmdFilePath[50];
} za_storage_cmdstore_config_t;

ZA_BOOL za_storage_init();

ZA_BOOL za_storage_nvs_init();

ZA_BOOL za_storage_spiffs_init();

ZA_BOOL za_storage_set_system_mode(za_storage_sys_mode_t tSysMode);
za_storage_sys_mode_t za_storage_get_system_mode(ZA_VOID *pvParameter);

ZA_BOOL za_storage_set_local_server(za_storage_aiot_local_server_t tLocalServer);
za_storage_aiot_local_server_t za_storage_get_local_server(ZA_VOID *pvParameter);

ZA_UINT32 za_storage_get_calibra_param1(ZA_VOID *pvParameter);
ZA_UINT32 za_storage_get_calibra_param2(ZA_VOID *pvParameter);

// Storage Path Name Space : like ZA_STORAGE_LOCALSTORAGE_PATH
ZA_BOOL za_storage_erase_path_space(const char* strPathSpace);

// Storage Key Space of Path Name Space : like ZA_STORAGE_SYSTEM_MODE_KEY
ZA_BOOL za_storage_erase_key_space(const char* strPathSpace, const char* strKeySpace);

// ZA_TRUE -- start to store the input cmd. ZA_FALSE -- stop to store the input cmd.
ZA_BOOL za_storage_cmd_store_mode_if_rec(ZA_BOOL bIfCmdStmSet);
ZA_BOOL za_storage_cmd_store_mode_if_play(ZA_BOOL bIfCmdStmGet);

ZA_BOOL za_storage_cmd_store_input_stream(const ZA_CHAR * const pcSrcParams);
ZA_BOOL za_storage_cmd_store_output_stream(ZA_CHAR * pcSrcParams);

ZA_BOOL za_storage_set_cmd_store_mode(za_storage_cmdstore_config_t tCmdStoreConf);
za_storage_cmdstore_config_t za_storage_get_cmd_store_mode(ZA_VOID *pvParameter);

ZA_BOOL za_storage_get_if_cmdstore_output_doing(ZA_VOID *pvParameter);
ZA_BOOL za_storage_get_if_cmdstore_input_doing(ZA_VOID *pvParameter);

ZA_BOOL za_storage_set_autoupdate_path(const ZA_CHAR *pcSrcParams);
ZA_BOOL za_storage_get_autoupdate_path(ZA_CHAR *pcSrcParams);
ZA_BOOL za_storage_delete_file_path(const ZA_CHAR *pcSrcParams);
ZA_BOOL za_storage_if_file_path_exist(const ZA_CHAR *pcSrcParams);

#ifdef __cplusplus
}
#endif

#endif
