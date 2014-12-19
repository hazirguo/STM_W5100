/********************************************************************************
	提供商：成都浩然电子有限公司
	网  址：http://www.hschip.com

    时  间: 2007-11-30

    本软件包括5个部分：
    	1. W5100初始化
    	2. W5100的Socket初始化
    	3. Socket连接
    	   如果Socket设置为TCP服务器模式，则调用Socket_Listen()函数,W5100处于侦听状态，直到远程客户端与它连接。
    	   如果Socket设置为TCP客户端模式，则调用Socket_Connect()函数，
    	                                  每调用一次Socket_Connect(s)函数，产生一次连接，
    	                                  如果连接不成功，则产生超时中断，然后可以再调用该函数进行连接。
    	   如果Socket设置为UDP模式,则调用Socket_UDP函数
    	4. Socket数据接收和发送
    	5. W5100中断处理

    置W5100为服务器模式的调用过程：W5100_Init()-->Socket_Init(s)-->Socket_Listen(s)，设置过程即完成，等待客户端的连接。
    置W5100为客户端模式的调用过程：W5100_Init()-->Socket_Init(s)-->Socket_Connect(s)，设置过程即完成，并与远程服务器连接。
    置W5100为UDP模式的调用过程：W5100_Init()-->Socket_Init(s)-->Socket_UDP(s)，设置过程即完成，可以与远程主机UDP通信。

    W5100产生的连接成功、终止连接、接收数据、发送数据、超时等事件，都可以从中断状态中获得。
********************************************************************************/
#include <stm32f10x.h>			/* STM32F10x库定义 */

#include"W5100.h"					/* 定义W5100的寄存器地址、状态 */

#define TRUE	0xff
#define FALSE	0x00

#define W5100_SCS		GPIO_Pin_12		/* 定义W5100的CS引脚 */

typedef unsigned char SOCKET;
extern void Delay(unsigned int d);

/* 端口数据缓冲区 */
extern unsigned char Rx_Buffer[2000];			/* 端口接收数据缓冲区 */
extern unsigned char Tx_Buffer[2000];			/* 端口发送数据缓冲区 */

/* 网络参数寄存器 */
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
输入：通过SPI发送的字节数据
输出：无
返回：通过SPI读取的字节数据
说明：通过SPI发送一个字节并返回接收的一个字节，该子程序与Read_W5100
	和Write_W5100配合使用
******************************************************************/
unsigned char SPI_SendByte(unsigned char dt)
{
	/* 等待数据寄存器空 */
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* 通过SPI1接口发送数据 */
	SPI_I2S_SendData(SPI2, dt);

	/* 等待接收到一个字节的数据 */
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

	/* 返回接收的数据 */
	return SPI_I2S_ReceiveData(SPI2);
}

/*****************************************************************
程序名：Read_W5100
输入: 地址
输出: 无
返回: 读取的数据
说明：从W5100指定的地址读取一个字节
*****************************************************************/
unsigned char Read_W5100(unsigned short addr)
{
	unsigned char i;

	/* 置W5100的CS为低电平 */
	GPIO_ResetBits(GPIOB, W5100_SCS);

	/* 发送读命令 */
	SPI_SendByte(0x0f);

	/* 发送地址 */
	SPI_SendByte(addr/256);
	SPI_SendByte(addr);

	/* 发送一个哑书记并读取数据 */
	i = SPI_SendByte(0);

	/* 置W5100的CS为高电平 */
	GPIO_SetBits(GPIOB, W5100_SCS);

	return i;
}

/*****************************************************************
程序名：Write_W5100
输入: 地址，字节数据
输出: 无
返回: 无
说明：将一个字节写入W5100指定的地址
*****************************************************************/
void Write_W5100(unsigned short addr, unsigned char dat)
{
	/* 置W5100的CS为低电平 */
	GPIO_ResetBits(GPIOB, W5100_SCS);

	/* 发送写命令 */
	SPI_SendByte(0xf0);

	/* 发送地址 */
	SPI_SendByte(addr/256);
	SPI_SendByte(addr);

	/* 写入数据 */
	SPI_SendByte(dat);

	/* 置W5100的CS为高电平 */
	GPIO_SetBits(GPIOB, W5100_SCS);
}

