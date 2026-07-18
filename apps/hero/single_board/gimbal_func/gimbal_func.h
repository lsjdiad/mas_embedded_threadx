#ifndef _GIMBAL_FUNC_H_
#define _GIMBAL_FUNC_H_

#include "hero_def.h"

void gimbal_init(void);

void gimbal_func(Gimbal_Ctrl_Cmd_t *gimbal_cmd, uint16_t *yaw_ecd);

float gimbal_get_yaw_angle_deg(void);
float gimbal_get_yaw_normalized_deg(void);

#endif // _GIMBAL_FUNC_H_