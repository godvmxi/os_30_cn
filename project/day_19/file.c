/* ==================================================================
	注释：宅
	时间：2013年2月23日
	这个文件中定义了与文件有关的操作
   ================================================================== */

#include "bootpack.h"

/* fat解压缩 */
void file_readfat(int *fat, unsigned char *img)
{
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) {		
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}

/* 将"软盘"中某个文件加载到内存中 */
/* "软盘"之所以打双引号是因为软盘的内容老早已经全部复制到内存中了 */
/* 这个函数的功能只不过是将一块内存中的数据复制到另一块内存中 */
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
	int i;
	for (;;) {
		if (size <= 512) {	/* 文件本身就小于512B或者一个长文件的最后小于512B的部分 */
			for (i = 0; i < size; i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		for (i = 0; i < 512; i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];	/* 得到下一个簇号 */
	}
	return;
}
