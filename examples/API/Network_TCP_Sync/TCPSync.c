/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* 
  This example creates a TCP Client in synchronous way and sends a HTTP request
  to an address and reads the response.

  It creates a sub-thread by using vm_thread_create(). In the sub-thread, it
  connects a server by calling vm_tcp_connect_sync(). After the connection is
  established, it calls vm_tcp_write_sync() to send a request and calls 
  vm_tcp_read_sync() to read the response.

  The connection address can be changed by modifying the macro
  ADDR. The cellular APN should be set according to the SIM card information.
  
  This example requires a valid SIM card installed on the board.
*/

#include <string.h>
#include "vmtype.h" 
#include "vmboard.h"
#include "vmsystem.h"
#include "vmlog.h" 
#include "vmcmd.h" 
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmthread.h"

#include "ResID.h"
#include "TCPSync.h"
#include "vmsock.h"
#include "vmbearer.h"
#include "vmtcp.h"
#include "vmtimer.h"
#include "vmgsm_gprs.h"

/* Configurable macros */
#define CUST_APN    "cmwap"         /* The APN of your test SIM */
#define USING_PROXY VM_TRUE         /* Whether your SIM uses proxy */
#define PROXY_ADDRESS   "10.0.0.172"    /* The proxy address */
#define PROXY_PORT  80              /* The proxy port */
#define TCP_SYNC_START_TIMER    60000   /* Start connection 60 seconds later after bootup */

#define MAX_SIZE 1024*2
VM_TCP_HANDLE soc_handle;
#define ADDR "180.76.3.151"
char *send = "GET http://www.baidu.com/ HTTP/1.1\r\nHOST:www.baidu.com\r\n\r\n";
static VM_THREAD_HANDLE g_thread_handle = 0;
VMBYTE buf[MAX_SIZE] = {0};

/* The sub-thread to make synchronous connection to the server */
static VMINT32 soc_sub_thread(VM_THREAD_HANDLE thread_handle, void* user_data)
{
    VMINT read_data;
    VMINT ret;
    while(1)
    {                         
        soc_handle = vm_tcp_connect_sync(ADDR, 80, VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN);

        ret = vm_tcp_write_sync(soc_handle, send, strlen(send));

        ret = 0;
        do {
            read_data = vm_tcp_read_sync(soc_handle, buf + ret, MAX_SIZE - ret);
            vm_log_debug("%d bytes data received", read_data);
            if (read_data < MAX_SIZE && read_data > 0)
            {
                ret += read_data;
            }
            else
            {
                break;
            }
        }while (1);
        ret = vm_tcp_close_sync(soc_handle);
        break;
    }
    return 0;
}

void set_custom_apn(void)
{
    vm_gsm_gprs_apn_info_t apn_info;
    
    memset(&apn_info, 0, sizeof(apn_info));
    strcpy(apn_info.apn, CUST_APN);
    strcpy(apn_info.proxy_address, PROXY_ADDRESS);
    apn_info.proxy_port = PROXY_PORT;
    apn_info.using_proxy = USING_PROXY;
    vm_gsm_gprs_set_customized_apn_info(&apn_info);
}

void start_timer_cb(VM_TIMER_ID_NON_PRECISE timer_id, void* user_data)
{
    vm_timer_delete_non_precise(timer_id);
    set_custom_apn();
    /* Creates a sub-thread */
    g_thread_handle = vm_thread_create(soc_sub_thread, NULL, 0);
}

/* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param) 
{
    switch (message) 
    {
    case VM_EVENT_CREATE:
        vm_timer_create_non_precise(TCP_SYNC_START_TIMER, start_timer_cb, NULL);

        break;

    case VM_EVENT_QUIT:
        break;
    }
}

/* Entry point */
void vm_main(void) 
{
    /* Registers system event handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}

