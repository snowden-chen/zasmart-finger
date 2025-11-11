#include "za_common.h"
#include "za_command.h"

#include "cmd_common.h"

static const char *TAG = "cmd_systminr";

typedef struct
{
    za_systminr_mode_t cmd;             /* 指令序号 */
    ZA_CHAR *str;                       /* 指令内容 */
    pFunc  cb;                          /* 指令执行 */
}za_systminr_func_t;

/* 交互命令指令表 */
const za_systminr_func_t za_cmd_system_func[] = {
    {ZA_SYS_INNER_NO_OPERATE,                               NULL,                   NULL},

    {ZA_SYS_INNER_REBOOT_NOW,                               "Reboot",               za_system_Reboot},
    {ZA_SYS_INNER_REBOOT_DELAY,                             "RebootDelay",          za_system_RebootDelay},
    {ZA_SYS_INNER_STANDBY_NOW,                              "Standby",              za_system_Standby},
    {ZA_SYS_INNER_ENERGY_CONSERVATION,                      "PowerEC",              za_system_PowerEC},
    {ZA_SYS_INNER_FULLY_LOADED,                             "PowerFL",              za_system_PowerFL},
    {ZA_SYS_INNER_POWER_STATUS_UPDATE,                      "PowerUpdate",          za_system_powercheck_update},
    {ZA_SYS_INNER_COMMAND_STORE_MODE_START,                 "CMD-StaStore",         za_system_CMD_StaStore},
    {ZA_SYS_INNER_COMMAND_STORE_MODE_STOP,                  "CMD-EndStore",         za_system_CMD_EndStore},
    {ZA_SYS_INNER_COMMAND_BOOTUP_DOING_ON_ORDER,            "CMD-BoOrder",          za_system_CMD_BoOrder},
    {ZA_SYS_INNER_COMMAND_BOOTUP_DOING_ON_CIRCLE,           "CMD-BoCircle",         za_system_CMD_BoCircle},
    {ZA_SYS_INNER_COMMAND_BOOTUP_DOING_ON_RANDOM,           "CMD-BoRandom",         za_system_CMD_BoRandom},
    {ZA_SYS_INNER_COMMAND_BOOTUP_DOING_OFF,                 "CMD-BoOff",            za_system_CMD_BoOff},
    {ZA_SYS_INNER_COMMAND_BOOTUP_GET_TYPE,                  "CMD-BoType",           za_system_CMD_BoType},
    {ZA_SYS_INNER_COMMAND_BOOTUP_DELAY_MS,                  "CMD-Delay ",           za_system_CMD_Delay},
    {ZA_SYS_INNER_AIOT_COM_MODE_INTERNET,                   "AIOT-Internet",        za_system_AIOT_Internet},
    {ZA_SYS_INNER_AIOT_COM_MODE_LOCALSER,                   "AIOT-Localserv",       za_system_AIOT_Localserv},
    {ZA_SYS_INNER_BINDID_SET_FORCE,                         "BindID ",              za_system_BindID},
    {ZA_SYS_INNER_TRY_SET_LOCAL_SERVER,                     "MQTT-Server ",         za_system_MQTT_Server},
    {ZA_SYS_INNER_WDT_SET,                                  "wdtset",               za_innersor_wdt_set},
    {ZA_SYS_INNER_WDT_FEED,                                 "wdtfeed",              za_innersor_wdt_feed},
    {ZA_SYS_WIRELESS_MODE_SET_BLE_HOST,                     "WL-BleHost",           za_system_WL_BleHost},
    {ZA_SYS_WIRELESS_MODE_SET_WIFI_CLIENT,                  "WL-WifiClient",        za_system_WL_WifiClient},
    {ZA_SYS_WIRELESS_MODE_SET_BLE_HOST_WIFI_CLIENT,         "WL-BleH-WifiC",        za_system_WL_BleHostWifiClient},
    {ZA_SYS_WIRELESS_MODE_GET_CURRENT,                      "WL-GetMode",           za_system_WL_GetMode},
    {ZA_SYS_OUTMODULE_SET_ZHENDONG_SENSOR,                  "OM-Zhendong",          za_system_OM_Zhendong},
    {ZA_SYS_OUTMODULE_SET_RENTIGANYING_SENSOR,              "OM-RTGanying",         za_system_OM_RTGanying},
    {ZA_SYS_OUTMODULE_SET_HONGWAIDUIGUAN_SENSOR,            "OM-HWDGGanying",       za_system_OM_HWDGGanying},
    {ZA_SYS_OUTMODULE_SET_LIUZHOUTUOLUOYI_SENSOR,           "OM-Accgyro",           za_system_OM_Accgyro},
    {ZA_SYS_OUTMODULE_SET_DIANZILUOPAN_SENSOR,              "OM-Compass",           za_system_OM_Compass},
    {ZA_SYS_OUTMODULE_SET_SERVO_DRIVER,                     "OM-Servo",             za_system_OM_Servo},
    {ZA_SYS_OUTMODULE_SET_ROBOT_T1,                         "OM-RobotT1",           za_system_OM_RobotT1},
    {ZA_SYS_OUTMODULE_SET_ACTION_BASE,                      "OM-Action",            za_system_OM_Action},
    {ZA_SYS_OUTMODULE_TYPE_GET_CURRENT,                     "OM-GetType",           za_system_OM_GetType},
    {ZA_SYS_ERASE_NVS_LOCALSTORE_PARTH,                     "EraseLS",              za_system_EraseLS},
    {ZA_SYS_ERASE_NVS_LOCALSTORE_SYSMODE_KEY,               "EraseLS-SysMode",      za_system_EraseLS_SysMode},
    {ZA_SYS_WIFI_DISCONNECT_SET_NULL,                       "Wifi-DisConNull",      za_system_Wifi_DisConNull},
    {ZA_SYS_INFO_MONITOR_SYSTEM,                            "Monitor-Sys",          za_system_Monitor_Sys},
    {ZA_SYS_INFO_DEBUG_JSON,                                "JSON",                 za_system_JSON},
    {ZA_SYS_REQUIRE_MACADDR_BLE,                            "GetMac",               za_system_GetMac},
    {ZA_SYS_INNER_DRIVERS_INIT_APDS9960,                    "APDS9960Init",         za_system_APDS9960Init},
    {ZA_SYS_INNER_DRIVERS_INIT_MPU6050,                     "MPU6050Init",          za_system_MPU6050Init},
    {ZA_SYS_OUTER_DRV_SHAKEMODULE_INIT,                     "ShakeInit",            za_system_ShakeInit},
    {ZA_SYS_OUTER_DRV_BODYSENMODULE_INIT,                   "BodysenInit",          za_system_BodysenInit},
    {ZA_SYS_OUTER_DRV_IRPARTY_INIT,                         "IrpartyInit",          za_system_IrpartyInit},
    {ZA_SYS_OUTER_DRV_ACCGYRO_INIT,                         "AccgyroInit",          za_system_AccgyroInit},
    {ZA_SYS_OUTER_DRV_COMPASS_INIT,                         "CompassInit",          za_system_CompassInit},
    {ZA_SYS_OUTER_DRV_MARK_ACTION_INIT,                     "ActionInit",           za_system_ActionInit},
    {ZA_SYS_OUTER_DRV_MARK_VISIBLE_INIT,                    "VisibleInit",          za_system_VisibleInit},
    {ZA_SYS_OUTER_DRV_MARK_RFID_INIT,                       "RFIDInit",             za_system_RFIDInit},

    {ZA_SYS_INNER_TYPE_END,                                 NULL,                   NULL}
};

