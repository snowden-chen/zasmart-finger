/* za_aiot_mqtts.h , MQTTS module in AI IoT. */

/* Version : V0.1 */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_AIOT_MQTTS_H
#define ZA_AIOT_MQTTS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ZA_AIOT_MQTTS_TYPE_NO_OPRATE = 0,        // (0)
    ZA_AIOT_MQTTS_TYPE_INITED,               // Have inited, but have not connected successfully
    ZA_AIOT_MQTTS_TYPE_TERMED,               // Mqtts module have be destroying
    ZA_AIOT_MQTTS_TYPE_RUNNING,              // Mqtts running , have connected successfully
    ZA_AIOT_MQTTS_TYPE_DISCONNECTED,         // Mqtts Disconnected, maybe need TERM and Reconnect with INIT func
    ZA_AIOT_MQTTS_TYPE_END
}za_aiot_mqtts_type_t;

ZA_BOOL za_aiot_mqtts_init(const ZA_CHAR * sMqttServUrl);

ZA_BOOL za_aiot_mqtts_term(ZA_VOID *pvParameter);

ZA_BOOL za_aiot_mqtts_reconnect(ZA_VOID *pvParameter);

ZA_BOOL za_aiot_mqtts_push_data(za_aiot_com_data_t tAiotComData);

// put the params in topic to send , and , mac address in data area
ZA_BOOL za_aiot_mqtts_push_topic_data(za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_mqtts_push_client_data(const ZA_CHAR * sClientIdata, za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_mqtts_pull_data(za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_mqtts_refuse_data(za_aiot_com_data_t tAiotComData);

#ifdef __cplusplus
}
#endif

#endif