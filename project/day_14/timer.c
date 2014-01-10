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
	struct TIMER *t;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	for (i = 0; i < MAX_TIMER; i++) {	
		timerctl.timers0[i].flags = 0; 	/* 未使用 */
	}
	t = timer_alloc(); 			/* 创建一个新的定时器(哨兵) */
	t->timeout = 0xffffffff;	/* 因为该定时器始终保持在最后面, 所以超时时间设置为最大 */
	t->flags = TIMER_FLAGS_USING;	
	t->next = 0; /* 它是最后一个,所以它的next域为0 */
	timerctl.t0 = t; /* 因为现在只有哨兵,所以它是最前面的 */
	timerctl.next = 0xffffffff; /* 因为只有哨兵,所以下一个超时时刻就是哨兵的时刻 */
	return;
}


/* 创建新的定时器并返回指向该定时器结构的指针 */
struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {	/* 找到第一个未使用的定时器位置 */
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;	/* 设定其状态为已配置 */
			return &timerctl.timers0[i];		/* 返回这个结构的指针 */
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
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

/* 定时器的设定 */
/* 如果对该函数的算法不太明白的可以参考任意一本数据结构书籍中关于单链表的插入操作 */
void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		/* 插入最前面的情况 */
		timerctl.t0 = timer;
		timer->next = t; /* 下一个定时器是设定为t */
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	/* 搜索插入位置 */
	for (;;) {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
			/* 插入s个t之间的情况 */
			s->next = timer; /* s的下一个是timer */
			timer->next = t; /* timer的下一个是t */
			io_store_eflags(e);
			return;
		}
	}
}


/* 时钟中断处理函数 */
void inthandler20(int *esp)
{
	struct TIMER *timer;
	io_out8(PIC0_OCW2, 0x60);	/* 通知PIC,IRQ0已经受理完毕 */
	timerctl.count++;
	if (timerctl.next > timerctl.count) {
		return;
	}
	timer = timerctl.t0; /* 首先把最前面的地址赋给timer */
	for (;;) {
		/* timers的定时器都处于动作中, 所以不确认flags */
		if (timer->timeout > timerctl.count) {	
		/* 一旦遇到未超时的定时器就跳出循环 */	
			break;
		}
		/* 超时 */
		timer->flags = TIMER_FLAGS_ALLOC;
		fifo32_put(timer->fifo, timer->data);
		timer = timer->next; /* 将下一个定时器的地址赋给timer */
	}
	timerctl.t0 = timer;	/* 始终保证t0存放的是第一个即将超时的定时器的地址 */
	timerctl.next = timer->timeout;
	return;
}