/* 查找指令表中对应的指令 */
static ZA_UCHAR za_cmd_system_search(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_UCHAR ret = 0;
    ZA_UCHAR i, n;
    
    for(i=1; za_cmd_system_func[i].cmd != ZA_SYS_INNER_TYPE_END; i++)
    {    
        n = za_mstrlen(za_cmd_system_func[i].str);

        /*  // no for short
        if (n > len)
        {
            n = len;
        }
        */

        if((len == n) 
            && (*((za_cmd_system_func[i].str)+(n-1)) != '-')
            && (*((za_cmd_system_func[i].str)+(n-1)) != ' ')
            && (!za_mstrncmp(p, za_cmd_system_func[i].str, n)))
        {
            ret = i;
            break;
        }

        if((len > n)
            && (*((za_cmd_system_func[i].str)+(n-1)) == ' ')
            && (!za_mstrncmp(p, za_cmd_system_func[i].str, n)))
        {
            ret = i;
            break;
        }
    }
    
    return ret;
}

/* 返回值参数内容如上， p-指向解析的指令，len-解析指令长度 */
/* 指令解析 */
static ZA_UCHAR za_cmd_system_parse(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_UCHAR ret = ZA_CMD_SUCCESS;
    ZA_UCHAR index = 0;
    
    if(len < 1)
    {
        return ZA_CMD_ERR; /* 不符合指令最小长度 */
    }

    if (len <= 2) /* " Y [x...]" or " y [x...]"  */
    {
        return ZA_CMD_ERR_UNINVAIL;
    }
    
    /* 执行指令解析 */
    index = za_cmd_system_search(p, len); /* 查找匹配的执行指令，0-已匹配，!0-未匹配 */
            
    if(index)
    {
        if(za_cmd_system_func[index].cb != NULL)
        {  
            /* 判断指令对应执行函数是否存在 */
            ZA_UCHAR n;
            n = za_mstrlen(za_cmd_system_func[index].str);

            if (n == len)
            {
                ret = za_cmd_system_func[index].cb(ZANULL, 0);
            }
            else if (len > n)
            {
                ret = za_cmd_system_func[index].cb(p+n, len-n); /* 执行对应的指令函数, p+n:将指令参数传输执行函数，len-n:指令参数有效长度 */
            }
        }
        else
        {
            ret = ZA_CMD_ERR_FUN_UNUSED; /* 没有可执行函数 */
        }
    }
    else
    {
        ret = ZA_CMD_ERR_UNINVAIL; /* 未找到匹配的指令 */
    }

    return ret;
}

ZA_UCHAR za_cmd_systminr(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_UCHAR ret;
    ZA_UCHAR m;

    m = za_mstrlen(p);

    if (m != len)
    {
        return ZA_CMD_ERR;
    }

    ret = za_cmd_system_parse(p, m);

    if(ret)
    {
        ZA_ESP_LOGE(TAG, "Command ERR! = %d\r\n", ret);
    }
      
    return ret;
}