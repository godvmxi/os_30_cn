/* ==================================================================
	注释：宅
	时间：2013年2月24日
	该文件中定义了与控制台相关的函数
   ================================================================== */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>

/* 控制台任务 */
void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int i, fifobuf[128], *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	struct CONSOLE cons;
	char cmdline[30];
	cons.sht = sheet;
	cons.cur_x =  8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	*((int *) 0x0fec) = (int) &cons;

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));

	/* 输出一个">" */
	cons_putchar(&cons, '>', 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {	/* 控制台任务自己的缓冲区中没有数据 */
			task_sleep(task);					/* 睡眠自己 */
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);		/* 读出数据 */
			io_sti();
			if (i <= 1) { 						/* 光标用定时器 */
				if (i != 0) {
					timer_init(timer, &task->fifo, 0);  /* 下面是设定0 */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				} else {
					timer_init(timer, &task->fifo, 1); /* 下面是设定1 */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);			/* 设置半秒 */
			}
			if (i == 2) {	/* 光标ON */
				cons.cur_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* 光标OFF */
				/* 直接用黑色将原来光标的位置消去 */
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				cons.cur_c = -1;
			}
			if (256 <= i && i <= 511) { 	/* 任务A传送过来的键盘的数据 */
				if (i == 8 + 256) {			/* 注意！在任务A中已经将键值转换成字符的ASCII码了 */
					/* 退格键 */
					if (cons.cur_x > 16) {
						/* 用空格把光标消去之后 光标向左移8个像素点 */
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) {	/* 如果是回车 (10是换行符的ASCII码) */
					/* Enter */
					/* 用空格将光标擦除 */
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.cur_x / 8 - 2] = 0;
					cons_newline(&cons);
					cons_runcmd(cmdline, &cons, fat, memtotal);	
					cons_putchar(&cons, '>', 1);
				} else {
					/* 一般字符 */
					if (cons.cur_x < 240) {
						cmdline[cons.cur_x / 8 - 2] = i - 256;	/* 将字符保存起来 */
						cons_putchar(&cons, i - 256, 1);		/* 将字符输出 */
					}
				}
			}
			/* 如果需要显示光标 */
			if (cons.cur_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
			}
			sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
		}
	}
}

/* 在控制台中输出单个字符 */
void cons_putchar(struct CONSOLE *cons, int chr, char move)
{
	char s[2];
	s[0] = chr;
	s[1] = 0;
	if (s[0] == 0x09) {			/* 制表符 */
		for (;;) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 240) {
				cons_newline(cons);
			}
			if (((cons->cur_x - 8) & 0x1f) == 0) {
				break;	/* 被32整除则break */
			}
		}
	} else if (s[0] == 0x0a) {	/* 换行 */
		cons_newline(cons);
	} else if (s[0] == 0x0d) {	/* 回车 */
		/* 忽略 */
	} else {	/* 一般字符 */
		putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		if (move != 0) {
			/* move为0时光标不后移 */
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 240) {
				cons_newline(cons);
			}
		}
	}
	return;
}

