/* ==================================================================
	注释：宅
	时间：2013年2月24日
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

/* 在软盘数据中查找某文件 */
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max)
{
	int i, j;
	char s[12];
	for (j = 0; j < 11; j++) {
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) { return 0; 		/* 没有找到 */ }
		if (name[i] == '.' && j <= 8) {	
			j = 8;						
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				/* 将小写字母转换成大写字母 */
				s[j] -= 0x20;
			} 
			j++;
		}
	}

	for (i = 0; i < max; ) {
		if (finfo[i].name[0] == 0x00) {		/* 该条目不包含任何文件信息 */
			break;
		}
		if ((finfo[i].type & 0x18) == 0) {	/* 该条目记录的不是目录 也不是非文件信息 */
			for (j = 0; j < 11; j++) {
				if (finfo[i].name[j] != s[j]) {
					goto next;				/* 比较下一个文件 */	
				}
			}
			return finfo + i; /* 找到文件 */
		}
next:
		i++;
	}
	return 0; /* 未找到文件 */
}
