/********************************************************************************
	�ṩ�̣��ɶ���Ȼ�������޹�˾
	��  ַ��http://www.hschip.com

    ʱ  ��: 2007-11-30

    ���������5�����֣�
    	1. W5100��ʼ��
    	2. W5100��Socket��ʼ��
    	3. Socket����
    	   ���Socket����ΪTCP������ģʽ�������Socket_Listen()����,W5100��������״̬��ֱ��Զ�̿ͻ����������ӡ�
    	   ���Socket����ΪTCP�ͻ���ģʽ�������Socket_Connect()������
    	                                  ÿ����һ��Socket_Connect(s)����������һ�����ӣ�
    	                                  ������Ӳ��ɹ����������ʱ�жϣ�Ȼ������ٵ��øú����������ӡ�
    	   ���Socket����ΪUDPģʽ,�����Socket_UDP����
    	4. Socket���ݽ��պͷ���
    	5. W5100�жϴ���

    ��W5100Ϊ������ģʽ�ĵ��ù��̣�W5100_Init()-->Socket_Init(s)-->Socket_Listen(s)�����ù��̼���ɣ��ȴ��ͻ��˵����ӡ�
    ��W5100Ϊ�ͻ���ģʽ�ĵ��ù��̣�W5100_Init()-->Socket_Init(s)-->Socket_Connect(s)�����ù��̼���ɣ�����Զ�̷��������ӡ�
    ��W5100ΪUDPģʽ�ĵ��ù��̣�W5100_Init()-->Socket_Init(s)-->Socket_UDP(s)�����ù��̼���ɣ�������Զ������UDPͨ�š�

    W5100���������ӳɹ�����ֹ���ӡ��������ݡ��������ݡ���ʱ���¼��������Դ��ж�״̬�л�á�
********************************************************************************/
#include <stm32f10x.h>			/* STM32F10x�ⶨ�� */

#include"W5100.h"					/* ����W5100�ļĴ�����ַ��״̬ */

#define TRUE	0xff
#define FALSE	0x00

#define W5100_SCS		GPIO_Pin_12		/* ����W5100��CS���� */

typedef unsigned char SOCKET;
extern void Delay(unsigned int d);

/* �˿����ݻ����� */
extern unsigned char Rx_Buffer[2000];			/* �˿ڽ������ݻ����� */
extern unsigned char Tx_Buffer[2000];			/* �˿ڷ������ݻ����� */

/* ��������Ĵ��� */
extern unsigned char Gateway_IP[4];	     		/* Gateway IP Address */
extern unsigned char Sub_Mask[4];				/* Subnet Mask */
extern unsigned char Phy_Addr[6];  			/* Physical Address */
extern unsigned char IP_Addr[4];				/* Loacal IP Address */

extern unsigned char S0_Port[2];   			/* Socket0 Port number */
extern unsigned char S0_DIP[4];					/* Socket0 Destination IP Address */
extern unsigned char S0_DPort[2];				/* Socket0 Destination Port number */

extern unsigned char S1_Port[2];   			/* Socket1 Port number */
extern unsigned char S1_DIP[4];   			/* Socket1 Destination IP Address */
extern unsigned char S1_DPort[2];				/* Socket1 Destination Port number */

extern unsigned char S2_Port[2];				/* Socket2 Port number */
extern unsigned char S2_DIP[4];				/* Socket2 Destination IP Address */
extern unsigned char S2_DPort[2];				/* Socket2 Destination Port number */

extern unsigned char S3_Port[2];				/* Socket3 Port number */
extern unsigned char S3_DIP[4];				/* Socket3 Destination IP Address */
extern unsigned char S3_DPort[2];				/* Socket3 Destination Port number */

extern unsigned char S0_State;				/* Socket0 state recorder */
extern unsigned char S1_State;				/* Socket1 state recorder */
extern unsigned char S2_State;				/* Socket2 state recorder */
extern unsigned char S3_State;				/* Socket3 state recorder */
	#define S_INIT	0x01
	#define S_CONN	0x02

