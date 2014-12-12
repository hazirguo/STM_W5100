/********************************************************************************


    ��Ҫ����W5100.hͷ�ļ�

    �������ANSI C��д���ںܶ൥Ƭ����C�������϶�����ͨ����
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

#include"W5100.h"				//����W5100�ļĴ�����ַ��״̬

typedef  unsigned char SOCKET;

#define S_RX_SIZE	2048		//����Socket���ջ������Ĵ�С�����Ը���W5100_RMSR�������޸�
#define S_TX_SIZE	2048  		//����Socket���ͻ������Ĵ�С�����Ը���W5100_TMSR�������޸�

/*------------------------------------------------------------------------------
						W5100��ʼ������
��ʹ��W5100֮ǰ����W5100��ʼ��
------------------------------------------------------------------------------*/
void W5100_Init(void)
{
	unsigned char *ptr;
	unsigned char i;

	ptr=(unsigned char*)W5100_MODE;		/*��λW5100*/
	*ptr=MODE_RST;

	delay(100);						/*��ʱ100ms���Լ�����ú���*/

	/*��������(Gateway)��IP��ַ��4�ֽ�*/
	/*ʹ�����ؿ���ʹͨ��ͻ�������ľ��ޣ�ͨ�����ؿ��Է��ʵ��������������Internet*/
	ptr=(unsigned char*)W5100_GAR;
	for(i=0;i<4;i++)
	{
		*ptr=Gateway_IP[i];			/*Gateway_IPΪ4�ֽ�unsigned char����,�Լ�����*/
		ptr++;
	}

	/*������������(MASK)ֵ��4�ֽڡ���������������������*/
	ptr=(unsigned char*)W5100_SUBR;
	for(i=0;i<4;i++)
	{
		*ptr=SUB_MASK[i];				/*SUB_MASKΪ4�ֽ�unsigned char����,�Լ�����*/
		ptr++;
	}

	/*���������ַ��6�ֽڣ�����Ψһ��ʶ�����豸�������ֵַ
	�õ�ֵַ��Ҫ��IEEE���룬����OUI�Ĺ涨��ǰ3���ֽ�Ϊ���̴��룬�������ֽ�Ϊ��Ʒ���
	����Լ����������ַ��ע���һ���ֽڱ���Ϊż��*/
	ptr=(unsigned char*)W5100_SHAR;
	for(i=0;i<6;i++)
	{
		*ptr=PHY_ADDR[i];				/*PHY_ADDR6�ֽ�unsigned char����,�Լ�����*/
		ptr++;
	}

	/*���ñ�����IP��ַ��4���ֽ�
	ע�⣬����IP�����뱾��IP����ͬһ�����������򱾻����޷��ҵ�����*/
	ptr=(unsigned char*)W5100_SIPR;		/*Set source IP address*/
	for(i=0;i<4;i++)
	{
		*ptr=IP_ADDR[i];				/*IP_ADDRΪ4�ֽ�unsigned char����,�Լ�����*/
		ptr++;
	}

	/*���÷��ͻ������ͽ��ջ������Ĵ�С���ο�W5100�����ֲ�*/
	ptr=(unsigned char*)W5100_RMSR;		/*Socket Rx memory size=2k*/
	*ptr=0x55;

	ptr=(unsigned char*)W5100_TMSR;		/*Socket Tx mempry size=2k*/
	*ptr=0x55;

	/* �����жϣ��ο�W5100�����ֲ�ȷ���Լ���Ҫ���ж�����
	IMR_CONFLICT��IP��ַ��ͻ�쳣�ж�
	IMR_UNREACH��UDPͨ��ʱ����ַ�޷�������쳣�ж�
	������Socket�¼��жϣ�������Ҫ��� */
	ptr=(unsigned char*)W5100_IMR;
	*ptr=(IMR_CONFLICT|IMR_UNREACH|IMR_S0_INT|IMR_S1_INT|IMR_S2_INT|IMR_S3_INT);
}

