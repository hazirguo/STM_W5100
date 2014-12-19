/*****************************************************************
 *                  EQSLAVE.C
 *
 *  PURPOSE: the underlying daemon that handles the actual communication
 *           with the driver via the ioctl call.
 *
 *
 *  EDIT HISTORY:
 *     Programmer          Change                  Date
 *     -----------------------------------------------------------
 *     Thomas Lohman      Initial Version         11/10/1999
 *     Thomas Lohman      Incorporated changes    09/24/2008
 *                        from Frank Hess @ NIST.
 *
 *  FUNCTION LIST:
 *     main()
 *
 ******************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define EQ_DAEMON
#include "../include/labeqc.h"

/****** external global functions *****************/

/****** global functions **************************/

/****** static functions **************************/

/****** external global variables *****************/

/****** global variables **************************/

/****** static variables **************************/
static char *homedir = EQC_HOME;


/******************************
 *                             *
 *      main.c                 *
 *                             *
 ******************************/
int main (int argc, char *argv[])
{
    unsigned int i, size, d1, d2;
    unsigned char lastcommand = 0;
    char buf[81];
    EQDAEMON_Struct val;
    LABEQC_Instruction_List drlist;
    LABEQC_Instruction *drinstr;
    int devicefd, adcValue;

    chdir(homedir);

    /* open the Labeqd Error log file for appending */
    sprintf(buf, "%s/%s", homedir, EQ_ERR_LOG);
    if ((DaemonErrLog = fopen(buf, "a")) == (FILE *) NULL)
     {
        fprintf(stderr, "EqSlave: Could not open Error log, exiting.");
        exit(-1);
     }

    /* open the Labeqd System log file for appending */
    sprintf(buf, "%s/%s", homedir, EQ_SYS_LOG);
    if ((DaemonSysLog = fopen(buf, "a")) == (FILE *) NULL)
     {
        fprintf(stderr, "EqSlave: Could not open System log, exiting.");
        exit(-1);
     }

    // open the underlying device
    if ((devicefd = open(LABEQC_DEVICE, O_RDWR)) < 0)
     {
        EQWriteSystemErrorLog("Driver open failed", "open", errno);
        exit(-1);
     }

    /* we continually loop, reading whatever data is sent to us via STDIN */
    while (fread((void *) &val, sizeof(EQDAEMON_Struct), 1, stdin) > 0)
     {
        size = val.instrcount * sizeof(LABEQC_Instruction);
        if (size <= 0)
         {
            // Invalid instuction count.  Send the data back.
            EQWriteErrorLog("Invalid data structure sent from daemon");
            if (fwrite((void *) &val, sizeof(EQDAEMON_Struct), 1, stdout) != 1)
             {
                EQWriteSystemErrorLog("EqSlave: Problem writing invalid data back to daemon", "fwrite", errno);
             }
            fflush(stdout);
            continue;
         }
        if ((drlist.instructions = (LABEQC_Instruction *) malloc(size)) == NULL)
         {
            EQWriteSystemErrorLog("Malloc failed for drlist", "malloc", errno);
            exit(-1);
         }

        drlist.num_instructions = val.instrcount;
        drinstr = drlist.instructions;

        for (i = 0; i < val.instrcount; i++)
         {
            drinstr->magic_num = MAGIC_NUM;
            drinstr->even_port = val.evenport;
            drinstr->odd_port  = val.oddport;
            drinstr->card_id   = val.cardid;
            drinstr->channel   = val.channel;
            drinstr->type     = val.command[i].ctype;
            drinstr->command   = val.command[i].write_command;
            drinstr->wait_time = val.command[i].delay_time;
            drinstr->container = (void *) NULL;

            drinstr++;
         }

        /* send the request to the driver */
        if (ioctl(devicefd, LABEQC_INSTRUCTION_LIST_REQ, &drlist) < 0)
         {
            EQWriteSystemErrorLog("Driver ioctl failed", "ioctl", errno);
            val.ok = FALSE;
            /* send the data back via STDOUT */
            if (fwrite((void *) &val, sizeof(EQDAEMON_Struct), 1, stdout) != 1)
             {
                EQWriteSystemErrorLog("EqSlave: Problem writing failed ioctl data", "fwrite", errno);
             }
            fflush(stdout);
            continue;
         }
        else
            val.ok = TRUE;

        drinstr = drlist.instructions;
        for (i = 0; i < val.instrcount; i++)
         {
            if ((drinstr->type == MANUAL_WRITE) ||
                (drinstr->type <= MAX_AUTO_INSTRUCTION))
                lastcommand = (unsigned char) drinstr->command;

            if ((drinstr->type == MANUAL_READ) ||
                (drinstr->type <= MAX_AUTO_INSTRUCTION))
             {
                val.command[i].sdOverload = (drinstr->even_result & 0x8) >> 3;
                val.command[i].rdOverload = (drinstr->even_result & 0x4) >> 2;
                val.command[i].adcDone    = (drinstr->even_result & 0x2) >> 1;
                val.command[i].busy       = drinstr->even_result & 0x1;

                /* there is a special case for channel 0 */
                if (drinstr->channel == 0)
                    val.command[i].returnValue = (int) drinstr->odd_result;
                else
                 {
                    // extract the value from the adc
                    drinstr->even_result = drinstr->even_result >> 4;
                    adcValue = drinstr->odd_result;
                    adcValue = adcValue * 16;
                    adcValue += drinstr->even_result;

                    d2 = (lastcommand & 0x4) >> 2;
                    d1 = (lastcommand & 0x2) >> 1;
                    if (adcValue > 2047)
                        adcValue = 0 - (4095 - adcValue);

                    if ((d2 == 0) && (d1 == 0))
                      val.command[i].returnValue = adcValue * 12.21;
                    else
                    if (d2 == 1)
                      val.command[i].returnValue = adcValue * 5;
                    else
                      val.command[i].returnValue = adcValue * 73.26;
                 }
        		sprintf(buf, "sovl/rovl/adcDone/Busy: %d/%d/%d/%d, returnValue: %d\n",
                            val.command[i].sdOverload,
                            val.command[i].rdOverload,
                            val.command[i].adcDone,
                            val.command[i].busy,
                            val.command[i].returnValue);
                EQWriteSystemLog(buf);
             }
            else
             {
                val.command[i].sdOverload  = 0;
                val.command[i].rdOverload  = 0;
                val.command[i].adcDone     = 0;
                val.command[i].busy        = 0;
                val.command[i].returnValue = 0;
             }

            drinstr++;
         }

        /* send the data back via STDOUT */
        if (fwrite((void *) &val, sizeof(EQDAEMON_Struct), 1, stdout) != 1)
         {
            EQWriteSystemErrorLog("EqSlave: Problem writing back to master", "fwrite", errno);
         }
        fflush(stdout);
     }

    close(devicefd);

    exit(0);
}
