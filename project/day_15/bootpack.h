/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��21��
	��ͷ�ļ����и��ļ��еĺ��������Լ�һЩ���ų����Ķ���
   ================================================================== */

/* asmhead.nas */
struct BOOTINFO { /* 0x0ff0-0x0fff */
	char cyls; 			/* �������ipl10�ж���������� */
	char leds; 			/* ����ż���led�Ƶ�״̬ */
	char vmode; 		/* �Կ�ģʽΪ����λ��ɫ */
	char reserve;		/* ��"ռλ��" ��asmhead.nas�и��ֽ�δʹ�� */
	short scrnx, scrny; /* ����ֱ��� */
	char *vram;			/* ͼ�񻺳����׵�ַ */
};
#define ADR_BOOTINFO	0x00000ff0	/* ���������Ϣ�ĵ�ַ */

/* naskfunc.nas */
void io_hlt(void);					/* hlt */
void io_cli(void);					/* �ر����п������ж� */
void io_sti(void);					/* �����п������ж� */
void io_stihlt(void);				/* �����п������жϲ����� */
int io_in8(int port);				/* ��port�˿ڶ�ȡһ�ֽ����� */
void io_out8(int port, int data);	/* ��dataд�뵽port�˿� */
int io_load_eflags(void);			/* ��ȡeflags�Ĵ�����ֵ */
void io_store_eflags(int eflags);	/* ����eflags�Ĵ�����ֵ */
void load_gdtr(int limit, int addr);/* ����GDT(����˵ ����GDTR�Ĵ���) */
void load_idtr(int limit, int addr);/* ����IDT(����˵ ����IDTR�Ĵ���) */
int load_cr0(void);					/* ��ȡCR0�Ĵ�����ֵ */
void store_cr0(int cr0);			/* ����CR0�Ĵ�����ֵ */
void asm_inthandler21(void);		/* IRQ1������� */
void asm_inthandler27(void);		/* IRQ7������� */
void asm_inthandler2c(void);		/* IRQ12������� */
unsigned int memtest_sub(unsigned int start, unsigned int end);	/* �ڴ��⺯�� */


/* fifo.c */
/* 32bit�Ķ��л������ṹ�� */
struct FIFO32 {
	int *buf;						/* ������ָ�� */
	int p, q, size, free, flags;	/*	p ��һ������д��λ��	q ��һ�����ݶ���λ��
										size �������Ĵ�С		free �������ж��ٿ���
										flags	��¼�������Ƿ����
									*/
};
/* ��������ʼ������ */
void fifo32_init(struct FIFO32 *fifo, int size, int *buf);
/* ������д�뺯�� */
int fifo32_put(struct FIFO32 *fifo, int data);
/* �������������� */
int fifo32_get(struct FIFO32 *fifo);
/* ���ػ�����״̬��Ϣ */
int fifo32_status(struct FIFO32 *fifo);


/* graphic.c */
void init_palette(void);			/* ��ʼ����ɫ�� */
void set_palette(int start, int end, unsigned char *rgb);	/* ���õ�ɫ�� */
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1); /* ����Ļ�ϻ���һ������ͼ�� */
void init_screen8(char *vram, int x, int y);				/* ����"����" */
void putfont8(char *vram, int xsize, int x, int y, char c, char *font); /* ����Ļ����������ַ� */
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s); /* ���ascii���ַ��� */
void init_mouse_cursor8(char *mouse, char bc);				/* ��ʼ�����ͼ�� */
void putblock8_8(char *vram, int vxsize, int pxsize,		/* ��ʾĳһͼ�� */
	int pysize, int px0, int py0, char *buf, int bxsize);
/* ������Ӧ��ɫ�ڵ�ɫ���е��±� */
#define COL8_000000		0		/* ��ɫ �ڵ�ɫ�����±�Ϊ0 */
#define COL8_FF0000		1		/* ����ɫ �ڵ�ɫ�����±�Ϊ1 */
#define COL8_00FF00		2		/* ����ɫ �ڵ�ɫ�����±�Ϊ2 */
#define COL8_FFFF00		3		/* ����ɫ �ڵ�ɫ�����±�Ϊ3 */
#define COL8_0000FF		4		/* ����ɫ �ڵ�ɫ�����±�Ϊ4 */	
#define COL8_FF00FF		5		/* ����ɫ �ڵ�ɫ�����±�Ϊ5 */
#define COL8_00FFFF		6		/* ǳ����ɫ �ڵ�ɫ�����±�Ϊ6 */
#define COL8_FFFFFF		7		/* ��ɫ �ڵ�ɫ�����±�Ϊ7 */
#define COL8_C6C6C6		8		/* ����ɫ �ڵ�ɫ�����±�Ϊ8 */
#define COL8_840000		9		/* ����ɫ �ڵ�ɫ�����±�Ϊ9 */
#define COL8_008400		10		/* ����ɫ �ڵ�ɫ�����±�Ϊ10 */
#define COL8_848400		11		/* ����ɫ �ڵ�ɫ�����±�Ϊ11 */
#define COL8_000084		12		/* ����ɫ �ڵ�ɫ�����±�Ϊ12 */
#define COL8_840084		13		/* ����ɫ �ڵ�ɫ�����±�Ϊ13 */
#define COL8_008484		14		/* ǳ����ɫ �ڵ�ɫ�����±�Ϊ14 */
#define COL8_848484		15		/* ����ɫ �ڵ�ɫ�����±�Ϊ15 */



