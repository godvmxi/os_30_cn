/* ==================================================================
	注释：宅
	时间：2013年2月18日
   ================================================================== */
#include <stdio.h>					

/* 函数声明部分 */
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen8(char *vram, int x, int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize);
/* 符号常量声明 */
/* 声明相应颜色在调色板中的下标 */
#define COL8_000000		0		/* 黑色 在调色板中下标为0 */
#define COL8_FF0000		1		/* 亮红色 在调色板中下标为1 */
#define COL8_00FF00		2		/* 亮绿色 在调色板中下标为2 */
#define COL8_FFFF00		3		/* 亮黄色 在调色板中下标为3 */
#define COL8_0000FF		4		/* 亮蓝色 在调色板中下标为4 */	
#define COL8_FF00FF		5		/* 亮紫色 在调色板中下标为5 */
#define COL8_00FFFF		6		/* 浅亮蓝色 在调色板中下标为6 */
#define COL8_FFFFFF		7		/* 白色 在调色板中下标为7 */
#define COL8_C6C6C6		8		/* 亮灰色 在调色板中下标为8 */
#define COL8_840000		9		/* 暗红色 在调色板中下标为9 */
#define COL8_008400		10		/* 暗绿色 在调色板中下标为10 */
#define COL8_848400		11		/* 暗黄色 在调色板中下标为11 */
#define COL8_000084		12		/* 暗蓝色 在调色板中下标为12 */
#define COL8_840084		13		/* 暗紫色 在调色板中下标为13 */
#define COL8_008484		14		/* 浅暗蓝色 在调色板中下标为14 */
#define COL8_848484		15		/* 暗灰色 在调色板中下标为15 */


/* 存储启动信息的结构体 */
/*
	cyls 存放着在ipl10中读入的柱面数	leds 存放着led灯的状态
	vmode 存放着关于颜色数目的信息。颜色的位数	reserve 是"占位符" 在asmhead.nas中该字节未使用
	scrnx 分辨率的x		scrny 分辨率的y		vram 图像缓冲区首地址
 */	
struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

/* 存放段描述符的结构体 */
/* 关于段描述符可自行百度 因为涉及保护模式 所以相关内容很多 无法一一注释
   个人认为暂时没必要纠结于保护模式的相关内容,而且作者也没有打算在现在就
   讲解过多关于保护模式的知识, 如果有兴趣的话可以参考Intel 80386手册的卷3
   或者赵博士的《Linux 内核完全剖析——基于0.11内核》的第四章的相关内容
 */
/*
	limit_low	段限长的0~15bit		base_low	段基址的0~15bit
	base_mid	段基址的16~23bit	access_right(0~3bit TYPE字段, 4bit S字段 5~6 DPL字段 7bit P字段)
	limit_high	段限长的16~19bit+AVL+D/B+G域	base_high	段基址的24~31bit
	- - 好吧 可能我上面关于段描述符的注释不是很清楚， 不理解的可以参考下面链接处的图片
	http://baike.baidu.com/picview/3289301/3289301/0/0db52faddf823c3d4b36d686.html#albumindex=0&picindex=0
 */
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

/* 存放门描述符的结构体 */
/*
	门描述符与段描述符类似,只是有些字段有些许差别
	offset_low	过程偏移地址的低16bit	selector 段选择子
	dw_count	参数个数, 只是调用门中有效
	access_right	0~3bit:TYPE字段, 4bit:S字段, 5~6bit:DPL字段, 7bit:P字段
	offset_high	过程偏移地址的高16bit
 */
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};


/* 函数声明 */
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	/* s是输出缓冲区	mcursor存放鼠标图形 */
	char s[40], mcursor[256];		
	/* mx, my是鼠标图形的左上角位置 */
	int mx, my;

	init_gdtidt();		/* 初始化GDT, IDT */
	init_palette();		/* 初始化调色板 */
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);	/* 绘制"桌面" */
	mx = (binfo->scrnx - 16) / 2; /* 计算鼠标图形在屏幕上的位置 它在整个桌面的中心位置 */
	my = (binfo->scrny - 28 - 16) / 2;	
	init_mouse_cursor8(mcursor, COL8_008484);	
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* 显示鼠标图形 */
	sprintf(s, "(%d, %d)", mx, my);										
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);		/* 输出鼠标图形左上角在屏幕上的坐标 */

	for (;;) {
		io_hlt();
	}
}

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

/* 显示某一图形 */
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

/* 初始化GDT, IDT */
void init_gdtidt(void)
{
	/* GDT的位置紧随在IDT之后 */
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) 0x0026f800;
	
	int i;

	/* GDT初始化 */
	for (i = 0; i < 8192; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	/* 编号为0的描述符是空描述符 是Intel要求必须这样设置的 */
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);	/* 内核数据段 
															   32位可读写数据段描述符 段限长4G-1 段基址0  DPL = 0*/
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);	/* 内核代码段 
																32位可读可执行代码段描述符 段限长512KB  段基址0x280000 DPL = 0*/
	load_gdtr(0xffff, 0x00270000);							/* 加载GDT */

	/* IDT初始化 */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7ff, 0x0026f800);							/* 加载IDT */

	return;
}

/* 设置段描述符 */
/*
	sd 段描述符结构指针	limit 段限长	base 段基址		ar 段描述符属性
 */
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {			/* 如果限长大于1M (20bit的段限长若以字节为单位最多只能表示1M的内存)*/
		ar |= 0x8000; /* G_bit = 1 */	/* 描述符中有一个G位， 若G = 1则表示段限长是以4KB为单位
											若G = 0 则表示段限长是以字节为单位的 */
		limit /= 0x1000;				/* 调整段限长的值即除以4K */
	}
	
	/* 将各值填入相应的域中 可参考上面给出的段描述符图 */
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

/* 设置门描述符 */
/*
	gd 门描述符结构指针		offset 过程入口偏移
	selector 段选择子		ar 门描述符属性
 */
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
