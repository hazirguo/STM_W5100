/********************************************************************************


    需要包含W5100.h头文件

    本软件由ANSI C编写，在很多单片机的C编译器上都可以通过。
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

#include"W5100.h"				//定义W5100的寄存器地址、状态

typedef  unsigned char SOCKET;

#define S_RX_SIZE	2048		//定义Socket接收缓冲区的大小，可以根据W5100_RMSR的设置修改
#define S_TX_SIZE	2048  		//定义Socket发送缓冲区的大小，可以根据W5100_TMSR的设置修改

/*------------------------------------------------------------------------------
						W5100初始化函数
在使用W5100之前，对W5100初始化
------------------------------------------------------------------------------*/
void W5100_Init(void)
{
	unsigned char *ptr;
	unsigned char i;

	ptr=(unsigned char*)W5100_MODE;		/*软复位W5100*/
	*ptr=MODE_RST;

	delay(100);						/*延时100ms，自己定义该函数*/

	/*设置网关(Gateway)的IP地址，4字节*/
	/*使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet*/
	ptr=(unsigned char*)W5100_GAR;
	for(i=0;i<4;i++)
	{
		*ptr=Gateway_IP[i];			/*Gateway_IP为4字节unsigned char数组,自己定义*/
		ptr++;
	}

	/*设置子网掩码(MASK)值，4字节。子网掩码用于子网运算*/
	ptr=(unsigned char*)W5100_SUBR;
	for(i=0;i<4;i++)
	{
		*ptr=SUB_MASK[i];				/*SUB_MASK为4字节unsigned char数组,自己定义*/
		ptr++;
	}

	/*设置物理地址，6字节，用于唯一标识网络设备的物理地址值
	该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
	如果自己定义物理地址，注意第一个字节必须为偶数*/
	ptr=(unsigned char*)W5100_SHAR;
	for(i=0;i<6;i++)
	{
		*ptr=PHY_ADDR[i];				/*PHY_ADDR6字节unsigned char数组,自己定义*/
		ptr++;
	}

	/*设置本机的IP地址，4个字节
	注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关*/
	ptr=(unsigned char*)W5100_SIPR;		/*Set source IP address*/
	for(i=0;i<4;i++)
	{
		*ptr=IP_ADDR[i];				/*IP_ADDR为4字节unsigned char数组,自己定义*/
		ptr++;
	}

	/*设置发送缓冲区和接收缓冲区的大小，参考W5100数据手册*/
	ptr=(unsigned char*)W5100_RMSR;		/*Socket Rx memory size=2k*/
	*ptr=0x55;

	ptr=(unsigned char*)W5100_TMSR;		/*Socket Tx mempry size=2k*/
	*ptr=0x55;

	/* 启动中断，参考W5100数据手册确定自己需要的中断类型
	IMR_CONFLICT是IP地址冲突异常中断
	IMR_UNREACH是UDP通信时，地址无法到达的异常中断
	其它是Socket事件中断，根据需要添加 */
	ptr=(unsigned char*)W5100_IMR;
	*ptr=(IMR_CONFLICT|IMR_UNREACH|IMR_S0_INT|IMR_S1_INT|IMR_S2_INT|IMR_S3_INT);
}