extern unsigned char S0_Data;			/* Socket0 receive data and transmit OK */
extern unsigned char S1_Data;			/* Socket1 receive data and transmit OK */
extern unsigned char S2_Data;			/* Socket2 receive data and transmit OK */
extern unsigned char S3_Data;			/* Socket3 receive data and transmit OK*/
	#define S_RECEIVE		0x01
	#define S_TRANSMITOK	0x02

extern unsigned char W5100_Interrupt;

extern void Process_Socket_Data(SOCKET s);


/******************************************************************
���룺ͨ��SPI���͵��ֽ�����
�������
���أ�ͨ��SPI��ȡ���ֽ�����
˵����ͨ��SPI����һ���ֽڲ����ؽ��յ�һ���ֽڣ����ӳ�����Read_W5100
	��Write_W5100���ʹ��
******************************************************************/
unsigned char SPI_SendByte(unsigned char dt)
{
	/* �ȴ����ݼĴ����� */
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* ͨ��SPI1�ӿڷ������� */
	SPI_I2S_SendData(SPI2, dt);

	/* �ȴ����յ�һ���ֽڵ����� */
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

	/* ���ؽ��յ����� */
	return SPI_I2S_ReceiveData(SPI2);
}

/*****************************************************************
��������Read_W5100
����: ��ַ
���: ��
����: ��ȡ������
˵������W5100ָ���ĵ�ַ��ȡһ���ֽ�
*****************************************************************/
unsigned char Read_W5100(unsigned short addr)
{
	unsigned char i;

	/* ��W5100��CSΪ�͵�ƽ */
	GPIO_ResetBits(GPIOB, W5100_SCS);

	/* ���Ͷ����� */
	SPI_SendByte(0x0f);

	/* ���͵�ַ */
	SPI_SendByte(addr/256);
	SPI_SendByte(addr);

	/* ����һ������ǲ���ȡ���� */
	i = SPI_SendByte(0);

	/* ��W5100��CSΪ�ߵ�ƽ */
	GPIO_SetBits(GPIOB, W5100_SCS);

	return i;
}

/*****************************************************************
��������Write_W5100
����: ��ַ���ֽ�����
���: ��
����: ��
˵������һ���ֽ�д��W5100ָ���ĵ�ַ
*****************************************************************/
void Write_W5100(unsigned short addr, unsigned char dat)
{
	/* ��W5100��CSΪ�͵�ƽ */
	GPIO_ResetBits(GPIOB, W5100_SCS);

	/* ����д���� */
	SPI_SendByte(0xf0);

	/* ���͵�ַ */
	SPI_SendByte(addr/256);
	SPI_SendByte(addr);

	/* д������ */
	SPI_SendByte(dat);

	/* ��W5100��CSΪ�ߵ�ƽ */
	GPIO_SetBits(GPIOB, W5100_SCS);
}

