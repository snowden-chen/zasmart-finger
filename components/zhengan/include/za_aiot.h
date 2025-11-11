/* za_aiot.h , AI IoT to server by wifi or ble mesh use MQTT or others. */

/* Version : V0.1 */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_AIOT_H
#define ZA_AIOT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
   ZA_AIOT_NO_PROTOCOL = 0,
   ZA_AIOT_MQTT_PROTOCOL,
   ZA_AIOT_MQTTS_PROTOCOL,
   ZA_AIOT_SOCKETS_PROTOCOL,
   ZA_AIOT_HTTP_PROTOCOL,
   ZA_AIOT_PROTOCOL_END
} za_aiot_protocol_type_t;

typedef enum
{
   ZA_AIOT_COM_NULL = 0,
   ZA_AIOT_COM_MAIN_SERVICE,
   ZA_AIOT_COM_UPDATE_TOUCH_KEY,
   ZA_AIOT_COM_UPDATE_EVENT,
   ZA_AIOT_COM_UPDATE_SYSINFO,
   ZA_AIOT_COM_BIND_SERVICE,
   ZA_AIOT_COM_UPDATA_BINDCODE,
   ZA_AIOT_COM_MAIN_SERVICE_MESSAGE,
   ZA_AIOT_COM_TBD_DATA, // To be defined data , for debug
   ZA_AIOT_COM_TEST,
   ZA_AIOT_COM_END
} za_aiot_com_type_t;

typedef enum
{
   ZA_AIOT_MAINSERVICE_NULL = 0,
   ZA_AIOT_MAINSERVICE_UPLOAD,
   ZA_AIOT_MAINSERVICE_DOWNLOAD,
   ZA_AIOT_MAINSERVICE_UPGRADE,
   ZA_AIOT_MAINSERVICE_END
}za_aiot_mainservice_type_t;

typedef enum
{
   ZA_AIOT_COMLEVEL_L1 = 0,
   ZA_AIOT_COMLEVEL_L2,
   ZA_AIOT_COMLEVEL_L3,
   ZA_AIOT_COMLEVEL_END
} za_aiot_comlevel_type_t;

typedef enum
{
   ZA_AIOT_REPORT_NULL = 0,
   ZA_AIOT_REPORT_SYSTEM_INFO_ONE_TIME,
   ZA_AIOT_REPORT_LIVE_ONE_TIME,
   ZA_AIOT_REPORT_LIVE_REPEAT,
   ZA_AIOT_REPORT_LIVE_STOP, // stop the reporting
   ZA_AIOT_REPORT_END
} za_aiot_report_type_t;

typedef struct
{
   za_aiot_com_type_t  tZaAiotComType;
   za_aiot_comlevel_type_t tZaComlevel;
   //ZA_UINT32 n_param1;
   //ZA_UINT32 n_param2;
   //ZA_UINT32 n_param3;
   ZA_VOID *p_data;    //指针地址，入队列之前申请，出队列使用完毕后释放，同是四个字节的指针，可以做int用，减少内存分配
   ZA_UINT32 n_length; // 指针中的数据长度，可能的数据处理时使用
} za_aiot_com_data_t;

/*
typedef enum
{
   ZA_AIOT_EVENT_NO_OPERATE = 0,
   ZA_AIOT_EVENT_TERM_TASK,
   ZA_AIOT_EVENT_CONNECTED,
   ZA_AIOT_EVENT_DISCONNECTED,
   ZA_AIOT_EVENT_REPORT,            //report some info to server
   ZA_AIOT_EVENT_SET_CHECKDATA,     //设置需要检查服务器反馈状态的数据点
   ZA_AIOT_EVENT_CHECKDATA,         //提交服务器反馈的某项数据状态
   ZA_AIOT_EVENT_END
} za_aiot_msg_type_t;

typedef struct
{
    za_aiot_msg_type_t  za_aiot_msg_type;
    //ZA_UINT32 n_param1; //备用参数 -- 无效
    //ZA_UINT32 n_param2;
    //ZA_UINT32 n_param3;
    ZA_VOID *p_data;    // 指针地址，入队列之前申请，出队列使用完毕后释放，同是四个字节的指针，可以做int用，减少内存分配
    ZA_UINT32 n_length; // 指针中的数据长度，可能的数据处理时使用
} za_aiot_msg_data_t;
*/

typedef ZA_VOID (* za_aiot_data_callback)(za_aiot_com_type_t tAiotComType, ZA_CHAR *pData, ZA_INT32 nLength);

ZA_BOOL za_aiot_init();

ZA_BOOL za_aiot_init_status(ZA_VOID *pvParameter);

ZA_BOOL za_aiot_term(ZA_VOID *pvParameter);

ZA_BOOL za_aiot_connect_server(za_aiot_protocol_type_t tProtocolType);

ZA_BOOL za_aiot_disconnect_server(ZA_VOID *pvParameter);

ZA_BOOL za_aiot_reconnect_server(ZA_VOID *pvParameter);

ZA_BOOL za_aiot_connect_set_status(ZA_BOOL bIfConnected);

ZA_BOOL za_aiot_connect_status(ZA_VOID *pvParameter);

ZA_BOOL za_aiot_report(za_aiot_report_type_t tReportType);

ZA_BOOL za_aiot_push_data(za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_push_topic_data(za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_push_client_data(const ZA_CHAR * sClientIdata, za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_pull_data(za_aiot_com_data_t tAiotComData);

//ZA_BOOL za_aiot_dealwith(za_aiot_msg_data_t tAiotMsgData);

ZA_BOOL za_aiot_regist_callback(za_aiot_com_type_t callType, za_aiot_data_callback Callback);

ZA_VOID za_aiot_callback_data(za_aiot_com_type_t tAiotComType, ZA_CHAR *pData, ZA_INT32 nLength);

// local aiot communications 
ZA_BOOL za_aiot_local_set_server_params(const ZA_CHAR * sMqttLocalServUrl, const ZA_CHAR * sUserName, const ZA_CHAR * sPassWord);
ZA_BOOL za_aiot_local_connect_server(const ZA_CHAR * sMqttLocalServUrl, const ZA_CHAR * sUserName, const ZA_CHAR * sPassWord);
ZA_BOOL za_aiot_local_disconnect_server(ZA_VOID *pvParameter);
ZA_BOOL za_aiot_local_set_status(ZA_BOOL bIfConnected);
ZA_BOOL za_aiot_local_get_status(ZA_VOID *pvParameter);

ZA_BOOL za_aiot_local_set_server_url(const ZA_CHAR * const pcSrcParams);
ZA_CHAR * za_aiot_local_get_server_url(ZA_VOID *pvParameter);
ZA_BOOL za_aiot_local_set_server_uname(const ZA_CHAR * const pcSrcParams);
ZA_CHAR * za_aiot_local_get_server_uname(ZA_VOID *pvParameter);
ZA_BOOL za_aiot_local_set_server_upasswd(const ZA_CHAR * const pcSrcParams);
ZA_CHAR * za_aiot_local_get_server_upasswd(ZA_VOID *pvParameter);
ZA_BOOL za_aiot_local_set_server_ipv4(const ZA_CHAR * const pcSrcParams);
ZA_CHAR * za_aiot_local_get_server_ipv4(ZA_VOID *pvParameter);
ZA_BOOL za_aiot_local_set_server_port(ZA_UINT16 nServPort);
ZA_UINT16 za_aiot_local_get_server_port(ZA_VOID *pvParameter);

ZA_BOOL za_aiot_local_push_data(za_aiot_com_data_t tAiotComData);

#ifdef __cplusplus
}
#endif

#endif