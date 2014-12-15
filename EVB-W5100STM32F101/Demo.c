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
	/* 加载网关参数 */
	Gateway_IP[0] = GATEWAY_IP_1;
	Gateway_IP[1] = GATEWAY_IP_2;
	Gateway_IP[2] = GATEWAY_IP_3;
	Gateway_IP[3] = GATEWAY_IP_4;

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
	IP_Addr[0] = IP_ADDR_1;
	IP_Addr[1] = IP_ADDR_2;
	IP_Addr[2] = IP_ADDR_3;
	IP_Addr[3] = IP_ADDR_4;

	/* 加载端口0的端口号5000 */
	S0_Port[0] = S0_PORT_1;  
	S0_Port[1] = S0_PORT_2; 

	/* 加载端口0的工作模式 */
	S0_Mode = S0_MODE; 
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

	/* 端口0 */
	Socket_Init(0);

	GPIO_ResetBits(GPIOB, LED_DRIVE); 		/*  开启LED指示灯  */
}

/*****************************************************************
程序名: W5100_Socket_Set
输入: 无
输出: 端口状态Socket_State
返回: 无
说明：分别设置4个端口，根据端口工作模式，将端口置于TCP服务器、TCP客户端
      或UDP模式。
      从端口状态字节Socket_State可以判断端口的工作情况
*****************************************************************/
void W5100_Socket_Set(void)
{
	/* 端口 0 */
	if(S0_State==0)
	{
		if(S0_Mode==TCP_SERVER)			/* TCP服务器模式 */
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
	}
}

/*****************************************************************
程序名: Rx_Data_process
输入: 数据字节长度
输出: 返回的数据，存储在Temp_Buffer中
返回:
说明：数据包的结构如下:
		| 0xaa | 0x55 | Length | Command | Object | Data |
	Length(1字节): 数据包字节长度，不包括数据包头和本身字节
	Command(1字节)：为0时读取对象数据，为1时设置对象参数
	Object(1字节):	0x00: 网关IP
				   	0x01: 子网掩码
				   	0x02: 物理地址
				   	0x03: 本机IP地址
				   	0x10/0x20/0x30/0x40: 分别为4个端口的端口号
				   	0x11/0x21/0x31/0x41: 分别为4个端口的目的IP地址(只有在TCP客户端和UDP模式下有效)
					0x12/0x22/0x32/0x42: 分别为4个端口的目的端口号(只有在TCP客户端和UDP模式下有效)
					0x12/0x22/0x32/0x42: 分别为4个端口的工作模式
					0x80: 温度(只读)
					0x81: 模拟电压信号(只读)
					0x82: LED的状态
*****************************************************************/
void error_process(void)
{
	Temp_Buffer[2] =2;
	Temp_Buffer[3] |= 0x80;
}

void set_data_ok(unsigned char i)
{
	Temp_Buffer[2]=3;
	Temp_Buffer[5]=i;
}

void Rx_Data_Process(short l)
{
	unsigned char i;

	if((Temp_Buffer[0]!=0xaa)||(Temp_Buffer[1]!=0x55))	/* 数据包头错误 */
		return;

	i=l-3;
	if(i!=Temp_Buffer[2])				/* 数据包字节长度错误 */
		return;

	i=Temp_Buffer[4];					/* 指向对象 */

	if(Temp_Buffer[3])
	{									/* 设置对象参数 */
		switch(i)
		{
			case 0x82:
				if(Temp_Buffer[2]!=3)
					error_process();
				else
				{
					if(Temp_Buffer[5]==1)
					{
						GPIO_SetBits(GPIOB, LED_DRIVE); 		/* 开启LED指示灯 */

						set_data_ok(1);
					}
					else if(Temp_Buffer[5]==0)
					{
						GPIO_ResetBits(GPIOB, LED_DRIVE); 		/* 关闭LED指示灯 */
						set_data_ok(1);
					}
					else
						error_process();
				}
				break;

			default:
				error_process();
				break;
		}
	}
	else				/* 读取对象参数 */
	{
		if(Temp_Buffer[2]!=2)
			error_process();
		else
		{
			Temp_Buffer[2]=6;
			switch(i)
			{
				case 0x82:			/* 读取LED的状态 */
					if(GPIO_ReadOutputDataBit(GPIOB, LED_DRIVE))
						Temp_Buffer[5]=1;
					else
						Temp_Buffer[5]=0;

					Temp_Buffer[2]-=3;
					break;
				default:
					error_process();
					break;
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

	size = S_rx_process(s);
	memcpy(Tx_Buffer, Rx_Buffer, size);

//	Rx_Data_Process(size);

//	size = Temp_Buffer[2]+3;
//	memcpy(Tx_Buffer, Temp_Buffer, size);

	S_tx_process(s, size);
}


/*****************************************************************
                           主程序
*****************************************************************/
int main(void)
{
	/* 初始化STM32F101 */
	System_Initialization();

	/* 检查是否进入默认参数设置状态，然后从24LC01 EEPROM中获取网络参数 */
	Load_Net_Parameters();

	/* 初始化W5100 */
	W5100_Initialization();

	do
	{
		/* 设置W5100端口 */
		W5100_Socket_Set();

		/* 处理W5100中断 */
		if(W5100_Interrupt)
			W5100_Interrupt_Process();

		/* 如果Socket0接收到数据 */
		if((S0_Data & S_RECEIVE) == S_RECEIVE)
		{
			S0_Data &= ~S_RECEIVE;
			Process_Socket_Data(0);
		}
	}while(1);
}
