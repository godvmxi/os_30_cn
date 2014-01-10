/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��18��
	���ļ��ж��������ж��йصĺ���
   ================================================================== */

#include "bootpack.h"
#include <stdio.h>

/*
	����8259A��������ݿɲο��Բ�ʿ�ġ�Linux �ں���ȫ������������0.11�ںˡ�P215
	���ж�8259A����ϸ�Ľ���
 */
/* ��ʼ��PIC */
void init_pic(void)
{
	io_out8(PIC0_IMR,  0xff  ); /* ��ֹ��PIC�����ж� */
	io_out8(PIC1_IMR,  0xff  ); /* ��ֹ��PIC�����ж� */

	io_out8(PIC0_ICW1, 0x11  ); /* ��ҪICW4�� ��Ƭ������ ���ش�����ʽ */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7����INT 0x20~0x27���� */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1��IRQ2���� */
	io_out8(PIC0_ICW4, 0x01  ); /* ��ͨȫǶ�� �ǻ��� ���Զ������жϷ�ʽ */

	io_out8(PIC1_ICW1, 0x11  ); /* ��ҪICW4�� ��Ƭ������ ���ش�����ʽ */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15����INT 0x28~0x2f���� */
	io_out8(PIC1_ICW3, 2     ); /* PIC1��IRQ2���� */
	io_out8(PIC1_ICW4, 0x01  ); /* ��ͨȫǶ�� �ǻ��� ���Զ������жϷ�ʽ */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1����ȫ����ֹ */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 ��ֹ��PIC�����ж� */

	return;
}
#define PORT_KEYDAT		0x0060		/* 8042�����ݶ˿ں� */

struct FIFO8 keyfifo;				/* ���̻��������� */

/* �������Լ��̵��ж�  ��naskfunc.nas�е�_asm_inthandler21���� */
void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	/* ֪ͨIRQ1�Ѿ�������� */
	data = io_in8(PORT_KEYDAT);	/* ��8042������������ж�������, ��������, ��8042���ٽ������� */
	fifo8_put(&keyfifo, data);	/* �����յ������ݴ�����̻����������� */
	return;
}

struct FIFO8 mousefifo;			/* ��껺�������� */

/* ��������PS/2�����ж� ��naskfunc.nas�е�_asm_inthandler2c���� */
void inthandler2c(int *esp)
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);	/* ֪ͨPIC1  IRQ12�Ѿ�������� */
	io_out8(PIC0_OCW2, 0x62);	/* ֪ͨPIC0  IRQ2�Ѿ�������� */
	data = io_in8(PORT_KEYDAT);	/* ��8042������������ж�������, ��������, ��8042���ٽ������� */
	fifo8_put(&mousefifo, data);/* �����յ������ݴ�����껺���������� */
	return;
}

/* ����IRQ7�ж� ��naskfunc.nas�е�_asm_inthandler27���� */
/*
	����IRQ7�Ĵ����ɶ����Բ�ʿ�ġ�Linux �ں���ȫ������������0.11�ںˡ�P219
	�ı���������
 */
void inthandler27(int *esp)								
{
	io_out8(PIC0_OCW2, 0x67); /* ֱ�ӷ���EOI���� ��ʾ�жϴ������� */
	return;
}