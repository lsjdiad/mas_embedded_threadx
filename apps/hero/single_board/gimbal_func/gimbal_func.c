/*
 * @Author: lsjdiad 949186291@qq.com
 * @Date: 2026-07-17 18:45:02
 * @LastEditors: lsjdiad 949186291@qq.com
 * @LastEditTime: 2026-07-18 20:21:02
 * @FilePath: \mas_embedded_threadx\apps\hero\single_board\gimbal_func\gimbal_func.c
 * @Description:
 */
#include "gimbal_func.h"
#include "module_bmi088.h"
#include "module_ins.h"
#include "motor_dji.h"
#include "motor_damiao.h"
#include "user_lib.h"

#define LOG_TAG "app_gimbal"
#define LOG_LVL LOG_LVL_INFO
#include "ulog_def.h"

// gimbal 电机指针定义
static DJI_Motor_t           *yaw_motor   = NULL; // yaw电机指针
const static Bmi088_device_t *bmi088_dev  = NULL;
static DM_Motor_t            *pitch_motor = NULL;
static const Ins_t           *ins         = NULL; // INS 姿态数据

/* Ozone 调试: gimbal_init 进度
 * 0=未开始  1=BMI088 OK  2=yaw 已注册  3=pitch 已注册  4=完成 */
volatile int dbg_gimbal_step = 0;

void gimbal_init(void)
{
    bmi088_dev = Module_BMI088_get_device();
    if (bmi088_dev == NULL)
    {
        LOG_E("bmi088_dev is null");
        return;
    }
    ins = Module_INS_get();
    if (ins == NULL)
    {
        LOG_E("ins is null");
        return;
    }
    dbg_gimbal_step                = 1; /* BMI088 OK */
    Motor_Init_Config_s yaw_config = {
        .offline_init_config =
            {
                .name       = "6020",
                .timeout_ms = 100,
                .beep_times = 3,
                .enable     = 1,
            },
        .transport = MOTOR_TRANSPORT_CAN,
        .transport_config.can =
            {
                .hcan  = BSP_CAN_HANDLE1,
                .tx_id = 1,
            },
        .controller_init_config =
            {
                .other_angle_feedback_ptr = &ins->YawTotalAngle_rad,
                .other_speed_feedback_ptr = &bmi088_dev->gyro[2],
                .angle_PID =
                    {
                        .Kp       = 9.0f,
                        .Ki       = 0.0f,
                        .Kd       = 0.0f,
                        .MaxOut   = 12.0f, /* 位置环输出限幅 = 速度给定上限 (rad/s) */
                        .DeadBand = 0.01f,
                    },
                .speed_PID =
                    {
                        .Kp       = 0.6f,
                        .Ki       = 0.0f,
                        .Kd       = 0.0f,
                        .MaxOut   = 2.223f, /* 速度环输出限幅 = 最大扭矩 (Nm) */
                        .DeadBand = 0.01f,
                    },
            },
        .setting_init_config =
            {
                .algorithm_type        = CONTROL_PID,
                .feedback_reverse_flag = 0,
                .angle_feedback_source = 1,
                .speed_feedback_source = 1,
                .loop_type             = ANGLE_AND_SPEED_LOOP,
            },
        .motor_init_info = {
            .motor_type = GM6020_CURRENT, .gear_ratio = 1 /*减速比*/, .max_torque = 2.223f /*最大输出扭矩*/, .torque_constant = 0.741f /*力矩常数*/}};
    yaw_motor = Motor_DJI_Init(&yaw_config);
    if (yaw_motor == NULL)
    {
        LOG_E("yaw_motor init failed");
        return;
    }
    dbg_gimbal_step                  = 2; /* yaw 已注册 */
    Motor_Init_Config_s pitch_config = {.offline_init_config =
                                            {
                                                .name       = "dm4310",
                                                .timeout_ms = 100,
                                                .beep_times = 4,
                                                .enable     = 1,
                                            },
                                        .transport = MOTOR_TRANSPORT_CAN,
                                        .transport_config.can =
                                            {
                                                .hcan  = BSP_CAN_HANDLE2,
                                                .tx_id = 0X01,
                                                .rx_id = 0Xf1,
                                            },
                                        .controller_init_config =
                                            {
                                                .other_angle_feedback_ptr = &ins->euler_rad[1],
                                                .other_speed_feedback_ptr = &bmi088_dev->gyro[0], // c板的pitch轴角速度，根据实际选择对应角速度
                                                .angle_PID =
                                                    {
                                                        .Kp       = 9.0f,
                                                        .Ki       = 0.0f,
                                                        .Kd       = 0.0f,
                                                        .MaxOut   = 12.0f, /* 位置环输出限幅 = 速度给定上限 (rad/s) */
                                                        .DeadBand = 0.01f,
                                                    },
                                                .speed_PID =
                                                    {
                                                        .Kp       = 0.6f,
                                                        .Ki       = 0.0f,
                                                        .Kd       = 0.0f,
                                                        .MaxOut   = 5.0f, /* 速度环输出限幅 = 最大扭矩 (Nm) */
                                                        .DeadBand = 0.01f,
                                                    },
                                            },
                                        .setting_init_config =
                                            {
                                                .algorithm_type        = CONTROL_PID,
                                                .feedback_reverse_flag = 1,
                                                .angle_feedback_source = 1,
                                                .speed_feedback_source = 1,
                                                .loop_type             = ANGLE_AND_SPEED_LOOP,
                                            },
                                        .motor_init_info = {.motor_type = DM4310, .gear_ratio = 10, .max_torque = 10, .torque_constant = 0.093f}};
    pitch_motor                      = Motor_DM_Init(&pitch_config, DM_MIT_MODE);
    if (pitch_motor == NULL)
    {
        LOG_E("pitch_motor init failed");
        return;
    }
    dbg_gimbal_step = 3; /* pitch 已注册 */

    dbg_gimbal_step = 4; /* gimbal_init 全部完成 */
}

float gimbal_get_yaw_angle_deg(void)
{
    if (yaw_motor == NULL) return 0.0f;
    return yaw_motor->base.measure.total_angle * RAD_2_DEGREE;
}

void gimbal_func(Gimbal_Ctrl_Cmd_t *gimbal_cmd, uint16_t *yaw_ecd)
{
    if (gimbal_cmd != NULL)
    {
        if (!Module_Offline_get_device_status(yaw_motor->base.offline_dev) && !Module_Offline_get_device_status(pitch_motor->base.offline_dev))
        {
            switch (gimbal_cmd->gimbal_mode)
            {
            case gimbal_zero_force:
                Motor_DJI_Stop(yaw_motor);
                Motor_DM_Stop(pitch_motor);
                break;
            case gimbal_gyro_mode:
                Motor_DJI_Start(yaw_motor);
                Motor_DM_Start(pitch_motor);
                Motor_DJI_SetRef(yaw_motor, gimbal_cmd->yaw * DEGREE_2_RAD);
                Motor_DM_SetRef(pitch_motor, gimbal_cmd->pitch * DEGREE_2_RAD);
                break;
            case gimbal_auto_mode:
                break;
            }
        }
        else
        {
            Motor_DJI_Stop(yaw_motor);
            Motor_DM_Stop(pitch_motor);
        }
    }
    if (!Module_Offline_get_device_status(yaw_motor->base.offline_dev) && yaw_ecd != NULL)
    {
        *yaw_ecd = yaw_motor->measure.ecd;
    }
};