/* za_wifi.h , wifi station & wifi mesh , maybe. */

/* Version : V0.1 */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_WIFI_H
#define ZA_WIFI_H

#ifdef __cplusplus
extern "C" {
#endif

ZA_BOOL za_wifi_init();

ZA_VOID za_wifi_disconnect_from_ap_set_null(ZA_VOID *pvParameter);

ZA_VOID za_wifi_print_local_ip_addr(ZA_VOID *pvParameter);

#ifdef __cplusplus
}
#endif

#endif
