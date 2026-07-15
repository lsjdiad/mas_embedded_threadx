/*
 * @Author: userName userEmail
 * @Date: 2026-07-15 18:19:31
 * @LastEditors: userName userEmail
 * @LastEditTime: 2026-07-15 19:12:18
 * @FilePath: \mas_embedded_threadx\apps\hero\single_board\robot_func\robot_func.c
 * @Description:
 */
#include "robot_func.h"
#include "module_remote.h"
#include <string.h>

void RemoteControlSet(Chassis_Ctrl_Cmd_t *Chassis_Ctrl)
{

    // TODO: 遥控器 → 底盘命令映射
    if (!Chassis_Ctrl) return;
    memset(Chassis_Ctrl, 0, sizeof(*Chassis_Ctrl));
}