/******************************************************************************
//                           Socket处理, 其它3个Socket的处理可参照此程序
//*****************************************************************************

						Socket初始化
如果成功则返回true, 否则返回false
-----------------------------------------------------------------------------*/
unsigned char Socket_Init(SOCKET s)
{
	unsigned char *ptr;
	unsigned char i;

	ptr=(unsigned char*)(W5100_S0_MR+s*0x100);
	*ptr=S_MR_TCP;						/*设置socket0为TCP模式*/

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);
	*ptr=S_CR_OPEN;						/*打开socket0*/

	ptr=(unsigned char*)(W5100_S0_SSR+s*0x100);
	if(*ptr!=S_SSR_INIT)
	{
		ptr=(unsigned char*)(W5100_S0_CR+s*0x100);	/*打开不成功，关闭Socket，然后返回*/
		*ptr=S_CR_CLOSE;
		return false;
	}

	/*检查网关及获取网关的物理地址*/
	ptr=(unsigned char*)(W5100_S0_DIPR+s*0x100);
	for(i=0;i<4;i++)
	{
		*ptr=IP_ADDR[i]+1;				/*向目的地址寄存器写入与本机IP不同的IP值*/
		ptr++;
	}

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);
	*ptr=S_CR_CONNECT;					/*打开socket0的TCP连接*/

	delay(20);							/*延时20ms，自己定义该函数，该时间长短可以根据需要调整*/

	ptr=(unsigned char*)(W5100_S0_DHAR+s*0x100);	/*读取目的主机的物理地址，该地址就是网关地址*/
	for(i=0;i<6;i++)
	{
		GATEWAY_PHY[i]=*ptr;				/*GATEWAY_PHY为6字节unsigned char数组,自己定义*/
		ptr++;
	}

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);
	*ptr=S_CR_CLOSE;					/*关闭socket0*/

	if(GATEWAY_PHY[0]==0xff)
	{
		/**********没有找到网关服务器，或没有与网关服务器成功连接***********/
		/**********              自己添加处理代码                ***********/
	}

	/*设置分片长度，参考W5100数据手册，该值可以不修改*/
	ptr=(unsigned char*)(W5100_S0_MSS+s*0x100);		/*最大分片字节数=1460*/
	*ptr=0x05;
	ptr++;
	*ptr=0xb4;

	return true;
}
/*-----------------------------------------------------------------------------
                           设置Socket为客户端与远程服务器连接
当本机Socket工作在客户端模式时，引用该程序，与远程服务器建立连接
如果设置成功则返回true，否则返回false
如果启动连接后出现超时中断，则与服务器连接失败，需要重新调用该程序连接
该程序每调用一次，就与服务器产生一次连接
------------------------------------------------------------------------------*/
unsigned char Socket_Connect(SOCKET s)
{
	unsigned char i,*ptr;

	ptr=(unsigned char*)(W5100_S0_MR+s*0x100);		/*设置Socket为UDP模式*/
	*ptr=S_MR_TCP;

	ptr=(unsigned char*)(W5100_S0_PORT+s*0x100);		/*设置本机source的端口号*/
	*ptr=PORT/256;							/*PORT为unisgned int型，自己定义*/
	ptr++;
	*ptr=PORT;

	ptr=(unsigned char*)(W5100_S0_DIPR+s*0x100);		/*设置远程主机IP地址，即服务器的IP地址*/
	for(i=0;i<4;i++)
	{
		*ptr=D_IP_ADDR[i];					/*D_IP_ADDR为4字节unsigned char数组，自己定义*/
		ptr++;
	}

	ptr=(unsigned char*)(W5100_S0_DPORT+s*0x100);		/*Socket的目的端口号*/
	*ptr=DPORT/256;							/*DPORT为unisgned int型，自己定义*/
	ptr++;
	*ptr=DPORT;

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*打开Socket*/
	*ptr=S_CR_OPEN;

	ptr=(unsigned char*)(W5100_S0_SSR+s*0x100);
	if(*ptr!=S_SSR_INIT)
	{
		ptr=(unsigned char*)(W5100_S0_CR+s*0x100);	/*打开不成功，关闭Socket，然后返回*/
		*ptr=S_CR_CLOSE;
		return false;
	}

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*设置Socket为Connect模式*/
	*ptr=S_CR_CONNECT;
	return true;

	/*至此完成了Socket的打开连接工作，至于它是否与远程服务器建立连接，则需要等待Socket中断，
	以判断Socket的连接是否成功。参考W5100数据手册的Socket中断状态*/
}

/*-----------------------------------------------------------------------------
                           设置Socket作为服务器等待远程主机的连接
当本机Socket工作在服务器模式时，引用该程序，等等远程主机的连接
如果设置成功则返回true, 否则返回false
该程序只调用一次，就使W5100设置为服务器模式
-----------------------------------------------------------------------------*/
unsigned char Socket_Listen(SOCKET s)
{
	unsigned char *ptr;

	ptr=(unsigned char*)(W5100_S0_MR+s*0x100);		/*设置Socket为UDP模式*/
	*ptr=S_MR_TCP;

	ptr=(unsigned char*)(W5100_S0_PORT+s*0x100);		/*设置本机source的端口号*/
	*ptr=PORT/256;							/*PORT为unisgned int型，自己定义，与前面定义的相同*/
	ptr++;
	*ptr=PORT;

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*打开Socket*/
	*ptr=S_CR_OPEN;

	ptr=(unsigned char*)(W5100_S0_SSR+s*0x100);
	if(*ptr!=S_SSR_INIT)
	{
		ptr=(unsigned char*)(W5100_S0_CR+s*0x100);	/*打开不成功，关闭Socket，然后返回*/
		*ptr=S_CR_CLOSE;
		return false;
	}

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*设置Socket为侦听模式*/
	*ptr=S_CR_LISTEN;

	ptr=(unsigned char*)(W5100_S0_SSR+s*0x100);
	if(*ptr!=S_SSR_LISTEN)
	{
		ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*设置不成功，关闭Socket，然后返回*/
		*ptr=S_CR_CLOSE;
		return false;
	}

	return true;

	/*至此完成了Socket的打开和设置侦听工作，至于远程客户端是否与它建立连接，则需要等待Socket中断，
	以判断Socket的连接是否成功。参考W5100数据手册的Socket中断状态
	在服务器侦听模式不需要设置目的IP和目的端口号*/
}

/*-----------------------------------------------------------------------------
					设置Socket为UDP模式
如果Socket工作在UDP模式，引用该程序。在UDP模式下，Socket通信不需要建立连接
如果设置成功则返回true, 否则返回false
该程序只调用一次，就使W5100设置为UDP模式
-----------------------------------------------------------------------------*/
unsigned char Socket_UDP(SOCKET s)
{
	unsigned char *ptr;

	ptr=(unsigned char*)(W5100_S0_MR+s*0x100);		/*设置Socket为UDP模式*/
	*ptr=S_MR_UDP;

	ptr=(unsigned char*)(W5100_S0_PORT+s*0x100);		/*设置本机source的端口号*/
	*ptr=PORT/256;							/*PORT为unisgned int型，自己定义，与前面定义的相同*/
	ptr++;
	*ptr=PORT;

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*打开Socket*/
	*ptr=S_CR_OPEN;

	ptr=(unsigned char*)(W5100_S0_SSR+s*0x100);
	if(*ptr!=S_SSR_UDP)
	{
		ptr=(unsigned char*)(W5100_S0_CR+s*0x100);	/*打开不成功，关闭Socket，然后返回*/
		*ptr=S_CR_CLOSE;
		return false;
	}
	else
		return true;

	/*至此完成了Socket的打开和UDP模式设置，在这种模式下它不需要与远程主机建立连接
	因为Socket不需要建立连接，所以在发送数据前都可以设置目的主机IP和目的Socket的端口号
	如果目的主机IP和目的Socket的端口号是固定的，在运行过程中没有改变，那么也可以在这里设置*/
}


/******************************************************************************
                              处理Socket接收和发送的数据
******************************************************************************/
/*-----------------------------------------------------------------------------
如果Socket产生接收数据的中断，则引用该程序进行处理
该程序将Socket的接收到的数据缓存到Rx_buffer数组中，并返回接收的数据字节数
-----------------------------------------------------------------------------*/
unsigned int S_rx_process(SOCKET s)
{
	unsigned char *ptr;
	unsigned int i, rx_size, rx_offset, rx_offset1;

	ptr=(unsigned char*)(W5100_S0_RX_RSR+s*0x100);		/*读取接收数据的字节数*/
	rx_size=*ptr;
	ptr++;
	rx_size*=256;
	rx_size+=*ptr;

	ptr=(unsigned char*)(W5100_S0_RX_RR+s*0x100);			/*读取接收缓冲区的偏移量*/
	rx_offset=*ptr;
	ptr++;
	rx_offset*=256;
	rx_offset+=*ptr;
	rx_offset1=rx_offset;
	
	i=rx_offset/S_RX_SIZE;							/*计算实际的物理偏移量，S0_RX_SIZE需要在前面#define中定义*/
											/*注意S0_RX_SIZE的值在W5100_Init()函数的W5100_RMSR中确定*/
	rx_offset=rx_offset-i*S_RX_SIZE;

	ptr=(unsigned char*)(W5100_RX+s*S_RX_SIZE+rx_offset);		/*实际物理地址为W5100_RX+rx_offset*/
	for(i=0;i<rx_size;i++)
	{
		if(rx_offset>=S_RX_SIZE)
		{
			ptr=(unsigned char*)(W5100_RX+s*S_RX_SIZE);
			rx_offset=0;
		}
		Rx_buffer[i]=*ptr;						/*将数据缓存到Rx_buffer数组中*/
		ptr++;
		rx_offset++;
	}

	i=rx_offset1+rx_size;
	ptr=(unsigned char*)(W5100_S0_RX_RR+s*0x100);
	*ptr=(i/256);
	ptr++;
	*ptr=i;

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);			/*设置RECV命令，等等下一次接收*/
	*ptr=S_CR_RECV;

	return rx_size;								/*返回接收的数据字节数*/
}

