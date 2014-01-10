/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��21��
	�����ں˵ĵ�һ��C�����ļ�,��asmhead.nas����,������HarrMain��������
   ================================================================== */
   
#include "bootpack.h"
#include <stdio.h>

/* �������ڵĺ��� */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title);
/* ������� */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo;			/* ����һ�������Ķ��л����������ṹ */
	char s[40];					/* ��������� */
	int fifobuf[128];			/* ���л����� */
	struct TIMER *timer, *timer2, *timer3;
	int mx, my, i, count = 0;
	unsigned int memtotal;		/* memtotal����ڴ��С */
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;	/* 	����һ��MEMMAN�ṹָ��, 
																ָ��MEMMAN_ADDRҲ����0x3c0000
																����൱�ڽ�MEMMAN�ṹ���������0x3c0000��
															*/
	struct SHTCTL *shtctl;		/* ͼ������ṹָ�� */
	struct SHEET *sht_back, *sht_mouse, *sht_win;	/* ����������Լ����ڵ�ͼ��ָ�� */
	unsigned char *buf_back, buf_mouse[256], *buf_win;

	init_gdtidt();		/* ��ʼ��GDT, IDT */
	init_pic();			/* ��ʼ��PIC */
	io_sti();			/* �����п������ж� */
	fifo32_init(&fifo, 128, fifobuf);	/* ��ʼ�����̻������ṹ�� */
	init_pit();							/* ��ʼ��PIT */
	init_keyboard(&fifo, 256);			/* ��ʼ�����̿��Ƶ�· */
	enable_mouse(&fifo, 512, &mdec);	/* ������� */
	io_out8(PIC0_IMR, 0xf8); /* PIC0(11111000) (��IRQ0ʱ���жϡ�IRQ1�����жϺ����Ӵ�PIC��IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (��PS2����ж� ��IRQ12)*/

	timer = timer_alloc();			
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);

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
	sht_win   = sheet_alloc(shtctl);		/* ��������ͼ�� */
	/* �����ڴ�ռ� ���ڻ���"����" */
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);	/* �����ڴ�ռ� ���ڻ���"���� */
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); 	/* ����"����"ͼ����Ϣ */
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);						/* �������ͼ����Ϣ */
	sheet_setbuf(sht_win, buf_win, 160, 52, -1);						/* ���ô���ͼ����Ϣ */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);	/* ����"����" ע�⣺����ĵ�һ��������buf_back������ԭ����binfo->vram */
														/* ������buf_backָ��һ���·�����ڴ�,Ҳ����˵ԭ��Ҫ����ͼ�λ�������"����"
														   �������·�����ǿ��ڴ�,�ǿ��ڴ��ŵľ���"����"ͼ����Ҫ��ʾ��ͼ�� ͬʱ
														   ����Ҫ�����������Ļ�ϲ�û��ԭ�������汳��,��Ϊ���ǻ�û����"����"ͼ����ʾ����*/
	init_mouse_cursor8(buf_mouse, 99);			/* �������ͼ�� */
	make_window8(buf_win, 160, 52, "counter");	/* ���ƴ���ͼ�� */
	sheet_slide(sht_back, 0, 0);				/* ����"����"ͼ���λ�� */
	mx = (binfo->scrnx - 16) / 2; /* �������ͼ������Ļ�ϵ�λ�� �����������������λ�� */
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_mouse, mx, my);				/* �������ͼ���λ�� */
	sheet_slide(sht_win, 80, 72);				/* ���ô���ͼ���λ�� */
	sheet_updown(sht_back,  0);		/* ����"����"ͼ�㡢���ͼ��ʹ���ͼ��ĸ߶� */
	sheet_updown(sht_win,   1);		/* ���һ���ʾ"����"�����ʹ���ͼ�� */
	sheet_updown(sht_mouse, 2);
	sprintf(s, "(%3d, %3d)", mx, my);	
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);		/* ����Ļ�����s������ */
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);	/* ����Ļ�����s������ */

	for (;;) {
		count++;

		io_cli();
		if (fifo32_status(&fifo) == 0) {	/* ��������û������ */
			io_sti();	
		} else {
			i = fifo32_get(&fifo);			/* ȡ�û������е����� */
			io_sti();
			if (256 <= i && i <= 511) { /* ���ڼ��̵����� */
				sprintf(s, "%02X", i - 256);	/* ֱ����� */
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
			} else if (512 <= i && i <= 767) { /* ����������� */
				if (mouse_decode(&mdec, i - 512) != 0) {	/* ������귢�͵����� */
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
					/* ���s������ */
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
					/* �����µ����λ�� */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {		/* ����λ�ò���С��0 */
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {	/* ͬ���Ƿ�Χ���� */
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					/* ���s������ */
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					/* �������ͼ���λ�ò���ʾ�µ����ͼ�� */
					sheet_slide(sht_mouse, mx, my);
				}
			} else if (i == 10) { /* 10��ʱ��*/
				/* ����Ļ����ʾ"10[sec]"�����count��ֵ */
				putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
				sprintf(s, "%010d", count);
				putfonts8_asc_sht(sht_win, 40, 28, COL8_000000, COL8_C6C6C6, s, 10);
			} else if (i == 3) { /* 3��ʱ�� */
				/* ����Ļ����ʾ"10[sec]"����count���� */
				putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
				count = 0;		/* ��ʼ���� */
			} else if (i == 1) { /* ����ö�ʱ�� */
				timer_init(timer3, &fifo, 0); /* �������趨0 */
				/* ���һ��С���� */
				boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 15, 111);
				/* ���ð��� */
				timer_settime(timer3, 50);
				/* ����Ļ����ʾ�Ǹ�С���� */
				sheet_refresh(sht_back, 8, 96, 16, 112);
			} else if (i == 0) { /* ����ö�ʱ�� */
				timer_init(timer3, &fifo, 1); /* �������趨1 */
				/* ���һ��С���� */
				boxfill8(buf_back, binfo->scrnx, COL8_008484, 8, 96, 15, 111);
				/* ���ð��� */
				timer_settime(timer3, 50);
				/* ����Ļ����ʾ�Ǹ�С���� */
				sheet_refresh(sht_back, 8, 96, 16, 112);
			}
		}
	}
}


/* ��������ͼ�� */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
	/* ������鶨����Ǵ������Ͻ��Ǹ����ͼ�� */
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	/* �Ȼ������ڵ���״ */
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	/* ������ڵı��� */
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
	/* �������ϽǵĲ�� */
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}


/* �������... */
/* 
	x,y	��ʾλ�õ����Ͻ�����
	c	�ַ���ɫ
	b	������ɫ
	s	�ַ���
	l	�ַ�������
 */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}