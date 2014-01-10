; naskfunc
; TAB=4
;==================================================================
; ע�ͣ�լ
; ʱ�䣺2013��2��24��
; �޸���һ��ע�ʹ��󣬾���ԭ������ע�͵�˵�ó����в�����ѹջ����
; �ð� ���Ǵ���ģ���ʱ��Ҳ�����ΪʲôC�ĺ����Ǵ���ѹջ�� û��ϸ
; �������������ŷ����ˡ������塣���������Ǵ�������ѹ��
;==================================================================
[FORMAT "WCOFF"]				; ����Ŀ���ļ���ģʽ
[INSTRSET "i486p"]				; ʹ�õ�486Ϊֹ��ָ��
[BITS 32]						; ����32λģʽ�õĻ�������
[FILE "naskfunc.nas"]			; Դ�����ļ���

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

[SECTION .text]					; �����

_io_hlt:	; void io_hlt(void);	
		HLT
		RET

_io_cli:	; void io_cli(void);	; �ر����п������ж�
		CLI
		RET

_io_sti:	; void io_sti(void);	; �����п������ж�
		STI
		RET

_io_stihlt:	; void io_stihlt(void);	
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);	; ��port�˿ڶ���8λ���ݵ�AL��
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

_io_in16:	; int io_in16(int port); ; ��port�˿ڶ���16λ���ݵ�AX��
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:	; int io_in32(int port); ; ��port�˿ڶ���32λ���ݵ�EAX��
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);		; ��8λ��data�����port�˿�
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);	; ��16λ��data�����port�˿�
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);	; ��32λ��data�����port�˿�
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:	; int io_load_eflags(void);		; ��EFLAGS�Ĵ��������ݷ���
		PUSHFD		; PUSH EFLAGS 
		POP		EAX	; EAX = EFLAGS
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);	; ������eflags��������ΪEFLAGS�Ĵ�����ֵ
		MOV		EAX,[ESP+4]	
		PUSH	EAX
		POPFD		; POP EFLAGS 
		RET

_load_gdtr:		; void load_gdtr(int limit, int addr);	; ����GDT	limit��GDT�޳�  addr��GDT��ַ
		MOV		AX,[ESP+4]		; limit�ĵ�16bit
		MOV		[ESP+6],AX		
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);	; ����IDT	limit��IDT�޳�	addr��IDT��ַ
		MOV		AX,[ESP+4]		; limit�ĵ�16bit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

_load_cr0:		; int load_cr0(void);		; ��ȡCR0�Ĵ�����ֵ
		MOV		EAX,CR0			
		RET

_store_cr0:		; void store_cr0(int cr0);	; ����CR0�Ĵ�����ֵ
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET

_load_tr:		; void load_tr(int tr);		; ����TR�Ĵ�����ֵ
		LTR		[ESP+4]			; tr
		RET

; ---------------------------------------------------------- 
;	   ��PIC					��PIC
;	IRQ0	ʱ��			IRQ8	ʵʱ��	
;	IRQ1	����			IRQ9	INTOAH
;	IRQ2	����int			IRQ10	����
;	IRQ3	���п�2			IRQ11	����		
;	IEQ4	���п�1			IRQ12	PS2���
;	IEQ5	���п�2			IRQ13	Э������
;	IEQ6	����			IRQ14	Ӳ��	
;	IEQ7	���п�1			IRQ15	����	
; ----------------------------------------------------------	
_asm_inthandler20:	; IRQ0	ʱ��
		PUSH	ES
		PUSH	DS
		PUSHAD					; ����Ĵ���
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS			; �޸�DS ES SS	
		MOV		DS,AX			
		MOV		ES,AX
		CALL	_inthandler20	; ���ô�������
		POP		EAX				; �ָ������Ĵ�����ֵ
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler21:	; IRQ 1		����
		PUSH	ES			
		PUSH	DS
		PUSHAD					; ����Ĵ���
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS			; �޸�DS ES SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21	; ���ô�������
		POP		EAX				; �ָ������Ĵ�����ֵ
		POPAD
		POP		DS
		POP		ES
		IRETD					; �жϷ���

_asm_inthandler27:	; IRQ 7		���п�1
		PUSH	ES
		PUSH	DS
		PUSHAD					; ����Ĵ���
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS			; �޸�DS ES SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27	; ���ô�������
		POP		EAX				; �ָ������Ĵ�����ֵ
		POPAD
		POP		DS
		POP		ES
		IRETD					; �жϷ���

_asm_inthandler2c:	; IRQ 12	PS2���
		PUSH	ES
		PUSH	DS
		PUSHAD					; ����Ĵ���
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS			; �޸�DS ES SS
		MOV		DS,AX
		MOV		ES,AX			
		CALL	_inthandler2c	; ���ô�������
		POP		EAX				; �ָ������Ĵ�����ֵ
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
		ADD		ESP,4			; INT 0x0c �ł��A���ꂪ�K�v
		IRETD

