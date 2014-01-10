/* ==================================================================
	注释：宅
	时间：2013年2月23日
	这是内核的第一个C语言文件,由asmhead.nas跳入,并进入HarrMain函数运行
   ================================================================== */
/* 
   今天的程序中涉及到了对软盘数据的操作，事实上大家翻一翻ipl10.nas就知道
   我们软盘使用的是FAT12文件系统。关于FAT12的细节网上资料很多。就不给出
   推荐资料了。
 */   
 
#include "bootpack.h"
#include <stdio.h>
#include <string.h>

/* 根目录区条目结构 */
/* 
   name			文件名	
   ext			扩展名
   type			文件类型 
   reserve		保留
   time			时间
   date			日期
   clustno		簇号	(作者说簇号可以理解为扇区号，
						那是因为我们在ip10中的最头部设置了每簇中只有1个扇区)
   size			文件大小
 */
struct FILEINFO {
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
};

/* 声明部分 */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);
void console_task(struct SHEET *sheet, unsigned int memtotal);
int cons_newline(int cursor_y, struct SHEET *sheet);

#define KEYCMD_LED		0xed		/* 需要发送的LED数据 */

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct SHTCTL *shtctl;
	char s[40];					/* 输出缓冲区 */
	/* fifo是公共的队列缓冲区管理结构 keycmd用来存放向键盘发生的命令 */
	struct FIFO32 fifo, keycmd;
	int fifobuf[128], keycmd_buf[32];
	/* cursor_x 记录光标位置  cursor_c 光标的颜色同时也用来判断是否显示光标 */
	int mx, my, i, cursor_x, cursor_c;
	unsigned int memtotal;			/* 记录内存大小 */
	struct MOUSE_DEC mdec;			
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	struct TASK *task_a, *task_cons;
	struct TIMER *timer;
	/* 没有按shift */
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
	/* 按下shift后 */
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

	init_gdtidt();			/* 初始化GDT、IDT */
	init_pic();				/* 初始化PIC */
	io_sti(); 				/* 打开所有可屏蔽中断 */
	fifo32_init(&fifo, 128, fifobuf, 0);	/* 队列缓冲区初始化 */
	init_pit();				/* 初始化PIT */
	init_keyboard(&fifo, 256);	/* 初始化i8042 */
	enable_mouse(&fifo, 512, &mdec);	/* 激活鼠标 */
	io_out8(PIC0_IMR, 0xf8); /* PIC0(11111000) (打开IRQ0时钟中断、IRQ1键盘中断和连接从PIC的IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (打开PS2鼠标中断 即IRQ12)*/
	fifo32_init(&keycmd, 32, keycmd_buf, 0);	/* 初始化键盘命令缓冲区 */	

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
	task_a = task_init(memman);
	/* 此处开始运行于任务A */
	fifo.task = task_a;		/* 如果fifo有数据写入就唤醒task_a */
	task_run(task_a, 1, 2);	/* 设置task_a的level和优先级 */

	/* sht_back */
	sht_back  = sheet_alloc(shtctl);	/* 创建"桌面"图层 */
	/* 分配内存空间  用于绘制“桌面” */
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	/* 设置桌面图层信息 */
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	/* 绘制"桌面"图形 */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	/* sht_cons */
	sht_cons = sheet_alloc(shtctl);	/* 创建控制台图层 */
	/* 分配内存空间  用于绘制控制台 */
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	/* 设置控制台图层的信息 */
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); 
	/* 绘制文本框 */
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	/* 创建控制台任务并进行必要的初始化 */
	task_cons = task_alloc();	
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp + 8)) = memtotal;
	/* 控制台可以运行,不过现在任务A在运行并且它没有睡眠,所以不会调度到控制台任务运行 */
	task_run(task_cons, 2, 2); /* level=2, priority=2 */

	/* sht_win */
	/* 这是任务A的窗口 */
	sht_win   = sheet_alloc(shtctl);	/* 创建任务A窗口图层 */
	/* 分配内存空间 */
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	/* 设置任务A窗口信息 */
	sheet_setbuf(sht_win, buf_win, 144, 52, -1); 
	make_window8(buf_win, 144, 52, "task_a", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	timer = timer_alloc();		/* 光标的定时器 */
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);

	/* sht_mouse */
	/* 鼠标图形的相关设置 */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_mouse_cursor8(buf_mouse, 99);
	mx = (binfo->scrnx - 16) / 2;  /* 计算鼠标图形在屏幕上的位置 它在整个桌面的中心位置 */
	my = (binfo->scrny - 28 - 16) / 2;

	/* 设置各个图层的位置 */
	sheet_slide(sht_back,  0,  0);
	sheet_slide(sht_cons, 32,  4);
	sheet_slide(sht_win,  64, 56);
	sheet_slide(sht_mouse, mx, my);
	/* 设置各个图层的高度 */
	sheet_updown(sht_back,  0);
	sheet_updown(sht_cons,  1);
	sheet_updown(sht_win,   2);
	sheet_updown(sht_mouse, 3);

	/* 为了避免和键盘当前状态冲突，在一开始先进行设置 */
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);


	/* 需要注意的一点是，所有对键盘LED的设置都是对键盘中的一块芯片8048的设置 */
	/* 不是我们以前设置的i8042,不过我们是通过i8042间接的对8048进行设置 */
	/* 关于8048的资料大家可以自行百度或者参考在键盘那一章我上传到群共享中 */
	/* 的i8042的资料，里面也有讲解 "i8042_键盘.pdf" */
	for (;;) {
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			/* 如果存在要向键盘发生的数据，则发送它 */
			keycmd_wait = fifo32_get(&keycmd);	/* 取出数据 */
			wait_KBC_sendready();	/* 清空i8042的输入缓冲区 */
			io_out8(PORT_KEYDAT, keycmd_wait);	/* 向0x60端口发生0xed */
	/* 发生了该命令之后，我们需要继续向0x60端口发生LED设置字节，书上P347描述了该字节格式 */
	/* 在下面等确认是否有按下控制LED灯的键后来决定LED设置字节的值，然后再发生 */
		}
		io_cli();			/* 关闭所有可屏蔽中断 */
		if (fifo32_status(&fifo) == 0) {	/* 如果公共的缓冲区没有数据 */
			task_sleep(task_a);		/* 睡眠任务A */
			io_sti();				/* 打开中断 */
		} else {			/* 对公共缓冲区中数据的处理 */
			i = fifo32_get(&fifo);	/* 获得数据 */
			io_sti();				/* 打开中断 */
			if (256 <= i && i <= 511) {	 	/* 属于键盘的数据 */
				if (i < 0x80 + 256) { 	/* 对键盘输入的处理，键盘各个键的序号是0~0x79 */
										/* 这里大于0x80+256又小于512的估计是留待将来使用的 */
					if (key_shift == 0) {	/* 如果shift没有被按下 */
						s[0] = keytable0[i - 256];	/* 将键值转换为字符ASCII码 */
					} else {				/* 如果shift被按下 */
						s[0] = keytable1[i - 256];	/* 将键值转换为字符ASCII码 */
					}
				} else {  /* 可能留待将来使用 */
					s[0] = 0;
				}
				if ('A' <= s[0] && s[0] <= 'Z') {	/* 如果输入的是英文字母 */
						/* CapsLock为OFF并且Shift为OFF 或者 */
					if (((key_leds & 4) == 0 && key_shift == 0) ||
						/* CapsLock为ON并且Shift为ON */
						((key_leds & 4) != 0 && key_shift != 0)) {
						s[0] += 0x20;	/* 将大写字母转换成小写字母 */
										/* 同一个字母的大小写ASCII码相差0x20 */
					}
				}
				if (s[0] != 0) { /* 一般字符 */
					if (key_to == 0) {	/* 发送给任务A的 */
						if (cursor_x < 128) {
							/* 显示一个字符之后将光标右移一位 */
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
					} else {	/* 发送给控制台的 */
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}
				if (i == 256 + 0x0e) {	/* 退格键 */
					if (key_to == 0) {	/* 发生给任务A的 */
						if (cursor_x > 8) {
							/* 用空格把光标消去之后 光标向左移一位 */
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else {	/* 发送给控制台的 */
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				if (i == 256 + 0x1c) {	/* Enter */
					if (key_to != 0) {	/* 发送给控制台的 */
						fifo32_put(&task_cons->fifo, 10 + 256);
					}
				}
				if (i == 256 + 0x0f) {	/* Tab */
					if (key_to == 0) {	/* 当前处于任务A的窗口中 */
						key_to = 1;		/* 修改该变量 标识处于控制台窗口中 */
						/* 修改任务A和控制台窗口标题栏的颜色 */
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1; /* 表示不显示光标(任务A的窗口中) */
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
						fifo32_put(&task_cons->fifo, 2); /* 控制台窗口光标ON */
					} else {			/* 当前处于控制台窗口中 */
						key_to = 0;		/* 修改该变量 标识处于任务A中 */
						/* 修改任务A和控制台窗口标题栏的颜色 */
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000; /* 设置光标颜色 */
						fifo32_put(&task_cons->fifo, 3); /* 控制台窗口光标OFF */
					}
					/* 刷新两个窗口 */
					sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				/* key_shift变量的bit0和bit1分别标识了左shift和右shift的开启与关闭状态 */
				if (i == 256 + 0x2a) {	/* 左shift ON */
					key_shift |= 1;
				}
				if (i == 256 + 0x36) {	/* 右shift ON */
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) {	/* 左shift OFF */
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6) {	/* 右shift OFF */
					key_shift &= ~2;
				}
				if (i == 256 + 0x3a) {	/* CapsLock */
					key_leds ^= 4;		/* key_leds中标识CapsLock的bit位取反 */
					fifo32_put(&keycmd, KEYCMD_LED);	/* 向i8042发生命令来修改8048 */
					fifo32_put(&keycmd, key_leds);		/* 改变CapsLock等的状态 */
				}
				if (i == 256 + 0x45) {	/* NumLock */
					key_leds ^= 2;		/* 和CapsLock类似的处理 */
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) {	/* ScrollLock */
					key_leds ^= 1;		/* 和CapsLock类似的处理 */
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				/* 0xfa是ACK信息，请参考"i8042_键盘.pdf" */
				if (i == 256 + 0xfa) {	/* 键盘成功接收到数据 */
					keycmd_wait = -1;	/* 等于-1表示可以发送指令 */
				}
				if (i == 256 + 0xfe) {	/* 键盘没有成功接收到数据 */
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);	/* 重新发送上次的指令 */
				}
			
				if (cursor_c >= 0) {		/* 如果需要显示光标 */
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) { /* 属于鼠标的数据 */
				if (mouse_decode(&mdec, i - 512) != 0) {
					/* 更新新的鼠标位置 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					sheet_slide(sht_mouse, mx, my);
					if ((mdec.btn & 0x01) != 0) {		/* 如果左键被按下 */
						/* 移动sht_win */
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			} else if (i <= 1) { /* 光标用定时器 */
				if (i != 0) {
					timer_init(timer, &fifo, 0);  /* 下面是设定0 */
					if (cursor_c >= 0) {			
						cursor_c = COL8_000000;		
					}
				} else {
					timer_init(timer, &fifo, 1); /* 下面是设定1 */
					if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				}
				timer_settime(timer, 50);		/* 设置半秒 */
				if (cursor_c >= 0) {			/* 如果需要显示光标 */
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				}
			}
		}
	}
}


/* 画出窗口的大致框架 */
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
/* 画出窗口右上角的叉叉并显示标题 */
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

/* 输出函数 */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

/* 画出一个文本框 */
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


/* 控制台任务 */
void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	int i, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_c = -1;
	char s[30], cmdline[30];
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int x, y;
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	/* 输出一个">" */
	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {	/* 控制台任务自己的缓冲区中没有数据 */
			task_sleep(task);					/* 睡眠自己 */
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);		/* 读出数据 */
			io_sti();
			if (i <= 1) { 						/* 光标用定时器 */
				if (i != 0) {
					timer_init(timer, &task->fifo, 0); /* 下面是设定0 */
					if (cursor_c >= 0) {			/* 如果需要显示光标 */
						cursor_c = COL8_FFFFFF;
					}
				} else {
					timer_init(timer, &task->fifo, 1); /* 下面是设定1 */
					if (cursor_c >= 0) {			/* 如果需要显示光标 */
						cursor_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);			/* 设置半秒 */
			}
			if (i == 2) {	/* 光标ON */
				cursor_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* 光标OFF */
				/* 直接用黑色将原来光标的位置消去 */
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
				cursor_c = -1;
			}
			if (256 <= i && i <= 511) { /* 任务A传送过来的键盘的数据 */
				if (i == 8 + 256) {		/* 注意！在任务A中已经将键值转换成字符的ASCII码了 */
					/* 退格键 */
					if (cursor_x > 16) {
						/* 用空格把光标消去之后 光标向左移8个像素点 */
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;
					}
				} else if (i == 10 + 256) {	/* 如果是回车 (10是换行符的ASCII码) */
					/* Enter */
					/* 用空格将光标擦除 */
					putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					cmdline[cursor_x / 8 - 2] = 0;	/* 计算前面有几个字符 在字符的后面加上'\0' */
													/* 加'\0'是因为C语言中字符串都是以它结尾的 */
													/* 减2是减去'\0'的位置还有一个回车的位置 所*/
													/* 以最后'\0'是刚好在最后一个字符后面 */
					cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
					/* 执行命令 */
					if (strcmp(cmdline, "mem") == 0) {
						/* mem命令 */
						/* 输出内存总大小和空闲内存大小 */
						sprintf(s, "total   %dMB", memtotal / (1024 * 1024));
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
						sprintf(s, "free %dKB", memman_total(memman) / 1024);
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
						cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
					} else if (strcmp(cmdline, "cls") == 0) {
						/* cls命令 */
						/* 最简单。。每个像素都设为黑色..... */
						for (y = 28; y < 28 + 128; y++) {
							for (x = 8; x < 8 + 240; x++) {
								sheet->buf[x + y * sheet->bxsize] = COL8_000000;
							}
						}
						sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
						cursor_y = 28;
					} else if (strcmp(cmdline, "dir") == 0) {
						/* dir命令 */
						for (x = 0; x < 224; x++) {	/* 遍历所有的根目录条目 */
													/* 这个224是在ipl10中定义的 */
							if (finfo[x].name[0] == 0x00) {	/* 该条目不包含任何文件信息 */
								break;
							}
							if (finfo[x].name[0] != 0xe5) {	/* 该条目包含文件信息 */
								if ((finfo[x].type & 0x18) == 0) {	/* 该条目记录的不是目录 也不是非文件信息 */
									sprintf(s, "filename.ext   %7d", finfo[x].size);
									for (y = 0; y < 8; y++) {	/* 设置文件名 */
										s[y] = finfo[x].name[y];
									}
									s[ 9] = finfo[x].ext[0];	/* 设置扩展名 */
									s[10] = finfo[x].ext[1];
									s[11] = finfo[x].ext[2];
									putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
									cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
								}
							}
						}
						cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
					} else if (cmdline[0] != 0) {
						/* 控制台有输入但不是上面的命令 */
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);
					}
					/* 输出一个'>' */
					putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 16;
				} else {
					/* 一般字符 */
					if (cursor_x < 240) {
						/* 显示一个字符后光标向右移8个像素点 */
						s[0] = i - 256;
						s[1] = 0;
						cmdline[cursor_x / 8 - 2] = i - 256;
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
			}
			/* 如果需要显示光标 */
			if (cursor_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
			}
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}



int cons_newline(int cursor_y, struct SHEET *sheet)
{
	int x, y;
	if (cursor_y < 28 + 112) {		/* 对这几个数字不太明白的仔细观察下控制台的形状 */
		cursor_y += 16; /* 换行 */
	} else {
		/* 滚动 */
		for (y = 28; y < 28 + 112; y++) {
			for (x = 8; x < 8 + 240; x++) {
				/* 将下一行的字符串移到上一行中...我不会描述了。。。。 */
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		/* 最下面一行填充颜色 */
		for (y = 28 + 112; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}
	return cursor_y;
}
