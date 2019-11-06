#include "mcu_init.h"
#include "cmsis_os.h"
#include "tiny_logo.h"

#define APPLICATION_TASK_STK_SIZE 1 * 1024
extern void application_entry(void *arg);
osThreadDef(application_entry, osPriorityNormal, 1, APPLICATION_TASK_STK_SIZE);

k_task_t task1;                                // 任务结构体
void task1_entry(void *arg);                   // 任务入口函数
uint32_t task1_arg_prio4_array[3] = {1, 2, 3}; // 任务入口函数参数
uint32_t task1_arg_prio3_array[3] = {4, 5, 6}; // 任务入口函数参数
#define TASK1_PRIO 4                           // 任务优先级
#define TASK1_STACK_SIZE 512                   // 任务栈空间大小
#define TASK1_TIME_SLICE 0                     // 任务时间片大小,0 为系统默认值
k_stack_t task1_stack[TASK1_STACK_SIZE];       // 任务栈空间

k_task_t task2;                           // 任务结构体
void task2_entry(void *arg);              // 任务入口函数
char *task2_arg_prio4_str = "Hello Tiny"; // 任务入口函数参数
#define TASK2_PRIO 4                      // 任务优先级
#define TASK2_STACK_SIZE 1 * 1024         // 任务栈空间大小
#define TASK2_TIME_SLICE 0                // 任务时间片大小,0 为系统默认值
k_stack_t task2_stack[TASK2_STACK_SIZE];  // 任务栈空间

/******************************************************************************/
__weak void application_entry(void *arg)
{
    while (K_TRUE)
    {
        printf("[%d:%s]TencentOS-tiny kernel is runing: %s\r\n",
               __LINE__, __FUNCTION__, tos_knl_is_running() != 0 ? "True" : "False");
        tos_task_delay(10 * 1000);
    }
}

/********************************************************/
//LCD刷屏时使用的颜色
int lcd_discolor[13] = {WHITE, BLUE, BRED, GRED, GBLUE, RED, MAGENTA,
                        GREEN, CYAN, YELLOW, BROWN, BRRED, GRAY};

/********************************************************/

int main(void)
{
    /* 初始化硬件 */
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

    /* 初始化内核,创建测试任务 */
    osKernelInitialize();
    // osThreadCreate(osThread(application_entry), NULL);

    tos_task_create(&task1, "task1", task1_entry, task1_arg_prio4_array,
                    TASK1_PRIO, task1_stack, TASK1_STACK_SIZE, 0);

    tos_task_create(&task2, "task2", task2_entry, task2_arg_prio4_str,
                    TASK2_PRIO, task2_stack, TASK2_STACK_SIZE, TASK2_TIME_SLICE);

    /* 启动内核 */
    osKernelStart();
}

/******************************************************************************/

void dump_uint32_array(uint32_t *array, size_t array_len)
{
    printf("\r\n----------\r\n");
    printf("dump uint32 data:\r\n");
    for (int i = 0; i < array_len; i++)
    {
        printf("%5d\t", array[i]);
        if (i >= 10)
        {
            printf("\r\n");
        }
    }
    printf("\r\n----------\r\n");
}

void task1_entry(void *arg)
{
    uint32_t *array = (uint32_t *)arg;

    printf("[%d:%s]: start\r\n", __LINE__, __FUNCTION__);
    dump_uint32_array(array, 3);

    uint8_t task1_count = 0;
    POINT_COLOR = BLUE;
    LCD_ShowString(10, 120, 215, 12, 12, "Task1 Run:");
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(10, 140, 90, 220);

    while (K_TRUE)
    {
        LCD_Fill(11, 141, 89, 219, lcd_discolor[task1_count++ % 13]);
        LCD_ShowNum(75, 120, task1_count, 3, 12);
        HAL_GPIO_TogglePin(GPIOE, LED_R_Pin);

        printf("[%d:%s]: runing...\r\n", __LINE__, __FUNCTION__);
        tos_task_delay(300);
    }
}

void task2_entry(void *arg)
{
    int i = 0;
    char *task2_arg = (char *)arg;
    printf("[%d:%s]: start\r\n", __LINE__, __FUNCTION__);
    printf("task2_arg:%s\r\n", task2_arg);

    uint8_t task2_count = 0;
    POINT_COLOR = BLUE;
    LCD_ShowString(135, 120, 215, 12, 12, "Task2 Run:");
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(135, 140, 215, 220);

    while (K_TRUE)
    {
        LCD_Fill(136, 141, 214, 219, lcd_discolor[task2_count++ % 13]);
        LCD_ShowNum(195, 120, task2_count, 3, 12);
        HAL_GPIO_TogglePin(GPIOE, LED_B_Pin);
        printf("i = :%d\r\n", i);

        switch (i)
        {
        case 2:
            /* 挂起 task1 */
            tos_task_suspend(&task1);
            break;

        case 4:
            /* 恢复挂起 task1 */
            tos_task_resume(&task1);
            break;

        case 6:
            /* 销毁 task1 */
            tos_task_destroy(&task1);
            break;

        case 10:
            // /* 重新创建 task1 */
            tos_task_create(&task1, "task1", task1_entry, task1_arg_prio3_array,
                            TASK1_PRIO, task1_stack, TASK1_STACK_SIZE, 0);
            break;

        case 12:
            /* 更改 task1 优先级 */
            tos_task_prio_change(&task1, TASK1_PRIO - 1);
            break;

        case 20:
            /* 销毁 task1,task2 */
            tos_task_destroy(&task1);
            tos_task_destroy(&task2);
            break;

        default:
            break;
        }

        tos_task_delay(800);
        i++;
    }
}
