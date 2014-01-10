/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��19��
	���ļ��ж������������صĺ���
   ================================================================== */

#include "bootpack.h"

struct FIFO8 keyfifo;

/* �������Լ��̵��ж�  ��naskfunc.nas�е�_asm_inthandler21���� */
void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	/* ֪ͨIRQ1�Ѿ�������� */
	data = io_in8(PORT_KEYDAT);	/* ��8042������������ж�������, ��������, ��8042���ٽ������� */
	fifo8_put(&keyfifo, data);	/* �����յ������ݴ�����̻����������� */
	return;
}

#define PORT_KEYSTA				0x0064			/* i8042��״̬�˿ں� */
#define KEYSTA_SEND_NOTREADY	0x02			/* ������ų��������ж�i8042�����뻺�����Ƿ��� */
#define KEYCMD_WRITE_MODE		0x60			/* ���͸�i8042������ ��������ϸ���� */
#define KBC_MODE				0x47			/* ��������Ϊi8042�Ŀ��ƼĴ�����ֵ */


/* �ȴ����̿��Ƶ�·׼����� */
/* 
	��ʵ������ͣ�ش�i8042��64h�˿ڶ�ȡStatus Register������
	Ȼ���ж�Status Register��bit1�Ƿ�Ϊ0  ��Ϊ0 ˵�����뻺�����ǿյ�
	���Խ���CPU���������������, ��Ϊ1 ˵�����뻺���������� �޷�����
	CPU���������������	
 */
void wait_KBC_sendready(void)
{
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

/* ��ʼ�����̿��Ƶ�· */
void init_keyboard(void)
{
	wait_KBC_sendready();		/* ���i8042�����뻺���� */
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);	/* ��i8042д������ */
												/* �������ʾ׼��д��i8042�Ŀ��ƼĴ��� 
												   ��һ��ͨ��60h�˿�д����ֽڽ�������
												   i8042�Ŀ��ƼĴ�����
												 */
	wait_KBC_sendready();		/* ���i8042�����뻺���� */
	io_out8(PORT_KEYDAT, KBC_MODE);	/* ��KBC_MODE����i8042�Ŀ��ƼĴ����� 
										������ꡢ����
										��������жϺͼ����ж�
									*/
	return;
}