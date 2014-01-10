/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��19��
	�����ں˵ĵ�һ��C�����ļ�,��asmhead.nas����,������HarrMain��������
   ================================================================== */

#include "bootpack.h"
#include <stdio.h>


/* ��껺�����ṹ */
/* 
	buf[]�ǻ�����,�����귢�͹�����3�ֽ�����
	phase������ʶ���յ�������ݵĵڼ���
	x, y������λ����	bth��¼������Ϣ
 */
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};

/* �������� */
extern struct FIFO8 keyfifo, mousefifo;
void enable_mouse(struct MOUSE_DEC *mdec);
void init_keyboard(void);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	/* s�����������	mcursor������ͼ�� keybuf�Ǽ��̻����� mousebuf����껺����*/
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	struct MOUSE_DEC mdec;

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
	init_mouse_cursor8(mcursor, COL8_008484);	/* ��ʼ�����ͼ�� */
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* ��ʾ���ͼ�� */
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);		/* ������ͼ�����Ͻ�����Ļ�ϵ����� */

	enable_mouse(&mdec);		/* ������� */

	for (;;) {
		io_cli();				/* �ر����п������ж� */
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {	/* ������̻�����������껺�����ж�û������ */
			io_stihlt();		/* ���жϲ����� ֱ����һ���ж����� */
		} else {				/* ���̻���껺������������ */
			if (fifo8_status(&keyfifo) != 0) {		/* ������̻������������� */
				i = fifo8_get(&keyfifo);			/* ��ȡ���� */
				io_sti();							/* �����п������ж� */
				sprintf(s, "%02X", i);				/* ����ȡ��������ʮ��������ʽ��� */
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {	/* �����껺������������ */
				i = fifo8_get(&mousefifo);			/* ��ȡ���� */
				io_sti();							/* �����п������ж� */
				if (mouse_decode(&mdec, i) != 0) {	/* ������귢�͵����� */
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);	
					if ((mdec.btn & 0x01) != 0) {	/* ������������ */
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {	/* ����Ҽ������� */
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {	/* ������ֱ����� */
						s[2] = 'C';
					}
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);	/* �����Ϣ */
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* ������� */
					mx += mdec.x;					/* �����µ����λ�� */
					my += mdec.y;
					if (mx < 0) {	/* ����λ�ò���С��0,�����ܳ�����Ļλ�� */
						mx = 0;		
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {	/* ͬ���Ƿ�Χ���� */
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* �������� */
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* ��ʾ������ */
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* �����µ���� */
				}
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
void enable_mouse(struct MOUSE_DEC *mdec)
{
	wait_KBC_sendready();		/* ���i8042�����뻺���� */
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);	/* ��i8042д������ */
												/* �������ʾ������0x60�˿ڵĲ������ݷ��������	*/
	wait_KBC_sendready();		/* ���i8042�����뻺���� */
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);	/* ������귢���� */
	mdec->phase = 0; /* �ȴ�0xfa�Ľ׶� */
	return;
}


/* ����������� */
/*
	������귢�͵�3���ֽڵĸ�bit���庬��,��ο�Ⱥ�����е�"PS2������Э��-����PS2����.pdf"
 */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		/* �ȴ�����0xfa�׶� */
		if (dat == 0xfa) {		/* �յ�ȷ����Ϣ */
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* �ȴ����ĵ�һ�ֽڽ׶� */
		if ((dat & 0xc8) == 0x08) {		/* �ж�λ���Ƿ����(-255~+255) */
			mdec->buf[0] = dat;	
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		/* �ȴ����ĵڶ��ֽڽ׶� */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* �ȴ����ĵ����ֽڽ׶� */
		mdec->buf[2] = dat;
		mdec->phase = 1;				
		mdec->btn = mdec->buf[0] & 0x07;	/* ȡBYTE1�ĵ�3λ ����״̬ */
		mdec->x = mdec->buf[1];				/* ����x,y��λ�� */
		mdec->y = mdec->buf[2];				
		if ((mdec->buf[0] & 0x10) != 0) {	/* �ж�x�ķ���λ */
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {	/* �ж�y�ķ���λ */
			mdec->y |= 0xffffff00;
		}									/* mdec->x��mdec->y�д�ŵĶ��ǲ��� */
		/*
			����x��λ������-100, -100�Ĳ�����1 1001 1100(���λ1��BYTE1��, ��8λ��BYTE2)
			��ʼmedc->x�д�ŵľ���1001 1100, ͨ��((mdec->buf[0] & 0x10) != 0)�жϳ�����
			λΪ1�����Ǹ���,��mdec->x = 0xFFFFFF9C,��ǰ��mx��152 = 0x98����ô�µ�λ����Ӧ
			����152+(-100) = 52.
			��86����mx += mdec.x; ��0xFFFFFF9C+0x98 = 0x100000034 ����int��4���ֽڵ�,����
			����ߵ�1�ᱻ��ȥ, ��mx = 0x34 = 52
		 */
		mdec->y = - mdec->y; /* ����y�����뻭������෴ */
		return 1;
	}
	return -1; /* ���ᵽ��� */
}
/* 
	
 */