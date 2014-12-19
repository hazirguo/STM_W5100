/*****************************************************************
 *                     LABEQD.C
 *
 *  PURPOSE: the main low level equipment daemon that sits on top and manages
 *           all the eqslave processes.
 *
 *  EDIT HISTORY:
 *     Programmer          Change                  Date
 *     -----------------------------------------------------------
 *     Thomas Lohman      Initial Version         11/10/1999 
 *     Thomas Lohman      Incorporated changes    09/24/2008
 *                        from Frank Hess @ NIST.
 *     Thomas Lohman      Added patch for handling 04/17/2009
 *                        requests coming for 
 *                        non-existent cards.
 *
 *  FUNCTION LIST:
 *     main()
 *     EqInit()
 *
 ******************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pwd.h>
#include <tcpd.h>

#define EQ_DAEMON
#include "../include/labeqc.h"

/* 
   An instruction record for the equipment daemon is made up of a 
   specific channel on a card, the type of command to be done and 
   depending on that command type, the actual written command (for a
   MANUAL_WRITE) as well as the wait time (for a MANUAL_DELAY).

   Each Master card has a separate queue associated with it with the list
   of outstanding instructions.  In addition, it also has a file descriptor
   that is used to communicate with its slave process.

   Each card on the system has a card number, an even/odd port pair, and
   a pointer to the Master Card record for it.
*/
/*
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
 */

typedef struct MasterQueue {
    FILE *AcceptFile;
    EQDAEMON_Struct val;
    struct MasterQueue *next;
    } MasterQueue;

typedef struct MasterCard {
    int SlaveFd;
    unsigned int Busy;
    MasterQueue *firstqueue;
    MasterQueue *lastqueue;
    struct MasterCard *next;
} MasterCard;

typedef struct Card {
    unsigned int cardid;
    unsigned int evenport;
    unsigned int oddport;
    MasterCard *mcard;
    struct Card *next;
} Card;

/****** external global functions *****************/

/****** global functions **************************/

/****** static functions **************************/
static ssize_t complete_read(int, void *, size_t);
static ssize_t complete_write(int, const void *, size_t);
static void EqInit();

/****** external global variables *****************/


/****** global variables **************************/

/****** static variables **************************/
static MasterCard *FirstMasterCard = (MasterCard *) NULL;
static Card *FirstCard = (Card *) NULL;
static char *homedir = EQC_HOME;


/******************************
*                             *
*  complete_read.c            *
*                             *
******************************/
//read count bytes data from fd to buf, return actually reading size
static ssize_t complete_read(int fd, void *buf, size_t count)
{
  ssize_t retval;
  ssize_t total_count = 0;
  while (count > 0)
   {
     if ((retval = read(fd, buf, count)) < 0)
       return (total_count ? total_count : retval);

     count -= retval;
     buf += retval;
     total_count += retval;
   }
  return (total_count);
}


/******************************
*                             *
*   complete_write.c          *
*                             *
******************************/
static ssize_t complete_write(int fd, const void *buf, size_t count)
{
  ssize_t retval;
  ssize_t total_count = 0;
  while (count > 0)
   {
     if ((retval = write(fd, buf, count)) < 0)
       return (total_count ? total_count : retval);

     count -= retval;
     buf += retval;
     total_count += retval;
   }
  return (total_count);
}


/******************************
 *                             *
 *    main.c                   *
 *                             *
 ******************************/
