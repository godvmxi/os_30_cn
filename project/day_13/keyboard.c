/* ==================================================================
	注释：宅
	时间：2013年2月21日
	该文件中定义了与键盘相关的函数
   ================================================================== */

#include "bootpack.h"

struct FIFO32 *keyfifo;
int keydata0;				/* 这是所有键盘数据都需要加上的一个值 书中P246 */

/* 处理来自键盘的中断  由naskfunc.nas中的_asm_inthandler21调用 */
void inthandler21(int *esp)
{
	int data;
	io_out8(PIC0_OCW2, 0x61);	/* 通知PIC，IRQ1已经受理完毕 */
	data = io_in8(PORT_KEYDAT);	/* 从8042的输出缓冲区中读出数据, 若不读出, 则8042不再接收数据 */
	fifo32_put(keyfifo, data + keydata0);	/* 将接收到的数据存入键盘缓冲区队列中 */
	return;
}

#define PORT_KEYSTA				0x0064			/* i8042的状态端口号 */
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
void init_keyboard(struct FIFO32 *fifo, int data0)
{
	keyfifo = fifo;
	keydata0 = data0;

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
