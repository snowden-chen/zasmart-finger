/*  Inner leds, buzz, motor ... , msg && task . */

/*
   NOTE: Writed By Zach.Snowden.Chen
*/

#include "za_common.h"
#include "za_command.h"

#include "zhengan.h"

#include "za_innersor.h"
#include "za_storage.h"
#include "za_system.h"

#define ZA_INNERSOR_TASK_STACKSIZE ZA_BUFFER_4K
#define ZA_INNERSOR_TASK_PRIORITY 5

#define ZA_INNER_LEDS_COUNT 2 //the same as LmCaidengInner.h

#define ZA_INNER_CIRCLE_BUFFER_MAX 200   // this value must > 50 or erros 

#define ZA_INNER_POWER_CTRL_GPIO  38
#define ZA_INNER_POWER_KEY_GPIO 41

#define ZA_INNER_STATUS_LED_A_GPIO 4
//#define ZA_INNER_STATUS_LED_B_GPIO 4

#define ZA_INNER_PEN_CTRL_GPIO 10

static const char *TAG = "ZA_INNERSOR";

//typedef ZA_VOID (* za_inner__func_t)(ZA_VOID *pvParameters);

typedef ZA_VOID (* za_inner_lm_wdt_inset_func_t)(ZA_VOID *pvParameters);
typedef ZA_VOID (* za_inner_lm_wdt_dofeed_func_t)(ZA_VOID *pvParameters);


typedef ZA_VOID (* za_inner_LM_GpioAdapterInit_func_t)( ZA_VOID *pvParameters );
typedef ZA_VOID (* za_inner_LM_PowerManagerInit_func_t)( ZA_VOID *pvParameters );

typedef ZA_BOOL (* za_inner_LM_PowerKeyCheck_func_t)( ZA_VOID *pvParameters );
typedef ZA_VOID (* za_inner_LM_PowerCtrlALL_func_t)( ZA_BOOL bIfPowerOn );

typedef ZA_VOID (* za_inner_led_wifi_status_func_t)( ZA_BOOL bIfConnected );
typedef ZA_VOID (* za_inner_led_bluetooth_status_func_t)( ZA_BOOL bIfConnected );


typedef struct
{
    ZA_SemaphoreHandle_t h_innersor_wdt_semph;

    za_inner_lm_wdt_inset_func_t                                lm_wdt_inset;
    za_inner_lm_wdt_dofeed_func_t                               lm_wdt_dofeed;

    za_inner_LM_GpioAdapterInit_func_t                          LM_GpioAdapterInit;
    za_inner_LM_PowerManagerInit_func_t                         LM_PowerManagerInit;
    za_inner_LM_PowerKeyCheck_func_t                            LM_PowerKeyCheck;
    za_inner_LM_PowerCtrlALL_func_t                             LM_PowerCtrlALL;

    za_inner_led_wifi_status_func_t                             led_wifi_status;
    za_inner_led_bluetooth_status_func_t                        led_bluetooth_status;

} za_innersor_context_t;


static za_innersor_context_t za_innersor_context;

/*
static ZA_VOID _za_innersor_task(ZA_VOID_PARA *arg)
{
    (ZA_VOID) arg;

    ZA_BOOL innerproc_run_flag = ZA_TRUE;

    while (innerproc_run_flag)
    {
        ZA_vTaskDelay(10 / ZA_portTICK_PERIOD_MS); // 10ms for test
    }

    return;
}*/

ZA_BOOL za_innersor_init()
{
    za_memset(&za_innersor_context, 0x00, za_sizeof(za_innersor_context_t));

    za_innersor_context.h_innersor_wdt_semph = za_semaphore_create(1,1);

    za_innersor_context.lm_wdt_inset = ZA_NULL;
    za_innersor_context.lm_wdt_dofeed = ZA_NULL;

    za_innersor_context.LM_GpioAdapterInit = za_inner_LM_GpioAdapterInit;
    za_innersor_context.LM_PowerCtrlALL = za_inner_LM_PowerCtrlALL;
    za_innersor_context.LM_PowerKeyCheck = za_inner_LM_PowerKeyCheck;
    za_innersor_context.LM_PowerManagerInit = za_inner_LM_PowerManagerInit;
    za_innersor_context.led_wifi_status = za_inner_led_wifi_status;
    za_innersor_context.led_bluetooth_status = za_inner_led_bluetooth_status;
    
    if (za_innersor_context.LM_GpioAdapterInit != ZA_NULL)
    {
        za_innersor_context.LM_GpioAdapterInit(ZANULL);
    }

    // for power on
    if (za_innersor_context.LM_PowerManagerInit != ZA_NULL)
    {
        za_innersor_context.LM_PowerManagerInit(ZANULL);
    }

    //za_task_create(_za_innersor_task, "za_innersor_task", ZA_INNERSOR_TASK_STACKSIZE, NULL, ZA_INNERSOR_TASK_PRIORITY, NULL);

    return ZA_TRUE;
}