/******************************************************************************
//                           Socket����, ����3��Socket�Ĵ���ɲ��մ˳���
//*****************************************************************************

						Socket��ʼ��
����ɹ��򷵻�true, ���򷵻�false
-----------------------------------------------------------------------------*/
unsigned char Socket_Init(SOCKET s)
{
	unsigned char *ptr;
	unsigned char i;

	ptr=(unsigned char*)(W5100_S0_MR+s*0x100);
	*ptr=S_MR_TCP;						/*����socket0ΪTCPģʽ*/

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);
	*ptr=S_CR_OPEN;						/*��socket0*/

	ptr=(unsigned char*)(W5100_S0_SSR+s*0x100);
	if(*ptr!=S_SSR_INIT)
	{
		ptr=(unsigned char*)(W5100_S0_CR+s*0x100);	/*�򿪲��ɹ����ر�Socket��Ȼ�󷵻�*/
		*ptr=S_CR_CLOSE;
		return false;
	}

	/*������ؼ���ȡ���ص������ַ*/
	ptr=(unsigned char*)(W5100_S0_DIPR+s*0x100);
	for(i=0;i<4;i++)
	{
		*ptr=IP_ADDR[i]+1;				/*��Ŀ�ĵ�ַ�Ĵ���д���뱾��IP��ͬ��IPֵ*/
		ptr++;
	}

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);
	*ptr=S_CR_CONNECT;					/*��socket0��TCP����*/

	delay(20);							/*��ʱ20ms���Լ�����ú�������ʱ�䳤�̿��Ը�����Ҫ����*/

	ptr=(unsigned char*)(W5100_S0_DHAR+s*0x100);	/*��ȡĿ�������������ַ���õ�ַ�������ص�ַ*/
	for(i=0;i<6;i++)
	{
		GATEWAY_PHY[i]=*ptr;				/*GATEWAY_PHYΪ6�ֽ�unsigned char����,�Լ�����*/
		ptr++;
	}

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);
	*ptr=S_CR_CLOSE;					/*�ر�socket0*/

	if(GATEWAY_PHY[0]==0xff)
	{
		/**********û���ҵ����ط���������û�������ط������ɹ�����***********/
		/**********              �Լ���Ӵ������                ***********/
	}

	/*���÷�Ƭ���ȣ��ο�W5100�����ֲᣬ��ֵ���Բ��޸�*/
	ptr=(unsigned char*)(W5100_S0_MSS+s*0x100);		/*����Ƭ�ֽ���=1460*/
	*ptr=0x05;
	ptr++;
	*ptr=0xb4;

	return true;
}
/*-----------------------------------------------------------------------------
                           ����SocketΪ�ͻ�����Զ�̷���������
������Socket�����ڿͻ���ģʽʱ�����øó�����Զ�̷�������������
������óɹ��򷵻�true�����򷵻�false
����������Ӻ���ֳ�ʱ�жϣ��������������ʧ�ܣ���Ҫ���µ��øó�������
�ó���ÿ����һ�Σ��������������һ������
------------------------------------------------------------------------------*/
unsigned char Socket_Connect(SOCKET s)
{
	unsigned char i,*ptr;

	ptr=(unsigned char*)(W5100_S0_MR+s*0x100);		/*����SocketΪUDPģʽ*/
	*ptr=S_MR_TCP;

	ptr=(unsigned char*)(W5100_S0_PORT+s*0x100);		/*���ñ���source�Ķ˿ں�*/
	*ptr=PORT/256;							/*PORTΪunisgned int�ͣ��Լ�����*/
	ptr++;
	*ptr=PORT;

	ptr=(unsigned char*)(W5100_S0_DIPR+s*0x100);		/*����Զ������IP��ַ������������IP��ַ*/
	for(i=0;i<4;i++)
	{
		*ptr=D_IP_ADDR[i];					/*D_IP_ADDRΪ4�ֽ�unsigned char���飬�Լ�����*/
		ptr++;
	}

	ptr=(unsigned char*)(W5100_S0_DPORT+s*0x100);		/*Socket��Ŀ�Ķ˿ں�*/
	*ptr=DPORT/256;							/*DPORTΪunisgned int�ͣ��Լ�����*/
	ptr++;
	*ptr=DPORT;

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*��Socket*/
	*ptr=S_CR_OPEN;

	ptr=(unsigned char*)(W5100_S0_SSR+s*0x100);
	if(*ptr!=S_SSR_INIT)
	{
		ptr=(unsigned char*)(W5100_S0_CR+s*0x100);	/*�򿪲��ɹ����ر�Socket��Ȼ�󷵻�*/
		*ptr=S_CR_CLOSE;
		return false;
	}

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*����SocketΪConnectģʽ*/
	*ptr=S_CR_CONNECT;
	return true;

	/*���������Socket�Ĵ����ӹ������������Ƿ���Զ�̷������������ӣ�����Ҫ�ȴ�Socket�жϣ�
	���ж�Socket�������Ƿ�ɹ����ο�W5100�����ֲ��Socket�ж�״̬*/
}

