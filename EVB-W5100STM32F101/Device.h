#define TRUE	0xff
#define FALSE	0x00

typedef  unsigned char SOCKET;

/* 24LC01 EEPROM地址 */
#define EEPROM_ADDRESS  0xa0

/* Timer2定时器计数 */
unsigned int Timer2_Counter;

/* ADC寄存器定义 */						 
unsigned char ADC_state;
signed short Temperature, temperature[8];	/* temperature为8次温度采集的AD值， Temperature为温度计算的结果值 */
signed short Vref, vref[8];					/* vref为8次参考电压的采集值， Vref为参考电压的计算结果，该结果用于计算温度 */
unsigned short Pot, pot[8];					/* pot为8次电位器电压的采集值，Pot为电压采集的计算结果 */
unsigned char ADC_Complete;					/* 完成一次AD转换，ADC_Complete置1，处理完AD转换的数据，ADC_Complete清0
											重新启动新的一次转换 */

unsigned char Temp_Buffer[128];

/* UART1数据缓冲区 */
unsigned char UART_Rx_Buffer[128];			/* UART1接收数据缓冲区 */
unsigned char UART_Tx_Buffer[128];			/* UART1发送数据缓冲区 */
unsigned short RxCounter;					/* 接收数据字节数的计数 */
unsigned short TxCounter, TxIndex;			/* 发送数据字节数的计数和发送字节索引 */
unsigned char UART_DataReceive;				/* 接收到一个完整的数据包，该寄存器置1，处理完数据后该寄存器清0 */

/* 端口数据缓冲区 */
unsigned char Rx_Buffer[2000];				/* 端口接收数据缓冲区 */
unsigned char Tx_Buffer[2000];				/* 端口发送数据缓冲区 */

/* Network parameter registers */
unsigned char Gateway_IP[4];			/* 网关IP地址 */
unsigned char Sub_Mask[4];				/* 子网掩码 */
unsigned char Phy_Addr[6];  			/* 物理地址 */
unsigned char IP_Addr[4];				/* 本机IP地址 */

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
extern unsigned char Socket_UDP(SOCKET s);
extern unsigned short S_rx_process(SOCKET s);
extern unsigned char S_tx_process(SOCKET s, unsigned int size);
extern void W5100_Interrupt_Process(void);
