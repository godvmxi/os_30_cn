/* ==================================================================
	注释：宅
	时间：2013年2月21日
	这是内核的第一个C语言文件,由asmhead.nas跳入,并进入HarrMain函数运行
   ================================================================== */

#include "bootpack.h"
#include <stdio.h>

/* 创建窗口的函数 */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title);
/* 输出函数 */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
/* 绘制一个文本框 */
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);

/* TSS结构, 请自行百度或参考赵博士的《Linux 内核完全剖析——基于0.11》第四章 */
struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

/* 任务B */
void task_b_main(struct SHEET *sht_back);


void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo;			/* 定义一个公共的队列缓冲区管理结构 */
	char s[40];					/* 输出缓冲区 */
	int fifobuf[128];			/* 队列缓冲区 */
	struct TIMER *timer, *timer2, *timer3;
	/* cursor_x 记录光标位置  cursor_c 光标的颜色 task_b_esp记录栈指针 */
	int mx, my, i, cursor_x, cursor_c, task_b_esp;	
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;	/* 	定义一个MEMMAN结构指针, 
																指向MEMMAN_ADDR也就是0x3c0000
																这就相当于将MEMMAN结构将被存放在0x3c0000处
															*/
	struct SHTCTL *shtctl;		/* 图层管理结构指针 */
	struct SHEET *sht_back, *sht_mouse, *sht_win;	/* 背景、鼠标以及窗口的图层指针 */
	unsigned char *buf_back, buf_mouse[256], *buf_win;
	static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	
	struct TSS32 tss_a, tss_b;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;	/* 申请一个段描述符 */

	init_gdtidt();		/* 初始化GDT, IDT */
	init_pic();			/* 初始化PIC */
	io_sti();			/* 打开所有可屏蔽中断 */
	fifo32_init(&fifo, 128, fifobuf);	/* 初始化键盘缓冲区结构体 */
	init_pit();							/* 初始化PIT */
	init_keyboard(&fifo, 256);			/* 初始化键盘控制电路 */
	enable_mouse(&fifo, 512, &mdec);	/* 激活鼠标 */
	io_out8(PIC0_IMR, 0xf8); /* PIC0(11111000) (打开IRQ0时钟中断、IRQ1键盘中断和连接从PIC的IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (打开PS2鼠标中断 即IRQ12)*/

	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);

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
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);	/* 初始化图层管理结构 */
	sht_back  = sheet_alloc(shtctl);		/* 创建"桌面"图层 */
	sht_mouse = sheet_alloc(shtctl);		/* 创建鼠标图层 */
	sht_win   = sheet_alloc(shtctl);		/* 创建窗口图层 */
	/* 分配内存空间 用于绘制"桌面" */
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);	/* 分配内存空间 用于绘制"窗口 */
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); 	/* 设置"桌面"图层信息 */
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);						/* 设置鼠标图层信息 */
	sheet_setbuf(sht_win, buf_win, 160, 52, -1);						/* 设置窗口图层信息 */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);	/* 绘制"桌面" 注意：这里的第一个参数是buf_back而不是原来的binfo->vram */
														/* 上面让buf_back指向一块新分配的内存,也就是说原本要画在图形缓冲区的"桌面"
														   画在了新分配的那块内存,那块内存存放的就是"桌面"图层需要显示的图形 同时
														   我们要清楚，现在屏幕上并没有原本的桌面背景,因为我们还没有让"桌面"图层显示出来*/
	init_mouse_cursor8(buf_mouse, 99);			/* 绘制鼠标图形 */
	make_window8(buf_win, 160, 52, "windows");	/* 绘制窗口图形 */
	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);	/* 在窗口中绘制文本框 */
	cursor_x = 8;				
	cursor_c = COL8_FFFFFF;
	sheet_slide(sht_back, 0, 0);				/* 设置"桌面"图层的位置 */
	mx = (binfo->scrnx - 16) / 2; /* 计算鼠标图形在屏幕上的位置 它在整个桌面的中心位置 */
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_mouse, mx, my);				/* 设置鼠标图层的位置 */
	sheet_slide(sht_win, 80, 72);				/* 设置窗口图层的位置 */
	sheet_updown(sht_back,  0);		/* 调整"桌面"图层、鼠标图层和窗口图层的高度 */
	sheet_updown(sht_win,   1);		/* 并且会显示"桌面"、鼠标和窗口图形 */
	sheet_updown(sht_mouse, 2);
	sprintf(s, "(%3d, %3d)", mx, my);	
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);		/* 在屏幕上输出s的内容 */
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);	/* 在屏幕上输出s的内容 */

	tss_a.ldtr = 0;			
	tss_a.iomap = 0x40000000;
	tss_b.ldtr = 0;
	tss_b.iomap = 0x40000000;
	set_segmdesc(gdt + 3, 103, (int) &tss_a, AR_TSS32);		/* 设置任务a和任务b的TSS描述符并添加到GDT中 */
	set_segmdesc(gdt + 4, 103, (int) &tss_b, AR_TSS32);
	load_tr(3 * 8);				/* 设置TR寄存器 GDT中第3号描述符也就是任务A */
	/* 因为在任务切换时,cpu会自动保存当前任务的寄存器的值, 所以我们这里只设置了任务B的TSS的值 */
	task_b_esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;	/* 创建任务B的堆栈 */
	tss_b.eip = (int) &task_b_main;		/* 设置任务B的寄存器 */
	tss_b.eflags = 0x00000202; /* IF = 1; */
	tss_b.eax = 0;
	tss_b.ecx = 0;
	tss_b.edx = 0;
	tss_b.ebx = 0;
	tss_b.esp = task_b_esp;
	tss_b.ebp = 0;
	tss_b.esi = 0;
	tss_b.edi = 0;
	tss_b.es = 1 * 8;
	tss_b.cs = 2 * 8;
	tss_b.ss = 1 * 8;
	tss_b.ds = 1 * 8;
	tss_b.fs = 1 * 8;
	tss_b.gs = 1 * 8;
	/* 先将task_b_esp + 4转换成int类型的指针  即(int *) (task_b_esp + 4) */
	/* 再将(int) sht_back赋值到该地址处*((int *) (task_b_esp + 4)) */
	*((int *) (task_b_esp + 4)) = (int) sht_back;		/* 将sht_back预先存入任务B的堆栈中 */
	mt_init();			/* 任务切换的初始化(在该函数中会定义非常重要的mt_timer定时器) */


	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {	/* 缓冲区中没有数据 */
			io_stihlt();
		} else {
			i = fifo32_get(&fifo);			/* 取得缓冲区中的数据 */
			io_sti();
			if (256 <= i && i <= 511) { 	/* 属于键盘的数据 */
				sprintf(s, "%02X", i - 256);/* 现在屏幕上显示其键值 */
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if (i < 0x54 + 256) {
					if (keytable[i - 256] != 0 && cursor_x < 144) { /* 一般字符 */
						/* 每显示一个字符光标就向后右8个像素点(我们的字符就是16行8列的) */
						s[0] = keytable[i - 256];
						s[1] = 0;
						putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
						cursor_x += 8;
					}
				}
				if (i == 256 + 0x0e && cursor_x > 8) { /* 退格键 */
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
					/* 输出s的内容 */
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
					if ((mdec.btn & 0x01) != 0) {	/* 如果左键被按下 */
						/* 移动sht_win */
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			} else if (i == 10) { /* 10定时器*/
				putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
			} else if (i == 3) { /* 3定时器 */
				putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
			} else if (i <= 1) { /* 光标用定时器 */
				if (i != 0) {
					timer_init(timer3, &fifo, 0); /* 下面是设定0 */
					cursor_c = COL8_000000;
				} else {
					timer_init(timer3, &fifo, 1); /* 下面是设定1 */
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer3, 50);		/* 设置半秒 */
				/* 描绘一个小矩形 */
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				/* 在屏幕上显示那个小矩形 */
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
		}
	}
}

/* 创建窗口图形 */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
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
	char c;
	/* 先画出窗口的形状 */
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
	/* 输出窗口的标题 */
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
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
void task_b_main(struct SHEET *sht_back)
{
	struct FIFO32 fifo;
	struct TIMER *timer_put, *timer_1s;
	int i, fifobuf[128], count = 0, count0 = 0;
	char s[12];

	fifo32_init(&fifo, 128, fifobuf);
	timer_put = timer_alloc();		
	timer_init(timer_put, &fifo, 1);
	timer_settime(timer_put, 1);
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
			if (i == 1) {
				sprintf(s, "%11d", count);
				putfonts8_asc_sht(sht_back, 0, 144, COL8_FFFFFF, COL8_008484, s, 11);
				timer_settime(timer_put, 1);
			} else if (i == 100) {
				sprintf(s, "%11d", count - count0);
				putfonts8_asc_sht(sht_back, 0, 128, COL8_FFFFFF, COL8_008484, s, 11);
				count0 = count;
				timer_settime(timer_1s, 100);
			}
		}
	}
}
