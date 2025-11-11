/* za_system.h , Inner system everything */
/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_SYSTEM_H
#define ZA_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "za_storage.h"

ZA_BOOL za_system_init();

ZA_UCHAR za_system_Reboot(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_RebootDelay(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_Standby(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_PowerEC(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_PowerFL(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_powercheck_update(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_CMD_StaStore(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_CMD_EndStore(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_CMD_BoOrder(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_CMD_BoCircle(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_CMD_BoRandom(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_CMD_BoOff(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_CMD_BoType(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_CMD_Delay(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_AIOT_Internet(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_AIOT_Localserv(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_BindID(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_MQTT_Server(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_WL_BleHost(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_WL_WifiClient(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_WL_BleHostWifiClient(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_WL_GetMode(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_OM_Zhendong(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_OM_RTGanying(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_OM_HWDGGanying(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_OM_Accgyro(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_OM_Compass(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_OM_Servo(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_OM_RobotT1(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_OM_Action(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_OM_GetType(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_EraseLS(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_EraseLS_SysMode(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_Wifi_DisConNull(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_Monitor_Sys(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_JSON(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_GetMac(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_APDS9960Init(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_MPU6050Init(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_ShakeInit(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_BodysenInit(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_IrpartyInit(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_AccgyroInit(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_CompassInit(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_ActionInit(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_VisibleInit(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_system_RFIDInit(ZA_CHAR *p, ZA_UCHAR len);


/*********************************************************/

ZA_UINT32 za_system_get_time_now(ZA_VOID_PARA *arg);

ZA_VOID za_system_gamemode_set(za_storage_game_ctrl_mode_t eGameCtrlMode);
za_storage_game_ctrl_mode_t za_system_gamemode_get(ZA_VOID_PARA *arg);

ZA_VOID za_system_game_msg_ctrlmode_set(za_storage_game_msg_ctrl_mode_t eGameMsgCtrlMode);
za_storage_game_msg_ctrl_mode_t za_system_game_msg_ctrlmode_get(ZA_VOID_PARA *arg);

ZA_VOID za_system_game_cmd_ctrlmode_set(za_storage_game_cmd_ctrl_mode_t eGameCmdCtrlMode);
za_storage_game_cmd_ctrl_mode_t za_system_game_cmd_ctrlmode_get(ZA_VOID_PARA *arg);

ZA_VOID za_system_game_msgcmd_mode_switch_set(za_storage_game_msg_cmd_mode_t eGameMsgCmdCtrlMode);
za_storage_game_msg_cmd_mode_t za_system_game_msgcmd_mode_switch_get(ZA_VOID_PARA *arg);

ZA_VOID za_system_msg_magic_box_update_report(ZA_VOID_PARA *arg);
ZA_VOID za_system_msg_magic_box_report_ontime(ZA_CHAR *pcMsgReport);


#ifdef __cplusplus
}
#endif

#endif
