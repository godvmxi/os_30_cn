/* ==================================================================
	注释：宅
	时间：2013年2月18日
   ================================================================== */
   
/* 函数声明部分 */
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

/* 符号常量声明 */
/* 声明相应颜色在调色板中的下标 */
#define COL8_000000		0		/* 黑色 在调色板中下标为0 */
#define COL8_FF0000		1		/* 亮红色 在调色板中下标为1 */
#define COL8_00FF00		2		/* 亮绿色 在调色板中下标为2 */
#define COL8_FFFF00		3		/* 亮黄色 在调色板中下标为3 */
#define COL8_0000FF		4		/* 亮蓝色 在调色板中下标为4 */	
#define COL8_FF00FF		5		/* 亮紫色 在调色板中下标为5 */
#define COL8_00FFFF		6		/* 浅亮蓝色 在调色板中下标为6 */
#define COL8_FFFFFF		7		/* 白色 在调色板中下标为7 */
#define COL8_C6C6C6		8		/* 亮灰色 在调色板中下标为8 */
#define COL8_840000		9		/* 暗红色 在调色板中下标为9 */
#define COL8_008400		10		/* 暗绿色 在调色板中下标为10 */
#define COL8_848400		11		/* 暗黄色 在调色板中下标为11 */
#define COL8_000084		12		/* 暗蓝色 在调色板中下标为12 */
#define COL8_840084		13		/* 暗紫色 在调色板中下标为13 */
#define COL8_008484		14		/* 浅暗蓝色 在调色板中下标为14 */
#define COL8_848484		15		/* 暗灰色 在调色板中下标为15 */
/* 关于调色板 */
/*
copy from  http://blog.sina.com.cn/s/blog_500bd63c01019y1s.html
	调色板只有图片的颜色小于等于256色的时候才有,16位高彩和24位32位真彩是没有调色板的.                                                                              
	调色板的存在的意义只是在当初486以前为了节省空间的一种采用索引的压缩算法,现在没有人这种东西。  
	调色板是为了节约空简所用的，相当于一个索引。只有16位以下的才用调色板，真彩色不用调色板。  
	让我们来看看下面的例子。  
	有一个长宽各为200个象素，颜色数为16色的彩色图，每一个象素都用R、G、B三个分量表示。因为每个分量有256个级别，
	要用8位(bit)，即一个字节(byte)来表示，所以每个象素需要用3个字节。整个图象要用200×200×3，约120k字节，可不是
	一个小数目呀！如果我们用下面的方法，就能省的多。 因为是一个16色图，也就是说这幅图中最多只有16种颜色，我们可
	以用一个表：表中的每一行记录一种颜色的R、G、B值。这样当我们表示一个象素的颜色时，只需要指出该颜色是在第几行，
	即该颜色在表中的索引值。举个例子，如果表的第0行为255，0，0(红色)，那么当某个象素为红色时，只需要标明0即可。 
	让我们再来计算一下：16种状态可以用4位(bit)表示，所以一个象素要用半个字节。整个图象要用200×200×0.5，约20k字节，
	再加上表占用的字节为3×16=48字节.整个占用的字节数约为前面的1/6，省很多吧？这张R、G、B的表，就是我们常说的调色板(Palette)，
	另一种叫法是颜色查找表LUT(Look Up Table)，似乎更确切一些。Windows位图中便用到了调色板技术。其实不光是Windows位图，
	许多图象文件格式如pcx、tif、gif等都用到了。所以很好地掌握调色板的概念是十分有用的。 
	有一种图，它的颜色数高达256×256×256种，也就是说包含我们上述提到的R、G、B颜色表示方法中所有的颜色，这种图叫做真彩色图(true color)。
	真彩色图并不是说一幅图包含了所有的颜色，而是说它具有显示所有颜色的能力，即最多可以包含所有的颜色。表示真彩色图时，
	每个象素直接用R、G、B三个分量字节表示，而不采用调色板技术。原因很明显：如果用调色板，表示一个象素也要用24位，
	这是因为每种颜色的索引要用24位(因为总共有224种颜色，即调色板有224行)，和直接用R，G，B三个分量表示用的字节数一样，
	不但没有任何便宜，还要加上一个256×256×256×3个字节的大调色板。所以真彩色图直接用R、G、B三个分量表示，它又叫做24位色图。
*/

/* 主函数 */
void HariMain(void)
{
	char *vram;
	int xsize, ysize;

	init_palette();						/* 初始化调色板 */
	vram = (char *) 0xa0000;			
	xsize = 320;						/* 每行320列 */
	ysize = 200;						/* 每列200行 */

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
	/* static声明的变量是"静态"的  在程序结束之前一直"存在" */
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:黑 */
		0xff, 0x00, 0x00,	/*  1:亮红 */
		0x00, 0xff, 0x00,	/*  2:亮绿 */
		0xff, 0xff, 0x00,	/*  3:亮黄 */
		0x00, 0x00, 0xff,	/*  4:亮蓝 */
		0xff, 0x00, 0xff,	/*  5:亮紫 */
		0x00, 0xff, 0xff,	/*  6:浅亮蓝 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:亮灰 */
		0x84, 0x00, 0x00,	/*  9:暗红 */
		0x00, 0x84, 0x00,	/* 10:暗绿 */
		0x84, 0x84, 0x00,	/* 11:暗黄 */
		0x00, 0x00, 0x84,	/* 12:暗青 */
		0x84, 0x00, 0x84,	/* 13:暗紫 */
		0x00, 0x84, 0x84,	/* 14:浅暗蓝 */
		0x84, 0x84, 0x84	/* 15:暗灰 */
	};
	set_palette(0, 15, table_rgb);		/* 设置调色板 */
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	/* 保存eflags的值 */
	io_cli(); 					/* 关闭所有可屏蔽中断 */
	io_out8(0x03c8, start);		
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	/* 恢复eflags的值 */
	return;
}

/*
	boxfill8函数会在屏幕上画出一个矩形图形
	unsigned char *vram	: 显存首地址
	int	xsize			: 每行的列数
	unsigned char c		: 要填充的颜色 (实际上它是该颜色在调色板中的下标)
	int x0, y0			: 矩形图形的左上角坐标
	int x1, y1			: 矩形图形的右下角坐标
	x表示列 y表示行	例如320*200表示的就是整个屏幕分成200行，每行有320列
 */
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {		/* 行 */
		for (x = x0; x <= x1; x++)		/* 列 */
			vram[y * xsize + x] = c;	/* 填充 */
	}
	return;
}
