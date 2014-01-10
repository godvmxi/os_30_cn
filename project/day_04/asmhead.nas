; haribote-os boot asm
; TAB=4
;==================================================================
; ע�ͣ�լ
; ʱ�䣺2013��1��21��
; �ó�������Ϊ���뱣��ģʽ��׼��
; Ȼ����ʽ���뱣��ģʽ
; �����°����������ڴ��еķֲ�	
; ��������C���Գ���
; �������û��ע���� ��Щ��ֵ��֪������
; �����Ժ󲹳�
;==================================================================
BOTPAK	EQU		0x00280000		; �ں˴���λ�ַ
DSKCAC	EQU		0x00100000		; �f�B�X�N�L���b�V���̏ꏊ
DSKCAC0	EQU		0x00008000		; �f�B�X�N�L���b�V���̏ꏊ�i���A�����[�h�j

; BOOT_INFO
CYLS	EQU		0x0ff0			; 0xff0�д������ipl10�ж����������
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; �õ�ַ����Ź�����ɫ��Ŀ����Ϣ����ɫ��λ��
SCRNX	EQU		0x0ff4			; �õ�ַ����ŷֱ��ʵ�X
SCRNY	EQU		0x0ff6			; �õ�ַ����ŷֱ��ʵ�Y
VRAM	EQU		0x0ff8			; �õ�ַ�����ͼ�񻺳����Ŀ�ʼ��ַ

		ORG		0xc200			

; ����������Ϣ

		MOV		AL,0x13			; VGA�Կ���320*100*8λ��ɫ
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; ��¼����ģʽ
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

; ��BIOSȡ�ü����ϸ���LEDָʾ�Ƶ�״̬

		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; ����Ĵ�����Ϊ���뱣��ģʽ��׼��

;----------------------------------------------------------------------	
;	������Ϊ�öδ���ִ��֮��  CPU��������Ӧ�κ��ж�
;	�ɲο�8259A���������		
		MOV		AL,0xff			
		OUT		0x21,AL			; ������оƬ�������ж�
		NOP						; ���δ�оƬ�������ж�
		OUT		0xa1,AL

		CLI						; �ر����п������ж�
;----------------------------------------------------------------------	
; ��A20��ַ����

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout
;----------------------------------------------------------------------	



[INSTRSET "i486p"]				

		LGDT	[GDTR0]			; ����GDTR�Ĵ���
		MOV		EAX,CR0		
		AND		EAX,0x7fffffff	; ��ֹ��ҳ����
		OR		EAX,0x00000001	; ��������ģʽ
		MOV		CR0,EAX
		JMP		pipelineflush	; �����һ����תָ��
;----------------------------------------------------------------------	
; �˺�Ĵ��������ڱ���ģʽ��
pipelineflush:
		MOV		AX,1*8			; �������öμĴ���
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

;----------------------------------------------------------------------	
; ��DS:ESIָ���512KB�����ݸ��Ƶ�ES:EDIָ����ڴ浥Ԫ
; DS:ESI = 8:���ļ�����λ��
; ES:EDI = 8:0x00280000 (�ں˴���εĻ���ַ�պ���0x00280000)
; ������δ��������Ϊ�ǽ��ں˴��븴�Ƶ��ó��������ڴ浥Ԫ��
		MOV		ESI,bootpack	
		MOV		EDI,BOTPAK		
		MOV		ECX,512*1024/4
		CALL	memcpy
;----------------------------------------------------------------------	
; ��ipl10�Ĵ���ת�Ƶ�0x00100000��
		MOV		ESI,0x7c00		
		MOV		EDI,DSKCAC		
		MOV		ECX,512/4
		CALL	memcpy

; ���������ж��������ת�Ƶ�0x00100200��

		MOV		ESI,DSKCAC0+512	
		MOV		EDI,DSKCAC+512	
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	
		SUB		ECX,512/4		; һ��Ҫ��ȡ�Ĵ��� (ÿ��4���ֽ�)
		CALL	memcpy


		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; �]������ׂ����̂��Ȃ�
		MOV		ESI,[EBX+20]	; �]����
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; �]����
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; �X�^�b�N�����l
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; AND�̌��ʂ�0�łȂ����waitkbdout��
		RET

;----------------------------------------------------------------------	
; �ú�����DS:ESIָ����ڴ浥Ԫ�����ݸ��Ƶ�ES:EDIָ����ڴ浥Ԫ
; ÿ�ζ�4���ֽ�
memcpy:
		MOV		EAX,[ESI]		
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			
		RET
;----------------------------------------------------------------------	
; 	ȫ�����������Լ�GDTR�Ĵ���������
		ALIGNB	16				; �ڴ���16�ֽڶ��� (���˲��� �����ڴ�����ǿ϶���)
GDT0:
		RESB	8				; ��������
		DW		0xffff,0x0000,0x9200,0x00cf	; 	32λ�ɶ�д���ݶ�������	���޳� 4G-1 	�λ�ַΪ0  	
											;	Ҳ����˵�öο�Ѱַ0~4G-1	DPL = 0 �ں����ݶ�
		DW		0xffff,0x0000,0x9a28,0x0047	; 	32λ�ɶ���ִ�д����������	���޳�512KB		�λ�ַΪ0x280000
											;	DPL = 0	�ں˴����

		DW		0
GDTR0:										
		DW		8*3-1						; GDT�޳�
		DD		GDT0						; GDT��ַ

		ALIGNB	16
bootpack: