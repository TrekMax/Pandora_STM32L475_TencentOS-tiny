#include "mcu_init.h"
#include "tos.h"
#include "cmsis_os.h"
#include "tiny_logo.h"

#define APPLICATION_TASK_STK_SIZE 4096
extern void application_entry(void *arg);
osThreadDef(application_entry, osPriorityNormal, 1, APPLICATION_TASK_STK_SIZE);

/* 定义任务栈空间大小 */
#define TASK_STACK_SIZE_PRODUCER 512
#define TASK_STACK_SIZE_CONSUMER 512

/* 定义任务优先级 */
#define TASK_PRIO_PRODUCER 4
#define TASK_PRIO_CONSUMER 4

/* 定义任务时间片大小, 0 为系统默认大小 */
#define TASK_TIMESLICE_PRODUCER 0
#define TASK_TIMESLICE_CONSUMER 0

/* 定义任务栈空间 */
k_stack_t task_stack_producer[TASK_STACK_SIZE_PRODUCER];
k_stack_t task_stack_consumer[TASK_STACK_SIZE_CONSUMER];

/* 定义任务句柄 */
k_task_t task_producer_handler;
k_task_t task_consumer_handler;

/* 声明任务函数 */
void task_entry_producer(void *arg);
void task_entry_consumer(void *arg);

/* 定义互斥量,信号量,定义共享资源 */
k_mutex_t buffer_locker;
k_sem_t full;
k_sem_t empty;

#define RESOURCE_COUNT_MAX 10

struct resource_st
{
    int cursor;
    uint32_t buffer[RESOURCE_COUNT_MAX];
} resource = {0, {0}};

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

    /* 创建互斥量,信号量 */
    tos_mutex_create(&buffer_locker);
    tos_sem_create(&empty, RESOURCE_COUNT_MAX);
    tos_sem_create(&full, 0);

    tos_task_create(&task_producer_handler, "task_producer_resource", task_entry_producer, NULL,
                    TASK_PRIO_PRODUCER, task_stack_producer,
                    TASK_STACK_SIZE_PRODUCER, TASK_TIMESLICE_PRODUCER);

    tos_task_create(&task_consumer_handler, "task_consumer_resource", task_entry_consumer, NULL,
                    TASK_PRIO_CONSUMER, task_stack_consumer,
                    TASK_STACK_SIZE_CONSUMER, TASK_TIMESLICE_CONSUMER);

    osKernelStart();
}

static void product_item(int salt)
{
    resource.buffer[resource.cursor++] = salt;
    printf("-->produce item resource.buffer[%d]: %d\r\n", resource.cursor, salt);
}

/* 生产任务 */
void task_entry_producer(void *arg)
{
    k_err_t err;
    size_t salt = 0;
    printf("[%d|%s]:start\r\n", __LINE__, __FUNCTION__);
    while (K_TRUE)
    {
        
        err = tos_sem_pend(&empty, TOS_TIME_FOREVER);
        if (err != K_ERR_NONE)
        {
            continue;
        }

        /* 锁住共享资源 */
        err = tos_mutex_pend(&buffer_locker);
        if (err != K_ERR_NONE)
        {
            continue;
        }
        product_item(salt);
        tos_mutex_post(&buffer_locker);
        /* 共享资源为空时,则阻塞消费者继续消费数据 */
        tos_sem_post(&full);
        tos_task_delay(800);
        if (resource.cursor > 8) {
            tos_task_delay(3000);
        }
        ++salt;
    }
}
static void consume_item(void)
{
    --resource.cursor;
    printf("consume item resource.buffer[%d]: %d\t", resource.cursor, resource.buffer[resource.cursor]);
    printf("\r\n");
}

void task_entry_consumer(void *arg)
{
    k_err_t err;

    printf("[%d|%s]:start\r\n", __LINE__, __FUNCTION__);
    while (K_TRUE)
    {
        
        err = tos_sem_pend(&full, TOS_TIME_FOREVER);
        if (err != K_ERR_NONE)
        {
            continue;
        }

        /* 锁住共享资源 */
        err = tos_mutex_pend(&buffer_locker);
        if (err != K_ERR_NONE)
        {
            continue;
        }
        consume_item();
        tos_mutex_post(&buffer_locker);
        
        /* 共享资源为满时,则阻塞生产者继续消费数据 */
        tos_sem_post(&empty);
        if (resource.cursor > 4) {
            tos_task_delay(500);
        }
        else
        {
            /* code */
            tos_task_delay(2000);
        }
        
    }
}