_asm_inthandler0d:				; ͨ�ñ����쳣
		STI						; ���ж�
		PUSH	ES				; ����Ĵ���
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP			
		PUSH	EAX	
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		CMP		EAX,0		; �Ƿ�Ϊ0
		JNE		_asm_end_app; ǿ�ƽ���Ӧ�ó���
		POP		EAX			; ���¼���ָ����ʱ�ǲ��ᱻִ�е���
		POPAD				; ��Ϊinthandler0dֻ�᷵�ط���ֵ
		POP		DS			; Ҳ�����߽������޸�inthandler0d
		POP		ES
		ADD		ESP,4		; INT 0x0d�������(����Ҫ����ջ�е�һ�㱣���쳣�Ĵ����)
		IRETD

; �ڴ��麯��	���start��ַ��ʼ��end��ַ�ķ�Χ��,�ܹ�ʹ�õ��ڴ��ĩβ��ַ,��������Ϊ����ֵ����
; ��memtest��������
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; ����EDI ESI EBX
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

		
_farjmp:		; void farjmp(int eip, int cs);		; Զ��ת
		JMP		FAR	[ESP+4]				; eip, cs
		RET


_farcall:		; void farcall(int eip, int cs);	; Զ����
		CALL	FAR	[ESP+4]				; eip, cs
		RET


_asm_hrb_api:	; ϵͳ�������
		STI								; ���ж� �ж��Ż��Զ��ر��ж�
	; ע�⣡��������ʹ�õ���Ӧ�ó�����ں�̬��ջ��  ��Ϊͨ��INT N��ʹ���ں�
	; �ṩ�����̣�����Ӧ�ó�����ring3��Ȩ�����������̴���ring0��Ȩ��������
	; ��Ȩ���ı仯  ��ջҲҪ�л����ں�̬��ջ�ĳ�ʼ״̬�뿴start_appa�л�����ͼ
		PUSH	DS						; ��Ӧ�ó���ļĴ�����������
		PUSH	ES
		PUSHAD							; ���ڱ����PUSH
		PUSHAD							; ������hrb_api��ֵ��PUSH
		MOV		AX,SS
		MOV		DS,AX		; ������ϵͳ�õĶε�ַ����DS��ES (�����ں����ݶεĶ�ѡ����)
		MOV		ES,AX
		CALL	_hrb_api
		CMP		EAX,0		; ��eax��Ϊ0ʱ�������
		JNE		_asm_end_app
		ADD		ESP,32
		POPAD
		POP		ES
		POP		DS
		IRETD
_asm_end_app:
;	EAXΪtss.esp0�ĵ�ַ
		MOV		ESP,[EAX]	; �˴� ���ǽ�ESPָ�����ں�̬��ջ�ĳ�ʼֵ
							; ��ʱ��ջ���������ݵ� ����start_app������״̬
		MOV		DWORD [EAX+4],0	; �ں�̬��ջ�Ķε�ַSS0��Ϊ0
		POPAD				; ��8��ͨ�üĴ�����ջ
		RET					; ����cmd_app!!!ջ������cmd_app�ķ��ص�ַ��

_start_app:		; void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
		PUSHAD		; ���ں�ʹ�õļĴ�����������
		MOV		EAX,[ESP+36]	; Ӧ�ó����õ�EIP
		MOV		ECX,[ESP+40]	; Ӧ�ó����õ�CS
		MOV		EDX,[ESP+44]	; Ӧ�ó����õ�ESP
		MOV		EBX,[ESP+48]	; Ӧ�ó����õ�DS/SS
		MOV		EBP,[ESP+52]	; tss.esp0�ĵ�ַ
		MOV		[EBP  ],ESP		; ע�⣡������������ΪTSS.esp0��ֵ�� �����ڵ�ǰ��espֵ
		MOV		[EBP+4],SS		; ע�⣡����ͨ���۲�TSS�ṹ����֪�� ��һ����ΪTSS.ss0��ֵ
								; ��������ָ������������Ӧ�ó�����ں�̬��ջ
		; ���� �ں�ջ�е��������
;	�͵�ַ	edi		<--	ջ��esp
;			esi
;			ebp
;			esp
;			ebx
;			edx
;			ecx
;			eax
;			����cmd_app�����ĵ�ַ����
;			eip
;			cs
;	�ߵ�ַ	esp
;			ds
;	  		&TSS.esp0
;			
;
;
;		�ֱ�Ը����μĴ�����ֵ
		MOV		ES,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
;	
		; ��ʵ�� ��������ָ���ǽ�Ӧ�ó����CS��DS��SS�Ķ�ѡ���ӵ�RPL��Ϊ3
		OR		ECX,3			; Ӧ�ó����öκź�3����OR����
		OR		EBX,3			; Ӧ�ó����öκź�3����OR����
		PUSH	EBX				; Ӧ�ó����SS
		PUSH	EDX				; Ӧ�ó���ESP
		PUSH	ECX				; Ӧ�ó���CS
		PUSH	EAX				; Ӧ�ó���EIP
		RETF
;	ע�⣡���˴�����������Ӧ�ó�����û�̬��ջ	
; 	���������	push ebx  �� push edx ���õ�
; 	��ʹ��retf����ʱ ���ڷ�����Ȩ���仯(����ԭ����������ring0 ����Ҫ���ص�ring3��Ӧ�ó���)
;   ���Ի�ȡ��ջ�е�CS EIP  SS  ESP������ ִ����retf֮�����ǵ�Ӧ�ó���Ҳ��ʼ������
;	����һ�㣬����Ȩ�������Ȩ��ת��ʱ���ᷢ����ջ�л���
