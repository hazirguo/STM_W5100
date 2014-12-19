/*****************************************************************
 *                     LABEQC.C
 *
 *  PURPOSE: the kernel module for the equipment controller cards
 *
 *  EDIT HISTORY:
 *     Programmer          Change                  Date
 *     -----------------------------------------------------------
 *     Nick Yang       Initial Version           02/05/1999
 *     John Shott      Modified                  03/20/1999
 *     Thomas Lohman   Ported to kernel 2.2.x    05/01/2000
 *     Thomas Lohman   Reworked for kernel 2.4.x 12/10/2003
 *     Thomas Lohman   Reworked for kernel 2.6.x 04/04/2006
 *     Thomas Lohman   Incorporated changes      09/24/2008
 *                     from Frank Hess @ NIST.
 *     Thomas Lohman   Incorporated changes      07/09/2012
 *                     from dylan.klomparens@nist.gov
 *
 ******************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/delay.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
#include <linux/mutex.h>
#else
#include <asm/system.h>
#endif

#include <asm/segment.h>
#include <asm/io.h>     
#include <asm/uaccess.h>

#include "../include/labeqc.h"

MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE(LABEQC_DEVICE);
MODULE_DESCRIPTION("The Coral Equipment Controller Driver");

#define MAX_NUM_IO_RESERVATIONS 2
#define NUM_IO_PORTS 2

/****** external global functions *****************/

/****** global functions **************************/

/****** static functions **************************/

static int ports_are_reserved(unsigned, unsigned);
static void auto_call(LABEQC_Instruction *, int);
static void manual_issue(LABEQC_Instruction *);
static void manual_acquire(LABEQC_Instruction *);
static void manual_delay(unsigned int);
static int labeqc_init(void);
static void labeqc_exit(void);
static int labeqc_open(struct inode *, struct file *);
static int labeqc_release(struct inode *, struct file *);
static int labeqc_instruction_list_ioctl(unsigned long);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
static long labeqc_ioctl(struct file *, unsigned int, unsigned long);
#else
static int labeqc_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
#endif

/****** external global variables *****************/

/****** global variables **************************/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
DEFINE_MUTEX(ioctl_mutex);
#endif

/****** static variables **************************/

static unsigned ioports_parameters_count[MAX_NUM_IO_RESERVATIONS];
static unsigned ioports_parameters[MAX_NUM_IO_RESERVATIONS][NUM_IO_PORTS] = { {0x300, 0x301}, {0x0, 0x0} };

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 18) 
module_param_array_named(ioports0, ioports_parameters[0], uint, ioports_parameters_count[0], 0);
#else
module_param_array_named(ioports0, ioports_parameters[0], uint, &ioports_parameters_count[0], 0);
#endif

MODULE_PARM_DESC(ioports0, "IO ports 0 config, a two element array: even_ioaddr,odd_ioaddr (defaults to ioports0=0x300,0x301)");

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 18) 
module_param_array_named(ioports1, ioports_parameters[1], uint, ioports_parameters_count[1], 0);
#else
module_param_array_named(ioports1, ioports_parameters[1], uint, &ioports_parameters_count[1], 0);
#endif

MODULE_PARM_DESC(ioports1, "IO ports 1 config, a two element array: even_ioaddr,odd_ioaddr (defaults to unused)");

struct labeqc_ioport_reservation
{
  unsigned even_port;
  unsigned odd_port;
  unsigned reserved : 1;
};

static int major_number;
static const char *driver_name = "labeqc";
static struct labeqc_ioport_reservation io_reservations[MAX_NUM_IO_RESERVATIONS];

/* A new way to define the file_operations structure. */
/* All the unspecified values get assigned NULL automatically. */
/* thomasl - 4/5/2006 */
static struct file_operations labeqc_fops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
  .unlocked_ioctl = labeqc_ioctl,
#else
  .ioctl = labeqc_ioctl,
#endif
  .open  = labeqc_open,
  .release = labeqc_release
};

/*******************************
*                              *
*  ports_are_reserved.c        *
*                              *
*******************************/
static int ports_are_reserved(unsigned even_port, unsigned odd_port)
{
  unsigned i;

  for (i = 0; i < MAX_NUM_IO_RESERVATIONS; ++i)
   {
     if ((io_reservations[i].reserved) && (io_reservations[i].even_port == even_port) && (io_reservations[i].odd_port == odd_port))
       return (1);
   }
  return (0);
}
 

/*******************************
 *                             *
 *   labeqc_init.c             *
 *                             *
 ******************************/
