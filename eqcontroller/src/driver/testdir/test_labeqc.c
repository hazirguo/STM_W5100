/*****************************************************************
 *                     TEST_LABEQC.C
 *
 *  PURPOSE: Test program for the labeqc kernel driver program
 *
 *  EDIT HISTORY:
 *     Programmer          Change                  Date
 *     -----------------------------------------------------------
 *
 *  FUNCTION LIST:
 *     load_events
 *     output_results
 *     main
 *
 ******************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/labeqc.h"
#include "lists.h"

#define MAXLINE 80

/*
   This program reads in a file with one command, comment, or blank line per
   line.  A block of commands is terminated by a blank line. Comments do not
   count as a blank line.
   Valid commands are:  
   PORT: oddport, evenport (Note: oddport and evenport are in Hex)
   AUTO: card, channel, command (Note: command is an int defined in labeqc.h)
   WRITE: card, channel. command (Note: command_byte is a hex value [00-ff])
   READ: card, channel
   DELAY: delay_time (Note: delay time is in milliseconds)
*/

struct LISTOBJ {
  LABEQC_Instruction *instruction;   // Instruction(s) to be sent to driver
  char line[MAXLINE];                // the line as it is in the input file
  _Bool isComment;                    // a flag to indicate a comment line
  _Bool isSpace;                      // a flag to indicate a blank line
};
typedef struct LISTOBJ LISTOBJ;


/****** external global functions *****************/

/****** global functions **************************/

/****** static functions **************************/

static int load_events();
static void output_results();

/****** external global variables *****************/

/****** global variables **************************/

int device_fd, err;
List *event_list;
List *done_list;

/****** static variables **************************/


/************************************************************************
 * load_events
 *
 * This method reads and parses input instructions from stdin.
 * They are supposed to be one instruction to a line and a blank
 * line separates blocks of commands. A comment does not count as
 * a command-block separator.
 ************************************************************************/
static int load_events()
{
  Element *entry;
  char tempLine[MAXLINE];
  char lefthalf[25];
  unsigned int evenport, oddport;
  LABEQC_Instruction *instruction;
  LISTOBJ *listobj;

  memset(lefthalf, 0, 25);
  while (fgets(tempLine, MAXLINE, stdin) != NULL) 
   {
     switch (tempLine[0])
      {
        case 'A': // auto
        case 'W': // write
        case 'D': // delay
        case 'R': // read
          break;
        case 'P': // we received the port command
          sscanf(tempLine, "PORT:%X, %X", &evenport, &oddport); 
        case '\n': // new line
        case '#': // comment
        case ' ': // space
        case '\0': // NULL
          // here we load the comment or new line into a structure and read
          // the next line
 	  listobj = (LISTOBJ *)malloc(sizeof(LISTOBJ));
	  listobj->instruction = NULL;
	  strcpy(listobj->line, tempLine);
	  listobj->isComment = TRUE;
	  if ((tempLine[0] == ' ') || (tempLine[0] == '\n') || 
	      (tempLine[0] == '\0'))
	    listobj->isSpace = TRUE;
	  entry = createElement(listobj);
	  addElement(event_list, entry);
	  continue;
        default:
          return (-1);
      }

     instruction = (LABEQC_Instruction *)malloc(sizeof(LABEQC_Instruction));
     // bzero(instruction, sizeof(LABEQC_Instruction));
     listobj = (LISTOBJ *)malloc(sizeof(LISTOBJ));
     // bzero(listobj, sizeof(LABEQC_Instruction));
      
     strcpy(listobj->line, tempLine);
     listobj->instruction = instruction;
     listobj->isComment = FALSE;
     listobj->isSpace = FALSE;

     instruction->magic_num = MAGIC_NUM;  // look at the labeqc.h file
     instruction->even_port = evenport;
     instruction->odd_port = oddport;
     instruction->container = listobj;
     sscanf(tempLine, "%s:", lefthalf);
     if (memcmp(lefthalf, "WRIT", 4) == 0)
      {
        instruction->type = MANUAL_WRITE;
        sscanf(tempLine, "WRITE:%d, %d, %x", &(instruction->card_id),
	       &(instruction->channel), &(instruction->command));
      }
     if (memcmp(lefthalf, "DELA", 4) == 0)
      {
        instruction->type = MANUAL_DELAY;
        sscanf(tempLine, "DELAY:%d", &(instruction->wait_time));
      }
     if (memcmp(lefthalf, "READ", 4) == 0)
      {
        instruction->type = MANUAL_READ;
        sscanf(tempLine, "READ:%d, %d", &(instruction->card_id),
                 &(instruction->channel));
      }
     if (memcmp(lefthalf, "AUTO", 4) == 0)
      {
        sscanf(tempLine, "AUTO:%d, %d, %d", &(instruction->card_id),
                 &(instruction->channel), &(instruction->type));
      }
     entry = createElement(listobj);
     addElement(event_list, entry);
   }
  fclose(stdin);
  return (0);
}


/*******************************
 *                             *
 *  output_results.c           *
 *                             *
 ******************************/