int main(int argc, char *argv[], char *envp[]) 
{
  int i, x, ParentSock, ChildSock, HighFd;
  unsigned char buffer[1024];
  struct sockaddr_in sockin, nsockin;
  struct hostent *hp;
  struct protoent *pp;
  struct servent *sv;
  fd_set ibits, obits;
  MasterCard *mcptr;
  MasterQueue *mqptr, *tmpmq;
  Card *cptr;
  char buf[1024], hostname[64], clientip[32];
  int tmpint, pid;
  struct passwd *userpwd;
  FILE *fptr;
  socklen_t fromlen;
  EQDAEMON_Struct val;


  /* 
     First thing to do is to fork into another process and
     have this process exit.  In addition, the new process
     creates a new process session with no controlling tty.
  */
  //create a daemon process
  if ((pid = fork()) == 0)
   {
     /* this is the child */
 
     /* create our own session and release from the terminal */
     if (setsid() < 0)
       exit(-1);

     /* look for the equipment controller user's password record */
     // getpwnam -- use to get the info of login user
    /*
    struct passwd {
       char * pw_name; /* Username. * /
       char * pw_passwd; /* Password. * /
       __uid_t -pw_uid; /* User ID. * /
       __gid_t -pw_gid; /* Group ID. * /
       char * pw_gecos; /* Real name. * /
       char * pw_dir; /* Home directory. -* /
       char * pw_shell; /* Shell program.  /
    };
    */
     if ((userpwd = getpwnam(EQC_USERNAME)) == (struct passwd *) NULL)
       exit(-1);

     /* become the equipment controller user at this point */
     if (setgid(userpwd->pw_gid) < 0)
       exit(-1);

     if (setuid(userpwd->pw_uid) < 0)
       exit(-1);

     if (chdir(homedir) < 0)
        exit(-1);
   
     /* initialize all slaves based on the cards we have */
     EqInit();

     /* Next we initialize our socket connection and enter our main loop */

     /* get our hostname */
     if (gethostname(hostname, sizeof(hostname) - 1) < 0) 
      {
        EQWriteSystemErrorLog("gethostname failed", "gethostname", errno);
        exit(-1);
      }
     /* get our hostname information over the network. */
     if ((hp = gethostbyname(hostname)) == (struct hostent *) NULL) 
      {
        EQWriteSystemErrorLog("gethostbyname failed", "gethostbyname", errno);
        exit(-1);
      }

     /* get tcp protocol */
     if ((pp = getprotobyname("tcp")) == (struct protoent *) NULL) 
      {
        EQWriteSystemErrorLog("getprotobyname failed", "getprotobyname", errno);
        exit(-1);
      }

     /* get the equipment daemon service */
     if ((sv = getservbyname(SERVICE_NAME, SERVICE_TYPE)) == (struct servent *) NULL) 
      {
        EQWriteSystemErrorLog("getservbyname failed", "getservbyname", errno);
        exit(-1);
      }
   
     memset((char *) &sockin, 0, sizeof(struct sockaddr_in));

     if ((ParentSock = socket(AF_INET, SOCK_STREAM, pp->p_proto)) < 0) 
      {
        EQWriteSystemErrorLog("socket for parent failed", "socket", errno);
        exit(-1);
      }

     sockin.sin_family = AF_INET;
     sockin.sin_port = sv->s_port;
     memcpy(&sockin.sin_addr, hp->h_addr, hp->h_length);
     
     /* bind to the address */
     if (bind(ParentSock, (struct sockaddr *) &sockin, sizeof(sockin)) < 0) 
      {
        EQWriteSystemErrorLog("ParentSock bind failed", "bind", errno);
        exit(-1);
      }

     if (listen(ParentSock, 5) < 0) 
      {
        EQWriteSystemErrorLog("ParentSock listen failed", "listen", errno);
        exit(-1);
      }

     HighFd = ParentSock;
     mcptr = FirstMasterCard;
     //find the max fd num
     while (mcptr != (MasterCard *) NULL) 
      {
        if (mcptr->SlaveFd > HighFd)
          HighFd = mcptr->SlaveFd;
        mcptr = mcptr->next;
      }

     while (TRUE) 
      {
        FD_ZERO(&ibits);
        FD_ZERO(&obits);

        FD_SET(ParentSock, &ibits);

        /* loop through and put all the MasterCard file descriptors in */
        mcptr = FirstMasterCard;
        while (mcptr != (MasterCard *) NULL) 
         {
           FD_SET(mcptr->SlaveFd, &ibits);
           mcptr = mcptr->next;
         }

        errno = 0;
        if (((x = select(HighFd + 1, &ibits, &obits, (fd_set *) NULL,
                         (struct timeval *) NULL)) < 0) && (errno == EINTR))
          continue;

        if (x < 0) 
         {
           EQWriteSystemErrorLog("select failed", "select", errno);
           exit(-1);
         }

        //event occurs
        /* First, check to see if we're getting data back from any slaves */
        mcptr = FirstMasterCard;
        while (mcptr != (MasterCard *) NULL) 
         {
           /*
            * If this slave process has something to return then:

            * (1) read the data from the file descriptor.
            * (2) Forward the returned data back over the AcceptFile.
            * (3) Pop the finished request from its queue.
            * (4) Send the next request if one exists.
           */
           if (FD_ISSET(mcptr->SlaveFd, &ibits))
            {
              /* read the data */
              if (complete_read(mcptr->SlaveFd, &val, sizeof(EQDAEMON_Struct)) < sizeof(EQDAEMON_Struct)) 
               {
                 EQWriteSystemErrorLog("Read from slave failed", "read", errno);
                 exit(-1);
               }
           
              for (i = 0; i < val.instrcount; i++)
               {
                   tmpint = val.command[i].ctype;
                   val.command[i].ctype = htonl(tmpint);
                   tmpint = val.command[i].write_command;
                   val.command[i].write_command = htonl(tmpint);
                   tmpint = val.command[i].delay_time;
                   val.command[i].delay_time = htonl(tmpint);
                   tmpint = val.command[i].returnValue;
                   val.command[i].returnValue = htonl(tmpint);
               }

              /* convert the data from eqslave to network byte order */
                tmpint = val.instrcount;
                val.instrcount = htonl(tmpint);
                tmpint = val.cardid;
                val.cardid = htonl(tmpint);
                tmpint = val.evenport;
                val.evenport = htonl(tmpint);
                tmpint = val.oddport;
                val.oddport = htonl(tmpint);
                tmpint = val.channel;
                val.channel = htonl(tmpint);
                tmpint = val.ok;
                val.ok = htonl(tmpint);

              /* write the data back to the process that made the request */
              if (fwrite(&val, sizeof(EQDAEMON_Struct), 1, mcptr->firstqueue->AcceptFile) != 1)
               {
                 EQWriteSystemErrorLog("Write to calling process failed", "fwrite", errno);
                 exit(-1);
               }
              else
               {
                 /* After data is returned, we are done with AcceptFile */
                 /* Closing AcceptFile closes ChildSock for this request */
                 fclose(mcptr->firstqueue->AcceptFile);
               }
           
              mqptr = mcptr->firstqueue;
              if (mqptr == mcptr->lastqueue)
                mcptr->lastqueue = mqptr->next; //NULL
              mcptr->firstqueue = mqptr->next;
              free(mqptr);

              /* send the next request if there is one */
              if (mcptr->firstqueue != (MasterQueue *) NULL) 
               {
                 mqptr = mcptr->firstqueue;
                 if (complete_write(mcptr->SlaveFd, &mqptr->val, sizeof(EQDAEMON_Struct)) != sizeof(EQDAEMON_Struct)) 
                  {
                    EQWriteSystemErrorLog("Write to slave process failed", "write", errno);
                    exit(-1);
                  }
               }
              else
                mcptr->Busy = FALSE;
            } //end of 'if (FD_ISSET(mcptr->SlaveFd, &ibits))'
           mcptr = mcptr->next;
         } //end of 'while (mcptr != (MasterCard *) NULL)'

        /*
         * If there is a request coming in to our daemon, then:

         * (1) Run accept, get a socket to handle the request.
         * (2) Read data coming in.  Check format for validity.
         * (3) Determine the slave process we want and check its busy flag.
         * (4) If slave process is busy, add new request to the queue.
         * (5) If slave is idle, send the request.
         */
        if (FD_ISSET(ParentSock, &ibits)) 
         {
             /* initialize the data structures before calling accept */
            /* this changed from RH9 to RH ES */
            memset((char *) &nsockin, 0, sizeof(struct sockaddr_in));
            fromlen  = sizeof(struct sockaddr_in);

            ChildSock = accept(ParentSock, (struct sockaddr *) &nsockin, &fromlen);
             sprintf(buf, "Handling new request for daemon (ChildSock = %d):",
                     ChildSock);
            EQWriteSystemLog(buf);
             if (ChildSock < 0)
             {
                if (errno != EINTR)
                 {
                    EQWriteSystemErrorLog("Accept of incoming data failed", "accept", errno);
                    sleep(3);
                 }
                continue;
             }

            /* check to see if the request is allowed via tcp wrappers */
            strcpy(clientip, inet_ntoa(nsockin.sin_addr)); 
            if (hosts_ctl(SERVICE_NAME, STRING_UNKNOWN, clientip, STRING_UNKNOWN) == 0)
             {
               sprintf(buf, "Access from %s is not allowed.", clientip);
               EQWriteSystemLog(buf);
               close(ChildSock);
               continue;
             }
            
            /* set the socket descriptor for blocking I/O */
            x = 0;
            if (ioctl(ChildSock, FIONBIO, (char *) &x) < 0) 
             {
                EQWriteSystemErrorLog("Blocking ioctl failed", "ioctl", errno);
                close(ChildSock);
                continue;
             }

            /* read from the socket - block until we get something */
            if ((x = read(ChildSock, buffer, sizeof(buffer))) < 0) 
             {
                EQWriteSystemErrorLog("Read from ChildSock failed", "read", errno);
                close(ChildSock);
                continue;
             }

            /* check for the begin and end handshake of the request */
            if ((memcmp(buffer, BEGIN_HANDSHAKE_STRING, strlen(BEGIN_HANDSHAKE_STRING)) != 0) ||
                (memcmp(buffer + (x - strlen(END_HANDSHAKE_STRING)),
                        END_HANDSHAKE_STRING, strlen(END_HANDSHAKE_STRING)) != 0))
            {
               sprintf(buf, "Illegal request from %s is not allowed.", clientip);
               EQWriteSystemLog(buf);
               close(ChildSock);
               continue;
             }

            /* allocate out a MasterQueue record to hold this request */
            if ((tmpmq = (MasterQueue *) malloc(sizeof(MasterQueue))) == NULL) 
             {
                EQWriteSystemErrorLog("Malloc for incoming request failed", "malloc", errno);
                exit(-1);
             }

            memset((void *) &(tmpmq->val), 0, sizeof(EQDAEMON_Struct));
            memcpy((void *) &(tmpmq->val), (const void *) buffer + strlen(BEGIN_HANDSHAKE_STRING), x - (strlen(BEGIN_HANDSHAKE_STRING) + strlen(END_HANDSHAKE_STRING)));
            tmpmq->AcceptFile = fdopen(ChildSock, "w");
            tmpmq->next = (MasterQueue *) NULL;

            /* convert the instruction count, cardid, even and odd ports to host byte order */
            /* we need these to figure out what master card we're using or if the card */
            /* requested exists at all. We'll convert everything else once we know that */
            /* we have a real existing card to deal with. */

            tmpint = tmpmq->val.instrcount;            
            tmpmq->val.instrcount = ntohl(tmpint);
             tmpint = tmpmq->val.cardid;
             tmpmq->val.cardid = ntohl(tmpint);
             tmpint = tmpmq->val.evenport;
             tmpmq->val.evenport = ntohl(tmpint);
             tmpint = tmpmq->val.oddport;
             tmpmq->val.oddport = ntohl(tmpint);

            /* which Mastercard are we dealing with here */
            cptr = FirstCard;
            mcptr = (MasterCard *) NULL;
            while (cptr != (Card *) NULL)
             {
                if ((cptr->cardid == tmpmq->val.cardid) &&
                    (cptr->evenport == tmpmq->val.evenport) &&
                    (cptr->oddport == tmpmq->val.oddport))
                 {
                    mcptr = cptr->mcard;
                    break;
                 }
                cptr = cptr->next;
             }

            /* the card doesn't exist - ignore the request */
            if (mcptr == (MasterCard *) NULL)
            {
                sprintf(buf, "Card: %d does not exist - ignoring this request", tmpmq->val.cardid);
                EQWriteSystemLog(buf);

               tmpmq->val.ok = FALSE;
                for (i = 0; i < tmpmq->val.instrcount; i++)
                {
                    tmpmq->val.command[i].sdOverload  = 0;
                    tmpmq->val.command[i].rdOverload  = 0;
                    tmpmq->val.command[i].adcDone     = 0;
                    tmpmq->val.command[i].busy        = 0;
                    tmpmq->val.command[i].returnValue = 0;
                }

                /* convert the converted original data back to network byte order */
                tmpint = tmpmq->val.instrcount;
                tmpmq->val.instrcount = htonl(tmpint);
                tmpint = tmpmq->val.cardid;
                tmpmq->val.cardid = htonl(tmpint);
                tmpint = tmpmq->val.evenport;
                tmpmq->val.evenport = htonl(tmpint);
                tmpint = tmpmq->val.oddport;
                tmpmq->val.oddport = htonl(tmpint);
                tmpint = tmpmq->val.ok;
                tmpmq->val.ok = htonl(tmpint);

                /* write original request data back to the process that made the request */
                if (fwrite(&(tmpmq->val), sizeof(EQDAEMON_Struct), 1, tmpmq->AcceptFile) != 1)
                {
                    EQWriteSystemErrorLog("Write to calling process failed", "fwrite", errno);
                    exit(-1);
                }
                else
                {
                    fclose(tmpmq->AcceptFile);
                    free(tmpmq);
                    continue;
                }
            } //end of 'if (mcptr == (MasterCard *) NULL)'

            //the card exist
            /* convert the rest of our data to host byte order from network byte order */
            tmpint = tmpmq->val.channel;            
            tmpmq->val.channel = ntohl(tmpint);
            tmpint = tmpmq->val.ok;            
            tmpmq->val.ok = ntohl(tmpint);

            for (i = 0; i < tmpmq->val.instrcount; i++)
             {
               tmpint = tmpmq->val.command[i].ctype;
               tmpmq->val.command[i].ctype = ntohl(tmpint);
               tmpint = tmpmq->val.command[i].write_command;
               tmpmq->val.command[i].write_command = ntohl(tmpint);
               tmpint = tmpmq->val.command[i].delay_time;
               tmpmq->val.command[i].delay_time = ntohl(tmpint);
               tmpint = tmpmq->val.command[i].returnValue;
               tmpmq->val.command[i].returnValue = ntohl(tmpint);
             }

             /* Log the commands to the log */
             sprintf(buf, "card/relay: %d/%d, ports: %d/%d, instructions: %d",
                     tmpmq->val.cardid, 
                     tmpmq->val.channel, 
                     tmpmq->val.evenport,
                     tmpmq->val.oddport,
                     tmpmq->val.instrcount);

             EQWriteSystemLog(buf);

             for (i = 0; i < tmpmq->val.instrcount; i++)
             {
                 sprintf(buf, "instruction: %d, ctype: %d, cmd: %d, delay: %d",
                            i,
                            tmpmq->val.command[i].ctype,
                            tmpmq->val.command[i].write_command,
                            tmpmq->val.command[i].delay_time);
                 EQWriteSystemLog(buf);
             }

            /* Add the MasterQueue record to its queue list */
            mqptr = mcptr->firstqueue;
            if (mqptr == (MasterQueue *) NULL) 
             {
                mcptr->firstqueue = tmpmq;
                mcptr->lastqueue = tmpmq;
             }
            else 
             {
                mcptr->lastqueue->next = tmpmq;
                mcptr->lastqueue = tmpmq;
             }
        
            /* If we're not busy, send the request directly */
            if (mcptr->Busy == FALSE) 
             {
                EQWriteSystemLog("Sending to master card driver ...");
                if (complete_write(mcptr->SlaveFd, &tmpmq->val, sizeof(EQDAEMON_Struct)) != sizeof(EQDAEMON_Struct)) 
                 {
                    EQWriteSystemErrorLog("Write to slave process failed", "write", errno);
                    exit(-1);
                 }
                mcptr->Busy = TRUE;   
             }
            else
             {
                 EQWriteSystemLog("Master is currently busy ... try later");
             }
         } //endo of 'if (FD_ISSET(ParentSock, &ibits)) '
      }  //end of 'while (TRUE)'
   }  //end of 'fork() == 0'
  else
   {
     if (pid > 0)   //parent process, write child process pid to labeqd.pid, then exit
      {
          if ((fptr = fopen("/var/run/labeqd.pid", "w+")) != (FILE *) NULL)
          {
           fprintf(fptr, "%d\n", pid); 
           fclose(fptr);
          }
          exit(0);
      }
     else
       exit(-1);
   } 
}


