/* ==================================================================
	注释：宅
	时间：2013年2月20日
	这是内核的第一个C语言文件,由asmhead.nas跳入,并进入HarrMain函数运行
   ================================================================== */

#include "bootpack.h"
#include <stdio.h>


/* 创建窗口的函数 */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	/* s是输出缓冲区 	keybuf是键盘缓冲区 	mousebuf是鼠标缓冲区*/
	char s[40], keybuf[32], mousebuf[128];
	int mx, my, i;
	unsigned int memtotal, count = 0;	/* memtotal存放内存大小  count是计数器*/
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;	/* 	定义一个MEMMAN结构指针, 
																指向MEMMAN_ADDR也就是0x3c0000
																这就相当于将MEMMAN结构将被存放在0x3c0000处
															*/
	struct SHTCTL *shtctl;		/* 图层管理结构指针 */
	struct SHEET *sht_back, *sht_mouse, *sht_win;	/* 背景、鼠标以及窗口的图层指针 */
	unsigned char *buf_back, buf_mouse[256], *buf_win;

	init_gdtidt();		/* 初始化GDT, IDT */
	init_pic();			/* 初始化PIC */
	io_sti();			/* 打开所有可屏蔽中断 */
	fifo8_init(&keyfifo, 32, keybuf);		/* 初始化键盘缓冲区结构体 */
	fifo8_init(&mousefifo, 128, mousebuf);	/* 初始化鼠标缓冲区结构体 */
	io_out8(PIC0_IMR, 0xf9); /* PIC0(11111001) (打开IRQ1键盘中断和连接从PIC的IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (打开PS2鼠标中断 即IRQ12)*/

	init_keyboard();		/* 初始化键盘控制电路 */
	enable_mouse(&mdec);	/* 激活鼠标 */
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
	make_window8(buf_win, 160, 52, "counter");	/* 绘制窗口图形 */
	sheet_slide(sht_back, 0, 0);				/* 设置"桌面"图层的位置 */
	mx = (binfo->scrnx - 16) / 2; /* 计算鼠标图形在屏幕上的位置 它在整个桌面的中心位置 */
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_mouse, mx, my);				/* 设置鼠标图层的位置 */
	sheet_slide(sht_win, 80, 72);				/* 设置窗口图层的位置 */
	sheet_updown(sht_back,  0);		/* 调整"桌面"图层、鼠标图层和窗口图层的高度 */
	sheet_updown(sht_win,   1);		/* 并且会显示"桌面"、鼠标和窗口图形 */
	sheet_updown(sht_mouse, 2);
	sprintf(s, "(%3d, %3d)", mx, my);	
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);	/* 输出鼠标图形的左上角坐标()注意：此时输出的信息并没有显示在屏幕上 
																		而是记录在"桌面"图层的缓冲区中*/
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);	/* 输出内存大小和空闲内存大小 同上,仅仅是输出到"桌面"图层的缓冲区中 */
	sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);				/* 刷新sht_back图层的某一矩形区域
																   该区域的左上角坐标(0, 0) 右下角坐标(binfo->scrnx, 48) 此处坐标是相对于缓冲区的
																   此时!上面桌面图层该区域的内容(即上面要输出的信息)会显示出来 */

	for (;;) {
		count++;		/* 计数器加1 */
		sprintf(s, "%010d", count);
		boxfill8(buf_win, 160, COL8_C6C6C6, 40, 28, 119, 43);
		putfonts8_asc(buf_win, 160, 40, 28, COL8_000000, s);
		sheet_refresh(sht_win, 40, 28, 120, 44);		/* 在窗口中显示计数器的值 */

		io_cli();		/* 关闭所有可屏蔽中断 */
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {	/* 如果键盘缓冲区或者鼠标缓冲区中都没有数据 */
			io_sti();	/* 重新打开所有可屏蔽中断 并执行下一次循环 */
		} else {		/* 键盘或鼠标缓冲区中有数据 */
			if (fifo8_status(&keyfifo) != 0) {		/* 如果键盘缓冲区中有数据 */
				i = fifo8_get(&keyfifo);			/* 读取数据 */
				io_sti();							/* 打开所有可屏蔽中断 */
				sprintf(s, "%02X", i);				/* 将读取的数据以十六进制形式输出 */
				boxfill8(buf_back, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
				sheet_refresh(sht_back, 0, 16, 16, 32);	/* 刷新sht_back图层缓冲区中的某一矩形区域 用以显示上面的信息 */
			} else if (fifo8_status(&mousefifo) != 0) {	/* 如果鼠标缓冲区中有数据 */
				i = fifo8_get(&mousefifo);		/* 读取数据 */
				io_sti();						/* 打开所有可屏蔽中断 */
				if (mouse_decode(&mdec, i) != 0) {	/* 接收鼠标发送的数据 */
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {	/* 如果左键被按下 */
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {	/* 如果右键被按下 */
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {	/* 如果滚轮被按下 */
						s[2] = 'C';
					}
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					sheet_refresh(sht_back, 32, 16, 32 + 15 * 8, 32);	/* 刷新sht_back图层缓冲区中的某一矩形区域以显示上面的更改 */
					/* 更新新的鼠标位置 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {	/* 鼠标的位置不能小于0 */
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
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 隐藏坐标 以下两句都是对缓冲区中的数据进行更改 */
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 显示新坐标 */
					sheet_refresh(sht_back, 0, 0, 80, 16);		/* 刷新sht_back图层缓冲区中的某一矩形区域以显示上面的更改 */
					sheet_slide(sht_mouse, mx, my);	/* 更新鼠标图层的位置并显示新的鼠标图层 */
				}
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