/* dsctbl.c */
/* ��Ŷ��������Ľṹ�� */
/* ���ڶ������������аٶ� ��Ϊ�漰����ģʽ ����������ݺܶ� �޷�һһע��
   ������Ϊ��ʱû��Ҫ�����ڱ���ģʽ���������,��������Ҳû�д��������ھ�
   ���������ڱ���ģʽ��֪ʶ, �������Ȥ�Ļ����Բο�Intel 80386�ֲ�ľ�3
   �����Բ�ʿ�ġ�Linux �ں���ȫ������������0.11�ںˡ��ĵ����µ��������
 */
/*
	limit_low	���޳���0~15bit		base_low	�λ�ַ��0~15bit
	base_mid	�λ�ַ��16~23bit	access_right(0~3bit TYPE�ֶ�, 4bit S�ֶ� 5~6 DPL�ֶ� 7bit P�ֶ�)
	limit_high	���޳���16~19bit+AVL+D/B+G��	base_high	�λ�ַ��24~31bit
	- - �ð� ������������ڶ���������ע�Ͳ��Ǻ������ ������Ŀ��Բο��������Ӵ���ͼƬ
	http://baike.baidu.com/picview/3289301/3289301/0/0db52faddf823c3d4b36d686.html#albumindex=0&picindex=0
 */
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

/* ������������Ľṹ�� */
/*
	���������������������,ֻ����Щ�ֶ���Щ�����
	offset_low	����ƫ�Ƶ�ַ�ĵ�16bit	selector ��ѡ����
	dw_count	��������, ֻ�ǵ���������Ч
	access_right	0~3bit:TYPE�ֶ�, 4bit:S�ֶ�, 5~6bit:DPL�ֶ�, 7bit:P�ֶ�
	offset_high	����ƫ�Ƶ�ַ�ĸ�16bit
 */
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void init_gdtidt(void);				/* ��ʼ��GDT, IDT */
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);	/* ���ö������� */
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);		/* ������������ */
#define ADR_IDT			0x0026f800		/* IDT�׵�ַ */
#define LIMIT_IDT		0x000007ff		/* IDT�޳� */
#define ADR_GDT			0x00270000		/* GDT�׵�ַ */
#define LIMIT_GDT		0x0000ffff		/* GDT�޳� */
#define ADR_BOTPAK		0x00280000		/* �ں˴����׵�ַ */
#define LIMIT_BOTPAK	0x0007ffff		/* �ں˴����޳�(��512KB) */
#define AR_DATA32_RW	0x4092			/* 32λ�ɶ�д���ݶ�����������ֵ G = 0 DPL = 0 */
#define AR_CODE32_ER	0x409a			/* 32λ�ɶ���ִ�д��������������ֵ G = 0 DPL = 0 */
#define AR_TSS32		0x0089			/* 32λTSS (����) */
#define AR_INTGATE32	0x008e			/* 32λ�ж�������������ֵ DPL = 0 */



/* int.c */
void init_pic(void);					/* ��ʼ��PIC */			
void inthandler27(int *esp);			/* IRQ7�Ĵ������� */
/* PIC�и����˿ں� */
#define PIC0_ICW1		0x0020			
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1


/* keyboard.c */
void inthandler21(int *esp);				/* IRQ1�Ĵ������� ��asm_inthandler21���� */
void wait_KBC_sendready(void);				/* ���i8042�����뻺���� */
void init_keyboard(void);					/* ��ʼ�����̿��Ƶ�· */
#define PORT_KEYDAT		0x0060				/* i8042�����ݶ˿ں� */
#define PORT_KEYCMD		0x0064				/* i8042������˿ں� */	


/* mouse.c */
/* �������Ľṹ */
/* 
	buf[]�ǻ�����,�����귢�͹�����3�ֽ�����
	phase������ʶ���յ�������ݵĵڼ���
	x, y������λ����	bth��¼������Ϣ
 */
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void inthandler2c(int *esp);				/* IRQ12�Ĵ������� ��asm_inthandler2c���� */
void enable_mouse(struct MOUSE_DEC *mdec);	/* ������� */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);	/* ����������� */