/*-----------------------------------------------------------------------------
                           ����Socket��Ϊ�������ȴ�Զ������������
������Socket�����ڷ�����ģʽʱ�����øó��򣬵ȵ�Զ������������
������óɹ��򷵻�true, ���򷵻�false
�ó���ֻ����һ�Σ���ʹW5100����Ϊ������ģʽ
-----------------------------------------------------------------------------*/
unsigned char Socket_Listen(SOCKET s)
{
	unsigned char *ptr;

	ptr=(unsigned char*)(W5100_S0_MR+s*0x100);		/*����SocketΪUDPģʽ*/
	*ptr=S_MR_TCP;

	ptr=(unsigned char*)(W5100_S0_PORT+s*0x100);		/*���ñ���source�Ķ˿ں�*/
	*ptr=PORT/256;							/*PORTΪunisgned int�ͣ��Լ����壬��ǰ�涨�����ͬ*/
	ptr++;
	*ptr=PORT;

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*��Socket*/
	*ptr=S_CR_OPEN;

	ptr=(unsigned char*)(W5100_S0_SSR+s*0x100);
	if(*ptr!=S_SSR_INIT)
	{
		ptr=(unsigned char*)(W5100_S0_CR+s*0x100);	/*�򿪲��ɹ����ر�Socket��Ȼ�󷵻�*/
		*ptr=S_CR_CLOSE;
		return false;
	}

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*����SocketΪ����ģʽ*/
	*ptr=S_CR_LISTEN;

	ptr=(unsigned char*)(W5100_S0_SSR+s*0x100);
	if(*ptr!=S_SSR_LISTEN)
	{
		ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*���ò��ɹ����ر�Socket��Ȼ�󷵻�*/
		*ptr=S_CR_CLOSE;
		return false;
	}

	return true;

	/*���������Socket�Ĵ򿪺�������������������Զ�̿ͻ����Ƿ������������ӣ�����Ҫ�ȴ�Socket�жϣ�
	���ж�Socket�������Ƿ�ɹ����ο�W5100�����ֲ��Socket�ж�״̬
	�ڷ���������ģʽ����Ҫ����Ŀ��IP��Ŀ�Ķ˿ں�*/
}

/*-----------------------------------------------------------------------------
					����SocketΪUDPģʽ
���Socket������UDPģʽ�����øó�����UDPģʽ�£�Socketͨ�Ų���Ҫ��������
������óɹ��򷵻�true, ���򷵻�false
�ó���ֻ����һ�Σ���ʹW5100����ΪUDPģʽ
-----------------------------------------------------------------------------*/
unsigned char Socket_UDP(SOCKET s)
{
	unsigned char *ptr;

	ptr=(unsigned char*)(W5100_S0_MR+s*0x100);		/*����SocketΪUDPģʽ*/
	*ptr=S_MR_UDP;

	ptr=(unsigned char*)(W5100_S0_PORT+s*0x100);		/*���ñ���source�Ķ˿ں�*/
	*ptr=PORT/256;							/*PORTΪunisgned int�ͣ��Լ����壬��ǰ�涨�����ͬ*/
	ptr++;
	*ptr=PORT;

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);		/*��Socket*/
	*ptr=S_CR_OPEN;

	ptr=(unsigned char*)(W5100_S0_SSR+s*0x100);
	if(*ptr!=S_SSR_UDP)
	{
		ptr=(unsigned char*)(W5100_S0_CR+s*0x100);	/*�򿪲��ɹ����ر�Socket��Ȼ�󷵻�*/
		*ptr=S_CR_CLOSE;
		return false;
	}
	else
		return true;

	/*���������Socket�Ĵ򿪺�UDPģʽ���ã�������ģʽ��������Ҫ��Զ��������������
	��ΪSocket����Ҫ�������ӣ������ڷ�������ǰ����������Ŀ������IP��Ŀ��Socket�Ķ˿ں�
	���Ŀ������IP��Ŀ��Socket�Ķ˿ں��ǹ̶��ģ������й�����û�иı䣬��ôҲ��������������*/
}


