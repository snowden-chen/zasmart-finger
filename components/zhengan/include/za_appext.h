/* za_app.h , Application of zhengan framework to deal all businesses. */

/* Version : V0.1 */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_APP_H
#define ZA_APP_H

#ifdef __cplusplus
extern "C" {
#endif

ZA_BOOL za_app_ext_init();

ZA_BOOL za_app_send_sysinfo_to_serv();
ZA_BOOL za_app_send_oid_event_to_serv(ZA_CHAR *pData);

ZA_BOOL za_app_require_from_serv();
ZA_BOOL za_app_require_mainservice_from_serv();

ZA_BOOL za_app_require_bindresult_from_serv();
ZA_BOOL za_app_send_bind_code_to_serv(const ZA_CHAR *strBindCode);

ZA_BOOL za_app_memory_load_bind_flag_id(const ZA_CHAR *strBindFlagIdNew);
ZA_BOOL za_app_check_bind_flag_id(const ZA_CHAR *strBindFlagIdRev);
ZA_CHAR * za_app_memory_read_bind_flag_id(ZA_VOID *arg);

ZA_CHAR * za_app_udp_package(ZA_CHAR *pTopicName, ZA_CHAR  *pPayloadInput, ZA_UINT32 *pLengthOut);

ZA_BOOL za_app_send_main_message_to_serv(ZA_CHAR  * strTag, ZA_CHAR *pData);
ZA_BOOL za_app_send_main_command_to_serv(ZA_CHAR  * strTag, ZA_CHAR *pData);

#ifdef __cplusplus
}
#endif

#endif