/* cmd_common.h , common functions for all cmds */

#ifndef CMD_COMMON_H
#define CMD_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "za_innersor.h"
#include "za_system.h"
#include "za_upgrade.h"

typedef enum
{
    ZA_CAIDENG_INNER_NO_OPERATE = 0,                                    // 没有操作
    ZA_CAIDENG_INNER_LIGHT_EVER,                                        // 彩灯一直亮（白色）
    ZA_CAIDENG_INNER_LIGHT_WHITE,                                       // 白色状态灯 （考虑做成呼吸模式）
    ZA_CAIDENG_INNER_LIGHT_RED,                                         // 红色状态灯 （考虑做成呼吸模式）
    ZA_CAIDENG_INNER_LIGHT_ORANGE,                                      // 橙色状态灯 （考虑做成呼吸模式）
    ZA_CAIDENG_INNER_LIGHT_YELLOW,                                      // 黄色状态灯 （考虑做成呼吸模式）
    ZA_CAIDENG_INNER_LIGHT_GREEN,                                       // 绿色状态灯 （考虑做成呼吸模式）
    ZA_CAIDENG_INNER_LIGHT_CYAN,                                        // 青色状态灯 （考虑做成呼吸模式）
    ZA_CAIDENG_INNER_LIGHT_BLUE,                                        // 蓝色状态灯 （考虑做成呼吸模式）
    ZA_CAIDENG_INNER_LIGHT_PURPLE,                                      // 紫色状态灯 （考虑做成呼吸模式）
    ZA_CAIDENG_INNER_LIGHT_RED_SINGLE,                                  // 单灯红色 / 通过补充字段设置 index （灯号）
    ZA_CAIDENG_INNER_LIGHT_YELLOW_SINGLE,                               // 单灯黄色
    ZA_CAIDENG_INNER_LIGHT_BLUE_SINGLE,                                 // 单灯蓝色
    ZA_CAIDENG_INNER_LIGHT_GREEN_SINGLE,                                // 单灯绿色
    ZA_CAIDENG_INNER_LIGHT_ORANGE_SINGLE,                               // 单灯橙色
    ZA_CAIDENG_INNER_LIGHT_PURPLE_SINGLE,                               // 单灯紫色
    ZA_CAIDENG_INNER_LIGHT_ORAPUR_SINGLE,                               // 橙紫双灯
    ZA_CAIDENG_INNER_LIGHT_PURORA_SINGLE,                               // 紫橙双灯
    ZA_CAIDENG_INNER_LIGHT_GREENFLASH_SINGLE,                           // 绿色闪烁
    ZA_CAIDENG_INNER_LIGHT_BLUEFLASH_SINGLE,                            // 蓝色闪烁
    ZA_CAIDENG_INNER_LIGHT_REDFLASH_SINGLE,                             // 红色闪烁
    ZA_CAIDENG_INNER_LIGHT_RANDOM,                                      // 随机状态灯 (随机出一个单色，支持呼吸模式)
    ZA_CAIDENG_INNER_LIGHT_RANDOM_SINGLE,                               // 单灯随机状态灯 (设置单灯随机出一个单色，支持呼吸模式)
    ZA_CAIDENG_INNER_LIGHT_FIRE,                                        // 模拟火焰颜色的状态灯
    ZA_CAIDENG_INNER_LIGHT_RGB,                                         // RGB三色循环
    ZA_CAIDENG_INNER_LIGHT_BREATH,                                      // 现有模式上叠加呼吸态
    ZA_CAIDENG_INNER_LIGHT_BLEND,                                       // 内部双灯 - 当前双灯颜色混合
    ZA_CAIDENG_INNER_LIGHT_BRIGHTNESS,                                  // 通过补充字符串设置亮度 0-255级
    ZA_CAIDENG_INNER_LIGHT_BRION,                                       // 打开亮度
    ZA_CAIDENG_INNER_LIGHT_BRIOFF,                                      // 关闭亮度
    ZA_CAIDENG_INNER_STOP_BREATH,                                       // 停止呼吸态
    ZA_CAIDENG_INNER_STOP_NOW,                                          // 彩灯立刻关闭
    ZA_CAIDENG_INNER_SET_RGBDATA,                                       // 设置RGB三色数据及亮度值
    ZA_CAIDENG_INNER_TYPE_END
}za_caidengi_mode_t;

