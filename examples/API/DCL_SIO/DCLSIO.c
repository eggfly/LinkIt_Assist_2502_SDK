/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
This example demonstrates how to turn on UART1, configure and register read event, and write/read data to/from UART1.

This example sets D8/D9 as pin mode to UART and then opens UART1 device. 
It sets UART settings such as baud rate, stop bits, and register read callback. 
When there are some data available to the device through UART1. 
It reads data with vm_dcl_read and then writes same data through UART1 with vm_dcl_write.

To use this example, connect a UART cable to RX/TX pin according to pin-out diagram. 
You might need a USB-to-UART conversion cable to connect the UART cable to your computer.
*/
#include "vmtype.h" 
#include "vmsystem.h"
#include "vmlog.h" 
#include "vmdcl.h"
#include "vmdcl_sio.h"
#include "vmboard.h"

/* handle of UART */
VM_DCL_HANDLE g_uart_handle = 0;
/* Module owner of APP */
VM_DCL_OWNER_ID g_owner_id = 0;
/*UART configuration setting */
vm_dcl_sio_control_dcb_t g_config;

/* handle read data */
void uart_irq_handler(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle){
    if(event == VM_DCL_SIO_UART_READY_TO_READ){
    	/* data has arrived */
        VMCHAR data[32];
        VMINT i;
        VM_DCL_STATUS status;
        VM_DCL_BUFFER_LENGTH returned_len;
        VM_DCL_BUFFER_LENGTH writen_len = 0;
        VMINT count = 0;
        /* read data into buffer */
        status = vm_dcl_read(device_handle,(VM_DCL_BUFFER*)data, 32, &returned_len, vm_dcl_get_owner_id());
        if(status < VM_DCL_STATUS_OK){
            vm_log_debug((char*)"read failed");
        }
        vm_log_info((char*)"read length = %d", returned_len);
        if(g_uart_handle != -1){
            /* write data */
            status = vm_dcl_write(g_uart_handle, (VM_DCL_BUFFER*)data, returned_len, &writen_len, vm_dcl_get_owner_id());

            /* continue to write data if write fails */
            while((status < VM_DCL_STATUS_OK || writen_len != returned_len) && (count < 3))
            {
                count++;
                status = vm_dcl_write(g_uart_handle,(VM_DCL_BUFFER*)data, returned_len, &writen_len, vm_dcl_get_owner_id());
            }
            vm_log_debug((char*)"write length = %d", writen_len);
        }
    }
}

/* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param) {
    switch (message){
        case VM_EVENT_CREATE:
        /* set pin as UART role */
#if defined(__HDK_LINKIT_ONE_V1__)
        vm_dcl_config_pin_mode(VM_PIN_D0, VM_DCL_PIN_MODE_UART);
        vm_dcl_config_pin_mode(VM_PIN_D1, VM_DCL_PIN_MODE_UART);
#elif defined(__HDK_LINKIT_ASSIST_2502__)
        vm_dcl_config_pin_mode(VM_PIN_P8, VM_DCL_PIN_MODE_UART);
        vm_dcl_config_pin_mode(VM_PIN_P9, VM_DCL_PIN_MODE_UART);
#endif
        g_owner_id = vm_dcl_get_owner_id();
        /* open UART1 */
        g_uart_handle = vm_dcl_open(VM_DCL_SIO_UART_PORT1, vm_dcl_get_owner_id());
        if(VM_DCL_HANDLE_INVALID == g_uart_handle){
            vm_log_info((char*)"open failed");
        }
        else{
            g_config.owner_id = g_owner_id;
            g_config.config.dsr_check = 0;
            g_config.config.data_bits_per_char_length = VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH_8;
            g_config.config.flow_control = VM_DCL_SIO_UART_FLOW_CONTROL_NONE;
            g_config.config.parity = VM_DCL_SIO_UART_PARITY_NONE;
            g_config.config.stop_bits = VM_DCL_SIO_UART_STOP_BITS_1;
            g_config.config.baud_rate = VM_DCL_SIO_UART_BAUDRATE_115200;
            g_config.config.sw_xoff_char = 0x13;
            g_config.config.sw_xon_char = 0x11;
            /*set UART1 configure */
            vm_dcl_control(g_uart_handle, VM_DCL_SIO_COMMAND_SET_DCB_CONFIG,(void *)&g_config);
            /* register read event arrived callback */
            vm_dcl_register_callback(g_uart_handle, VM_DCL_SIO_UART_READY_TO_READ, (vm_dcl_callback)uart_irq_handler, (void*)NULL);
        }
       
        break;

        case VM_EVENT_QUIT:
        vm_dcl_close(g_uart_handle);
        break;
    }
}

/* Entry point */
void vm_main(void){
    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
