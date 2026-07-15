#ifndef _HERO_DEF_H_
#define _HERO_DEF_H_

#include <stdint.h>

// 底盘参数
#define CHASSIS_MAX_SPEED_MPS 3.0f // 底盘最大线速度 (m/s), 遥控满杆映射到此值

#pragma pack(1)

// 底盘模式
typedef enum {
    chassis_zero_force = 0, // 无力
    chassis_rotate,         // 自旋
    chassis_rotate_reverse, // 反向自旋
} chassis_mode_e;

typedef struct {
    float          vx; // 前进速度 (m/s)
    float          vy; // 横移速度 (m/s)
    float          wz; // 旋转速度 (rad/s)
    chassis_mode_e chassis_mode;
} Chassis_Ctrl_Cmd_t;

#pragma pack()

#endif // _HERO_DEF_H_
