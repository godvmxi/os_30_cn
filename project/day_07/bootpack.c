/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��18��
   ================================================================== */
   
#include "bootpack.h"
#include <stdio.h>

/* �������� */
extern struct FIFO8 keyfifo, mousefifo;
void enable_mouse(void);
void init_keyboard(void);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	/* s�����������	mcursor������ͼ�� keybuf�Ǽ��̻����� mousebuf����껺����*/
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;

	init_gdtidt();		/* ��ʼ��GDT, IDT */
	init_pic();			/* ��ʼ��PIC */
	io_sti();			/* �����п������ж� */
	
	fifo8_init(&keyfifo, 32, keybuf);		/* ��ʼ�����̻������ṹ�� */
	fifo8_init(&mousefifo, 128, mousebuf);	/* ��ʼ����껺�����ṹ�� */
	io_out8(PIC0_IMR, 0xf9); /* PIC0(11111001) (��IRQ1�����жϺ����Ӵ�PIC��IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (��PS2����ж� ��IRQ12)*/

	init_keyboard();		/* ��ʼ�����̿��Ƶ�· */

	init_palette();		/* ��ʼ����ɫ�� */
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);	/* ����"����" */
	mx = (binfo->scrnx - 16) / 2; /* �������ͼ������Ļ�ϵ�λ�� �����������������λ�� */
	my = (binfo->scrny - 28 - 16) / 2;	
	init_mouse_cursor8(mcursor, COL8_008484);	
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* ��ʾ���ͼ�� */
	sprintf(s, "(%d, %d)", mx, my);										
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);		/* ������ͼ�����Ͻ�����Ļ�ϵ����� */


	enable_mouse();		/* ������� */

	for (;;) {
		io_cli();		/* �ر����п������ж� */
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {	/* ������̻�����������껺�����ж�û������ */
			io_stihlt();	/* ���жϲ����� ֱ����һ���ж����� */
		} else {		/* ���̻���껺������������ */
			if (fifo8_status(&keyfifo) != 0) {		/* ������̻������������� */
				i = fifo8_get(&keyfifo);			/* ��ȡ���� */
				io_sti();							/* �����п������ж� */
				sprintf(s, "%02X", i);				/* ����ȡ��������ʮ��������ʽ��� */
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {	/* �����껺������������ */
				i = fifo8_get(&mousefifo);				/* ��ȡ���� */
				io_sti();								/* �����п������ж� */
				sprintf(s, "%02X", i);					/* ����ȡ��������ʮ��������ʽ��� */
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 47, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
			}
		}
	}
}

#define PORT_KEYDAT				0x0060			/* i8042�����ݶ˿ں� */
#define PORT_KEYSTA				0x0064			/* i8042��״̬�˿ں� */
#define PORT_KEYCMD				0x0064			/* i8042������˿ں� */	
												/* ��ʵ��״̬�˿ں�����˿���ͬһ���˿�
												   ֻ�����߶�����������ͬ�ķ��ų����� */
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

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4	/* �����������������������������ݰ� */

/* ������� */
void enable_mouse(void)
{
	wait_KBC_sendready();		/* ���i8042�����뻺���� */
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);	/* ��i8042д������ */
												/* �������ʾ������0x60�˿ڵĲ������ݷ��������	*/
	wait_KBC_sendready();		/* ���i8042�����뻺���� */
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);	/* ������귢���� */
	return; 	
}