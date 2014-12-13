/*********************************************************************************
	提供商：成都浩然电子
	网  址：http://www.hschip.com
	时  间: 2007-11-30
*********************************************************************************/
#include <stm32f10x_lib.h>              /* STM32F10x库定义 */
#include <string.h>

#include "Device.h"
#include "Net_Parameter.h"				/* 24LC01 EEPROM存储的网络通信参数定义 */
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
	unsigned int i,j;

	i=Timer2_Counter;
	do
	{
		if(Timer2_Counter>i)
			j=Timer2_Counter-i;
		else
			j=0x100000000-i+Timer2_Counter;
	}while(j<=d);
}

/**********************************************************************
程序名: I2C_Write
输入: 地址值，数据字节
输出: 无
返回:
说明：将一个字节的数据写入24LC01B EEPROM的指定的地址
**********************************************************************/
void I2C_Write(unsigned char addr, unsigned char dat)
{
	I2C_Cmd(I2C1, ENABLE);

	/* 发送I2C的START信号 */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* 检查I2C的EV5状态并清除 */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* 发送24LC01B的设备地址 */
	I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);

	/* 检查I2C的EV6状态并清除 */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* 发送写入到24LC01B地址 */
	I2C_SendData(I2C1, addr);

	/* 检查I2C的EV8状态并清除 */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* 发送数据 */
	I2C_SendData(I2C1, dat);

	/* 检查I2C的EV8状态并清除 */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* 发送I2C的STOP信号 */
	I2C_GenerateSTOP(I2C1, ENABLE);

	Delay(10);
}

/**********************************************************************
程序名: I2C_Read
输入: 地址值
输出: 无
返回: 数据字节
说明：从24LC01 EEPROM指定的地址读取一个字节的数据
**********************************************************************/
unsigned char I2C_Read(unsigned char addr)
{
	unsigned char i;

	I2C_Cmd(I2C1, ENABLE);

	/* 发送I2C的START信号 */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* 检查I2C的EV5状态并清除 */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* 发送24LC01B的地址 */
      I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);
	/* 检查I2C的EV6状态并清除 */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* 清除EV6状态 */
	I2C_Cmd(I2C1, ENABLE);

	/* 发送读取数据的地址 */
	I2C_SendData(I2C1, addr);
	/* 检查I2C的EV8状态并清除 */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* 发送I2C的START信号 */
	I2C_GenerateSTART(I2C1, ENABLE);
	/* 检查I2C的EV5状态并清除 */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* 发送I2C读命令 */
      I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Receiver);
	/* 检查I2C的EV6状态并清除 */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

	/* 检查I2C的EV7并清除 */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));

	/* 发送I2C的STOP信号 */
	I2C_GenerateSTOP(I2C1, ENABLE);

	/* 读取数据 */
	i = I2C_ReceiveData(I2C1);

	I2C_Cmd(I2C1, DISABLE);
	Delay(2);
	return i;
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
	unsigned short i;

	/* 检查Jumper 1跳线, 如果J1短路，设置EVB的网络参数为默认值 */
//	if(GPIO_ReadInputDataBit(GPIOC, DIG_IN)==0)
//	{
		/* 设置默认的网关为192.168.0.1 */
//		I2C_Write(GATEWAY_IP_EE,192);
//		I2C_Write(GATEWAY_IP_EE+1,168);
//		I2C_Write(GATEWAY_IP_EE+2,0);
//		I2C_Write(GATEWAY_IP_EE+3,1);

		/* 设置默认的子网掩码为255.255.255.0 */
//		I2C_Write(SUBNET_MASK_EE,255);
//		I2C_Write(SUBNET_MASK_EE+1,255);
//		I2C_Write(SUBNET_MASK_EE+2,255);
//		I2C_Write(SUBNET_MASK_EE+3,0);
  
		/* 设置默认的物理地址为 00.08.DC.01.02.03 */
//		I2C_Write(PHY_ADDR_EE,0x00);
//		I2C_Write(PHY_ADDR_EE+1,0x08);
//		I2C_Write(PHY_ADDR_EE+2,0xdc);
//		I2C_Write(PHY_ADDR_EE+3,0x01);
//		I2C_Write(PHY_ADDR_EE+4,0x02);
//		I2C_Write(PHY_ADDR_EE+5,0x03);
  
 		/* 设置本机默认的IP地址为192.168.0.2 */
