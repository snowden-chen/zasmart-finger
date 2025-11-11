/* za_info.h , All system info & auth or others being about system,chip,key etc . */

/* Version : V0.1 */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_INFO_H
#define ZA_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

ZA_BOOL za_info_monitor_init();

ZA_VOID za_info_monitor_sys(ZA_VOID *pvParameter);

ZA_UINT32 za_info_get_free_memory(ZA_VOID *pvParameter);

ZA_VOID za_info_get_mac_wifi_sta(ZA_CHAR *pMacAddr, ZA_UINT32 nLength20more);

ZA_VOID za_info_get_mac_bt(ZA_CHAR *pMacAddr, ZA_UINT32 nLength20more);


#ifdef __cplusplus
}
#endif

#endif