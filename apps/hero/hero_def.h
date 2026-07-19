/*
 * @Author: lsjdiad 949186291@qq.com
 * @Date: 2026-07-15 18:18:16
 * @LastEditors: lsjdiad 949186291@qq.com
 * @LastEditTime: 2026-07-18 14:26:14
 * @FilePath: \mas_embedded_threadx\apps\hero\hero_def.h
 * @Description:
 */
#ifndef _HERO_DEF_H_
#define _HERO_DEF_H_

#include <stdint.h>

// 云台参数
#define PITCH_MAX_ANGLE       20.0f  // 云台竖直方向最大角度
#define PITCH_MIN_ANGLE       -20.0f // 云台竖直方向最小角度
#define YAW_CHASSIS_ALIGN_ECD 3878   // 云台和底盘对齐指向相同方向时的电机编码器值,若对云台有机械改动需要修改
#define CHASSIS_MAX_SPEED_MPS 3.0f   // 底盘最大线速度 (m/s), 遥控满杆映射到此速度
#pragma pack(1)
// 云台
typedef enum
{
    gimbal_zero_force = 0, /* 云台停止模式 */
    gimbal_gyro_mode,      /*云台控制模式*/
    gimbal_auto_mode,      /*云台自动模式*/
} gimbal_mode_e;

typedef struct
{
    float         yaw;
    float         pitch;
    uint8_t       auto_search;
    gimbal_mode_e gimbal_mode;
} Gimbal_Ctrl_Cmd_t;
// 发射模式设置
typedef enum
{
    shoot_off = 0,
    shoot_on,
} shoot_mode_e;
typedef enum
{
    friction_off = 0, /* 摩擦轮关闭 */
    friction_on,      /* 摩擦轮开启 */
} friction_mode_e;
typedef enum
{
    load_stop = 0,  /* 停止发射 */
    load_reverse,   /* 反转 */
    load_1_bullet,  /* 单发 */
    load_burstfire, /* 连发 */
} loader_mode_e;

typedef struct
{
    shoot_mode_e    shoot_mode;
    loader_mode_e   load_mode;
    friction_mode_e friction_mode;
} Shoot_Ctrl_Cmd_t;
// 底盘模式设置
typedef enum
{
    chassis_zero_force = 0,    /* 底盘停止模式 */
    chassis_follow_gimbal_yaw, /* 跟随云台模式 */
    chassis_false,             /* 无底盘旋转 */
    chassis_rotate_reverse,    /* 旋转反向 */
} chassis_mode_e;

typedef struct
{
    float          vx;
    float          vy;
    float          wz;
    float          offset_angle;
    chassis_mode_e chassis_mode;
} Chassis_Ctrl_Cmd_t;

#pragma pack()

#endif // _HERO_DEF_H_
