/* ==================================================================
	注释：宅
	时间：2013年2月20日
	该头文件中有各文件中的函数声明以及一些符号常量的定义
   ================================================================== */
   
/* asmhead.nas */
struct BOOTINFO { /* 0x0ff0-0x0fff */
	char cyls; 			/* 存放着在ipl10中读入的柱面数 */
	char leds; 			/* 存放着键盘led灯的状态 */
	char vmode; 		/* 显卡模式为多少位彩色 */
	char reserve;		/* 是"占位符" 在asmhead.nas中该字节未使用 */
	short scrnx, scrny; /* 画面分辨率 */
	char *vram;			/* 图像缓冲区首地址 */
};
#define ADR_BOOTINFO	0x00000ff0	/* 存放启动信息的地址 */

/* naskfunc.nas */
void io_hlt(void);					/* hlt */
void io_cli(void);					/* 关闭所有可屏蔽中断 */
void io_sti(void);					/* 打开所有可屏蔽中断 */
void io_stihlt(void);				/* 打开所有可屏蔽中断并待机 */
int io_in8(int port);				/* 从port端口读取一字节数据 */
void io_out8(int port, int data);	/* 将data写入到port端口 */
int io_load_eflags(void);			/* 获取eflags寄存器的值 */
void io_store_eflags(int eflags);	/* 设置eflags寄存器的值 */
void load_gdtr(int limit, int addr);/* 加载GDT(或者说 设置GDTR寄存器) */
void load_idtr(int limit, int addr);/* 加载IDT(或者说 设置IDTR寄存器) */
int load_cr0(void);					/* 获取CR0寄存器的值 */
void store_cr0(int cr0);			/* 设置CR0寄存器的值 */
void asm_inthandler21(void);		/* IRQ1服务程序 */
void asm_inthandler27(void);		/* IRQ7服务程序 */
void asm_inthandler2c(void);		/* IRQ12服务程序 */
unsigned int memtest_sub(unsigned int start, unsigned int end);	/* 内存检测函数 */

/* fifo.c */
struct FIFO8 {						/* 缓冲区结构体 */
	unsigned char *buf;				/* 缓冲区指针 */
	int p, q, size, free, flags;	/*	p 下一个数据写入位置	q 下一个数据读出位置
										size 缓冲区的总字节数	free 缓冲区的空闲字节数
										flags	记录缓冲区是否溢出
									*/
};
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);	/* 缓冲区初始化函数 */
int fifo8_put(struct FIFO8 *fifo, unsigned char data);	/* 缓冲区写入函数 */
int fifo8_get(struct FIFO8 *fifo);						/* 缓冲区读出函数 */
int fifo8_status(struct FIFO8 *fifo);					/* 返回缓冲区状态信息 */



/* graphic.c */
void init_palette(void);			/* 初始化调色板 */
void set_palette(int start, int end, unsigned char *rgb);	/* 设置调色板 */
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1); /* 在屏幕上画出一个矩形图形 */
void init_screen8(char *vram, int x, int y);				/* 绘制"桌面" */
void putfont8(char *vram, int xsize, int x, int y, char c, char *font); /* 在屏幕上输出单个字符 */
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s); /* 输出ascii码字符串 */
void init_mouse_cursor8(char *mouse, char bc);				/* 初始化鼠标图形 */
void putblock8_8(char *vram, int vxsize, int pxsize,		/* 显示某一图形 */
	int pysize, int px0, int py0, char *buf, int bxsize);
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


/* dsctbl.c */
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

void init_gdtidt(void);				/* 初始化GDT, IDT */
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);	/* 设置段描述符 */
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);		/* 设置门描述符 */
#define ADR_IDT			0x0026f800		/* IDT首地址 */
#define LIMIT_IDT		0x000007ff		/* IDT限长 */
#define ADR_GDT			0x00270000		/* GDT首地址 */
#define LIMIT_GDT		0x0000ffff		/* GDT限长 */
#define ADR_BOTPAK		0x00280000		/* 内核代码首地址 */
#define LIMIT_BOTPAK	0x0007ffff		/* 内核代码限长(共512KB) */
#define AR_DATA32_RW	0x4092			/* 32位可读写数据段描述符属性值 G = 0 DPL = 0 */
#define AR_CODE32_ER	0x409a			/* 32位可读可执行代码段描述符属性值 G = 0 DPL = 0 */
#define AR_INTGATE32	0x008e			/* 32位中断门描述符属性值 DPL = 0 */


