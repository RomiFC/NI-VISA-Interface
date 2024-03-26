/*********************************************************************/
/* This example demonstrates how you might query your system for     */
/* a particular instrument.  This example queries for all            */
/* GPIB, serial or VXI instruments.  Notice that VISA is able to     */
/* find GPIB and VXI instruments because the instruments have a      */
/* predefined protocol.  But serial instruments do not.  Hence,      */
/* VISA merely indicates that a serial port is available.            */
/*                                                                   */
/* The general flow of the code is                                   */
/*      Open Resource Manager                                        */
/*      Use viFindRsrc() to query for the first available instrument */
/*      Open a session to this device                                */
/*      Find the next instrument using viFindNext()                  */
/*      Open a session to this device.                               */
/*      Loop on finding the next instrument until all have been found*/
/*      Prompt user to select a device to connect to                 */
/*      Identify device and open options menu for saving data        */
/*********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "visa.h"
#include "integer-input.h"


/*   CONSTANTS   */
#define LOG_MAX 256     // Maximum amount of scanned resources to log
#define TIMEOUT_MS 2500 // Default VISA timeout in milliseconds

/*   STATE CONSTANTS    */
#define RETURN_SUCCESS 0
#define RETURN_ERROR 1
#define RETURN_LOOP 2
#define MAINMENU 10
#define RSRC_SELECT 20
#define MEMORY 30
#define EXIT 0
#define CHANGE 1
#define IDENTIFY 2
#define QUERY 3
#define WRITE 4
#define READ 5
#define SET_TIMEOUT 6
#define SET_READ 7
#define NEXT 9
#define MEM_CATALOG 1
#define MEM_SAVE 2

/*   VI VARIABLES   */
static char instrDescriptor[VI_FIND_BUFLEN];
static ViUInt32 numInstrs;
static ViFindList findList;
static ViSession defaultRM, instr;
static ViStatus status;
static ViUInt32 retCount;
static ViUInt32 writeCount;

/*   STATE VARIABLES    */
int menuState;

/*   GLOBAL VARIABLES   */
int rsrcIndx;           // Stores the VISA parameter 'instr'
int instFound;          // Stores the VISA parameter 'numInstr'
int rsrcSelect;         // Stores the index of instrLog[] which is then passed to VISA functions as 'instr'
char instDescLog[LOG_MAX][VI_FIND_BUFLEN] = { {0} };    // Array which stores the VISA string 'instrDescriptor'
ViSession* instrLog[LOG_MAX];                           // Array which stores VISA parameter 'instr'
static unsigned char buffer[100];
static char stringinput[512];

#include "visacommands.h"

/**
 * @brief Saves instrDescriptor and instr, then iterates rsrcIndx while scanning for resources.
 */
static void logResource() {
    strcpy(instDescLog[rsrcIndx], instrDescriptor);
    instrLog[rsrcIndx] = &instr;
    
    rsrcIndx++;
}


/**
 * @brief Prompts input to select which resource to open a session to.
 * 
 * @return 1 on error, 0 otherwise.
 */
