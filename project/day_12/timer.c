/* ==================================================================
	注释：宅
	时间：2013年2月21日
	该文件中定义了与定时器相关的函数
   ================================================================== */
   
#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC		1	/* 已配置状态*/
#define TIMER_FLAGS_USING		2	/* 定时器运行中 */

/* 初始化pit函数 */
/* 
   关于PIT设定的资料我并没有找到, 姑且就按作者的设定吧
   不追究去原理了。。。= =
 */
void init_pit(void)
{
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	timerctl.next = 0xffffffff; /* 最初没有正在运行的定时器(这是next能表示的最大值) */
	timerctl.using = 0;			/* 没有处于活动状态的定时器 */
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0; /* 未使用 */
	}
	return;
}

/* 创建新的定时器并返回指向该定时器结构的指针 */
struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {	
		if (timerctl.timers0[i].flags == 0) {		/* 找到第一个未使用的定时器位置 */
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;	/* 设定其状态为已配置 */
			return &timerctl.timers0[i];			/* 返回这个结构的指针 */
		}
	}
	return 0; /* 没找到就返回0 */
}

/* 释放timer所指向的定时器 */
void timer_free(struct TIMER *timer)
{
	timer->flags = 0; /* 直接修改flag为未使用状态 */
	return;
}

/* 定时器结构的初始化 */
void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

/* 定时器的设定 */
void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e, i, j;
	timer->timeout = timeout + timerctl.count;		
	timer->flags = TIMER_FLAGS_USING;		/* 定时器状态设置 */
	e = io_load_eflags();
	io_cli();								/* 关闭所有可屏蔽中断(所有的IRQ中断都是可屏蔽的) */
	/* 搜索注册位置 */
	for (i = 0; i < timerctl.using; i++) {
		if (timerctl.timers[i]->timeout >= timer->timeout) {
			break;
		}
	}
	/* i号之后全部后移一位 */
	for (j = timerctl.using; j > i; j--) {
		timerctl.timers[j] = timerctl.timers[j - 1];
	}
	timerctl.using++;
	/* 插入到空位上 */
	timerctl.timers[i] = timer;
	timerctl.next = timerctl.timers[0]->timeout;		/* 注意：timers数组是有序的,所以才使用第0个元素的timeout作为next值 */
														/* 这是为了防止i=0的情况 */
	io_store_eflags(e);				/* 恢复eflags寄存器的值 */
	return;
}


/* 时钟中断处理函数 */
void inthandler20(int *esp)
{
	int i, j;
	io_out8(PIC0_OCW2, 0x60);	/* 通知IRQ0已经受理完毕 */
	timerctl.count++;			
	if (timerctl.next > timerctl.count) {		
		return;
	}
	for (i = 0; i < timerctl.using; i++) {
		/* timers的定时器都处于动作中, 所以不确认flags */
		if (timerctl.timers[i]->timeout > timerctl.count) {	/* 一旦遇到未超时的定时器就跳出循环 */	
			break;
		}
		/* 超时 */
		timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
		fifo8_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
	}
	/* 正好有i个定时器超时, 其余的进行移位 */
	timerctl.using -= i;
	for (j = 0; j < timerctl.using; j++) {		/* 前移i个位置 */
		timerctl.timers[j] = timerctl.timers[i + j];
	}
	if (timerctl.using > 0) {	/* 还有活动的定时器 */
		timerctl.next = timerctl.timers[0]->timeout;	
	} else {
		timerctl.next = 0xffffffff;
	}
	return;
}
