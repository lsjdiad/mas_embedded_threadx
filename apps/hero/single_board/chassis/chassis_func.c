/*
 * @Author: userName userEmail
 * @Date: 2026-07-15 18:20:29
 * @LastEditors: userName userEmail
 * @LastEditTime: 2026-07-15 18:48:18
 * @FilePath: \mas_embedded_threadx\apps\hero\single_board\chassis\chassis_func.c
 * @Description:
 */
#include "chassis_func.h"
#include "module_offline.h"
#include "motor_def.h"
#include <stdint.h>
#include "motor_dji.c"
static PIDInstance  chassis_follow_pid;
static DJI_Motor_t *chassis_motors[4];
void                chassis_init(void)
{
    PID_Init_Config_s config = {
        .MaxOut = 5, .IntegralLimit = 0.01, .DeadBand = 10, .Kp = 0.1, .Ki = 0, .Kd = 0.001, .Improve = 0x01}; // enable integratiaon limit
    PIDInit(&chassis_follow_pid, &config);

    Motor_Init_Config_s chassis_motor_config = {
        .offline_init_config =
            {
                .timeout_ms = 100,
                .enable     = 1,
            },
        .transport = MOTOR_TRANSPORT_CAN,
        .transport_config =
            {
                .can.hcan = BSP_CAN_HANDLE1,
            },
        .controller_init_config =
            {
                .speed_PID =
                    {
                        .Kd       = 0.01f,
                        .Ki       = 0.0f,
                        .Kp       = 0.001f,
                        .MaxOut   = 5.0f,
                        .DeadBand = 10.0f,
                    },
            },
        .setting_init_config =
            {
                .loop_type             = SPEED_LOOP,
                .algorithm_type        = CONTROL_PID,
                .enableflag            = 1,
                .motor_reverse_flag    = 0,
                .feedback_reverse_flag = 0,
                .angle_feedback_source = 0, // 0=电机自身反馈
                .speed_feedback_source = 0,
            },
        .motor_init_info =
            {
                .motor_type      = M3508,
                .gear_ratio      = 16,
                .max_torque      = 5.32,
                .torque_constant = 0.016,
            },
    };
    chassis_motor_config.transport_config.can.tx_id             = 1;
    chassis_motor_config.setting_init_config.motor_reverse_flag = 0;
    chassis_motor_config.offline_init_config.name               = "m3508_1";
    chassis_motor_config.offline_init_config.beep_times         = 1;
    chassis_motors[0]                                           = Motor_DJI_Init(&chassis_motor_config);
    if (chassis_motors[0] == NULL)
    {
        LOG_E("chassis motor[0] init failed");
        return;
    }
    chassis_motor_config.transport_config.can.tx_id             = 2;
    chassis_motor_config.setting_init_config.motor_reverse_flag = 0;
    chassis_motor_config.offline_init_config.name               = "m3508_2";
    chassis_motor_config.offline_init_config.beep_times         = 2;
    chassis_motors[1]                                           = Motor_DJI_Init(&chassis_motor_config);
    if (chassis_motors[1] == NULL)
    {
        LOG_E("chassis motor[1] init failed");
        return;
    }
    chassis_motor_config.transport_config.can.tx_id             = 3;
    chassis_motor_config.setting_init_config.motor_reverse_flag = 1;
    chassis_motor_config.offline_init_config.name               = "m3508_3";
    chassis_motor_config.offline_init_config.beep_times         = 3;
    chassis_motors[2]                                           = Motor_DJI_Init(&chassis_motor_config);
    if (chassis_motors[2] == NULL)
    {
        LOG_E("chassis motor[2] init failed");
        return;
    }
    chassis_motor_config.transport_config.can.tx_id             = 4;
    chassis_motor_config.setting_init_config.motor_reverse_flag = 1;
    chassis_motor_config.offline_init_config.name               = "m3508_4";
    chassis_motor_config.offline_init_config.beep_times         = 4;
    chassis_motors[3]                                           = Motor_DJI_Init(&chassis_motor_config);
    if (chassis_motors[3] == NULL)
    {
        LOG_E("chassis motor[3] init failed");
        return;
    }
    // TODO: 初始化底盘电机
}

void chassis_func(Chassis_Ctrl_Cmd_t *chassis_cmd)
{
    if (chassis_cmd == NULL)
    {
        Motor_DJI_Stop(chassis_motors[0]);
        Motor_DJI_Stop(chassis_motors[1]);
        Motor_DJI_Stop(chassis_motors[2]);
        Motor_DJI_Stop(chassis_motors[3]);
    }
    else
    {
        Motor_DJI_Start(chassis_motors[0]);
        Motor_DJI_Start(chassis_motors[1]);
        Motor_DJI_Start(chassis_motors[2]);
        Motor_DJI_Start(chassis_motors[3]);
    }

    (void)chassis_cmd;
}