static int connectToRsrc() {
    int errorFlag;

    printf("\nPlease enter a resource index to open:\n");
    fflush(stdin);
    getIntegerFromStdin(&rsrcSelect);
    if (0 <= rsrcSelect && rsrcSelect <= instFound - 1) {
        printf("\n ------------------------------------- \n");
        printf("Opening session to resource %s\n", instDescLog[rsrcSelect]);
    }
    else {
        printf("Invalid input: integer out of range.\n");
        errorFlag = RETURN_ERROR;
        goto Close;
    }  
    /* Now open a session to the resource*/
    status = viOpen(defaultRM, instDescLog[rsrcSelect], VI_NULL, VI_NULL, &instrLog[rsrcSelect]);
    if (status < VI_SUCCESS)
    {
        printf("Error code 0x%X. An error occurred opening a session to %s\n", status, instrDescriptor);
        errorFlag = RETURN_ERROR;
    }
    else
    {   /* Set timeout value and send an *IDN? query */
        status = viSetAttribute(instrLog[rsrcSelect], VI_ATTR_TMO_VALUE, TIMEOUT_MS);

        strcpy(stringinput, "*IDN?");
        status = viWrite(instrLog[rsrcSelect], (ViBuf)stringinput, (ViUInt32)strlen(stringinput), &writeCount);
        if (status < VI_SUCCESS)
        {
            printf("Error code 0x%X. Error writing *IDN? to the device\n", status);
        }

        status = viRead(instrLog[rsrcSelect], buffer, 128, &retCount);
        if (status < VI_SUCCESS)
        {
            printf("Error code 0x%X. Error reading *IDN? response from the device\n", status);
        }
        else
        {
            printf("%*s\n", retCount, buffer);
        }
    }

    return RETURN_SUCCESS;

    Close:
    viClose(instrLog[rsrcSelect]);
    return RETURN_ERROR;
}

/**
 * @brief Finite state machine that handles user options once connected to a resource.
 * 
 * @return 0 on success (exit), 1 on error, 2 when the function should be called again to change states.
 */
static int optionsMenuFSM() {
    switch (menuState) {
    case MAINMENU:
        printf("\n-------- MAIN MENU --------\n");
        printf(" Please select an option:\n");
        printf("%d: Exit program.\n", EXIT);
        printf("%d: Connect to a different resource.\n", CHANGE);
        printf("%d: Identify resource.\n", IDENTIFY);
        printf("%d: Query command.\n", QUERY);
        printf("%d: Write command.\n", WRITE);
        printf("%d: Read command.\n", READ);
        printf("%d: Set timeout.\n", SET_TIMEOUT);
        printf("%d: Set read bytes.\n", SET_READ);
        printf("%d: Memory options.\n", NEXT);

        switch (getInput(9)) {
        case EXIT:
            return RETURN_SUCCESS;
        case CHANGE:
            menuState = RSRC_SELECT;
            return RETURN_LOOP;
        case IDENTIFY:
            visaIdentify();
            enterToContinue();
            return RETURN_LOOP;
        case QUERY:
            visaQuery();
            enterToContinue();
            return RETURN_LOOP;
        case WRITE:
            visaWriteFromStdin();
            enterToContinue();
            return RETURN_LOOP;
        case READ:
            visaRead();
            enterToContinue();
            return RETURN_LOOP;
        case SET_TIMEOUT:
            visaSetTimeout();
            enterToContinue();
            return RETURN_LOOP;
        case SET_READ:
            visaSetReadBytes();
            enterToContinue();
            return RETURN_LOOP;
        case NEXT:
            menuState = MEMORY;
            return RETURN_LOOP;
        default:
            goto errInvInput;
        }
    case MEMORY:
        printf("\n--------- MEMORY COMMANDS --------\n");
        printf(" Please select an option:\n");
        printf("%d: Previous page.\n", EXIT);
        printf("%d: View local memory at C:\\\n", MEM_CATALOG);
        printf("%d: Save trace to computer using markers.\n", MEM_SAVE);
        
        switch (getInput(3)) {
        case EXIT:
            menuState = MAINMENU;
            return RETURN_LOOP;
        case MEM_CATALOG:
            printf("Return format: <mem_used>, <mem_free>, <file_listing>\n");
            printf("Where <file_listing> is <file_name>, <file_size> for each file in the directory.\n");
            visaWrite(":MMEM:CAT? \"C:\"");
            visaRead();
            return RETURN_LOOP;
        case MEM_SAVE:
            visaGetTraceFromMarkers();
            enterToContinue();
            return RETURN_LOOP;
        }
    case RSRC_SELECT:
        viClose(instrLog[rsrcSelect]);
        printf("%d instruments, serial ports, and other resources found:\n\n", instFound);
        for (int i = 0; i < instFound; i++) {
            printf("%3d --- %s\n", i, instDescLog[i]);
        }

        int exitFlag;
        do {
            exitFlag = connectToRsrc();
        } while (exitFlag != RETURN_SUCCESS);

        menuState = MAINMENU;
        return RETURN_LOOP;
    default:
        goto errInvState;
    }

errInvState:
    printf("Error: Menu state invalid.\n");
    return RETURN_ERROR;

errInvInput:
    printf("Error: Input variable invalid.\n");
    return RETURN_ERROR;
}


