/* ==================================================================
	注释：宅
	时间：2013年2月20日
	该文件定义与内存管理相关的函数及符号常量
   ================================================================== */
#include "bootpack.h"

/* 标志位AC参与控制地址不对齐异常的发生。所谓地址不对齐是指如下情形：
   访问一个奇地址的字，或访问地址不是4的倍数的双字等等。如果AC置1，
   那么当出现地址不对齐情形时，引起地址对齐异常。但在特权级0、1和2运行时，
   忽略AC位的设置，在CR0中的AM位为1时也忽略AC位的设置
 */
/*
 新设的片上超高速缓存控制位CD控制是否允许超高速缓存填充。
 CD=0，允许片上超高速缓存填充。CD=1，禁止片上超高速缓存填充。     
 新设的片上超高速缓存直写方式控制位NW控制是否采用直写方式。
 NW=1，采用直写方式和允许使无效，这是系统复位时的缺省状态。
 NW=0，禁止直写方式及使无效
 */
/*
 这几个标志位都是486新增加的,更详细的解释请参考杨季文先生的
 《80x86汇编语言程序设计教程》的第11章
 */
#define EFLAGS_AC_BIT		0x00040000	/* 用于设置EFLAGS中的AC位 */
#define CR0_CACHE_DISABLE	0x60000000	/* 用于设置和清除CR0的NW与CD位 */


/* 内存检测函数 */
/* 检测从start地址到end地址范围内, 能够使用的内存的末尾地址 */
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	eflg = io_load_eflags();		/* 获取eflags寄存器的值 */
	eflg |= EFLAGS_AC_BIT; 			/* AC位置1 */
	io_store_eflags(eflg);			/* 写回eflags寄存器 */
	eflg = io_load_eflags();		/* 获取eflags寄存器的值 */
	if ((eflg & EFLAGS_AC_BIT) != 0) { /* 如果是386,即使AC=1,AC的值也会自动回到0 */
		flg486 = 1;		/* 这是因为386没有定义AC位,而没有定义的位总是被设置为0的 */
	}
	eflg &= ~EFLAGS_AC_BIT; 		/* AC位清0  事实上AC位对我们没有影响,因为我们现在是 */
	io_store_eflags(eflg);			/* 运行在ring0特权级的，是忽略AC位的 所以这两条指令 */
									/* 有点多余，不过作者可能是考虑到以后运行在ring3的 */
									/* 程序的内存对齐问题才让AC = 0 */
	if (flg486 != 0) {				/* 如果是486 */
		cr0 = load_cr0();			/* 获取CR0寄存器的值 */
		cr0 |= CR0_CACHE_DISABLE; 	/* 设置NW与CD位为1 */
		store_cr0(cr0);				/* 写回CR0寄存器 */
	}

	i = memtest_sub(start, end);	/* 定义在naskfunc.nas中 */

	if (flg486 != 0) {				/* 如果是486 */
		cr0 = load_cr0();			/* 获取CR0寄存器的值 */
		cr0 &= ~CR0_CACHE_DISABLE;  /* NW与CD位清零 */
		store_cr0(cr0);				/* 写回CR0寄存器 */
	}

	return i;
}


/* 内存管理结构初始化函数 */
void memman_init(struct MEMMAN *man)
{
	man->frees = 0;			/* 内存可用信息条目的数目 */
	man->maxfrees = 0;		/* frees的最大值 */
	man->lostsize = 0;		/* 释放失败的内存大小总和 */
	man->losts = 0;			/* 释放失败的次数 */
	return;
}

/* 返回空闲内存大小的总和 */
unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

/* 内存分配函数 */
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* 找到第一个足够大的内存 */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* 如果free[i]变成0 就减掉一条可用信息 */
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1]; /* 代入结构体 */
				}
			}
			return a;
		}
	}
	return 0; /* 没用可用空间 */
}

/* 内存释放函数 */
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	/* 为便于归纳内存  将free[]按照addr的顺序排列 */
	/* 所以 先查找应该放在哪里 */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* 前面有可用内存 */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* 可以与前面的可用内存归纳到一起 即有上邻 */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/* 后面也有 */
				if (addr + size == man->free[i].addr) {
					/* 也可以与后面的可用内存归纳到一起  即有下邻 */
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i]删除 */
					/* free[i]变成0后归纳到前面去 */
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1]; /* 结构体赋值 */
					}
				}
			}
			return 0; 
		}
	}
	/* 不能与前面的可用内存归纳到一起 */
	if (i < man->frees) {
		/* 后面还有 */
		if (addr + size == man->free[i].addr) {
			/* 可以与后面的内容归纳到一起 */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; 
		}
	}
	/* 既不能与前面归纳到一起 也不能与后面归纳到一起 即 没有上邻也没有下邻*/
	if (man->frees < MEMMAN_FREES) {
		/* free[i]之后的 向后移动 腾出一点可用空间 */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees; /* 更新最大值 */
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	/* 不能往后移动 */
	man->losts++;
	man->lostsize += size;
	return -1; /* 失败 */
}

/* 内存分配函数  4K为单位 */
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;		/* 向上以4K取整 */
	a = memman_alloc(man, size);
	return a;
}

/* 内存释放函数	4K为单位 */
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;		/* 向上以4K取整 */
	i = memman_free(man, addr, size);
	return i;
}
