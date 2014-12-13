/*********************************************************************************
	�ṩ�̣��ɶ���Ȼ����
	��  ַ��http://www.hschip.com
	ʱ  ��: 2007-11-30
*********************************************************************************/
#include <stm32f10x_lib.h>              /* STM32F10x�ⶨ�� */
#include <string.h>

#include "Device.h"
#include "Net_Parameter.h"				/* 24LC01 EEPROM�洢������ͨ�Ų������� */
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
������: I2C_Write
����: ��ֵַ�������ֽ�
���: ��
����:
˵������һ���ֽڵ�����д��24LC01B EEPROM��ָ���ĵ�ַ
**********************************************************************/
void I2C_Write(unsigned char addr, unsigned char dat)
{
	I2C_Cmd(I2C1, ENABLE);

	/* ����I2C��START�ź� */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* ���I2C��EV5״̬����� */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* ����24LC01B���豸��ַ */
	I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);

	/* ���I2C��EV6״̬����� */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* ����д�뵽24LC01B��ַ */
	I2C_SendData(I2C1, addr);

	/* ���I2C��EV8״̬����� */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* �������� */
	I2C_SendData(I2C1, dat);

	/* ���I2C��EV8״̬����� */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* ����I2C��STOP�ź� */
	I2C_GenerateSTOP(I2C1, ENABLE);

	Delay(10);
}

/**********************************************************************
������: I2C_Read
����: ��ֵַ
���: ��
����: �����ֽ�
˵������24LC01 EEPROMָ���ĵ�ַ��ȡһ���ֽڵ�����
**********************************************************************/
unsigned char I2C_Read(unsigned char addr)
{
	unsigned char i;

	I2C_Cmd(I2C1, ENABLE);

	/* ����I2C��START�ź� */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* ���I2C��EV5״̬����� */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* ����24LC01B�ĵ�ַ */
      I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);
	/* ���I2C��EV6״̬����� */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* ���EV6״̬ */
	I2C_Cmd(I2C1, ENABLE);

	/* ���Ͷ�ȡ���ݵĵ�ַ */
	I2C_SendData(I2C1, addr);
	/* ���I2C��EV8״̬����� */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* ����I2C��START�ź� */
	I2C_GenerateSTART(I2C1, ENABLE);
	/* ���I2C��EV5״̬����� */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* ����I2C������ */
      I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Receiver);
	/* ���I2C��EV6״̬����� */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

	/* ���I2C��EV7����� */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));

	/* ����I2C��STOP�ź� */
	I2C_GenerateSTOP(I2C1, ENABLE);

	/* ��ȡ���� */
	i = I2C_ReceiveData(I2C1);

	I2C_Cmd(I2C1, DISABLE);
	Delay(2);
	return i;
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
	unsigned short i;

	/* ���Jumper 1����, ���J1��·������EVB���������ΪĬ��ֵ */
//	if(GPIO_ReadInputDataBit(GPIOC, DIG_IN)==0)
//	{
		/* ����Ĭ�ϵ�����Ϊ192.168.0.1 */
//		I2C_Write(GATEWAY_IP_EE,192);
//		I2C_Write(GATEWAY_IP_EE+1,168);
//		I2C_Write(GATEWAY_IP_EE+2,0);
//		I2C_Write(GATEWAY_IP_EE+3,1);

		/* ����Ĭ�ϵ���������Ϊ255.255.255.0 */
//		I2C_Write(SUBNET_MASK_EE,255);
//		I2C_Write(SUBNET_MASK_EE+1,255);
//		I2C_Write(SUBNET_MASK_EE+2,255);
//		I2C_Write(SUBNET_MASK_EE+3,0);
  
		/* ����Ĭ�ϵ������ַΪ 00.08.DC.01.02.03 */
//		I2C_Write(PHY_ADDR_EE,0x00);
//		I2C_Write(PHY_ADDR_EE+1,0x08);
//		I2C_Write(PHY_ADDR_EE+2,0xdc);
//		I2C_Write(PHY_ADDR_EE+3,0x01);
//		I2C_Write(PHY_ADDR_EE+4,0x02);
//		I2C_Write(PHY_ADDR_EE+5,0x03);
  
 		/* ���ñ���Ĭ�ϵ�IP��ַΪ192.168.0.2 */
//		I2C_Write(IP_ADDR_EE,192);
//		I2C_Write(IP_ADDR_EE+1,168);
//		I2C_Write(IP_ADDR_EE+2,0);
//		I2C_Write(IP_ADDR_EE+3,2);
  
		/* ���ö˿�0Ĭ�ϵĶ˿ں�Ϊ50000 */
