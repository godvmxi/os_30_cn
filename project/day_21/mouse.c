/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��21��
	���ļ��ж������������صĺ���
   ================================================================== */

#include "bootpack.h"

struct FIFO32 *mousefifo;
int mousedata0;				/* ���������յ����ݶ���Ҫ�������ֵ */


/* ��������PS/2�����ж� ��naskfunc.nas�е�_asm_inthandler2c���� */
void inthandler2c(int *esp)
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);	/* ֪ͨPIC1  IRQ12�Ѿ�������� */
	io_out8(PIC0_OCW2, 0x62);	/* ֪ͨPIC0  IRQ2�Ѿ�������� */
	data = io_in8(PORT_KEYDAT);	/* ��8042������������ж�������, ��������, ��8042���ٽ������� */
	fifo32_put(&mousefifo, data);/* �����յ������ݴ�����껺���������� */
	return;
}


#define KEYCMD_SENDTO_MOUSE		0xd4	/* Ҫ��i8042д�������,�����н��� */
#define MOUSECMD_ENABLE			0xf4	/* �����������������������������ݰ� */

/* ������� */
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	mousefifo = fifo;
	mousedata0 = data0;

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