/******************************************************************************
                              ����Socket���պͷ��͵�����
******************************************************************************/
/*-----------------------------------------------------------------------------
���Socket�����������ݵ��жϣ������øó�����д���
�ó���Socket�Ľ��յ������ݻ��浽Rx_buffer�����У������ؽ��յ������ֽ���
-----------------------------------------------------------------------------*/
unsigned int S_rx_process(SOCKET s)
{
	unsigned char *ptr;
	unsigned int i, rx_size, rx_offset, rx_offset1;

	ptr=(unsigned char*)(W5100_S0_RX_RSR+s*0x100);		/*��ȡ�������ݵ��ֽ���*/
	rx_size=*ptr;
	ptr++;
	rx_size*=256;
	rx_size+=*ptr;

	ptr=(unsigned char*)(W5100_S0_RX_RR+s*0x100);			/*��ȡ���ջ�������ƫ����*/
	rx_offset=*ptr;
	ptr++;
	rx_offset*=256;
	rx_offset+=*ptr;
	rx_offset1=rx_offset;
	
	i=rx_offset/S_RX_SIZE;							/*����ʵ�ʵ�����ƫ������S0_RX_SIZE��Ҫ��ǰ��#define�ж���*/
											/*ע��S0_RX_SIZE��ֵ��W5100_Init()������W5100_RMSR��ȷ��*/
	rx_offset=rx_offset-i*S_RX_SIZE;

	ptr=(unsigned char*)(W5100_RX+s*S_RX_SIZE+rx_offset);		/*ʵ�������ַΪW5100_RX+rx_offset*/
	for(i=0;i<rx_size;i++)
	{
		if(rx_offset>=S_RX_SIZE)
		{
			ptr=(unsigned char*)(W5100_RX+s*S_RX_SIZE);
			rx_offset=0;
		}
		Rx_buffer[i]=*ptr;						/*�����ݻ��浽Rx_buffer������*/
		ptr++;
		rx_offset++;
	}

	i=rx_offset1+rx_size;
	ptr=(unsigned char*)(W5100_S0_RX_RR+s*0x100);
	*ptr=(i/256);
	ptr++;
	*ptr=i;

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);			/*����RECV����ȵ���һ�ν���*/
	*ptr=S_CR_RECV;

	return rx_size;								/*���ؽ��յ������ֽ���*/
}

