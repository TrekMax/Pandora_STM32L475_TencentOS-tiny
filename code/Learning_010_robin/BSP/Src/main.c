#include "mcu_init.h"
#include "tos.h"
#include "cmsis_os.h"
#include "tiny_logo.h"

#define TASK_STACK_SIZE_DEMO 512              // 定义任务栈空间大小
#define TASK_STACK_SIZE_SAMPLE 512            // 定义任务栈空间大小
#define TASK_PRIO_DEMO 4                      // 定义任务优先级
#define TASK_PRIO_SAMPLE (TASK_PRIO_DEMO - 1) // 定义任务优先级

k_stack_t task_stack_demo1[TASK_STACK_SIZE_DEMO];    // 定义任务栈空间
k_stack_t task_stack_demo2[TASK_STACK_SIZE_DEMO];    // 定义任务栈空间
k_stack_t task_stack_sample[TASK_STACK_SIZE_SAMPLE]; // 定义任务栈空间

k_task_t task_demo1_handler;  // 定义任务句柄
k_task_t task_demo2_handler;  // 定义任务句柄
k_task_t task_sample_handler; // 定义任务句柄

const k_timeslice_t timeslice_demo1 = 10;
const k_timeslice_t timeslice_demo2 = 30;

uint64_t demo1_counter = 0;
uint64_t demo2_counter = 0;

void task_entry_demo1(void *arg);  // 声明任务函数
void task_entry_demo2(void *arg);  // 声明任务函数
void task_entry_sample(void *arg); // 声明任务函数

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
    tos_robin_default_timeslice_config((k_timeslice_t)500u);
    tos_task_create(&task_demo1_handler, "demo1", task_entry_demo1, K_NULL,
                    TASK_PRIO_DEMO, task_stack_demo1, TASK_STACK_SIZE_DEMO, timeslice_demo1);
    tos_task_create(&task_demo2_handler, "demo2", task_entry_demo2, K_NULL,
                    TASK_PRIO_DEMO, task_stack_demo2, TASK_STACK_SIZE_DEMO, timeslice_demo2);
    tos_task_create(&task_sample_handler, "sample", task_entry_sample, K_NULL,
                    TASK_PRIO_SAMPLE, task_stack_sample, TASK_STACK_SIZE_SAMPLE, 0);

    osKernelStart();
}

/******************************************************************************/

void task_entry_demo1(void *arg)
{
    printf("[%d|%s] Start\n", __LINE__, __FUNCTION__);
    while (K_TRUE)
    {
        ++demo1_counter;
        if (demo1_counter%1000 == 0)
        {
            HAL_GPIO_TogglePin(GPIOE, LED_R_Pin);
        }
    }
}

void task_entry_demo2(void *arg)
{
    printf("[%d|%s] Start\n", __LINE__, __FUNCTION__);
    while (K_TRUE)
    {
        ++demo2_counter;
        if (demo2_counter%1000 == 0)
        {
            HAL_GPIO_TogglePin(GPIOE, LED_B_Pin);
        }
    }
}

void task_entry_sample(void *arg)
{
    uint64_t i = 0;
    while (K_TRUE)
    {
        printf("[%lld]------------------------\n", i++);
        printf("demo1_counter: %lld\n", demo1_counter);
        printf("demo2_counter: %lld\n", demo2_counter);
        printf("demo2_counter / demo1_counter = %f\n",
               (double)demo2_counter / demo1_counter);
        printf("should almost equals to:\n");
        printf("timeslice_demo2 / timeslice_demo1 = %f\n\n", (double)timeslice_demo2 / timeslice_demo1);
        tos_task_delay(1000);
    }
}
