/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��22��
	���ļ��ж������붨ʱ����صĺ���
   ================================================================== */

#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC		1	/* ������״̬*/
#define TIMER_FLAGS_USING		2	/* ��ʱ�������� */

/* ��ʼ��pit���� */
/* 
   ����PIT�趨�������Ҳ�û���ҵ�, ���ҾͰ����ߵ��趨��
   ��׷��ȥԭ���ˡ�����= =
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
		timerctl.timers0[i].flags = 0; 	/* δʹ�� */
	}
	t = timer_alloc(); 			/* ����һ���µĶ�ʱ��(�ڱ�) */
	t->timeout = 0xffffffff;	/* ��Ϊ�ö�ʱ��ʼ�ձ����������, ���Գ�ʱʱ������Ϊ��� */
	t->flags = TIMER_FLAGS_USING;	
	t->next = 0; /* �������һ��,��������next��Ϊ0 */
	timerctl.t0 = t; /* ��Ϊ����ֻ���ڱ�,����������ǰ��� */
	timerctl.next = 0xffffffff; /* ��Ϊֻ���ڱ�,������һ����ʱʱ�̾����ڱ���ʱ�� */
	return;
}

/* �����µĶ�ʱ��������ָ��ö�ʱ���ṹ��ָ�� */
struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {	/* �ҵ���һ��δʹ�õĶ�ʱ��λ�� */
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;	/* �趨��״̬Ϊ������ */
			return &timerctl.timers0[i];	/* ��������ṹ��ָ�� */
		}
	}
	return 0;	/* û�ҵ��ͷ���0 */
}

/* �ͷ�timer��ָ��Ķ�ʱ�� */
void timer_free(struct TIMER *timer)
{
	timer->flags = 0; /* ֱ���޸�flagΪδʹ��״̬ */
	return;
}

/* ��ʱ���ṹ�ĳ�ʼ�� */
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

/* ��ʱ�����趨 */
/* ����Ըú������㷨��̫���׵Ŀ��Բο�����һ�����ݽṹ�鼮�й��ڵ������Ĳ������ */
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
		/* ������ǰ������ */
		timerctl.t0 = timer;
		timer->next = t; /* ��һ����ʱ�����趨Ϊt */
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	/* ��������λ�� */
	for (;;) {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
			/* ����s��t֮������ */
			s->next = timer; /* s����һ����timer */
			timer->next = t; /* timer����һ����t */
			io_store_eflags(e);
			return;
		}
	}
}


/* ʱ���жϴ������� */
void inthandler20(int *esp)
{
	struct TIMER *timer;
	char ts = 0;
	io_out8(PIC0_OCW2, 0x60);	/* ֪ͨPIC,IRQ0�Ѿ�������� */
	timerctl.count++;
	if (timerctl.next > timerctl.count) {
		return;
	}
	timer = timerctl.t0; /* ���Ȱ���ǰ��ĵ�ַ����timer */
	for (;;) {
		/* timers�Ķ�ʱ�������ڶ�����, ���Բ�ȷ��flags */
		if (timer->timeout > timerctl.count) {
		/* һ������δ��ʱ�Ķ�ʱ��������ѭ�� */	
			break;
		}
		/* ��ʱ */
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer) {		/* �жϲ����жϵĶ�ʱ���ǲ���task_timer */
										/* �����ʱ����mtask.c�ж��� */
			fifo32_put(timer->fifo, timer->data);
		} else {
			ts = 1; /* task_timer��ʱ */
		}
		timer = timer->next; /* ����һ����ʱ���ĵ�ַ����timer */
	}
	timerctl.t0 = timer;
	timerctl.next = timer->timeout;
	if (ts != 0) {		/* ���task_timer��ʱ �͵���task_switch() */
		task_switch();
	}
	return;
}