/*-----------------------------------------------------------------------------
如果要通过Socket发送数据，则引用该程序
要发送的数据缓存在Tx_buffer中, size则是要发送的字节长度
-----------------------------------------------------------------------------*/
unsigned char S_tx_process(SOCKET s, unsigned int size)
{
	unsigned char *ptr;
	unsigned int i, tx_offset, tx_offset1;

	ptr=(unsigned char*)(W5100_S0_MR+s*0x100);		/*如果是UDP模式,可以在此设置目的主机的IP和端口号*/
	if(((*ptr)&0x0f)==0x02)
	{
		ptr=(unsigned char*)(W5100_S0_DIPR+s*0x100);
		for(i=0;i<4;i++){						/*设置目的主机IP*/
			*ptr=UDP_DIPR[i];					/*UDP_DIPR为4字节unsigned char数组，自己定义*/
			ptr++;
		}
		ptr=(unsigned char*)(W5100_S0_DPORT+s*0x100);
		*ptr=UDP_DPORT/256;						/*UDP_DIPR为unsigned int型, 自己定义*/
		ptr++;
		*ptr=UDP_DPORT;
	}

	ptr=(unsigned char*)(W5100_S0_TX_FSR+s*0x100);		/*读取缓冲区剩余的长度*/
	i=*ptr;
	i*=256;
	ptr++;
	i+=(*ptr);
	if(i<size)						/*如果剩余的字节长度小于发送字节长度,则返回*/
		return false;

	ptr=(unsigned char*)(W5100_S0_TX_WR+s*0x100);			/*读取发送缓冲区的偏移量*/
	tx_offset=*ptr;
	tx_offset*=256;
	ptr++;
	tx_offset+=(*ptr);
	tx_offset1=tx_offset;
	
	i=tx_offset/S_TX_SIZE;					/*计算实际的物理偏移量，S0_TX_SIZE需要在前面#define中定义*/
											/*注意S0_TX_SIZE的值在W5100_Init()函数的W5100_TMSR中确定*/
	tx_offset=tx_offset-i*S_TX_SIZE;

	ptr=(unsigned char*)(W5100_TX+s*S_TX_SIZE+tx_offset);		/*实际物理地址为W5100_TX+tx_offset*/
	for(i=0;i<size;i++)
	{
		if(tx_offset>=S_TX_SIZE)
		{
			ptr=(unsigned char*)(W5100_TX+s*S_TX_SIZE);
			tx_offset=0;
		}
		*ptr=Tx_buffer[i];						/*将Tx_buffer缓冲区中的数据写入到发送缓冲区*/
		ptr++;
		tx_offset++;
	}

	i=tx_offset1+size;
	ptr=(unsigned char*)(W5100_S0_TX_WR+s*0x100);
	*ptr=(i/256);
	ptr++;
	*ptr=i;

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);	/*设置SEND命令,启动发送*/
	*ptr=S_CR_SEND;

	return true;								/*返回成功*/
}


