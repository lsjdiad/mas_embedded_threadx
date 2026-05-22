#include "lqr.h"
#include <arm_math.h>
#include <math.h>
#include <string.h>

static void LQR_ErrorHandle(LQRInstance *lqr)
{
    const float    OUTPUT_SAT_RATIO = 0.95f; /* 输出饱和阈值 */
    const float    REF_MIN_DETECT   = 0.5f;  /* 目标过小不检测, 避免静止误判 */
    const float    STALL_RATIO      = 0.05f; /* 实测值不到目标 5% 判定为堵转 */
    const uint16_t STALL_COUNT      = 500;

    float out_abs     = fabsf(lqr->output);
    float ref_abs     = fabsf(lqr->ref);
    float measure_abs = fabsf(lqr->measure);

    /* 输出未饱和或者目标值过小不检测 */
    if (out_abs < lqr->max_out * OUTPUT_SAT_RATIO || ref_abs < REF_MIN_DETECT)
    {
        lqr->blocked_count = 0;
        lqr->errortype     = LQR_ERROR_NONE;
        return;
    }

    /* 堵转判定: 期望有运动但实际几乎无响应 */
    if (measure_abs < ref_abs * STALL_RATIO)
    {
        lqr->blocked_count++;
    }
    else
    {
        lqr->blocked_count = 0;
        lqr->errortype     = LQR_ERROR_NONE;
    }

    if (lqr->blocked_count >= STALL_COUNT) lqr->errortype = LQR_MOTOR_BLOCKED_ERROR;

    if (lqr->errortype == LQR_MOTOR_BLOCKED_ERROR) lqr->output = 0.0f;
}

void LQRInit(LQRInstance *lqr, LQR_Init_Config_s *config)
{
    if (lqr == NULL || config == NULL) return;

    memset(lqr, 0, sizeof(LQRInstance));

    lqr->state_dim = config->state_dim;
    if (lqr->state_dim < 1 || lqr->state_dim > 2) return;
    lqr->max_out = config->max_out;

    for (uint8_t i = 0; i < lqr->state_dim; i++) lqr->K[i] = config->K[i];
}

float LQRCalculate(LQRInstance *lqr, float rad_degree, float rad_angular_velocity, float rad_ref)
{
    if (lqr == NULL || lqr->state_dim < 1 || lqr->state_dim > 2) return 0.0f;

    /* 计算 LQR 原始输出 */
    if (lqr->state_dim == 1)
    {
        lqr->measure = rad_angular_velocity;
        lqr->ref     = rad_ref;
        float err    = lqr->measure - lqr->ref;
        lqr->output  = -(lqr->K[0] * err);
    }
    else /* state_dim == 2 */
    {
        lqr->measure = rad_degree;
        lqr->ref     = rad_ref;
        float err    = lqr->measure - lqr->ref;
        lqr->output  = -(lqr->K[0] * err) - lqr->K[1] * rad_angular_velocity;
    }

    /* 堵转保护 */
    LQR_ErrorHandle(lqr);

    return lqr->output;
}
