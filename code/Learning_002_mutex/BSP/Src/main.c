#include "mcu_init.h"
#include "tos.h"
#include "cmsis_os.h"
#include "tiny_logo.h"

#define APPLICATION_TASK_STK_SIZE 4096
extern void application_entry(void *arg);
osThreadDef(application_entry, osPriorityNormal, 1, APPLICATION_TASK_STK_SIZE);

/* 定义任务栈空间大小 */
#define TASK_WRITER_STACK_SIZE 512
#define TASK_READER_STACK_SIZE 512

/* 定义任务优先级 */
#define TASK_WRITER_PRIO 4
#define TASK_READER_PRIO 4

/* 定义任务时间片大小, 0 为系统默认大小 */
#define TASK_WRITER_TIMESLICE 0
#define TASK_READER_TIMESLICE 0

/* 定义任务栈空间 */
k_stack_t task_writer_stack[TASK_WRITER_STACK_SIZE];
k_stack_t task_reader_stack[TASK_READER_STACK_SIZE];

k_task_t task_writer;
k_task_t task_reader;

/* 声明任务函数 */
void task_writer_entry(void *arg);
void task_reader_entry(void *arg);

/* 定义资源互斥锁 */
k_mutex_t critical_resource_locker;

/* 定义互斥资源 */
static uint32_t critical_resource[3];

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
    // osThreadCreate(osThread(application_entry), NULL);
    /* 创建互斥量 */
    tos_mutex_create(&critical_resource_locker);
    tos_task_create(&task_writer, "task_writer_resource", task_writer_entry, NULL,
                    TASK_WRITER_PRIO, task_writer_stack,
                    TASK_WRITER_STACK_SIZE, TASK_WRITER_TIMESLICE);

    tos_task_create(&task_reader, "task_reader_resource", task_reader_entry, NULL,
                    TASK_READER_PRIO, task_reader_stack,
                    TASK_READER_STACK_SIZE, TASK_READER_TIMESLICE);

    osKernelStart();
}

static void write_critical_resource(int salt)
{
    printf("-----------------------------\r\n");
    printf("write critical resource\r\n");
    for (int i = 0; i < 3; i++)
    {
        critical_resource[i] = salt + i;
        printf("%d\t", critical_resource[i]);
    }
    printf("\r\n");
}

void task_writer_entry(void *arg)
{
    size_t salt = 0;
    k_err_t err;
    printf("[%d|%s]:start\r\n", __LINE__, __FUNCTION__);
    while (K_TRUE)
    {
        /* 尝试获取互斥量(互斥锁) */
        err = tos_mutex_pend(&critical_resource_locker);
        if (err == K_ERR_NONE)
        {
            /* 更改共享资源 */
            write_critical_resource(salt);
            /* 释放互斥量(互斥锁)) */
            tos_mutex_post(&critical_resource_locker);
        }
        tos_task_delay(1000);
        salt++;
    }
}

static void read_critical_resource(void)
{
    printf("-----------------------------\r\n");
    printf("reade critical resource\r\n");
    for (int i = 0; i < 3; i++)
    {
        printf("%d\t", critical_resource[i]);
    }
    printf("\r\n");
}

void task_reader_entry(void *arg)
{
    k_err_t err;
    printf("[%d|%s]:start\r\n", __LINE__, __FUNCTION__);
    while (K_TRUE)
    {
        /* 尝试获取互斥量(互斥锁),永久阻塞 */
        err = tos_mutex_pend(&critical_resource_locker);
        if (err == K_ERR_NONE)
        {
            /* 读取共享资源 */
            read_critical_resource();
            /* 释放互斥量(互斥锁) */
            tos_mutex_post(&critical_resource_locker);
        }
        tos_task_delay(1000);
    }
}
