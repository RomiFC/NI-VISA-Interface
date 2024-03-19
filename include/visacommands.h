
#define READ_BYTES 100  // How many bytes to read on viRead 
#define CHARACTER_MAX 128 // How many characters to store on input from visaSendCommandFromStdin

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
 * @brief Gets a string from the standard input and appends it with a null terminator (as opposed to newline)
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
 * @brief Sends user input string to the instrument at instrLog[rsrcSelect] and reads its response.
 * The resource manager and a session to the device must be opened
 */
void visaSendCommandFromStdin() {
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