static void output_results()
{
  Element *entry;
  LISTOBJ *obj;
  while (done_list->number > 0)
   {
     entry = removeElement(done_list);
     obj = (LISTOBJ *)entry->Object;
     write(1, obj->line, strlen(obj->line));
     free(obj);
     free(entry);
   }
}


/*******************************
 *                             *
 *      main.c                 *
 *                             *
 ******************************/
int main (int argc, char** argv)
{
  Element *entry;
  LISTOBJ *obj;
  char sOvl = 0, rOvl = 0, adcDone = 0, busy = 0;
  int adcValue = 0, result;
  int returnValue = 0;
  int i, cmd_count;
  unsigned char lastcommand = 0;
  unsigned int d1, d2;

  if ( argc != 1 ) 
   {
     fprintf(stderr, "Usage: %s \n", argv[0]);
     fprintf(stderr, "\t or %s <command_file\n", argv[0]);
     fprintf(stderr, "\t or %s <command_file >output_file\n", argv[0]);
     fprintf(stderr, "\t or %s <command_file >&output_file\n", argv[0]);
     fprintf(stderr, "\t or %s <command_file >>output_file\n", argv[0]);
     fprintf(stderr, "\t or %s <command_file >>&output_file\n", argv[0]);
     exit(1);
   }

  printf("Labnet Equipment Controller Test Program\n");
  printf("------------------------------------------- \n");
  device_fd = open(LABEQC_DEVICE, O_RDWR);
  if (device_fd < 0)
   {
     perror("open unsuccessful");
     return (-1);
   }

  event_list = createList("LABEQC");

  if ( (result = load_events()) < 0)
   {
     printf("There is an error loading the input file, ");
     printf("please check that file \n");
     exit (-1);
   }

  done_list = createList("Output List");

  // WE START A LOOP FOR CONTINUOUS TESTING HERE
  while ( event_list->number > 0 )
   {
     LABEQC_Instruction instruction[MAX_SEQUENCE];
     LABEQC_Instruction_List instruction_list;
     cmd_count = 0;
     while (cmd_count < MAX_SEQUENCE)
      {
        entry = removeElement(event_list);
        obj = (LISTOBJ *)(entry->Object);
        if (obj->isSpace)
         {
           addElement(done_list, entry);
           break;
         }
        if (obj->isComment)
         {
           addElement(done_list, entry);
           if (event_list->number == 0)
             break;
           continue;
         }
        instruction[cmd_count] = *(obj->instruction);
        obj->instruction = &(instruction[cmd_count]);
        addElement(done_list, entry);
        cmd_count++;
        if (event_list->number == 0)
          break;
      }

     if (cmd_count == 0)  // We have found an extra space, so we will
       continue;          // jump back to the top of the loop and start again

     // send out command
     printf("Sending ioctl command with %d sub-commands\n", cmd_count);
     instruction_list.num_instructions = cmd_count;
     instruction_list.instructions = instruction;
     err = ioctl(device_fd, LABEQC_INSTRUCTION_LIST_REQ, &instruction_list);
     if (err < 0)
      {
        perror("ERROR: ioctl failed");
        close(device_fd);
        return (-1);
      }
     printf("Note: ioctl successful for %d commands\n", cmd_count);
	 
     // We do extra processing for an Auto or manual Read
     for (i = 0; i < cmd_count; i++)
      {
	if ((instruction[i].type == MANUAL_WRITE) ||
	    (instruction[i].type <= MAX_AUTO_INSTRUCTION))
	  lastcommand = (unsigned char) instruction[i].command;
        if ((instruction[i].type <= MAX_AUTO_INSTRUCTION) ||
            (instruction[i].type == MANUAL_READ))
         {
           sOvl = (instruction[i].even_result & 0x8) >> 3;
           rOvl = (instruction[i].even_result & 0x4) >> 2;
           adcDone = (instruction[i].even_result & 0x2) >> 1;
           busy = instruction[i].even_result & 0x1;
	   obj = (LISTOBJ *)(instruction[i].container);
	   sprintf(obj->line, "%s      sOvl: %d, rOvl: %d, adcDone: %d, busy: %d", obj->line, sOvl, rOvl, adcDone, busy);
        
  	   // extract the value from the adc
	   instruction[i].even_result = instruction[i].even_result & 0xF0;
	   instruction[i].even_result = instruction[i].even_result >> 4;
	   adcValue = instruction[i].odd_result;
	   adcValue = adcValue * 16;
	   adcValue += instruction[i].even_result;
	   sprintf(obj->line, "%s, ADC = %03X (", obj->line, adcValue);
	   d2 = (lastcommand & 0x4) >> 2;
	   d1 = (lastcommand & 0x2) >> 1;
	   if (adcValue > 2047)
	     returnValue = 0 - (4095 - adcValue);
	   else
	     returnValue = adcValue;

	   if ((d2 == 0) && (d1 == 0))
	     returnValue = returnValue * 12.21;
	   else
	     if (d2 == 1)
	       returnValue = returnValue * 5;
	     else
	       returnValue = returnValue * 73.26;
	   sprintf(obj->line, "%s%d)\n", obj->line, returnValue);
         }
       }
   }

  close(device_fd);
  output_results();
  return (0);
}
