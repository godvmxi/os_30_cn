; naskfunc
; TAB=4
;==================================================================
; 注释：宅
; 时间：2013年2月18日
;==================================================================
[FORMAT "WCOFF"]				; 制作目标文件的模式
[INSTRSET "i486p"]				; 使用到486为止的指令
[BITS 32]						; 制作32位模式用的机器语言
[FILE "naskfunc.nas"]			; 源程序文件名

		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags

[SECTION .text]					; 代码段

_io_hlt:	; void io_hlt(void);	
		HLT
		RET

_io_cli:	; void io_cli(void);	; 关闭所有可屏蔽中断
		CLI
		RET

_io_sti:	; void io_sti(void);	; 打开所有可屏蔽中断
		STI
		RET

_io_stihlt:	; void io_stihlt(void);	
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);	; 从port端口读入8位数据到AL中
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

_io_in16:	; int io_in16(int port); ; 从port端口读入16位数据到AX中
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:	; int io_in32(int port); ; 从port端口读入32位数据到EAX中
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);		; 将8位的data输出到port端口
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);	; 将16位的data输出到port端口
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);	; 将32位的data输出到port端口
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:	; int io_load_eflags(void);		; 将EFLAGS寄存器的内容返回
		PUSHFD		; PUSH EFLAGS 
		POP		EAX	; EAX = EFLAGS
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);	; 将参数eflags的内容作为EFLAGS寄存器的值
		MOV		EAX,[ESP+4]	
		PUSH	EAX
		POPFD		; POP EFLAGS 
		RET
