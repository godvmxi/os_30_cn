/* ==================================================================
	注释：宅
	时间：2013年2月22日
	这是内核的第一个C语言文件,由asmhead.nas跳入,并进入HarrMain函数运行
   ================================================================== */

#include "bootpack.h"
#include <stdio.h>

/* 创建窗口的函数 */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
/* 输出函数 */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
/* 绘制一个文本框 */
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
/* 任务B */
void task_b_main(struct SHEET *sht_back);


void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo;		/* 定义一个公共的队列缓冲区管理结构 */
	char s[40];				/* 输出缓冲区 */
	int fifobuf[128];		/* 队列缓冲区 */
	/* cursor_x 记录光标位置  cursor_c 光标的颜色 */
	int mx, my, i, cursor_x, cursor_c;
	unsigned int memtotal;		/* 记录内存大小 */
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;	/* 	定义一个MEMMAN结构指针, 
																指向MEMMAN_ADDR也就是0x3c0000
																这就相当于将MEMMAN结构将被存放在0x3c0000处
															*/
	struct SHTCTL *shtctl;	/* 图层管理结构指针 */
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

	init_gdtidt();		/* 初始化GDT、IDT */
	init_pic();			/* 初始化PIC */
	io_sti(); /* 打开所有可屏蔽中断 */
	fifo32_init(&fifo, 128, fifobuf, 0);	/* 队列缓冲区初始化 */
	init_pit();			/* 初始化pit */
	init_keyboard(&fifo, 256);	/* 初始化键盘控制电路 */
	enable_mouse(&fifo, 512, &mdec);	/* 激活鼠标 */
	io_out8(PIC0_IMR, 0xf8); /* PIC0(11111000) (打开IRQ0时钟中断、IRQ1键盘中断和连接从PIC的IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (打开PS2鼠标中断 即IRQ12)*/

	memtotal = memtest(0x00400000, 0xbfffffff);	/* 检测4M~3G-1的内存  memtotal是内存实际大小 */
	memman_init(memman);	/* 初始化内存管理结构 */
	memman_free(memman, 0x00001000, 0x0009e000); 	/* 0x00001000 - 0x0009efff */
													/* 这段内存是4K~636K-1, 这段内存是"安全"的
													   具体这段内存中有哪些数据可参考群共享中的
													   "内存分布图.wps"(注意：此时我们引导扇区即ipl10
													   的512B也在此范围之内,所以说那512B内存现在也是空闲的)
													  */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);	/* 4M~内存实际大小 */

	init_palette();		/* 初始化调色板 */
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny); /* 初始化图层管理结构 */
	task_a = task_init(memman);		
	/* 从此处开始运行于任务a */
	/* 在task_init中虽然设置了定时器, 但是由于现在系统中只有一个任务 所以不会进行任务切换task_switch中会判断 */
	fifo.task = task_a;	/* 如果fifo有数据写入就唤醒任务a */
	task_run(task_a, 1, 2);	/* 设置任务a的level和优先级 */

	/* sht_back */
	sht_back  = sheet_alloc(shtctl);		/* 创建"桌面"图层 */
	/* 分配内存空间 用于绘制"桌面" */
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);	/* 设置"桌面"图层信息 */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);	/* 绘制"桌面" 注意：这里的第一个参数是buf_back而不是原来的binfo->vram */
														/* 上面让buf_back指向一块新分配的内存,也就是说原本要画在图形缓冲区的"桌面"
														   画在了新分配的那块内存,那块内存存放的就是"桌面"图层需要显示的图形 同时
														   我们要清楚，现在屏幕上并没有原本的桌面背景,因为我们还没有让"桌面"图层显示出来*/

	/* sht_win_b */
	for (i = 0; i < 3; i++) {		/* 3个窗口 */
		sht_win_b[i] = sheet_alloc(shtctl);		/* 创建窗口图层 */
		buf_win_b = (unsigned char *) memman_alloc_4k(memman, 144 * 52);	/* 分配内存用于绘制窗口 */
		sheet_setbuf(sht_win_b[i], buf_win_b, 144, 52, -1); /* 设置窗口图层信息 */
		sprintf(s, "task_b%d", i);	
		make_window8(buf_win_b, 144, 52, s, 0);	/* 创建窗口 */
		task_b[i] = task_alloc();		
		task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
		task_b[i]->tss.eip = (int) &task_b_main;
		task_b[i]->tss.es = 1 * 8;
		task_b[i]->tss.cs = 2 * 8;
		task_b[i]->tss.ss = 1 * 8;
		task_b[i]->tss.ds = 1 * 8;
		task_b[i]->tss.fs = 1 * 8;
		task_b[i]->tss.gs = 1 * 8;	
		*((int *) (task_b[i]->tss.esp + 4)) = (int) sht_win_b[i];	/* 将sht_win预先存入任务B[i]的堆栈中 */
		task_run(task_b[i], 2, i + 1);	/* 任务B[i]可以运行 这个时候就可能发生任务切换了 */
	}

	/* sht_win */
	/* 这是任务A的窗口 */
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
	mx = (binfo->scrnx - 16) / 2; 	/* 计算鼠标图形在屏幕上的位置 它在整个桌面的中心位置 */
	my = (binfo->scrny - 28 - 16) / 2;

	/* 设置各个图层的位置 */
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_win_b[0], 168,  56);
	sheet_slide(sht_win_b[1],   8, 116);
	sheet_slide(sht_win_b[2], 168, 116);
	sheet_slide(sht_win,        8,  56);
	sheet_slide(sht_mouse, mx, my);
	/* 设置各个图层的高度 */
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
		if (fifo32_status(&fifo) == 0) {	/* 缓冲区中没有数据 */
			task_sleep(task_a);				/* 睡眠任务A */
			io_sti();
		} else {
			i = fifo32_get(&fifo);			/* 取得缓冲区中的数据 */
			io_sti();
			if (256 <= i && i <= 511) { 	/* 属于键盘的数据 */
				sprintf(s, "%02X", i - 256);/* 先在屏幕上显示其键值 */
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if (i < 0x54 + 256) {
					if (keytable[i - 256] != 0 && cursor_x < 128) { /* 一般字符 */
						/* 每显示一个字符光标就向后右8个像素点(我们的字符就是16行8列的) */
						s[0] = keytable[i - 256];
						s[1] = 0;
						putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
						cursor_x += 8;
					}
				}
				if (i == 256 + 0x0e && cursor_x > 8) {  /* 退格键 */
					/* 用空格把光标消去之后 光标向左移8个像素点 */
					putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
					cursor_x -= 8;
				}
				/* 光标再显示 */
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) { /* 属于鼠标数据 */
				if (mouse_decode(&mdec, i - 512) != 0) {	/* 接收鼠标发送的数据 */
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {		/* 如果左键被按下 */
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {		/* 如果右键被按下 */
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {		/* 如果滚轮被按下 */
						s[2] = 'C';
					}
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
					/* 更新新的鼠标位置 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {					/* 鼠标的位置不能小于0 */
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {	/* 同样是范围控制 */
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					/* 输出s的内容 */
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					/* 更新鼠标图层的位置并显示新的鼠标图层 */
					sheet_slide(sht_mouse, mx, my);
					if ((mdec.btn & 0x01) != 0) {		/* 如果左键被按下 */
						/* 移动sht_win */
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			} else if (i <= 1) { /* 光标用定时器 */
				if (i != 0) {
					timer_init(timer, &fifo, 0);  /* 下面是设定0 */
					cursor_c = COL8_000000;
				} else {
					timer_init(timer, &fifo, 1); /* 下面是设定1 */
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer, 50);		/* 设置半秒 */
				/* 描绘一个小矩形 */
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				/* 在屏幕上显示那个小矩形 */
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
		}
	}
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act)
{
	/* 这个数组定义的是窗口右上角那个叉叉图形 */
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
	/* 先画出窗口的形状 */
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
	/* 输出窗口的标题 */
	putfonts8_asc(buf, xsize, 24, 4, tc, title);
	/* 画出右上角的叉叉 */
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

/* 输出函数... */
/* 
	x,y	显示位置的左上角坐标
	c	字符颜色
	b	背景颜色
	s	字符串
	l	字符串长度
 */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

/* 绘制一个文本框 */
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

/* 任务B */
void task_b_main(struct SHEET *sht_win_b)
{
	/* 任务B有自己的缓冲区 */
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
