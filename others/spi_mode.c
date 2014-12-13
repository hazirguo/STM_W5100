/*使用前请注意修改端口及其相关配置
RESET ----> PB.0
INT_W ----> PC.5
SEN   ----> PC.4
MOSI  ----> PA.7  SPI1
MISO  ----> PA.6  SPI1
SCK   ----> PA.5  SPI1
NSS   ----> PA.4  SPI1
*/

#ifdef HARDWARE_SPI
/**********************************************************
函数名称: W5100_GPIO_Configuration
功    能: 与W5100通信的GPIO端口配置（硬件SPI）
**********************************************************/
void W5100_GPIO_Configuration(void)
{
        GPIO_InitTypeDef  W5100_GPIO_InitStructure;
        
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB, ENABLE);        //打开GPIOB时钟
        
        W5100_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;                        //RESET(PB.0) 推免输出
        W5100_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        W5100_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB,&W5100_GPIO_InitStructure);
        
        ////////////////////////////////////////////////////////////////////////

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);        //打开GPIOD时钟

        W5100_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;                        //INT_W(PC.5) 上拉输入
        //W5100_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        W5100_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init(GPIOC,&W5100_GPIO_InitStructure);
        
        W5100_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;                //SEN(PC.4) 推免输出
        W5100_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        W5100_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOC,&W5100_GPIO_InitStructure);
        
        ///////////////////////////////////////////////////////////////////////
        
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);        //打开GPIOA时钟

        W5100_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7;        //MOSI(PA.7) SCK(PA.5) 复用推免输出
        W5100_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        W5100_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA,&W5100_GPIO_InitStructure);

        W5100_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;        //MISO(PA.6) 浮点输入 
        W5100_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        W5100_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA,&W5100_GPIO_InitStructure);        
        
        W5100_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;        //NSS(PA.4) 推免输出
        W5100_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        W5100_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOA,&W5100_GPIO_InitStructure);                
}

/**********************************************************
函数名称: W5100_SPI1_Configuration
功    能: 与W5100通信的SPI1配置（硬件SPI）
**********************************************************/
void W5100_SPI1_Configuration(void)
{
        SPI_InitTypeDef  SPI_InitStructure;
        
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);                //打开端口复用时钟、SPI1时钟

        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;         //双线双向全双工
        SPI_InitStructure.SPI_Mode = SPI_Mode_Master;  //主机模式
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;  //SPI发送接收8位帧结构
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                 //时钟悬空低
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;         //数据捕获于第1个时钟沿
        SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                  //软件控制NSS型号
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //比特率8分频
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //数据传输从MSB位开始
        SPI_InitStructure.SPI_CRCPolynomial = 7;  //定义了用于CRC值计算的多项式
        
        SPI_Init(SPI1,&SPI_InitStructure);

        SPI_Cmd(SPI1, ENABLE);         //使能SPI1
}

/**********************************************************
函数名称: W5100_EXTI9_5_Configuration
功    能: 与W5100通信的外部中断5配置（硬件SPI）
**********************************************************/
void W5100_EXTI9_5_Configuration(void)
{
        EXTI_InitTypeDef W5100_EXTI_InitStructure;
        NVIC_InitTypeDef W5100_NVIC_InitStructure;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
        GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource5);         //选择PC5作为外部中断线路

        // Configure EXTI5 line
          W5100_EXTI_InitStructure.EXTI_Line = EXTI_Line5;
          W5100_EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;         //中断请求
          W5100_EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;         //下降沿触发
          W5100_EXTI_InitStructure.EXTI_LineCmd = ENABLE;
          EXTI_Init(&W5100_EXTI_InitStructure);

        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//设置优先级配置的模式，详情请阅读原材料中的文章
          
        //Enable and set EXTI9_5 Interrupt to the lowest priority
          W5100_NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
          W5100_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
          W5100_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
          W5100_NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

          NVIC_Init(&W5100_NVIC_InitStructure);
}