/*------------------------------------------------------------------------------
						W5100��ʼ������
��ʹ��W5100֮ǰ����W5100��ʼ��
------------------------------------------------------------------------------*/
void W5100_Init(void)
{
	unsigned char i;

	Write_W5100(W5100_MODE, MODE_RST);		/*��λW5100*/

	Delay(100);						/*��ʱ100ms���Լ�����ú���*/

	/*��������(Gateway)��IP��ַ��4�ֽ� */
	/*ʹ�����ؿ���ʹͨ��ͻ�������ľ��ޣ�ͨ�����ؿ��Է��ʵ��������������Internet*/
	for(i=0; i<4; i++)
		Write_W5100(W5100_GAR+i, Gateway_IP[i]);			/*Gateway_IPΪ4�ֽ�unsigned char����,�Լ�����*/

	/*������������(MASK)ֵ��4�ֽڡ���������������������*/
	for(i=0; i<4; i++)
		Write_W5100(W5100_SUBR+i, Sub_Mask[i]);			/*SUB_MASKΪ4�ֽ�unsigned char����,�Լ�����*/

	/*���������ַ��6�ֽڣ�����Ψһ��ʶ�����豸�������ֵַ
	�õ�ֵַ��Ҫ��IEEE���룬����OUI�Ĺ涨��ǰ3���ֽ�Ϊ���̴��룬�������ֽ�Ϊ��Ʒ���
	����Լ����������ַ��ע���һ���ֽڱ���Ϊż��*/
	for(i=0; i<6; i++)
		Write_W5100(W5100_SHAR+i, Phy_Addr[i]);			/*PHY_ADDR6�ֽ�unsigned char����,�Լ�����*/

	/*���ñ�����IP��ַ��4���ֽ�
	ע�⣬����IP�����뱾��IP����ͬһ�����������򱾻����޷��ҵ�����*/
	for(i=0; i<4; i++)
		Write_W5100(W5100_SIPR+i, IP_Addr[i]);			/*IP_ADDRΪ4�ֽ�unsigned char����,�Լ�����*/

	/*���÷��ͻ������ͽ��ջ������Ĵ�С���ο�W5100�����ֲ�*/
	Write_W5100(W5100_RMSR, 0x55);		/*Socket Rx memory size=2k*/
	Write_W5100(W5100_TMSR, 0x55);		/*Socket Tx mempry size=2k*/

	/* ��������ʱ�䣬Ĭ��Ϊ2000(200ms) */
	Write_W5100(W5100_RTR, 0x07);
	Write_W5100(W5100_RTR+1, 0xd0);

	/* �������Դ�����Ĭ��Ϊ8�� */
	Write_W5100(W5100_RCR, 8);

	/* �����жϣ��ο�W5100�����ֲ�ȷ���Լ���Ҫ���ж�����
	IMR_CONFLICT��IP��ַ��ͻ�쳣�ж�
	IMR_UNREACH��UDPͨ��ʱ����ַ�޷�������쳣�ж�
	������Socket�¼��жϣ�������Ҫ��� */
	Write_W5100(W5100_IMR, (IMR_CONFLICT | IMR_UNREACH | IMR_S0_INT));
}

/****************************************************************************
                            Detect Gateway
input:  	None
Output: 	None
Return: 	if fail to detect gateway, return FALSE
		if detect the gateway, return TRUE
****************************************************************************/
unsigned char Detect_Gateway(void)
{
	unsigned char i;

	Write_W5100((W5100_S0_MR), S_MR_TCP);		/*����socket0ΪTCPģʽ*/

	Write_W5100((W5100_S0_CR), S_CR_OPEN);		/*��socket0*/

	if(Read_W5100(W5100_S0_SSR) != S_SSR_INIT)
	{
		Write_W5100((W5100_S0_CR), S_CR_CLOSE);	/*�򿪲��ɹ����ر�Socket��Ȼ�󷵻�*/
		return FALSE;
	}

	/*������ؼ���ȡ���ص������ַ*/
	for(i=0; i<4; i++)
		Write_W5100((W5100_S0_DIPR+i), IP_Addr[i]+1);	/*��Ŀ�ĵ�ַ�Ĵ���д���뱾��IP��ͬ��IPֵ*/

	Write_W5100((W5100_S0_CR), S_CR_CONNECT);		/*��socket0��TCP����*/

	Delay(50);						/* ��ʱ20ms */

	i = Read_W5100(W5100_S0_DHAR);			/*��ȡĿ�������������ַ���õ�ַ�������ص�ַ*/

	Write_W5100((W5100_S0_CR), S_CR_CLOSE);			/*�ر�socket0*/

	if(i == 0xff)
	{
		/**********û���ҵ����ط���������û�������ط������ɹ�����***********/
		/**********              �Լ���Ӵ������                ***********/
		return FALSE;
	}
	return TRUE;
}

/******************************************************************************
                           Socket����, ����3��Socket�Ĵ���ɲ��մ˳���
*****************************************************************************

						Socket��ʼ��
����ɹ��򷵻�true, ���򷵻�false
-----------------------------------------------------------------------------*/
void Socket_Init(SOCKET s)
{
	/*���÷�Ƭ���ȣ��ο�W5100�����ֲᣬ��ֵ���Բ��޸�*/
	Write_W5100((W5100_S0_MSS+s*0x100), 0x05);		/*����Ƭ�ֽ���=1460*/
	Write_W5100((W5100_S0_MSS+s*0x100+1), 0xb4);

	/* Set Socket Port number */
	switch(s)
	{
		case 0:
			Write_W5100(W5100_S0_PORT, S0_Port[0]);	/* Set Local Socket Port number */
			Write_W5100(W5100_S0_PORT+1, S0_Port[1]);
			break;
		default:
			break;
	}
}

