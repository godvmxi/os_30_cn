/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��23��
	���ļ��ж����������̨��صĺ���
   ================================================================== */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>


/* ����̨���� */
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

	/* ���һ��">" */
	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {	/* ����̨�����Լ��Ļ�������û������ */
			task_sleep(task);					/* ˯���Լ� */
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);		/* �������� */
			io_sti();
			if (i <= 1) { 						/* ����ö�ʱ�� */
				if (i != 0) {
					timer_init(timer, &task->fifo, 0); /* �������趨0 */
					if (cursor_c >= 0) {			/* �����Ҫ��ʾ��� */
						cursor_c = COL8_FFFFFF;
					}
				} else {
					timer_init(timer, &task->fifo, 1); /* �������趨1 */
					if (cursor_c >= 0) {			/* �����Ҫ��ʾ��� */
						cursor_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);			/* ���ð��� */
			}
			if (i == 2) {	/* ���ON */
				cursor_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* ���OFF */
				/* ֱ���ú�ɫ��ԭ������λ����ȥ */
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
				cursor_c = -1;
			}
			if (256 <= i && i <= 511) { /* ����A���͹����ļ��̵����� */
				if (i == 8 + 256) {		/* ע�⣡������A���Ѿ�����ֵת�����ַ���ASCII���� */
					/* �˸�� */
					if (cursor_x > 16) {
						/* �ÿո�ѹ����ȥ֮�� ���������8�����ص� */
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;
					}
				} else if (i == 10 + 256) {	/* ����ǻس� (10�ǻ��з���ASCII��) */
					/* Enter */
					/* �ÿո񽫹����� */
					putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					cmdline[cursor_x / 8 - 2] = 0;	/* ����ǰ���м����ַ� ���ַ��ĺ������'\0' */
													/* ��'\0'����ΪC�������ַ�������������β�� */
													/* ��2�Ǽ�ȥ'\0'��λ�û���һ���س���λ�� ��*/
													/* �����'\0'�Ǹպ������һ���ַ����� */
					cursor_y = cons_newline(cursor_y, sheet);	/* ���� */
					/* ִ������ */
					if (strcmp(cmdline, "mem") == 0) {
						/* mem���� */
						/* ����ڴ��ܴ�С�Ϳ����ڴ��С */
						sprintf(s, "total   %dMB", memtotal / (1024 * 1024));
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);	/* ���� */
						sprintf(s, "free %dKB", memman_total(memman) / 1024);
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);	/* ���� */
						cursor_y = cons_newline(cursor_y, sheet);	/* ���� */
					} else if (strcmp(cmdline, "cls") == 0) {
						/* cls���� */
						/* ��򵥡���ÿ�����ض���Ϊ��ɫ..... */
						for (y = 28; y < 28 + 128; y++) {
							for (x = 8; x < 8 + 240; x++) {
								sheet->buf[x + y * sheet->bxsize] = COL8_000000;
							}
						}
						sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
						cursor_y = 28;
					} else if (strcmp(cmdline, "dir") == 0) {
						/* dir���� */
						for (x = 0; x < 224; x++) {	/* �������еĸ�Ŀ¼��Ŀ */
													/* ���224����ipl10�ж���� */
							if (finfo[x].name[0] == 0x00) {	/* ����Ŀ�������κ��ļ���Ϣ */
								break;
							}
							if (finfo[x].name[0] != 0xe5) {	/* ����Ŀ�����ļ���Ϣ */
								if ((finfo[x].type & 0x18) == 0) {	/* ����Ŀ��¼�Ĳ���Ŀ¼ Ҳ���Ƿ��ļ���Ϣ */
									sprintf(s, "filename.ext   %7d", finfo[x].size);
									for (y = 0; y < 8; y++) {	/* �����ļ��� */
										s[y] = finfo[x].name[y];
									}
									s[ 9] = finfo[x].ext[0];	/* ������չ�� */
									s[10] = finfo[x].ext[1];
									s[11] = finfo[x].ext[2];
									putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
									cursor_y = cons_newline(cursor_y, sheet);	/* ���� */
								}
							}
						}
						cursor_y = cons_newline(cursor_y, sheet);	/* ���� */
					} else if (strncmp(cmdline, "type ", 5) == 0) {
						/* type���� */
						/* ׼���ļ��� */
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
									/* ��Сд��ĸת���ɴ�д��ĸ */
									s[y] -= 0x20;
								} 
								y++;
							}
						}
						/* Ѱ���ļ� */
						for (x = 0; x < 224; ) {
							if (finfo[x].name[0] == 0x00) {	/* ����Ŀ�������κ��ļ���Ϣ */
								break;
							}
							if ((finfo[x].type & 0x18) == 0) {	/* ����Ŀ��¼�Ĳ���Ŀ¼ Ҳ���Ƿ��ļ���Ϣ */
								for (y = 0; y < 11; y++) {
									if (finfo[x].name[y] != s[y]) {
										goto type_next_file;
									}
								}
								break; /* �ҵ��ļ����˳�ѭ�� */
							}
		type_next_file:
							x++;
						}
						if (x < 224 && finfo[x].name[0] != 0x00) {
							/* �ҵ��ļ������ */
							/* �����ڴ� ����ڴ����ڴ���ļ������� */
							p = (char *) memman_alloc_4k(memman, finfo[x].size);
							/* �������и��ļ������ݸ��Ƶ�pָ����ڴ��� */
							file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
							cursor_x = 8;	
							for (y = 0; y < finfo[x].size; y++) {
								/* �������ļ��е����� */
								s[0] = p[y];
								s[1] = 0;
								if (s[0] == 0x09) {	/* �Ʊ��� */
									for (;;) {
										/* ��һС�ξͲ�ע���ˣ�͵���� ���н��͵ĺ���ϸ */
										putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
										cursor_x += 8;
										if (cursor_x == 8 + 240) {
											cursor_x = 8;
											cursor_y = cons_newline(cursor_y, sheet);
										}
										if (((cursor_x - 8) & 0x1f) == 0) {
											break;	/* ��32������break */
										}
									}
								} else if (s[0] == 0x0a) {	/* ���� */
									cursor_x = 8;
									cursor_y = cons_newline(cursor_y, sheet);
								} else if (s[0] == 0x0d) {	/* �س� */
									/* ���� */
								} else {	/* һ���ַ� */
									putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
									cursor_x += 8;
									if (cursor_x == 8 + 240) {
										cursor_x = 8;
										cursor_y = cons_newline(cursor_y, sheet);
									}
								}
							}
							/* ������ ����������ڴ�ռ��ͷŵ� */
							memman_free_4k(memman, (int) p, finfo[x].size);
						} else {		/* �ļ�û���ҵ� */
							putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
							cursor_y = cons_newline(cursor_y, sheet);
						}
						cursor_y = cons_newline(cursor_y, sheet);
					} else if (strcmp(cmdline, "hlt") == 0) {	/* ����Ӧ�ó��� */
						for (y = 0; y < 11; y++) {
							s[y] = ' ';
						}
						/* �����ļ��� */
						s[0] = 'H';
						s[1] = 'L';
						s[2] = 'T';
						s[8] = 'H';
						s[9] = 'R';
						s[10] = 'B';
						/* �����ļ� */
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
								break; /* �ҵ��ļ� */
							}
		hlt_next_file:
							x++;
						}
						if (x < 224 && finfo[x].name[0] != 0x00) {
							/* �ҵ��ļ������ */
							/* ����һ���ڴ� �ļ��������ص������ִ�� */
							p = (char *) memman_alloc_4k(memman, finfo[x].size);
							/* ���ļ������ݸ��Ƶ�pָ��Ļ����� */
							file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
							/* Ϊ���ǵ�Ӧ�ó��򴴽�һ�������� */
							set_segmdesc(gdt + 1003, finfo[x].size - 1, (int) p, AR_CODE32_ER);
							/* ��ת��Ӧ�ó���ִ�� */
							farjmp(0, 1003 * 8);
							/* �ͷ��ڴ� */
							memman_free_4k(memman, (int) p, finfo[x].size);
						} else {
							/* �ļ�û���ҵ� */
							putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
							cursor_y = cons_newline(cursor_y, sheet);
						}
						cursor_y = cons_newline(cursor_y, sheet);
					} else if (cmdline[0] != 0) {
						/* ����̨�����뵫������������� */
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);
					}
					/* ���һ��'>' */
					putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 16;
				} else {
					/* һ���ַ� */
					if (cursor_x < 240) {
						/* ��ʾһ���ַ�����������8�����ص� */
						s[0] = i - 256;
						s[1] = 0;
						cmdline[cursor_x / 8 - 2] = i - 256;
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
			}
			/* �����Ҫ��ʾ��� */
			if (cursor_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
			}
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}

/* ����̨�л��� */
int cons_newline(int cursor_y, struct SHEET *sheet)
{
	int x, y;
	if (cursor_y < 28 + 112) {		/* ���⼸�����ֲ�̫���׵���ϸ�۲��¿���̨����״ */
		cursor_y += 16; /* ���� */
	} else {
		/* ���� */
		for (y = 28; y < 28 + 112; y++) {
			for (x = 8; x < 8 + 240; x++) {
				/* ����һ�е��ַ����Ƶ���һ����...�Ҳ��������ˡ������� */
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		/* ������һ�������ɫ */
		for (y = 28 + 112; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}
	return cursor_y;
}