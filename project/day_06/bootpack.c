/* ==================================================================
	注释：宅
	时间：2013年2月18日
   ================================================================== */
#include "bootpack.h"
#include <stdio.h>

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	/* s是输出缓冲区	mcursor存放鼠标图形 */
	char s[40], mcursor[256];
	int mx, my;

	init_gdtidt();		/* 初始化GDT, IDT */
	init_pic();			/* 初始化PIC */
	io_sti();			/* 打开所有可屏蔽中断 */

	init_palette();		/* 初始化调色板 */
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);	/* 绘制"桌面" */
	mx = (binfo->scrnx - 16) / 2; /* 计算鼠标图形在屏幕上的位置 它在整个桌面的中心位置 */
	my = (binfo->scrny - 28 - 16) / 2;	
	init_mouse_cursor8(mcursor, COL8_008484);	
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* 显示鼠标图形 */
	sprintf(s, "(%d, %d)", mx, my);										
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);		/* 输出鼠标图形左上角在屏幕上的坐标 */


	io_out8(PIC0_IMR, 0xf9); /* PIC0(11111001) (打开IRQ1键盘中断和连接从PIC的IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (打开PS2鼠标中断 即IRQ12)*/

	for (;;) {
		io_hlt();
	}
}
