#include "mcu_init.h"
#include "tos.h"
#include "cmsis_os.h"
#include "tiny_logo.h"

/* 定义任务栈空间大小 */
#define TASK_STACK_SIZE_DEMO 512

/* 定义任务优先级 */
#define TASK_PRIO_DEMO 4

/* 定义任务时间片大小, 0 为系统默认大小 */
#define TASK_TIMESLICE_DEMO 0

/* 定义任务栈空间 */
k_stack_t task_stack_demo[TASK_STACK_SIZE_DEMO];

/* 定义任务句柄 */
k_task_t task_demo_handler;

/* 声明任务函数 */
void task_entry_demo(void *arg);

#define MMBLK_BLK_NUM 5
#define MMBLK_BLK_SIZE 0x20

k_mmblk_pool_t mmblk_pool;

uint8_t mmblk_pool_buffer[MMBLK_BLK_NUM * MMBLK_BLK_SIZE];

void *p[MMBLK_BLK_NUM] = {K_NULL};

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
    /* 创建内存块管理池 */
    tos_mmblk_pool_create(&mmblk_pool, mmblk_pool_buffer, MMBLK_BLK_NUM, MMBLK_BLK_SIZE);
    tos_task_create(&task_demo_handler, "demo", task_entry_demo,
                    NULL, TASK_PRIO_DEMO, task_stack_demo,
                    TASK_STACK_SIZE_DEMO, TASK_TIMESLICE_DEMO);
    osKernelStart();
}

void task_entry_demo(void *arg)
{
    k_err_t err;
    void *p_dummy = K_NULL;
    int i = 0;
    printf("[%d|%s]mmblk_pool has %d blocks, size of each block is 0x%X\r\n",
           __LINE__, __FUNCTION__, MMBLK_BLK_NUM, MMBLK_BLK_SIZE);
    for (i = 0; i < MMBLK_BLK_NUM; i++)
    {
        err = tos_mmblk_alloc(&mmblk_pool, &p[i]);
        if (err == K_ERR_NONE)
        {
            printf("[%d|%s]%d block alloced: 0x%X\r\n", __LINE__, __FUNCTION__, i, (uint32_t)p[i]);
        }
        else
        {
            printf("[%d|%s]should not happen\r\n", __LINE__, __FUNCTION__);
        }
    }

    err = tos_mmblk_alloc(&mmblk_pool, &p_dummy);
    if (err == K_ERR_MMBLK_POOL_EMPTY)
    {
        printf("[%d|%s]block exhausted, all block is alloced\r\n", __LINE__, __FUNCTION__);
    }
    else
    {
        printf("[%d|%s]should not happen\r\n", __LINE__, __FUNCTION__);
    }

    for (i = 0; i < MMBLK_BLK_NUM; i++)
    {
        err = tos_mmblk_free(&mmblk_pool, p[i]);
        if (err != K_ERR_NONE)
        {
            printf("[%d|%s]should not happen\r\n", __LINE__, __FUNCTION__);
        }
    }

    err = tos_mmblk_free(&mmblk_pool, p[0]);
    if (err == K_ERR_MMBLK_POOL_FULL)
    {
        printf("[%d|%s]pool is full\r\n", __LINE__, __FUNCTION__);
    }
    else
    {
        printf("[%d|%s]should not happen\r\n", __LINE__, __FUNCTION__);
    }
}
