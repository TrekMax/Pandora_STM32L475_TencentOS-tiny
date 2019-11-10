#include "mcu_init.h"
#include "tos.h"
#include "cmsis_os.h"
#include "tiny_logo.h"

#define TASK_STACK_SIZE_DEMO 512                 // 定义任务栈空间大小
#define TASK_PRIO_DEMO 4                         // 定义任务优先级
#define TASK_TIMESLICE_DEMO 0                    // 定义任务时间片大小, 0 为系统默认大小
k_stack_t task_stack_demo[TASK_STACK_SIZE_DEMO]; // 定义任务栈空间
k_task_t task_demo_handler;                      // 定义任务句柄
void task_entry_demo(void *arg);                 // 声明任务函数

/********************************************************/
//LCD 刷屏时使用的颜色
int lcd_discolor[13] = {WHITE, BLUE, BRED, GRED, GBLUE, RED, MAGENTA,
                        GREEN, CYAN, YELLOW, BROWN, BRRED, GRAY};

/********************************************************/

int main(void)
{
    board_init();
    LCD_DisplayOn();
    LCD_Show_Image(10, 14, 220, 50, tiny_logo);

    BACK_COLOR = WHITE;
    POINT_COLOR = BLUE;
    LCD_ShowString(10, 64, 240, 24, 24, "Pandora STM32L475");

    POINT_COLOR = RED;
    LCD_ShowString(10, 96, 240, 16, 16, "QinYUN575"
                                        "@" __DATE__);

    printf("\r\n===========================================================\r\n");
    printf("\r\n");
    printf("\tWelcome to TencentOS tiny\r\n");
    printf("\tCompile Time:%s, %s\r\n", __TIME__, __DATE__);
    printf("\tCoding by QinYUN575\r\n");
    printf("\r\n");
    printf("===========================================================\r\n");

    osKernelInitialize();
    tos_task_create(&task_demo_handler, "demo", task_entry_demo,
                    NULL, TASK_PRIO_DEMO, task_stack_demo,
                    TASK_STACK_SIZE_DEMO, TASK_TIMESLICE_DEMO);
    osKernelStart();
}

/******************************************************************************/
void tmr_oneshort_handler_cb(void *arg)
{
    printf("[%d|%s]this is oneshort timer callback handler functions,current systick: %lld\r\n",
           __LINE__, __FUNCTION__, tos_systick_get());
}

void tmr_periodic_handler_cb(void *arg)
{
    printf("[%d|%s]this is periodic timer callback handler functions,current systick: %lld\r\n",
           __LINE__, __FUNCTION__, tos_systick_get());
}

void task_entry_demo(void *arg)
{
    k_timer_t tmr_onshot;
    k_timer_t tmr_periodic;

    /* 创建单次性定时器 */
    tos_timer_create(&tmr_onshot, 4000, 0, tmr_oneshort_handler_cb,
                     K_NULL, TOS_OPT_TIMER_ONESHOT);

    /* 创建周期性定时器 */
    tos_timer_create(&tmr_periodic, 1000, 2000, tmr_periodic_handler_cb,
                     K_NULL, TOS_OPT_TIMER_PERIODIC);

    tos_timer_start(&tmr_onshot);
    tos_timer_start(&tmr_periodic);
    printf("[%d|%s]current systick: %lld\r\n", __LINE__, __FUNCTION__, tos_systick_get());

    while (1)
    {
        tos_task_delay(5000);
    }
}
