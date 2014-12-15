/* define Digital Input Port, PORTC */
#define DIG_IN	GPIO_Pin_3


/* define three LED lights, PORTE */
#define LED_1 			GPIO_Pin_13
#define LED_2 			GPIO_Pin_14
#define LED_3 			GPIO_Pin_15
#define LED_DRIVE 	LED_1|LED_2|LED_3


/* define two relays, PE2¡¢PE3 */
#define RELAY_1			GPIO_Pin_2
#define RELAY_2			GPIO_Pin_3
#define RELAY_ALL		RELAY_1|RELAY_2

