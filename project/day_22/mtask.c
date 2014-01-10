/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��24��
	���ļ��������������л���صĺ���
   ================================================================== */
/* 
	����TSS��������ο�
	http://blog.csdn.net/programmingring/article/details/7425463
	��������֮ǰ�Ƽ����Բ�ʿ����,�������й��ڱ���ģʽ�Ľ������ϸ
 */
/* 
   ����task�ṹ��flag��Ա������ȡֵ
   0��δ� 	1������ 	2������ 
 */
#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;		/* �����ʱ������Ҫ, �����л���ȫ������ */


/* ��ȡ��ǰ�������е������task�ṹָ�� */
struct TASK *task_now(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	return tl->tasks[tl->now];
}

/* �������� */
void task_add(struct TASK *task)
{
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	tl->tasks[tl->running] = task;	
	tl->running++;
	task->flags = 2; /* ��б��  */
	return;
}


/* ɾ������ */
void task_remove(struct TASK *task)
{
	int i;
	struct TASKLEVEL *tl = &taskctl->level[task->level];

	/* Ѱַtask����λ�� */
	for (i = 0; i < tl->running; i++) {
		if (tl->tasks[i] == task) {
			/* �ҵ����˳�ѭ�� */
			break;
		}
	}

	tl->running--;
	if (i < tl->now) {
		tl->now--; /* ����now��ǰ��,Ҳ����˵i����Ķ�Ҫǰ��һ��λ��, �±�Ϊnow�Ľṹ��ȻҲҪǰ�� */
				   /* ��������������޸�now��ֵ, ָ���ƶ���ԭ��ָ��Ľṹ����λ�� */
	}
	if (tl->now >= tl->running) {	
	/* ���now��ֵ�����쳣������ */
		tl->now = 0;
	}
	task->flags = 1; /* ������ */

	/* ��λ */
	for (; i < tl->running; i++) {
		tl->tasks[i] = tl->tasks[i + 1];
	}

	return;
}


/* Ѱַ���ϲ��level */
void task_switchsub(void)
{
	int i;
	/* Ѱַ���ϲ��level */
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (taskctl->level[i].running > 0) {
			break; /* �ҵ�������ѭ�� */
		}
	}
	taskctl->now_lv = i;		/* ���ĵ�ǰ���еĲ�� */
	taskctl->lv_change = 0;		
	return;
}

/* �������� ����������ײ� */
void task_idle(void)
{
	for (;;) {
		io_hlt();
	}
}

/* ����������ϵͳ�ĳ�ʼ����ʼ�� */
struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task, *idle;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;

	taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof (struct TASKCTL));
	/* ��ʼ�����е������task�ṹ */
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;	/* ѡ���ӳ�ʼ�� */
		/* ��������ʼ�� */
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	/* ��ʼ������level�ṹ */
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		taskctl->level[i].running = 0;		/* û���������е����� */
		taskctl->level[i].now = 0;
	}

	task = task_alloc();
	task->flags = 2;	/* ��б�־ */
	task->priority = 2; /* 0.02�� */
	task->level = 0;	/* ���LEVEL */
	task_add(task);
	task_switchsub();	/* LEVEL���� */
	load_tr(task->sel);	/* �޸�tr�Ĵ��� */
	task_timer = timer_alloc();	/* ��ͷϷ���������л��Ķ�ʱ�� */
	timer_settime(task_timer, task->priority);

	/* ��������ĳ�ʼ�� */
	idle = task_alloc();
	idle->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	idle->tss.eip = (int) &task_idle;
	idle->tss.es = 1 * 8;
	idle->tss.cs = 2 * 8;
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
	task_run(idle, MAX_TASKLEVELS - 1, 1);

	return task;
}


/* ����һ��task�ṹ */
struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {			/* �������е�task�ṹ */
		if (taskctl->tasks0[i].flags == 0) {	/* δ� */
			task = &taskctl->tasks0[i];
			task->flags = 1; /* ������ */
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; 	/* ��ʼ�������Ĵ��� */
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
			task->tss.ss0 = 0;		/* �������ں�̬��ջ�μĴ���Ϊ0 */
			return task;
		}
	}
	return 0; 
}


void task_run(struct TASK *task, int level, int priority)
{
	if (level < 0) {
		level = task->level; /* ���ı�level */
	}
	if (priority > 0) {		
		task->priority = priority;
	}

	/* ���ڸոյ���task_alloc()��������������ֱ�ӽ��份��(��ΪĬ�ϸմ����������flag = 1) */
	/* �����Ѿ���������(flag=2��������������,ֻ��˵������������Ҳ�����ڶ����еȴ����Ⱥ���������������) */
	/* ������, ��Ҫ�ж�����level�Ƿ����������level���,�������Ҫ�޸ġ� */
	/* �������, ���Ƚ���ӵȴ����еĶ�����ɾ�����޸���level */
		if (task->flags == 2 && task->level != level) { /* �ı��е������level */
		task_remove(task); /* ����ִ��֮�� flags��ֵ���Ϊ1�����������if����Ҳ�ᱻִ�� */
	}
	if (task->flags != 2) {
		/* ������״̬���ѵ����� */
		task->level = level;
		task_add(task);
	}
	/* ��Ϊ��������Ѿ��޸���ĳ�������level �������´ε���ʱ������level */
	taskctl->lv_change = 1; /* �´��л�����ʱ���level */
	return;
}

/* ʹĳ����˯�� */
void task_sleep(struct TASK *task)
{
	struct TASK *now_task;
	if (task->flags == 2) {
		/* ������ڻ״̬ */
		now_task = task_now();	/* ��õ�ǰ�������е������taskָ�� */
		task_remove(task); /* ִ�д����Ļ�flag�����1 */
		if (task == now_task) {
			/* ��������Լ����� ����Ҫ�����л� */
			task_switchsub();
			now_task = task_now(); /* ���趨���ȡ�µ�level�д����е����� */
			farjmp(0, now_task->sel);	/* �л����� */
		}
	}
	return;
}


/* �����л����� */
/* �ú���ֻ��void inthandler20(int *esp)ʱ���жϴ����б����� */
void task_switch(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = tl->tasks[tl->now];
	tl->now++;
	if (tl->now == tl->running) {
	/* ��now�����쳣ʱ������ */
		tl->now = 0;
	}	
	if (taskctl->lv_change != 0) {		/* �Ƿ���Ҫ���level */
		task_switchsub();	/* Ѱ�����ϲ��level */
		tl = &taskctl->level[taskctl->now_lv];	/* �޸�tl����ָ�����ϲ��level */
	}
	new_task = tl->tasks[tl->now];
	timer_settime(task_timer, new_task->priority);
	if (new_task != now_task) {
		farjmp(0, new_task->sel);
	}
	return;
}