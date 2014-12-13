/* For W5100 Indirect Bus Interface, PORTA*/  
#define W5100_WR	GPIO_Pin_1
#define W5100_RD	GPIO_Pin_2
#define W5100_CS	GPIO_Pin_3
#define W5100_A0	GPIO_Pin_11
#define W5100_A1	GPIO_Pin_12

/* For W5100 Indierct Bus Interface, Data Port, PORTB */
#define W5100_DATA	GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15

/* define Digital Output(Drive LED) Port, PORTB */
#define LED_DRIVE		GPIO_Pin_0

/* define Digital Input Port, PORTC */
#define DIG_IN	GPIO_Pin_3