/* int.c */
void init_pic(void);					/* 初始化PIC */			
void inthandler27(int *esp);			/* IRQ7的处理函数 */
/* PIC中各个端口号 */
#define PIC0_ICW1		0x0020			
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1


/* keyboard.c */
void inthandler21(int *esp);				/* IRQ1的处理函数 由asm_inthandler21调用 */
void wait_KBC_sendready(void);				/* 清空i8042的输入缓冲区 */
void init_keyboard(void);					/* 初始化键盘控制电路 */
extern struct FIFO8 keyfifo;
#define PORT_KEYDAT		0x0060			/* i8042的数据端口号 */
#define PORT_KEYCMD		0x0064			/* i8042的命令端口号 */	

/* mouse.c */
/* 鼠标缓冲区结构 */
/* 
	buf[]是缓冲区,存放鼠标发送过来的3字节数据
	phase用来标识接收到鼠标数据的第几步
	x, y存放鼠标位移量	bth记录按键信息
 */
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void inthandler2c(int *esp);				/* IRQ12的处理函数 由asm_inthandler2c调用 */
void enable_mouse(struct MOUSE_DEC *mdec);	/* 激活鼠标 */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);	/* 接收鼠标数据 */
extern struct FIFO8 mousefifo;


/* memory.c */
#define MEMMAN_FREES		4090	/* FREEINFO结构的数量 大约是32KB */
#define MEMMAN_ADDR			0x003c0000	/* MEMMAN结构将被存放在该地址处 */
/* 内存可用信息条目 */
/* addr 可用内存块首地址	size 可用内存块大小 */
struct FREEINFO {
	unsigned int addr, size;
};
/* 内存管理结构 */
/* 
	frees 内存可用信息条目的数目
	maxfrees	frees的最大值
	lostsize	释放失败的内存大小的总和
	losts		释放失败的次数
	struct FREEINFO free[MEMMAN_FREES];	内存可用信息条目数组
 */
struct MEMMAN {		
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);		/* 内存检测函数 */
void memman_init(struct MEMMAN *man);				/* 内存管理结构初始化函数 */
unsigned int memman_total(struct MEMMAN *man);		/* 返回空闲内存大小的总和 */
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);	/* 内存分配函数 */
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);	/* 内存释放函数 */
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);	/* 内存分配函数 4K为单位 */
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);	/* 内存释放函数 4K为单位 */

/* sheet.c */
#define MAX_SHEETS		256			/* 图层的最大数 */
/* 图层结构体 */
/*
   用来记录单个图层的信息
   buf	是记录图层上所描画内容的地址
   bxsize, bysize	图层的行数和列数
   vx0, vy0			图层左上角的坐标
   col_inv			图层的透明色色号 (所谓的图层的透明色:以鼠标图层为例,在HariMain函数中分别有以下两条调用语句
									  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);		设置鼠标图层信息
									  init_mouse_cursor8(buf_mouse, 99);		 			初始化鼠标图形 
									  sheet_setbuf的最后一个参数99就是设置了鼠标图层的col_inv成员变量 即透明色
									  而init_mouse_cursor8中的参数99设置了鼠标图形中需显示背景的部分的色号,具体
									  请参考该函数的定义,在描绘鼠标图形的矩形中不是鼠标形状的部分即使用该色号
									  也就是说凡是色号为99的部分都不需要重新描绘)
   height			图层的高度
   flags			记录图层各种状态信息
 */
struct SHEET {
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
};
/* 管理各个图层的结构体 */
/*
   vram		图像缓冲区首地址	
   xsize, ysize		分辨率的x和y	与BOOTINFO中的值相同
   sheets	图层指针数组, 指向sheets0中各个图层
   sheets0	图层数组, 存放各个图层结构
 */
struct SHTCTL {
	unsigned char *vram;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};
/* 图层管理结构初始化函数 */
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
/* 创建新的图层 */
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
/* 图层设置函数 */
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
/* 图层高度设定函数 */
void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height);
/* 显示所有图层的函数 */
void sheet_refresh(struct SHTCTL *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1);
/* 移动图层的函数 */
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0);
/* 释放某一图层的函数 */
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht);
