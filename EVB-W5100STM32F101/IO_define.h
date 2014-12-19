/* define Net SPI Port */
#define NET_CS 		GPIO_Pin_12
#define NET_SCK 	GPIO_Pin_13
#define NET_MISO 	GPIO_Pin_14
#define NET_MOSI	GPIO_Pin_15

#define GPIO_NET_RESET  GPIOB
#define NET_RESET				GPIO_Pin_9
#define GPIO_NET_INT  	GPIOB
#define NET_INT					GPIO_Pin_8


/* define Digital Input Port, PORTC */
#define DIG_IN	GPIO_Pin_3


/* define three LED lights, PORTE */
#define LED_1 			GPIO_Pin_2
#define LED_2 			GPIO_Pin_3
#define LED_3 			GPIO_Pin_4
#define LED_DRIVE 	LED_1 | LED_2 | LED_3


/* define two relays, PC4¡¢PC5 */
#define RELAY_1			GPIO_Pin_4
#define RELAY_2			GPIO_Pin_5
#define RELAY_ALL		RELAY_1|RELAY_2

