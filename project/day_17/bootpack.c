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
/* �����ı������ϽǵĲ�� */
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);
/* ����̨���� */
void console_task(struct SHEET *sheet);

#define KEYCMD_LED		0xed		/* ��Ҫ���͵�LED���� */

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct SHTCTL *shtctl;
	char s[40];				/* ��������� */
	struct FIFO32 fifo, keycmd;	/* fifo�ǹ����Ķ��л����������ṹ keycmd�����������̷��������� */
	int fifobuf[128], keycmd_buf[32];	
	int mx, my, i, cursor_x, cursor_c;	/* cursor_x ��¼���λ��  cursor_c ������ɫ */
	unsigned int memtotal;				/* ��¼�ڴ��С */
	struct MOUSE_DEC mdec;			
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;	
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	struct TASK *task_a, *task_cons;
	struct TIMER *timer;
	/* û�а�shift */
	static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	/* ����shift�� */
	static char keytable1[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};
	int key_to = 0, key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;

	init_gdtidt();		/* ��ʼ��GDT��IDT */
	init_pic();			/* ��ʼ��PIC */
	io_sti(); /* �����п������ж� */
	fifo32_init(&fifo, 128, fifobuf, 0);	/* ���л�������ʼ�� */
	init_pit();			/* ��ʼ��pit */
	init_keyboard(&fifo, 256);	/* ��ʼ�����̿��Ƶ�· */
	enable_mouse(&fifo, 512, &mdec);	/* ������� */
	io_out8(PIC0_IMR, 0xf8); /* PIC0(11111000) (��IRQ0ʱ���жϡ�IRQ1�����жϺ����Ӵ�PIC��IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (��PS2����ж� ��IRQ12)*/
	fifo32_init(&keycmd, 32, keycmd_buf, 0);	/* ��ʼ������������� */

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
	fifo.task = task_a;			/* ���fifo������д��ͻ�������a */
	task_run(task_a, 1, 2);		/* ��������a��level�����ȼ� */

	/* sht_back */
	sht_back  = sheet_alloc(shtctl);	/* ����"����"ͼ�� */
	/* �����ڴ�ռ� ���ڻ���"����" */
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* ����"����"ͼ����Ϣ */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);/* ����"����" */

	/* sht_cons */
	sht_cons = sheet_alloc(shtctl);	/* ��������̨ͼ�� */
	/* �����ڴ�ռ� ���ڻ��ƿ���̨ */
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); /* ���ÿ���̨ͼ����Ϣ */
	make_window8(buf_cons, 256, 165, "console", 0);	/* ���ƿ���̨ͼ�� */
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);	/* ���ƿ���̨ͼ������������������ı��� */
	task_cons = task_alloc();		/* ��ʼ��������̨���� */
	/* ���б�Ҫ�ĳ�ʼ�� */
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	/* ����̨��������,������������A�����в�����û��˯��,���Բ�����ȵ�����̨�������� */
	task_run(task_cons, 2, 2); /* level=2, priority=2 */

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
	/* ���ͼ�ε�������� */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_mouse_cursor8(buf_mouse, 99);
	mx = (binfo->scrnx - 16) / 2; /* �������ͼ������Ļ�ϵ�λ�� �����������������λ�� */
	my = (binfo->scrny - 28 - 16) / 2;

	/* ���ø���ͼ���λ�� */
	sheet_slide(sht_back,  0,  0);
	sheet_slide(sht_cons, 32,  4);
	sheet_slide(sht_win,  64, 56);
	sheet_slide(sht_mouse, mx, my);
	/* ���ø���ͼ��ĸ߶� */
	sheet_updown(sht_back,  0);
	sheet_updown(sht_cons,  1);
	sheet_updown(sht_win,   2);
	sheet_updown(sht_mouse, 3);
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

	/* Ϊ�˱���ͼ��̵�ǰ״̬��ͻ����һ��ʼ�Ƚ������� */
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	/* ��Ҫע���һ���ǣ����жԼ���LED�����ö��ǶԼ����е�һ��оƬ8048������ */
	/* ����������ǰ���õ�i8042,����������ͨ��i8042��ӵĶ�8048�������� */
	/* ����8048�����ϴ�ҿ������аٶȻ��߲ο��ڼ�����һ�����ϴ���Ⱥ������ */
	/* ��i8042�����ϣ�����Ҳ�н��� "i8042_����.pdf" */
	for (;;) {
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			/* �������Ҫ����̷��������ݣ������� */
			keycmd_wait = fifo32_get(&keycmd);	/* ȡ������ */
			wait_KBC_sendready();	/* ���i8042�����뻺���� */
			io_out8(PORT_KEYDAT, keycmd_wait);	/* ��0x60�˿ڷ���0xed */
	/* �����˸�����֮��������Ҫ������0x60�˿ڷ���LED�����ֽڣ�����P347�����˸��ֽڸ�ʽ */
	/* �������ȷ���Ƿ��а��¿���LED�Ƶļ���������LED�����ֽڵ�ֵ��Ȼ���ٷ��� */
		}
		io_cli();	/* �ر����п������ж� */
		if (fifo32_status(&fifo) == 0) {	/* ��������Ļ�����û������ */
			task_sleep(task_a);		/* ˯������A */
			io_sti();				/* ���ж� */
		} else {				/* �Թ��������������ݵĴ��� */
			i = fifo32_get(&fifo);	/* ������� */
			io_sti();				/* ���ж� */
			if (256 <= i && i <= 511) { 	/* ���ڼ��̵����� */
				sprintf(s, "%02X", i - 256);
				/* ������Ļ����ʾ���ֵ */
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if (i < 0x80 + 256) { 
					if (key_shift == 0) {	/* ���shiftû�б����� */
						s[0] = keytable0[i - 256];		/* ����ֵת�����ַ���ASCII�� */
					} else {				/* ���shift�������� */
						s[0] = keytable1[i - 256];		/* ����ֵת�����ַ���ASCII�� */
					}
				} else {	
					s[0] = 0;
				}
				if ('A' <= s[0] && s[0] <= 'Z') {	/* ����������Ӣ����ĸ */
						/* CapsLockΪOFF����ShiftΪOFF ���� */
					if (((key_leds & 4) == 0 && key_shift == 0) ||
						/* CapsLockΪON����ShiftΪON */
							((key_leds & 4) != 0 && key_shift != 0)) {
						s[0] += 0x20;	/* ����д��ĸת����Сд��ĸ */
										/* ͬһ����ĸ�Ĵ�СдASCII�����0x20 */
					}
				}
				if (s[0] != 0) { /* һ���ַ� */
					if (key_to == 0) {	/* ����������A�� */
						if (cursor_x < 128) {
							/* ��ʾһ���ַ�֮�󽫹������һλ */
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
					} else {	/* ���͸�����̨�� */
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}
				if (i == 256 + 0x0e) {	/* �˸�� */
					if (key_to == 0) {	/* ����������A�� */
						if (cursor_x > 8) {
							/* �ÿո�ѹ����ȥ֮�� ���������һλ */
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else {	/* ���͸�����̨�� */
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				if (i == 256 + 0x0f) {	/* Tab */
					if (key_to == 0) {	/* ��ǰ��������A�Ĵ����� */
						key_to = 1;		/* �޸ĸñ��� ��ʶ���ڿ���̨������ */
						/* �޸�����A�Ϳ���̨���ڱ���������ɫ */
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
					} else {		/* ��ǰ���ڿ���̨������ */
						key_to = 0;	/* �޸ĸñ��� ��ʶ��������A�� */
						/* �޸�����A�Ϳ���̨���ڱ���������ɫ */
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
					}
					/* ˢ���������� */
					sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				/* key_shift������bit0��bit1�ֱ��ʶ����shift����shift�Ŀ�����ر�״̬ */
				if (i == 256 + 0x2a) {	/* ��shift ON */
					key_shift |= 1;
				}
				if (i == 256 + 0x36) {	/* ��shift ON */
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) {	/* ��shift OFF */
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6) {	/* ��shift OFF */
					key_shift &= ~2;
				}
				if (i == 256 + 0x3a) {	/* CapsLock */
					key_leds ^= 4;		/* key_leds�б�ʶCapsLock��bitλȡ�� */
					fifo32_put(&keycmd, KEYCMD_LED);	/* ��i8042�����������޸�8048 */
					fifo32_put(&keycmd, key_leds);		/* �ı�CapsLock�ȵ�״̬ */
				}
				if (i == 256 + 0x45) {	/* NumLock */
					key_leds ^= 2;		/* ��CapsLock���ƵĴ��� */
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) {	/* ScrollLock */
					key_leds ^= 1;		/* ��CapsLock���ƵĴ��� */
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				/* 0xfa��ACK��Ϣ����ο�"i8042_����.pdf" */
				if (i == 256 + 0xfa) {	/* ���̳ɹ����յ����� */
					keycmd_wait = -1;	/* ����-1��ʾ���Է���ָ�� */
				}
				if (i == 256 + 0xfe) {	/* ����û�гɹ����յ����� */
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);	/* ���·����ϴε�ָ�� */
				}
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) { /* ������������ */
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
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					sheet_slide(sht_mouse, mx, my);
					if ((mdec.btn & 0x01) != 0) {	/* ������������ */
						/* �ƶ�sht_win */
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			} else if (i <= 1) { /* ����ö�ʱ�� */
				if (i != 0) {
					timer_init(timer, &fifo, 0); /* �������趨0 */
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

/* ���������Ĵ��¿�� */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act)
{
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	make_wtitle8(buf, xsize, title, act);
	return;
}

/* �����������ϽǵĲ�沢��ʾ���� */
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act)
{
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
	boxfill8(buf, xsize, tbc, 3, 3, xsize - 4, 20);
	putfonts8_asc(buf, xsize, 24, 4, tc, title);
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

/* ������� */
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

/* ����̨���� */
void console_task(struct SHEET *sheet)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	int i, fifobuf[128], cursor_x = 16, cursor_c = COL8_000000;
	char s[2];

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	/* ���һ��">" */
	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {	/* ����̨�����Լ��Ļ�������û������ */
			task_sleep(task);					/* ˯���Լ� */
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);		/* �������� */
			io_sti();
			if (i <= 1) { /* ����ö�ʱ�� */
				if (i != 0) {
					timer_init(timer, &task->fifo, 0); /* �������趨0 */
					cursor_c = COL8_FFFFFF;
				} else {
					timer_init(timer, &task->fifo, 1); /* �������趨1 */
					cursor_c = COL8_000000;
				}
				timer_settime(timer, 50);		/* ���ð��� */
			}
			if (256 <= i && i <= 511) { /* ����A���͹����ļ��̵����� */
				if (i == 8 + 256) {		/* ע�⣡������A���Ѿ�����ֵת�����ַ���ASCII���� */
					/* �˸�� */
					if (cursor_x > 16) {
						/* �ÿո�ѹ����ȥ֮�� ���������8�����ص� */
						putfonts8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;
					}
				} else {
					/* һ���ַ� */
					if (cursor_x < 240) {
						/* ��ʾһ���ַ�����������8�����ص� */
						s[0] = i - 256;
						s[1] = 0;
						putfonts8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
			}
			/* ������ʾ��� */
			boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
			sheet_refresh(sheet, cursor_x, 28, cursor_x + 8, 44);
		}
	}
}