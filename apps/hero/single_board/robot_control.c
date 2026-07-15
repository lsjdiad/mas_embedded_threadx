#include "robot_control.h"
#include "hero_def.h"
#include "chassis_func.h"
#include "robot_func.h"
#include "tx_api.h"
#include "bsp_def.h"

#define LOG_TAG "app_robot_control"
#define LOG_LVL LOG_LVL_INFO
#include "ulog_def.h"

static TX_THREAD                  robot_control_thread;
APPS_STACK_SECTION static uint8_t robot_control_thread_stack[1024];
static Chassis_Ctrl_Cmd_t         chassis_cmd;

static void robot_control_task(ULONG thread_input)
{
    (void)thread_input;
    while (1)
    {
        RemoteControlSet(&chassis_cmd);
        chassis_func(&chassis_cmd);
        tx_thread_sleep(2);
    }
}

void robot_control_init(void)
{
    chassis_init();
    UINT status = tx_thread_create(&robot_control_thread, "robot_control_thread", robot_control_task, 0,
                                    robot_control_thread_stack, 1024, 30, 30,
                                    TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        LOG_E("robot_control_task failed!");
        return;
    }
    LOG_I("robot_control init success!");
}
