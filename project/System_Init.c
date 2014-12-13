/****************************************************************
提供商：成都浩然电子有限公司
网址：http://www.hschip.com

时间: 2007-11-6
说明：STM32F10x初始化
****************************************************************/
#include <stm32f10x_lib.h>              /* STM32F10x库 */
#include"IO_define.h"

#define I2C1_SLAVE_ADDRESS7   0
#define ClockSpeed            100000

extern void Delay(unsigned int d);

extern unsigned char ADC_state;

#define TRUE	0xff
#define FALSE	0x00
/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
unsigned char RCC_Configuration(void)
{
	ErrorStatus		HSEStartUpStatus;

  	/* RCC system reset(for debug purpose) */
  	RCC_DeInit();

  	/* Enable HSE */
  	RCC_HSEConfig(RCC_HSE_ON);

  	/* Wait till HSE is ready */
  	HSEStartUpStatus = RCC_WaitForHSEStartUp();

  	if(HSEStartUpStatus == SUCCESS)
  	{
	    /* HCLK = SYSCLK */
    	RCC_HCLKConfig(RCC_SYSCLK_Div1);

	    /* PCLK2 = HCLK */
    	RCC_PCLK2Config(RCC_HCLK_Div1);

	    /* PCLK1 = HCLK/2 */
		RCC_PCLK1Config(RCC_HCLK_Div2);

		/* ADCCLK = PCLK2/4 */
		RCC_ADCCLKConfig(RCC_PCLK2_Div4);

	    /* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);

	    /* Enable Prefetch Buffer */
    	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

	    /* PLLCLK = 8MHz * 7 = 56 MHz */
   		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_7);

	    /* Enable PLL */
    	RCC_PLLCmd(ENABLE);

	    /* Wait till PLL is ready */
    	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET){};

	    /* Select PLL as system clock source */
    	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

	    /* Wait till PLL is used as system clock source */
    	while(RCC_GetSYSCLKSource() != 0x08){};
  	}
	else
		return FALSE;

	/* Enable peripheral clocks --------------------------------------------------*/
  	/* Enable I2C1 and I2C1 clock */
 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 | RCC_APB1Periph_TIM2, ENABLE);

  	/* Enable GPIOA GPIOB SPI1 and USART1 clocks */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
					| RCC_APB2Periph_GPIOC | RCC_APB2Periph_USART1
					| RCC_APB2Periph_SPI1 | RCC_APB2Periph_ADC1
					| RCC_APB2Periph_AFIO, ENABLE);

	return TRUE;
}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures the nested vectored interrupt controller.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
  	NVIC_InitTypeDef	NVIC_InitStructure;

  	/* Set the Vector Table base location at 0x08000000 */
  	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);

 	/* Enable the TIM2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the EXTI4 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

 	/* Enable the USART1 Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);

	/* Configure and enable ADC interrupt 
	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);		   */
}

/*    I2C Initialization    */
void I2C_Configuration(void)
{
	I2C_InitTypeDef  I2C_InitStructure;
  	GPIO_InitTypeDef GPIO_InitStructure;

  	/* Configure I2C1 pins: SCL and SDA -----------*/
  	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

  	/* Enable I2C1 and I2C2 event and buffer interrupt
  	I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, ENABLE);
	*/
  	/* I2C1 configuration ------------------------------------------------------*/
  	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  	I2C_InitStructure.I2C_OwnAddress1 = I2C1_SLAVE_ADDRESS7;
  	I2C_InitStructure.I2C_Ack = I2C_Ack_Disable;
  	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  	I2C_InitStructure.I2C_ClockSpeed = ClockSpeed;
  	I2C_Init(I2C1, &I2C_InitStructure);

  	I2C_Cmd(I2C1, ENABLE);
}

/*   SPI Initialization  */
void SPI_Configuration(void)
{
	GPIO_InitTypeDef 		GPIO_InitStructure;
	SPI_InitTypeDef   	SPI_InitStructure;

  	/* Configure SPI1 pins: SCK, MISO and MOSI -------------*/
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Set Chip Select pin */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);

	/* Set SPI interface */
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial=7;

	SPI_Init(SPI1,&SPI_InitStructure);

	SPI_Cmd(SPI1,ENABLE);					//Enable  SPI1
}

/*    USART Initialization    */
void UART_Configuration(void)
{
	USART_InitTypeDef	USART_InitStructure;
  	GPIO_InitTypeDef 	GPIO_InitStructure;

  	/* Configure USART1 Tx (PA9) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);

  	/* Configure USART1 Rx (PA10) as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate=9600;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;
//	USART_InitStructure.USART_Clock=USART_Clock_Disable;
//	USART_InitStructure.USART_CPOL=USART_CPOL_Low;
//	USART_InitStructure.USART_CPHA=USART_CPHA_2Edge;
//	USART_InitStructure.USART_LastBit=USART_LastBit_Disable;

	USART_Init(USART1,&USART_InitStructure);

/* Enable the USART Transmoit interrupt: this interrupt is generated when the
   USART1 transmit data register is empty */
	USART_ITConfig(USART1, USART_IT_TC, ENABLE);

/* Enable the USART Receive interrupt: this interrupt is generated when the
   USART1 receive data register is not empty */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART1,ENABLE);				//Enable USART1
}

/* Timer2 interrupt every 1ms */
void Timer_Configuration(void)
{
	/* Time base configuration */
	TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;

	TIM_TimeBaseStructure.TIM_Period = 18000;
	TIM_TimeBaseStructure.TIM_Prescaler = 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* TIM enable counter */
	TIM_Cmd(TIM2, ENABLE);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE );
}

/* ADC Initialization */
void ADC_Configuration(void)
{
	ADC_InitTypeDef	ADC_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* Configure PC2 (ADC Channel 12)as analog input */
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_239Cycles5);

	ADC_ITConfig(ADC1, ADC_IT_EOC,ENABLE);

	/* Enable Temperature ADC */
	ADC_TempSensorVrefintCmd(ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1,ENABLE);

	/* Enable ADC1 reset calibaration register */
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));

	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));

	Delay(5);
	ADC_state=0;
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/* IO port Configuration */
void IO_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	EXTI_InitTypeDef	EXTI_InitStructure;

	/* define Digital Input Port */
	GPIO_InitStructure.GPIO_Pin  = DIG_IN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* define Digital Output Port */
	GPIO_InitStructure.GPIO_Pin  = LED_DRIVE;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Turn off LED */
	GPIO_SetBits(GPIOB, LED_DRIVE);

	/* Define	W5100 Indirect Bus interface Port */
	GPIO_InitStructure.GPIO_Pin  = W5100_WR | W5100_RD | W5100_CS | W5100_A0 | W5100_A1;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* define Digital Input Port */
	GPIO_InitStructure.GPIO_Pin  = W5100_DATA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure PC4 as input floating (EXTI Line4) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Connect EXTI Line4 to PC4 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);

	/* PC4 as W5100 interrupt input */
	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

/* STM32F10x Configuration */
void System_Initialization(void)
{
 	/* System Clocks Configuration */
  	while(RCC_Configuration()==FALSE);

  	/* NVIC configuration */
  	NVIC_Configuration();

	/* I2C Configuration */
	I2C_Configuration();

	/* SPI Configuration */
	SPI_Configuration();

	/* IO Configuration */
	IO_Configuration();

	/* UART Configuration */
	UART_Configuration();

	/* Timer configuration */
	Timer_Configuration();

	/* ADC Configuration */
	/* ADC_Configuration();*/
}

