#define TRUE	0xff
#define FALSE	0x00

/* Network parameters in EEPROM */
#define GATEWAY_IP_EE	0x00		/* Gateway IP address */
#define SUBNET_MASK_EE	0x04		/* Subnet mask */
#define PHY_ADDR_EE	0x08		/* Physical address */
#define IP_ADDR_EE	0x0e		/* Local IP Address */

/* Socket0 parameters */
#define S0_PORT_EE	0x20		/* Socket0 Port Number */
#define S0_DIP_EE		0x22		/* Socket0 Destination IP Address */
#define S0_DPORT_EE	0x26		/* Socket0 Destination  Port Number */
#define S0_MODE_EE	0x28		/* Socket0 operation mode */

/* Socket1 parameters */
#define S1_PORT_EE	0x30		/* Socket1 Port Number */
#define S1_DIP_EE		0x32		/* Socket1 Destination IP Address */
#define S1_DPORT_EE	0x36		/* Socket1 Destination  Port Number */
#define S1_MODE_EE	0x38		/* Socket1 operation mode */

/* Socket1 parameters */
#define S2_PORT_EE	0x40		/* Socket2 Port Number */
#define S2_DIP_EE		0x42		/* Socket2 Destination IP Address */
#define S2_DPORT_EE	0x46		/* Socket2 Destination  Port Number */
#define S2_MODE_EE	0x48		/* Socket2 operation mode */

/* Socket1 parameters */
#define S3_PORT_EE	0x50		/* Socket3 Port Number */
#define S3_DIP_EE		0x52		/* Socket3 Destination IP Address */
#define S3_DPORT_EE	0x56		/* Socket3 Destination  Port Number */
#define S3_MODE_EE	0x58		/* Socket3 operation mode */