/******************************************************************************
					W5100中断处理程序框架
******************************************************************************/
void W5100_interrupt_handler(void)
{
	unsigned char *ptr;
	unsigned char i,j;

	ptr=(unsigned char*)W5100_IR;
	i=*ptr;
	*ptr=(i&0xf0);					/*回写清除中断标志*/

	if((i&IR_CONFLICT)==IR_CONFLICT)
	{							/*IP地址冲突异常处理，自己添加代码*/

	}

	if((i&IR_UNREACH)==IR_UNREACH)
	{							/*UDP模式下地址无法到达异常处理，自己添加代码*/

	}

	if((i&IR_S0_INT)==IR_S0_INT)		/*Socket事件处理*/
	{
		ptr=(unsigned char*)W5100_S0_IR;
		j=*ptr;
		*ptr=j;				/*回写清中断标志，注意，有的编译器会优化掉该语句*/

		if(j&S_IR_CON){		/* 在TCP模式下,Socket0成功连接*/
						/* 自己添加处理代码 */
		}
		if(j&S_IR_DISCON){	/* 在TCP模式下Socket断开连接处理，自己添加代码 */
			ptr=(unsigned char*)W5100_S0_CR;		//关闭端口，等待重新打开连接
			*ptr=S_CR_CLOSE;
		}
		if(j&S_IR_SENDOK)
		{			/* Socket0数据发送完成，可以再次启动S_tx_process()函数发送数据 */
					/* 自己添加处理代码 */
		}
		if(j&S_IR_RECV)
		{			/* Socket接收到数据，可以启动S_rx_process()函数*/
					/* 自己添加处理代码 */
		}
		if(j&S_IR_TIMEOUT)
		{			/* Socket连接或数据传输超时处理*/
					/* 自己添加代码 */
		}
	}

	if((i&IR_S1_INT)==IR_S1_INT)		/*Socket1事件处理*/
	{
		ptr=(unsigned char*)W5100_S1_IR;
		j=*ptr;
		*ptr=j;			/*回写清中断标志，注意，有的编译器会优化掉该语句*/

		if(j&S_IR_CON)
		{				/* 在TCP模式下,Socket1成功连接*/
						/* 自己添加处理代码 */
		}
		if(j&S_IR_DISCON)
		{				/* 在TCP模式下Socket1断开连接处理，自己添加代码 */
			ptr=(unsigned char*)W5100_S1_CR;		//关闭端口，等待重新打开连接
			*ptr=S_CR_CLOSE;
		}
		if(j&S_IR_SENDOK)
		{				/* Socket1数据发送完成，可以再次启动S_tx_process()函数发送数据 */
						/* 自己添加处理代码 */
		}
		if(j&S_IR_RECV)
		{				/* Socket1接收到数据，可以启动S_rx_process()函数*/
						/* 自己添加处理代码 */
		}
		if(j&S_IR_TIMEOUT)
		{				/* Socket1连接或数据传输超时处理*/
						/* 自己添加代码 */
		}
	}

	if((i&IR_S2_INT)==IR_S2_INT)		/*Socket2事件处理*/
	{
		ptr=(unsigned char*)W5100_S2_IR;
		j=*ptr;
		*ptr=j;			/*回写清中断标志，注意，有的编译器会优化掉该语句*/

		if(j&S_IR_CON)
		{				/* 在TCP模式下,Socket2成功连接*/
						/* 自己添加处理代码 */
		}
		if(j&S_IR_DISCON)
		{				/* 在TCP模式下Socket2断开连接处理，自己添加代码 */
			ptr=(unsigned char*)W5100_S2_CR;		//关闭端口，等待重新打开连接
			*ptr=S_CR_CLOSE;
		}
		if(j&S_IR_SENDOK)
		{				/* Socket2数据发送完成，可以再次启动S_tx_process()函数发送数据 */
						/* 自己添加处理代码 */
		}
		if(j&S_IR_RECV)
		{				/* Socket2接收到数据，可以启动S_rx_process()函数*/
						/* 自己添加处理代码 */
		}
		if(j&S_IR_TIMEOUT)
		{				/* Socket2连接或数据传输超时处理*/
						/* 自己添加代码 */
		}
	}

	if((i&IR_S3_INT)==IR_S3_INT)		/*Socket3事件处理*/
	{
		ptr=(unsigned char*)W5100_S3_IR;
		i=*ptr;
		*ptr=j;			/*回写清中断标志，注意，有的编译器会优化掉该语句*/

		if(j&S_IR_CON)
		{				/* 在TCP模式下,Socket3成功连接*/
						/* 自己添加处理代码 */
		}
		if(j&S_IR_DISCON)
		{				/* 在TCP模式下Socket3断开连接处理，自己添加代码 */
			ptr=(unsigned char*)W5100_S3_CR;		//关闭端口，等待重新打开连接
			*ptr=S_CR_CLOSE;
		}
		if(j&S_IR_SENDOK)
		{				/* Socket3数据发送完成，可以再次启动S_tx_process()函数发送数据 */
						/* 自己添加处理代码 */
		}
		if(j&S_IR_RECV)
		{				/* Socket3接收到数据，可以启动S_rx_process()函数*/
						/* 自己添加处理代码 */
		}
		if(j&S_IR_TIMEOUT)
		{				/* Socket3连接或数据传输超时处理*/
						/* 自己添加代码 */
		}
	}
}
