
#define READ_BYTES 100  // How many bytes to read on viRead 

void visaIdentify() {
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
        printf("\n%*s\n", retCount, buffer);
    }
}