/*------------------------------------------------------------------------------
						W5100初始化函数
在使用W5100之前，对W5100初始化
------------------------------------------------------------------------------*/
void W5100_Init(void)
{
	unsigned char i;

	Write_W5100(W5100_MODE, MODE_RST);		/*软复位W5100*/

	Delay(100);						/*延时100ms，自己定义该函数*/

	/*设置网关(Gateway)的IP地址，4字节 */
	/*使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet*/
	for(i=0; i<4; i++)
		Write_W5100(W5100_GAR+i, Gateway_IP[i]);			/*Gateway_IP为4字节unsigned char数组,自己定义*/

	/*设置子网掩码(MASK)值，4字节。子网掩码用于子网运算*/
	for(i=0; i<4; i++)
		Write_W5100(W5100_SUBR+i, Sub_Mask[i]);			/*SUB_MASK为4字节unsigned char数组,自己定义*/

	/*设置物理地址，6字节，用于唯一标识网络设备的物理地址值
	该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
	如果自己定义物理地址，注意第一个字节必须为偶数*/
	for(i=0; i<6; i++)
		Write_W5100(W5100_SHAR+i, Phy_Addr[i]);			/*PHY_ADDR6字节unsigned char数组,自己定义*/

	/*设置本机的IP地址，4个字节
	注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关*/
	for(i=0; i<4; i++)
		Write_W5100(W5100_SIPR+i, IP_Addr[i]);			/*IP_ADDR为4字节unsigned char数组,自己定义*/

	/*设置发送缓冲区和接收缓冲区的大小，参考W5100数据手册*/
	Write_W5100(W5100_RMSR, 0x55);		/*Socket Rx memory size=2k*/
	Write_W5100(W5100_TMSR, 0x55);		/*Socket Tx mempry size=2k*/

	/* 设置重试时间，默认为2000(200ms) */
	Write_W5100(W5100_RTR, 0x07);
	Write_W5100(W5100_RTR+1, 0xd0);

	/* 设置重试次数，默认为8次 */
	Write_W5100(W5100_RCR, 8);

	/* 启动中断，参考W5100数据手册确定自己需要的中断类型
	IMR_CONFLICT是IP地址冲突异常中断
	IMR_UNREACH是UDP通信时，地址无法到达的异常中断
	其它是Socket事件中断，根据需要添加 */
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

	Write_W5100((W5100_S0_MR), S_MR_TCP);		/*设置socket0为TCP模式*/

	Write_W5100((W5100_S0_CR), S_CR_OPEN);		/*打开socket0*/

	if(Read_W5100(W5100_S0_SSR) != S_SSR_INIT)
	{
		Write_W5100((W5100_S0_CR), S_CR_CLOSE);	/*打开不成功，关闭Socket，然后返回*/
		return FALSE;
	}

	/*检查网关及获取网关的物理地址*/
	for(i=0; i<4; i++)
		Write_W5100((W5100_S0_DIPR+i), IP_Addr[i]+1);	/*向目的地址寄存器写入与本机IP不同的IP值*/

	Write_W5100((W5100_S0_CR), S_CR_CONNECT);		/*打开socket0的TCP连接*/

	Delay(50);						/* 延时20ms */

	i = Read_W5100(W5100_S0_DHAR);			/*读取目的主机的物理地址，该地址就是网关地址*/

	Write_W5100((W5100_S0_CR), S_CR_CLOSE);			/*关闭socket0*/

	if(i == 0xff)
	{
		/**********没有找到网关服务器，或没有与网关服务器成功连接***********/
		/**********              自己添加处理代码                ***********/
		return FALSE;
	}
	return TRUE;
}

