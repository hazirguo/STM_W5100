#define TRUE	0xff
#define FALSE	0x00

#define FLASH_BASE_ADDR 	0

/* Network parameters */

#define FLASH_GATEWAY_IP_ADDR 0x00000
//#define FLASH_GATEWAY_IP_ADDR		(FLASH_BASE_ADDR)
#define FLASH_GATEWAY_IP_SIZE	4
#define GATEWAY_IP_ADDR_1	1		/* Gateway IP address */
#define GATEWAY_IP_ADDR_2	0
#define GATEWAY_IP_ADDR_3	0
#define GATEWAY_IP_ADDR_4	1

#define FLASH_SUBNET_MASK_ADDR 0x10000
//#define FLASH_SUBNET_MASK_ADDR		(FLASH_GATEWAY_IP_ADDR + FLASH_GATEWAY_IP_SIZE)
#define FLASH_SUBNET_MASK_SIZE		4
#define SUBNET_MASK_1	255		/* Subnet mask */
#define SUBNET_MASK_2	255		
#define SUBNET_MASK_3	255		
#define SUBNET_MASK_4	0		

#define FLASH_PHY_ADDR_ADDR 0x20000
//#define FLASH_PHY_ADDR_ADDR			(FLASH_SUBNET_MASK_ADDR + FLASH_SUBNET_MASK_SIZE)
#define FLASH_PHY_ADDR_SIZE			6
#define PHY_ADDR_1	0x00		/* Physical address */
#define PHY_ADDR_2	0x80
#define PHY_ADDR_3	0xA3
#define PHY_ADDR_4	0x9E
#define PHY_ADDR_5	0xC5
#define PHY_ADDR_6	0x75

#define FLASH_LOCAL_IP_ADDR 0x30000
//#define FLASH_LOCAL_IP_ADDR		(FLASH_PHY_ADDR_ADDR + FLASH_PHY_ADDR_SIZE)
#define FLASH_LOCAL_IP_SIZE		4
#define LOCAL_IP_ADDR_1	1		/* Local IP Address */
#define LOCAL_IP_ADDR_2	0
#define LOCAL_IP_ADDR_3	0
#define LOCAL_IP_ADDR_4	223

#define FLASH_LOCAL_PORT_ADDR 		0x40000
//#define FLASH_S0_PORT_ADDR		(FLASH_LOCAL_IP_ADDR + FLASH_LOCAL_IP_SIZE)
#define FLASH_LOCAL_PORT_SIZE		2

#define FLASH_REMOTE_IP_ADDR 		0x50000
#define FLASH_REMOTE_IP_SIZE		4

#define FLASH_REMOTE_PORT_ADDR	0x60000
#define FLASH_REMOTE_PORT_SIZE	2

/* Socket0 parameters */
#define S0_PORT_1	0x13		/* Socket0 Port Number */
#define S0_PORT_2	0x88


#define S0_MODE		TCP_SERVER		/* Socket0 operation mode */
#define S1_MODE		TCP_CLIENT		/* Socket1 operation mode */
