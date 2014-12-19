#define TRUE	0xff
#define FALSE	0x00

typedef  unsigned char SOCKET;

/* Timer2定时器计数 */
unsigned int Timer2_Counter;

unsigned char Temp_Buffer[128];

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
extern unsigned short S_rx_process(SOCKET s);
extern unsigned char S_tx_process(SOCKET s, unsigned int size);
extern void W5100_Interrupt_Process(void);
