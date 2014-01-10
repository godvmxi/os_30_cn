/* ==================================================================
	注释：宅
	时间：2013年2月23日
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
#define ADR_DISKIMG		0x00100000	/* 软盘的数据保存在该地址处(参考P158的内存分配图或asmhead.nas) */


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
void load_tr(int tr);				/* 设置TR寄存器 */
void asm_inthandler20(void);		/* IRQ0服务程序 */
void asm_inthandler21(void);		/* IRQ1服务程序 */
void asm_inthandler27(void);		/* IRQ7服务程序 */
void asm_inthandler2c(void);		/* IRQ12服务程序 */
unsigned int memtest_sub(unsigned int start, unsigned int end);	/* 内存检测函数 */
void farjmp(int eip, int cs);		/* 远跳转 用于任务切换 */

/* fifo.c */
/* 32bit的队列缓冲区结构体 */
struct FIFO32 {
	int *buf;						/* 缓冲区指针 */
	int p, q, size, free, flags;	/*	p 下一个数据写入位置	q 下一个数据读出位置
										size 缓冲区的大小		free 缓冲区有多少空闲
										flags	记录缓冲区是否溢出
									*/
	struct TASK *task;				/* 当队列缓冲区写入数据时需要唤醒的任务 */
};
/* 缓冲区初始化函数 */
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
/* 缓冲区写入函数 */
int fifo32_put(struct FIFO32 *fifo, int data);
/* 缓冲区读出函数 */
int fifo32_get(struct FIFO32 *fifo);
/* 返回缓冲区状态信息 */
int fifo32_status(struct FIFO32 *fifo);

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
#define AR_TSS32		0x0089			/* 32位TSS (可用) */
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
void init_keyboard(struct FIFO32 *fifo, int data0);	/* 初始化键盘控制电路 */
#define PORT_KEYDAT		0x0060				/* i8042的数据端口号 */
#define PORT_KEYCMD		0x0064				/* i8042的命令端口号 */	

/* mouse.c */
/* 管理鼠标的结构 */
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
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);	/* 激活鼠标 */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);	/* 接收鼠标数据 */

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
   ctl				该图层从属于哪个图层管理结构
 */
struct SHEET {
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct SHTCTL *ctl;
};

/* 管理各个图层的结构体 */
/*
   vram		图像缓冲区首地址	map	..这个不知道应该怎么称呼比较准备 干脆就叫map吧
   xsize, ysize		分辨率的x和y	与BOOTINFO中的值相同
   sheets	图层指针数组, 指向sheets0中各个图层
   sheets0	图层数组, 存放各个图层结构
 */
struct SHTCTL {
	unsigned char *vram, *map;
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
void sheet_updown(struct SHEET *sht, int height);
/* 显示所有图层的函数 */
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
/* 移动图层的函数 */
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
/* 释放某一图层的函数 */
void sheet_free(struct SHEET *sht);


/* timer.c */
#define MAX_TIMER		500			/* 定时器最多可以有500个 */
/* 定时器结构体 */
/* 
   next		指向下一个定时器结构
   timeout	相对于当前时间的预定时刻
   flags	记录定时器状态
   fifo		定时器队列指针
   data		定时器数据
 */
struct TIMER {
	struct TIMER *next;
	unsigned int timeout, flags;
	struct FIFO32 *fifo;
	int data;
};
/* 管理定时器的结构 */
/* 
   count	计数器		next	"下一时刻"(不知道怎么描述..不过我想大家应该是懂得)
   t0		记录下一个即将超时的定时器的地址
   timers0	定时器结构数组
 */
struct TIMERCTL {
	unsigned int count, next;
	struct TIMER *t0;
	struct TIMER timers0[MAX_TIMER];
};

extern struct TIMERCTL timerctl;
/* pit初始化函数 */
void init_pit(void);
/* 创建定时器函数 */
struct TIMER *timer_alloc(void);
/* 释放定时器函数 */
void timer_free(struct TIMER *timer);
/* 定时器初始化函数 */
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
/* 定时器设定函数 */
void timer_settime(struct TIMER *timer, unsigned int timeout);
/* 时钟中断处理函数 */
void inthandler20(int *esp);



/* mtask.c */
#define MAX_TASKS		1000	/* 最大任务数 */
#define TASK_GDT0		3		/* TSS描述符从GDT中下标为3处开始 */
#define MAX_TASKS_LV	100		/* 每个LEVEL最多允许建立100个任务 */
#define MAX_TASKLEVELS	10		/* 最多允许建立10个LEVEL */

/* TSS结构	固定格式 详细资料可百度或参考赵博士那本书的第四章 */
struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

/* 标识一个任务的数据结构 */
/* 
   sel		该任务的段选择子(GDT中的选择子)
   flag		标识该任务的状态
   level	该任务运行在哪个等级上(或者叫"层"会比较合适吧。。)
   priority	该任务的优先级
   fifo		该任务的缓冲区
   tss		TSS数据结构 用于任务切换时保存任务的寄存器及任务的配置信息
 */
struct TASK {
	int sel, flags; 
	int level, priority;
	struct FIFO32 fifo;
	struct TSS32 tss;
};

/* 这个真心不知道应该怎么叫它。。 */
/* 
   好吧，就叫它层结构吧。。。
   running	该层中有多少个任务在运行
   now		该层中正在运行的是哪个任务
   tasks	TASK的指针数组,这个数组和我们以前sheets类似,数组中的元素都是按照顺序排放的
 */
struct TASKLEVEL {
	int running; 
	int now; 
	struct TASK *tasks[MAX_TASKS_LV];
};

/* 管理所有任务的结构体 */
/* 
   now_lv	当前任务运行在哪层上
   lv_change	下次任务切换时,是否需要修改level
   level[]	所有的level都定义在该数组中
   tasks0	注意！系统中的任务虽然分属不同的层，但是任何标识一个任务的结构体都是在这个数组中的
 */
struct TASKCTL {
	int now_lv; 
	char lv_change; 
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};

/* 这是一个非常重要的定时器。。。任务切换就靠它了 */
extern struct TIMER *task_timer;
/* 初始化 */
struct TASK *task_init(struct MEMMAN *memman);
/* 分配一个task结构 */
struct TASK *task_alloc(void);
/* 切换到另一个task标识的任务去运行 */
void task_run(struct TASK *task, int level, int priority);
/* 决定下一个要运行的任务 */
void task_switch(void);
/* 睡眠某个任务 */
void task_sleep(struct TASK *task);
