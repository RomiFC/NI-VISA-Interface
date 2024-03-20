

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
 */
void visaRead() {
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
    }
    else
    {
        printf("%d bytes returned:\n", retCount);
        printf("%*s\n", retCount, buffer);
    }
}

void visaSetReadBytes() {
    printf("Enter a value of bytes to read. Default: %d bytes. Max: %d bytes.\n", READ_BYTES, MAX_READ_BYTES);
    int ret = getInput(MAX_READ_BYTES);
    printf("Confirmed: Read count set to %d bytes.\n", ret);
}