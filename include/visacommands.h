

#define READ_BYTES 4096         // Default byte count to read when issuing viRead
#define MAX_READ_BYTES 1048576  // Max read bytes allowed
#define CHARACTER_MAX 256       // How many characters to store on input from visaWriteFromStdin and visaWrite
#define TIMEOUT_MIN 1000        // Minimum VISA timeout value
#define TIMEOUT_MAX 25000       // Maximum VISA timeout value


int readBytes;

/**
 * @brief Sends the *IDN? command to the instrument at instrLog[rsrcSelect] and reads its response.
 * The resource manager and a session to the device must be opened.
 */
void visaIdentify() {
    printf("Sending *IDN? to the device...\n");
    strcpy(stringinput, "*IDN?");
    status = viWrite(instrLog[rsrcSelect], (ViBuf)stringinput, (ViUInt32)strlen(stringinput), &writeCount);
    if (status < VI_SUCCESS)
    {
        printf("Error writing *IDN? to the device\n");
    }

    status = viRead(instrLog[rsrcSelect], buffer, READ_BYTES, &retCount);
    if (status < VI_SUCCESS)
    {
        printf("Error reading *IDN? response from the device\n");
    }
    else
    {
        printf("%*s\n", retCount, buffer);
    }
}

/**
 * @brief Prompts user input for integer within range (0 to rangeMax) and tests for validity. Recursive on error.
 *
 * @param rangeMax Maximum integer value that user can input.
 * @return User input integer provided it is in range.
 */
int getInput(int rangeMax) {
    int input;
    printf("\n");
    fflush(stdin);
    getIntegerFromStdin(&input);
    if (0 <= input && input <= rangeMax) {
        return input;
    }
    else {
        printf("Invalid input: integer out of range.\n");
        getInput(rangeMax);
    }
}

/**
 * @brief Gets a string from the standard input and appends it with a null terminator (as opposed to newline).
 * @param str String that is received from standard input
 * @param n Amount of characters allotted to the string array (Can read n-1 characters)
 */
void s_gets(char* str, int n) {
    char* str_read = fgets(str, n, stdin);
    if (!str_read)
        return;

    int i = 0;
    while (str[i] != '\n' && str[i] != '\0')
        i++;

    if (str[i] == '\n')
        str[i] = '\0';
}

/**
 * @brief Prompts user to hit enter to continue.
 */
void enterToContinue() {
    printf("Hit enter to continue.\n");
    fflush(stdin);
    getchar();
}

/**
 * @brief Sets timeout value to user input integer.
 * The resource manager and a session to the device must be opened.
 */
void visaSetTimeout() {
    printf("Enter desired VISA timeout in milliseconds between %d and %d. Default: %d\n", TIMEOUT_MIN, TIMEOUT_MAX, TIMEOUT_MS);
    int timeout;
    int errorFlag = 1;
    do {
        timeout = getInput(TIMEOUT_MAX);
        if (TIMEOUT_MIN <= timeout && timeout <= TIMEOUT_MAX) {
            errorFlag = 0;
        }
        else {
            printf("Invalid input: integer out of range.\n");
        }
    } while (errorFlag);

    status = viSetAttribute(instrLog[rsrcSelect], VI_ATTR_TMO_VALUE, timeout);
    float timeoutFloat = timeout;
    printf("New timeout value: %.3f seconds\n", timeoutFloat / 1000);
}

/**
 * @brief Sends user input string to the instrument at instrLog[rsrcSelect] and reads its response.
 * The resource manager and a session to the device must be opened.
 */
void visaQuery() {
    printf("Enter SCPI command to send.\n");
    char stringFromStdin[CHARACTER_MAX];
    s_gets(stringFromStdin, CHARACTER_MAX);
    printf("Sending %s to the device...\n", stringFromStdin);

    strcpy(stringinput, stringFromStdin);
    status = viWrite(instrLog[rsrcSelect], (ViBuf)stringinput, (ViUInt32)strlen(stringinput), &writeCount);
    if (status < VI_SUCCESS)
    {
        printf("Error %X: Cannot write %s to the device.\n", status, stringFromStdin);
    }

    status = viRead(instrLog[rsrcSelect], buffer, READ_BYTES, &retCount);
    if (status < VI_SUCCESS)
    {
        printf("Error %X: Cannot read response from the device.\n", status);
    }
    else
    {
        printf("Response:\n");
        printf("%*s\n", retCount, buffer);
    }
}

