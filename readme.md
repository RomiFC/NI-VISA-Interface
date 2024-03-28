# NI-VISA

Command-line interface (CLI) that allows users to interact with SCPI-controlled devices. This was created to allow C scripting for functions that are too convoluted to be implemented with singular write/read/query commands. This is not a finished product but provides a framework for functions to be added as needed.

To use, run `FindRsrc.exe` or build the Visual Studio solution. The program will automatically attempt to search for VISA resources to connect to.

For help with the NI-VISA C API, see the [user manual](https://www.ni.com/docs/en-US/bundle/ni-visa/page/user-manual-welcome.html).

## Requirements

- [NI-VISA](https://www.ni.com/en/support/downloads/drivers/download.ni-visa.html)
- [NI-488.2](https://www.ni.com/en/support/downloads/drivers/download.ni-488-2.html#484357)
- Ethernet, [GPIB](https://www.ni.com/en-us/shop/model/gpib-usb-hs.html), serial, or other instrument connection.