typedef enum
{
    ZA_MOTOR_INNER_NO_OPERATE = 0,                                      // 没有操作
    ZA_MOTOR_INNER_ZHENDONG_EVER,                                       // 内部马达一直动
    ZA_MOTOR_INNER_STOP_NOW,                                            // 内部马达立刻停下来
    ZA_MOTOR_INNER_TYPE_END
}za_motorinr_mode_t;

typedef enum
{
    ZA_BUZZ_INNER_NO_OPERATE = 0,                                       // 没有操作
    ZA_BUZZ_INNER_VOICE_EVER,                                           // 内部蜂鸣器一直响
    ZA_BUZZ_INNER_MUSIC_NOTE_C,                                         // 音色模式 C调 NOTE_C - 音阶，取值0~7 字符串描述：dao ruai mi fa sao la xi dou , default High
    ZA_BUZZ_INNER_MUSIC_NOTE_CS,                                        // NOTE_Cs 0-7 
    ZA_BUZZ_INNER_MUSIC_NOTE_D,                                         // NOTE_D 0-7 
    ZA_BUZZ_INNER_MUSIC_NOTE_EB,                                        // NOTE_Eb 0-7
    ZA_BUZZ_INNER_MUSIC_NOTE_E,                                         // NOTE_E 0-7
    ZA_BUZZ_INNER_MUSIC_NOTE_F,                                         // NOTE_F 0-7
    ZA_BUZZ_INNER_MUSIC_NOTE_FS,                                        // NOTE_Fs 0-7
    ZA_BUZZ_INNER_MUSIC_NOTE_G,                                         // NOTE_G 0-7
    ZA_BUZZ_INNER_MUSIC_NOTE_GS,                                        // NOTE_Gs 0-7
    ZA_BUZZ_INNER_MUSIC_NOTE_A,                                         // NOTE_A 0-7
    ZA_BUZZ_INNER_MUSIC_NOTE_BB,                                        // NOTE_Bb 0-7
    ZA_BUZZ_INNER_MUSIC_NOTE_B,                                         // NOTE_B 0-7
    ZA_BUZZ_INNER_VOLTS_LOW,                                            // POWER LOW
    ZA_BUZZ_INNER_MUSIC_NOTE_C_SET,                                     // Setting 0-7 value in this Note
    ZA_BUZZ_INNER_MUSIC_NOTE_CS_SET,                                    // 
    ZA_BUZZ_INNER_MUSIC_NOTE_D_SET,                                     //
    ZA_BUZZ_INNER_MUSIC_NOTE_EB_SET,                                    //
    ZA_BUZZ_INNER_MUSIC_NOTE_E_SET,                                     //
    ZA_BUZZ_INNER_MUSIC_NOTE_F_SET,                                     //
    ZA_BUZZ_INNER_MUSIC_NOTE_FS_SET,                                    //
    ZA_BUZZ_INNER_MUSIC_NOTE_G_SET,                                     //
    ZA_BUZZ_INNER_MUSIC_NOTE_GS_SET,                                    //
    ZA_BUZZ_INNER_MUSIC_NOTE_A_SET,                                     //
    ZA_BUZZ_INNER_MUSIC_NOTE_BB_SET,                                    //
    ZA_BUZZ_INNER_MUSIC_NOTE_B_SET,                                     //
    ZA_BUZZ_INNER_STOP_NOW,                                             // 内部蜂鸣器立刻停下来
    ZA_BUZZ_INNER_TYPE_END
}za_buzziner_mode_t;

