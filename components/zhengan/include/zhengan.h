/* zhengan.h , zhengan framework base head file. */

/*    Application    */
/* System -- Module Machine */
/*Arch: A - S - M */

/* Version : V0.1 */

/*
   NOTE: Writed By ZachSnowdenChen
*/

#ifndef ZHENGAN_H
#define ZHENGAN_H

#ifdef __cplusplus
extern "C" {
#endif

#define ZHENGAN_APP_MAG_DATABUF_LEN 255

extern const ZA_UINT8 server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const ZA_UINT8 server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

//static const char *ZASMART_URL = "https://ff3nod.zdmedia.com.cn/cza/ZASMART.bin";

typedef enum
{
    ZA_SYS_EVENT_KEY_DOWN = 0,        // (0)
    ZA_SYS_EVENT_KEY_UP,              // (1)
    ZA_SYS_EVENT_KEY_SHORT_DOWN,      // (2) 
    ZA_SYS_EVENT_KEY_SHORT_UP,        // (3)
    ZA_SYS_EVENT_KEY_LONG_DOWN,       // (4)
    ZA_SYS_EVENT_KEY_LONG_UP,         // (5)
    ZA_SYS_EVENT_CHARGE,              // (6)
    ZA_SYS_EVENT_POWER_SWITCH,        // (7)
    ZA_SYS_EVENT_BATTERY_CAPACITY,    // (8)
    ZA_SYS_EVENT_WIFI_CONNECTION,     // (9)
    ZA_SYS_EVENT_INIT_DONE,           // (10)
    ZA_SYS_EVENT_WILL_POWER_OFF,      // (11)
    ZA_SYS_EVENT_WAKEUP,              // (12)
    ZA_SYS_EVENT_PLAYER_STARTED,      // (13)
    ZA_SYS_EVENT_PLAYER_STOPED,       // (14)
    ZA_SYS_EVENT_PLAYER_CANCELED,     // (15)
    ZA_SYS_EVENT_PLAYER_ERROR,        // (16)
    ZA_SYS_EVENT_RECORD_STARTED,      // (17)
    ZA_SYS_EVENT_RECORD_STOPED,       // (18)
	ZA_SYS_EVENT_RECORD_END,	      // (19)
    ZA_SYS_EVENT_OTA_FINISHED,        // (20)
    ZA_SYS_EVENT_OTA_FAILED,          // (21)
    ZA_SYS_EVENT_VOL_WHELL_CHANGED,   // (22)
    ZA_SYS_EVENT_LED_BLINK_FINISHED,  // (23)
    ZA_SYS_EVENT_PLAYER_END,          // (24)
    ZA_SYS_EVENT_OTA_FTFS_FINISHED,   // (25)
    ZA_SYS_EVENT_OTA_FTFS_FAILED,     // (26)
    ZA_SYS_EVENT_POWER_OFF_CHARGE,    // (27)
    ZA_SYS_EVENT_WIFI_MIX_MODE,       // (28)
    ZA_SYS_EVENT_OID_CODE,            // (29)
    ZA_SYS_EVENT_WIFI_CONFIG_END,     // (30)
    ZA_SYS_EVENT_WIFI_CONFIG_START,   // (31)
    ZA_SYS_EVENT_ZHENGAN_APP_CODE,    // (32)
    ZA_SYS_EVENT_EXIST_PASS_THROUGH,  // (33)
    ZA_SYS_EVENT_INFO_PASS_THROUGH,   // (34)
    ZA_SYS_EVENT_TEST_MODE,           // 35
    ZA_SYS_EVENT_WIFI_CONFIG_SSID,    // 36
    ZA_SYS_EVENT_WIFI_CONFIG_PASSWD,  // 37
    ZA_SYS_EVENT_WIFI_CONNECT_TO_AP,  // 38
    ZA_SYS_EVENT_WIFI_REQ_DISCONNECT_TO_AP, // 39
    ZA_SYS_EVENT_WIFI_REQ_DISCONNECT_AND_SET_NULL, //40
    ZA_SYS_EVENT_WIFI_SET_OP_MODE, // 41
    ZA_SYS_EVENT_WIFI_STATUS_CONNECTED, // 42
    ZA_SYS_EVENT_WIFI_STATUS_UNCONNECTED, // 43
    ZA_SYS_EVENT_BLUFI_STATUS_CONNECTED, // 44
    ZA_SYS_EVENT_BLUFI_STATUS_UNCONNECTED, // 45
    ZA_SYS_EVENT_BLEWIFI_ALL_CONNECTED, // 46
    ZA_SYS_EVENT_BLE_BLUFI_CUSTOM_DATA_RECV, // 47
    ZA_SYS_EVENT_BLE_BLUFI_CUSTOM_DATA_SEND, // 48
    ZA_SYS_EVENT_CMD_DO_IT_NOW, // 49
    ZA_SYS_EVENT_UPDATE_BIND_FLAG_ID,           //存储更新模块与上位机的绑定标识ID
    ZA_SYS_EVENT_END
} za_sys_msg_type_t;

typedef struct
{
    za_sys_msg_type_t  za_sysmsg_type;
    ZA_UINT32 n_param1;
    ZA_UINT32 n_param2;
    ZA_UINT32 n_param3;
    ZA_VOID *p_data;    //指针地址，入队列之前申请，出队列使用完毕后释放，同是四个字节的指针，可以做int用，减少内存分配
    ZA_UINT8 aDataBuffer[ZA_SYS_DATA_BUF_LEN];
} za_sys_msg_data_t;

typedef enum
{
    ZA_APP_EVENT_BIND = 1000,               // (0) 绑定消息
    ZA_APP_EVENT_INFORM,                    // (1) 上报消息
    ZA_APP_EVENT_HEATRBEAT,                 // (2) 自动心跳上报消息 收到该消息是 向服务器上报心跳
    ZA_APP_EVENT_ATTESTATION,               // (3) 用于由于网络原因认证失败的，重试消息
    ZA_APP_EVENT_OID_REQUEST,               // (4) 求情bnf文件消息，本地oid查找失败时候，向服务器请求资源路径，然后下载
    ZA_APP_EVENT_UPDATE_TOKEN,              // (5) 自动更新token消息 收到该消息是 向服务器请求token
    ZA_APP_EVENT_SYSTEM_UPDATE,             // (6) 自动检查是否有升级消息 收到该消息是 向服务器请求升级信息
    ZA_APP_EVENT_WIFI_CONFIGED,             // (7) WIFI配置完成消息
    ZA_APP_EVENT_UP_DOWN_LOAD_START,        // (8) bnf 启动文件下载
    ZA_APP_EVENT_UP_DOWN_LOAD_OVER,         // (9) bnf 文件下载完成
    ZA_APP_EVENT_UP_DOWN_LOAD_EXIT,         // (10) bnf 外部回收下载线程资源
    ZA_APP_EVENT_UP_DOWN_LOAD_FAILED,       // (11) bnf 文件下载失败
    ZA_APP_EVENT_UP_DOWN_LOAD_CANCEL,       // (12) bnf 文件下载被取消
    ZA_APP_EVENT_WECHAT_MSG_REQUEST,        // (13) 请求微信信息
    ZA_APP_EVENT_MESSAGE_UPLOAD,            // (14) 发送数据到微信用户
    ZA_APP_EVENT_WECHAT_CMD_REQUEST,        // (15) 请求微信信息
    ZA_APP_EVENT_HEATRBEAT_EVENT,           // (16) 检查本地是否有心跳交换的命令没有执行
    ZA_APP_EVENT_SYSTEM_CONFIG,             // (17) 执行系统配置请求
    ZA_APP_EVENT_UPLOAD_RESOURCE,           // (18) 上传本地资源文件
    ZA_APP_EVENT_HEARTBEAT_WORK,            // (19) 重启心跳线程
    ZA_APP_EVENT_HEATRBEAT_INFORM,          // (20) 心跳事件
    ZA_APP_EVENT_HEATRBEAT_UPDATE,          // (21) 心跳数据更新，发送到同一线程 安全考虑
    ZA_APP_EVENT_GET_OFFLINE_MSG,           // (22) 获取离线情况下的语音消息
    ZA_APP_EVENT_BATTERY_CHARGE,            // (23) 充电事件
    ZA_APP_EVENT_BATTERY_NORMAL,            // 增：电流正常
    ZA_APP_EVENT_BATTERY_LOW_POWER,         // (24) 电量不足事件
    ZA_APP_EVENT_BATTERY_TOO_LOW_POWER,     // (25) 电量不足事件
    ZA_APP_EVENT_BATTERY_LOW_TO_STANDBY,    // 电量太低需要立刻关机
    ZA_APP_EVENT_OTA_CMD,                   // (26) bnf 外部回收下载线程资源
    ZA_APP_EVENT_OTA_SUCCESS,               // (27) bnf 外部回收下载线程资源
    ZA_APP_EVENT_OTA_FAILED,                // (28) bnf 文件下载失败
    ZA_APP_EVENT_OTA_CANCEL,                // (29) bnf 文件下载失败
    ZA_APP_EVENT_INFORM_INFO,               // (30) 在power on之后， 上报智能笔信息
    ZA_APP_EVENT_NET_DETECT,                // (31) 探测业务网络
    ZA_APP_EVENT_PLAY_STRING,               // (32) 语音播放字符串
    ZA_APP_EVENT_VOICE_UPDATE,              // (33) 提示音升级请求
    ZA_APP_EVENT_ZIP_VOICE_OVER,            // (34)压缩完成
	ZA_APP_EVENT_ZIP_VOICE_FAILURE,		    // (35)压缩完成
    ZA_APP_EVENT_DOWN_LOAD_OVERFLOW,        // (36) sdcard卡溢出
    ZA_APP_EVENT_LOOP_PLAYBACK,             // (37) 循环播放
	ZA_APP_EVENT_BLE_STATUS,                // (38) BLE 状态相关
	ZA_APP_EVENT_RECORD_ONLINE_SUSSESS,	    // (39) 在线录制成功
	ZA_APP_EVENT_RECORD_ONLINE_FAILURE,	    // (40) 在线录制失败
	ZA_APP_EVENT_NET_DETECT_AI,			    // (41) 探测AI网络
    ZA_APP_EVENT_DO_COMMAND_TO_LONGMORE,    //（42） 执行LONGMORE层命令
    ZA_APP_EVENT_DO_CONNECT_MQTTS,          // 43
    ZA_APP_EVENT_DO_CONNECT_MQTT,           // 44
    ZA_APP_EVENT_AIOT_CONNECTED,            // 45, AIOT 通信正常连接时，启动各类设备初始化
    ZA_APP_EVENT_AIOT_DISCONNECTED,         // 46,AIOT 通信出现断开连接状态时，启动重新连接机制
    ZA_APP_EVENT_AIOT_ERROR_COM,            // 47,IOT 通信时出现通信错误，这类错误连续出现多次后，需要触发重启机制
    ZA_APP_EVENT_AIOTLOCAL_SET_PARAMS,      // 48,设置AIOT本地通信的服务器参数
    ZA_APP_EVENT_AIOTLOCAL_DO_CONNECT,      // 49,连接本地MQTT服务器 
    ZA_APP_EVENT_AIOTLOCAL_CONNECTED,       // to local mqtt/mqtts server connected
    ZA_APP_EVENT_AIOTLOCAL_DISCONNECTED,    // diconnected from mqtt/mqtts local server
    ZA_APP_EVENT_LOCAL_COMMAND_STORE_CTRL,  // 本地命令存储控制 - 控制参数缓存在消息BUF中
    ZA_APP_EVENT_LOCAL_COMMAND_DELAY_MS,    // 本地命令延时控制 - 控制参数缓存在消息BUF中    
    ZA_APP_EVENT_AIOT_SEND_SHAKE,           // 向服务端发送SHAKE-震动状态
    ZA_APP_EVENT_AIOT_SEND_INFRARED,        // 向服务端发送INFRARED-红外感应状态 - 感应中
    ZA_APP_EVENT_AIOT_SEND_INFRARED_NEW,    // 向服务端发送INFRARED-红外感应状态 - 感应触发
    ZA_APP_EVENT_AIOT_SEND_INFRARED_END,    // 向服务端发送INFRARED-红外感应状态 - 本轮感应结束
    ZA_APP_EVENT_AIOT_SEND_IRPARTY,         // 向服务端发送IR红外对管持续检测到反射的计数
    ZA_APP_EVENT_AIOT_SEND_ACCGYRO,         // 向服务端发送陀螺仪数据(以固定周期有规律发送)
    ZA_APP_EVENT_AIOT_SEND_COMPASS,         // 向服务端发送电子罗盘数据(以固定周期有规律发送)
    ZA_APP_EVENT_AIOT_SEND_OID,             // 向服务端发送OID隐码数据(以事件触发)
    ZA_APP_EVENT_AIOT_START_BIND,           // start binding to server
    ZA_APP_EVENT_AIOT_SEND_BIND_CODE,       // send binding code to server
    ZA_APP_EVENT_AIOT_REPORT_SYSINFO,
    ZA_APP_EVENT_AIOT_MARK_ACTION_BASE,     // 动作数据底座
    ZA_APP_EVENT_AIOT_SEND_MAIN_COMMAND,    // 向服务端发送命令
    ZA_APP_EVENT_AIOT_SEND_MAIN_MESSAGE,    // 向服务端发送消息
    ZA_APP_EVENT_UPDATE_AFTER_AMOMENT,      // go to update by http after 1.5 second
    ZA_APP_EVENT_TEST_MODE,
    ZA_APP_EVENT_END
} za_app_msg_type_t;

typedef struct
{
    za_app_msg_type_t  za_appmsg_type;
    ZA_UINT32 n_param1;
    ZA_UINT32 n_param2;
    ZA_UINT32 n_param3;
    ZA_VOID *p_data;        //指针地址，入队列之前申请，出队列使用完毕后释放，同是四个字节的指针，可以做int用，减少内存分配
    ZA_CHAR cBuffer[ZHENGAN_APP_MAG_DATABUF_LEN];
    ZA_CHAR cTagBuf[ZHENGAN_APP_MAG_DATABUF_LEN];
} za_app_msg_data_t;

ZA_BOOL za_app_init();
ZA_BOOL za_app_longmore_pre();

ZA_VOID za_sys_msg_proc(ZA_VOID *arg);

ZA_QueueHandle_t za_get_sys_msg_handle(ZA_VOID *arg);
ZA_QueueHandle_t za_get_app_msg_handle(ZA_VOID *arg);

ZA_VOID za_status_start(ZA_VOID *arg);
ZA_VOID za_status_end(ZA_VOID *arg);

wifi_config_t za_app_get_current_wifi(ZA_VOID *arg);
ZA_UINT32 za_app_get_current_battery_percent(ZA_VOID *arg);

ZA_VOID za_app_module_action(ZA_VOID *arg);

ZA_VOID za_app_out_module_init(ZA_VOID *arg);

ZA_VOID za_app_game_set_datacheck_time_interval(ZA_UINT32 nTimeInterval);

ZA_VOID za_app_game_set_magic_box_msg_report(ZA_CHAR *pcMsgReport);
ZA_CHAR * za_app_game_get_magic_box_msg_report(ZA_VOID *arg);

#ifdef __cplusplus
}
#endif

#endif