/*-----------------------------------------------------------------------------
���Ҫͨ��Socket�������ݣ������øó���
Ҫ���͵����ݻ�����Tx_buffer��, size����Ҫ���͵��ֽڳ���
-----------------------------------------------------------------------------*/
unsigned char S_tx_process(SOCKET s, unsigned int size)
{
	unsigned char *ptr;
	unsigned int i, tx_offset, tx_offset1;

	ptr=(unsigned char*)(W5100_S0_MR+s*0x100);		/*�����UDPģʽ,�����ڴ�����Ŀ��������IP�Ͷ˿ں�*/
	if(((*ptr)&0x0f)==0x02)
	{
		ptr=(unsigned char*)(W5100_S0_DIPR+s*0x100);
		for(i=0;i<4;i++){						/*����Ŀ������IP*/
			*ptr=UDP_DIPR[i];					/*UDP_DIPRΪ4�ֽ�unsigned char���飬�Լ�����*/
			ptr++;
		}
		ptr=(unsigned char*)(W5100_S0_DPORT+s*0x100);
		*ptr=UDP_DPORT/256;						/*UDP_DIPRΪunsigned int��, �Լ�����*/
		ptr++;
		*ptr=UDP_DPORT;
	}

	ptr=(unsigned char*)(W5100_S0_TX_FSR+s*0x100);		/*��ȡ������ʣ��ĳ���*/
	i=*ptr;
	i*=256;
	ptr++;
	i+=(*ptr);
	if(i<size)						/*���ʣ����ֽڳ���С�ڷ����ֽڳ���,�򷵻�*/
		return false;

	ptr=(unsigned char*)(W5100_S0_TX_WR+s*0x100);			/*��ȡ���ͻ�������ƫ����*/
	tx_offset=*ptr;
	tx_offset*=256;
	ptr++;
	tx_offset+=(*ptr);
	tx_offset1=tx_offset;
	
	i=tx_offset/S_TX_SIZE;					/*����ʵ�ʵ�����ƫ������S0_TX_SIZE��Ҫ��ǰ��#define�ж���*/
											/*ע��S0_TX_SIZE��ֵ��W5100_Init()������W5100_TMSR��ȷ��*/
	tx_offset=tx_offset-i*S_TX_SIZE;

	ptr=(unsigned char*)(W5100_TX+s*S_TX_SIZE+tx_offset);		/*ʵ�������ַΪW5100_TX+tx_offset*/
	for(i=0;i<size;i++)
	{
		if(tx_offset>=S_TX_SIZE)
		{
			ptr=(unsigned char*)(W5100_TX+s*S_TX_SIZE);
			tx_offset=0;
		}
		*ptr=Tx_buffer[i];						/*��Tx_buffer�������е�����д�뵽���ͻ�����*/
		ptr++;
		tx_offset++;
	}

	i=tx_offset1+size;
	ptr=(unsigned char*)(W5100_S0_TX_WR+s*0x100);
	*ptr=(i/256);
	ptr++;
	*ptr=i;

	ptr=(unsigned char*)(W5100_S0_CR+s*0x100);	/*����SEND����,��������*/
	*ptr=S_CR_SEND;

	return true;								/*���سɹ�*/
}