typedef enum
{
    ZA_SYS_INNER_NO_OPERATE = 0,                                        // 没有操作
    ZA_SYS_INNER_REBOOT_NOW,                                            // go reboot system now
    ZA_SYS_INNER_REBOOT_DELAY,                                          // go reboot system after about 2 seconds
    ZA_SYS_INNER_STANDBY_NOW,                                           // 立刻进入待机状态 
    ZA_SYS_INNER_ENERGY_CONSERVATION,                                   // 启动节能模式
    ZA_SYS_INNER_FULLY_LOADED,                                          // 启动满载模式
    ZA_SYS_INNER_POWER_STATUS_UPDATE,                                   // 电量状态更新
    ZA_SYS_INNER_COMMAND_STORE_MODE_START,                              // 启动指令存储模式 - 该模式下接收到的指令不会执行，而是直接存储在本地文件中
    ZA_SYS_INNER_COMMAND_STORE_MODE_STOP,                               // 停止指令存储模式 - 结束该模式后，接收到的指令被直接执行，而不再被存储
    ZA_SYS_INNER_COMMAND_BOOTUP_DOING_ON_ORDER,                         // 开机执行本地指令 - 开机自动顺序执行一遍本地存储的指令。 若遇到平台此时下传指令时，则停止本地指令执行，即时相应平台指令
    ZA_SYS_INNER_COMMAND_BOOTUP_DOING_ON_CIRCLE,                        // 开机执行本地指令 - 循环执行
    ZA_SYS_INNER_COMMAND_BOOTUP_DOING_ON_RANDOM,                        // 开机执行本地指令 - 随机执行
    ZA_SYS_INNER_COMMAND_BOOTUP_DOING_OFF,                              // 关闭开机执行本地指令 - 开机自动忽略本地预存指令，不执行
    ZA_SYS_INNER_COMMAND_BOOTUP_GET_TYPE,
    ZA_SYS_INNER_COMMAND_BOOTUP_DELAY_MS,                               // 指令执行延时 xxx ms
    ZA_SYS_INNER_AIOT_COM_MODE_INTERNET,                                // 设置AIOT通信，通过互联网进行
    ZA_SYS_INNER_AIOT_COM_MODE_LOCALSER,                                // 设置AIOT通信，通过局域网进行
    ZA_SYS_INNER_BINDID_SET_FORCE,                                      // 强制设定绑定ID并存储
    ZA_SYS_INNER_TRY_SET_LOCAL_SERVER,                                  // try to connect local mqtt server
    ZA_SYS_INNER_WDT_SET,                                               // watchdog startup
    ZA_SYS_INNER_WDT_FEED,                                              // watchdog feeding
    ZA_SYS_WIRELESS_MODE_SET_BLE_HOST,                                  // 系统设置为蓝牙主机模式
    ZA_SYS_WIRELESS_MODE_SET_WIFI_CLIENT,                               // 系统设置为WIFI客户端模式
    ZA_SYS_WIRELESS_MODE_SET_BLE_HOST_WIFI_CLIENT,                      // 系统设置为蓝牙主机与WIFI客户端同时存在的模式
    ZA_SYS_WIRELESS_MODE_GET_CURRENT,                                   // 获取当前系统的无线模式
    ZA_SYS_OUTMODULE_SET_ZHENDONG_SENSOR,                               // 设置当前外部模块为灵敏震动传感器
    ZA_SYS_OUTMODULE_SET_RENTIGANYING_SENSOR,                           // 设置当前外部模块为红外人体感应传感器
    ZA_SYS_OUTMODULE_SET_HONGWAIDUIGUAN_SENSOR,                         // 设置当前外部模块为红外对管传感器
    ZA_SYS_OUTMODULE_SET_LIUZHOUTUOLUOYI_SENSOR,                        // 设置当前外部模块为六轴陀螺仪
    ZA_SYS_OUTMODULE_SET_DIANZILUOPAN_SENSOR,                           // 设置当前外部模块为电子罗盘
    ZA_SYS_OUTMODULE_SET_SERVO_DRIVER,                                  // 设置当前外部模块为直流电机驱动器
    ZA_SYS_OUTMODULE_SET_ROBOT_T1,                                      // 设置当前外部模块组合为科创机器人T1型
    ZA_SYS_OUTMODULE_SET_ACTION_BASE,                                   // 设置当前外部模块为动作类数据底座
    ZA_SYS_OUTMODULE_TYPE_GET_CURRENT,                                  // 获取当前外部模块类型模式
    ZA_SYS_ERASE_NVS_LOCALSTORE_PARTH,                                  // 擦除NVS中的Localstore存储路径（含内部所有KEY）
    ZA_SYS_ERASE_NVS_LOCALSTORE_SYSMODE_KEY,                            // 擦除NVS中的Localstore存储路径里的System Mode Key
    ZA_SYS_WIFI_DISCONNECT_SET_NULL,                                    // 断开WIFI连接并设定WIFI参数为null，以便于下次启动后处于非连接状态
    ZA_SYS_INFO_MONITOR_SYSTEM,                                         // 执行一次系统运行信息监控
    ZA_SYS_INFO_DEBUG_JSON,                                             // 基于打印的配合调试JSON打包与解析
    ZA_SYS_REQUIRE_MACADDR_BLE,                                         // 通过上位机的BLE请求MAC地址
    ZA_SYS_INNER_DRIVERS_INIT_APDS9960,                                 // to Init outer device's driver
    ZA_SYS_INNER_DRIVERS_INIT_MPU6050,                                  // to Init outer device's driver
    ZA_SYS_OUTER_DRV_SHAKEMODULE_INIT,                                  // to Init outer devices shake module
    ZA_SYS_OUTER_DRV_BODYSENMODULE_INIT,                                // to Init outer devices bodysen module
    ZA_SYS_OUTER_DRV_IRPARTY_INIT,                                      // to Init outer devices irparty module
    ZA_SYS_OUTER_DRV_ACCGYRO_INIT,                                      // to Init outer devices accgyro module
    ZA_SYS_OUTER_DRV_COMPASS_INIT,                                      // to Init outer devices compass module
    ZA_SYS_OUTER_DRV_MARK_ACTION_INIT,                                  // 动作感应式数据底座模块
    ZA_SYS_OUTER_DRV_MARK_VISIBLE_INIT,                                 // 可见光感应式数据底座模块
    ZA_SYS_OUTER_DRV_MARK_RFID_INIT,                                    // 射频识别式数据底座模块
    
    ZA_SYS_INNER_TYPE_END
}za_systminr_mode_t;