/**
 * @brief Sends user input string to the instrument at instrLog[rsrcSelect].
 * The resource manager and a session to the device must be opened.
 */
void visaWriteFromStdin() {
    printf("Enter SCPI command to send.\n");
    char stringFromStdin[CHARACTER_MAX];
    s_gets(stringFromStdin, CHARACTER_MAX);
    printf("Sending %s to the device...\n", stringFromStdin);

    strcpy(stringinput, stringFromStdin);
    status = viWrite(instrLog[rsrcSelect], (ViBuf)stringinput, (ViUInt32)strlen(stringinput), &writeCount);
    if (status < VI_SUCCESS)
    {
        printf("Error %X: Cannot write %s to the device.\n", status, stringFromStdin);
    }
}

/**
 * @brief Sends function input string to the instrument at instrLog[rsrcSelect].
 * The resource manager and a session to the device must be opened.
 * @param string Function input which will be send to the instrument
 */
void visaWrite(char string[CHARACTER_MAX]) {
    printf("Sending %s to the device...\n", string);

    strcpy(stringinput, string);
    status = viWrite(instrLog[rsrcSelect], (ViBuf)stringinput, (ViUInt32)strlen(stringinput), &writeCount);
    if (status < VI_SUCCESS)
    {
        printf("Error %X: Cannot write %s to the device.\n", status, string);
    }
}

/**
 * @brief Reads response from the instrument at instrLog[rsrcSelect]
 * The resource manager and a session to the device must be opened.
 * 
 * @return Pointer to the string read from the device.
 */
char* visaRead() {
    if (readBytes == 0) {
        readBytes = READ_BYTES;
    }

    status = viRead(instrLog[rsrcSelect], buffer, readBytes, &retCount);
    if (status == VI_SUCCESS_TERM_CHAR || status == VI_SUCCESS_MAX_CNT) {
        printf("Warning %X: No termination character or END indicator received. Increase read bytes to fix.\n\n", status);
    }
    if (status < VI_SUCCESS)
    {
        printf("Error %X: Cannot read response from the device.\n", status);
        return NULL;
    }
    else
    {
        printf("%d bytes returned:\n", retCount);
        printf("%*s\n", retCount, buffer);
        char* returnString = malloc(readBytes);
        strcpy(returnString, buffer);
        return returnString;
    }
}

/**
 * @brief Reads response from the instrument at instrLog[rsrcSelect] without printing to Stdout
 * The resource manager and a session to the device must be opened.
 * 
 * @return Pointer to the string read from the device.
 */
char* visaReadNoPrint() {
    if (readBytes == 0) {
        readBytes = READ_BYTES;
    }

    status = viRead(instrLog[rsrcSelect], buffer, readBytes, &retCount);
    if (status == VI_SUCCESS_TERM_CHAR || status == VI_SUCCESS_MAX_CNT) {
        printf("Warning %X: No termination character or END indicator received. Increase read bytes to fix.\n\n", status);
    }
    if (status < VI_SUCCESS)
    {
        printf("Error %X: Cannot read response from the device.\n", status);
        return NULL;
    }
    else
    {
        char* returnString = malloc(readBytes);
        strcpy(returnString, buffer);
        return returnString;
    }
}

/**
 * @brief Sets amount of return bytes to read on visaRead
 */
void visaSetReadBytes() {
    printf("Enter a value of bytes to read. Default: %d bytes. Max: %d bytes.\n", READ_BYTES, MAX_READ_BYTES);
    int ret = getInput(MAX_READ_BYTES);
    printf("Confirmed: Read count set to %d bytes.\n", ret);
}

/**
 * @brief Uses markers to generate a csv of the current trace. Should only be used for devices that don't support file transfer via SCPI.
 * Traces are saved to trace000.csv in the location the program is run.
 */