/**********************************************************
函数名称: W5100_Prot_INIT
功    能: MCU端口初始化（硬件SPI）
**********************************************************/
void W5100_Port_Init(void)
{
        W5100_GPIO_Configuration();
        W5100_SCS_HIGH;
        
        W5100_SEN_HIGH;                 //SEN:1 SPI模式
        W5100_RST_HIGH;                //RST:1 禁止复位

        W5100_RST_LOW;           //W5100硬件复位
    W5100_Delay(200);
    W5100_RST_HIGH;
        
        W5100_EXTI9_5_Configuration();
        W5100_SPI1_Configuration();        
}

/**********************************************************
函数名称: SPI_SendByte
功    能: SPI1发/收一个字节数据（硬件SPI）
**********************************************************/
unsigned char SPI_SendByte(unsigned char dt)
{
        //等待SPI1发送完毕
        while((SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) == RESET));
        
        SPI_I2S_SendData(SPI1,dt);
        
        //等待SPI1接收完毕
        while((SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET));
        
        return SPI_I2S_ReceiveData(SPI1);        
}

/**********************************************************
函数名称: Write_W5100
功    能: 向W5100寄存器地址为Addr写byte一字节（硬件SPI）
**********************************************************/
void Write_W5100(unsigned int Addr,unsigned char byte)
{
        W5100_SCS_LOW;  //SS拉低
         
        SPI_SendByte(0xf0);

        SPI_SendByte(Addr/256);
        SPI_SendByte(Addr);

        SPI_SendByte(byte);
                        
        W5100_SCS_HIGH;
}

/**********************************************************
函数名称: Read_W5100
功    能: 读取W5100寄存器地址为Addr的值并返回（硬件SPI）
**********************************************************/
unsigned char Read_W5100(unsigned int Addr)
{
        unsigned char i;

        W5100_SCS_LOW;  //SS拉低
        
        SPI_SendByte(0x0f);

        SPI_SendByte(Addr/256);
        SPI_SendByte(Addr);
        
        i = SPI_SendByte(0);

        W5100_SCS_HIGH;

        return i;
}

#else

/**********************************************************
函数名称: W5100_GPIO_Configuration
功    能: 与W5100通信的GPIO端口配置（模拟SPI）
**********************************************************/
void W5100_GPIO_Configuration(void)
{
        GPIO_InitTypeDef  W5100_GPIO_InitStructure;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);        //打开GPIOA时钟

        W5100_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7|GPIO_Pin_4;        //MOSI(PA.7) SCK(PA.5) 推免输出
        W5100_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        W5100_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOA,&W5100_GPIO_InitStructure);

        W5100_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;        //MISO(PA.6) 浮点输入
        //W5100_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        W5100_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA,&W5100_GPIO_InitStructure);
        
        ///////////////////////////////////////////////////////////////////////////////////////////////
        
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);        //打开GPIOB时钟

        W5100_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;        //SEN(PB.6) NSS(PB.7) 推免输出
        W5100_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        W5100_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOC,&W5100_GPIO_InitStructure);

        ///////////////////////////////////////////////////////////////////////////////////////////////

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB, ENABLE);        //打开GPIOD时钟

        W5100_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;                        //INT_W(PC.5) 上拉输入
        //W5100_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        W5100_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init(GPIOC,&W5100_GPIO_InitStructure);

        W5100_GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;                        //RESET(PD.4) 推免输出
        W5100_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        W5100_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB,&W5100_GPIO_InitStructure);
}

