/* ==================================================================
	注释：宅
	时间：2013年2月23日
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
	int i, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_c = -1;
	char s[30], cmdline[30], *p;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int x, y;
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));

	/* 输出一个">" */
	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

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
					timer_init(timer, &task->fifo, 0); /* 下面是设定0 */
					if (cursor_c >= 0) {			/* 如果需要显示光标 */
						cursor_c = COL8_FFFFFF;
					}
				} else {
					timer_init(timer, &task->fifo, 1); /* 下面是设定1 */
					if (cursor_c >= 0) {			/* 如果需要显示光标 */
						cursor_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);			/* 设置半秒 */
			}
			if (i == 2) {	/* 光标ON */
				cursor_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* 光标OFF */
				/* 直接用黑色将原来光标的位置消去 */
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
				cursor_c = -1;
			}
			if (256 <= i && i <= 511) { /* 任务A传送过来的键盘的数据 */
				if (i == 8 + 256) {		/* 注意！在任务A中已经将键值转换成字符的ASCII码了 */
					/* 退格键 */
					if (cursor_x > 16) {
						/* 用空格把光标消去之后 光标向左移8个像素点 */
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;
					}
				} else if (i == 10 + 256) {	/* 如果是回车 (10是换行符的ASCII码) */
					/* Enter */
					/* 用空格将光标擦除 */
					putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					cmdline[cursor_x / 8 - 2] = 0;	/* 计算前面有几个字符 在字符的后面加上'\0' */
													/* 加'\0'是因为C语言中字符串都是以它结尾的 */
													/* 减2是减去'\0'的位置还有一个回车的位置 所*/
													/* 以最后'\0'是刚好在最后一个字符后面 */
					cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
					/* 执行命令 */
					if (strcmp(cmdline, "mem") == 0) {
						/* mem命令 */
						/* 输出内存总大小和空闲内存大小 */
						sprintf(s, "total   %dMB", memtotal / (1024 * 1024));
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
						sprintf(s, "free %dKB", memman_total(memman) / 1024);
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
						cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
					} else if (strcmp(cmdline, "cls") == 0) {
						/* cls命令 */
						/* 最简单。。每个像素都设为黑色..... */
						for (y = 28; y < 28 + 128; y++) {
							for (x = 8; x < 8 + 240; x++) {
								sheet->buf[x + y * sheet->bxsize] = COL8_000000;
							}
						}
						sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
						cursor_y = 28;
					} else if (strcmp(cmdline, "dir") == 0) {
						/* dir命令 */
						for (x = 0; x < 224; x++) {	/* 遍历所有的根目录条目 */
													/* 这个224是在ipl10中定义的 */
							if (finfo[x].name[0] == 0x00) {	/* 该条目不包含任何文件信息 */
								break;
							}
							if (finfo[x].name[0] != 0xe5) {	/* 该条目包含文件信息 */
								if ((finfo[x].type & 0x18) == 0) {	/* 该条目记录的不是目录 也不是非文件信息 */
									sprintf(s, "filename.ext   %7d", finfo[x].size);
									for (y = 0; y < 8; y++) {	/* 设置文件名 */
										s[y] = finfo[x].name[y];
									}
									s[ 9] = finfo[x].ext[0];	/* 设置扩展名 */
									s[10] = finfo[x].ext[1];
									s[11] = finfo[x].ext[2];
									putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
									cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
								}
							}
						}
						cursor_y = cons_newline(cursor_y, sheet);	/* 换行 */
					} else if (strncmp(cmdline, "type ", 5) == 0) {
						/* type命令 */
						/* 准备文件名 */
						for (y = 0; y < 11; y++) {
							s[y] = ' ';
						}
						y = 0;
						for (x = 5; y < 11 && cmdline[x] != 0; x++) {
							if (cmdline[x] == '.' && y <= 8) {
								y = 8;
							} else {
								s[y] = cmdline[x];
								if ('a' <= s[y] && s[y] <= 'z') {
									/* 将小写字母转换成大写字母 */
									s[y] -= 0x20;
								} 
								y++;
							}
						}
						/* 寻找文件 */
						for (x = 0; x < 224; ) {
							if (finfo[x].name[0] == 0x00) {	/* 该条目不包含任何文件信息 */
								break;
							}
							if ((finfo[x].type & 0x18) == 0) {	/* 该条目记录的不是目录 也不是非文件信息 */
								for (y = 0; y < 11; y++) {
									if (finfo[x].name[y] != s[y]) {
										goto type_next_file;
									}
								}
								break; /* 找到文件就退出循环 */
							}
		type_next_file:
							x++;
						}
						if (x < 224 && finfo[x].name[0] != 0x00) {
							/* 找到文件的情况 */
							/* 分配内存 这块内存用于存放文件的内容 */
							p = (char *) memman_alloc_4k(memman, finfo[x].size);
							/* 将软盘中该文件的内容复制到p指向的内存中 */
							file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
							cursor_x = 8;	
							for (y = 0; y < finfo[x].size; y++) {
								/* 逐个输出文件中的内容 */
								s[0] = p[y];
								s[1] = 0;
								if (s[0] == 0x09) {	/* 制表符 */
									for (;;) {
										/* 这一小段就不注释了，偷下懒 书中解释的很详细 */
										putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
										cursor_x += 8;
										if (cursor_x == 8 + 240) {
											cursor_x = 8;
											cursor_y = cons_newline(cursor_y, sheet);
										}
										if (((cursor_x - 8) & 0x1f) == 0) {
											break;	/* 被32整除则break */
										}
									}
								} else if (s[0] == 0x0a) {	/* 换行 */
									cursor_x = 8;
									cursor_y = cons_newline(cursor_y, sheet);
								} else if (s[0] == 0x0d) {	/* 回车 */
									/* 忽略 */
								} else {	/* 一般字符 */
									putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
									cursor_x += 8;
									if (cursor_x == 8 + 240) {
										cursor_x = 8;
										cursor_y = cons_newline(cursor_y, sheet);
									}
								}
							}
							/* 输出完毕 将刚申请的内存空间释放掉 */
							memman_free_4k(memman, (int) p, finfo[x].size);
						} else {		/* 文件没有找到 */
							putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
							cursor_y = cons_newline(cursor_y, sheet);
						}
						cursor_y = cons_newline(cursor_y, sheet);
					} else if (strcmp(cmdline, "hlt") == 0) {	/* 调用应用程序 */
						for (y = 0; y < 11; y++) {
							s[y] = ' ';
						}
						/* 设置文件名 */
						s[0] = 'H';
						s[1] = 'L';
						s[2] = 'T';
						s[8] = 'H';
						s[9] = 'R';
						s[10] = 'B';
						/* 查找文件 */
						for (x = 0; x < 224; ) {
							if (finfo[x].name[0] == 0x00) {
								break;
							}
							if ((finfo[x].type & 0x18) == 0) {
								for (y = 0; y < 11; y++) {
									if (finfo[x].name[y] != s[y]) {
										goto hlt_next_file;
									}
								}
								break; /* 找到文件 */
							}
		hlt_next_file:
							x++;
						}
						if (x < 224 && finfo[x].name[0] != 0x00) {
							/* 找到文件的情况 */
							/* 分配一块内存 文件将被加载到这个并执行 */
							p = (char *) memman_alloc_4k(memman, finfo[x].size);
							/* 将文件的内容复制到p指向的缓冲区 */
							file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
							/* 为我们的应用程序创建一个描述符 */
							set_segmdesc(gdt + 1003, finfo[x].size - 1, (int) p, AR_CODE32_ER);
							/* 跳转到应用程序执行 */
							farjmp(0, 1003 * 8);
							/* 释放内存 */
							memman_free_4k(memman, (int) p, finfo[x].size);
						} else {
							/* 文件没有找到 */
							putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
							cursor_y = cons_newline(cursor_y, sheet);
						}
						cursor_y = cons_newline(cursor_y, sheet);
					} else if (cmdline[0] != 0) {
						/* 控制台有输入但不是上面的命令 */
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);
					}
					/* 输出一个'>' */
					putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 16;
				} else {
					/* 一般字符 */
					if (cursor_x < 240) {
						/* 显示一个字符后光标向右移8个像素点 */
						s[0] = i - 256;
						s[1] = 0;
						cmdline[cursor_x / 8 - 2] = i - 256;
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
			}
			/* 如果需要显示光标 */
			if (cursor_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
			}
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}

/* 控制台中换行 */
int cons_newline(int cursor_y, struct SHEET *sheet)
{
	int x, y;
	if (cursor_y < 28 + 112) {		/* 对这几个数字不太明白的仔细观察下控制台的形状 */
		cursor_y += 16; /* 换行 */
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
	return cursor_y;
}