void visaGetTraceFromMarkers() {
    if (readBytes == 0) {
        readBytes = READ_BYTES;
    }

    int numPoints;                  // Number of points to sweep trace over
    double startFreq, stopFreq;     //
    double freqSpacing;             // Spacing in frequency between each swept point

    /* User menu to select sweep points */
    printf("-------- SELECT NUMBER OF POINTS --------\n");
    printf("%d: Back\n", EXIT);
    printf("1: 101\n");
    printf("2: 201\n");
    printf("3: 401\n");
    printf("4: 801\n");
    printf("5: 1601\n");
    printf("6: Set custom\n");

    switch (getInput(6)) {
    case EXIT:
        return;
    case 1:
        numPoints = 101;
        break;
    case 2:
        numPoints = 201;
        break;
    case 3:
        numPoints = 401;
        break;
    case 4:
        numPoints = 801;
        break;
    case 5:
        numPoints = 1601;
        break;
    case 6:
        printf("Enter number of points to sweep. Min: 21, Max: 24001\n");
        do {
            numPoints = getInput(24001);
            if (numPoints < 21)
                printf("Invalid input: integer out of range.\n");
        } while (numPoints < 21);
        break;
    }

    /* Get the start and stop frequency from the spec-an by writing commands and converting the response to a float value. */
    char* startFreqBuffer = malloc(sizeof(char) * readBytes);   // Stores start frequency string read from instrument
    char* stopFreqBuffer = malloc(sizeof(char) * readBytes);    // Stores stop frequency string read from instrument
    
    visaWrite(":SENSe:FREQuency:STARt?");
    strcpy(startFreqBuffer, visaRead());
    visaWrite(":SENSe:FREQuency:STOP?");
    strcpy(stopFreqBuffer, visaRead());
    startFreq = atof(startFreqBuffer);
    stopFreq = atof(stopFreqBuffer);
    if (startFreq < 0 || stopFreq <= 0) {
        printf("Error: Start or stop frequency could not be read from the device.\n");
        return;
    }
    else {
        freqSpacing = (stopFreq - startFreq) / (numPoints - 1);
    }

    /* Setup the marker functions and move it to each point across the trace, recording the y value each time. */
    double* freq = malloc(sizeof(double) * numPoints);      // Array which stores frequency values of trace
    double* amp = malloc(sizeof(double) * numPoints);       // Array which stores amplitude values of trace
    char buffer[256];                                       //
    double resBW, vidBW;                                    // Resolution and video bandwidth read from instrument
    
    visaWrite(":INITiate:CONTinuous OFF");
    visaWrite(":CALCulate:MARKer:AOff");
    visaWrite(":CALCulate:MARKer1:FUNCtion BPower");
    visaWrite(":CALCulate:MARKer1:FCOunt:STATe ON");
    visaWrite(":CALCulate:MARKer1:MODE POSition");
    visaWrite(":SENSe:BANDwidth:RESolution?");
    resBW = atof(visaRead());
    visaWrite(":SENSe:BANDwidth:VIDeo?");
    vidBW = atof(visaRead());

    for (int i = 0; i < numPoints; i++) {
        freq[i] = round(startFreq + i * freqSpacing);
        sprintf(buffer, ":CALC:MARK1:X %f", freq[i]);
        visaWrite(buffer);
        visaWrite(":CALC:MARK1:Y?");
        amp[i] = atof(visaReadNoPrint());
    }
    visaWrite(":INITiate:CONTinuous ON");

    printf("\nStart frequency: %e, Stop frequency: %e\n", startFreq, stopFreq);
    printf("Number of points: %d, Frequency spacing: %g\n", numPoints, freqSpacing);
    printf("Resolution bandwidth: %e, Video bandwidth: %e\n", resBW, vidBW);
    
    /* Check if a file exists with the name trace000.csv */
    /* If yes, increment number until an unused name is found */
    FILE* filePtr;
    int i = 0;
    char fileName[64];
    do {
        sprintf(fileName, "trace%.3d.csv", i);
        i++;
        filePtr = fopen(fileName, "r");
        if (filePtr == NULL)
            continue;
        else
            fclose(filePtr);
    } while (filePtr != NULL);
    /* Write header and trace information to the file */
    filePtr = fopen(fileName, "w");
    fprintf(filePtr, "# %s\n# Start: %e\n# Stop: %e\n# Points: %d\n# RBW: %e\n# VBW: %e\n# Frequency, Amplitude\n", fileName, startFreq, stopFreq, numPoints, resBW, vidBW);
    for (int n = 0; n < numPoints; n++) {
        fprintf(filePtr, "%f,%f\n", freq[n], amp[n]);
    }
    if (fclose(filePtr) == 0)
        printf("Trace data saved to %s\n", fileName);
    else
        printf("Error: fclose() could not close the file stream.\n");

    return;
}


/**
 * @brief Detects if the trace is set to continuous or not, then toggles it.
 */
void visaToggleFreeze() {
    visaWrite(":INITiate:CONTinuous?");
    int ifCont = atof(visaReadNoPrint());
    switch (ifCont) {
    case 0:
        visaWrite(":INIT:CONT ON");
        break;
    case 1:
        visaWrite(":INIT:CONT OFF");
        break;
    default:
        printf("Error\n");
    }
}