/* za_aiot_udp.h , UDP Local Communication module in AI IoT. */

/* Version : V0.1 */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_AIOT_UDP_H
#define ZA_AIOT_UDP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ZA_AIOT_UDP_TYPE_NO_OPRATE = 0,         // (0)
    ZA_AIOT_UDP_TYPE_INITED,                // Local UDP Have inited, but have not connected successfully
    ZA_AIOT_UDP_TYPE_TERMED,                // Local UDP module have be destroying
    ZA_AIOT_UDP_TYPE_RUNNING,               // Local UDP running , have connected successfully
    ZA_AIOT_UDP_TYPE_DISCONNECTED,          // Local UDP Disconnected, maybe need TERM and Reconnect with INIT func
    ZA_AIOT_UDP_TYPE_END
}za_aiot_udp_type_t;

typedef enum
{
    ZA_AIOT_UDP_EVENT_WAIT = 901000,        // 901000 - (0) 等待服务器答复
    ZA_AIOT_UDP_EVENT_RECONNECT,            //          (1) 重新连接服务器
    ZA_AIOT_UDP_EVENT_END
} za_aiot_udp_msg_type_t;

typedef struct
{
    za_aiot_udp_msg_type_t  za_aiotudp_type;
    ZA_UINT32 n_param1;
    ZA_UINT32 n_param2;
    ZA_UINT32 n_param3;
    ZA_VOID *p_data;                        //指针地址，入队列之前申请，出队列使用完毕后释放，同是四个字节的指针，可以做int用，减少内存分配
    ZA_CHAR cBuffer[ZA_OPEN_CODE_LEN];
} za_aiot_udp_msg_data_t;

ZA_BOOL za_aiot_udp_init(const ZA_CHAR * sUdpServAddr, const ZA_CHAR * sUserName, const ZA_CHAR * sPassWord);

ZA_BOOL za_aiot_udp_term(ZA_VOID *pvParameter);

ZA_BOOL za_aiot_udp_push_data(za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_udp_push_topic_data(za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_udp_pull_data(za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_udp_refuse_data(za_aiot_com_data_t tAiotComData);

#ifdef __cplusplus
}
#endif

#endif