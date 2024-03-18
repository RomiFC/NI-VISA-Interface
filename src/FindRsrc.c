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
#include "visa.h"
#include "integer-input.h"

/*   CONSTANTS   */
#define LOG_MAX 256     // Maximum amount of scanned resources to log
#define TIMEOUT_MS 2000 //  VISA timeout in milliseconds

/*   VI VARIABLES   */
static char instrDescriptor[VI_FIND_BUFLEN];
static ViUInt32 numInstrs;
static ViFindList findList;
static ViSession defaultRM, instr;
static ViStatus status;
static ViUInt32 retCount;
static ViUInt32 writeCount;

/*   GLOBAL VARIABLES   */
int rsrcIndx, instFound;
int input;
char instDescLog[LOG_MAX][VI_FIND_BUFLEN] = { {0} };
ViSession* instrLog[LOG_MAX];
static unsigned char buffer[100];
static char stringinput[512];

/*  Saves instrDescriptor, & instr, and iterates rsrcIndx while scanning for resources. */
static void logResource(void) {
    strcpy(instDescLog[rsrcIndx], instrDescriptor);
    instrLog[rsrcIndx] = &instr;

    rsrcIndx++;
}

/*  Prompts input to select which resource to open a session to.    */
static void connectToRsrc(void) {
    printf("\nPlease enter a resource index to open:\n");
    fflush(stdin);
    getIntegerFromStdin(&input);
    if (0 <= input && input <= instFound - 1) {
        printf("Opening session to resource %s\n", instDescLog[input]);
    }
    else {
        printf("Invalid input: integer out of range.\n");
        connectToRsrc();
    }  
    /* Now open a session to the resource*/
    status = viOpen(defaultRM, instDescLog[input], VI_NULL, VI_NULL, &instrLog[input]);
    if (status < VI_SUCCESS)
    {
        printf("An error occurred opening a session to %s\n", instrDescriptor);
    }
    else
    {   /* Set timeout value and send an *IDN? query */
        status = viSetAttribute(instrLog[input], VI_ATTR_TMO_VALUE, TIMEOUT_MS);

        strcpy(stringinput, "*IDN?");
        status = viWrite(instrLog[input], (ViBuf)stringinput, (ViUInt32)strlen(stringinput), &writeCount);
        if (status < VI_SUCCESS)
        {
            printf("Error writing *IDN? to the device\n");
        }

        status = viRead(instrLog[input], buffer, 100, &retCount);
        if (status < VI_SUCCESS)
        {
            printf("Error reading *IDN? response from the device\n");
        }
        else
        {
            printf("%*s\n", retCount, buffer);
        }
    }

    viClose(instrLog[input]);
    connectToRsrc();
}

int main(void) {
   /* First we will need to open the default resource manager. */
   status = viOpenDefaultRM (&defaultRM);
   if (status < VI_SUCCESS)
   {
      printf("Could not open a session to the VISA Resource Manager!\n");
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
      printf ("An error occurred opening a session to %s\n",instrDescriptor);
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
         printf ("An error occurred finding the next resource.\nHit enter to continue.");
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
          printf ("An error occurred opening a session to %s\n",instrDescriptor);
      }
      else
      {
          logResource();
          viClose (instr);
      }
   }    /* end while */

   /* List all resources and prompt user to select one to open */
   connectToRsrc();


   /* Close program */
   printf("Closing Program\nHit enter to continue.");
   fflush(stdin);
   getchar();
   status = viClose(instr);
   status = viClose(defaultRM);

   return 0;
}
