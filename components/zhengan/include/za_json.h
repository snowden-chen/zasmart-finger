/* za_json.h , Json process for all components with some certain datas. */

/* Version : V0.1 */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_JSON_H
#define ZA_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "za_aiot.h"

ZA_BOOL za_json_init();
ZA_BOOL za_json_show_version();

// pack

ZA_BOOL za_json_start_to_pack(ZA_VOID *pvParameter);
ZA_CHAR * za_json_pack_format(ZA_VOID *pvParameter);
ZA_CHAR * za_json_pack_unformat(ZA_VOID *pvParameter);
ZA_BOOL za_json_end_to_pack(ZA_VOID *pvParameter);

ZA_BOOL za_json_pack_system_info(ZA_VOID *pvParameter);
ZA_BOOL za_json_pack_oid_event(ZA_CHAR * strCode);

ZA_BOOL za_json_pack_udp_data_event(ZA_CHAR * strCodeTopic, ZA_CHAR  * strCodePayload);
ZA_BOOL za_json_pack_pushmain_message(ZA_CHAR  * strTag, ZA_CHAR * strParams);
ZA_BOOL za_json_pack_pushmain_command(ZA_CHAR  * strTag, ZA_CHAR * strParams);

// unpack

ZA_BOOL za_json_start_to_unpack(const ZA_CHAR *UnpackStr);
ZA_BOOL za_json_end_to_unpack(ZA_VOID *pvParameter);

za_aiot_mainservice_type_t za_json_unpack_mainservice_type(ZA_VOID *pvParameter);
ZA_CHAR * za_json_unpack_mainservice_download_cmd(ZA_VOID *pvParameter);
ZA_CHAR * za_json_unpack_bind_flag_id(ZA_VOID *pvParameter);
ZA_CHAR * za_json_unpack_localserver_ip(ZA_VOID *pvParameter);


#ifdef __cplusplus
}
#endif

#endif


