/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��24��
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
#define ADR_DISKIMG		0x00100000	/* ���̵����ݱ����ڸõ�ַ��(�ο�P158���ڴ����ͼ��asmhead.nas) */


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
void load_tr(int tr);				/* ����TR�Ĵ��� */
void asm_inthandler20(void);		/* IRQ0������� */
void asm_inthandler21(void);		/* IRQ1������� */
void asm_inthandler27(void);		/* IRQ7������� */
void asm_inthandler2c(void);		/* IRQ12������� */
unsigned int memtest_sub(unsigned int start, unsigned int end);	/* �ڴ��⺯�� */
void farjmp(int eip, int cs);		/* Զ��ת */
void farcall(int eip, int cs);		/* Զ���� */
void asm_hrb_api(void);				/* ϵͳ���õ���� */
void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);	/* ��������Ӧ�ó��� */
void asm_end_app(void);				/* ����Ӧ�ó��� */

/* fifo.c */
/* 32bit�Ķ��л������ṹ�� */
struct FIFO32 {
	int *buf;						/* ������ָ�� */
	int p, q, size, free, flags;	/*	p ��һ������д��λ��	q ��һ�����ݶ���λ��
										size �������Ĵ�С		free �������ж��ٿ���
										flags	��¼�������Ƿ����
									*/
	struct TASK *task;				/* �����л�����д������ʱ��Ҫ���ѵ����� */
};
/* ��������ʼ������ */
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
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
void init_keyboard(struct FIFO32 *fifo, int data0);	/* ��ʼ�����̿��Ƶ�· */
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
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);	/* ������� */
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
   task				��ͼ�������ĸ�����				
 */
struct SHEET {
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct SHTCTL *ctl;
	struct TASK *task;
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
#define MAX_TASKS		1000	/* ��������� */
#define TASK_GDT0		3		/* TSS��������GDT���±�Ϊ3����ʼ */
#define MAX_TASKS_LV	100		/* ÿ��LEVEL�����������100������ */
#define MAX_TASKLEVELS	10		/* �����������10��LEVEL */

/* TSS�ṹ	�̶���ʽ ��ϸ���ϿɰٶȻ�ο��Բ�ʿ�Ǳ���ĵ����� */
struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

/* ��ʶһ����������ݽṹ */
/* 
   sel		������Ķ�ѡ����(GDT�е�ѡ����)
   flag		��ʶ�������״̬
   level	�������������ĸ��ȼ���(���߽�"��"��ȽϺ��ʰɡ���)
   priority	����������ȼ�
   fifo		������Ļ�����
   tss		TSS���ݽṹ ���������л�ʱ��������ļĴ����������������Ϣ
 */
struct TASK {
	int sel, flags; 
	int level, priority;
	struct FIFO32 fifo;
	struct TSS32 tss;
};

/* ������Ĳ�֪��Ӧ����ô�������� */
/* 
   �ðɣ��ͽ�����ṹ�ɡ�����
   running	�ò����ж��ٸ�����������
   now		�ò����������е����ĸ�����
   tasks	TASK��ָ������,��������������ǰsheets����,�����е�Ԫ�ض��ǰ���˳���ŷŵ�
 */
struct TASKLEVEL {
	int running; 
	int now; 
	struct TASK *tasks[MAX_TASKS_LV];
};

/* ������������Ľṹ�� */
/* 
   now_lv	��ǰ�����������Ĳ���
   lv_change	�´������л�ʱ,�Ƿ���Ҫ�޸�level
   level[]	���е�level�������ڸ�������
   tasks0	ע�⣡ϵͳ�е�������Ȼ������ͬ�Ĳ㣬�����κα�ʶһ������Ľṹ�嶼������������е�
 */
struct TASKCTL {
	int now_lv; 
	char lv_change; 
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};

/* ����һ���ǳ���Ҫ�Ķ�ʱ�������������л��Ϳ����� */
extern struct TIMER *task_timer;
/* ��ʼ�� */
struct TASK *task_init(struct MEMMAN *memman);
/* ����һ��task�ṹ */
struct TASK *task_alloc(void);
/* �л�����һ��task��ʶ������ȥ���� */
void task_run(struct TASK *task, int level, int priority);
/* ������һ��Ҫ���е����� */
void task_switch(void);
/* ˯��ĳ������ */
void task_sleep(struct TASK *task);

/* window.c */
/* �������ڵĺ��� */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
/* ������� */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
/* ����һ���ı��� */
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
/* �����ı������ϽǵĲ�� */
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);


/* console.c */
/* ����̨�ṹ */
/* 
   sht		����̨ͼ��
   cur_x cur_y	����̨���ı�������
   cur_c	���ƹ����˸������ɫ
   timer	��ʱ���ṹ
 */
struct CONSOLE {
	struct SHEET *sht;
	int cur_x, cur_y, cur_c;
	struct TIMER *timer;
};
/* ����̨���� */
void console_task(struct SHEET *sheet, unsigned int memtotal);
/* �ڿ���̨����������ַ� */
void cons_putchar(struct CONSOLE *cons, int chr, char move);
/* �ڿ���̨�л��� */
void cons_newline(struct CONSOLE *cons);
/* �ڿ���̨������ַ��� */
void cons_putstr0(struct CONSOLE *cons, char *s);
/* �ڿ���̨������ַ�����ǰl���ַ� */
void cons_putstr1(struct CONSOLE *cons, char *s, int l);
/* �ڿ���̨��ִ������ */
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal);
/* �ڿ���̨����ʾ�ڴ���Ϣ */
void cmd_mem(struct CONSOLE *cons, unsigned int memtotal);
/* �������� */
void cmd_cls(struct CONSOLE *cons);
/* ��ʾ�����е��ļ���Ϣ */
void cmd_dir(struct CONSOLE *cons);
/* ��ʾĳ�ļ��е����� */
void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline);
/* ִ��Ӧ�ó��� */
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline);
/* API��ڳ��� */
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
int *inthandler0d(int *esp);
int *inthandler0c(int *esp);


/* file.c */
/* ��Ŀ¼����Ŀ�ṹ */
/* 
   name			�ļ���	
   ext			��չ��
   type			�ļ����� 
   reserve		����
   time			ʱ��
   date			����
   clustno		�غ�	(����˵�غſ�������Ϊ�����ţ�
						������Ϊ������ip10�е���ͷ��������ÿ����ֻ��1������)
   size			�ļ���С
 */
struct FILEINFO {
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
};
/* ��FAT����ѹ���� */
void file_readfat(int *fat, unsigned char *img);
/* ��"����"��ĳ���ļ����ص��ڴ��� */
/* "����"֮���Դ�˫��������Ϊ���̵����������Ѿ�ȫ�����Ƶ��ڴ����� */
/* ��������Ĺ���ֻ�����ǽ�һ���ڴ��е����ݸ��Ƶ���һ���ڴ��� */
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
/* �����������в���ĳ�ļ� */
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);



