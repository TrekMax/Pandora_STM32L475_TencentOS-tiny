#include "mcu_init.h"
#include "tos.h"
#include "cmsis_os.h"
#include "tiny_logo.h"

/* 定义任务栈空间大小 */
#define TASK_STACK_SIZE_LISTENER 512
#define TASK_STACK_SIZE_TRIGGER 512

/* 定义任务优先级 */
#define TASK_PRIO_LISTENER1 4
#define TASK_PRIO_LISTENER2 4
#define TASK_PRIO_TRIGGER 4

/* 定义任务时间片大小, 0 为系统默认大小 */
#define TASK_TIMESLICE_LISTENER1 0
#define TASK_TIMESLICE_LISTENER2 0
#define TASK_TIMESLICE_TRIGGER 0

/* 定义任务栈空间 */
k_stack_t task_stack_listener1[TASK_STACK_SIZE_LISTENER];
k_stack_t task_stack_listener2[TASK_STACK_SIZE_LISTENER];
k_stack_t task_stack_trigger[TASK_STACK_SIZE_TRIGGER];

/* 定义任务句柄 */
k_task_t task_listener1_handler;
k_task_t task_listener2_handler;
k_task_t task_trigger_handler;

/* 声明任务函数 */
void task_entry_listener1(void *arg);
void task_entry_listener2(void *arg);
void task_entry_trigger(void *arg);

const k_event_flag_t event_eeny = (k_event_flag_t)(1 << 0);
const k_event_flag_t event_meeny = (k_event_flag_t)(1 << 1);
const k_event_flag_t event_miny = (k_event_flag_t)(1 << 2);
const k_event_flag_t event_moe = (k_event_flag_t)(1 << 3);

k_event_t event;
/********************************************************/
//LCD刷屏时使用的颜色
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

    tos_event_create(&event, (k_event_flag_t)0u);

    tos_task_create(&task_listener1_handler, "task_listener1", task_entry_listener1, NULL,
                    TASK_PRIO_LISTENER1, task_stack_listener1,
                    TASK_STACK_SIZE_LISTENER, TASK_TIMESLICE_LISTENER1);

    tos_task_create(&task_listener2_handler, "task_listener2", task_entry_listener2, NULL,
                    TASK_PRIO_LISTENER2, task_stack_listener2,
                    TASK_STACK_SIZE_LISTENER, TASK_TIMESLICE_LISTENER2);

    tos_task_create(&task_trigger_handler, "task_trigger", task_entry_trigger, NULL,
                    TASK_PRIO_TRIGGER, task_stack_trigger,
                    TASK_STACK_SIZE_TRIGGER, TASK_TIMESLICE_TRIGGER);

    osKernelStart();
}

/******************************************************************************/

void task_entry_trigger(void *arg)
{
    int i = 0;
    while (K_TRUE)
    {
        switch (i)
        {
        case 2:
            printf("[%d|%s]: event_eeny whill come\r\n", __LINE__, __FUNCTION__);
            tos_event_post(&event, event_eeny);
            break;
        case 3:
            printf("[%d|%s]: event_meeny whill come\r\n", __LINE__, __FUNCTION__);
            tos_event_post(&event, event_meeny);
            break;
        case 4:
            printf("[%d|%s]: event_miny whill come\r\n", __LINE__, __FUNCTION__);
            tos_event_post(&event, event_miny);
            break;
        case 5:
            printf("[%d|%s]: event_moe whill come\r\n", __LINE__, __FUNCTION__);
            tos_event_post(&event, event_moe);
            break;
        case 6:
            printf("[%d|%s]: all event whill come\r\n", __LINE__, __FUNCTION__);
            tos_event_post(&event, event_eeny | event_meeny | event_miny | event_moe);
            break;

        default:
            break;
        }
        tos_task_delay(1000);
        i++;
    }
}

void task_entry_listener1(void *arg)
{
    k_event_flag_t flag_match;
    k_err_t err;
    while (K_TRUE)
    {
        err = tos_event_pend(&event, (event_eeny | event_meeny | event_miny | event_moe),
                             &flag_match, TOS_TIME_FOREVER, TOS_OPT_EVENT_PEND_ALL);
        if (err == K_ERR_NONE)
        {
            if (flag_match == (event_eeny | event_meeny | event_miny | event_moe))
            {
                printf("[%d|%s]: all comes\r\n", __LINE__, __FUNCTION__);
            }
        }
    }
}

void task_entry_listener2(void *arg)
{
    k_event_flag_t flag_match;
    k_err_t err;
    while (K_TRUE)
    {
        err = tos_event_pend(&event, (event_eeny | event_meeny | event_miny | event_moe),
                             &flag_match, TOS_TIME_FOREVER, TOS_OPT_EVENT_PEND_ANY | TOS_OPT_EVENT_PEND_CLR);
        if (err == K_ERR_NONE)
        {
            switch (flag_match)
            {
            case event_eeny:
                printf("[%d|%s]: eeny comes\r\n", __LINE__, __FUNCTION__);
                break;

            case event_meeny:
                printf("[%d|%s]: event_meeny comes\r\n", __LINE__, __FUNCTION__);

                break;

            case event_miny:
                printf("[%d|%s]: event_miny comes\r\n", __LINE__, __FUNCTION__);

                break;

            case event_moe:
                printf("[%d|%s]: event_moe comes\r\n", __LINE__, __FUNCTION__);

                break;

            case (event_eeny | event_meeny | event_miny | event_moe):
                printf("[%d|%s]: all comes\r\n", __LINE__, __FUNCTION__);

                break;

            default:
                break;
            }
        }
    }
}
