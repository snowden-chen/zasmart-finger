/*  alternation command - core of zasmart . */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"
#include "za_command.h"

#include "za_wifi.h"
#include "za_info.h"
#include "za_upgrade.h"
#include "za_system.h"

#include "zhengan.h"

static const char *TAG = "ZA_COMMAND";

typedef struct
{
    za_command_mode_t cmd;          /* 指令序号 */
    ZA_CHAR *str;                   /* 指令内容 */
    pFunc  cb;                      /* 指令执行 */
}za_command_func_t;

/* 交互命令指令表 */
const za_command_func_t za_cmd_func[] = {
    {ZA_CMD_START,        NULL,                NULL},
    
    {ZA_CMD_TEST,         "zasmartt",          za_cmd_test},
    {ZA_CMD_SYSTMINR,     "SystmInr",          za_cmd_systminr},
    {ZA_CMD_UPGRADEI,     "UpgradeI",          za_cmd_upgradei},
    
    {ZA_CMD_GAMECTRL,     "gamectrl",          za_cmd_gamectrl},

    {ZA_CMD_POWEROFF,     "poweroff",          za_cmd_poweroff},
    {ZA_CMD_REBOOT,       "reboot",            za_cmd_reboot},
    {ZA_CMD_IFCONFIG,     "ifconfig",          za_cmd_ifconfig},
    {ZA_CMD_UPDATE,       "update",            za_cmd_update},
    {ZA_CMD_UPGRADE,      "upgrade",           za_cmd_upgrade},
    {ZA_CMD_FACTORY,      "factory",           za_cmd_factory},
    {ZA_CMD_DEFAULT,      "default",           za_cmd_default},
    {ZA_CMD_UPDEBUG,      "updebug",           za_cmd_updebug},

    {ZA_CMD_END,          NULL,                NULL}
};

unsigned int za_mstrlen(const char *s)
{
    const char *ss = s;
    
    while (*ss)
        ss++;
    
    return ss - s;
}

int za_mstrncmp(const char *s1, const char *s2, int n)
{
    const unsigned char *c1 = (const unsigned char *)s1;
    const unsigned char *c2 = (const unsigned char *)s2;
    unsigned char ch;
    int d = 0;

    while (n--) {
        d = (int)(ch = *c1++) - (int)*c2++;
        if (d || !ch)
            break;
    }

    return d;
}

/* 查找指令表中对应的指令 */
static ZA_UCHAR za_cmd_search(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_UCHAR ret = 0;
    ZA_UCHAR i, n;
    
    for(i=1; za_cmd_func[i].cmd != ZA_CMD_END; i++)
    {    
        n = za_mstrlen(za_cmd_func[i].str);

        if (n > len)
        {
            n = len;
        }

        if(!za_mstrncmp(p, za_cmd_func[i].str, n))
        {
            ret = i;
            break;
        }
    }
    
    return ret;
}

static ZA_BOOL za_cmd_ifvaild(ZA_CHAR *p)
{
    ZA_BOOL bIfVaild = ZA_FALSE;
    ZA_UCHAR index = 0;

    index = za_cmd_search(p, 8);

    if ((index > 0) && (index < ZA_CMD_END) && (za_cmd_func[index].cmd < ZA_CMD_END))
    {
        bIfVaild = ZA_TRUE;
    }

    return bIfVaild;
}

