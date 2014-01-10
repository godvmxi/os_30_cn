/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��20��
	�����ں˵ĵ�һ��C�����ļ�,��asmhead.nas����,������HarrMain��������
   ================================================================== */

#include "bootpack.h"
#include <stdio.h>

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	/* s����������� 	keybuf�Ǽ��̻����� 	mousebuf����껺����*/
	char s[40], keybuf[32], mousebuf[128];
	int mx, my, i;
	unsigned int memtotal;		/* ����ڴ��С */
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;	/* 	����һ��MEMMAN�ṹָ��, 
																ָ��MEMMAN_ADDRҲ����0x3c0000
																����൱�ڽ�MEMMAN�ṹ���������0x3c0000��
															*/

	struct SHTCTL *shtctl;		/* ͼ������ṹָ�� */
	struct SHEET *sht_back, *sht_mouse;	/* �����Լ�����ͼ��ָ�� */
	unsigned char *buf_back, buf_mouse[256];	

	init_gdtidt();		/* ��ʼ��GDT, IDT */
	init_pic();			/* ��ʼ��PIC */
	io_sti();			/* �����п������ж� */
	fifo8_init(&keyfifo, 32, keybuf);		/* ��ʼ�����̻������ṹ�� */
	fifo8_init(&mousefifo, 128, mousebuf);	/* ��ʼ����껺�����ṹ�� */
	io_out8(PIC0_IMR, 0xf9); /* PIC0(11111001) (��IRQ1�����жϺ����Ӵ�PIC��IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (��PS2����ж� ��IRQ12)*/

	init_keyboard();		/* ��ʼ�����̿��Ƶ�· */
	enable_mouse(&mdec);	/* ������� */
	memtotal = memtest(0x00400000, 0xbfffffff);	/* ���4M~3G-1���ڴ�  memtotal���ڴ�ʵ�ʴ�С */
	memman_init(memman);	/* ��ʼ���ڴ�����ṹ */
	memman_free(memman, 0x00001000, 0x0009e000); 	/* 0x00001000 - 0x0009efff */
													/* ����ڴ���4K~636K-1, ����ڴ���"��ȫ"��
													   ��������ڴ�������Щ���ݿɲο�Ⱥ�����е�
													   "�ڴ�ֲ�ͼ.wps"(ע�⣺��ʱ��������������ipl10
													   ��512BҲ�ڴ˷�Χ֮��,����˵��512B�ڴ�����Ҳ�ǿ��е�)
													  */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);	/* 4M~�ڴ�ʵ�ʴ�С */

	init_palette();		/* ��ʼ����ɫ�� */
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);	/* ��ʼ��ͼ������ṹ */
	sht_back  = sheet_alloc(shtctl);		/* ����"����"ͼ�� */
	sht_mouse = sheet_alloc(shtctl);		/* �������ͼ�� */
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);	/* �����ڴ�ռ� ���ڻ���"����" */
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* ����"����"ͼ����Ϣ */
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);		/* �������ͼ����Ϣ */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);	/* ����"����" ע�⣺����ĵ�һ��������buf_back������ԭ����binfo->vram */
														/* ������buf_backָ��һ���·�����ڴ�,Ҳ����˵ԭ��Ҫ����ͼ�λ�������"����"
														   �������·�����ǿ��ڴ�,�ǿ��ڴ��ŵľ���"����"ͼ����Ҫ��ʾ��ͼ�� ͬʱ
														   ����Ҫ�����������Ļ�ϲ�û��ԭ�������汳��,��Ϊ���ǻ�û����"����"ͼ����ʾ����*/
	init_mouse_cursor8(buf_mouse, 99);		/* ��ʼ�����ͼ�� */
	sheet_slide(shtctl, sht_back, 0, 0);	/* ����"����"ͼ���λ�� */
	mx = (binfo->scrnx - 16) / 2; /* �������ͼ������Ļ�ϵ�λ�� �����������������λ�� */
	my = (binfo->scrny - 28 - 16) / 2;	
	sheet_slide(shtctl, sht_mouse, mx, my);	/* �������ͼ���λ�� */
	sheet_updown(shtctl, sht_back,  0);		/* ����"����"ͼ������ͼ��ĸ߶� */
	sheet_updown(shtctl, sht_mouse, 1);		/* ���һ���ʾ"����"�����ͼ�� */
	sprintf(s, "(%3d, %3d)", mx, my);		
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);	/* ������ͼ�ε����Ͻ�����()ע�⣺��ʱ�������Ϣ��û����ʾ����Ļ�� 
																		���Ǽ�¼��"����"ͼ��Ļ�������*/
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);	
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);	/* ����ڴ��С�Ϳ����ڴ��С ͬ��,�����������"����"ͼ��Ļ������� */
	sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);	/* ˢ��sht_backͼ���ĳһ��������
																   ����������Ͻ�����(0, 0) ���½�����(binfo->scrnx, 48) �˴�����������ڻ�������
																   ��ʱ!��������ͼ������������(������Ҫ�������Ϣ)����ʾ���� */

	for (;;) {
		io_cli();				/* �ر����п������ж� */
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {	/* ������̻�����������껺�����ж�û������ */
			io_stihlt();		/* ���жϲ����� ֱ����һ���ж����� */
		} else {				/* ���̻���껺������������ */
			if (fifo8_status(&keyfifo) != 0) {		/* ������̻������������� */
				i = fifo8_get(&keyfifo);			/* ��ȡ���� */
				io_sti();							/* �����п������ж� */
				sprintf(s, "%02X", i);				/* ����ȡ��������ʮ��������ʽ��� */
				boxfill8(buf_back, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
				sheet_refresh(shtctl, sht_back, 0, 16, 16, 32);		/* ˢ��sht_backͼ�㻺�����е�ĳһ�������� */
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
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);	/* �������䶼�ǶԻ������е����ݽ��и��� */
					putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);	
					sheet_refresh(shtctl, sht_back, 32, 16, 32 + 15 * 8, 32);	/* ˢ��sht_backͼ�㻺�����е�ĳһ������������ʾ����ĸ��� */
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
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* �������� �������䶼�ǶԻ������е����ݽ��и��� */
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* ��ʾ������ */
					sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);			/* ˢ��sht_backͼ�㻺�����е�ĳһ������������ʾ����ĸ��� */
					sheet_slide(shtctl, sht_mouse, mx, my);	/* �������ͼ���λ�ò���ʾ�µ����ͼ�� */
				}
			}
		}
	}
}