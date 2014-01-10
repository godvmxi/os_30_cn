; naskfunc
; TAB=4
;==================================================================
; 注释：宅
; 时间：2013年2月18日
;	注意：这里调用汇编函数时，参数是从左往右压栈的，例如调用test(a,b)
; 	则栈中esp指向的是返回地址, esp+4指向的是参数a, esp+8指向的是参数b
;==================================================================
[FORMAT "WCOFF"]				; 制作目标文件的模式
[INSTRSET "i486p"]				; 使用到486为止的指令
[BITS 32]						; 制作32位模式用的机器语言
[FILE "naskfunc.nas"]			; 源程序文件名

		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
		EXTERN	_inthandler21, _inthandler27, _inthandler2c

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

_load_gdtr:		; void load_gdtr(int limit, int addr);	; 加载GDT	limit是GDT限长  addr是GDT基址
		MOV		AX,[ESP+4]		; limit的低16bit
		MOV		[ESP+6],AX		
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);	; 加载IDT	limit是IDT限长	addr是IDT基址
		MOV		AX,[ESP+4]		; limit的低16bit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET
		
		
; ---------------------------------------------------------- 
;	   主PIC					从PIC
;	IRQ0	时钟			IRQ8	实时钟	
;	IRQ1	键盘			IRQ9	INTOAH
;	IRQ2	接连int			IRQ10	保留
;	IRQ3	串行口2			IRQ11	保留		
;	IEQ4	串行口1			IRQ12	PS2鼠标
;	IEQ5	并行口2			IRQ13	协处理器
;	IEQ6	软盘			IRQ14	硬盘	
;	IEQ7	并行口1			IRQ15	保留	
; ----------------------------------------------------------		
_asm_inthandler21:	; IRQ 1		键盘
		PUSH	ES			
		PUSH	DS
		PUSHAD					; 保存寄存器
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS			; 修改DS ES SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21	; 调用处理函数
		POP		EAX				; 恢复各个寄存器的值
		POPAD
		POP		DS
		POP		ES
		IRETD					; 中断返回

_asm_inthandler27:	; IRQ 7		并行口1
		PUSH	ES
		PUSH	DS
		PUSHAD					; 保存寄存器
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS			; 修改DS ES SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27	; 调用处理函数
		POP		EAX				; 恢复各个寄存器的值
		POPAD
		POP		DS
		POP		ES
		IRETD					; 中断返回

_asm_inthandler2c:	; IRQ 12	PS2鼠标
		PUSH	ES
		PUSH	DS
		PUSHAD					; 保存寄存器
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS			; 修改DS ES SS
		MOV		DS,AX
		MOV		ES,AX			
		CALL	_inthandler2c	; 调用处理函数
		POP		EAX				; 恢复各个寄存器的值
		POPAD
		POP		DS
		POP		ES
		IRETD					; 中断返回
