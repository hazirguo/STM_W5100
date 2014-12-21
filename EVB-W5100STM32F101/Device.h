#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "IO_define.h"

#define TRUE	0xff
#define FALSE	0x00

typedef  unsigned char SOCKET;

/* Timer2��ʱ������ */
unsigned int Timer2_Counter;

unsigned char Temp_Buffer[128];

/* UART1���ݻ����� */
unsigned char USART_Rx_Buffer[128];			/* UART1�������ݻ����� */
unsigned char USART_Tx_Buffer[128];			/* UART1�������ݻ����� */
unsigned short RxCounter;								/* ���������ֽ����ļ��� */
unsigned short TxCounter, TxIndex;			/* ���������ֽ����ļ����ͷ����ֽ����� */
unsigned char USART_DataReceive;				/* ���յ�һ�����������ݰ����üĴ�����1�����������ݺ�üĴ�����0 */

enum COMMAND {
	GATEWAY_IP,
	SUBNET_MASK,
	PHYSICAL_ADDR,
	LOCAL_IP,
	LISTEN_PORT,
	COMMAND_NUM
};
enum COMMAND	 RxCommand;
unsigned short RxDataSizeArr[COMMAND_NUM] = {7, 7, 9, 7, 5};
unsigned short RxDataSize = 128;


/* �˿����ݻ����� */
unsigned char Rx_Buffer[2000];				/* �˿ڽ������ݻ����� */
unsigned char Tx_Buffer[2000];				/* �˿ڷ������ݻ����� */

/* Network parameter registers */
unsigned char Gateway_IP[4];			/* ����IP��ַ */
unsigned char Sub_Mask[4];				/* �������� */
unsigned char Phy_Addr[6];  			/* �����ַ */
unsigned char Local_IP[4];				/* ����IP��ַ */

unsigned char S0_Port[2];   			/* �˿�0�Ķ˿ں� */
unsigned char S0_DIP[4];				/* �˿�0Ŀ��IP��ַ */
unsigned char S0_DPort[2];				/* �˿�0Ŀ�Ķ˿ں� */

unsigned char S1_Port[2];   			/* �˿�1�Ķ˿ں� */
unsigned char S1_DIP[4];   				/* �˿�1Ŀ��IP��ַ */
unsigned char S1_DPort[2];				/* �˿�1Ŀ�Ķ˿ں� */

unsigned char S2_Port[2];				/* �˿�2�Ķ˿ں� */
unsigned char S2_DIP[4];				/* �˿�2Ŀ��IP��ַ */
unsigned char S2_DPort[2];				/* �˿�2Ŀ�Ķ˿ں� */

unsigned char S3_Port[2];				/* �˿�3�Ķ˿ں� */
unsigned char S3_DIP[4];				/* �˿�3Ŀ��IP��ַ */
unsigned char S3_DPort[2];				/* �˿�3Ŀ�Ķ˿ں� */

/* �˿ڵ�����ģʽ */
unsigned char S0_Mode;
unsigned char S1_Mode;
unsigned char S2_Mode;
unsigned char S3_Mode;
	#define TCP_SERVER		0x00		/* TCP������ģʽ */
	#define TCP_CLIENT		0x01		/* TCP�ͻ���ģʽ */
	#define UDP_MODE		0x02		/* UDPģʽ */

unsigned char S0_State;				/* �˿�0״̬��¼ */
unsigned char S1_State;				/* �˿�1״̬��¼ */
unsigned char S2_State;				/* �˿�2״̬��¼ */
unsigned char S3_State;				/* �˿�3״̬��¼ */
	#define S_INIT	0x01				/* �˿���ɳ�ʼ�� */
	#define S_CONN	0x02				/* �˿�������ӣ����������������� */

unsigned char S0_Data;			/* �˿�0���պͷ������ݵ�״̬ */
unsigned char S1_Data;			/* �˿�1���պͷ������ݵ�״̬ */
unsigned char S2_Data;			/* �˿�2���պͷ������ݵ�״̬ */
unsigned char S3_Data;			/* �˿�3���պͷ������ݵ�״̬ */
	#define S_RECEIVE		0x01		/* �˿ڽ��յ�һ�����ݰ� */
	#define S_TRANSMITOK	0x02		/* �˿ڷ���һ�����ݰ���� */

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