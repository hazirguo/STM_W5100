/*********************************************************************************
	�ṩ�̣��ɶ���Ȼ����
	��  ַ��http://www.hschip.com
	ʱ  ��: 2007-11-30
*********************************************************************************/
#include <stm32f10x.h>              /* STM32F10x�ⶨ�� */
#include <string.h>

#include "Device.h"
#include "Net_Parameter.h"				/* ����ͨ�Ų������� */
#include "IO_define.h"					/* ������Ӳ���ӿڶ��� */
#include "W5100.h"						/* W5100���� */
#include "m25p64.h"				/*flash*/


unsigned char HeartBeat = 0;

void delay()
{
	unsigned int j;
	for (j=0; j<0xffee; )
	{
		j++;
	}
}

/**********************************************************************
������: Delay
����: ��ʱϵ������λΪ����
���: ��
����: ��
˵������ʱ������Timer2��ʱ��������1����ļ�����ʵ�ֵ�
**********************************************************************/
void Delay(unsigned int d)
{
	unsigned int i, j;

	i = Timer2_Counter;
	do
	{
		if(Timer2_Counter > i)
			j = Timer2_Counter - i;
		else
			j = 0x100000000 -i + Timer2_Counter;
	}while(j <= d);
}

/**********************************************************************
������: Load_Net_Parameters
����: ��
���: ��
����: ��
˵������24LC01�ж�ȡ����ͨ�Ų���,��Щ��������: ����IP���������룬 �����ַ��
	  ����IP��ַ�������忪����W5100��4���˿ڣ���˻�Ҫ�ֱ��ȡ4���˿ڵĶ˿�
	  �š�Ŀ��IP��ַ��Ŀ�Ķ˿ں�(ֻ����TCP�ͻ��˺�UDPʱ��Ч)��

	  �˿ڹ���ģʽ���ƣ�0��TCP������ģʽ��1��TCP�ͻ���ģʽ��2��UDPģʽ

	  �ڶ�ȡ����֮ǰ���ȼ��J1״̬�����J1��·����ô���������Ĭ�ϲ�������
	  ״̬��Ĭ�ϲ���д��24LC01�С�
**********************************************************************/
void Load_Net_Parameters(void)
{
	//if flash is enabled
	if (SPI_FLASH_ReadID() == FLASH_ID)
	{
		delay();
		SPI_FLASH_BufferRead(Gateway_IP, FLASH_GATEWAY_IP_ADDR, FLASH_GATEWAY_IP_SIZE);
		delay();
		SPI_FLASH_BufferRead(Sub_Mask, FLASH_SUBNET_MASK_ADDR, FLASH_SUBNET_MASK_SIZE);
		delay();
		SPI_FLASH_BufferRead(Phy_Addr, FLASH_PHY_ADDR_ADDR, FLASH_PHY_ADDR_SIZE);
		delay();
		SPI_FLASH_BufferRead(Local_IP, FLASH_LOCAL_IP_ADDR, FLASH_LOCAL_IP_SIZE);
		delay();
		SPI_FLASH_BufferRead(S0_Port, FLASH_LOCAL_PORT_ADDR, FLASH_LOCAL_PORT_SIZE);
	}
	else
	{
		/* �������ز��� */
		Gateway_IP[0] = GATEWAY_IP_ADDR_1;
		Gateway_IP[1] = GATEWAY_IP_ADDR_2;
		Gateway_IP[2] = GATEWAY_IP_ADDR_3;
		Gateway_IP[3] = GATEWAY_IP_ADDR_4;

		/* ������������ */
		Sub_Mask[0] = SUBNET_MASK_1;
		Sub_Mask[1] = SUBNET_MASK_2;
		Sub_Mask[2] = SUBNET_MASK_3;
		Sub_Mask[3] = SUBNET_MASK_4;

		/* ���������ַ */
		Phy_Addr[0] = PHY_ADDR_1;
		Phy_Addr[1] = PHY_ADDR_2;
		Phy_Addr[2] = PHY_ADDR_3;
		Phy_Addr[3] = PHY_ADDR_4;
		Phy_Addr[4] = PHY_ADDR_5;
		Phy_Addr[5] = PHY_ADDR_6;

		/* ����IP��ַ */
		Local_IP[0] = LOCAL_IP_ADDR_1;
		Local_IP[1] = LOCAL_IP_ADDR_2;
		Local_IP[2] = LOCAL_IP_ADDR_3;
		Local_IP[3] = LOCAL_IP_ADDR_4;

		/* ���ض˿�0�Ķ˿ں�5000 */
		S0_Port[0] = S0_PORT_1;  
		S0_Port[1] = S0_PORT_2; 
	}
	/* ���ض˿�0/1�Ĺ���ģʽ */
	S0_Mode = S0_MODE; 
	S1_Mode = S1_MODE;
}

