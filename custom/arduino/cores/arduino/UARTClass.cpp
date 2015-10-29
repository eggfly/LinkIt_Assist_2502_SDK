/*
  Copyright (c) 2014 MediaTek Inc.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License..

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
   See the GNU Lesser General Public License for more details.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "UARTClass.h"
#include "variant.h"
#include "vmlog.h"

static VM_DCL_HANDLE usb_device_handle = -1;
       
typedef struct  
{       
    VMINT 	result;
}vm_uart_ctrl_com_response;

// Constructors ////////////////////////////////////////////////////////////////

UARTClass::UARTClass( int usbNum, RingBuffer* pRx_buffer )
{
    _rx_buffer = pRx_buffer ;
    _usbNum = usbNum;
    uart_handle = -1;
}

void UartIrqHandler(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
    if(event == VM_DCL_SIO_UART_READY_TO_READ)
    {
        char data[SERIAL_BUFFER_SIZE];
        int i;
        VM_DCL_STATUS status;
        VM_DCL_BUFFER_LENGTH returned_len;

        status = vm_dcl_read(device_handle,(VM_DCL_BUFFER*)data,SERIAL_BUFFER_SIZE,&returned_len,vm_dcl_get_owner_id());
        if(status<VM_DCL_STATUS_OK)
        {
            vm_log_info((char*)"read failed");
        }  
        if(device_handle == g_APinDescription[0].ulHandle)
        {
            for(i=0;i<returned_len;i++)
            {
                Serial1._rx_buffer->store_char(data[i]);
            }
        }
        else
        {
            for(i=0;i<returned_len;i++)
            {
                Serial._rx_buffer->store_char(data[i]);
            }
        }
    }
    else
    {
    }
}

// Public Methods //////////////////////////////////////////////////////////////

void UARTClass::begin( const uint32_t dwBaudRate )
{

    vm_dcl_sio_control_dcb_t data;
	
    if(_usbNum == 2)
    {
        if(!changePinType(0, PIO_UART, &uart_handle))
            return;
	
        if(uart_handle == VM_DCL_HANDLE_INVALID)
            uart_handle = vm_dcl_open(VM_DCL_SIO_UART_PORT1,vm_dcl_get_owner_id());
    }
    else
    {
        uart_handle = vm_dcl_open(VM_DCL_SIO_USB_PORT1,vm_dcl_get_owner_id());
    }

    if(VM_DCL_HANDLE_INVALID==uart_handle) 
    {
        vm_log_info((char*)"UARTClass failed");
        return;
    }

    data.owner_id = vm_dcl_get_owner_id();	
    data.config.baud_rate = (VM_DCL_SIO_UART_BAUDRATE)dwBaudRate;
    data.config.data_bits_per_char_length = 8;
    data.config.stop_bits = 1;
    data.config.parity = 0;
    data.config.flow_control = 1;
    data.config.sw_xon_char = 0x11;
    data.config.sw_xoff_char = 0x13;
    data.config.dsr_check = 0;
    vm_dcl_control(uart_handle,VM_DCL_SIO_COMMAND_SET_DCB_CONFIG,(void *)&data);
    vm_dcl_register_callback(uart_handle,VM_DCL_SIO_UART_READY_TO_READ,(vm_dcl_callback)UartIrqHandler,(void*)NULL);

    if(_usbNum == 2)
    {
        setPinHandle(0, uart_handle);
    }
    else
    {
        usb_device_handle = uart_handle;
    }
}

void UARTClass::end( void )
{
// clear any received data
    _rx_buffer->_iHead = _rx_buffer->_iTail ;
    vm_dcl_close(uart_handle);
  
    if(_usbNum == 2)
    {
        g_APinDescription[0].ulHandle = VM_DCL_HANDLE_INVALID;
        g_APinDescription[0].ulPinType = PIO_DIGITAL;
    }
    else
    {
        usb_device_handle = -1;
    }
}

int UARTClass::available( void )
{
    return (uint32_t)(SERIAL_BUFFER_SIZE + _rx_buffer->_iHead - _rx_buffer->_iTail) % SERIAL_BUFFER_SIZE ;
}

int UARTClass::peek( void )
{
    if( _rx_buffer->_iHead == _rx_buffer->_iTail )
        return -1 ;

    return _rx_buffer->_aucBuffer[_rx_buffer->_iTail] ;
}

int UARTClass::read( void )
{
// if the head isn't ahead of the tail, we don't have any characters
    if ( _rx_buffer->_iHead == _rx_buffer->_iTail )
        return -1 ;

    uint8_t uc = _rx_buffer->_aucBuffer[_rx_buffer->_iTail] ;
    _rx_buffer->_iTail = (unsigned int)(_rx_buffer->_iTail + 1) % SERIAL_BUFFER_SIZE ;
    return uc ;
}

void UARTClass::flush( void )
{
    while(read()!=-1);
}

size_t UARTClass::write( const uint8_t uc_data )
{

    VM_DCL_STATUS status;
    VM_DCL_BUFFER_LENGTH writen_len = 0;
    int count = 0;
  
    if(uart_handle != -1)
    {
        status = vm_dcl_write(uart_handle,(VM_DCL_BUFFER*)&uc_data,1,&writen_len,vm_dcl_get_owner_id());

        while((status<VM_DCL_STATUS_OK || writen_len != 1) && (count < 3))
        {
            delayMicroseconds(10);
            count++;
            status = vm_dcl_write(uart_handle,(VM_DCL_BUFFER*)&uc_data,1,&writen_len,vm_dcl_get_owner_id());
        }  
	  
        return writen_len;
    }
  
    return 0;
  
}

UARTClass::operator bool()
{
    if(_usbNum == 2)
    {
        return true;
    }
	
    if(VM_DCL_HANDLE_INVALID != uart_handle) 
    {
        vm_uart_ctrl_com_response dtr;
        vm_dcl_control(uart_handle, 528, (vm_uart_ctrl_com_response*) &dtr);
        if (dtr.result == TRUE)
        {
            return true;
        }
    }
	
    delay(5);
    return false;

}


