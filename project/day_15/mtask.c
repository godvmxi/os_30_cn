/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��20��
	���ļ��������������л���صĺ���
   ================================================================== */
/* 
	����TSS��������ο�
	http://blog.csdn.net/programmingring/article/details/7425463
	��������֮ǰ�Ƽ����Բ�ʿ����,�������й��ڱ���ģʽ�Ľ������ϸ
 */
#include "bootpack.h"

struct TIMER *mt_timer;
int mt_tr;							/* ���TSS��ѡ���� */

/* ��ʼ������ */
void mt_init(void)
{
	mt_timer = timer_alloc();		/* ����һ����ʱ�� */
	/* ����û��Ҫʹ��timer_init */
	timer_settime(mt_timer, 2);
	mt_tr = 3 * 8;					
	return;
}

/* �����л����� */
void mt_taskswitch(void)
{
	if (mt_tr == 3 * 8) {
		mt_tr = 4 * 8;
	} else {
		mt_tr = 3 * 8;
	}
	timer_settime(mt_timer, 2);
	farjmp(0, mt_tr);				/* Զ��ת */
	return;
}