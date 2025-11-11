/* za_command.h , alternation command . */
/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#ifndef ZA_COMMAND_H
#define ZA_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#define ZA_CMD_SUCCESS         (0) /* 指令正常 */
#define ZA_CMD_ERR             (1) /* 指令异常 */
#define ZA_CMD_ERR_UNINVAIL    (2) /* 没有对应指令 */
#define ZA_CMD_ERR_FUN_UNUSED  (3) /* 没有可执行函数 */

typedef enum
{
    ZA_CMD_START = 0,

    ZA_CMD_TEST,             /* 测试指令 */
    ZA_CMD_CAIDENGI,         /* 内在部件 - 彩灯命令系 */
    ZA_CMD_MOTORINR,         /* 内在部件 - 振动命令系*/
    ZA_CMD_BUZZINER,         /* 内在部件 - MIDI命令系*/
    ZA_CMD_SYSTMINR,         /* 内部系统命令系*/
    ZA_CMD_UPGRADEI,         /* 固件升级指令系*/
    ZA_CMD_GAMECTRL,         /* 游戏控制指令系*/

    ZA_CMD_POWEROFF,         /* 关机 - 单命令： poweroff */
    ZA_CMD_REBOOT,           /* 重启 - 单命令： reboot */
    ZA_CMD_IFCONFIG,         /* 查看网络信息 - 单命令： ifconfig */
    ZA_CMD_UPDATE,           /* 启动检查版本的升级 - 单命令： update */
    ZA_CMD_UPGRADE,          /* 强制升级 - 单命令： upgrade */
    ZA_CMD_FACTORY,          /* go to factory - 单命令： factory */
    ZA_CMD_DEFAULT,          /* 恢复默认状态 - 单命令: default */
    ZA_CMD_UPDEBUG,          /* 强制升级调试路径的固件 - 单命令: updebug */

    ZA_CMD_END
}za_command_mode_t;

ZA_BOOL za_command_init();

ZA_VOID za_command_excute(ZA_CHAR * pcSrcParams);

ZA_UCHAR za_cmd_test(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_cmd_systminr(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_cmd_upgradei(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_cmd_gamectrl(ZA_CHAR *p, ZA_UCHAR len);

ZA_UCHAR za_cmd_poweroff(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_cmd_reboot(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_cmd_ifconfig(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_cmd_update(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_cmd_upgrade(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_cmd_factory(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_cmd_default(ZA_CHAR *p, ZA_UCHAR len);
ZA_UCHAR za_cmd_updebug(ZA_CHAR *p, ZA_UCHAR len);

unsigned int za_mstrlen(const char *s);
int za_mstrncmp(const char *s1, const char *s2, int n);

typedef ZA_UCHAR (*pFunc)(ZA_CHAR *ptr, ZA_UCHAR len);

#ifdef __cplusplus
}
#endif

#endif