/*-----------------------------------------------------------------------------
                           ����Socket��Ϊ�������ȴ�Զ������������
������Socket�����ڷ�����ģʽʱ�����øó��򣬵ȵ�Զ������������
������óɹ��򷵻�true, ���򷵻�false
�ó���ֻ����һ�Σ���ʹW5100����Ϊ������ģʽ
-----------------------------------------------------------------------------*/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5100((W5100_S0_MR+s*0x100), S_MR_TCP);		/*����socketΪTCPģʽ */
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_OPEN);		/*��Socket*/

	if(Read_W5100(W5100_S0_SSR+s*0x100) != S_SSR_INIT)
	{
		Write_W5100((W5100_S0_CR+s*0x100), S_CR_CLOSE);	/*�򿪲��ɹ����ر�Socket��Ȼ�󷵻�*/
		return FALSE;
	}

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_LISTEN);		/*����SocketΪ����ģʽ*/

	if(Read_W5100(W5100_S0_SSR+s*0x100) != S_SSR_LISTEN)
	{
		Write_W5100((W5100_S0_CR+s*0x100), S_CR_CLOSE);		/*���ò��ɹ����ر�Socket��Ȼ�󷵻�*/
		return FALSE;
	}

	return TRUE;

	/*���������Socket�Ĵ򿪺�������������������Զ�̿ͻ����Ƿ������������ӣ�����Ҫ�ȴ�Socket�жϣ�
	���ж�Socket�������Ƿ�ɹ����ο�W5100�����ֲ��Socket�ж�״̬
	�ڷ���������ģʽ����Ҫ����Ŀ��IP��Ŀ�Ķ˿ں�*/
}


/******************************************************************************
                              ����Socket���պͷ��͵�����
******************************************************************************/
/*-----------------------------------------------------------------------------
���Socket�����������ݵ��жϣ������øó�����д���
�ó���Socket�Ľ��յ������ݻ��浽Rx_buffer�����У������ؽ��յ������ֽ���
-----------------------------------------------------------------------------*/
unsigned short S_rx_process(SOCKET s)
{
	unsigned short i, j;
	unsigned short rx_size, rx_offset, rx_offset1;

	/*��ȡ�������ݵ��ֽ���*/
	rx_size = Read_W5100(W5100_S0_RX_RSR+s*0x100);
	rx_size *= 256;
	rx_size += Read_W5100(W5100_S0_RX_RSR+s*0x100+1);

	/*��ȡ���ջ�������ƫ����*/
	rx_offset = Read_W5100(W5100_S0_RX_RR+s*0x100);
	rx_offset *= 256;
	rx_offset += Read_W5100(W5100_S0_RX_RR+s*0x100+1);
	rx_offset1 = rx_offset;

	i = rx_offset / S_RX_SIZE;				/*����ʵ�ʵ�����ƫ������S0_RX_SIZE��Ҫ��ǰ��#define�ж���*/
																		/*ע��S_RX_SIZE��ֵ��W5100_Init()������W5100_RMSR��ȷ��*/
	rx_offset = rx_offset - i*S_RX_SIZE;

	j = W5100_RX + s*S_RX_SIZE + rx_offset;		/*ʵ�������ַΪW5100_RX+rx_offset*/
	for(i=0; i<rx_size; i++)
	{
		if(rx_offset >= S_RX_SIZE)
		{
			j = W5100_RX + s*S_RX_SIZE;
			rx_offset = 0;
		}
		Rx_Buffer[i] = Read_W5100(j);		/*�����ݻ��浽Rx_buffer������*/
		j++;
		rx_offset++;
	}

	/*������һ��ƫ����*/
	rx_offset1 += rx_size;
	Write_W5100((W5100_S0_RX_RR+s*0x100), (rx_offset1/256));
	Write_W5100((W5100_S0_RX_RR+s*0x100+1), rx_offset1);

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_RECV);			/*����RECV����ȵ���һ�ν���*/

	return rx_size;								/*���ؽ��յ������ֽ���*/
}

