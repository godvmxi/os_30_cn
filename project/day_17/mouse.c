/* ==================================================================
	注释：宅
	时间：2013年2月21日
	该文件中定义了与鼠标相关的函数
   ================================================================== */

#include "bootpack.h"

struct FIFO32 *mousefifo;
int mousedata0;				/* 所有鼠标接收的数据都需要加上这个值 */


/* 处理来自PS/2鼠标的中断 由naskfunc.nas中的_asm_inthandler2c调用 */
void inthandler2c(int *esp)
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);	/* 通知PIC1  IRQ12已经受理完毕 */
	io_out8(PIC0_OCW2, 0x62);	/* 通知PIC0  IRQ2已经受理完毕 */
	data = io_in8(PORT_KEYDAT);	/* 从8042的输出缓冲区中读出数据, 若不读出, 则8042不再接收数据 */
	fifo32_put(&mousefifo, data);/* 将接收到的数据存入鼠标缓冲区队列中 */
	return;
}


#define KEYCMD_SENDTO_MOUSE		0xd4	/* 要向i8042写入的命令,下面有解释 */
#define MOUSECMD_ENABLE			0xf4	/* 该命令可以允许鼠标向主机发送数据包 */

/* 激活鼠标 */
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	mousefifo = fifo;
	mousedata0 = data0;

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

