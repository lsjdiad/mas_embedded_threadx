/*
 * @Author: lsjdiad 949186291@qq.com
 * @Date: 2026-07-18 09:53:09
 * @LastEditors: lsjdiad 949186291@qq.com
 * @LastEditTime: 2026-07-18 20:51:47
 * @FilePath: \mas_embedded_threadx\apps\hero\single_board\robot_control.c
 * @Description:
 */
#include "robot_control.h"
#include "hero_def.h"
#include "gimbal_func.h"
#include "shoot_func.h"
#include "chassis_func.h"
#include "robot_func.h"
#include "module_ins.h"
#include "tx_api.h"
#include "bsp_def.h"

#define LOG_TAG "app_robot_control"
#define LOG_LVL LOG_LVL_INFO
#include "ulog_def.h"

static TX_THREAD                  robot_control_thread;
APPS_STACK_SECTION static uint8_t robot_control_thread_stack[1024];
static Gimbal_Ctrl_Cmd_t          gimbal_cmd;
static Shoot_Ctrl_Cmd_t           shoot_cmd;
static Chassis_Ctrl_Cmd_t         chassis_cmd;
static uint16_t                   yaw_ecd;
/* Ozone 调试: 控制循环心跳 */
volatile int dbg_ctrl_beat = 0;

static void robot_control_task(ULONG thread_input)
{
    (void)thread_input;
    while (1)
    {
        /* 遥控器控制输入 */
        RemoteControlSet(&gimbal_cmd, &shoot_cmd, &chassis_cmd);

        /* 云台控制（同时获取 yaw 编码器值） */
        gimbal_func(&gimbal_cmd, &yaw_ecd);
        /* 发射机构控制 */
        shoot_func(&shoot_cmd);
        /* 底盘控制：偏移角 = (当前ecd - 对齐ecd) 归一化后转为度 */
        chassis_cmd.offset_angle = CalcOffsetAngle(yaw_ecd) * 360.0f / 8191.0f;
        chassis_cmd.vx           = chassis_cmd.vx * CHASSIS_MAX_SPEED_MPS;
        chassis_cmd.vy           = chassis_cmd.vy * CHASSIS_MAX_SPEED_MPS;
        chassis_cmd.wz           = chassis_cmd.wz * CHASSIS_MAX_SPEED_MPS;
        chassis_func(&chassis_cmd);

        dbg_ctrl_beat++;

        tx_thread_sleep(2);
    }
}

void robot_control_init(void)
{
    Module_INS_Init();
    gimbal_init();
    shoot_init();
    chassis_init();

    UINT status = tx_thread_create(&robot_control_thread, "robot_control_thread", robot_control_task, 0, robot_control_thread_stack, 1024, 30, 30,
                                   TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        LOG_E("robot_control_task failed!");
        return;
    }
    LOG_I("robot_control init success!");
}