//		I2C_Write(S0_PORT_EE,0xc3);
//		I2C_Write(S0_PORT_EE+1,0x50);

		/* ���ö˿�0Ĭ�ϵĹ�����ʽΪTCP������ */
//		I2C_Write(S0_MODE_EE, 0x00);

		/* ���ö˿�1Ĭ�ϵĶ˿ں�Ϊ51000 */
//		I2C_Write(S1_PORT_EE,0xc7);
//		I2C_Write(S1_PORT_EE+1,0x38);

		/* ���ö˿�1Ĭ�ϵĹ�����ʽΪTCP������ */
//		I2C_Write(S1_MODE_EE, 0x00);

		/* ���ö˿�2Ĭ�ϵĶ˿ں�Ϊ52000 */
//		I2C_Write(S2_PORT_EE,0xcb);
//		I2C_Write(S2_PORT_EE+1,0x20);

		/* ���ö˿�2Ĭ�ϵĹ�����ʽΪTCP������ */
//		I2C_Write(S2_MODE_EE, 0x00);

		/* ���ö˿�3Ĭ�ϵĶ˿ں�Ϊ53000 */
//		I2C_Write(S3_PORT_EE,0xcf);
//		I2C_Write(S3_PORT_EE+1,0x08);

		/* ���ö˿�3Ĭ�ϵĹ�����ʽΪTCP������ */
//		I2C_Write(S3_MODE_EE, 0x00);
//	}
	/* �������ز��� */
//	for(i=0; i<4; i++)
//		Gateway_IP[i] = I2C_Read(GATEWAY_IP_EE+i);
	Gateway_IP[0] = 192;
	Gateway_IP[1] = 168;
	Gateway_IP[2] = 0;
	Gateway_IP[3] = 1;

	/* ������������ */
//	for(i=0; i<4; i++)
//		Sub_Mask[i] = I2C_Read(SUBNET_MASK_EE+i);
	Sub_Mask[0]=255;
	Sub_Mask[1]=255;
	Sub_Mask[2]=255;
	Sub_Mask[3]=0;

	/* ���������ַ */
//	for(i=0; i<6; i++)
//		Phy_Addr[i]=I2C_Read(PHY_ADDR_EE+i);
	Phy_Addr[0]=0x0c;
	Phy_Addr[1]=0x29;
	Phy_Addr[2]=0xab;
	Phy_Addr[3]=0x7c;
	Phy_Addr[4]=0x00;
	Phy_Addr[5]=0x01;

	/* ����IP��ַ */
//	for(i=0; i<4; i++)
//		IP_Addr[i]=I2C_Read(IP_ADDR_EE+i);
	IP_Addr[0]=192;
	IP_Addr[1]=168;
	IP_Addr[2]=0;
	IP_Addr[3]=20;

	/* ���ض˿�0�Ķ˿ں�5000 */
	S0_Port[0] = 0x13; //I2C_Read(S0_PORT_EE);
	S0_Port[1] = 0x88;//I2C_Read(S0_PORT_EE+1);

	/* ���ض˿�0��Ŀ��IP��ַ */
