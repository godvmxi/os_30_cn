/* ==================================================================
	注释：宅
	时间：2013年2月18日
	该文件中定义了关于描画的函数
   ================================================================== */
#include "bootpack.h"

/* 初始化调色板 */
void init_palette(void)
{
	/* static声明的变量是"静态"的  在程序结束之前一直"存在" */
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:黑 */
		0xff, 0x00, 0x00,	/*  1:亮红 */
		0x00, 0xff, 0x00,	/*  2:亮绿 */
		0xff, 0xff, 0x00,	/*  3:亮黄 */
		0x00, 0x00, 0xff,	/*  4:亮蓝 */
		0xff, 0x00, 0xff,	/*  5:亮紫 */
		0x00, 0xff, 0xff,	/*  6:浅亮蓝 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:亮灰 */
		0x84, 0x00, 0x00,	/*  9:暗红 */
		0x00, 0x84, 0x00,	/* 10:暗绿 */
		0x84, 0x84, 0x00,	/* 11:暗黄 */
		0x00, 0x00, 0x84,	/* 12:暗青 */
		0x84, 0x00, 0x84,	/* 13:暗紫 */
		0x00, 0x84, 0x84,	/* 14:浅暗蓝 */
		0x84, 0x84, 0x84	/* 15:暗灰 */
	};
	set_palette(0, 15, table_rgb);		/* 设置调色板 */
	return;
}

/* 设置调色板函数 */
void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	/* 保存eflags的值 */
	io_cli(); 					/* 关闭所有可屏蔽中断 */
	io_out8(0x03c8, start);		
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	/* 恢复eflags的值 */
	return;
}

/*
	boxfill8函数会在屏幕上画出一个矩形图形
	unsigned char *vram	: 显存首地址
	int	xsize			: 每行的列数
	unsigned char c		: 要填充的颜色 (实际上它是该颜色在调色板中的下标)
	int x0, y0			: 矩形图形的左上角坐标
	int x1, y1			: 矩形图形的右下角坐标
	x表示列 y表示行	例如320*200表示的就是整个屏幕分成200行，每行有320列
 */
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {		/* 行 */
		for (x = x0; x <= x1; x++)		/* 列 */
			vram[y * xsize + x] = c;	/* 填充 */
	}
	return;
}

/* 绘制"桌面"函数 */
void init_screen8(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}


/* 在屏幕上输出单个字符 */
/*
   vram	图像缓冲区首地址	xsize是每行的列数
   x, y 要输出的字符在屏幕上的坐标
   c	字符的颜色
   font	指向要输出的字符(我不知道应该怎么解释这个参数, 它实际上是一个字符指针
						它所指向的是要显示某个字符需要绘制的具体像素, 额。。
						好吧, 不明白的请参考书上P94最下面的'A'字符的图形)
 */
void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {				/* 每次绘制一行*/
		p = vram + (y + i) * xsize + x;	
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

/* 输出ascii码字符串 */
/* vram	图像缓冲区首地址 	xsize是每行的列数	
   x, y 要输出的字符串在屏幕上的起始坐标
   c	字符的颜色			s	要输出的字符串内容
 */
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];		/* 声明外部变量 */
	for (; *s != 0x00; s++) {		/* 如果*s != '\0'就一直循环  '\0'是C语言字符串的结束标志 */
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);	/* 输出单个字符 */
		x += 8;						/* 调整下一个字符的输出位置 */
	}
	return;
}

/* 初始化鼠标图形 */
void init_mouse_cursor8(char *mouse, char bc) 
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	for (y = 0; y < 16; y++) {		
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

/* 显示某一矩形 */
/*
	vram 图像缓冲区首地址	vxsize 每行的列数
	pxsize	要显示的图形的列数	pysize 要显示的图形的行数
	px0, py0 要显示的图形的左上角在屏幕上的坐标
	buf	要显示的图形		bxsize 图形每行的元素数
 */
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}
