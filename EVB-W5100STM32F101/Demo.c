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
	/* �������ز��� */
	Gateway_IP[0] = GATEWAY_IP_1;
	Gateway_IP[1] = GATEWAY_IP_2;
	Gateway_IP[2] = GATEWAY_IP_3;
	Gateway_IP[3] = GATEWAY_IP_4;

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
	IP_Addr[0] = IP_ADDR_1;
	IP_Addr[1] = IP_ADDR_2;
	IP_Addr[2] = IP_ADDR_3;
	IP_Addr[3] = IP_ADDR_4;

	/* ���ض˿�0�Ķ˿ں�5000 */
	S0_Port[0] = S0_PORT_1;  
	S0_Port[1] = S0_PORT_2; 

	/* ���ض˿�0�Ĺ���ģʽ */
	S0_Mode = S0_MODE; 
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

	/* �˿�0 */
	Socket_Init(0);

	GPIO_ResetBits(GPIOB, LED_DRIVE); 		/*  ����LEDָʾ��  */
}

/*****************************************************************
������: W5100_Socket_Set
����: ��
���: �˿�״̬Socket_State
����: ��
˵�����ֱ�����4���˿ڣ����ݶ˿ڹ���ģʽ�����˿�����TCP��������TCP�ͻ���
      ��UDPģʽ��
      �Ӷ˿�״̬�ֽ�Socket_State�����ж϶˿ڵĹ������
*****************************************************************/
void W5100_Socket_Set(void)
{
	/* �˿� 0 */
	if(S0_State==0)
	{
		if(S0_Mode==TCP_SERVER)			/* TCP������ģʽ */
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
	}
}

/*****************************************************************
������: Rx_Data_process
����: �����ֽڳ���
���: ���ص����ݣ��洢��Temp_Buffer��
����:
˵�������ݰ��Ľṹ����:
		| 0xaa | 0x55 | Length | Command | Object | Data |
	Length(1�ֽ�): ���ݰ��ֽڳ��ȣ����������ݰ�ͷ�ͱ����ֽ�
	Command(1�ֽ�)��Ϊ0ʱ��ȡ�������ݣ�Ϊ1ʱ���ö������
	Object(1�ֽ�):	0x00: ����IP
				   	0x01: ��������
				   	0x02: �����ַ
				   	0x03: ����IP��ַ
				   	0x10/0x20/0x30/0x40: �ֱ�Ϊ4���˿ڵĶ˿ں�
				   	0x11/0x21/0x31/0x41: �ֱ�Ϊ4���˿ڵ�Ŀ��IP��ַ(ֻ����TCP�ͻ��˺�UDPģʽ����Ч)
					0x12/0x22/0x32/0x42: �ֱ�Ϊ4���˿ڵ�Ŀ�Ķ˿ں�(ֻ����TCP�ͻ��˺�UDPģʽ����Ч)
					0x12/0x22/0x32/0x42: �ֱ�Ϊ4���˿ڵĹ���ģʽ
					0x80: �¶�(ֻ��)
					0x81: ģ���ѹ�ź�(ֻ��)
					0x82: LED��״̬
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

	if((Temp_Buffer[0]!=0xaa)||(Temp_Buffer[1]!=0x55))	/* ���ݰ�ͷ���� */
		return;

	i=l-3;
	if(i!=Temp_Buffer[2])				/* ���ݰ��ֽڳ��ȴ��� */
		return;

	i=Temp_Buffer[4];					/* ָ����� */

	if(Temp_Buffer[3])
	{									/* ���ö������ */
		switch(i)
		{
			case 0x82:
				if(Temp_Buffer[2]!=3)
					error_process();
				else
				{
					if(Temp_Buffer[5]==1)
					{
						GPIO_SetBits(GPIOB, LED_DRIVE); 		/* ����LEDָʾ�� */

						set_data_ok(1);
					}
					else if(Temp_Buffer[5]==0)
					{
						GPIO_ResetBits(GPIOB, LED_DRIVE); 		/* �ر�LEDָʾ�� */
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
	else				/* ��ȡ������� */
	{
		if(Temp_Buffer[2]!=2)
			error_process();
		else
		{
			Temp_Buffer[2]=6;
			switch(i)
			{
				case 0x82:			/* ��ȡLED��״̬ */
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

	size = S_rx_process(s);
	memcpy(Tx_Buffer, Rx_Buffer, size);

//	Rx_Data_Process(size);

//	size = Temp_Buffer[2]+3;
//	memcpy(Tx_Buffer, Temp_Buffer, size);

	S_tx_process(s, size);
}


/*****************************************************************
                           ������
*****************************************************************/
int main(void)
{
	/* ��ʼ��STM32F101 */
	System_Initialization();

	/* ����Ƿ����Ĭ�ϲ�������״̬��Ȼ���24LC01 EEPROM�л�ȡ������� */
	Load_Net_Parameters();

	/* ��ʼ��W5100 */
	W5100_Initialization();

	do
	{
		/* ����W5100�˿� */
		W5100_Socket_Set();

		/* ����W5100�ж� */
		if(W5100_Interrupt)
			W5100_Interrupt_Process();

		/* ���Socket0���յ����� */
		if((S0_Data & S_RECEIVE) == S_RECEIVE)
		{
			S0_Data &= ~S_RECEIVE;
			Process_Socket_Data(0);
		}
	}while(1);
}
