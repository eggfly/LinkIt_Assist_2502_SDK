/*
  main.cpp - Main loop for Arduino sketches
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 20 Aug 2014 by MediaTek Inc.
  
*/

#define ARDUINO_MAIN
#include "Arduino.h"
#include "vmtype.h"
#include "vmsystem.h"
#include "vmmemory.h"
#include "vmthread.h"
#include "vmlog.h"
#include "vmgsm_tel.h"

typedef VMINT (*vm_get_sym_entry_t)(char* symbol);
extern vm_get_sym_entry_t vm_get_sym_entry;

extern "C" void vm_thread_change_priority(VM_THREAD_HANDLE thread_handle, VMUINT32 new_priority);

unsigned char* spi_w_data = NULL;
unsigned char* spi_r_data= NULL;
unsigned char* spi_data_memory= NULL;

vm_gsm_tel_call_listener_callback g_call_status_callback = NULL;

/*
 * \brief System event callback
 */
void __handle_sysevt(VMINT message, VMINT param) 
{
	/* Special event for arduino application, when arduino thead need call LinkIt 2.0 inteface, it will use this event  */
    if(message == VM_MSG_ARDUINO_CALL)
    {
    	 msg_struct* pMsg = (msg_struct*)param;
    	 if(pMsg->remote_func(pMsg->userdata))
        {
        	vm_signal_post(pMsg->signal);
    	 }
        return ;
    }
}

/*
 * \brief Voice call event callback
 */
void __call_listener_func(vm_gsm_tel_call_listener_data_t* data)
{
	if(g_call_status_callback)
	{
		g_call_status_callback(data);
	}
}

/*
 * \brief Main loop for ardino thread
 */
VMINT32 __arduino_thread(VM_THREAD_HANDLE thread_handle, void* user_data)
{
	init();

	delay(1);
	
	setup();

	for (;;)
	{
		loop();
		if (serialEventRun) serialEventRun();
	}
}

/*
 * \brief Main entry point of Arduino application
 */
extern "C" void vm_main( void )
{
	VM_THREAD_HANDLE handle;

	/* Alloc DMA memory for SPI read and write, for SPI must be use non-cache memory, so we need use
	   vm_malloc_dma to alloc memory, but in arduino thread we cannot invoke this interface directly,
	   so use here to prepare memory, you can see its usage in SPI.transfer in SPI.cpp */
	spi_w_data = (unsigned char*)vm_malloc_dma(2);
	spi_r_data = (unsigned char*)vm_malloc_dma(2);

	/* Same reason like above, this is used for transferring large of data, like screen buffer, you
	   can see its usage in SPI.write in SPI.cpp  */
	spi_data_memory = (unsigned char*)vm_malloc_dma(64*1024);
	memset(spi_data_memory,0, 64*1024);

	/* only prepare something for rand, please don't delete this , otherwise arduino cannot use these
	   two interface */
	srand(0) ;
	rand();

	/* Register system event callback */
	vm_pmng_register_system_event_callback(__handle_sysevt);

	/* Register voice call event callback */
	vm_gsm_tel_call_reg_listener(__call_listener_func);

	/* Create arduino thread, and change its priority, please don't change the code, or it will effort
	   system stable */
	handle = vm_thread_create(__arduino_thread, NULL, 0);
	vm_thread_change_priority(handle, 245);
}

