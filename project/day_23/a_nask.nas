;==================================================================
; 注释：宅
; 时间：2013年2月24日
; 该文件定义了应用程序可以使用的API
;==================================================================
[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]

		GLOBAL	_api_putchar
		GLOBAL	_api_putstr0
		GLOBAL	_api_end
		GLOBAL	_api_openwin
		GLOBAL	_api_putstrwin
		GLOBAL	_api_boxfilwin
		GLOBAL	_api_initmalloc
		GLOBAL	_api_malloc
		GLOBAL	_api_free
		GLOBAL	_api_point
		GLOBAL	_api_refreshwin
		GLOBAL	_api_linewin
		GLOBAL	_api_closewin
		GLOBAL	_api_getkey

[SECTION .text]

_api_putchar:	; void api_putchar(int c);		; 输出单个字符
		MOV		EDX,1
		MOV		AL,[ESP+4]		; c
		INT		0x40
		RET

_api_putstr0:	; void api_putstr0(char *s);	; 输出字符串
		PUSH	EBX
		MOV		EDX,2
		MOV		EBX,[ESP+8]		; s
		INT		0x40
		POP		EBX
		RET

_api_end:	; void api_end(void);				; 结束程序
		MOV		EDX,4
		INT		0x40

; 显示窗口		
_api_openwin:	; int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		EDX,5
		MOV		EBX,[ESP+16]	; buf
		MOV		ESI,[ESP+20]	; xsiz
		MOV		EDI,[ESP+24]	; ysiz
		MOV		EAX,[ESP+28]	; col_inv
		MOV		ECX,[ESP+32]	; title
		INT		0x40
		POP		EBX
		POP		ESI
		POP		EDI
		RET

; 在窗口中显示字符
_api_putstrwin:	; void api_putstrwin(int win, int x, int y, int col, int len, char *str);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,6
		MOV		EBX,[ESP+20]	; win
		MOV		ESI,[ESP+24]	; x
		MOV		EDI,[ESP+28]	; y
		MOV		EAX,[ESP+32]	; col
		MOV		ECX,[ESP+36]	; len
		MOV		EBP,[ESP+40]	; str
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET

; 在窗口中描绘方块
_api_boxfilwin:	; void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,7
		MOV		EBX,[ESP+20]	; win
		MOV		EAX,[ESP+24]	; x0
		MOV		ECX,[ESP+28]	; y0
		MOV		ESI,[ESP+32]	; x1
		MOV		EDI,[ESP+36]	; y1
		MOV		EBP,[ESP+40]	; col
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET

; memmam初始化
_api_initmalloc:	; void api_initmalloc(void);
		PUSH	EBX
		MOV		EDX,8
		MOV		EBX,[CS:0x0020]		; malloc椞堟偺斣抧
		MOV		EAX,EBX
		ADD		EAX,32*1024			; 32KB傪懌偡
		MOV		ECX,[CS:0x0000]		; 僨乕僞僙僌儊儞僩偺戝偒偝
		SUB		ECX,EAX
		INT		0x40
		POP		EBX
		RET

; malloc
_api_malloc:		; char *api_malloc(int size);
		PUSH	EBX
		MOV		EDX,9
		MOV		EBX,[CS:0x0020]
		MOV		ECX,[ESP+8]			; size
		INT		0x40
		POP		EBX
		RET

; free
_api_free:			; void api_free(char *addr, int size);
		PUSH	EBX
		MOV		EDX,10
		MOV		EBX,[CS:0x0020]
		MOV		EAX,[ESP+ 8]		; addr
		MOV		ECX,[ESP+12]		; size
		INT		0x40
		POP		EBX
		RET

; 在窗口中画点
_api_point:		; void api_point(int win, int x, int y, int col);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		EDX,11
		MOV		EBX,[ESP+16]	; win
		MOV		ESI,[ESP+20]	; x
		MOV		EDI,[ESP+24]	; y
		MOV		EAX,[ESP+28]	; col
		INT		0x40
		POP		EBX
		POP		ESI
		POP		EDI
		RET

; 刷新窗口
_api_refreshwin:	; void api_refreshwin(int win, int x0, int y0, int x1, int y1);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		EDX,12
		MOV		EBX,[ESP+16]	; win
		MOV		EAX,[ESP+20]	; x0
		MOV		ECX,[ESP+24]	; y0
		MOV		ESI,[ESP+28]	; x1
		MOV		EDI,[ESP+32]	; y1
		INT		0x40
		POP		EBX
		POP		ESI
		POP		EDI
		RET

; 窗口中画直线
_api_linewin:		; void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,13
		MOV		EBX,[ESP+20]	; win
		MOV		EAX,[ESP+24]	; x0
		MOV		ECX,[ESP+28]	; y0
		MOV		ESI,[ESP+32]	; x1
		MOV		EDI,[ESP+36]	; y1
		MOV		EBP,[ESP+40]	; col
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET

; 关闭窗口
_api_closewin:		; void api_closewin(int win);
		PUSH	EBX
		MOV		EDX,14
		MOV		EBX,[ESP+8]	; win
		INT		0x40
		POP		EBX
		RET

; 键盘输入
_api_getkey:		; int api_getkey(int mode);
		MOV		EDX,15
		MOV		EAX,[ESP+4]	; mode
		INT		0x40
		RET