/* 返回值参数内容如上， p-指向解析的指令，len-解析指令长度 */
/* 指令解析 */
static ZA_UCHAR za_cmd_parse(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_UCHAR ret = ZA_CMD_SUCCESS;
    ZA_UCHAR index = 0;
    
    if(len < 4)
    {
        return ZA_CMD_ERR; /* 不符合指令最小长度 */
    }

    if ((len >8)&&(len <= 12)) /* " Y x..." or " y x..."  */
    {
        return ZA_CMD_ERR_UNINVAIL; 
    }
    
    /*xxx\r\n*/
    //if((p[0] == 'A') && (p[1] == 'T') && (p[len-2] == 0x0D) && (p[len-1] == 0x0A))
    if (za_cmd_ifvaild(p))
    {
        /* 执行指令解析 */
        index = za_cmd_search(p, len); /* 查找匹配的执行指令，0-已匹配，!0-未匹配 */
            
        if(index)
        {
            if(za_cmd_func[index].cb != NULL)
            {  
                /* 判断指令对应执行函数是否存在 */
                ZA_UCHAR n;
                n = za_mstrlen(za_cmd_func[index].str);

                if ((*(p+n+1) == 'Y') || (*(p+n+1) == 'y'))
                {
                    ret = za_cmd_func[index].cb(p+n+3, len-n-3); /* 执行对应的指令函数, p+n+3:将指令参数传输执行函数，len-n-3:指令参数有效长度 */
                }
                else if (*(p+n) == '\0')
                {
                    ret = za_cmd_func[index].cb(ZANULL, 0); 
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
    }
    else
    {
        /* 格式不匹配 */
        return ZA_CMD_ERR;
    }
    
    return ret;
}

ZA_VOID za_command_excute(ZA_CHAR * pcSrcParams)
{
    ZA_UCHAR ret;
    ZA_UCHAR m;

    m = za_mstrlen(pcSrcParams);
    ret = za_cmd_parse(pcSrcParams, m);

    if(ret)
    {
        ZA_ESP_LOGI(TAG, "Command ERR! = %d\r\n", ret);
    }

    return;
}

/* 指令执行函数（测试用） */
ZA_UCHAR za_cmd_test(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_ESP_LOGI(TAG, "test OK: %s \r\n", p);

    return ZA_CMD_SUCCESS;
}

/* -----------------------------    Simulate as Linux Commands            -----------------------------*/

ZA_UCHAR za_cmd_poweroff(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    ZA_ESP_LOGI(TAG, "power off - shutdown , cut the power to standy %s \r\n", p);

    za_system_msg_magic_box_report_ontime("关闭电源");

    za_command_excute("SystmInr Y Standby");

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_cmd_reboot(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    ZA_ESP_LOGI(TAG, "reboot the system just now. %s \r\n", p);

    za_system_msg_magic_box_report_ontime("重新启动");

    vTaskDelay( 100 / ZA_portTICK_PERIOD_MS );

    za_command_excute("SystmInr Y Reboot");

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_cmd_ifconfig(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

/*
    ZA_CHAR aMacAddr[30] = {0};
    ZA_CHAR aTopic[150] = {0};
    ZA_CHAR *pTopicName = ZA_NULL;

    za_system_msg_magic_box_report_ontime("小盒的运行信息请在信息窗口中查看");

    pTopicName = aTopic;

    za_info_get_mac_wifi_sta(aMacAddr, 20);
    za_wifi_print_local_ip_addr(ZANULL);
    printf("WiFi Mac Addr : %s \n",aMacAddr);

    printf("WiFi AP SSID : %s \n", (const ZA_CHAR *)(za_app_get_current_wifi(ZANULL).sta.ssid));

    za_info_get_mac_bt(aMacAddr, 20);
    printf("Bluetooth Mac Addr : %s \n",aMacAddr);

    printf("Firmware version : %s \n", (const ZA_CHAR *)(za_upgrade_firmware_ver(ZANULL)));
    printf("Battery : %f \n", (const double)(za_app_get_current_battery_percent(ZANULL)));
*/
    ZA_CHAR aMassage[255] = {0};
    ZA_CHAR aMassageTemp[255] = {0};

    za_sprintf(aMassage, "free: %"PRIu32" , ", esp_get_free_heap_size());
    za_sprintf(aMassageTemp, "min heap size: %"PRIu32" \n", (uint32_t)heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
    strcat(aMassage, aMassageTemp);

    za_system_msg_magic_box_report_ontime(aMassage);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_cmd_update(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    ZA_ESP_LOGI(TAG, "update the firmware just now. %s \r\n", p);

    za_system_msg_magic_box_report_ontime("强制升级");

    za_command_excute("UpgradeI Y Zasmart");

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_cmd_upgrade(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    ZA_ESP_LOGI(TAG, "update the firmware just now with version. %s \r\n", p);

    za_system_msg_magic_box_report_ontime("版本升级");

    za_command_excute("UpgradeI Y Zasmart-ver");

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_cmd_factory(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    ZA_ESP_LOGI(TAG, "Go to factory. %s \r\n", p);

    za_system_msg_magic_box_report_ontime("恢复出厂");

    za_command_excute("UpgradeI Y GoFactory");

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_cmd_default(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    ZA_ESP_LOGI(TAG, "Go to default. %s \r\n", p);

    za_system_msg_magic_box_report_ontime("默认配置");

    za_command_excute("SystmInr Y Wifi-DisConNull");

    za_command_excute("SystmInr Y RebootDelay");

    return ZA_CMD_SUCCESS;
}



ZA_UCHAR za_cmd_updebug(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    ZA_ESP_LOGI(TAG, "update forcly for debug. %s \r\n", p);

    za_system_msg_magic_box_report_ontime("调试升级");

    za_command_excute("UpgradeI Y Debug");

    return ZA_CMD_SUCCESS;
}