/*****************************************************************
������: W5100_Initialization
����: ��
���: ��
����: ��
˵�����ȶ�W5100��ʼ����Ȼ�������أ����ֱ��ʼ��4���˿�
*****************************************************************/ 
void W5100_Initialization(void)
{
	W5100_Init();

	/* ������ط����� */
	Detect_Gateway();

	/* �˿�0 -- ��Ϊ������*/
	Socket_Init(0);
	
	/* �˿�1 -- ��Ϊ�ͻ���*/
	Socket_Init(1);

	//GPIO_ResetBits(GPIOE, LED_DRIVE); 		/*  ����LEDָʾ��  */
}

/*****************************************************************
������: W5100_Socket_Set
����: �˿ں�
���: �˿�״̬Socket_State
����: ��
˵�����ֱ�����4���˿ڣ����ݶ˿ڹ���ģʽ�����˿�����TCP��������TCP�ͻ���
      ��UDPģʽ��
      �Ӷ˿�״̬�ֽ�Socket_State�����ж϶˿ڵĹ������
*****************************************************************/
void W5100_Socket_Set(SOCKET s)
{
	/* �˿� 0 */
	if (s == 0)
	{
		if(S0_State == 0)
		{
			if(S0_Mode == TCP_SERVER)			/* TCP������ģʽ */
			{
				if(Socket_Listen(0) == TRUE)
					S0_State = S_INIT;
				else
					S0_State = 0;
			}
		}
	}
	else if (s == 1)/* �˿� 1 */
	{
		if(S1_State == 0)
		{
			if(S1_Mode == TCP_SERVER)			/* TCP������ģʽ */
			{
				if(Socket_Listen(1) == TRUE)
					S1_State = S_INIT;
				else
					S1_State = 0;
			}
			else if(S1_Mode == TCP_CLIENT)	/* TCP�ͻ���ģʽ */
			{
				if(Socket_Connect(1) == TRUE)
					S1_State = S_INIT;
				else
					S1_State=0;
			}
		}
	}

}


