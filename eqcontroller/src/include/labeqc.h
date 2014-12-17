/*****************************************************************
 *                      LABEQC.H
 *
 *  PURPOSE: Header file for EQ Controller Daemon and Driver
 *
 *  EDIT HISTORY:
 *     Programmer          Change                  Date
 *     -----------------------------------------------------------
 *
 ******************************************************************/

#ifndef LABEQC_H
#define LABEQC_H

#define TRUE 1
#define FALSE 0

#define MAX_SEQUENCE 32  // Maximum number of instructions in a block

// Device name for the labeqc device
#define LABEQC_DEVICE "/dev/labeqc"

// Types of automatic instructions
#define AUTO_ENABLE 1
#define AUTO_DISABLE 2
#define AUTO_SENSE 3
#define AUTO_STATUS 4

#define MAX_AUTO_INSTRUCTION 99

// Types of manual instructions
#define MANUAL_WRITE 101
#define MANUAL_READ 102
#define MANUAL_DELAY 103

// Hex values for automatic command bytes ...
#define LABEQC_ENABLE 0x78
#define LABEQC_DISABLE 0x58
#define LABEQC_SENSE 0x18
#define LABEQC_STATUS 0x10

// Automatic commands have an built in delay of 16 msec
#define AUTO_SLEEP 16
#define MAGIC_NUM 0xefab     // Magic number to authenticate "real" commands

#ifdef EQ_DAEMON

#define EQC_HOME "/usr/local/eqcontroller"
#define EQC_USERNAME "eqcsys"

#define EQCONFFILE "etc/eqc.conf"
#define EQ_SLAVE_EXEC "bin/eqslave"
#define EQ_SYS_LOG "log/daemon_sys.log"
#define EQ_ERR_LOG "log/daemon_err.log"

#define SERVICE_NAME "equipcntl"
#define SERVICE_TYPE "tcp"

#define BEGIN_HANDSHAKE_STRING  "EQCNTL_BEGIN_COMMAND"
#define END_HANDSHAKE_STRING    "EQCNTL_END_COMMAND"

#endif /* EQ_DAEMON */

struct LABEQC_Instruction
{
    unsigned int magic_num;  // this is used to check whether we have a
                             // valid packet it's always should be set to 
                             // 168 by design.
    unsigned int odd_port;
    unsigned int even_port;
    unsigned int command;         // Instruction [0-255] (sent to odd port)
    unsigned int card_id;         // card_id [0-7] (sent to even port)
    unsigned int channel;         // channel [0-28] (sent to even port)
    unsigned char odd_result;     // Result byte from the odd port
    unsigned char even_result;    // Result byte from the even port
    unsigned int type;            // Instruction type (see #DEFINEs above)
    unsigned int wait_time;       // Delay time in milliseconds
    void *container;              // Pointer to issuer of instruction
};
typedef struct LABEQC_Instruction LABEQC_Instruction;

typedef struct
{
  unsigned num_instructions;
  LABEQC_Instruction *instructions;
} LABEQC_Instruction_List;

typedef struct EQDAEMON_Struct
{
  unsigned int instrcount;
  unsigned int cardid;
  unsigned int evenport;
  unsigned int oddport;
  unsigned int channel;
  unsigned int ok;
  struct {
     unsigned int ctype;
     unsigned int write_command;
     unsigned int delay_time;
     unsigned char sdOverload;
     unsigned char rdOverload;
     unsigned char adcDone;
     unsigned char busy;
     int returnValue;
  } command[MAX_SEQUENCE];
} EQDAEMON_Struct;

static const char labeqc_ioctl_type = 'X'; // arbitrary number

// ioctl request numbers
//  #define _IOWR(type,nr,size)    _IOC(_IOC_READ|_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define LABEQC_INSTRUCTION_LIST_REQ _IOWR(labeqc_ioctl_type, 0, LABEQC_Instruction_List)

#ifdef EQ_DAEMON

extern void EQWriteSystemLog (const char *);
extern void EQWriteErrorLog (const char *);
extern void EQWriteSystemErrorLog (const char *, const char *, int);
extern void LabeqdLog (int fd, const char *);
extern FILE *DaemonSysLog;
extern FILE *DaemonErrLog;

#endif /* EQ_DAEMON */

#endif
