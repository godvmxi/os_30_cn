/* ==================================================================
	注释：宅
	时间：2013年2月24日
	该文件中定义了关于描述符操作的函数
   ================================================================== */

#include "bootpack.h"

/* 初始化GDT, IDT */
void init_gdtidt(void)
{
	/* GDT的位置紧随在IDT之后 */
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) ADR_IDT;
	int i;

	/* GDT初始化 */
	for (i = 0; i <= LIMIT_GDT / 8; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	/* 编号为0的描述符是空描述符 是Intel要求必须这样设置的 */
	set_segmdesc(gdt + 1, 0xffffffff,   0x00000000, AR_DATA32_RW);	/* 内核数据段 
																	32位可读写数据段描述符 段限长4G-1 段基址0  DPL = 0*/
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);	/* 内核代码段 
																	32位可读可执行代码段描述符 段限长512KB  段基址0x280000 DPL = 0*/
	load_gdtr(0xffff, 0x00270000);							/* 加载GDT */

	/* IDT初始化 */
	/* 所有的门都设置为中断门 */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7ff, 0x0026f800);							/* 加载IDT */

	/* IDT初始化 */
	set_gatedesc(idt + 0x0c, (int) asm_inthandler0c, 2 * 8, AR_INTGATE32);	/* 堆栈异常 */
	set_gatedesc(idt + 0x0d, (int) asm_inthandler0d, 2 * 8, AR_INTGATE32);	/* 通用保护异常 */
	set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2 * 8, AR_INTGATE32);	/* 设置INT 0x20中断的门描述符 */
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32);	/* 设置INT 0x21中断的门描述符 */
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);	/* 设置INT 0x27中断的门描述符 */
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);	/* 设置INT 0x2c中断的门描述符 */
	/* 注意0x40的描述符的DPL = 3！*/
	set_gatedesc(idt + 0x40, (int) asm_hrb_api,      2 * 8, AR_INTGATE32 + 0x60);	/* 设置INT 0x40中断的门描述符 */

	return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
