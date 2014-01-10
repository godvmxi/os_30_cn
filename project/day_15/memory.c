/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��20��
	���ļ��������ڴ������صĺ��������ų���
   ================================================================== */
#include "bootpack.h"

/* ��־λAC������Ƶ�ַ�������쳣�ķ�������ν��ַ��������ָ�������Σ�
   ����һ�����ַ���֣�����ʵ�ַ����4�ı�����˫�ֵȵȡ����AC��1��
   ��ô�����ֵ�ַ����������ʱ�������ַ�����쳣��������Ȩ��0��1��2����ʱ��
   ����ACλ�����ã���CR0�е�AMλΪ1ʱҲ����ACλ������
 */
/*
 �����Ƭ�ϳ����ٻ������λCD�����Ƿ����������ٻ�����䡣
 CD=0������Ƭ�ϳ����ٻ�����䡣CD=1����ֹƬ�ϳ����ٻ�����䡣     
 �����Ƭ�ϳ����ٻ���ֱд��ʽ����λNW�����Ƿ����ֱд��ʽ��
 NW=1������ֱд��ʽ������ʹ��Ч������ϵͳ��λʱ��ȱʡ״̬��
 NW=0����ֱֹд��ʽ��ʹ��Ч
 */
/*
 �⼸����־λ����486�����ӵ�,����ϸ�Ľ�����ο����������
 ��80x86������Գ�����ƽ̡̳��ĵ�11��
 */
#define EFLAGS_AC_BIT		0x00040000	/* ��������EFLAGS�е�ACλ */
#define CR0_CACHE_DISABLE	0x60000000	/* �������ú����CR0��NW��CDλ */


/* �ڴ��⺯�� */
/* ����start��ַ��end��ַ��Χ��, �ܹ�ʹ�õ��ڴ��ĩβ��ַ */
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	eflg = io_load_eflags();		/* ��ȡeflags�Ĵ�����ֵ */
	eflg |= EFLAGS_AC_BIT; 			/* ACλ��1 */
	io_store_eflags(eflg);			/* д��eflags�Ĵ��� */
	eflg = io_load_eflags();		/* ��ȡeflags�Ĵ�����ֵ */
	if ((eflg & EFLAGS_AC_BIT) != 0) { /* �����386,��ʹAC=1,AC��ֵҲ���Զ��ص�0 */
		flg486 = 1;		/* ������Ϊ386û�ж���ACλ,��û�ж����λ���Ǳ�����Ϊ0�� */
	}
	eflg &= ~EFLAGS_AC_BIT; 		/* ACλ��0  ��ʵ��ACλ������û��Ӱ��,��Ϊ���������� */
	io_store_eflags(eflg);			/* ������ring0��Ȩ���ģ��Ǻ���ACλ�� ����������ָ�� */
									/* �е���࣬�������߿����ǿ��ǵ��Ժ�������ring3�� */
									/* ������ڴ�����������AC = 0 */
	if (flg486 != 0) {				/* �����486 */
		cr0 = load_cr0();			/* ��ȡCR0�Ĵ�����ֵ */
		cr0 |= CR0_CACHE_DISABLE; 	/* ����NW��CDλΪ1 */
		store_cr0(cr0);				/* д��CR0�Ĵ��� */
	}

	i = memtest_sub(start, end);	/* ������naskfunc.nas�� */

	if (flg486 != 0) {				/* �����486 */
		cr0 = load_cr0();			/* ��ȡCR0�Ĵ�����ֵ */
		cr0 &= ~CR0_CACHE_DISABLE;  /* NW��CDλ���� */
		store_cr0(cr0);				/* д��CR0�Ĵ��� */
	}

	return i;
}


/* �ڴ�����ṹ��ʼ������ */
void memman_init(struct MEMMAN *man)
{
	man->frees = 0;			/* �ڴ������Ϣ��Ŀ����Ŀ */
	man->maxfrees = 0;		/* frees�����ֵ */
	man->lostsize = 0;		/* �ͷ�ʧ�ܵ��ڴ��С�ܺ� */
	man->losts = 0;			/* �ͷ�ʧ�ܵĴ��� */
	return;
}

/* ���ؿ����ڴ��С���ܺ� */
unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

/* �ڴ���亯�� */
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* �ҵ���һ���㹻����ڴ� */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* ���free[i]���0 �ͼ���һ��������Ϣ */
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1]; /* ����ṹ�� */
				}
			}
			return a;
		}
	}
	return 0; /* û�ÿ��ÿռ� */
}

/* �ڴ��ͷź��� */
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	/* Ϊ���ڹ����ڴ�  ��free[]����addr��˳������ */
	/* ���� �Ȳ���Ӧ�÷������� */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* ǰ���п����ڴ� */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* ������ǰ��Ŀ����ڴ���ɵ�һ�� �������� */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/* ����Ҳ�� */
				if (addr + size == man->free[i].addr) {
					/* Ҳ���������Ŀ����ڴ���ɵ�һ��  �������� */
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i]ɾ�� */
					/* free[i]���0����ɵ�ǰ��ȥ */
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1]; /* �ṹ�帳ֵ */
					}
				}
			}
			return 0; 
		}
	}
	/* ������ǰ��Ŀ����ڴ���ɵ�һ�� */
	if (i < man->frees) {
		/* ���滹�� */
		if (addr + size == man->free[i].addr) {
			/* �������������ݹ��ɵ�һ�� */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; 
		}
	}
	/* �Ȳ�����ǰ����ɵ�һ�� Ҳ�����������ɵ�һ�� �� û������Ҳû������*/
	if (man->frees < MEMMAN_FREES) {
		/* free[i]֮��� ����ƶ� �ڳ�һ����ÿռ� */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees; /* �������ֵ */
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	/* ���������ƶ� */
	man->losts++;
	man->lostsize += size;
	return -1; /* ʧ�� */
}

/* �ڴ���亯��  4KΪ��λ */
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;		/* ������4Kȡ�� */
	a = memman_alloc(man, size);
	return a;
}

/* �ڴ��ͷź���	4KΪ��λ */
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;		/* ������4Kȡ�� */
	i = memman_free(man, addr, size);
	return i;
}