int main(void) {
   /* Open the default resource manager. */
   status = viOpenDefaultRM (&defaultRM);
   if (status < VI_SUCCESS)
   {
      printf("Error code 0x%X. Could not open a session to the VISA Resource Manager!\n", status);
      exit (EXIT_FAILURE);
   }  

    /*
     * Find all the VISA resources in our system and store the number of resources
     * in the system in numInstrs.  Notice the different query descriptions a
     * that are available.

        Interface         Expression
    --------------------------------------
        GPIB              "GPIB[0-9]*::?*INSTR"
        VXI               "VXI?*INSTR"
        GPIB-VXI          "GPIB-VXI?*INSTR"
        Any VXI           "?*VXI[0-9]*::?*INSTR"
        Serial            "ASRL[0-9]*::?*INSTR"
        PXI               "PXI?*INSTR"
        All instruments   "?*INSTR"
        All resources     "?*"
    */
   status = viFindRsrc (defaultRM, "?*", &findList, &numInstrs, instrDescriptor);
   if (status < VI_SUCCESS)
   {
      printf ("Error code 0x%X. An error occurred while finding resources.\nHit enter to continue.", status);
      fflush(stdin);
      getchar();
      viClose (defaultRM);
      return status;
   }
   instFound = numInstrs;
   printf("%d instruments, serial ports, and other resources found:\n\n",numInstrs);
   printf("%3d --- %s\n", rsrcIndx, instrDescriptor);

   /* Now we will open a session to the instrument we just found. */
   status = viOpen (defaultRM, instrDescriptor, VI_NULL, VI_NULL, &instr);
   if (status < VI_SUCCESS)
   {
      printf ("Error code 0x%X. An error occurred opening a session to %s\n", status, instrDescriptor);
   }
   else
   {
      logResource();
      viClose (instr);
   }
        
   while (--numInstrs)
   {
      /* stay in this loop until we find all instruments */
      status = viFindNext (findList, instrDescriptor);  /* find next desriptor */
      if (status < VI_SUCCESS) 
      {   /* did we find the next resource? */
         printf ("Error code 0x%X. An error occurred finding the next resource.\nHit enter to continue.", status);
         fflush(stdin);
         getchar();
         viClose (defaultRM);
         return status; 
      } 
      printf("%3d --- %s\n", rsrcIndx, instrDescriptor);
    
      /* Now we will open a session to the instrument we just found */
      status = viOpen (defaultRM, instrDescriptor, VI_NULL, VI_NULL, &instr);
      if (status < VI_SUCCESS)
      {
          printf ("Error code 0x%X. An error occurred opening a session to %s\n", status, instrDescriptor);
      }
      else
      {
          logResource();
          viClose (instr);
      }
   }    /* end while */

   /* List all resources and prompt user to select one to open */
   int exitFlag;
   do {
       exitFlag = connectToRsrc();
   } while (exitFlag != RETURN_SUCCESS);

   /* User actions for opened resource */
   menuState = MAINMENU;
   do {
       exitFlag = optionsMenuFSM();
   } while (exitFlag != RETURN_SUCCESS);
   

   /* Close program */
   printf("Closing Program\nHit enter to continue.");
   fflush(stdin);
   getchar();
   status = viClose(instr);
   status = viClose(defaultRM);

   return 0;
}
