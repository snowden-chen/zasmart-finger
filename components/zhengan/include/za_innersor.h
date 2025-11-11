/* za_innersor.h , Inner leds, buzz, motor ... , msg && task */
/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_INNERSOR_H
#define ZA_INNERSOR_H

#ifdef __cplusplus
extern "C" {
#endif


ZA_BOOL za_innersor_init();

ZA_UCHAR za_innersor_wdt_set(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_innersor_wdt_feed(ZA_CHAR *p, ZA_UCHAR len);


ZA_UCHAR za_innersor_game_close_event(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_innersor_game_open_event(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_innersor_game_msgcmd_ctrl_msg_mode(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_innersor_game_msgcmd_ctrl_cmd_mode(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_innersor_game_cmd_ctrl_simple_display(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_innersor_game_cmd_ctrl_json_display(ZA_CHAR *p, ZA_UCHAR len);

/*********************** end of cmd func ************************/

ZA_VOID za_inner_LM_GpioAdapterInit( ZA_VOID *pvParameters );
ZA_VOID za_inner_LM_PowerCtrlALL( ZA_BOOL bIfPowerOn );
ZA_BOOL za_inner_LM_PowerKeyCheck( ZA_VOID *pvParameters );
ZA_VOID za_inner_LM_PowerManagerInit( ZA_VOID *pvParameters );

ZA_VOID za_inner_led_wifi_status( ZA_BOOL bIfConnected );
ZA_VOID za_inner_led_bluetooth_status( ZA_BOOL bIfConnected );

ZA_BOOL za_innersor_power_key_check(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_innersor_power_all_ctrl(ZA_BOOL bIfPowerOn);

ZA_UCHAR za_innersor_wifi_led_ctrl(ZA_BOOL bIfOnOFF);
ZA_UCHAR za_innersor_ble_led_ctrl(ZA_BOOL bIfOnOFF);
ZA_UCHAR za_innersor_wifi_led_shink_ctrl(ZA_BOOL bIfOnOFF);
ZA_UCHAR za_innersor_ble_led_shink_ctrl(ZA_BOOL bIfOnOFF);

/*************************************** funcs of function ************************************/

ZA_VOID za_innersor_game_do_msg_magic_box_update_report(ZA_CHAR *pcMsgReport);


#ifdef __cplusplus
}
#endif

#endif
