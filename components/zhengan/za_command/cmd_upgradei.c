#include "za_common.h"
#include "za_command.h"

#include "cmd_common.h"

static const char *TAG = "cmd_upgradei";

typedef struct
{
    za_upgradei_mode_t cmd;             /* 指令序号 */
    ZA_CHAR *str;                       /* 指令内容 */
    pFunc  cb;                          /* 指令执行 */
}za_upgradei_func_t;

/* 交互命令指令表 */
const za_upgradei_func_t za_cmd_upgrade_func[] = {
    {ZA_UPGRADE_NO_OPERATE,                               NULL,                   NULL},

    {ZA_UPGRADE_ZASMART_FORCE,                            "Zasmart",              za_system_Zasmart},
    {ZA_UPGRADE_ZASMART_VERSION,                          "Zasmart-ver",          za_system_ZasmartVer},
    {ZA_UPGRADE_RESTORE_FACTORY,                          "GoFactory",            za_system_GoFactory},
    {ZA_UPGRADE_ZASMART_DEBUG,                            "Debug",                za_system_update_debug},
    {ZA_UPGRADE_ZACHANGE_FIRMWARE,                        "zachange ",            za_system_zachange_force},
    {ZA_UPGRADE_ZASMART_OPENFORCE,                        "updateopen",           za_system_zasmart_open},
    {ZA_UPGRADE_ZASMART_AUTO,                             "setauto",              za_system_update_auto_default},
    {ZA_UPGRADE_ZASMART_AUTO_OPEN,                        "setautoopen",          za_system_update_auto_open},
    {ZA_UPGRADE_ZASMART_MANUAL,                           "setmanual",            za_system_update_manual_default},
    {ZA_UPGRADE_SET_OBJPATH,                              "setpath ",             za_system_update_set_path},
    {ZA_UPGRADE_DO_FROM_OBJPATH,                          "updatebypath",         za_system_update_from_path},
    {ZA_UPGRADE_TYPE_END,                                 NULL,                   NULL}
};

/* 查找指令表中对应的指令 */
static ZA_UCHAR za_cmd_upgrade_search(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_UCHAR ret = 0;
    ZA_UCHAR i, n;
    
    for(i=1; za_cmd_upgrade_func[i].cmd != ZA_UPGRADE_TYPE_END; i++)
    {    
        n = za_mstrlen(za_cmd_upgrade_func[i].str);

        /*  // no for short
        if (n > len)
        {
            n = len;
        }
        */

        if((len == n) 
            && (*((za_cmd_upgrade_func[i].str)+(n-1)) != '-')
            && (*((za_cmd_upgrade_func[i].str)+(n-1)) != ' ')
            && (!za_mstrncmp(p, za_cmd_upgrade_func[i].str, n)))
        {
            ret = i;
            break;
        }

        if((len > n)
            && (*((za_cmd_upgrade_func[i].str)+(n-1)) == ' ')
            && (!za_mstrncmp(p, za_cmd_upgrade_func[i].str, n)))
        {
            ret = i;
            break;
        }
    }
    
    return ret;
}

/* 返回值参数内容如上， p-指向解析的指令，len-解析指令长度 */
/* 指令解析 */
static ZA_UCHAR za_cmd_upgrade_parse(ZA_CHAR *p, ZA_UCHAR len)
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
    index = za_cmd_upgrade_search(p, len); /* 查找匹配的执行指令，0-已匹配，!0-未匹配 */
            
    if(index)
    {
        if(za_cmd_upgrade_func[index].cb != NULL)
        {  
            /* 判断指令对应执行函数是否存在 */
            ZA_UCHAR n;
            n = za_mstrlen(za_cmd_upgrade_func[index].str);

            if (n == len)
            {
                ret = za_cmd_upgrade_func[index].cb(ZANULL, 0);
            }
            else if (len > n)
            {
                ret = za_cmd_upgrade_func[index].cb(p+n, len-n); /* 执行对应的指令函数, p+n:将指令参数传输执行函数，len-n:指令参数有效长度 */
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

ZA_UCHAR za_cmd_upgradei(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_UCHAR ret;
    ZA_UCHAR m;

    m = za_mstrlen(p);

    if (m != len)
    {
        return ZA_CMD_ERR;
    }

    ret = za_cmd_upgrade_parse(p, m);

    if(ret)
    {
        ZA_ESP_LOGE(TAG, "Command ERR! = %d\r\n", ret);
    }
      
    return ret;
}
