/*****************************************************************
 *                    LOGUTILS.C
 *
 *  PURPOSE: Utilities used for logging error and system messages
 *          
 *
 *  EDIT HISTORY:
 *     Programmer          Change                  Date
 *     -----------------------------------------------------------
 *     Thomas Lohman      Initial Version         11/10/1999
 *
 *  FUNCTION LIST:
 *     EQWriteSystemLog
 *     EQWriteErrorLog
 *     EQWriteSystemErrorLog
 *
 ******************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define EQ_DAEMON
#include "../include/labeqc.h"

/****** external global functions *****************/

/****** global functions **************************/

/****** static functions **************************/

/****** external global variables *****************/

/****** global variables **************************/

FILE *DaemonSysLog;
FILE *DaemonErrLog;

/****** static variables **************************/


/*******************************
 *                             *
 *  EQWriteSystemLog.c         *
 *                             *
 ******************************/
void EQWriteSystemLog (const char *mesg) 
{
    FILE *log = DaemonSysLog;

    time_t current_time = time(NULL);
    const char *time_string = ctime(&current_time);
 
    fwrite(time_string, 1, strlen(time_string) - 1, log);
    fwrite(": ", 1, 2, log);
    fwrite(mesg, 1, strlen(mesg), log);
    fwrite("\n", 1, 1, log);

    fflush(log);
}


/*******************************
 *                             *
 *  EQWriteErrorLog.c          *
 *                             *
 ******************************/
void EQWriteErrorLog (const char *mesg) 
{
    FILE *log = DaemonErrLog;

    time_t current_time = time(NULL);
    const char *time_string = ctime(&current_time);

    fwrite(time_string, 1, strlen(time_string) - 1, log);
    fwrite(": ", 1, 2, log);
    fwrite(mesg, 1, strlen(mesg), log);
    fwrite("\n ", 1, 1, log);

    fflush(log);
}


/*******************************
 *                             *
 *  EQWriteSystemErrorLog.c    *
 *                             *
 ******************************/
void EQWriteSystemErrorLog (const char *mesg, const char *funcname, int error) 
{
  FILE *log = DaemonErrLog;
  char strint[6];

  time_t current_time = time(NULL);
  const char *time_string = ctime(&current_time);

  fwrite(time_string, 1, strlen(time_string) - 1, log);
  fwrite(": ", 1, 2, log);
  fwrite(mesg, 1, strlen(mesg), log);
  fwrite("\n ", 1, 1, log);
  if ((strerror(error) != NULL) && (errno != 0))
   {
     fwrite("\t\t\t", 1, strlen("\t\t\t"), log);
     fwrite(funcname, 1, strlen(funcname), log);
     fwrite(" System Error: ", 1, strlen(" System Error: "), log);
     sprintf(strint, "%d", error);
     fwrite(strint, 1, strlen(strint), log);
     fwrite(" --> ", 1, strlen(" --> "), log);
     fwrite(strerror(error), 1, strlen(strerror(error)), log);
     fwrite("\n ", 1, 1, log);
   }
  else
   {
     fwrite("\t\t\t", 1, strlen("\t\t\t"), log);
     fwrite(funcname, 1, strlen(funcname), log);
     fwrite(" System Error: ", 1, strlen(" System Error: "), log);
     sprintf(strint, "%d", error);
     fwrite(strint, 1, strlen(strint), log);
     fwrite(" --> ", 1, strlen(" --> "), log);
     fwrite("No accompanying text message", 1, strlen("No accompanying text message"), log);
     fwrite("\n ", 1, 1, log);
   }

  fflush(log);
}
