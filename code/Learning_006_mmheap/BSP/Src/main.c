#include "mcu_init.h"
#include "tos.h"
#include "cmsis_os.h"
#include "tiny_logo.h"

/* 定义任务栈空间大小 */
#define TASK_STACK_SIZE_MMHEAP_DEMO 512

/* 定义任务优先级 */
#define TASK_PRIO_MMHEAP_DEMO 4

/* 定义任务时间片大小, 0 为系统默认大小 */
#define TASK_TIMESLICE_MMHEAP_DEMO 0

/* 定义任务栈空间 */
k_stack_t task_stack_mmheap_demo[TASK_STACK_SIZE_MMHEAP_DEMO];

/* 定义任务句柄 */
k_task_t task_mmheap_demo_handler;

/* 声明任务函数 */
void task_entry_mmheap_demo(void *arg);
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

    tos_task_create(&task_mmheap_demo_handler, "mmheap_demo", task_entry_mmheap_demo,
                    NULL, TASK_PRIO_MMHEAP_DEMO, task_stack_mmheap_demo,
                    TASK_STACK_SIZE_MMHEAP_DEMO, TASK_TIMESLICE_MMHEAP_DEMO);
    osKernelStart();
}

/******************************************************************************/
void task_entry_mmheap_demo(void *arg)
{
    void *p = K_NULL, *p_aligned = NULL;
    int i = 0;
    k_mmheap_info_t info;
    while (K_TRUE)
    {
        tos_mmheap_check(&info);
        printf("[%3d|%s] i[%d], mmheap free:%d used:%d\n", __LINE__, __FUNCTION__,
               i, info.free, info.used);
        switch (i)
        {
        case 1:
            p = tos_mmheap_alloc(0x30);
            if (p != K_NULL)
            {
                printf("[%3d|%s] p alloc:0x%x\n", __LINE__, __FUNCTION__, (cpu_addr_t)p);
            }
            break;

        case 2:
            if (p != K_NULL)
            {
                printf("[%3d|%s] p free:0x%x\n", __LINE__, __FUNCTION__, (cpu_addr_t)p);
                tos_mmheap_free(p);
            }
            break;

        case 3:
            p = tos_mmheap_alloc(0x30);
            if (p != K_NULL)
            {
                printf("[%3d|%s] p alloc:0x%x\n", __LINE__, __FUNCTION__, (cpu_addr_t)p);
            }
            break;

        case 4:
            p_aligned = tos_mmheap_aligned_alloc(0x50, 16);
            if (p_aligned != K_NULL)
            {
                printf("[%3d|%s] aligned alloc:0x%x\n", __LINE__, __FUNCTION__, (cpu_addr_t)p_aligned);
                if (((cpu_addr_t)p_aligned % 16) == 0)
                {
                    printf("[%3d|%s] 0x%x is 16 aligned\n", __LINE__, __FUNCTION__, (cpu_addr_t)p_aligned);
                }
                else
                {
                    printf("[%3d|%s] should not happen\n", __LINE__, __FUNCTION__);
                }
            }
            break;

        case 5:
            /* 不对堆内存进行释放,造成内存泄漏 */
            // if (p != K_NULL)
            // {
            //     tos_mmheap_free(p);
            // }
            p = tos_mmheap_alloc(0x40);
            if (p != K_NULL)
            {
                printf("[%3d|%s] p free:0x%x\n", __LINE__, __FUNCTION__, (cpu_addr_t)p);
            }
            break;

        case 6:
            if (p != K_NULL)
            {
                printf("[%3d|%s] p free:0x%x\n", __LINE__, __FUNCTION__, (cpu_addr_t)p);
                tos_mmheap_free(p);
            }

            if (p_aligned != K_NULL)
            {
                printf("[%3d|%s] p_aligned free:0x%x\n", __LINE__, __FUNCTION__, (cpu_addr_t)p_aligned);
                tos_mmheap_free(p_aligned);
            }
            if (info.used != 0)
            {
                printf("[%3d|%s] mmheap Memory leak\n", __LINE__, __FUNCTION__);
            }
            break;

        default:
            break;
        }
        tos_task_delay(1000);
        ++i;
    }
}
