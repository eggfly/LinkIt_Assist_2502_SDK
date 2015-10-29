This example demonstrates how to turn on UART1, configure and register read event, and write/read data to/from UART1.

This example sets VM_PIN_P8/VM_PIN_P9 as pin mode to UART and then opens UART1 device. It sets UART settings such as baud rate, stop bits, and register read callback. When there are some data available to the device through UART1. It reads data with vm_dcl_read and then writes same data through UART1 with vm_dcl_write.

To use this example, connect a UART cable to RX/TX pin according to pin-out diagram. You might need a USB-to-UART conversion cable to connect the UART cable to your computer.