/******************************************************************************
                           Socket处理, 其它3个Socket的处理可参照此程序
*****************************************************************************

						Socket初始化
如果成功则返回true, 否则返回false
-----------------------------------------------------------------------------*/
void Socket_Init(SOCKET s)
{
	/*设置分片长度，参考W5100数据手册，该值可以不修改*/
	Write_W5100((W5100_S0_MSS+s*0x100), 0x05);		/*最大分片字节数=1460*/
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
                           设置Socket作为服务器等待远程主机的连接
当本机Socket工作在服务器模式时，引用该程序，等等远程主机的连接
如果设置成功则返回true, 否则返回false
该程序只调用一次，就使W5100设置为服务器模式
-----------------------------------------------------------------------------*/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5100((W5100_S0_MR+s*0x100), S_MR_TCP);		/*设置socket为TCP模式 */
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_OPEN);		/*打开Socket*/

	if(Read_W5100(W5100_S0_SSR+s*0x100) != S_SSR_INIT)
	{
		Write_W5100((W5100_S0_CR+s*0x100), S_CR_CLOSE);	/*打开不成功，关闭Socket，然后返回*/
		return FALSE;
	}

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_LISTEN);		/*设置Socket为侦听模式*/

	if(Read_W5100(W5100_S0_SSR+s*0x100) != S_SSR_LISTEN)
	{
		Write_W5100((W5100_S0_CR+s*0x100), S_CR_CLOSE);		/*设置不成功，关闭Socket，然后返回*/
		return FALSE;
	}

	return TRUE;

	/*至此完成了Socket的打开和设置侦听工作，至于远程客户端是否与它建立连接，则需要等待Socket中断，
	以判断Socket的连接是否成功。参考W5100数据手册的Socket中断状态
	在服务器侦听模式不需要设置目的IP和目的端口号*/
}


/******************************************************************************
                              处理Socket接收和发送的数据
******************************************************************************/
/*-----------------------------------------------------------------------------
如果Socket产生接收数据的中断，则引用该程序进行处理
该程序将Socket的接收到的数据缓存到Rx_buffer数组中，并返回接收的数据字节数
-----------------------------------------------------------------------------*/
unsigned short S_rx_process(SOCKET s)
{
	unsigned short i, j;
	unsigned short rx_size, rx_offset, rx_offset1;

	/*读取接收数据的字节数*/
	rx_size = Read_W5100(W5100_S0_RX_RSR+s*0x100);
	rx_size *= 256;
	rx_size += Read_W5100(W5100_S0_RX_RSR+s*0x100+1);

	/*读取接收缓冲区的偏移量*/
	rx_offset = Read_W5100(W5100_S0_RX_RR+s*0x100);
	rx_offset *= 256;
	rx_offset += Read_W5100(W5100_S0_RX_RR+s*0x100+1);
	rx_offset1 = rx_offset;

	i = rx_offset / S_RX_SIZE;				/*计算实际的物理偏移量，S0_RX_SIZE需要在前面#define中定义*/
																		/*注意S_RX_SIZE的值在W5100_Init()函数的W5100_RMSR中确定*/
	rx_offset = rx_offset - i*S_RX_SIZE;

	j = W5100_RX + s*S_RX_SIZE + rx_offset;		/*实际物理地址为W5100_RX+rx_offset*/
	for(i=0; i<rx_size; i++)
	{
		if(rx_offset >= S_RX_SIZE)
		{
			j = W5100_RX + s*S_RX_SIZE;
			rx_offset = 0;
		}
		Rx_Buffer[i] = Read_W5100(j);		/*将数据缓存到Rx_buffer数组中*/
		j++;
		rx_offset++;
	}

	/*计算下一次偏移量*/
	rx_offset1 += rx_size;
	Write_W5100((W5100_S0_RX_RR+s*0x100), (rx_offset1/256));
	Write_W5100((W5100_S0_RX_RR+s*0x100+1), rx_offset1);

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_RECV);			/*设置RECV命令，等等下一次接收*/

	return rx_size;								/*返回接收的数据字节数*/
}

