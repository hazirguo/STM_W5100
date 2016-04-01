#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "IO_define.h"

#define TRUE	0xff
#define FALSE	0x00

typedef  unsigned char SOCKET;

/* Timer2定时器计数 */
unsigned int Timer2_Counter;

unsigned char Temp_Buffer[128];

/* UART1数据缓冲区 */
unsigned char USART_Rx_Buffer[128];			/* UART1接收数据缓冲区 */
unsigned char USART_Tx_Buffer[128];			/* UART1发送数据缓冲区 */
unsigned short RxCounter;								/* 接收数据字节数的计数 */
unsigned short TxCounter, TxIndex;			/* 发送数据字节数的计数和发送字节索引 */
unsigned char USART_DataReceive;				/* 接收到一个完整的数据包，该寄存器置1，处理完数据后该寄存器清0 */

enum COMMAND {
	GATEWAY_IP,
	SUBNET_MASK,
	PHYSICAL_ADDR,
	LOCAL_IP,
	LISTEN_PORT,
	REMOTE_IP,
	REMOTE_PORT,
	READ_NETINFO,
	COMMAND_NUM
};
enum COMMAND	 RxCommand;

/*
命令表：

命令头    | 命令  | 数据
==========|=======|====================
		|	0x00	|	D1 D2 D3 D4	
		|	0x01	|	D1 D2 D3 D4	
		|	0x02	|	D1 D2 D3 D4 D5 D6	
0Xaa 0x55	|	0x03	|	D1 D2 D3 D4
		|	0x04	|	D1 D2 	
		|	0x05	|	D1 D2 D3 D4	
		|	0x06	|	D1 D2	
		|	0x07	|
*/

unsigned short RxDataSizeArr[COMMAND_NUM] = {7, 7, 9, 7, 5, 7, 5, 3};
unsigned short RxDataSize = 128;


/* 端口数据缓冲区 */
unsigned char Rx_Buffer[2000];				/* 端口接收数据缓冲区 */
unsigned char Tx_Buffer[2000];				/* 端口发送数据缓冲区 */

/* Network parameter registers */
unsigned char Gateway_IP[4];			/* 网关IP地址 */
unsigned char Sub_Mask[4];				/* 子网掩码 */
unsigned char Phy_Addr[6];  			/* 物理地址 */
unsigned char Local_IP[4];				/* 本机IP地址 */
unsigned char Remote_IP[4];				/* 远程IP地址 */


unsigned char S0_Port[2];   			/* 端口0的端口号 */
unsigned char S0_DIP[4];				/* 端口0目的IP地址 */
unsigned char S0_DPort[2];				/* 端口0目的端口号 */

unsigned char S1_Port[2];   			/* 端口1的端口号 */
unsigned char S1_DIP[4];   				/* 端口1目的IP地址 */
unsigned char S1_DPort[2];				/* 端口1目的端口号 */

unsigned char S2_Port[2];				/* 端口2的端口号 */
unsigned char S2_DIP[4];				/* 端口2目的IP地址 */
unsigned char S2_DPort[2];				/* 端口2目的端口号 */

unsigned char S3_Port[2];				/* 端口3的端口号 */
unsigned char S3_DIP[4];				/* 端口3目的IP地址 */
unsigned char S3_DPort[2];				/* 端口3目的端口号 */

/* 端口的运行模式 */
unsigned char S0_Mode;
unsigned char S1_Mode;
unsigned char S2_Mode;
unsigned char S3_Mode;
	#define TCP_SERVER		0x00		/* TCP服务器模式 */
	#define TCP_CLIENT		0x01		/* TCP客户端模式 */
	#define UDP_MODE		0x02		/* UDP模式 */

unsigned char S0_State;				/* 端口0状态记录 */
unsigned char S1_State;				/* 端口1状态记录 */
unsigned char S2_State;				/* 端口2状态记录 */
unsigned char S3_State;				/* 端口3状态记录 */
	#define S_INIT	0x01				/* 端口完成初始化 */
	#define S_CONN	0x02				/* 端口完成连接，可以正常传输数据 */

unsigned char S0_Data;			/* 端口0接收和发送数据的状态 */
unsigned char S1_Data;			/* 端口1接收和发送数据的状态 */
unsigned char S2_Data;			/* 端口2接收和发送数据的状态 */
unsigned char S3_Data;			/* 端口3接收和发送数据的状态 */
	#define S_RECEIVE		0x01		/* 端口接收到一个数据包 */
	#define S_TRANSMITOK	0x02		/* 端口发送一个数据包完成 */

unsigned char W5100_Interrupt;

extern void System_Initialization(void);

extern void W5100_Init(void);
extern unsigned char Detect_Gateway(void);
extern void Socket_Init(SOCKET s);
extern unsigned char Socket_Connect(SOCKET s);
extern unsigned char Socket_Listen(SOCKET s);
extern unsigned short S_rx_process(SOCKET s);
extern unsigned char S_tx_process(SOCKET s, unsigned int size);
extern void W5100_Interrupt_Process(void);


#define NCD_OFF_BASE	0
#define NCD_ON_BASE		8
#define NCD_RELAY_SENSE_BASE	16
#define NCD_ALL_OFF		29
#define NCD_ALL_ON		30

#define IS_NCD_OFF_CMD(cmd)		(cmd>=NCD_OFF_BASE && cmd<NCD_ON_BASE)
#define IS_NCD_ON_CMD(cmd)		(cmd>=NCD_ON_BASE && cmd<NCD_RELAY_SENSE_BASE)
#define IS_NCD_RELAY_SENSE_BASE(cmd)	(cmd>=NCD_RELAY_SENSE_BASE && cmd<NCD_RELAY_SENSE_BASE+8)
#define IS_NCD_ALL_OFF(cmd)		(cmd==NCD_ALL_OFF)
#define IS_NCD_ALL_ON(cmd)		(cmd==NCD_ALL_ON)

uint16_t RELAY_PINS[8] = { RELAY_1, RELAY_2 };

enum NETCOMMAND
{
	NCD_OFF_CMD,
	NCD_ON_CMD,
	NCD_SENSE_CMD,
	NCD_ALL_OFF_CMD,
	NCD_ALL_ON_CMD
};
enum NETCOMMAND NetCommand = NCD_SENSE_CMD;
unsigned char channel;

#endif
