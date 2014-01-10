/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��21��
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
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	timerctl.next = 0xffffffff; /* ���û���������еĶ�ʱ��(����next�ܱ�ʾ�����ֵ) */
	timerctl.using = 0;			/* û�д��ڻ״̬�Ķ�ʱ�� */
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0; /* δʹ�� */
	}
	return;
}

/* �����µĶ�ʱ��������ָ��ö�ʱ���ṹ��ָ�� */
struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {	
		if (timerctl.timers0[i].flags == 0) {		/* �ҵ���һ��δʹ�õĶ�ʱ��λ�� */
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;	/* �趨��״̬Ϊ������ */
			return &timerctl.timers0[i];			/* ��������ṹ��ָ�� */
		}
	}
	return 0; /* û�ҵ��ͷ���0 */
}

/* �ͷ�timer��ָ��Ķ�ʱ�� */
void timer_free(struct TIMER *timer)
{
	timer->flags = 0; /* ֱ���޸�flagΪδʹ��״̬ */
	return;
}

/* ��ʱ���ṹ�ĳ�ʼ�� */
void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

/* ��ʱ�����趨 */
void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e, i, j;
	timer->timeout = timeout + timerctl.count;		
	timer->flags = TIMER_FLAGS_USING;		/* ��ʱ��״̬���� */
	e = io_load_eflags();
	io_cli();								/* �ر����п������ж�(���е�IRQ�ж϶��ǿ����ε�) */
	/* ����ע��λ�� */
	for (i = 0; i < timerctl.using; i++) {
		if (timerctl.timers[i]->timeout >= timer->timeout) {
			break;
		}
	}
	/* i��֮��ȫ������һλ */
	for (j = timerctl.using; j > i; j--) {
		timerctl.timers[j] = timerctl.timers[j - 1];
	}
	timerctl.using++;
	/* ���뵽��λ�� */
	timerctl.timers[i] = timer;
	timerctl.next = timerctl.timers[0]->timeout;		/* ע�⣺timers�����������,���Բ�ʹ�õ�0��Ԫ�ص�timeout��Ϊnextֵ */
														/* ����Ϊ�˷�ֹi=0����� */
	io_store_eflags(e);				/* �ָ�eflags�Ĵ�����ֵ */
	return;
}


/* ʱ���жϴ������� */
void inthandler20(int *esp)
{
	int i, j;
	io_out8(PIC0_OCW2, 0x60);	/* ֪ͨIRQ0�Ѿ�������� */
	timerctl.count++;			
	if (timerctl.next > timerctl.count) {		
		return;
	}
	for (i = 0; i < timerctl.using; i++) {
		/* timers�Ķ�ʱ�������ڶ�����, ���Բ�ȷ��flags */
		if (timerctl.timers[i]->timeout > timerctl.count) {	/* һ������δ��ʱ�Ķ�ʱ��������ѭ�� */	
			break;
		}
		/* ��ʱ */
		timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
		fifo8_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
	}
	/* ������i����ʱ����ʱ, ����Ľ�����λ */
	timerctl.using -= i;
	for (j = 0; j < timerctl.using; j++) {		/* ǰ��i��λ�� */
		timerctl.timers[j] = timerctl.timers[i + j];
	}
	if (timerctl.using > 0) {	/* ���л�Ķ�ʱ�� */
		timerctl.next = timerctl.timers[0]->timeout;	
	} else {
		timerctl.next = 0xffffffff;
	}
	return;
}