/**********************************************************
函数名称: W5100_EXTI9_5_Configuration
功    能: 与W5100通信的外部中断5配置（模拟SPI）
**********************************************************/
void W5100_EXTI9_5_Configuration(void)
{
        EXTI_InitTypeDef W5100_EXTI_InitStructure;
        NVIC_InitTypeDef W5100_NVIC_InitStructure;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
        GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);         //选择PD5作为外部中断线路

        // Configure EXTI5 line
          W5100_EXTI_InitStructure.EXTI_Line = EXTI_Line5;
          W5100_EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;                 //中断请求
          W5100_EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;         //下降沿触发
          W5100_EXTI_InitStructure.EXTI_LineCmd = ENABLE;
          EXTI_Init(&W5100_EXTI_InitStructure);

        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);                //设置优先级配置的模式，详情请阅读原材料中的文章
          
        //Enable and set EXTI9_5 Interrupt to the lowest priority
          W5100_NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
          W5100_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
          W5100_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
          W5100_NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

          NVIC_Init(&W5100_NVIC_InitStructure);
}

/**********************************************************
函数名称: W5100_Prot_INIT
功    能: MCU端口初始化（模拟SPI）
**********************************************************/
void W5100_Port_Init(void)
{
        W5100_GPIO_Configuration();
        W5100_SCS_HIGH;
        W5100_SEN_HIGH;                 //SEN:1 SPI模式
        W5100_RST_HIGH;                //RST:1 禁止复位

        W5100_RST_LOW;           //W5100硬件复位
    W5100_Delay(200);
    W5100_RST_HIGH;
        
        //W5100_SPI1_Configuration();
        W5100_EXTI9_5_Configuration();        
}

/**********************************************************
函数名称: MISO_PIN
功    能: 读MISO端口的值（模拟SPI）
**********************************************************/
unsigned char MISO_PIN(void)
{
        unsigned char ReadValue;
        ReadValue = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6);
        return ReadValue;
}

/**********************************************************
函数名称: SPI_WR_Byte
功    能: SPI发送一字节数据（模拟SPI）
**********************************************************/
void SPI_WR_Byte(unsigned char byte)
{
    char i;
    for(i=0;i<8;i++)
    {
        if(byte&0x80)  W5100_MOSI_HIGH;
        else W5100_MOSI_LOW;
        byte=byte<<1;
        W5100_SCK_HIGH;;;
        W5100_SCK_LOW;
    }
}

/**********************************************************
函数名称: SPI_RD_Byte
功    能: SPI接收一字节数据并返回（模拟SPI）
**********************************************************/
unsigned char SPI_RD_Byte(void)
{
    unsigned char i,temp=0;
    for(i=0;i<8;i++)
    {
        temp=temp<<1;
        W5100_SCK_HIGH;
        if(MISO_PIN()) temp++;
        W5100_SCK_LOW;
    }
    return temp;
}

/**********************************************************
函数名称: Write_W5100
功    能: 向W5100寄存器地址为Addr写byte一字节（模拟SPI）
**********************************************************/
void Write_W5100(unsigned int Addr,unsigned char byte)
{
    //CLI();
    W5100_SCS_LOW;
    //W5100_Delay(0xff);
    SPI_WR_Byte(0xf0);      //发送写操作0XF0
    SPI_WR_Byte(Addr/256);  //发送寄存器地址高8位
    SPI_WR_Byte(Addr);      //发送寄存器地址低8位
    SPI_WR_Byte(byte);      //发送数据
    //W5100_Delay(0xff);
    W5100_SCS_HIGH;
    //SEI();
}

/**********************************************************
函数名称: Read_W5100
功    能: 读取W5100寄存器地址为Addr的值并返回
**********************************************************/
unsigned char Read_W5100(unsigned int Addr)
{
    unsigned char j=0;
    //CLI();
    W5100_SCS_LOW;
    //W5100_Delay(0xff);
    SPI_WR_Byte(0x0f);      //发送读操作0X0F
    SPI_WR_Byte(Addr/256);  //发送寄存器地址高8位
    SPI_WR_Byte(Addr);      //发送寄存器地址低8位
    j=SPI_RD_Byte();        //读取数据
    
    W5100_SCS_HIGH;
    //SEI();
    return j;
}

#endif