static int labeqc_init(void)
{
  int error;
  unsigned i = 0, j = 0;

  memset(io_reservations, 0, sizeof(io_reservations));

  /* have the kernel register a major number with this driver */
  error = register_chrdev(0, driver_name, &labeqc_fops);
  if (error < 0)
    return (error);
  else
    major_number = error; 

  for (i = 0; i < MAX_NUM_IO_RESERVATIONS; ++i)
   {
     if (ioports_parameters_count[i] == 0)
      {
        // use default ports if appropriate
        if (ioports_parameters[i][0] && ioports_parameters[i][1])
          ioports_parameters_count[i] = NUM_IO_PORTS;
        else
          continue;
      }

     if (ioports_parameters_count[i] != NUM_IO_PORTS)
      {
        printk(KERN_WARNING "%s: user supplied an ioports%i parameter with %i ports, but driver expected %i.\n", driver_name, i, ioports_parameters_count[i], NUM_IO_PORTS);
        labeqc_exit();
        return (-EINVAL);
      }
    
     if (!request_region(ioports_parameters[i][0], 1, driver_name))
      {
        printk(KERN_WARNING "%s: ioport %i is already in use.\n", driver_name, ioports_parameters[i][0]);
        labeqc_exit();
        return (-EBUSY);
      }

     if (!request_region(ioports_parameters[i][1], 1, driver_name))
      {
        release_region(ioports_parameters[i][0], 1);
        printk(KERN_WARNING "%s: ioport %i is already in use.\n", driver_name, ioports_parameters[i][0]);
        labeqc_exit();
        return (-EBUSY);
      }

     io_reservations[j].even_port = ioports_parameters[i][0];
     io_reservations[j].odd_port = ioports_parameters[i][1];
     io_reservations[j].reserved = 1;
     ++j;
   }

  if (j != 0)
    return (0);
  else
   {
     printk(KERN_WARNING "%s: at least one pair of ioports must be specified using a module parameter such as the ioports0 array.\n", driver_name);
     labeqc_exit();
     return (-EINVAL);
   }
}


/*******************************
 *                             *
 *    labeqc_exit.c            *
 *                             *
 ******************************/
static void labeqc_exit(void)
{
  unsigned i = 0;

  for (i = 0; i < MAX_NUM_IO_RESERVATIONS; ++i)
   {
     if (io_reservations[i].reserved == 0)
       continue;
     release_region(io_reservations[i].even_port, 1);
     release_region(io_reservations[i].odd_port, 1);
   }

  unregister_chrdev(major_number, driver_name);

  return;
}


/*******************************
 *                             *
 *   labeqc_open.c             *
 *                             *
 ******************************/
static int labeqc_open(struct inode *inode, struct file *filp)
{
#ifdef DEBUG
  printk("Opening labeqc device...\n");
#endif

  try_module_get(THIS_MODULE);

  return (0);
}


/*******************************
 *                             *
 *   labeqc_release.c          *
 *                             *
 ******************************/
static int labeqc_release(struct inode *inode, struct file *filp)
{
#ifdef DEBUG
  printk("Releasing labeqc device...\n");
#endif

  module_put(THIS_MODULE);

  return (0);
}


/************************************
 *                                  *
 * labeqc_instruction_list_ioctl.c  *
 *                                  *
 ***********************************/