/*-----------------------------------------------------------------------------
���Ҫͨ��Socket�������ݣ������øó���
Ҫ���͵����ݻ�����Tx_buffer��, size����Ҫ���͵��ֽڳ���
-----------------------------------------------------------------------------*/
unsigned char S_tx_process(SOCKET s, unsigned int size)
{
	unsigned short i, j;
	unsigned short tx_offset, tx_offset1;

	/*��ȡ���ͻ�������ƫ����*/
	tx_offset = Read_W5100(W5100_S0_TX_WR+s*0x100);
	tx_offset *= 256;
	tx_offset += Read_W5100(W5100_S0_TX_WR+s*0x100+1);
	tx_offset1 = tx_offset;

	i = tx_offset/S_TX_SIZE;					/*����ʵ�ʵ�����ƫ������S0_TX_SIZE��Ҫ��ǰ��#define�ж���*/
									/*ע��S0_TX_SIZE��ֵ��W5100_Init()������W5100_TMSR��ȷ��*/
	tx_offset = tx_offset - i*S_TX_SIZE;
	j = W5100_TX + s*S_TX_SIZE + tx_offset;		/*ʵ�������ַΪW5100_TX+tx_offset*/

	for(i=0; i<size; i++)
	{
		if(tx_offset >= S_TX_SIZE)
		{
			j = W5100_TX + s*S_TX_SIZE;
			tx_offset = 0;
		}
		Write_W5100(j, Tx_Buffer[i]);						/*��Tx_buffer�������е�����д�뵽���ͻ�����*/
		j++;
		tx_offset++;
	}

	/*������һ�ε�ƫ����*/
	tx_offset1 += size;
	Write_W5100((W5100_S0_TX_WR+s*0x100), (tx_offset1/256));
	Write_W5100((W5100_S0_TX_WR+s*0x100+1), tx_offset1);

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_SEND);			/*����SEND����,��������*/

	return TRUE;								/*���سɹ�*/
}


/******************************************************************************
					W5100�жϴ��������
******************************************************************************/
void W5100_Interrupt_Process(void)
{
	unsigned char i,j;

	W5100_Interrupt = 0;

	i = Read_W5100(W5100_IR);
	Write_W5100(W5100_IR, (i&0xf0));					/*��д����жϱ�־*/

	if((i & IR_CONFLICT) == IR_CONFLICT)	 	/*IP��ַ��ͻ�쳣�����Լ���Ӵ���*/
	{

	}
	
	/* Socket�¼����� */
	if((i & IR_S0_INT) == IR_S0_INT)
	{
		j = Read_W5100(W5100_S0_IR);
		Write_W5100(W5100_S0_IR, j);		/* ��д���жϱ�־ */

		if(j & S_IR_CON)				/* ��TCPģʽ��,Socket0�ɹ����� */
		{
			S0_State |= S_CONN;
		}
		if(j & S_IR_DISCON)				/* ��TCPģʽ��Socket�Ͽ����Ӵ����Լ���Ӵ��� */
		{
			Write_W5100(W5100_S0_CR, S_CR_CLOSE);		/* �رն˿ڣ��ȴ����´����� */
			S0_State = 0;
		}
		if(j & S_IR_SENDOK)				/* Socket0���ݷ�����ɣ������ٴ�����S_tx_process()������������ */
		{
			S0_Data |= S_TRANSMITOK;
		}
		if(j & S_IR_RECV)				/* Socket���յ����ݣ���������S_rx_process()���� */
		{
			S0_Data |= S_RECEIVE;
			Process_Socket_Data(0);
		}
		if(j & S_IR_TIMEOUT)			/* Socket���ӻ����ݴ��䳬ʱ���� */
		{
			Write_W5100(W5100_S0_CR, S_CR_CLOSE);		/* �رն˿ڣ��ȴ����´����� */
			S0_State = 0;
		}
	}
}
