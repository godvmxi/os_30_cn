; hello-os
; TAB=4

;==================================================================
; ����FAT12�ĸ���ϸ�����ݿɲ������ϻ�ο���Orange`s һ������ϵͳ��ʵ�֡�
; ��4���й��ڴ˲��ֵĽ���
; ע�ͣ�լ
; ʱ�䣺2013��1��21��
;==================================================================
; ��������Ǳ�׼FAT12��ʽ����ר�õĴ���

		DB		0xeb, 0x4e, 0x90; �˴���3���ֽڵĶ���תָ��Ļ����� ������ת���������ڴ��봦
		DB		"HELLOIPL"		; ������������
		DW		512				; ÿ�����ֽ���
		DB		1				; ÿ�ص������� �˴�Ϊ1 ����һ���ؾ͵���һ������
		DW		1				; ���ֵӦ�ñ�ʾΪ��һ��FAT�ļ������֮ǰ������������һ�����ֻ����һ������
		DB		2				; FAT���ĸ���
		DW		224				; ��Ŀ¼���ļ��������ֵ
		DW		2880			; �߼���������
		DB		0xf0			; ���̵�����
		DW		9				; ÿ��FATռ�ö��ٸ�����
		DW		18				; ÿ�ŵ�������
		DW		2				; ��ͷ��
		DD		0				; ����������
		DD		2880			; ����߼���������Ϊ0�����������¼��������
		DB		0,0,0x29		; �ж�13���������š�δʹ�á���չ������־
		DD		0xffffffff		; �����к�
		DB		"HELLO-OS   "	; ���꣬������11���ַ��������Կո����
		DB		"FAT12   "		; �ļ�ϵͳ���ͣ�������8���ַ����������ո�
		RESB	18				; �ȿճ�18�ֽ�

; �������룬��ƫ��0�ֽڴ��Ķ���ת����

		DB		0xb8, 0x00, 0x00, 0x8e, 0xd0, 0xbc, 0x00, 0x7c
		DB		0x8e, 0xd8, 0x8e, 0xc0, 0xbe, 0x74, 0x7c, 0x8a
		DB		0x04, 0x83, 0xc6, 0x01, 0x3c, 0x00, 0x74, 0x09
		DB		0xb4, 0x0e, 0xbb, 0x0f, 0x00, 0xcd, 0x10, 0xeb
		DB		0xee, 0xf4, 0xeb, 0xfd

; ��Ϣ��ʾ����

		DB		0x0a, 0x0a		; 2������
		DB		"hello, world"
		DB		0x0a			; ����
		DB		0

		RESB	0x1fe-$			; ���0x00

		DB		0x55, 0xaa
; ���²���ʵ���ϲ����������� ǰ��Ĵ��빤ռ����512B�������̵ĵ�һ�����������²��������̵�������������������
		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	4600
		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	1469432