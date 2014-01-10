/* ==================================================================
	ע�ͣ�լ
	ʱ�䣺2013��2��18��
   ================================================================== */
   
/* ������������ */
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

/* ���ų������� */
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
/* ���ڵ�ɫ�� */
/*
copy from  http://blog.sina.com.cn/s/blog_500bd63c01019y1s.html
	��ɫ��ֻ��ͼƬ����ɫС�ڵ���256ɫ��ʱ�����,16λ�߲ʺ�24λ32λ�����û�е�ɫ���.                                                                              
	��ɫ��Ĵ��ڵ�����ֻ���ڵ���486��ǰΪ�˽�ʡ�ռ��һ�ֲ���������ѹ���㷨,����û�������ֶ�����  
	��ɫ����Ϊ�˽�Լ�ռ����õģ��൱��һ��������ֻ��16λ���µĲ��õ�ɫ�壬���ɫ���õ�ɫ�塣  
	��������������������ӡ�  
	��һ��������Ϊ200�����أ���ɫ��Ϊ16ɫ�Ĳ�ɫͼ��ÿһ�����ض���R��G��B����������ʾ����Ϊÿ��������256������
	Ҫ��8λ(bit)����һ���ֽ�(byte)����ʾ������ÿ��������Ҫ��3���ֽڡ�����ͼ��Ҫ��200��200��3��Լ120k�ֽڣ��ɲ���
	һ��С��Ŀѽ���������������ķ���������ʡ�Ķࡣ ��Ϊ��һ��16ɫͼ��Ҳ����˵���ͼ�����ֻ��16����ɫ�����ǿ�
	����һ���������е�ÿһ�м�¼һ����ɫ��R��G��Bֵ�����������Ǳ�ʾһ�����ص���ɫʱ��ֻ��Ҫָ������ɫ���ڵڼ��У�
	������ɫ�ڱ��е�����ֵ���ٸ����ӣ�������ĵ�0��Ϊ255��0��0(��ɫ)����ô��ĳ������Ϊ��ɫʱ��ֻ��Ҫ����0���ɡ� 
	��������������һ�£�16��״̬������4λ(bit)��ʾ������һ������Ҫ�ð���ֽڡ�����ͼ��Ҫ��200��200��0.5��Լ20k�ֽڣ�
	�ټ��ϱ�ռ�õ��ֽ�Ϊ3��16=48�ֽ�.����ռ�õ��ֽ���ԼΪǰ���1/6��ʡ�ܶ�ɣ�����R��G��B�ı����������ǳ�˵�ĵ�ɫ��(Palette)��
	��һ�ֽз�����ɫ���ұ�LUT(Look Up Table)���ƺ���ȷ��һЩ��Windowsλͼ�б��õ��˵�ɫ�弼������ʵ������Windowsλͼ��
	����ͼ���ļ���ʽ��pcx��tif��gif�ȶ��õ��ˡ����Ժܺõ����յ�ɫ��ĸ�����ʮ�����õġ� 
	��һ��ͼ��������ɫ���ߴ�256��256��256�֣�Ҳ����˵�������������ᵽ��R��G��B��ɫ��ʾ���������е���ɫ������ͼ�������ɫͼ(true color)��
	���ɫͼ������˵һ��ͼ���������е���ɫ������˵��������ʾ������ɫ���������������԰������е���ɫ����ʾ���ɫͼʱ��
	ÿ������ֱ����R��G��B���������ֽڱ�ʾ���������õ�ɫ�弼����ԭ������ԣ�����õ�ɫ�壬��ʾһ������ҲҪ��24λ��
	������Ϊÿ����ɫ������Ҫ��24λ(��Ϊ�ܹ���224����ɫ������ɫ����224��)����ֱ����R��G��B����������ʾ�õ��ֽ���һ����
	����û���κα��ˣ���Ҫ����һ��256��256��256��3���ֽڵĴ��ɫ�塣�������ɫͼֱ����R��G��B����������ʾ�����ֽ���24λɫͼ��
*/

/* ������ */
void HariMain(void)
{
	char *vram;
	int xsize, ysize;

	init_palette();						/* ��ʼ����ɫ�� */
	vram = (char *) 0xa0000;			
	xsize = 320;						/* ÿ��320�� */
	ysize = 200;						/* ÿ��200�� */

	boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);

	for (;;) {
		io_hlt();
	}
}

void init_palette(void)
{
	/* static�����ı�����"��̬"��  �ڳ������֮ǰһֱ"����" */
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:�� */
		0xff, 0x00, 0x00,	/*  1:���� */
		0x00, 0xff, 0x00,	/*  2:���� */
		0xff, 0xff, 0x00,	/*  3:���� */
		0x00, 0x00, 0xff,	/*  4:���� */
		0xff, 0x00, 0xff,	/*  5:���� */
		0x00, 0xff, 0xff,	/*  6:ǳ���� */
		0xff, 0xff, 0xff,	/*  7:�� */
		0xc6, 0xc6, 0xc6,	/*  8:���� */
		0x84, 0x00, 0x00,	/*  9:���� */
		0x00, 0x84, 0x00,	/* 10:���� */
		0x84, 0x84, 0x00,	/* 11:���� */
		0x00, 0x00, 0x84,	/* 12:���� */
		0x84, 0x00, 0x84,	/* 13:���� */
		0x00, 0x84, 0x84,	/* 14:ǳ���� */
		0x84, 0x84, 0x84	/* 15:���� */
	};
	set_palette(0, 15, table_rgb);		/* ���õ�ɫ�� */
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	/* ����eflags��ֵ */
	io_cli(); 					/* �ر����п������ж� */
	io_out8(0x03c8, start);		
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	/* �ָ�eflags��ֵ */
	return;
}

/*
	boxfill8����������Ļ�ϻ���һ������ͼ��
	unsigned char *vram	: �Դ��׵�ַ
	int	xsize			: ÿ�е�����
	unsigned char c		: Ҫ������ɫ (ʵ�������Ǹ���ɫ�ڵ�ɫ���е��±�)
	int x0, y0			: ����ͼ�ε����Ͻ�����
	int x1, y1			: ����ͼ�ε����½�����
	x��ʾ�� y��ʾ��	����320*200��ʾ�ľ���������Ļ�ֳ�200�У�ÿ����320��
 */
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {		/* �� */
		for (x = x0; x <= x1; x++)		/* �� */
			vram[y * xsize + x] = c;	/* ��� */
	}
	return;
}