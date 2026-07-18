#include "shoot_func.h"
#include "motor_dji.h"
#include "user_lib.h"
#define LOG_TAG "app_shoot"
#define LOG_LVL LOG_LVL_INFO
#include "ulog_def.h"
DJI_Motor_t *friction_l = NULL;
DJI_Motor_t *friction_2 = NULL;
DJI_Motor_t *loader     = NULL;
void         shoot_init(void)
{
    Motor_Init_Config_s shoot = {
        .controller_init_config =
            {
                .speed_PID =
                    {
                        .Kp       = 0.01f,
                        .Ki       = 0.0f,
                        .Kd       = 0.005f,
                        .MaxOut   = 5.0f,
                        .DeadBand = 0.1f,
                    },
            },
        .motor_init_info =
            {
                .motor_type      = M3508,
                .gear_ratio      = 19,
                .max_torque      = 6,
                .torque_constant = 0.0016f,
            },
        .offline_init_config =
            {
                .beep_times = 6,
                .enable     = 1,
                .name       = "35081",
                .timeout_ms = 100,
            },
        .setting_init_config =
            {
                .angle_feedback_source = 0,
                .speed_feedback_source = 0,
                .loop_type             = SPEED_LOOP,
                .feedback_reverse_flag = 0,
                .algorithm_type        = CONTROL_PID,
            },
        .transport = MOTOR_TRANSPORT_CAN,
        .transport_config =
            {
                .can =
                    {
                        .hcan = BSP_CAN_HANDLE2,
                    },

            },
        .transport_config.can.tx_id = 6,
    };
    friction_l = Motor_DJI_Init(&shoot);
    if (friction_l == NULL)
    {
        LOG_E("friction_l init failed");
        return;
    }
    // 右摩擦轮
    shoot.transport_config.can.tx_id     = 8;
    shoot.offline_init_config.name       = "35082";
    shoot.offline_init_config.beep_times = 6;
    friction_2                           = Motor_DJI_Init(&shoot);
    if (friction_2 == NULL)
    {
        LOG_E("friction_r init failed");
        return;
    }
    Motor_Init_Config_s loader_config = {
        .offline_init_config =
            {
                .name       = "m2006",
                .timeout_ms = 100,
                .beep_times = 7,
                .enable     = 1,
            },
        .transport = MOTOR_TRANSPORT_CAN,
        .transport_config.can =
            {
                .hcan  = BSP_CAN_HANDLE1,
                .tx_id = 7,
            },
        .controller_init_config =
            {
                .angle_PID =
                    {
                        .Kp       = 2.0f,
                        .Ki       = 0.0f,
                        .Kd       = 0.1f,
                        .MaxOut   = 8000,
                        .DeadBand = 0.5f,
                    },

                .speed_PID =
                    {
                        .Kp       = 0.005f,
                        .Ki       = 0.0f,
                        .Kd       = 0.001f,
                        .MaxOut   = 0.5f,
                        .DeadBand = 10.0f,
                    },
            },
        .setting_init_config =
            {
                .angle_feedback_source = 0,
                .speed_feedback_source = 0,
                .loop_type             = ANGLE_AND_SPEED_LOOP,
                .feedback_reverse_flag = 0,
                .algorithm_type        = CONTROL_PID,
            },
        .motor_init_info =
            {
                .motor_type      = M2006,
                .gear_ratio      = 36,
                .max_torque      = 1.8f,
                .torque_constant = 0.005f,
            },
    };
    loader = Motor_DJI_Init(&loader_config);
    if (loader == NULL)
    {
        LOG_E("loader init failed");
        return;
    }

    LOG_I("shoot init");
};

void shoot_func(Shoot_Ctrl_Cmd_t *shoot_Ctrl)
{
    static loader_mode_e last_load_mode      = load_stop;
    static float         loader_angle_target = 0;
    const float          slot_angle          = PI / 4.0f * 36.0f;

    if (shoot_Ctrl == NULL) return;
    if (!friction_l || !friction_2 || !loader) return;

    /* 离线保护 */
    if (Module_Offline_get_device_status(friction_l->base.offline_dev) || Module_Offline_get_device_status(friction_2->base.offline_dev) ||
        Module_Offline_get_device_status(loader->base.offline_dev))
    {
        Motor_DJI_Stop(friction_l);
        Motor_DJI_Stop(friction_2);
        Motor_DJI_Stop(loader);
        return;
    }

    switch (shoot_Ctrl->shoot_mode)
    {
    case shoot_off:
        Motor_DJI_Stop(friction_l);
        Motor_DJI_Stop(friction_2);
        Motor_DJI_Stop(loader);
        break;

    case shoot_on:
        Motor_DJI_Start(friction_l);
        Motor_DJI_Start(friction_2);
        Motor_DJI_Start(loader);

        /* ---- 摩擦轮 ---- */
        switch (shoot_Ctrl->friction_mode)
        {
        case friction_on:
            Motor_DJI_SetRef(friction_l, -6800 * RPM_2_RAD_PER_SEC);
            Motor_DJI_SetRef(friction_2, 6800 * RPM_2_RAD_PER_SEC);
            break;
        case friction_off:
            Motor_DJI_SetRef(friction_l, 0);
            Motor_DJI_SetRef(friction_2, 0);
            break;
        }

        /* ---- 拨弹盘 ---- */
        switch (shoot_Ctrl->load_mode)
        {
        case load_stop:
            loader_angle_target = loader->base.measure.total_angle;
            break;
        case load_reverse:
            break;
        case load_1_bullet:
            if (last_load_mode != load_1_bullet)
            { // 上升沿触发一次
                loader_angle_target += slot_angle;
            }
            break;
        case load_burstfire:
            loader_angle_target += slot_angle * 4; // 持续累加，速度由 MaxOut 限幅
            break;
        }
        Motor_DJI_SetRef(loader, loader_angle_target);
        break;
    }

    last_load_mode = shoot_Ctrl->load_mode;
}
