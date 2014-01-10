/* ==================================================================
	注释：宅
	时间：2013年2月19日
	这是内核的第一个C语言文件,由asmhead.nas跳入,并进入HarrMain函数运行
   ================================================================== */

#include "bootpack.h"
#include <stdio.h>


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

/* 声明部分 */
extern struct FIFO8 keyfifo, mousefifo;
void enable_mouse(struct MOUSE_DEC *mdec);
void init_keyboard(void);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	/* s是输出缓冲区	mcursor存放鼠标图形 keybuf是键盘缓冲区 mousebuf是鼠标缓冲区*/
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	struct MOUSE_DEC mdec;

	init_gdtidt();		/* 初始化GDT, IDT */
	init_pic();			/* 初始化PIC */
	io_sti();			/* 打开所有可屏蔽中断 */
	
	fifo8_init(&keyfifo, 32, keybuf);		/* 初始化键盘缓冲区结构体 */
	fifo8_init(&mousefifo, 128, mousebuf);	/* 初始化鼠标缓冲区结构体 */
	io_out8(PIC0_IMR, 0xf9); /* PIC0(11111001) (打开IRQ1键盘中断和连接从PIC的IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (打开PS2鼠标中断 即IRQ12)*/

	init_keyboard();		/* 初始化键盘控制电路 */

	init_palette();		/* 初始化调色板 */
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);	/* 绘制"桌面" */
	mx = (binfo->scrnx - 16) / 2; /* 计算鼠标图形在屏幕上的位置 它在整个桌面的中心位置 */
	my = (binfo->scrny - 28 - 16) / 2;	
	init_mouse_cursor8(mcursor, COL8_008484);	/* 初始化鼠标图形 */
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* 显示鼠标图形 */
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);		/* 输出鼠标图形左上角在屏幕上的坐标 */

	enable_mouse(&mdec);		/* 激活鼠标 */

	for (;;) {
		io_cli();				/* 关闭所有可屏蔽中断 */
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {	/* 如果键盘缓冲区或者鼠标缓冲区中都没有数据 */
			io_stihlt();		/* 开中断并待机 直到下一次中断来临 */
		} else {				/* 键盘或鼠标缓冲区中有数据 */
			if (fifo8_status(&keyfifo) != 0) {		/* 如果键盘缓冲区中有数据 */
				i = fifo8_get(&keyfifo);			/* 读取数据 */
				io_sti();							/* 打开所有可屏蔽中断 */
				sprintf(s, "%02X", i);				/* 将读取的数据以十六进制形式输出 */
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {	/* 如果鼠标缓冲区中有数据 */
				i = fifo8_get(&mousefifo);			/* 读取数据 */
				io_sti();							/* 打开所有可屏蔽中断 */
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
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);	/* 输出信息 */
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* 隐藏鼠标 */
					mx += mdec.x;					/* 更新新的鼠标位置 */
					my += mdec.y;
					if (mx < 0) {	/* 鼠标的位置不能小于0,即不能超出屏幕位置 */
						mx = 0;		
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {	/* 同样是范围控制 */
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 隐藏坐标 */
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 显示新坐标 */
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* 画出新的鼠标 */
				}
			}
		}
	}
}

#define PORT_KEYDAT				0x0060			/* i8042的数据端口号 */
#define PORT_KEYSTA				0x0064			/* i8042的状态端口号 */
#define PORT_KEYCMD				0x0064			/* i8042的命令端口号 */	
												/* 事实上状态端口和命令端口是同一个端口
												   只是作者定义了两个不同的符号常量名 */
#define KEYSTA_SEND_NOTREADY	0x02			/* 这个符号常量用来判断i8042的输入缓冲区是否满 */
#define KEYCMD_WRITE_MODE		0x60			/* 发送给i8042的命令 下面有详细解释 */
#define KBC_MODE				0x47			/* 将被设置为i8042的控制寄存器的值 */

/* 等待键盘控制电路准备完毕 */
/* 
	事实上它不停地从i8042的64h端口读取Status Register的内容
	然后判断Status Register的bit1是否为0  若为0 说明输入缓冲区是空的
	可以接收CPU发来的命令或数据, 若为1 说明输入缓冲区是满的 无法接收
	CPU发来的命令或数据	
 */
void wait_KBC_sendready(void)
{
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

/* 初始化键盘控制电路 */
void init_keyboard(void)
{
	wait_KBC_sendready();		/* 清空i8042的输入缓冲区 */
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);	/* 向i8042写入命令 */
												/* 该命令表示准备写入i8042的控制寄存器 
												   下一个通过60h端口写入的字节将被放入
												   i8042的控制寄存器中
												 */
	wait_KBC_sendready();		/* 清空i8042的输入缓冲区 */
	io_out8(PORT_KEYDAT, KBC_MODE);	/* 将KBC_MODE放入i8042的控制寄存器中 
										开启鼠标、键盘
										允许鼠标中断和键盘中断
									*/
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4	/* 该命令可以允许鼠标向主机发送数据包 */

/* 激活鼠标 */
void enable_mouse(struct MOUSE_DEC *mdec)
{
	wait_KBC_sendready();		/* 清空i8042的输入缓冲区 */
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);	/* 向i8042写入命令 */
												/* 该命令表示将发生0x60端口的参数数据发生给鼠标	*/
	wait_KBC_sendready();		/* 清空i8042的输入缓冲区 */
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);	/* 允许鼠标发数据 */
	mdec->phase = 0; /* 等待0xfa的阶段 */
	return;
}


/* 接收鼠标数据 */
/*
	关于鼠标发送的3个字节的各bit具体含义,请参考群共享中的"PS2鼠标键盘协议-仅含PS2部分.pdf"
 */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		/* 等待鼠标的0xfa阶段 */
		if (dat == 0xfa) {		/* 收到确认信息 */
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* 等待鼠标的第一字节阶段 */
		if ((dat & 0xc8) == 0x08) {		/* 判断位移是否溢出(-255~+255) */
			mdec->buf[0] = dat;	
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		/* 等待鼠标的第二字节阶段 */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* 等待鼠标的第三字节阶段 */
		mdec->buf[2] = dat;
		mdec->phase = 1;				
		mdec->btn = mdec->buf[0] & 0x07;	/* 取BYTE1的低3位 按键状态 */
		mdec->x = mdec->buf[1];				/* 保存x,y的位移 */
		mdec->y = mdec->buf[2];				
		if ((mdec->buf[0] & 0x10) != 0) {	/* 判断x的符号位 */
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {	/* 判断y的符号位 */
			mdec->y |= 0xffffff00;
		}									/* mdec->x和mdec->y中存放的都是补码 */
		/*
			假设x的位移量是-100, -100的补码是1 1001 1100(最高位1在BYTE1中, 低8位是BYTE2)
			初始medc->x中存放的就是1001 1100, 通过((mdec->buf[0] & 0x10) != 0)判断出符号
			位为1，即是负数,则mdec->x = 0xFFFFFF9C,当前的mx是152 = 0x98。那么新的位移量应
			该是152+(-100) = 52.
			在86行有mx += mdec.x; 即0xFFFFFF9C+0x98 = 0x100000034 由于int是4个字节的,所以
			最左边的1会被舍去, 即mx = 0x34 = 52
		 */
		mdec->y = - mdec->y; /* 鼠标的y方向与画面符号相反 */
		return 1;
	}
	return -1; /* 不会到这儿 */
}
/* 
	
 */
