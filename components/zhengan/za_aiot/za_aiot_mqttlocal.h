/* za_aiot_mqttlocal.h , MQTT Local Communication module in AI IoT. */

/* Version : V0.1 */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_AIOT_MQTTLOCAL_H
#define ZA_AIOT_MQTTLOCAL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ZA_AIOT_MQTTLOCAL_TYPE_NO_OPRATE = 0,        // (0)
    ZA_AIOT_MQTTLOCAL_TYPE_INITED,               // Local MQTT Have inited, but have not connected successfully
    ZA_AIOT_MQTTLOCAL_TYPE_TERMED,               // Local Mqtt module have be destroying
    ZA_AIOT_MQTTLOCAL_TYPE_RUNNING,              // Local Mqtt running , have connected successfully
    ZA_AIOT_MQTTLOCAL_TYPE_DISCONNECTED,         // Local Mqtt Disconnected, maybe need TERM and Reconnect with INIT func
    ZA_AIOT_MQTTLOCAL_TYPE_END
}za_aiot_mqttlocal_type_t;

ZA_BOOL za_aiot_mqttlocal_init(const ZA_CHAR * sMqttServUrl, const ZA_CHAR * sUserName, const ZA_CHAR * sPassWord);

ZA_BOOL za_aiot_mqttlocal_term(ZA_VOID *pvParameter);

ZA_BOOL za_aiot_mqttlocal_push_data(za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_mqttlocal_push_topic_data(za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_mqttlocal_push_client_data(const ZA_CHAR * sClientIdata, za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_mqttlocal_pull_data(za_aiot_com_data_t tAiotComData);

ZA_BOOL za_aiot_mqttlocal_refuse_data(za_aiot_com_data_t tAiotComData);

#ifdef __cplusplus
}
#endif

#endif