#include "za_common.h"
#include "za_command.h"

#include "cmd_common.h"

static const char *TAG = "cmd_gamectrl";

typedef struct
{
    za_gamectrl_mode_t cmd;             /* 指令序号 */
    ZA_CHAR *str;                       /* 指令内容 */
    pFunc  cb;                          /* 指令执行 */
}za_gamectrl_func_t;

/* 交互命令指令表 */
const za_gamectrl_func_t za_cmd_game_func[] = {
    {ZA_GAME_MODE_NO_OPERATE,                               ZANULL,                               ZANULL},
    
    {ZA_GAME_MODE_CLOSE_EVENT,                              "close-event",                      za_innersor_game_close_event},
    {ZA_GAME_MODE_OPEN_EVENT,                               "open-event",                       za_innersor_game_open_event},

    {ZA_GAME_MODE_CMD_CTRL_SIMPLE_DIS,                      "cmd-ctrl-simple-display",          za_innersor_game_cmd_ctrl_simple_display},
    {ZA_GAME_MODE_CMD_CTRL_JSON_DIS,                        "cmd-ctrl-json-display",            za_innersor_game_cmd_ctrl_json_display},

    {ZA_GAME_MODE_SWITCH_MSG_ON,                            "switch-msg",                       za_innersor_game_msgcmd_ctrl_msg_mode},
    {ZA_GAME_MODE_SWITCH_CMD_ON,                            "switch-cmd",                       za_innersor_game_msgcmd_ctrl_cmd_mode},

    {ZA_GAME_MODE_TYPE_END,           ZANULL,               ZANULL}
};

/* 查找指令表中对应的指令 */
static ZA_UCHAR za_cmd_game_search(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_UCHAR ret = 0;
    ZA_UCHAR i, n;
    
    for(i=1; za_cmd_game_func[i].cmd != ZA_GAME_MODE_TYPE_END; i++)
    {    
        n = za_mstrlen(za_cmd_game_func[i].str);

        /*  // no for short
        if (n > len)
        {
            n = len;
        }
        */

        if((len == n) 
            && (*((za_cmd_game_func[i].str)+(n-1)) != '-')
            && (*((za_cmd_game_func[i].str)+(n-1)) != ' ')
            && (!za_mstrncmp(p, za_cmd_game_func[i].str, n)))
        {
            ret = i;
            break;
        }

        if((len > n)
            && (*((za_cmd_game_func[i].str)+(n-1)) == '-')
            && (!za_mstrncmp(p, za_cmd_game_func[i].str, n)))
        {
            ret = i;
            break;
        }

        if((len > n)
            && (*((za_cmd_game_func[i].str)+(n-1)) == ' ')
            && (!za_mstrncmp(p, za_cmd_game_func[i].str, n)))
        {
            ret = i;
            break;
        }
    }
    
    return ret;
}

/* 返回值参数内容如上， p-指向解析的指令，len-解析指令长度 */
/* 指令解析 */
static ZA_UCHAR za_cmd_game_parse(ZA_CHAR *p, ZA_UCHAR len)
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
    index = za_cmd_game_search(p, len); /* 查找匹配的执行指令，0-已匹配，!0-未匹配 */
            
    if(index)
    {
        if(za_cmd_game_func[index].cb != NULL)
        {  
            /* 判断指令对应执行函数是否存在 */
            ZA_UCHAR n;
            n = za_mstrlen(za_cmd_game_func[index].str);

            if (n == len)
            {
                ret = za_cmd_game_func[index].cb(ZANULL, 0);
            }
            else if (len > n)
            {
                ret = za_cmd_game_func[index].cb(p+n, len-n); /* 执行对应的指令函数, p+n+1:将指令参数传输执行函数，len-n:指令参数有效长度 */
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

ZA_UCHAR za_cmd_gamectrl(ZA_CHAR *p, ZA_UCHAR len)
{
    ZA_UCHAR ret;
    ZA_UCHAR m;

    m = za_mstrlen(p);

    if (m != len)
    {
        return ZA_CMD_ERR;
    }

    ret = za_cmd_game_parse(p, m);

    if(ret)
    {
        ZA_ESP_LOGE(TAG, "Command ERR! = %d\r\n", ret);
    }
      
    return ret;
}
