/* ==================================================================
	注释：宅
	时间：2013年2月22日
	该文件定义了与任务切换相关的函数
   ================================================================== */
/* 
	关于TSS的资料请参考
	http://blog.csdn.net/programmingring/article/details/7425463
	或者是我之前推荐的赵博士的书,他的书中关于保护模式的讲解很详细
 */
/* 
   关于这个文件的注释，我个人认为做的并不好。对多任务描述的不清楚
   (作者书上描述的也不是很清楚)不过想描述清楚的话也不容易，我怕注
   释太长了。所以我写了一篇对该操作系统的多任务分析的文章保存在群
   共享中。对OSASK的多任务不太明白的可以参考以下(不保证我的分析就
   是完全正确的。。我也是菜鸟) 
 */
/* 
   关于task结构中flags成员变量的取值
   0是未活动 	1是休眠 	2是运行 
 */
#include "bootpack.h"

struct TASKCTL *taskctl;		
struct TIMER *task_timer;		/* 这个定时器很重要, 任务切换可全靠它了 */

/* 获取当前正在运行的任务的task结构指针 */
struct TASK *task_now(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	return tl->tasks[tl->now];
}

/* 添加任务 */
void task_add(struct TASK *task)
{
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	tl->tasks[tl->running] = task;	
	tl->running++;
	task->flags = 2; /* 活动中标记  */
	return;
}

/* 删除任务 */
void task_remove(struct TASK *task)
{
	int i;
	struct TASKLEVEL *tl = &taskctl->level[task->level];

	/* 寻址task所在位置 */
	for (i = 0; i < tl->running; i++) {
		if (tl->tasks[i] == task) {
			/* 找到就退出循环 */
			break;
		}
	}

	tl->running--;
	if (i < tl->now) {
		tl->now--; /* 它在now的前面,也就是说i后面的都要前移一个位置, 下标为now的结构当然也要前移 */
				   /* 所以先在这里就修改now的值, 指向移动后原来指向的结构的新位置 */
	}
	if (tl->now >= tl->running) {	
	/* 如果now的值出现异常就修正 */
		tl->now = 0;
	}
	task->flags = 1; /* 休眠中 */

	/* 移位 */
	for (; i < tl->running; i++) {
		tl->tasks[i] = tl->tasks[i + 1];
	}

	return;
}

/* 寻址最上层的level */
void task_switchsub(void)
{
	int i;
	/* 寻址最上层的level */
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (taskctl->level[i].running > 0) {
			break; /* 找到就跳出循环 */
		}
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;		
	return;
}

/* 初始化 */
struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof (struct TASKCTL));
	/* 初始化所有的任务的task结构 */
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = 0;		
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;		/* 选择子初始化 */
		/* 描述符初始化 */
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	/* 初始化所有level结构 */
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		taskctl->level[i].running = 0;	/* 没有正在运行的任务 */
		taskctl->level[i].now = 0;		
	}
	task = task_alloc();
	task->flags = 2;	/* 活动中标志 */
	task->priority = 2; /* 0.02秒 */
	task->level = 0;	/* 最高LEVEL */
	task_add(task);		
	task_switchsub();	/* LEVEL设置 */
	load_tr(task->sel);	/* 修改tr寄存器 */
	task_timer = timer_alloc();	/* 重头戏！！任务切换的定时器 */
	timer_settime(task_timer, task->priority);	
	return task;
}

/* 分配一个task结构 */
struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {		/* 遍历所有的task结构 */
		if (taskctl->tasks0[i].flags == 0) {	/* 未活动 */
			task = &taskctl->tasks0[i];
			task->flags = 1; /* 休眠中 */
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; 	/* 初始化各个寄存器 */
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0; 
}


void task_run(struct TASK *task, int level, int priority)
{
	if (level < 0) {
		level = task->level; /* 不改变level */
	}
	if (priority > 0) {		
		task->priority = priority;
	}

	/* 对于刚刚调用task_alloc()函数创建的任务，直接将其唤醒(因为默认刚创建的任务的flag = 1) */
	/* 对于已经可以运行(flag=2不代表正在运行,只能说它可能在运行也可能在队列中等待调度函数调度它来运行) */
	/* 的任务, 则还要判断它的level是否与参数给的level相等,相等则不需要修改。 */
	/* 若不相等, 则先将其从等待运行的队列中删除再修改其level */
	if (task->flags == 2 && task->level != level) { /* 改变活动中的任务的level */
		task_remove(task); /* 这里执行之后 flags的值会变为1，于是下面的if语句块也会被执行 */
	}
	if (task->flags != 2) {
		/* 从休眠状态唤醒的情形 */
		task->level = level;
		task_add(task);
	}
	/* 因为上面可能已经修改了某个任务的level 所以再下次调度时必须检测level */
	taskctl->lv_change = 1; /* 下次切换任务时检查level */
	return;
}


/* 是任务睡眠 */
void task_sleep(struct TASK *task)
{
	struct TASK *now_task;
	if (task->flags == 2) {
		/* 如果处于活动状态 */
		now_task = task_now();	/* 获得当前正在运行的任务的task指针 */
		task_remove(task); /* 执行词语句的话flag会等于1 */
		if (task == now_task) {
			/* 如果是让自己休眠 则需要任务切换 */
			task_switchsub();
			now_task = task_now(); /* 在设定后获取新的level中待运行的任务 */
			farjmp(0, now_task->sel);	/* 切换任务 */
		}
	}
	return;
}


/* 任务切换函数 */
/* 该函数只在void inthandler20(int *esp)时钟中断处理中被调用 */
void task_switch(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = tl->tasks[tl->now];
	tl->now++;
	if (tl->now == tl->running) {
	/* 当now出现异常时调整它 */
		tl->now = 0;
	}
	
	if (taskctl->lv_change != 0) {	/* 是否需要检测level */
		task_switchsub();	/* 寻找最上层的level */
		tl = &taskctl->level[taskctl->now_lv];	/* 修改tl让它指向最上层的level */
	}

	new_task = tl->tasks[tl->now];
	timer_settime(task_timer, new_task->priority);
	if (new_task != now_task) {
		farjmp(0, new_task->sel);
	}
	return;
}
