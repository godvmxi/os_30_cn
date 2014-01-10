/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��22��
	�����ں˵ĵ�һ��C�����ļ�,��asmhead.nas����,������HarrMain��������
   ================================================================== */

#include "bootpack.h"
#include <stdio.h>

/* �������ڵĺ��� */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
/* ������� */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
/* ����һ���ı��� */
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
/* ����B */
void task_b_main(struct SHEET *sht_back);


void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo;		/* ����һ�������Ķ��л����������ṹ */
	char s[40];				/* ��������� */
	int fifobuf[128];		/* ���л����� */
	/* cursor_x ��¼���λ��  cursor_c ������ɫ */
	int mx, my, i, cursor_x, cursor_c;
	unsigned int memtotal;		/* ��¼�ڴ��С */
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;	/* 	����һ��MEMMAN�ṹָ��, 
																ָ��MEMMAN_ADDRҲ����0x3c0000
																����൱�ڽ�MEMMAN�ṹ���������0x3c0000��
															*/
	struct SHTCTL *shtctl;	/* ͼ������ṹָ�� */
	static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_win_b;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_win_b[3];
	struct TASK *task_a, *task_b[3];
	struct TIMER *timer;

	init_gdtidt();		/* ��ʼ��GDT��IDT */
	init_pic();			/* ��ʼ��PIC */
	io_sti(); /* �����п������ж� */
	fifo32_init(&fifo, 128, fifobuf, 0);	/* ���л�������ʼ�� */
	init_pit();			/* ��ʼ��pit */
	init_keyboard(&fifo, 256);	/* ��ʼ�����̿��Ƶ�· */
	enable_mouse(&fifo, 512, &mdec);	/* ������� */
	io_out8(PIC0_IMR, 0xf8); /* PIC0(11111000) (��IRQ0ʱ���жϡ�IRQ1�����жϺ����Ӵ�PIC��IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (��PS2����ж� ��IRQ12)*/

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
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny); /* ��ʼ��ͼ������ṹ */
	task_a = task_init(memman);		
	/* �Ӵ˴���ʼ����������a */
	/* ��task_init����Ȼ�����˶�ʱ��, ������������ϵͳ��ֻ��һ������ ���Բ�����������л�task_switch�л��ж� */
	fifo.task = task_a;	/* ���fifo������д��ͻ�������a */
	task_run(task_a, 1, 2);	/* ��������a��level�����ȼ� */

	/* sht_back */
	sht_back  = sheet_alloc(shtctl);		/* ����"����"ͼ�� */
	/* �����ڴ�ռ� ���ڻ���"����" */
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);	/* ����"����"ͼ����Ϣ */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);	/* ����"����" ע�⣺����ĵ�һ��������buf_back������ԭ����binfo->vram */
														/* ������buf_backָ��һ���·�����ڴ�,Ҳ����˵ԭ��Ҫ����ͼ�λ�������"����"
														   �������·�����ǿ��ڴ�,�ǿ��ڴ��ŵľ���"����"ͼ����Ҫ��ʾ��ͼ�� ͬʱ
														   ����Ҫ�����������Ļ�ϲ�û��ԭ�������汳��,��Ϊ���ǻ�û����"����"ͼ����ʾ����*/

	/* sht_win_b */
	for (i = 0; i < 3; i++) {		/* 3������ */
		sht_win_b[i] = sheet_alloc(shtctl);		/* ��������ͼ�� */
		buf_win_b = (unsigned char *) memman_alloc_4k(memman, 144 * 52);	/* �����ڴ����ڻ��ƴ��� */
		sheet_setbuf(sht_win_b[i], buf_win_b, 144, 52, -1); /* ���ô���ͼ����Ϣ */
		sprintf(s, "task_b%d", i);	
		make_window8(buf_win_b, 144, 52, s, 0);	/* �������� */
		task_b[i] = task_alloc();		
		task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
		task_b[i]->tss.eip = (int) &task_b_main;
		task_b[i]->tss.es = 1 * 8;
		task_b[i]->tss.cs = 2 * 8;
		task_b[i]->tss.ss = 1 * 8;
		task_b[i]->tss.ds = 1 * 8;
		task_b[i]->tss.fs = 1 * 8;
		task_b[i]->tss.gs = 1 * 8;	
		*((int *) (task_b[i]->tss.esp + 4)) = (int) sht_win_b[i];	/* ��sht_winԤ�ȴ�������B[i]�Ķ�ջ�� */
		task_run(task_b[i], 2, i + 1);	/* ����B[i]�������� ���ʱ��Ϳ��ܷ��������л��� */
	}

	/* sht_win */
	/* ��������A�Ĵ��� */
	sht_win   = sheet_alloc(shtctl);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1); 
	make_window8(buf_win, 144, 52, "task_a", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);

	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_mouse_cursor8(buf_mouse, 99);
	mx = (binfo->scrnx - 16) / 2; 	/* �������ͼ������Ļ�ϵ�λ�� �����������������λ�� */
	my = (binfo->scrny - 28 - 16) / 2;

	/* ���ø���ͼ���λ�� */
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_win_b[0], 168,  56);
	sheet_slide(sht_win_b[1],   8, 116);
	sheet_slide(sht_win_b[2], 168, 116);
	sheet_slide(sht_win,        8,  56);
	sheet_slide(sht_mouse, mx, my);
	/* ���ø���ͼ��ĸ߶� */
	sheet_updown(sht_back,     0);
	sheet_updown(sht_win_b[0], 1);
	sheet_updown(sht_win_b[1], 2);
	sheet_updown(sht_win_b[2], 3);
	sheet_updown(sht_win,      4);
	sheet_updown(sht_mouse,    5);
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {	/* ��������û������ */
			task_sleep(task_a);				/* ˯������A */
			io_sti();
		} else {
			i = fifo32_get(&fifo);			/* ȡ�û������е����� */
			io_sti();
			if (256 <= i && i <= 511) { 	/* ���ڼ��̵����� */
				sprintf(s, "%02X", i - 256);/* ������Ļ����ʾ���ֵ */
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if (i < 0x54 + 256) {
					if (keytable[i - 256] != 0 && cursor_x < 128) { /* һ���ַ� */
						/* ÿ��ʾһ���ַ����������8�����ص�(���ǵ��ַ�����16��8�е�) */
						s[0] = keytable[i - 256];
						s[1] = 0;
						putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
						cursor_x += 8;
					}
				}
				if (i == 256 + 0x0e && cursor_x > 8) {  /* �˸�� */
					/* �ÿո�ѹ����ȥ֮�� ���������8�����ص� */
					putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
					cursor_x -= 8;
				}
				/* �������ʾ */
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) { /* ����������� */
				if (mouse_decode(&mdec, i - 512) != 0) {	/* ������귢�͵����� */
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {		/* ������������ */
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {		/* ����Ҽ������� */
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {		/* ������ֱ����� */
						s[2] = 'C';
					}
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
					/* �����µ����λ�� */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {					/* ����λ�ò���С��0 */
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
					if ((mdec.btn & 0x01) != 0) {		/* ������������ */
						/* �ƶ�sht_win */
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			} else if (i <= 1) { /* ����ö�ʱ�� */
				if (i != 0) {
					timer_init(timer, &fifo, 0);  /* �������趨0 */
					cursor_c = COL8_000000;
				} else {
					timer_init(timer, &fifo, 1); /* �������趨1 */
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer, 50);		/* ���ð��� */
				/* ���һ��С���� */
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				/* ����Ļ����ʾ�Ǹ�С���� */
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
		}
	}
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act)
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
	char c, tc, tbc;
	if (act != 0) {
		tc = COL8_FFFFFF;
		tbc = COL8_000084;
	} else {
		tc = COL8_C6C6C6;
		tbc = COL8_848484;
	}
	/* �Ȼ������ڵ���״ */
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, tbc,         3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	/* ������ڵı��� */
	putfonts8_asc(buf, xsize, 24, 4, tc, title);
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

/* ����һ���ı��� */
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

/* ����B */
void task_b_main(struct SHEET *sht_win_b)
{
	/* ����B���Լ��Ļ����� */
	struct FIFO32 fifo;
	struct TIMER *timer_1s;
	int i, fifobuf[128], count = 0, count0 = 0;
	char s[12];

	fifo32_init(&fifo, 128, fifobuf, 0);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 100);
	timer_settime(timer_1s, 100);

	for (;;) {
		count++;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i == 100) {
				sprintf(s, "%11d", count - count0);
				putfonts8_asc_sht(sht_win_b, 24, 28, COL8_000000, COL8_C6C6C6, s, 11);
				count0 = count;
				timer_settime(timer_1s, 100);
			}
		}
	}
}