//		I2C_Write(IP_ADDR_EE,192);
//		I2C_Write(IP_ADDR_EE+1,168);
//		I2C_Write(IP_ADDR_EE+2,0);
//		I2C_Write(IP_ADDR_EE+3,2);
  
		/* 设置端口0默认的端口号为50000 */
//		I2C_Write(S0_PORT_EE,0xc3);
//		I2C_Write(S0_PORT_EE+1,0x50);

		/* 设置端口0默认的工作方式为TCP服务器 */
//		I2C_Write(S0_MODE_EE, 0x00);

		/* 设置端口1默认的端口号为51000 */
//		I2C_Write(S1_PORT_EE,0xc7);
//		I2C_Write(S1_PORT_EE+1,0x38);

		/* 设置端口1默认的工作方式为TCP服务器 */
//		I2C_Write(S1_MODE_EE, 0x00);

		/* 设置端口2默认的端口号为52000 */
//		I2C_Write(S2_PORT_EE,0xcb);
//		I2C_Write(S2_PORT_EE+1,0x20);

		/* 设置端口2默认的工作方式为TCP服务器 */
//		I2C_Write(S2_MODE_EE, 0x00);

		/* 设置端口3默认的端口号为53000 */
//		I2C_Write(S3_PORT_EE,0xcf);
//		I2C_Write(S3_PORT_EE+1,0x08);

		/* 设置端口3默认的工作方式为TCP服务器 */
//		I2C_Write(S3_MODE_EE, 0x00);
//	}
	/* 加载网关参数 */
//	for(i=0; i<4; i++)
//		Gateway_IP[i] = I2C_Read(GATEWAY_IP_EE+i);
	Gateway_IP[0] = 192;
	Gateway_IP[1] = 168;
	Gateway_IP[2] = 0;
	Gateway_IP[3] = 1;

	/* 加载子网掩码 */
//	for(i=0; i<4; i++)
//		Sub_Mask[i] = I2C_Read(SUBNET_MASK_EE+i);
	Sub_Mask[0]=255;
	Sub_Mask[1]=255;
	Sub_Mask[2]=255;
	Sub_Mask[3]=0;

	/* 加载物理地址 */
//	for(i=0; i<6; i++)
//		Phy_Addr[i]=I2C_Read(PHY_ADDR_EE+i);
	Phy_Addr[0]=0x0c;
	Phy_Addr[1]=0x29;
	Phy_Addr[2]=0xab;
	Phy_Addr[3]=0x7c;
	Phy_Addr[4]=0x00;
	Phy_Addr[5]=0x01;

	/* 加载IP地址 */
//	for(i=0; i<4; i++)
//		IP_Addr[i]=I2C_Read(IP_ADDR_EE+i);
	IP_Addr[0]=192;
	IP_Addr[1]=168;
	IP_Addr[2]=0;
	IP_Addr[3]=20;

	/* 加载端口0的端口号5000 */
	S0_Port[0] = 0x13; //I2C_Read(S0_PORT_EE);
	S0_Port[1] = 0x88;//I2C_Read(S0_PORT_EE+1);

	/* 加载端口0的目的IP地址 */
