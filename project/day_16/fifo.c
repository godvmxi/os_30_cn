/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��22��
	���ļ��������뻺����������صĲ���
   ================================================================== */
#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001		/* �����������־ */


/* ��������ʼ������ */
/*
	��ʼ���������ṹ��fifo, size��ʾ��������С, buf��ʾ��������ַ
	��������������д��ʱ��Ҫ����task��ʶ������
 */
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task)
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; 
	fifo->flags = 0;
	fifo->p = 0; /* ��һ������д��λ�� */
	fifo->q = 0; /* ��һ�����ݶ���λ�� */
	fifo->task = task; 
	return;
}

/* ������д�뺯�� */
/*
	�򻺳����ṹ��fifo�еĻ�����д������data
 */
int fifo32_put(struct FIFO32 *fifo, int data)
{
	if (fifo->free == 0) {				/* ������������� */
		fifo->flags |= FLAGS_OVERRUN;	/* ���������־ */
		return -1;
	}
	fifo->buf[fifo->p] = data;			/* ����д�뻺���� */
	fifo->p++;							/* ������һ��д��λ�� */
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;						/* ��������������һ */
	if (fifo->task != 0) {				/* �������Ҫ���ѵ����� */
		if (fifo->task->flags != 2) { /* ������������� */
			task_run(fifo->task, -1, 0); /* ���Ѹ����� */
		}
	}
	return 0;
}

/* �������������� */
/*
	��ȡ�������ṹ��fifo�еĻ�������һ���ֽ����� 
	����Ϊ��������ֵ���ظ�������
 */
int fifo32_get(struct FIFO32 *fifo)
{
	int data;
	if (fifo->free == fifo->size) {
		/* ������Ϊ�� û�����ݿɶ� */
		return -1;
	}
	data = fifo->buf[fifo->q];			/* �������� */
	fifo->q++;							/* ������һ������λ�� */
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;						/* ��������������1 */
	return data;						/* ���ظոն��������� */
}

/* ���ػ������ṹ��fifo�еĻ������й��ж������� */
int fifo32_status(struct FIFO32 *fifo)
{
	return fifo->size - fifo->free;
}