/* ==================================================================
	注释：宅
	时间：2013年2月18日
   ================================================================== */
   
#include "bootpack.h"
#include <stdio.h>

/* 声明部分 */
extern struct FIFO8 keyfifo, mousefifo;
void enable_mouse(void);
void init_keyboard(void);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	/* s是输出缓冲区	mcursor存放鼠标图形 keybuf是键盘缓冲区 mousebuf是鼠标缓冲区*/
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;

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
	init_mouse_cursor8(mcursor, COL8_008484);	
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* 显示鼠标图形 */
	sprintf(s, "(%d, %d)", mx, my);										
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);		/* 输出鼠标图形左上角在屏幕上的坐标 */


	enable_mouse();		/* 激活鼠标 */

	for (;;) {
		io_cli();		/* 关闭所有可屏蔽中断 */
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {	/* 如果键盘缓冲区或者鼠标缓冲区中都没有数据 */
			io_stihlt();	/* 开中断并待机 直到下一次中断来临 */
		} else {		/* 键盘或鼠标缓冲区中有数据 */
			if (fifo8_status(&keyfifo) != 0) {		/* 如果键盘缓冲区中有数据 */
				i = fifo8_get(&keyfifo);			/* 读取数据 */
				io_sti();							/* 打开所有可屏蔽中断 */
				sprintf(s, "%02X", i);				/* 将读取的数据以十六进制形式输出 */
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {	/* 如果鼠标缓冲区中有数据 */
				i = fifo8_get(&mousefifo);				/* 读取数据 */
				io_sti();								/* 打开所有可屏蔽中断 */
				sprintf(s, "%02X", i);					/* 将读取的数据以十六进制形式输出 */
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 47, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
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
void enable_mouse(void)
{
	wait_KBC_sendready();		/* 清空i8042的输入缓冲区 */
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);	/* 向i8042写入命令 */
												/* 该命令表示将发生0x60端口的参数数据发生给鼠标	*/
	wait_KBC_sendready();		/* 清空i8042的输入缓冲区 */
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);	/* 允许鼠标发数据 */
	return; 	
}
