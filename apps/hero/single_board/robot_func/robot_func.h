#ifndef _ROBOT_FUNC_H_
#define _ROBOT_FUNC_H_

#include "hero_def.h"

/**
 * @brief 根据遥控器输入设置云台、发射和底盘控制命令
 * @param Gimbal_Ctrl  云台控制命令结构体指针
 * @param Shoot_Ctrl   发射控制命令结构体指针
 * @param Chassis_Ctrl 底盘控制命令结构体指针
 */
void RemoteControlSet(Gimbal_Ctrl_Cmd_t *Gimbal_Ctrl, Shoot_Ctrl_Cmd_t *Shoot_Ctrl, Chassis_Ctrl_Cmd_t *Chassis_Ctrl);

#endif // _ROBOT_FUNC_H_