/******************************
 *                             *
 *    EqInit.c                 *
 *                             *
 ******************************/
static void EqInit() 
{
    char buffer[81], buf[81];
    LABEQC_Instruction_List instruction_list;
    LABEQC_Instruction instruction[16];
    unsigned int commandEcho = 0, evenport, oddport;
    /* struct MasterQueue *queue; */
    struct MasterCard *master, *zero = NULL, *lastmaster = NULL;
    struct Card *card, *lastcard = NULL;
    sigset_t zeromask;
    int fds[2], i, j, LabEqcFD = -1;
    FILE *fileptr;  
    pid_t pid;

    /* open the ERR log file for appending */
    sprintf(buffer, "%s/%s", homedir, EQ_ERR_LOG);
    if ((DaemonErrLog = fopen(buffer, "a")) == (FILE *) NULL) 
     {
        fprintf(stderr, "EqInit: Could not open daemon Error log, exiting.");
        exit(-1);
     }

    /* open the SYS log file for appending */
    sprintf(buffer, "%s/%s", homedir, EQ_SYS_LOG);
    if ((DaemonSysLog = fopen(buffer, "a")) == (FILE *) NULL) 
     {
        fprintf(stderr, "EqInit: Could not open daemon System log, exiting.");
        exit(-1);
     }

    EQWriteSystemLog("EqInit() has begun.");

    /* open the underlying device */
    if ((LabEqcFD = open(LABEQC_DEVICE, O_RDWR)) < 0) 
     {
        EQWriteSystemErrorLog("EqInit: open failed", "open", errno);
        exit(-1);
     }

    /* open the equip control config file for reading */
    sprintf(buffer, "%s/%s", homedir, EQCONFFILE);
    if ((fileptr = fopen(buffer, "r")) == (FILE *) NULL) 
     {
        EQWriteSystemErrorLog("EqInit: The configuration file could not be opened", "fopen", errno);
        exit(-1);
     }

    /* loop through the file and figure out what cards we have */
    while (!feof(fileptr))
     {
        if (fgets(buffer, sizeof(buffer), fileptr)) //for each eq configuration
         {
            /* if the line is commented or empty then ignore it */
            if ((buffer[0] == '#') || (buffer[0] == '\0'))
                continue;

            if (sscanf(buffer, "%X:%X", &evenport, &oddport) != 2)
                continue;

            EQWriteSystemLog("Successfully read line of eqctab.");
 
            j = 15;
            for (i = 0; i < 16; i++) //initialize 16 instructions
             {
                instruction[i].magic_num = MAGIC_NUM;
                instruction[i].even_port = evenport;
                instruction[i].odd_port = oddport;
                instruction[i].container = (void *) NULL;
                instruction[i].wait_time = 0;
                instruction[i].odd_result = 0;
                instruction[i].even_result = 0;
     
                if (i < 8)
                 {
                    instruction[i].type = MANUAL_WRITE;
                    instruction[i].card_id = j - 8;
                    instruction[i].channel = 0;
                    instruction[i].command = j - 8;
                    j--;
                 }
                else 
                 {
                    instruction[i].type = MANUAL_READ;
                    instruction[i].card_id = j;
                    instruction[i].channel = 0;
                    instruction[i].command = 0;    
                    j--;
                 }
             }//end of 'for(i = 0; i < 16; i++)'

            /* send the request to the driver */
             instruction_list.instructions = instruction;
             instruction_list.num_instructions = sizeof (instruction) / sizeof (instruction[0]);
             if (ioctl(LabEqcFD, LABEQC_INSTRUCTION_LIST_REQ, &instruction_list) < 0)
             {
                EQWriteSystemErrorLog("EqInit: ioctl failed", "ioctl", errno);
                exit(-1);
             }

            /* look at the results starting with card 0 */
            for (i = 15; i > 7; i--) 
             {
                commandEcho = instruction[i].odd_result;

                /* the card doesn't exist if commandEcho = 255 = 0xFF */
                if (commandEcho != 255) 
                 {
                    if ((card = (Card *) malloc(sizeof(Card))) == NULL) 
                     {
                        EQWriteSystemErrorLog("EqInit: malloc failed for Card.", "malloc", errno);
                        exit(-1);
                     }
                    card->cardid  = instruction[i].card_id;
                    card->evenport = evenport;
                    card->oddport  = oddport;
                    card->next     = (Card *) NULL;  

                    if (FirstCard == (Card *) NULL)
                        FirstCard = card;
                    if (lastcard != (Card *) NULL)
                        lastcard->next = card;
                    lastcard = card;

                    /* the card is a master */
                    if (commandEcho == instruction[i].card_id) 
                     {
                        sprintf(buf, "Found a master card at %d", instruction[i].card_id);
                        EQWriteSystemLog(buf);
                        /* allocate the appropriate structures */
                        if ((master = (MasterCard *) malloc(sizeof(MasterCard))) == NULL) 
                         {
                            EQWriteSystemErrorLog("EqInit: malloc failed for MasterCard.", "malloc", errno);
                            exit(-1);
                         } 
                        master->firstqueue = (MasterQueue *) NULL;
                        master->lastqueue = (MasterQueue *) NULL;
                        master->next = (MasterCard *) NULL;
                        master->Busy = FALSE;

                        card->mcard = master;
                 
                        /* Zero must be a master card if we want slave cards */
                        if (instruction[i].card_id == 0)
                            zero = master;

                        if (FirstMasterCard == (MasterCard *) NULL)
                            FirstMasterCard = master;

                        if (lastmaster != NULL)
                            lastmaster->next = master;
                        lastmaster = master;

                        /* if we're a master we must fork a slave process */
                        sprintf(buf, "_labeqd%d", commandEcho);
  
                        /* set up the socket pair we'll use to communicate */
                        socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

                        /* fork off our _labeqd# process */
                        if ((pid = fork()) == 0) 
                         {
                            close(fds[0]);

                            /* reset the signal mask to zero */
                            sigemptyset(&zeromask);
                            sigprocmask(SIG_SETMASK, &zeromask, NULL);

                            dup2(fds[1], STDIN_FILENO);
                            dup2(fds[1], STDOUT_FILENO);
                    
                            sprintf(buffer, "%s/%s", homedir, EQ_SLAVE_EXEC);
                            execl(buffer, buf, (char *) NULL);
                            _exit(0);
                         }    
                        else 
                         { /* parent */
                            if (pid > 0) 
                             {
                                close(fds[1]);
                                master->SlaveFd = fds[0];
                             }
                            else 
                             {
                                EQWriteSystemErrorLog("EqInit: fork failed.", "fork", errno);
                                exit(-1);
                             }
                         }
                     }
                    else /* the card is a slave */ 
                     {
                        if (zero != NULL) 
                         {
                             sprintf(buf, "Found a slave card at %d", instruction[i].card_id);
                             EQWriteSystemLog(buf);
                            card->mcard = zero;
                         }
                        else
                         {
                            sprintf(buf, "Card %d is a slave and needs master card 0", instruction[i].card_id);
                             EQWriteErrorLog(buf);
                             exit(-1);
                         }
                     }
                 }
             }//end of 'for (i = 15; i > 7; i--)'
         }//end of 'if (fgets(buffer, sizeof(buffer), fileptr)) '
     }//end of 'while (!feof(fileptr))'

    close(LabEqcFD);
    fclose(fileptr);

    EQWriteSystemLog("EqInit() has finished.\n");
}