ZA_UCHAR za_innersor_wdt_set(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    za_semaphore_request(za_innersor_context.h_innersor_wdt_semph, ZA_portMAX_DELAY);

    if (za_innersor_context.lm_wdt_inset != ZA_NULL)
    {
        za_innersor_context.lm_wdt_inset(ZANULL);
    }

    za_semaphore_release(za_innersor_context.h_innersor_wdt_semph);

    return ZA_CMD_SUCCESS;
}


ZA_UCHAR za_innersor_wdt_feed(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    za_semaphore_request(za_innersor_context.h_innersor_wdt_semph, ZA_portMAX_DELAY);

    if (za_innersor_context.lm_wdt_dofeed != ZA_NULL)
    {
        za_innersor_context.lm_wdt_dofeed(ZANULL);
    }

    za_semaphore_release(za_innersor_context.h_innersor_wdt_semph);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_innersor_game_close_event(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    za_system_gamemode_set(ZA_STORAGE_GAME_CLOSE_EVENT);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_innersor_game_open_event(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    za_system_gamemode_set(ZA_STORAGE_GAME_OPEN_EVENT);

    return ZA_CMD_SUCCESS;
}


ZA_UCHAR za_innersor_game_msgcmd_ctrl_msg_mode(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    za_system_game_msgcmd_mode_switch_set(ZA_STORAGE_GAME_MSG_MODE_ON);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_innersor_game_msgcmd_ctrl_cmd_mode(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    za_system_game_msgcmd_mode_switch_set(ZA_STORAGE_GAME_CMD_MODE_ON);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_innersor_game_cmd_ctrl_simple_display(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    za_system_game_msgcmd_mode_switch_set(ZA_STORAGE_GAME_CMD_MODE_ON);

    za_system_game_cmd_ctrlmode_set(ZA_STORAGE_GAME_CMD_CTRLMODE_SIMPLE);

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_innersor_game_cmd_ctrl_json_display(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    za_system_game_msgcmd_mode_switch_set(ZA_STORAGE_GAME_CMD_MODE_ON);

    za_system_game_msg_ctrlmode_set(ZA_STORAGE_GAME_CMD_CTRLMODE_JSON);

    return ZA_CMD_SUCCESS;
}


/*********************** end of cmd func ************************/
ZA_VOID za_inner_LM_GpioAdapterInit( ZA_VOID *pvParameters )
{
    (ZA_VOID) pvParameters;

    gpio_reset_pin(ZA_INNER_POWER_CTRL_GPIO);
    gpio_reset_pin(ZA_INNER_POWER_KEY_GPIO);
    gpio_reset_pin(ZA_INNER_STATUS_LED_A_GPIO);
    gpio_reset_pin(ZA_INNER_PEN_CTRL_GPIO);

    gpio_set_direction(ZA_INNER_POWER_CTRL_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(ZA_INNER_STATUS_LED_A_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(ZA_INNER_PEN_CTRL_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_direction(ZA_INNER_POWER_KEY_GPIO, GPIO_MODE_INPUT);


    return;
}

ZA_VOID za_inner_LM_PowerCtrlALL( ZA_BOOL bIfPowerOn )
{
    if (bIfPowerOn)
    {
        gpio_set_level(ZA_INNER_POWER_CTRL_GPIO, 1);
    }
    else
    {
        gpio_set_level(ZA_INNER_POWER_CTRL_GPIO, 0);
    }

    return;
}

ZA_BOOL za_inner_LM_PowerKeyCheck( ZA_VOID *pvParameters )
{
    (ZA_VOID) pvParameters;

    ZA_BOOL bRet = ZA_FALSE;

    bRet = gpio_get_level(ZA_INNER_POWER_KEY_GPIO) < 1 ? ZA_TRUE : ZA_FALSE;

    return bRet;
}

ZA_VOID za_inner_LM_PowerManagerInit( ZA_VOID *pvParameters )
{
    (ZA_VOID) pvParameters;

    gpio_set_level(ZA_INNER_POWER_CTRL_GPIO, 1);
    gpio_set_level(ZA_INNER_PEN_CTRL_GPIO, 0);

    return;
}

static ZA_BOOL sbWifiLedShinkCtrl = ZA_TRUE;
static ZA_BOOL sbIfWifiShink = ZA_FALSE;
ZA_VOID za_inner_led_wifi_status( ZA_BOOL bIfConnected )
{
    if (bIfConnected)
    {
        if (sbWifiLedShinkCtrl)
        {
            gpio_set_level(ZA_INNER_STATUS_LED_A_GPIO, (sbIfWifiShink ? 0 : 1));
            sbIfWifiShink = ! sbIfWifiShink;
        }
        else
        {
            gpio_set_level(ZA_INNER_STATUS_LED_A_GPIO, 1);
        }
    }
    else
    {   
        if (sbWifiLedShinkCtrl)
        {
            gpio_set_level(ZA_INNER_STATUS_LED_A_GPIO, (sbIfWifiShink ? 0 : 1));
            sbIfWifiShink = ! sbIfWifiShink;
        }
        else
        {
            gpio_set_level(ZA_INNER_STATUS_LED_A_GPIO, 0);
        }
    }

    return;
}

static ZA_BOOL sbBleLedShinkCtrl = ZA_TRUE;
static ZA_BOOL sbIfBluetoothShink = ZA_FALSE;
ZA_VOID za_inner_led_bluetooth_status( ZA_BOOL bIfConnected )
{
    if (bIfConnected)
    {
        if (sbBleLedShinkCtrl)
        {
            //gpio_set_level(ZA_INNER_STATUS_LED_B_GPIO, (sbIfBluetoothShink ? 0 : 1));
            sbIfBluetoothShink = ! sbIfBluetoothShink;
        }
        else
        {
            //gpio_set_level(ZA_INNER_STATUS_LED_B_GPIO, 0);
        }
    }
    else
    {   
        if (sbBleLedShinkCtrl)
        {
            //gpio_set_level(ZA_INNER_STATUS_LED_B_GPIO, (sbIfBluetoothShink ? 0 : 1));
            sbIfBluetoothShink = ! sbIfBluetoothShink;
        }
        else
        {
            //gpio_set_level(ZA_INNER_STATUS_LED_B_GPIO, 1);
        }
    }

    return;
}





/****************************************************************/


ZA_BOOL za_innersor_power_key_check(ZA_CHAR *p, ZA_UCHAR len)
{
    (ZA_VOID) p;

    if (za_innersor_context.LM_PowerKeyCheck != ZA_NULL)
    {
        return (za_innersor_context.LM_PowerKeyCheck(ZANULL));
    }

    return ZA_FALSE;
}

ZA_UCHAR za_innersor_power_all_ctrl(ZA_BOOL bIfPowerOn)
{
    if (za_innersor_context.LM_PowerKeyCheck != ZA_NULL)
    {
        za_innersor_context.LM_PowerCtrlALL(bIfPowerOn);
    }

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_innersor_wifi_led_ctrl(ZA_BOOL bIfOnOFF)
{
    if (za_innersor_context.led_wifi_status != ZA_NULL)
    {
        za_innersor_context.led_wifi_status(bIfOnOFF);
    }

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_innersor_ble_led_ctrl(ZA_BOOL bIfOnOFF)
{
    if (za_innersor_context.led_bluetooth_status != ZA_NULL)
    {
        za_innersor_context.led_bluetooth_status(bIfOnOFF);
    }

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_innersor_wifi_led_shink_ctrl(ZA_BOOL bIfOnOFF)
{
    sbWifiLedShinkCtrl = bIfOnOFF ? ZA_TRUE : ZA_FALSE;

    return ZA_CMD_SUCCESS;
}

ZA_UCHAR za_innersor_ble_led_shink_ctrl(ZA_BOOL bIfOnOFF)
{
    sbBleLedShinkCtrl = bIfOnOFF ? ZA_TRUE : ZA_FALSE;

    return ZA_CMD_SUCCESS;
}

/***************************************game funcs ************************************/



ZA_VOID za_innersor_game_do_msg_magic_box_update_report(ZA_CHAR *pcMsgReport)
{
    za_app_msg_data_t za_appmsg_data;

    memset(za_appmsg_data.cBuffer, 0, sizeof(ZA_CHAR)*ZHENGAN_APP_MAG_DATABUF_LEN);
    memset(za_appmsg_data.cTagBuf, 0, sizeof(ZA_CHAR)*ZHENGAN_APP_MAG_DATABUF_LEN);

    za_appmsg_data.za_appmsg_type = ZA_APP_EVENT_AIOT_SEND_MAIN_MESSAGE;

    za_sprintf(za_appmsg_data.cTagBuf, "数字装具报告");
    za_sprintf(za_appmsg_data.cBuffer, pcMsgReport);

    if(ZA_ESP_PASS != za_msg_queue_send(za_get_app_msg_handle(NULL), &za_appmsg_data, portMAX_DELAY))
    {
        ZA_ESP_LOGE(TAG, "za_innersor_game_do_msgcmd_magic_box_update_report send msg failure. ---- > %d", za_appmsg_data.za_appmsg_type);
    }

    return;
}