//	for(i=0; i<4; i++)
//		S0_DIP[i]=I2C_Read(S0_DIP_EE+i);
	S0_DIP[0]=192;
	S0_DIP[1]=168;
	S0_DIP[2]=0;
	S0_DIP[3]=30;
	
	/* 加载端口0的目的端口号6000 */
	S0_DPort[0] = 0x17;//I2C_Read(S0_DPORT_EE);
	S0_DPort[1] = 0x70;//I2C_Read(S0_DPORT_EE+1);
	/* 加载端口0的工作模式 */
	S0_Mode=1;//I2C_Read(S0_MODE_EE);

	/* 加载端口1的端口号 */
	S1_Port[0] = I2C_Read(S1_PORT_EE);
	S1_Port[1] = I2C_Read(S1_PORT_EE+1);
	/* 加载端口1的目的IP地址 */
	for(i=0; i<4; i++)
		S1_DIP[i]=I2C_Read(S1_DIP_EE+i);
	/* 加载端口1的目的端口号 */
	S1_DPort[0] = I2C_Read(S1_DPORT_EE);
	S1_DPort[1] = I2C_Read(S1_DPORT_EE+1);
	/* 加载端口1的工作模式 */
	S1_Mode=I2C_Read(S1_MODE_EE);

	/* 加载端口2的端口号 */
	S2_Port[0] = I2C_Read(S2_PORT_EE);
	S2_Port[1] = I2C_Read(S2_PORT_EE+1);
	/* 加载端口2的目的IP地址 */
	for(i=0; i<4; i++)
		S2_DIP[i]=I2C_Read(S2_DIP_EE+i);
	/* 加载端口2的目的端口号 */
	S2_DPort[0] = I2C_Read(S2_DPORT_EE);
	S2_DPort[1] = I2C_Read(S2_DPORT_EE+1);
	/* 加载端口2的工作模式 */
	S2_Mode=I2C_Read(S2_MODE_EE);

	/* 加载端口3的端口号 */
	S3_Port[0] = I2C_Read(S3_PORT_EE);
	S3_Port[1] = I2C_Read(S3_PORT_EE+1);
	/* 加载端口3的目的IP地址 */
	for(i=0; i<4; i++)
		S3_DIP[i]=I2C_Read(S3_DIP_EE+i);
	/* 加载端口3的目的端口号 */
	S3_DPort[0] = I2C_Read(S3_DPORT_EE);
	S3_DPort[1] = I2C_Read(S3_DPORT_EE+1);
	/* 加载端口3的工作模式 */
	S3_Mode=I2C_Read(S3_MODE_EE);
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

	/* 端口1 */
	Socket_Init(1);

	/* 端口2 */
	Socket_Init(2);

	/* 端口3 */
	Socket_Init(3);

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
		else if(S0_Mode==TCP_CLIENT) 	/* TCP客户端模式 */
		{
			if(Socket_Connect(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else							/* UDP模式 */
		{
			if(Socket_UDP(0)==TRUE)
				S0_State=S_INIT|S_CONN;
			else
				S0_State=0;
		}
	}

	/* 端口 1 */
	if(S1_State==0)
	{
		if(S1_Mode==TCP_SERVER)			/* TCP服务器模式 */
		{
			if(Socket_Listen(1)==TRUE)
				S1_State=S_INIT;
			else
				S1_State=0;
		}
		else if(S1_Mode==TCP_CLIENT)	/* TCP客户端模式 */
		{
			if(Socket_Connect(1)==TRUE)
				S1_State=S_INIT;
			else
				S1_State=0;
		}
		else							/* UDP模式 */
		{
			if(Socket_UDP(1)==TRUE)
				S1_State=S_INIT|S_CONN;
			else
				S1_State=0;
		}
	}

	/* 端口 2 */
	if(S2_State==0)
	{
		if(S2_Mode==TCP_SERVER)			/* TCP服务器模式 */
		{
			if(Socket_Listen(2)==TRUE)
				S2_State=S_INIT;
			else
				S2_State=0;
		}
		else if(S2_Mode==TCP_CLIENT) 	/* TCP客户端模式 */
		{
			if(Socket_Connect(2)==TRUE)
				S2_State=S_INIT;
			else
				S2_State=0;
		}
		else							/* UDP模式 */
		{
			if(Socket_UDP(2)==TRUE)
			S2_State=S_INIT|S_CONN;
			else
				S2_State=0;
		}
	}

	/* 端口 3 */
	if(S3_State==0)
	{
		if(S3_Mode==TCP_SERVER)			/* TCP服务器模式 */
		{
			if(Socket_Listen(3)==TRUE)
				S3_State=S_INIT;
			else
				S3_State=0;
		}
		else if(S3_Mode==TCP_CLIENT) 	/* TCP客户端模式 */
		{
			if(Socket_Connect(3)==TRUE)
				S3_State=S_INIT;
			else
				S3_State=0;
		}
		else							/* UDP模式 */
		{
			if(Socket_UDP(3)==TRUE)
				S3_State=S_INIT|S_CONN;
			else
				S3_State=0;
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
			case 0:			/* 网关 */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(GATEWAY_IP_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 1:			/* 子网掩码 */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(SUBNET_MASK_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 2:			/* 物理地址 */
				if(Temp_Buffer[2]!=8)
					error_process();
				else
				{
					for(i=0;i<6;i++)
						I2C_Write(PHY_ADDR_EE+i,Temp_Buffer[5+i]);

					set_data_ok(6);
				}
				break;
			case 3:			/* 本机IP地址 */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(IP_ADDR_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;

			/* 端口0设置 */
			case 0x10:			/* 端口号 */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S0_PORT_EE,Temp_Buffer[5]);
					I2C_Write(S0_PORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x11:			/* 目的IP地址 */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(S0_DIP_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 0x12:			/* 目的端口号 */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S0_DPORT_EE,Temp_Buffer[5]);
					I2C_Write(S0_DPORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x13:			/* 端口0的工作模式 */
				if(Temp_Buffer[2]!=3)
					error_process();
				else
				{
					S0_Mode=Temp_Buffer[5];
					I2C_Write(S0_MODE_EE,S0_Mode);
					set_data_ok(1);
				}
				break;

			/* 端口1设置 */
			case 0x20:			/* 端口号 */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S1_PORT_EE,Temp_Buffer[5]);
					I2C_Write(S1_PORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x21:			/* 目的IP地址 */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(S1_DIP_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 0x22:			/* 目的端口号 */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S1_DPORT_EE,Temp_Buffer[5]);
					I2C_Write(S1_DPORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x23:			/* 端口1的工作模式 */
				if(Temp_Buffer[2]!=3)
					error_process();
				else
				{
					S1_Mode=Temp_Buffer[5];
					I2C_Write(S1_MODE_EE,S1_Mode);
					set_data_ok(1);
				}
				break;

			/* 端口2设置 */
			case 0x30:			/* 端口号 */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S2_PORT_EE,Temp_Buffer[5]);
					I2C_Write(S2_PORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x31:			/* 目的IP地址 */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(S2_DIP_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 0x32:			/* 目的端口号 */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S2_DPORT_EE,Temp_Buffer[5]);
					I2C_Write(S2_DPORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x33:			/* 端口2的工作模式 */
				if(Temp_Buffer[2]!=3)
					error_process();
				else
				{
					S2_Mode=Temp_Buffer[5];
					I2C_Write(S2_MODE_EE,S2_Mode);
					set_data_ok(1);
				}
				break;

			/* 端口3设置 */
			case 0x40:			/* 端口号 */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S3_PORT_EE,Temp_Buffer[5]);
					I2C_Write(S3_PORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x41:			/* 目的IP地址 */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(S3_DIP_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 0x42:			/* 目的端口号 */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S3_DPORT_EE,Temp_Buffer[5]);
					I2C_Write(S3_DPORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x43:			/* 端口3的工作模式 */
				if(Temp_Buffer[2]!=3)
					error_process();
				else
				{
					S3_Mode=Temp_Buffer[5];
					I2C_Write(S3_MODE_EE,S3_Mode);
					set_data_ok(1);
				}
				break;
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
				case 0:			/* 网关 */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(GATEWAY_IP_EE+i);
					break;
				case 1:			/* 子网掩码 */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(SUBNET_MASK_EE+i);
					break;
				case 2:			/* 物理地址 */
					for(i=0;i<6;i++)
						Temp_Buffer[5+i]=I2C_Read(PHY_ADDR_EE+i);

					Temp_Buffer[2]+=2;
					break;
				case 3:			/* 本机IP地址 */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(IP_ADDR_EE+i);
					break;

				/* 读取端口0的配置参数 */
				case 0x10:			/* 端口0的端口号 */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S0_PORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x11:			/* 端口0的目的IP地址 */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(S0_DIP_EE+i);
					break;
				case 0x12:			/* 端口0的目的端口号 */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S0_DPORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x13:			/* 端口0的工作模式 */
					Temp_Buffer[5]=S0_Mode;
					Temp_Buffer[2]-=3;
					break;

				/* 读取端口1的配置参数 */
				case 0x20:			/* 端口1的端口号 */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S1_PORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x21:			/* 端口1的目的IP地址 */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(S1_DIP_EE+i);
					break;
				case 0x22:			/* 端口1的目的端口号 */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S1_DPORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x23:			/* 端口1的工作模式 */
					Temp_Buffer[5]=S1_Mode;
					Temp_Buffer[2]-=3;
					break;

				/* 读取端口2的配置参数 */
				case 0x30:			/* 端口2的端口号 */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S2_PORT_EE+i);
					Temp_Buffer[2]-=2;
					break;
				case 0x31:			/* 端口2的目的IP地址 */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(S2_DIP_EE+i);
					break;
				case 0x32:			/* 端口2的目的端口号 */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S2_DPORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x33:			/* 端口2的工作模式 */
					Temp_Buffer[5]=S2_Mode;
					Temp_Buffer[2]-=3;
					break;

				/* 读取端口3的配置参数 */
				case 0x40:			/* 端口3的端口号 */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S3_PORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x41:			/* 端口3的目的IP地址 */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(S3_DIP_EE+i);
					break;
				case 0x42:			/* 端口3的目的端口号 */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S3_DPORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x43:			/* 端口3的工作模式 */
					Temp_Buffer[5]=S3_Mode;
					Temp_Buffer[2]-=3;
					break;

				case 0x80:			/* 读取温度值 */
					Temp_Buffer[5]=Temperature>>8;
					Temp_Buffer[6]=Temperature;
					Temp_Buffer[2]-=2;
					break;
				case 0x81:			/* 读取电位器的电压值 */
					Temp_Buffer[5]=Pot>>8;
					Temp_Buffer[6]=Pot;
					Temp_Buffer[2]-=2;
					break;
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

	size=S_rx_process(s);
	memcpy(Tx_Buffer, Rx_Buffer, size);

//	Rx_Data_Process(size);

//	size=Temp_Buffer[2]+3;
//	memcpy(Tx_Buffer, Temp_Buffer, size);

	S_tx_process(s, size);
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
	memcpy(Temp_Buffer, UART_Rx_Buffer, RxCounter);

	Rx_Data_Process(RxCounter);

	TxCounter=Temp_Buffer[2]+3;
	memcpy(UART_Tx_Buffer, Temp_Buffer, TxCounter);

	USART_SendData(USART1, UART_Tx_Buffer[0]);
	TxIndex=1;

	RxCounter=0;
	UART_DataReceive=0;
}

/*********************************************************************
程序名: ADC_Value_Conv
输入: 无
输出: 温度值和模拟电压值
返回:
说明：分别采集8个数据进行算术平均
	  计算温度时,不仅采集温度值，还采集参考电压值，温度值通过计算获得
*********************************************************************/
void ADC_Value_Conv(void)
{
	unsigned char i;

	if(ADC_Complete==1)
	{
		ADC_Complete=0;

		if(ADC_state==0)			/* 完成3个参数的8次采集 */
		{
			Temperature=temperature[0];			/* 8次温度值平均 */
			for(i=1; i<8 ;i++)
				Temperature+=temperature[i];
			Temperature/=8;

			Vref=vref[0];						/* 8次参考电压值平均 */
			for(i=1; i<8; i++)
				Vref+=vref[i];
			Vref/=8;

			Temperature=3376-2680*Temperature/Vref;		/* 计算温度值 */

			Pot=pot[0];							/* 8次模拟电压值平均 */
			for(i=1; i<8; i++)
				Pot+=pot[i];
			Pot=Pot/99;						/* Pot*330/8/4096 */
		}
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);		/* 启动新的一次ADC */
	}
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
			S0_Data&=~S_RECEIVE;
			Process_Socket_Data(0);
		}

		/* 如果Socket1接收到数据 */
		if((S1_Data & S_RECEIVE) == S_RECEIVE)
		{
			S1_Data&=~S_RECEIVE;
			Process_Socket_Data(1);
		}

		/* 如果Socket2接收到数据 */
//		if((S2_Data & S_RECEIVE) == S_RECEIVE)
//		{
//			S2_Data&=~S_RECEIVE;
//			Process_Socket_Data(2);
//		}

		/* 如果Socket3接收到数据 */
//		if((S3_Data & S_RECEIVE) == S_RECEIVE)
//		{
//			S3_Data&=~S_RECEIVE;
//			Process_Socket_Data(3);
//		}

		/* 如果UART接收到完整的数据包 */
		if(UART_DataReceive==1)
		{
			UART_DataReceive=0;
			Process_UART_Data();
		}

		/* 处理AD转换的数据 */
		ADC_Value_Conv();
	}while(1);
}
