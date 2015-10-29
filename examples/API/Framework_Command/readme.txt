This example shows how to use AT command capability.

In this example, we will open the command port first, using vm_cmd_open_port(). Then the application of this example can receive AT commands and display the AT commends in the Monitor tool. The AT command with Test02 uses the vm_cmd_close_port() to close the AT command port. After launching the example, the AT command can be issued through the Monitor tool as below:

AT+[1000]Test01: writes an AT command and log it through the Monitor tool.

AT+[1000]Test02: closes the AT command port.