/******************************************************************************
					W5100�жϴ��������
******************************************************************************/
void W5100_interrupt_handler(void)
{
	unsigned char *ptr;
	unsigned char i,j;

	ptr=(unsigned char*)W5100_IR;
	i=*ptr;
	*ptr=(i&0xf0);					/*��д����жϱ�־*/

	if((i&IR_CONFLICT)==IR_CONFLICT)
	{							/*IP��ַ��ͻ�쳣�����Լ���Ӵ���*/

	}

	if((i&IR_UNREACH)==IR_UNREACH)
	{							/*UDPģʽ�µ�ַ�޷������쳣�����Լ���Ӵ���*/

	}

	if((i&IR_S0_INT)==IR_S0_INT)		/*Socket�¼�����*/
	{
		ptr=(unsigned char*)W5100_S0_IR;
		j=*ptr;
		*ptr=j;				/*��д���жϱ�־��ע�⣬�еı��������Ż��������*/

		if(j&S_IR_CON){		/* ��TCPģʽ��,Socket0�ɹ�����*/
						/* �Լ���Ӵ������ */
		}
		if(j&S_IR_DISCON){	/* ��TCPģʽ��Socket�Ͽ����Ӵ����Լ���Ӵ��� */
			ptr=(unsigned char*)W5100_S0_CR;		//�رն˿ڣ��ȴ����´�����
			*ptr=S_CR_CLOSE;
		}
		if(j&S_IR_SENDOK)
		{			/* Socket0���ݷ�����ɣ������ٴ�����S_tx_process()������������ */
					/* �Լ���Ӵ������ */
		}
		if(j&S_IR_RECV)
		{			/* Socket���յ����ݣ���������S_rx_process()����*/
					/* �Լ���Ӵ������ */
		}
		if(j&S_IR_TIMEOUT)
		{			/* Socket���ӻ����ݴ��䳬ʱ����*/
					/* �Լ���Ӵ��� */
		}
	}

	if((i&IR_S1_INT)==IR_S1_INT)		/*Socket1�¼�����*/
	{
		ptr=(unsigned char*)W5100_S1_IR;
		j=*ptr;
		*ptr=j;			/*��д���жϱ�־��ע�⣬�еı��������Ż��������*/

		if(j&S_IR_CON)
		{				/* ��TCPģʽ��,Socket1�ɹ�����*/
						/* �Լ���Ӵ������ */
		}
		if(j&S_IR_DISCON)
		{				/* ��TCPģʽ��Socket1�Ͽ����Ӵ����Լ���Ӵ��� */
			ptr=(unsigned char*)W5100_S1_CR;		//�رն˿ڣ��ȴ����´�����
			*ptr=S_CR_CLOSE;
		}
		if(j&S_IR_SENDOK)
		{				/* Socket1���ݷ�����ɣ������ٴ�����S_tx_process()������������ */
						/* �Լ���Ӵ������ */
		}
		if(j&S_IR_RECV)
		{				/* Socket1���յ����ݣ���������S_rx_process()����*/
						/* �Լ���Ӵ������ */
		}
		if(j&S_IR_TIMEOUT)
		{				/* Socket1���ӻ����ݴ��䳬ʱ����*/
						/* �Լ���Ӵ��� */
		}
	}

	if((i&IR_S2_INT)==IR_S2_INT)		/*Socket2�¼�����*/
	{
		ptr=(unsigned char*)W5100_S2_IR;
		j=*ptr;
		*ptr=j;			/*��д���жϱ�־��ע�⣬�еı��������Ż��������*/

		if(j&S_IR_CON)
		{				/* ��TCPģʽ��,Socket2�ɹ�����*/
						/* �Լ���Ӵ������ */
		}
		if(j&S_IR_DISCON)
		{				/* ��TCPģʽ��Socket2�Ͽ����Ӵ����Լ���Ӵ��� */
			ptr=(unsigned char*)W5100_S2_CR;		//�رն˿ڣ��ȴ����´�����
			*ptr=S_CR_CLOSE;
		}
		if(j&S_IR_SENDOK)
		{				/* Socket2���ݷ�����ɣ������ٴ�����S_tx_process()������������ */
						/* �Լ���Ӵ������ */
		}
		if(j&S_IR_RECV)
		{				/* Socket2���յ����ݣ���������S_rx_process()����*/
						/* �Լ���Ӵ������ */
		}
		if(j&S_IR_TIMEOUT)
		{				/* Socket2���ӻ����ݴ��䳬ʱ����*/
						/* �Լ���Ӵ��� */
		}
	}

	if((i&IR_S3_INT)==IR_S3_INT)		/*Socket3�¼�����*/
	{
		ptr=(unsigned char*)W5100_S3_IR;
		i=*ptr;
		*ptr=j;			/*��д���жϱ�־��ע�⣬�еı��������Ż��������*/

		if(j&S_IR_CON)
		{				/* ��TCPģʽ��,Socket3�ɹ�����*/
						/* �Լ���Ӵ������ */
		}
		if(j&S_IR_DISCON)
		{				/* ��TCPģʽ��Socket3�Ͽ����Ӵ����Լ���Ӵ��� */
			ptr=(unsigned char*)W5100_S3_CR;		//�رն˿ڣ��ȴ����´�����
			*ptr=S_CR_CLOSE;
		}
		if(j&S_IR_SENDOK)
		{				/* Socket3���ݷ�����ɣ������ٴ�����S_tx_process()������������ */
						/* �Լ���Ӵ������ */
		}
		if(j&S_IR_RECV)
		{				/* Socket3���յ����ݣ���������S_rx_process()����*/
						/* �Լ���Ӵ������ */
		}
		if(j&S_IR_TIMEOUT)
		{				/* Socket3���ӻ����ݴ��䳬ʱ����*/
						/* �Լ���Ӵ��� */
		}
	}
}