/*********************************************************************
������: Process_Socket_Data
����: �˿ں�
���: ��
����:
˵�����������ȵ���S_rx_process()��W5100�Ķ˿ڽ������ݻ�������ȡ���ݣ�
	Ȼ�󽫶�ȡ�����ݴ�Rx_Buffer������Temp_Buffer���������д���

	������ϣ������ݴ�Temp_Buffer������Tx_Buffer������������S_tx_process()
	�������ݡ�
*********************************************************************/
void Process_Socket_Data(SOCKET s)
{
	unsigned short size;
	unsigned char cmd;
	unsigned char ret_code = 0;
	
	size = S_rx_process(s);
	
	
	if(s == 0)
	{
		//FE + cmd
		if (size == 2)
		{	
			cmd = Rx_Buffer[1];
			if (IS_NCD_OFF_CMD(cmd))
			{
				channel = cmd - NCD_OFF_BASE;
				NetCommand = NCD_OFF_CMD;
				RELAY_OFF(channel);
				ret_code = 0x55;		
			}
			else if (IS_NCD_ON_CMD(cmd))
			{
				channel = cmd - NCD_ON_BASE;
				NetCommand = NCD_ON_CMD;
				RELAY_ON(channel);
				ret_code = 0x55;
			}
			else if (IS_NCD_RELAY_SENSE_BASE(cmd))
			{
				channel = cmd - NCD_RELAY_SENSE_BASE;
				switch(NetCommand)
				{
					case NCD_OFF_CMD:
					case NCD_ON_CMD:
						ret_code = RELAY_SENSE(channel);
						break;
					case NCD_ALL_OFF_CMD:
						ret_code = RELAY_ALL_OFF_SENSE();
						break;
					case NCD_ALL_ON_CMD:
						ret_code = RELAY_ALL_ON_SENSE();
						break;
					default:
						ret_code = RELAY_SENSE(channel);
						break;
				}
				NetCommand = NCD_SENSE_CMD;
			}
			else if (IS_NCD_ALL_OFF(cmd))
			{
				NetCommand = NCD_ALL_OFF_CMD;	
				RELAY_ALL_OFF();
				ret_code = 0x55;	
			}
			else if (IS_NCD_ALL_ON(cmd))
			{
				NetCommand = NCD_ALL_ON_CMD;	
				RELAY_ALL_ON();
				ret_code = 0x55;
			}
			else
			{
				ret_code = 0;
			}
		}
		
		Tx_Buffer[0] = ret_code;
		S_tx_process(s, 1);
	}
	else if (s == 1)  //�ͻ��˽��յ����ݣ�״̬���ݣ������������ƣ�
	{
	
	}
}


/*********************************************************************
������: Process_UART_Data
����: ��
���: ��
����:
˵�����������Ƚ�UART�����ݴ�UART_Rx_Buffer������Temp_Buffer���������д���

	������ϣ������ݴ�Temp_Buffer������UART_Tx_Buffer�������ȴ��������ݡ�
*********************************************************************/
void Process_UART_Data(void)
{
	unsigned char i;
	
	if (SPI_FLASH_ReadID() != FLASH_ID)
	{
		USART_SendData(USART1, 0xEE);
		return;
	}

	switch(RxCommand)
	{
		case GATEWAY_IP:
				memcpy(Gateway_IP, USART_Rx_Buffer+3, FLASH_GATEWAY_IP_SIZE);
				SPI_FLASH_SectorErase(FLASH_GATEWAY_IP_ADDR);
				SPI_FLASH_BufferWrite(Gateway_IP, FLASH_GATEWAY_IP_ADDR, FLASH_GATEWAY_IP_SIZE);
				delay();
		//		SPI_FLASH_BufferRead(Temp_Buffer, FLASH_GATEWAY_IP_ADDR, FLASH_GATEWAY_IP_SIZE);
				break;
		case SUBNET_MASK:
				memcpy(Sub_Mask, USART_Rx_Buffer+3, FLASH_SUBNET_MASK_SIZE);
				SPI_FLASH_SectorErase(FLASH_SUBNET_MASK_ADDR);
				SPI_FLASH_BufferWrite(Sub_Mask, FLASH_SUBNET_MASK_ADDR, FLASH_SUBNET_MASK_SIZE);
				delay();
			//	SPI_FLASH_BufferRead(Temp_Buffer, FLASH_SUBNET_MASK_ADDR, FLASH_SUBNET_MASK_SIZE);
				break;
	
		case PHYSICAL_ADDR:
			memcpy(Phy_Addr, USART_Rx_Buffer+3, FLASH_PHY_ADDR_SIZE);
			SPI_FLASH_SectorErase(FLASH_PHY_ADDR_ADDR);
			SPI_FLASH_BufferWrite(Phy_Addr, FLASH_PHY_ADDR_ADDR, FLASH_PHY_ADDR_SIZE);
			delay();
		//	SPI_FLASH_BufferRead(Temp_Buffer, FLASH_PHY_ADDR_ADDR, FLASH_PHY_ADDR_SIZE);
			break;
		case LOCAL_IP:
				memcpy(Local_IP, USART_Rx_Buffer+3, FLASH_LOCAL_IP_SIZE);
				SPI_FLASH_SectorErase(FLASH_LOCAL_IP_SIZE);
				SPI_FLASH_BufferWrite(Local_IP, FLASH_LOCAL_IP_ADDR, FLASH_LOCAL_IP_SIZE);
				delay();		
			//	SPI_FLASH_BufferRead(Temp_Buffer, FLASH_LOCAL_IP_ADDR, FLASH_LOCAL_IP_SIZE);
				break;
		case LISTEN_PORT:
			memcpy(S0_Port, USART_Rx_Buffer+3, FLASH_LOCAL_PORT_SIZE);
			SPI_FLASH_SectorErase(FLASH_LOCAL_PORT_ADDR);	
			SPI_FLASH_BufferWrite(S0_Port, FLASH_LOCAL_PORT_ADDR, FLASH_LOCAL_PORT_SIZE);
		//	SPI_FLASH_BufferRead(S0_Port, FLASH_S0_PORT_ADDR, FLASH_S0_PORT_SIZE);
			delay();
			break;
		case REMOTE_IP:
			memcpy(Remote_IP, USART_Rx_Buffer+3, FLASH_REMOTE_IP_SIZE);
			SPI_FLASH_SectorErase(FLASH_LOCAL_IP_SIZE);
			SPI_FLASH_BufferWrite(Remote_IP, FLASH_REMOTE_IP_ADDR, FLASH_REMOTE_IP_SIZE);
			delay();		
			break;
		case REMOTE_PORT:
			memcpy(S1_Port, USART_Rx_Buffer+3, FLASH_REMOTE_PORT_ADDR);
			SPI_FLASH_SectorErase(FLASH_REMOTE_PORT_ADDR);	
			SPI_FLASH_BufferWrite(S1_Port, FLASH_REMOTE_PORT_ADDR, FLASH_REMOTE_PORT_SIZE);
			delay();
			break;
		default:
			USART_SendData(USART1, 0xFF);
			return;
	}
	
	//echo to user
	for(i=0; i<RxCounter; i++)
	{
		USART_SendData(USART1, USART_Rx_Buffer[i]);
		delay();
	}

	RxCounter = 0;
	USART_DataReceive = 0;
}

