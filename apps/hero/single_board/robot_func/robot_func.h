#ifndef _ROBOT_FUNC_H_
#define _ROBOT_FUNC_H_

#include "hero_def.h"

/**
 * @brief 根据遥控器输入设置底盘控制命令
 * @param Chassis_Ctrl 底盘控制命令结构体指针
 */
void RemoteControlSet(Chassis_Ctrl_Cmd_t *Chassis_Ctrl);

#endif // _ROBOT_FUNC_H_
