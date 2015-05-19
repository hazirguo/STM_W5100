/*********************************************************************************
	提供商：成都浩然电子
	网  址：http://www.hschip.com
	时  间: 2007-11-30
*********************************************************************************/
#include <stm32f10x.h>              /* STM32F10x库定义 */
#include <string.h>

#include "Device.h"
#include "Net_Parameter.h"				/* 网络通信参数定义 */
#include "IO_define.h"					/* 评估板硬件接口定义 */
#include "W5100.h"						/* W5100定义 */
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
程序名: Delay
输入: 延时系数，单位为毫秒
输出: 无
返回: 无
说明：延时是利用Timer2定时器产生的1毫秒的计数来实现的
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
程序名: Load_Net_Parameters
输入: 无
输出: 无
返回: 无
说明：从24LC01中读取网络通信参数,这些参数包括: 网关IP，子网掩码， 物理地址，
	  本机IP地址。评估板开放了W5100的4个端口，因此还要分别读取4个端口的端口
	  号、目的IP地址和目的端口号(只有在TCP客户端和UDP时有效)。

	  端口工作模式控制：0：TCP服务器模式；1：TCP客户端模式；2：UDP模式

	  在读取参数之前，先检查J1状态。如果J1短路，那么评估板进入默认参数设置
	  状态。默认参数写入24LC01中。
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
		/* 加载网关参数 */
		Gateway_IP[0] = GATEWAY_IP_ADDR_1;
		Gateway_IP[1] = GATEWAY_IP_ADDR_2;
		Gateway_IP[2] = GATEWAY_IP_ADDR_3;
		Gateway_IP[3] = GATEWAY_IP_ADDR_4;

		/* 加载子网掩码 */
		Sub_Mask[0] = SUBNET_MASK_1;
		Sub_Mask[1] = SUBNET_MASK_2;
		Sub_Mask[2] = SUBNET_MASK_3;
		Sub_Mask[3] = SUBNET_MASK_4;

		/* 加载物理地址 */
		Phy_Addr[0] = PHY_ADDR_1;
		Phy_Addr[1] = PHY_ADDR_2;
		Phy_Addr[2] = PHY_ADDR_3;
		Phy_Addr[3] = PHY_ADDR_4;
		Phy_Addr[4] = PHY_ADDR_5;
		Phy_Addr[5] = PHY_ADDR_6;

		/* 加载IP地址 */
		Local_IP[0] = LOCAL_IP_ADDR_1;
		Local_IP[1] = LOCAL_IP_ADDR_2;
		Local_IP[2] = LOCAL_IP_ADDR_3;
		Local_IP[3] = LOCAL_IP_ADDR_4;

		/* 加载端口0的端口号5000 */
		S0_Port[0] = S0_PORT_1;  
		S0_Port[1] = S0_PORT_2; 
	}
	/* 加载端口0/1的工作模式 */
	S0_Mode = S0_MODE; 
	S1_Mode = S1_MODE;
}

/*****************************************************************
程序名: W5100_Initialization
输入: 无
输出: 无
返回: 无
说明：先对W5100初始化，然后检查网关，最后分别初始化4个端口
*****************************************************************/ 
void W5100_Initialization(void)
{
	W5100_Init();

	/* 检查网关服务器 */
	Detect_Gateway();

	/* 端口0 -- 作为服务器*/
	Socket_Init(0);
	
	/* 端口1 -- 作为客户端*/
	Socket_Init(1);

	//GPIO_ResetBits(GPIOE, LED_DRIVE); 		/*  开启LED指示灯  */
}

/*****************************************************************
程序名: W5100_Socket_Set
输入: 端口号
输出: 端口状态Socket_State
返回: 无
说明：分别设置4个端口，根据端口工作模式，将端口置于TCP服务器、TCP客户端
      或UDP模式。
      从端口状态字节Socket_State可以判断端口的工作情况
*****************************************************************/
void W5100_Socket_Set(SOCKET s)
{
	/* 端口 0 */
	if (s == 0)
	{
		if(S0_State == 0)
		{
			if(S0_Mode == TCP_SERVER)			/* TCP服务器模式 */
			{
				if(Socket_Listen(0) == TRUE)
					S0_State = S_INIT;
				else
					S0_State = 0;
			}
		}
	}
	else if (s == 1)/* 端口 1 */
	{
		if(S1_State == 0)
		{
			if(S1_Mode == TCP_SERVER)			/* TCP服务器模式 */
			{
				if(Socket_Listen(1) == TRUE)
					S1_State = S_INIT;
				else
					S1_State = 0;
			}
			else if(S1_Mode == TCP_CLIENT)	/* TCP客户端模式 */
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
程序名: Process_Socket_Data
输入: 端口号
输出: 无
返回:
说明：本过程先调用S_rx_process()从W5100的端口接收数据缓冲区读取数据，
	然后将读取的数据从Rx_Buffer拷贝到Temp_Buffer缓冲区进行处理。

	处理完毕，将数据从Temp_Buffer拷贝到Tx_Buffer缓冲区。调用S_tx_process()
	发送数据。
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
	else if (s == 1)  //客户端接收到数据（状态数据，用来更新塔灯）
	{
	
	}
}


/*********************************************************************
程序名: Process_UART_Data
输入: 无
输出: 无
返回:
说明：本过程先将UART的数据从UART_Rx_Buffer拷贝到Temp_Buffer缓冲区进行处理。

	处理完毕，将数据从Temp_Buffer拷贝到UART_Tx_Buffer缓冲区等待发送数据。
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
                            主程序
*****************************************************************/	
u8 val;
int main(void)
{
	/* 初始化STM32F103 */
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
	
	/* 检查是否进入默认参数设置状态*/
	Load_Net_Parameters();
	
	/* 初始化W5100 */
	W5100_Initialization();

	do
	{
		/* 设置W5100端口 0*/
		W5100_Socket_Set(0);

		/* 处理W5100中断 */
		if(W5100_Interrupt)
			W5100_Interrupt_Process();

		/* 如果Socket0接收到数据 */
		if((S0_Data & S_RECEIVE) == S_RECEIVE)
		{
			S0_Data &= ~S_RECEIVE;
			Process_Socket_Data(0);
		}
		
		/* 如果Socket1接收到数据 */
		if((S1_Data & S_RECEIVE) == S_RECEIVE)
		{
			S1_Data &= ~S_RECEIVE;
			Process_Socket_Data(1);
		}
		
		/* 如果接收到 USART1 的数据 */
		if(USART_DataReceive == 1)
		{
			USART_DataReceive = 0;
			Process_UART_Data();
		}
		
		/* 1s 钟保持心跳一次 */
		if (HeartBeat == 1)
		{
			HeartBeat = 0;
			/* 设置W5100端口 1*/
			W5100_Socket_Set(1);
		}

	}while(1);
}