static int labeqc_instruction_list_ioctl(unsigned long user_instruction_list)
{
  int i;

  LABEQC_Instruction_List instruction_list;
  LABEQC_Instruction *instruction = (LABEQC_Instruction *) NULL;

  //copy user_instruciton_list to kernel
  if (copy_from_user(&instruction_list, (void *) user_instruction_list, sizeof(LABEQC_Instruction_List)) != 0)
    return (-EFAULT);

  // Check the number of commands to eliminate memory problems
  if ((instruction_list.num_instructions > MAX_SEQUENCE) || (instruction_list.num_instructions <= 0))
   {
     printk("LABEQC ERROR: %d is an invalid number of instructions!\n", instruction_list.num_instructions);
     return (-EINVAL);
   }
  else 
   {
#ifdef DEBUG
     printk("LABEQC: Received %d instructions:\n", instruction_list.num_instructions);
#endif
   }

  /* call to copy data from user space to kernel space */
  instruction = kzalloc(sizeof(LABEQC_Instruction) * instruction_list.num_instructions, GFP_KERNEL);
  if (instruction == (LABEQC_Instruction *) NULL) 
    return (-ENOMEM);

  if (copy_from_user(instruction, instruction_list.instructions, instruction_list.num_instructions * sizeof(LABEQC_Instruction)) != 0)
   {
     kfree(instruction);
     return (-EFAULT);
   }

  // Sanity check to make sure that all instructions are valid.
  for (i = 0; i < instruction_list.num_instructions; i++)
   {
      if (instruction[i].magic_num != MAGIC_NUM)
       {
         printk("LABEQC ERROR: Instruction %d is not valid!\n", i + 1);
         kfree(instruction);
         return (-EINVAL);
       }

      if (ports_are_reserved(instruction[i].even_port, instruction[i].odd_port) == 0)
       {
         kfree(instruction);
         return (-EINVAL);
       }
   }

  // Now we'll send out this command/group to the card
  for (i = 0; i < instruction_list.num_instructions; i++)
   {
#ifdef DEBUG
      printk("LABEQC: Instruction %d:\n", i);
      printk("LABEQC:   Type: %d\n", instruction[i].type);
      printk("LABEQC:   Card: %d\n", instruction[i].card_id);
      printk("LABEQC:   Channel: %d\n", instruction[i].channel);
      printk("LABEQC:   Command: %d\n", instruction[i].command);
      printk("LABEQC:   Wait_time: %d\n", instruction[i].wait_time);
#endif

     switch (instruction[i].type)
      {
        case AUTO_ENABLE: // auto enable
          auto_call(&(instruction[i]), LABEQC_ENABLE);
          break;
        case AUTO_DISABLE: // auto disable
          auto_call(&(instruction[i]), LABEQC_DISABLE);
          break;
        case AUTO_SENSE: // auto sense
          auto_call(&(instruction[i]), LABEQC_SENSE);
          break;
        case AUTO_STATUS: // auto status
          auto_call(&(instruction[i]), LABEQC_STATUS);
          break;
        case MANUAL_WRITE: //manual write
          manual_issue(&(instruction[i]));
          break;
        case MANUAL_READ:  //manual read
  	      manual_acquire(&(instruction[i]));
	      break;
        case MANUAL_DELAY: ////manual delay
          manual_delay(instruction[i].wait_time);
	      break;
        default:
          printk("LABEQC ERROR: Invalid instruction type in inst. %d!\n", i);
          kfree(instruction);
	      return (-EINVAL);
       }
   }

  if (copy_to_user(instruction_list.instructions, instruction, instruction_list.num_instructions * sizeof(LABEQC_Instruction)) != 0)
   {
     kfree(instruction);
     return (-EFAULT);
   }

  kfree(instruction);
  return (0);
}


/********************************
*                               *
*   labeqc_ioctl.c              *
*                               *
+ ******************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
static long labeqc_ioctl(struct file *filp, unsigned int request, unsigned long arg)
{
  long result;

  /* Acquire the I/O control lock. Abort if the user interrupts the attempt. */
  if(mutex_lock_interruptible(&ioctl_mutex))
    return (-EINTR);

  /* Perform the IO control operation. */
  if(request == LABEQC_INSTRUCTION_LIST_REQ)
    result = labeqc_instruction_list_ioctl(arg);
  else
    result = (-ENOTTY);

  /* Release the I/O control lock. */
  mutex_unlock(&ioctl_mutex);

  return result;
}
#else
static int labeqc_ioctl(struct inode *inode, struct file *filp, unsigned int request, unsigned long arg)
{
  switch (request)
   {
     case LABEQC_INSTRUCTION_LIST_REQ:
       return (labeqc_instruction_list_ioctl(arg));
       break;
     default:
       return (-ENOTTY);
       break;
   }

  return (0);
}
#endif


/*******************************
 *                             *
 *   auto_call.c               *
 *                             *
 ******************************/
static void auto_call(LABEQC_Instruction *instruction, int command)
{
  instruction->command = command;
  manual_issue(instruction);
  manual_delay(AUTO_SLEEP);
  manual_acquire(instruction);

  /* loop until we get a non-busy result back */
  while (instruction->even_result & 0x1)
   {
     manual_delay(1);
     manual_acquire(instruction);
   }
}


/*******************************
 *                             *
 *  manual_issue.c             *
 *                             *
 ******************************/
static void manual_issue(LABEQC_Instruction *instruction)
{
  int selection;
  
  selection = instruction->card_id << 5;
  selection = selection|instruction->channel;
  outb_p(selection, instruction->even_port);           //card_id & channel -> even port
  outb_p(instruction->command, instruction->odd_port); //command -> odd port
}


/*******************************
 *                             *
 *   manual_acquire.c          *
 *                             *
 ******************************/
static void manual_acquire(LABEQC_Instruction *instruction)
{
  int selection;

  selection = instruction->card_id << 5;
  selection = selection|instruction->channel;
  /* Note: we must always re-send the card and channel information before a
     read in case the card was doing something else in the meantime */
  outb_p(selection, instruction->even_port);
  instruction->even_result = inb_p(instruction->even_port);
  instruction->odd_result = inb_p(instruction->odd_port);
#ifdef DEBUG
  printk("LABEQC:   Even result: %d\n", (int) instruction->even_result);
  printk("LABEQC:   Odd result: %d\n", (int) instruction->odd_result);
#endif
}


/*******************************
 *                             *
 *   manual_delay.c            *
 *                             *
 ******************************/
static void manual_delay(unsigned int delay_time)
{
  msleep_interruptible(delay_time);

  return;
}

module_init(labeqc_init);
module_exit(labeqc_exit);
