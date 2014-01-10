; naskfunc
; TAB=4
;==================================================================
; 注释：宅
; 时间：2013年2月24日
; 修改了一处注释错误，就是原本在这注释的说该程序中参数的压栈方向
; 好吧 我是错误的，当时我也很奇怪为什么C的函数是从左压栈的 没仔细
; 看清楚。。今天才发现了。。。囧。。。参数是从右往左压的
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
		GLOBAL	_load_cr0, _store_cr0
		GLOBAL	_load_tr
		GLOBAL	_asm_inthandler20, _asm_inthandler21
		GLOBAL	_asm_inthandler27, _asm_inthandler2c
		GLOBAL	_asm_inthandler0d
		GLOBAL	_memtest_sub
		GLOBAL	_farjmp, _farcall
		GLOBAL	_asm_hrb_api, _start_app
		EXTERN	_inthandler20, _inthandler21
		EXTERN	_inthandler27, _inthandler2c
		EXTERN	_inthandler0d
		EXTERN	_hrb_api

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

_load_cr0:		; int load_cr0(void);		; 获取CR0寄存器的值
		MOV		EAX,CR0			
		RET

_store_cr0:		; void store_cr0(int cr0);	; 设置CR0寄存器的值
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET

_load_tr:		; void load_tr(int tr);		; 设置TR寄存器的值
		LTR		[ESP+4]			; tr
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
_asm_inthandler20:	; IRQ0	时钟
		PUSH	ES
		PUSH	DS
		PUSHAD					; 保存寄存器
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS			; 修改DS ES SS	
		MOV		DS,AX			
		MOV		ES,AX
		CALL	_inthandler20	; 调用处理函数
		POP		EAX				; 恢复各个寄存器的值
		POPAD
		POP		DS
		POP		ES
		IRETD

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
		IRETD	

_asm_inthandler0c:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0c
		CMP		EAX,0
		JNE		_asm_end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0c 偱傕丄偙傟偑昁梫
		IRETD

_asm_inthandler0d:				; 通用保护异常
		STI						; 开中断
		PUSH	ES				; 保存寄存器
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP			
		PUSH	EAX	
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		CMP		EAX,0		; 是否为0
		JNE		_asm_end_app; 强制结束应用程序
		POP		EAX			; 以下几条指令暂时是不会被执行到的
		POPAD				; 因为inthandler0d只会返回非零值
		POP		DS			; 也许作者将来会修改inthandler0d
		POP		ES
		ADD		ESP,4		; INT 0x0d必须加上(这是要消掉栈中的一般保护异常的错误号)
		IRETD

; 内存检查函数	检查start地址开始到end地址的范围内,能够使用的内存的末尾地址,并将其作为返回值返回
; 由memtest函数调用
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; 保存EDI ESI EBX
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;
mts_loop:
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET

		
_farjmp:		; void farjmp(int eip, int cs);		; 远跳转
		JMP		FAR	[ESP+4]				; eip, cs
		RET


_farcall:		; void farcall(int eip, int cs);	; 远调用
		CALL	FAR	[ESP+4]				; eip, cs
		RET


_asm_hrb_api:	; 系统调用入口
		STI								; 开中断 中断门会自动关闭中断
	; 注意！！！现在使用的是应用程序的内核态堆栈了  因为通过INT N来使用内核
	; 提供的例程，由于应用程序处于ring3特权级而服务例程处于ring0特权级发生了
	; 特权级的变化  堆栈也要切换。内核态堆栈的初始状态请看start_appa中画出的图
		PUSH	DS						; 将应用程序的寄存器保存起来
		PUSH	ES
		PUSHAD							; 用于保存的PUSH
		PUSHAD							; 用于向hrb_api传值的PUSH
		MOV		AX,SS
		MOV		DS,AX		; 将操作系统用的段地址存入DS和ES (就是内核数据段的段选择子)
		MOV		ES,AX
		CALL	_hrb_api
		CMP		EAX,0		; 当eax不为0时程序结束
		JNE		_asm_end_app
		ADD		ESP,32
		POPAD
		POP		ES
		POP		DS
		IRETD
_asm_end_app:
;	EAX为tss.esp0的地址
		MOV		ESP,[EAX]	; 此处 我们将ESP指向了内核态堆栈的初始值
							; 此时堆栈中是有数据的 就是start_app画出的状态
		MOV		DWORD [EAX+4],0	; 内核态堆栈的段地址SS0设为0
		POPAD				; 将8个通用寄存器出栈
		RET					; 返回cmd_app!!!栈中是有cmd_app的返回地址的

_start_app:		; void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
		PUSHAD		; 将内核使用的寄存器保存起来
		MOV		EAX,[ESP+36]	; 应用程序用的EIP
		MOV		ECX,[ESP+40]	; 应用程序用的CS
		MOV		EDX,[ESP+44]	; 应用程序用的ESP
		MOV		EBX,[ESP+48]	; 应用程序用的DS/SS
		MOV		EBP,[ESP+52]	; tss.esp0的地址
		MOV		[EBP  ],ESP		; 注意！！！这里我们为TSS.esp0赋值了 它等于当前的esp值
		MOV		[EBP+4],SS		; 注意！！！通过观察TSS结构我们知道 这一句是为TSS.ss0赋值
								; 上面两条指令设置了我们应用程序的内核态堆栈
		; 现在 内核栈中的情况如下
;	低地址	edi		<--	栈顶esp
;			esi
;			ebp
;			esp
;			ebx
;			edx
;			ecx
;			eax
;			返回cmd_app函数的地址！！
;			eip
;			cs
;	高地址	esp
;			ds
;	  		&TSS.esp0
;			
;
;
;		分别对各个段寄存器赋值
		MOV		ES,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
;	
		; 事实上 下面两条指令是将应用程序的CS、DS、SS的段选择子的RPL设为3
		OR		ECX,3			; 应用程序用段号和3进行OR运行
		OR		EBX,3			; 应用程序用段号和3进行OR运行
		PUSH	EBX				; 应用程序的SS
		PUSH	EDX				; 应用程序ESP
		PUSH	ECX				; 应用程序CS
		PUSH	EAX				; 应用程序EIP
		RETF
;	注意！！此处我们设置了应用程序的用户态堆栈	
; 	就是上面的	push ebx  和 push edx 设置的
; 	当使用retf返回时 由于发生特权级变化(我们原本是运行在ring0 现在要返回到ring3的应用程序)
;   所以会取出栈中的CS EIP  SS  ESP。所以 执行完retf之后我们的应用程序也开始运行了
;	补充一点，高特权级向低特权级转移时不会发生堆栈切换。

