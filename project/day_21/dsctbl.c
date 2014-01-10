/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��24��
	���ļ��ж����˹��������������ĺ���
   ================================================================== */

#include "bootpack.h"

/* ��ʼ��GDT, IDT */
void init_gdtidt(void)
{
	/* GDT��λ�ý�����IDT֮�� */
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) ADR_IDT;
	int i;

	/* GDT��ʼ�� */
	for (i = 0; i <= LIMIT_GDT / 8; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	/* ���Ϊ0���������ǿ������� ��IntelҪ������������õ� */
	set_segmdesc(gdt + 1, 0xffffffff,   0x00000000, AR_DATA32_RW);	/* �ں����ݶ� 
																	32λ�ɶ�д���ݶ������� ���޳�4G-1 �λ�ַ0  DPL = 0*/
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);	/* �ں˴���� 
																	32λ�ɶ���ִ�д���������� ���޳�512KB  �λ�ַ0x280000 DPL = 0*/
	load_gdtr(0xffff, 0x00270000);							/* ����GDT */

	/* IDT��ʼ�� */
	/* ���е��Ŷ�����Ϊ�ж��� */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7ff, 0x0026f800);							/* ����IDT */

	/* IDT��ʼ�� */
	set_gatedesc(idt + 0x0d, (int) asm_inthandler0d, 2 * 8, AR_INTGATE32);	/* ͨ�ñ����쳣 */
	set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2 * 8, AR_INTGATE32);	/* ����INT 0x20�жϵ��������� */
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32);	/* ����INT 0x21�жϵ��������� */
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);	/* ����INT 0x27�жϵ��������� */
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);	/* ����INT 0x2c�жϵ��������� */
	/* ע��0x40����������DPL = 3��*/
	set_gatedesc(idt + 0x40, (int) asm_hrb_api,      2 * 8, AR_INTGATE32 + 0x60);	/* ����INT 0x40�жϵ��������� */

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