typedef enum
{
    ZA_UPGRADE_NO_OPERATE = 0,                                          // 没有操作
    ZA_UPGRADE_ZASMART_FORCE,                                           // to upgrade the firmware forcely
    ZA_UPGRADE_ZASMART_VERSION,                                         // to upgrade the firmware by version
    ZA_UPGRADE_RESTORE_FACTORY,                                         // erease the ota area
    ZA_UPGRADE_ZASMART_DEBUG,                                           // update for debug
    ZA_UPGRADE_ZACHANGE_FIRMWARE,                                       // change the firmware of za diy board.
    ZA_UPGRADE_ZASMART_OPENFORCE,                                       // to upgrade the firmware forcely with http
    ZA_UPGRADE_ZASMART_AUTO,                                            // set auto update when connect web by default path(https).
    ZA_UPGRADE_ZASMART_AUTO_OPEN,                                       // set auto update when connect web by http path.
    ZA_UPGRADE_ZASMART_MANUAL,                                          // set no auto update by default path.
    ZA_UPGRADE_SET_OBJPATH,                                             // set auto update path when connect web.
    ZA_UPGRADE_DO_FROM_OBJPATH,                                         // start update from path been setted when connect web.
    ZA_UPGRADE_TYPE_END
}za_upgradei_mode_t;