//	for(i=0; i<4; i++)
//		S0_DIP[i]=I2C_Read(S0_DIP_EE+i);
	S0_DIP[0]=192;
	S0_DIP[1]=168;
	S0_DIP[2]=0;
	S0_DIP[3]=30;
	
	/* ���ض˿�0��Ŀ�Ķ˿ں�6000 */
	S0_DPort[0] = 0x17;//I2C_Read(S0_DPORT_EE);
	S0_DPort[1] = 0x70;//I2C_Read(S0_DPORT_EE+1);
	/* ���ض˿�0�Ĺ���ģʽ */
	S0_Mode=1;//I2C_Read(S0_MODE_EE);

	/* ���ض˿�1�Ķ˿ں� */
	S1_Port[0] = I2C_Read(S1_PORT_EE);
	S1_Port[1] = I2C_Read(S1_PORT_EE+1);
	/* ���ض˿�1��Ŀ��IP��ַ */
	for(i=0; i<4; i++)
		S1_DIP[i]=I2C_Read(S1_DIP_EE+i);
	/* ���ض˿�1��Ŀ�Ķ˿ں� */
	S1_DPort[0] = I2C_Read(S1_DPORT_EE);
	S1_DPort[1] = I2C_Read(S1_DPORT_EE+1);
	/* ���ض˿�1�Ĺ���ģʽ */
	S1_Mode=I2C_Read(S1_MODE_EE);

	/* ���ض˿�2�Ķ˿ں� */
	S2_Port[0] = I2C_Read(S2_PORT_EE);
	S2_Port[1] = I2C_Read(S2_PORT_EE+1);
	/* ���ض˿�2��Ŀ��IP��ַ */
	for(i=0; i<4; i++)
		S2_DIP[i]=I2C_Read(S2_DIP_EE+i);
	/* ���ض˿�2��Ŀ�Ķ˿ں� */
	S2_DPort[0] = I2C_Read(S2_DPORT_EE);
	S2_DPort[1] = I2C_Read(S2_DPORT_EE+1);
	/* ���ض˿�2�Ĺ���ģʽ */
	S2_Mode=I2C_Read(S2_MODE_EE);

	/* ���ض˿�3�Ķ˿ں� */
	S3_Port[0] = I2C_Read(S3_PORT_EE);
	S3_Port[1] = I2C_Read(S3_PORT_EE+1);
	/* ���ض˿�3��Ŀ��IP��ַ */
	for(i=0; i<4; i++)
		S3_DIP[i]=I2C_Read(S3_DIP_EE+i);
	/* ���ض˿�3��Ŀ�Ķ˿ں� */
	S3_DPort[0] = I2C_Read(S3_DPORT_EE);
	S3_DPort[1] = I2C_Read(S3_DPORT_EE+1);
	/* ���ض˿�3�Ĺ���ģʽ */
	S3_Mode=I2C_Read(S3_MODE_EE);
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

	/* �˿�1 */
	Socket_Init(1);

	/* �˿�2 */
	Socket_Init(2);

	/* �˿�3 */
	Socket_Init(3);

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
		else if(S0_Mode==TCP_CLIENT) 	/* TCP�ͻ���ģʽ */
		{
			if(Socket_Connect(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else							/* UDPģʽ */
		{
			if(Socket_UDP(0)==TRUE)
				S0_State=S_INIT|S_CONN;
			else
				S0_State=0;
		}
	}

	/* �˿� 1 */
	if(S1_State==0)
	{
		if(S1_Mode==TCP_SERVER)			/* TCP������ģʽ */
		{
			if(Socket_Listen(1)==TRUE)
				S1_State=S_INIT;
			else
				S1_State=0;
		}
		else if(S1_Mode==TCP_CLIENT)	/* TCP�ͻ���ģʽ */
		{
			if(Socket_Connect(1)==TRUE)
				S1_State=S_INIT;
			else
				S1_State=0;
		}
		else							/* UDPģʽ */
		{
			if(Socket_UDP(1)==TRUE)
				S1_State=S_INIT|S_CONN;
			else
				S1_State=0;
		}
	}

	/* �˿� 2 */
	if(S2_State==0)
	{
		if(S2_Mode==TCP_SERVER)			/* TCP������ģʽ */
		{
			if(Socket_Listen(2)==TRUE)
				S2_State=S_INIT;
			else
				S2_State=0;
		}
		else if(S2_Mode==TCP_CLIENT) 	/* TCP�ͻ���ģʽ */
		{
			if(Socket_Connect(2)==TRUE)
				S2_State=S_INIT;
			else
				S2_State=0;
		}
		else							/* UDPģʽ */
		{
			if(Socket_UDP(2)==TRUE)
			S2_State=S_INIT|S_CONN;
			else
				S2_State=0;
		}
	}

	/* �˿� 3 */
	if(S3_State==0)
	{
		if(S3_Mode==TCP_SERVER)			/* TCP������ģʽ */
		{
			if(Socket_Listen(3)==TRUE)
				S3_State=S_INIT;
			else
				S3_State=0;
		}
		else if(S3_Mode==TCP_CLIENT) 	/* TCP�ͻ���ģʽ */
		{
			if(Socket_Connect(3)==TRUE)
				S3_State=S_INIT;
			else
				S3_State=0;
		}
		else							/* UDPģʽ */
		{
			if(Socket_UDP(3)==TRUE)
				S3_State=S_INIT|S_CONN;
			else
				S3_State=0;
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
			case 0:			/* ���� */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(GATEWAY_IP_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 1:			/* �������� */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(SUBNET_MASK_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 2:			/* �����ַ */
				if(Temp_Buffer[2]!=8)
					error_process();
				else
				{
					for(i=0;i<6;i++)
						I2C_Write(PHY_ADDR_EE+i,Temp_Buffer[5+i]);

					set_data_ok(6);
				}
				break;
			case 3:			/* ����IP��ַ */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(IP_ADDR_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;

			/* �˿�0���� */
			case 0x10:			/* �˿ں� */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S0_PORT_EE,Temp_Buffer[5]);
					I2C_Write(S0_PORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x11:			/* Ŀ��IP��ַ */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(S0_DIP_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 0x12:			/* Ŀ�Ķ˿ں� */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S0_DPORT_EE,Temp_Buffer[5]);
					I2C_Write(S0_DPORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x13:			/* �˿�0�Ĺ���ģʽ */
				if(Temp_Buffer[2]!=3)
					error_process();
				else
				{
					S0_Mode=Temp_Buffer[5];
					I2C_Write(S0_MODE_EE,S0_Mode);
					set_data_ok(1);
				}
				break;

			/* �˿�1���� */
			case 0x20:			/* �˿ں� */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S1_PORT_EE,Temp_Buffer[5]);
					I2C_Write(S1_PORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x21:			/* Ŀ��IP��ַ */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(S1_DIP_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 0x22:			/* Ŀ�Ķ˿ں� */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S1_DPORT_EE,Temp_Buffer[5]);
					I2C_Write(S1_DPORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x23:			/* �˿�1�Ĺ���ģʽ */
				if(Temp_Buffer[2]!=3)
					error_process();
				else
				{
					S1_Mode=Temp_Buffer[5];
					I2C_Write(S1_MODE_EE,S1_Mode);
					set_data_ok(1);
				}
				break;

			/* �˿�2���� */
			case 0x30:			/* �˿ں� */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S2_PORT_EE,Temp_Buffer[5]);
					I2C_Write(S2_PORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x31:			/* Ŀ��IP��ַ */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(S2_DIP_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 0x32:			/* Ŀ�Ķ˿ں� */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S2_DPORT_EE,Temp_Buffer[5]);
					I2C_Write(S2_DPORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x33:			/* �˿�2�Ĺ���ģʽ */
				if(Temp_Buffer[2]!=3)
					error_process();
				else
				{
					S2_Mode=Temp_Buffer[5];
					I2C_Write(S2_MODE_EE,S2_Mode);
					set_data_ok(1);
				}
				break;

			/* �˿�3���� */
			case 0x40:			/* �˿ں� */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S3_PORT_EE,Temp_Buffer[5]);
					I2C_Write(S3_PORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x41:			/* Ŀ��IP��ַ */
				if(Temp_Buffer[2]!=6)
					error_process();
				else
				{
					for(i=0;i<4;i++)
						I2C_Write(S3_DIP_EE+i,Temp_Buffer[5+i]);

					set_data_ok(4);
				}
				break;
			case 0x42:			/* Ŀ�Ķ˿ں� */
				if(Temp_Buffer[2]!=4)
					error_process();
				else
				{
					I2C_Write(S3_DPORT_EE,Temp_Buffer[5]);
					I2C_Write(S3_DPORT_EE+1,Temp_Buffer[6]);
					set_data_ok(2);
				}
				break;
			case 0x43:			/* �˿�3�Ĺ���ģʽ */
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
				case 0:			/* ���� */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(GATEWAY_IP_EE+i);
					break;
				case 1:			/* �������� */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(SUBNET_MASK_EE+i);
					break;
				case 2:			/* �����ַ */
					for(i=0;i<6;i++)
						Temp_Buffer[5+i]=I2C_Read(PHY_ADDR_EE+i);

					Temp_Buffer[2]+=2;
					break;
				case 3:			/* ����IP��ַ */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(IP_ADDR_EE+i);
					break;

				/* ��ȡ�˿�0�����ò��� */
				case 0x10:			/* �˿�0�Ķ˿ں� */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S0_PORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x11:			/* �˿�0��Ŀ��IP��ַ */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(S0_DIP_EE+i);
					break;
				case 0x12:			/* �˿�0��Ŀ�Ķ˿ں� */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S0_DPORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x13:			/* �˿�0�Ĺ���ģʽ */
					Temp_Buffer[5]=S0_Mode;
					Temp_Buffer[2]-=3;
					break;

				/* ��ȡ�˿�1�����ò��� */
				case 0x20:			/* �˿�1�Ķ˿ں� */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S1_PORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x21:			/* �˿�1��Ŀ��IP��ַ */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(S1_DIP_EE+i);
					break;
				case 0x22:			/* �˿�1��Ŀ�Ķ˿ں� */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S1_DPORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x23:			/* �˿�1�Ĺ���ģʽ */
					Temp_Buffer[5]=S1_Mode;
					Temp_Buffer[2]-=3;
					break;

				/* ��ȡ�˿�2�����ò��� */
				case 0x30:			/* �˿�2�Ķ˿ں� */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S2_PORT_EE+i);
					Temp_Buffer[2]-=2;
					break;
				case 0x31:			/* �˿�2��Ŀ��IP��ַ */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(S2_DIP_EE+i);
					break;
				case 0x32:			/* �˿�2��Ŀ�Ķ˿ں� */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S2_DPORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x33:			/* �˿�2�Ĺ���ģʽ */
					Temp_Buffer[5]=S2_Mode;
					Temp_Buffer[2]-=3;
					break;

				/* ��ȡ�˿�3�����ò��� */
				case 0x40:			/* �˿�3�Ķ˿ں� */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S3_PORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x41:			/* �˿�3��Ŀ��IP��ַ */
					for(i=0;i<4;i++)
						Temp_Buffer[5+i]=I2C_Read(S3_DIP_EE+i);
					break;
				case 0x42:			/* �˿�3��Ŀ�Ķ˿ں� */
					for(i=0;i<2;i++)
						Temp_Buffer[5+i]=I2C_Read(S3_DPORT_EE+i);

					Temp_Buffer[2]-=2;
					break;
				case 0x43:			/* �˿�3�Ĺ���ģʽ */
					Temp_Buffer[5]=S3_Mode;
					Temp_Buffer[2]-=3;
					break;

				case 0x80:			/* ��ȡ�¶�ֵ */
					Temp_Buffer[5]=Temperature>>8;
					Temp_Buffer[6]=Temperature;
					Temp_Buffer[2]-=2;
					break;
				case 0x81:			/* ��ȡ��λ���ĵ�ѹֵ */
					Temp_Buffer[5]=Pot>>8;
					Temp_Buffer[6]=Pot;
					Temp_Buffer[2]-=2;
					break;
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

	size=S_rx_process(s);
	memcpy(Tx_Buffer, Rx_Buffer, size);

//	Rx_Data_Process(size);

//	size=Temp_Buffer[2]+3;
//	memcpy(Tx_Buffer, Temp_Buffer, size);

	S_tx_process(s, size);
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
������: ADC_Value_Conv
����: ��
���: �¶�ֵ��ģ���ѹֵ
����:
˵�����ֱ�ɼ�8�����ݽ�������ƽ��
	  �����¶�ʱ,�����ɼ��¶�ֵ�����ɼ��ο���ѹֵ���¶�ֵͨ��������
*********************************************************************/
void ADC_Value_Conv(void)
{
	unsigned char i;

	if(ADC_Complete==1)
	{
		ADC_Complete=0;

		if(ADC_state==0)			/* ���3��������8�βɼ� */
		{
			Temperature=temperature[0];			/* 8���¶�ֵƽ�� */
			for(i=1; i<8 ;i++)
				Temperature+=temperature[i];
			Temperature/=8;

			Vref=vref[0];						/* 8�βο���ѹֵƽ�� */
			for(i=1; i<8; i++)
				Vref+=vref[i];
			Vref/=8;

			Temperature=3376-2680*Temperature/Vref;		/* �����¶�ֵ */

			Pot=pot[0];							/* 8��ģ���ѹֵƽ�� */
			for(i=1; i<8; i++)
				Pot+=pot[i];
			Pot=Pot/99;						/* Pot*330/8/4096 */
		}
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);		/* �����µ�һ��ADC */
	}
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
			S0_Data&=~S_RECEIVE;
			Process_Socket_Data(0);
		}

		/* ���Socket1���յ����� */
		if((S1_Data & S_RECEIVE) == S_RECEIVE)
		{
			S1_Data&=~S_RECEIVE;
			Process_Socket_Data(1);
		}

		/* ���Socket2���յ����� */
//		if((S2_Data & S_RECEIVE) == S_RECEIVE)
//		{
//			S2_Data&=~S_RECEIVE;
//			Process_Socket_Data(2);
//		}

		/* ���Socket3���յ����� */
//		if((S3_Data & S_RECEIVE) == S_RECEIVE)
//		{
//			S3_Data&=~S_RECEIVE;
//			Process_Socket_Data(3);
//		}

		/* ���UART���յ����������ݰ� */
		if(UART_DataReceive==1)
		{
			UART_DataReceive=0;
			Process_UART_Data();
		}

		/* ����ADת�������� */
		ADC_Value_Conv();
	}while(1);
}