/* 控制台中换行 */
void cons_newline(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	if (cons->cur_y < 28 + 112) {	/* 对这几个数字不太明白的仔细观察下控制台的形状 */
		cons->cur_y += 16;  /* 换行 */
	} else {
		/* 滚动 */
		for (y = 28; y < 28 + 112; y++) {
			for (x = 8; x < 8 + 240; x++) {
				/* 将下一行的字符串移到上一行中...我不会描述了。。。。 */
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		/* 最下面一行填充颜色 */
		for (y = 28 + 112; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}
	cons->cur_x = 8;
	return;
}

/* 在控制台中输出字符串 */
void cons_putstr0(struct CONSOLE *cons, char *s)
{
	for (; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return;
}

/* 在控制台中输出字符串的前l个字符 */
void cons_putstr1(struct CONSOLE *cons, char *s, int l)
{
	int i;
	for (i = 0; i < l; i++) {
		cons_putchar(cons, s[i], 1);
	}
	return;
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal)
{
	if (strcmp(cmdline, "mem") == 0) {
		cmd_mem(cons, memtotal);
	} else if (strcmp(cmdline, "cls") == 0) {
		cmd_cls(cons);
	} else if (strcmp(cmdline, "dir") == 0) {
		cmd_dir(cons);
	} else if (strncmp(cmdline, "type ", 5) == 0) {
		cmd_type(cons, fat, cmdline);
	} else if (cmdline[0] != 0) {
		if (cmd_app(cons, fat, cmdline) == 0) {
			/* 如果不是应用程序 */
			cons_putstr0(cons, "Bad command.\n\n");
		}
	}
	return;
}


/* 在控制台中输出内存信息 */
void cmd_mem(struct CONSOLE *cons, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[60];
	sprintf(s, "total   %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	cons_putstr0(cons, s);
	return;
}

/* 控制台清屏 */
void cmd_cls(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	for (y = 28; y < 28 + 128; y++) {
		for (x = 8; x < 8 + 240; x++) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	cons->cur_y = 28;
	return;
}


/* 控制台中输出软盘文件信息 */
void cmd_dir(struct CONSOLE *cons)
{
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < 224; i++) {
		if (finfo[i].name[0] == 0x00) {
			break;
		}
		if (finfo[i].name[0] != 0xe5) {
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j];
				}
				s[ 9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];
				cons_putstr0(cons, s);
			}
		}
	}
	cons_newline(cons);
	return;
}


/* 控制台中输出某文件信息 */
void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = file_search(cmdline + 5, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	char *p;
	if (finfo != 0) {	/* 如果内存分配成功 */
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		cons_putstr1(cons, p, finfo->size);
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		/* 文件没找到 */
		cons_putstr0(cons, "File not found.\n");
	}
	cons_newline(cons);
	return;
}

/* 根据输入的文件名启动应用程序 */
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	char name[18], *p, *q;
	struct TASK *task = task_now();
	int i;

	/* 根据命令行生成文件名 */
	for (i = 0; i < 13; i++) {
		if (cmdline[i] <= ' ') {
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0;  /* 暂且将文件名的后面置为0 */

	/* 寻址文件 */
	finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 && name[i - 1] != '.') {
		/* 由于找不到文件 故在文件后面加上".hrb"后重新查找 */
		name[i    ] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	}

	if (finfo != 0) {
		/* 找到文件的情况 */
	/************************************************************/
		/* 为我们的应用程序申请内存空间 p所指向的这段内存是我们应用程序的代码段 */
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		/* 如果是OSASK能够识别的可执行文件格式 */
		if (finfo->size >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00) {
			segsiz = *((int *) (p + 0x0000));		/* 数据段大小 */
			esp    = *((int *) (p + 0x000c));		/* 应用程序的堆栈段(ring3)的栈指针初始值 */
			datsiz = *((int *) (p + 0x0010));		/* hrb文件中数据部分的大小 */
			dathrb = *((int *) (p + 0x0014));		/* hrb文件中数据部分的偏移地址 */
			q = (char *) memman_alloc_4k(memman, segsiz);	/* 根据数据段的大小来申请内存空间 */
			*((int *) 0xfe8) = (int) q;
			/* 设置段描述符  注意！ 两个段描述符的DPL = 3 */
			set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER + 0x60);
			set_segmdesc(gdt + 1004, segsiz - 1,      (int) q, AR_DATA32_RW + 0x60);
			/* 将hrb文件的数据部分复制到数据段中 */
			for (i = 0; i < datsiz; i++) {
				q[esp + i] = p[dathrb + i];
			}
			/* 启动应用程序 */
			start_app(0x1b, 1003 * 8, esp, 1004 * 8, &(task->tss.esp0));
			/* 释放数据段的内存空间 */
			memman_free_4k(memman, (int) q, segsiz);
		} else {
			cons_putstr0(cons, ".hrb file format error.\n");
		}
		/* 释放代码段的内存空间 */
		memman_free_4k(memman, (int) p, finfo->size);
	/************************************************************/
		cons_newline(cons);
		return 1;
	}
		/* 没有找到文件 */
	return 0;
}


int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	/* 这个函数没什么好注释的 唯一要注意的就是参数都是应用程序寄存器的值 */
	int ds_base = *((int *) 0xfe8);
	struct TASK *task = task_now();
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct SHEET *sht;
	int *reg = &eax + 1;	/* eax偺師偺斣抧 */
		/* 曐懚偺偨傔偺PUSHAD傪嫮堷偵彂偒姺偊傞 */
		/* reg[0] : EDI,   reg[1] : ESI,   reg[2] : EBP,   reg[3] : ESP */
		/* reg[4] : EBX,   reg[5] : EDX,   reg[6] : ECX,   reg[7] : EAX */

	if (edx == 1) {
		cons_putchar(cons, eax & 0xff, 1);
	} else if (edx == 2) {
		cons_putstr0(cons, (char *) ebx + ds_base);
	} else if (edx == 3) {
		cons_putstr1(cons, (char *) ebx + ds_base, ecx);
	} else if (edx == 4) {
		return &(task->tss.esp0);
	} else if (edx == 5) {
		sht = sheet_alloc(shtctl);
		sheet_setbuf(sht, (char *) ebx + ds_base, esi, edi, eax);
		make_window8((char *) ebx + ds_base, esi, edi, (char *) ecx + ds_base, 0);
		sheet_slide(sht, 100, 50);
		sheet_updown(sht, 3);	
		reg[7] = (int) sht;
	} else if (edx == 6) {
		sht = (struct SHEET *) ebx;
		putfonts8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char *) ebp + ds_base);
		sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
	} else if (edx == 7) {
		sht = (struct SHEET *) ebx;
		boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
		sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
	}
	return 0;
}

/* 堆栈异常 与一般保护异常的处理类似 */
int *inthandler0c(int *esp)
{
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	struct TASK *task = task_now();
	char s[30];
	cons_putstr0(cons, "\nINT 0C :\n Stack Exception.\n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0);	
}

/* 一般保护异常处理函数 */
int *inthandler0d(int *esp)
{
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	struct TASK *task = task_now();
	char s[30];
	cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0);	/* 直接返回控制台任务的内核态堆栈指针 */
}