typedef enum
{
    ZA_GAME_MODE_NO_OPERATE = 0,                                        // 没有操作
    ZA_GAME_MODE_CLOSE_EVENT,                                           // 关掉上传服务器的游戏事件报告
    ZA_GAME_MODE_OPEN_EVENT,                                            // 打开上传服务器的游戏事件报告
    ZA_GAME_MODE_MSG_SHAKE_CHECK_STATUS,                                // 在消息通道中执行一次SHAKE状态检查
    ZA_GAME_MODE_MSG_SHAKE_REPORT,                                      // 在消息通道中持续监测SHAKE状态，可以设定间隔时间
    ZA_GAME_MODE_MSG_TEMPERATURE_STATUS,                                // 在消息通道中探测一次Temprature (感知芯片的温度)
    ZA_GAME_MODE_MSG_ACC_XYZ_REPORT,                                    // 在消息通道中持续监测相对于重力加速度的向量倍数，Acceleration 分别在 X 、Y 、Z 轴上的变化, 可以设定间隔时间
    ZA_GAME_MODE_MSG_GYRO_XYZ_REPORT,                                   // 在消息通道中持续监测以度/秒为单位的角速度 angular speed ,分别以 X 、Y 、Z 轴为绕转测量, 可以设定间隔时间
    ZA_GAME_MODE_MSG_ACC_ANGLE_XY_REPORT,                               // 在消息通道中持续监测以重力加速向量数据为参考的X轴和Y轴的旋转角(该数据存在噪声，在小幅度加速状态下有效)
    ZA_GAME_MODE_MSG_ANGLE_XYZ_REPORT,                                  // 在消息通道中持续监测X、Y、Z轴方向上的旋转角度的向量值
    ZA_GAME_MODE_MSG_CTRL_SIMPLE_DIS,                                   // 控制消息通道为精简显示模式
    ZA_GAME_MODE_MSG_CTRL_JSON_DIS,                                     // 控制消息通道为JSON显示模式
    ZA_GAME_MODE_MSG_ACC_GYRO_ANGLE_XYZ_REPORT,                         // 在消息通道中持续监测ACC、GYRO、ANGLE的 x y z 三轴数据
    ZA_GAME_MODE_MSG_ACC_XYZ_MAX_REPORT,                                // 在消息通道中持续监测设置的时间间隔内的X/Y/Z轴上最大值时刻的 (x y z) 值.
    ZA_GAME_MODE_MSG_GYRO_XYZ_MAX_REPORT,                               // 在消息通道中持续监测设置的时间间隔内的X/Y/Z轴上最大值时刻的 (x y z) 值.
    ZA_GAME_MODE_MSG_ACC_ANGLE_XY_MAX_REPORT,                           // 在消息通道中持续监测设置的时间间隔内的X/Y/Z轴上最大值时刻的 (x y) 值.
    ZA_GAME_MODE_MSG_ANGLE_XYZ_MAX_REPORT,                              // 在消息通道中持续监测设置的时间间隔内的X/Y/Z各轴上的最大转角差值（该值不是同一瞬间的数据，是各轴上的最大角差）.
    ZA_GAME_MODE_MSG_ACC_XYZ_STATIC_REPORT,                             // 当静止下来后，上报ACC在xyz轴上的值
    ZA_GAME_MODE_MSG_ACC_XYZ_DYNAMIC_REPORT,                            // 当运动起来后，上报ACC在xyz轴上的值
    ZA_GAME_MODE_MSG_GYRO_XYZ_DYNAMIC_REPORT,                           // 当运动起来后，上报GYRO在xyz轴上的值
    ZA_GAME_MODE_MSG_ANGLE_XYZ_STATIC_REPORT,                           // 当静止下来后，上报Angle在xyz轴上的值
    ZA_GAME_MODE_MSG_ANGLE_XYZ_DYNAMIC_REPORT,                          // 当运动起来后，上报Angle在xyz轴上的值

    ZA_GAME_MODE_CMD_CTRL_SIMPLE_DIS,                                   // 控制命令通道为精简显示模式
    ZA_GAME_MODE_CMD_CTRL_JSON_DIS,                                     // 控制命令通道为JSON显示模式

    ZA_GAME_MODE_CMD_SHAKE_CHECK_STATUS,                                // 在命令通道中执行一次SHAKE状态检查
    ZA_GAME_MODE_CMD_SHAKE_REPORT,                                      // 在命令通道中持续监测SHAKE状态，可以设定间隔时间
    ZA_GAME_MODE_CMD_TEMPERATURE_STATUS,                                // 在命令通道中探测一次Temprature (感知芯片的温度)
    ZA_GAME_MODE_CMD_ACC_XYZ_REPORT,                                    // 在命令通道中持续监测相对于重力加速度的向量倍数，Acceleration 分别在 X 、Y 、Z 轴上的变化, 可以设定间隔时间
    ZA_GAME_MODE_CMD_GYRO_XYZ_REPORT,                                   // 在命令通道中持续监测以度/秒为单位的角速度 angular speed ,分别以 X 、Y 、Z 轴为绕转测量, 可以设定间隔时间
    ZA_GAME_MODE_CMD_ACC_ANGLE_XY_REPORT,                               // 在命令通道中持续监测以重力加速向量数据为参考的X轴和Y轴的旋转角(该数据存在噪声，在小幅度加速状态下有效)
    ZA_GAME_MODE_CMD_ANGLE_XYZ_REPORT,                                  // 在命令通道中持续监测X、Y、Z轴方向上的旋转角度的向量值
    ZA_GAME_MODE_CMD_ACC_GYRO_ANGLE_XYZ_REPORT,                         // 在命令通道中持续监测ACC、GYRO、ANGLE的 x y z 三轴数据
    ZA_GAME_MODE_CMD_ACC_XYZ_MAX_REPORT,                                // 在命令通道中持续监测设置的时间间隔内的X/Y/Z轴上最大值时刻的 (x y z) 值.
    ZA_GAME_MODE_CMD_GYRO_XYZ_MAX_REPORT,                               // 在命令通道中持续监测设置的时间间隔内的X/Y/Z轴上最大值时刻的 (x y z) 值.
    ZA_GAME_MODE_CMD_ACC_ANGLE_XY_MAX_REPORT,                           // 在命令通道中持续监测设置的时间间隔内的X/Y/Z轴上最大值时刻的 (x y) 值.
    ZA_GAME_MODE_CMD_ANGLE_XYZ_MAX_REPORT,                              // 在命令通道中持续监测设置的时间间隔内的X/Y/Z各轴上的最大转角差值（该值不是同一瞬间的数据，是各轴上的最大角差）.
    ZA_GAME_MODE_CMD_ACC_XYZ_STATIC_REPORT,                             // 当静止下来后，命令通道上报ACC在xyz轴上的值
    ZA_GAME_MODE_CMD_ACC_XYZ_DYNAMIC_REPORT,                            // 当运动起来后，命令通道上报ACC在xyz轴上的值
    ZA_GAME_MODE_CMD_GYRO_XYZ_DYNAMIC_REPORT,                           // 当运动起来后，命令通道上报GYRO在xyz轴上的值
    ZA_GAME_MODE_CMD_ANGLE_XYZ_STATIC_REPORT,                           // 当静止下来后，命令通道上报Angle在xyz轴上的值
    ZA_GAME_MODE_CMD_ANGLE_XYZ_DYNAMIC_REPORT,                          // 当运动起来后，命令通道上报Angle在xyz轴上的值

    ZA_GAME_MODE_CMD_ANGLE_XYZ_DYNAMIC_TO_STATIC_REPORT,                // 检测到动之后，采集间隔内的最优值缓存，并在静止后，算出xyz轴上运动了多少值

    ZA_GAME_MODE_SWITCH_MSG_ON,                                         // 切换为MSG模式
    ZA_GAME_MODE_SWITCH_CMD_ON,                                         // 切换为CMD模式

    ZA_GAME_MODE_CMD_SET_LED_RGBC,                                      // 设置数字交互感知型道具灯光的颜色亮度值
    ZA_GAME_MODE_CMD_SET_MOTOR_STATUS,                                  // 设置工作马达的震动状态
    ZA_GAME_MODE_CMD_SET_BUZZER_NOTE,                                   // 设置蜂鸣器的音符

    ZA_GAME_MODE_TYPE_END
}za_gamectrl_mode_t;


//ZA_BOOL cmd_common_init();


#ifdef __cplusplus
}
#endif

#endif