/* memory.c */
#define MEMMAN_FREES		4090	/* FREEINFO�ṹ������ ��Լ��32KB */
#define MEMMAN_ADDR			0x003c0000	/* MEMMAN�ṹ��������ڸõ�ַ�� */
/* �ڴ������Ϣ��Ŀ */
/* addr �����ڴ���׵�ַ	size �����ڴ���С */
struct FREEINFO {
	unsigned int addr, size;
};
/* �ڴ�����ṹ */
/* 
	frees �ڴ������Ϣ��Ŀ����Ŀ
	maxfrees	frees�����ֵ
	lostsize	�ͷ�ʧ�ܵ��ڴ��С���ܺ�
	losts		�ͷ�ʧ�ܵĴ���
	struct FREEINFO free[MEMMAN_FREES];	�ڴ������Ϣ��Ŀ����
 */
struct MEMMAN {		
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);		/* �ڴ��⺯�� */
void memman_init(struct MEMMAN *man);				/* �ڴ�����ṹ��ʼ������ */
unsigned int memman_total(struct MEMMAN *man);		/* ���ؿ����ڴ��С���ܺ� */
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);	/* �ڴ���亯�� */
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);	/* �ڴ��ͷź��� */
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);	/* �ڴ���亯�� 4KΪ��λ */
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);	/* �ڴ��ͷź��� 4KΪ��λ */


/* sheet.c */
#define MAX_SHEETS		256			/* ͼ�������� */
/* ͼ��ṹ�� */
/*
   ������¼����ͼ�����Ϣ
   buf	�Ǽ�¼ͼ�������軭���ݵĵ�ַ
   bxsize, bysize	ͼ�������������
   vx0, vy0			ͼ�����Ͻǵ�����
   col_inv			ͼ���͸��ɫɫ�� (��ν��ͼ���͸��ɫ:�����ͼ��Ϊ��,��HariMain�����зֱ������������������
									  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);		�������ͼ����Ϣ
									  init_mouse_cursor8(buf_mouse, 99);		 			��ʼ�����ͼ�� 
									  sheet_setbuf�����һ������99�������������ͼ���col_inv��Ա���� ��͸��ɫ
									  ��init_mouse_cursor8�еĲ���99���������ͼ��������ʾ�����Ĳ��ֵ�ɫ��,����
									  ��ο��ú����Ķ���,��������ͼ�εľ����в��������״�Ĳ��ּ�ʹ�ø�ɫ��
									  Ҳ����˵����ɫ��Ϊ99�Ĳ��ֶ�����Ҫ�������)
   height			ͼ��ĸ߶�
   flags			��¼ͼ�����״̬��Ϣ
   ctl				��ͼ��������ĸ�ͼ������ṹ
 */
struct SHEET {
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct SHTCTL *ctl;
};

/* ��������ͼ��Ľṹ�� */
/*
   vram		ͼ�񻺳����׵�ַ	map	..�����֪��Ӧ����ô�ƺ��Ƚ�׼�� �ɴ�ͽ�map��
   xsize, ysize		�ֱ��ʵ�x��y	��BOOTINFO�е�ֵ��ͬ
   sheets	ͼ��ָ������, ָ��sheets0�и���ͼ��
   sheets0	ͼ������, ��Ÿ���ͼ��ṹ
 */
struct SHTCTL {
	unsigned char *vram, *map;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};

/* ͼ������ṹ��ʼ������ */
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
/* �����µ�ͼ�� */
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
/* ͼ�����ú��� */
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
/* ͼ��߶��趨���� */
void sheet_updown(struct SHEET *sht, int height);
/* ��ʾ����ͼ��ĺ��� */
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
/* �ƶ�ͼ��ĺ��� */
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
/* �ͷ�ĳһͼ��ĺ��� */
void sheet_free(struct SHEET *sht);


/* timer.c */
#define MAX_TIMER		500			/* ��ʱ����������500�� */

/* ��ʱ���ṹ�� */
/* 
   next		ָ����һ����ʱ���ṹ
   timeout	����ڵ�ǰʱ���Ԥ��ʱ��
   flags	��¼��ʱ��״̬
   fifo		��ʱ������ָ��
   data		��ʱ������
 */
struct TIMER {
	struct TIMER *next;
	unsigned int timeout, flags;
	struct FIFO32 *fifo;
	int data;
};

/* ������ʱ���Ľṹ */
/* 
   count	������		next	"��һʱ��"(��֪����ô����..����������Ӧ���Ƕ���)
   t0		��¼��һ��������ʱ�Ķ�ʱ���ĵ�ַ
   timers0	��ʱ���ṹ����
 */
struct TIMERCTL {
	unsigned int count, next;
	struct TIMER *t0;
	struct TIMER timers0[MAX_TIMER];
};

extern struct TIMERCTL timerctl;
/* pit��ʼ������ */
void init_pit(void);
/* ������ʱ������ */
struct TIMER *timer_alloc(void);
/* �ͷŶ�ʱ������ */
void timer_free(struct TIMER *timer);
/* ��ʱ����ʼ������ */
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
/* ��ʱ���趨���� */
void timer_settime(struct TIMER *timer, unsigned int timeout);
/* ʱ���жϴ������� */
void inthandler20(int *esp);

/* mtask.c */
extern struct TIMER *mt_timer;
void mt_init(void);
void mt_taskswitch(void);