/*****************************************************************
                            ������
*****************************************************************/	
u8 val;
int main(void)
{
	/* ��ʼ��STM32F103 */
	System_Initialization();
	
	/*
	GPIO_ResetBits(GPIOE, LED_2);
	GPIO_SetBits(GPIOE, LED_1);
	GPIO_SetBits(GPIOE, LED_3);
	
	val = GPIO_ReadOutputDataBit(GPIOE, LED_1);
	
	val = GPIO_ReadOutputDataBit(GPIOE, LED_2);
	
	val = GPIO_ReadOutputDataBit(GPIOE, LED_1 | LED_2);
	
	val = GPIO_ReadOutputData(GPIOE);
	*/
	
	/* ����Ƿ����Ĭ�ϲ�������״̬*/
	Load_Net_Parameters();
	
	/* ��ʼ��W5100 */
	W5100_Initialization();

	do
	{
		/* ����W5100�˿� 0*/
		W5100_Socket_Set(0);

		/* ����W5100�ж� */
		if(W5100_Interrupt)
			W5100_Interrupt_Process();

		/* ���Socket0���յ����� */
		if((S0_Data & S_RECEIVE) == S_RECEIVE)
		{
			S0_Data &= ~S_RECEIVE;
			Process_Socket_Data(0);
		}
		
		/* ���Socket1���յ����� */
		if((S1_Data & S_RECEIVE) == S_RECEIVE)
		{
			S1_Data &= ~S_RECEIVE;
			Process_Socket_Data(1);
		}
		
		/* ������յ� USART1 ������ */
		if(USART_DataReceive == 1)
		{
			USART_DataReceive = 0;
			Process_UART_Data();
		}
		
		/* 1s �ӱ�������һ�� */
		if (HeartBeat == 1)
		{
			HeartBeat = 0;
			/* ����W5100�˿� 1*/
			W5100_Socket_Set(1);
		}

	}while(1);
}
