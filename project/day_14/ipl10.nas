; haribote-ipl
; TAB=4
;==================================================================
; 注释：宅
; 时间：2013年1月21日
; 该程序会将软盘中从0柱面0磁头2扇区开始直到9柱面1磁头18扇区的内容
; 读入内存0x8200~0x34fff中，并跳转到0xc200处继续执行asmhead
;==================================================================

CYLS	EQU		10				; 需要读入的柱面数

		ORG		0x7c00			; 指明程序的装载地址

; 以下的记述用于标准FAT12格式的软盘
		JMP		entry			; 跳转到entry处执行 注意：该跳转指令只占用2个字节，而标准规定的是3个字节 所以才会有下面的DB 0x90
		DB		0x90			; 机器码，事实上，这相当于一个"nop"指令，即空指令
		DB		"HELLOIPL"		; 启动区的名称
		DW		512				; 每扇区字节数
		DB		1				; 每簇的扇区数 此处为1 表面一个簇就等于一个扇区
		DW		1				; 这个值应该表示为第一个FAT文件分配表之前的引导扇区，一般情况只保留一个扇区
		DB		2				; FAT表的个数
		DW		224				; 根目录中文件数的最大值
		DW		2880			; 逻辑扇区总数
		DB		0xf0			; 磁盘的种类
		DW		9				; 每个FAT占用多少个扇区
		DW		18				; 每磁道扇区数
		DW		2				; 磁头数
		DD		0				; 隐藏扇区数
		DD		2880			; 如果逻辑扇区总数为0，则在这里记录扇区总数
		DB		0,0,0x29		; 中断13的驱动器号、未使用、扩展引导标志
		DD		0xffffffff		; 卷序列号
		DB		"HELLO-OS   "	; 卷标，必须是11个字符，不足以空格填充
		DB		"FAT12   "		; 文件系统类型，必须是8个字符，不足填充空格
		RESB	18				; 先空出18字节

; 程序主体

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

; 读磁盘

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; 柱面0
		MOV		DH,0			; 磁头0
		MOV		CL,2			; 扇区2
readloop:
		MOV		SI,0			; 记录失败次数的寄存器
retry:
		MOV		AH,0x02			; AH=0x02 :读入磁盘
		MOV		AL,1			; 读入1个扇区
		MOV		BX,0
		MOV		DL,0x00			; A驱动器
		INT		0x13			; 调用磁盘BIOS
		JNC		next			; 没有出错则跳转到next
		ADD		SI,1			; SI加1
		CMP		SI,5			; SI与5进行比较
		JAE		error			; SI >= 5 跳转到error
		MOV		AH,0x00
		MOV		DL,0x00			; A驱动器
		INT		0x13			; 重置驱动器
		JMP		retry
next:
		MOV		AX,ES			; 把内存地址后移0x200
		ADD		AX,0x0020
		MOV		ES,AX			
		ADD		CL,1			; CL加1
		CMP		CL,18			; 比较CL与18
		JBE		readloop		; CL <= 18 则跳转到readloop
		MOV		CL,1			; 读另一磁头通柱面的1扇区
		ADD		DH,1			; 修改磁头号 使它始终为0或1
		CMP		DH,2			
		JB		readloop		; DH < 2 跳转readloop 开始读另一磁头同柱面的1~18扇区
		MOV		DH,0			; 开始读下一柱面 0磁头 1扇区
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop		; CH < CYLS 跳转readloop

; 	所有的数据读入工作已经完成

		MOV		[0x0ff0],CH		; 可以认为是保存该启动区共读入的柱面数
		JMP		0xc200			; 此处将跳转到asmhead中执行

; 如果不出错的话 此段代码不会被执行
;---------------------------------------------------------------------
error:
		MOV		SI,msg
putloop:						; 主要是输出错误信息
		MOV		AL,[SI]
		ADD		SI,1			; SI加1
		CMP		AL,0
		JE		fin				; 字符串输出结束 则跳转
		MOV		AH,0x0e			
		MOV		BX,15			; bh = 0   bl为配色方案
		INT		0x10			; 调用显卡BIOS
		JMP		putloop			; 继续输出
fin:
		HLT						; 让CPU停止 等待指令
		JMP		fin				; 死循环
msg:
		DB		0x0a, 0x0a		; 换行2次
		DB		"load error"	; 要输出的字符
		DB		0x0a			; 换行
		DB		0				; 结束输出的标识
;-----------------------------------------------------------------------
		RESB	0x7dfe-$		; 填充字符

		DB		0x55, 0xaa