/*-----------------------------------------------------------------------------
如果要通过Socket发送数据，则引用该程序
要发送的数据缓存在Tx_buffer中, size则是要发送的字节长度
-----------------------------------------------------------------------------*/
unsigned char S_tx_process(SOCKET s, unsigned int size)
{
	unsigned short i, j;
	unsigned short tx_offset, tx_offset1;

	/*读取发送缓冲区的偏移量*/
	tx_offset = Read_W5100(W5100_S0_TX_WR+s*0x100);
	tx_offset *= 256;
	tx_offset += Read_W5100(W5100_S0_TX_WR+s*0x100+1);
	tx_offset1 = tx_offset;

	i = tx_offset/S_TX_SIZE;					/*计算实际的物理偏移量，S0_TX_SIZE需要在前面#define中定义*/
									/*注意S0_TX_SIZE的值在W5100_Init()函数的W5100_TMSR中确定*/
	tx_offset = tx_offset - i*S_TX_SIZE;
	j = W5100_TX + s*S_TX_SIZE + tx_offset;		/*实际物理地址为W5100_TX+tx_offset*/

	for(i=0; i<size; i++)
	{
		if(tx_offset >= S_TX_SIZE)
		{
			j = W5100_TX + s*S_TX_SIZE;
			tx_offset = 0;
		}
		Write_W5100(j, Tx_Buffer[i]);						/*将Tx_buffer缓冲区中的数据写入到发送缓冲区*/
		j++;
		tx_offset++;
	}

	/*计算下一次的偏移量*/
	tx_offset1 += size;
	Write_W5100((W5100_S0_TX_WR+s*0x100), (tx_offset1/256));
	Write_W5100((W5100_S0_TX_WR+s*0x100+1), tx_offset1);

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_SEND);			/*设置SEND命令,启动发送*/

	return TRUE;								/*返回成功*/
}


/******************************************************************************
					W5100中断处理程序框架
******************************************************************************/
void W5100_Interrupt_Process(void)
{
	unsigned char i,j;

	W5100_Interrupt = 0;

	i = Read_W5100(W5100_IR);
	Write_W5100(W5100_IR, (i&0xf0));					/*回写清除中断标志*/

	if((i & IR_CONFLICT) == IR_CONFLICT)	 	/*IP地址冲突异常处理，自己添加代码*/
	{

	}
	
	/* Socket事件处理 */
	if((i & IR_S0_INT) == IR_S0_INT)
	{
		j = Read_W5100(W5100_S0_IR);
		Write_W5100(W5100_S0_IR, j);		/* 回写清中断标志 */

		if(j & S_IR_CON)				/* 在TCP模式下,Socket0成功连接 */
		{
			S0_State |= S_CONN;
		}
		if(j & S_IR_DISCON)				/* 在TCP模式下Socket断开连接处理，自己添加代码 */
		{
			Write_W5100(W5100_S0_CR, S_CR_CLOSE);		/* 关闭端口，等待重新打开连接 */
			S0_State = 0;
		}
		if(j & S_IR_SENDOK)				/* Socket0数据发送完成，可以再次启动S_tx_process()函数发送数据 */
		{
			S0_Data |= S_TRANSMITOK;
		}
		if(j & S_IR_RECV)				/* Socket接收到数据，可以启动S_rx_process()函数 */
		{
			S0_Data |= S_RECEIVE;
			Process_Socket_Data(0);
		}
		if(j & S_IR_TIMEOUT)			/* Socket连接或数据传输超时处理 */
		{
			Write_W5100(W5100_S0_CR, S_CR_CLOSE);		/* 关闭端口，等待重新打开连接 */
			S0_State = 0;
		}
	}
}
