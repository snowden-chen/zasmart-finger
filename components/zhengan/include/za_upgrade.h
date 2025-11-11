/* za_upgrade.h , Used for ZASMART upgrade data , files , ota ... . */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_UPGRADE_H
#define ZA_UPGRADE_H

#ifdef __cplusplus
extern "C" {
#endif

ZA_BOOL za_upgrade_init();

ZA_BOOL za_upgrade_goto_factory(ZA_VOID *pvParameter);

ZA_BOOL za_upgrade_goto_factory_reboot(ZA_VOID *pvParameter);

ZA_BOOL za_upgrade_set_force_reboot();

ZA_BOOL za_upgrade_set_version_reboot();

ZA_BOOL za_upgrade_set_factory_reboot();

ZA_BOOL za_upgrade_set_force_mode();

ZA_BOOL za_upgrade_set_ignore_mode();

ZA_CHAR * za_upgrade_firmware_ver(ZA_VOID *pvParameter);

ZA_UCHAR za_system_Zasmart(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_system_ZasmartVer(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_system_GoFactory(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_system_update_debug(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_system_zachange_force(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_system_zasmart_open(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_system_update_from_path(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_system_update_auto_default(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_system_update_auto_open(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_system_update_manual_default(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_system_update_set_path(ZA_CHAR *p, ZA_UCHAR len);

#ifdef __cplusplus
}
#endif

#endif
