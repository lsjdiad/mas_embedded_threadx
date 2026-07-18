/*
 * @Author: lsjdiad 949186291@qq.com
 * @Date: 2026-07-18 09:22:52
 * @LastEditors: lsjdiad 949186291@qq.com
 * @LastEditTime: 2026-07-18 14:51:16
 * @FilePath: \mas_embedded_threadx\apps\hero\single_board\robot_func\robot_func.c
 * @Description:
 */
#include "robot_func.h"
#include "module_remote.h"
#include <stdint.h>
#include "hero_def.h"
#include "user_lib.h"

void RemoteControlSet(Gimbal_Ctrl_Cmd_t *Gimbal_Ctrl, Shoot_Ctrl_Cmd_t *Shoot_Ctrl, Chassis_Ctrl_Cmd_t *Chassis_Ctrl)
{
    if (!Gimbal_Ctrl || !Shoot_Ctrl || !Chassis_Ctrl) return;

    uint8_t state = Module_Remote_get_offline_status();

    /* RC在线才响应 */
    if (state & 0x01)
    {
        int16_t ch5 = Module_Remote_get_channel(5);
        int16_t ch6 = Module_Remote_get_channel(6);
        int16_t ch7 = Module_Remote_get_channel(7);
        int16_t ch8 = Module_Remote_get_channel(8);

        /* 云台控制: ch5 控制模式 (范围容差 ±100) */
        if (ch5 <= SBUS_CHX_UP + 100)
        {
            Gimbal_Ctrl->gimbal_mode = gimbal_gyro_mode;
            Gimbal_Ctrl->yaw -= 0.001f * (float)Module_Remote_get_channel(1);
            Gimbal_Ctrl->pitch += 0.001f * (float)Module_Remote_get_channel(2);
            VAL_LIMIT(Gimbal_Ctrl->pitch, PITCH_MIN_ANGLE, PITCH_MAX_ANGLE);
        }
        else if (ch5 >= SBUS_CHX_DOWN - 100)
        {
            Gimbal_Ctrl->gimbal_mode  = gimbal_zero_force;
            Shoot_Ctrl->shoot_mode    = shoot_off;
            Shoot_Ctrl->friction_mode = friction_off;
            Shoot_Ctrl->load_mode     = load_stop;
        }
        else
        {
            Gimbal_Ctrl->gimbal_mode = gimbal_auto_mode;
        }

        /* 发射机构控制: ch6 控制模式, ch7 控制拨盘 (范围容差 ±100) */
        if (ch6 <= SBUS_CHX_UP + 100)
        {
            Shoot_Ctrl->shoot_mode    = shoot_off;
            Shoot_Ctrl->friction_mode = friction_off;
            Shoot_Ctrl->load_mode     = load_stop;
        }
        else if (ch6 >= SBUS_CHX_DOWN - 100)
        {
            Shoot_Ctrl->shoot_mode    = shoot_on;
            Shoot_Ctrl->friction_mode = friction_on;
            if (ch7 <= SBUS_CHX_UP + 100)
                Shoot_Ctrl->load_mode = load_1_bullet;
            else if (ch7 >= SBUS_CHX_DOWN - 100)
                Shoot_Ctrl->load_mode = load_burstfire;
            else
                Shoot_Ctrl->load_mode = load_stop;
        }
        else
        {
            Shoot_Ctrl->shoot_mode    = shoot_on;
            Shoot_Ctrl->friction_mode = friction_off;
        }

        /* 底盘控制: Ch3=vx(前后), Ch4=vy(左右), */
        Chassis_Ctrl->vx = (float)Module_Remote_get_channel(3);
        Chassis_Ctrl->vy = (float)Module_Remote_get_channel(4);
        // Ch8=模式 (范围容差 ±100)
        if (ch8 <= SBUS_CHX_UP + 100)
            Chassis_Ctrl->chassis_mode = chassis_follow_gimbal_yaw;
        else if (ch8 >= SBUS_CHX_DOWN - 100)
            Chassis_Ctrl->chassis_mode = chassis_rotate_reverse;
        else
            Chassis_Ctrl->chassis_mode = chassis_rotate;
    }
    else
    {
        Gimbal_Ctrl->gimbal_mode   = gimbal_zero_force;
        Chassis_Ctrl->chassis_mode = chassis_zero_force;
        Shoot_Ctrl->shoot_mode     = shoot_off;
        Shoot_Ctrl->friction_mode  = friction_off;
        Shoot_Ctrl->load_mode      = load_stop;
        Chassis_Ctrl->vx           = 0;
        Chassis_Ctrl->vy           = 0;
    }
}
