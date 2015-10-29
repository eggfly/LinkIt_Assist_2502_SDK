/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* 
  This example creates TCP Client and sends a HTTP request to an server and
  prints the first 20 bytes of the data received to the log on the Monitor.

  It connects the server by vm_tcp_connect(). After the connection is established,
  it will call vm_tcp_write() to send the request. When the response arrives,
  it calls vm_tcp_read() to read the data from the response.

  The connection address can be changed by modifying the macro CONNECT. The APN
  should be set according to the SIM card information.
  
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
#include "TCPAsync.h"
#include "vmsock.h"
#include "vmbearer.h"
#include "vmtcp.h"
#include "vmtimer.h"
#include "vmgsm_gprs.h"

#define CONTENT "GET http://www.baidu.com/ HTTP/1.1\r\nHOST:www.baidu.com\r\n\r\n"
#define ADDR "180.76.3.151"
#define PORT    80
#define APN "cmwap"
#define USING_PROXY VM_TRUE
#define PROXY_IP    "10.0.0.172"
#define PROXY_PORT  80
#define START_TIMER 60000
#define BUF_MAX_SIZE 1024
static VM_TCP_HANDLE g_tcp_handle;

static void tcp_cb(VM_TCP_HANDLE handle, VM_TCP_EVENT event, void* user_data)
{
	VMINT ret = 0;
    if (g_tcp_handle == handle)
    {
	    if (VM_TCP_EVENT_CONNECTED == event || VM_TCP_EVENT_CAN_WRITE == event)
	    {
	        ret = vm_tcp_write(g_tcp_handle, CONTENT, strlen(CONTENT));
	    }
	    if (VM_TCP_EVENT_CAN_READ == event)
	    {
	        VMCHAR buf[BUF_MAX_SIZE] = {0};
	        ret = vm_tcp_read(g_tcp_handle, buf, BUF_MAX_SIZE);
            buf[20] = 0;
            vm_log_debug("First 20 bytes received:%s", buf);
            vm_tcp_close(g_tcp_handle);
	    }
    }

}

void set_custom_apn(void)
{
    VMINT ret;
    vm_gsm_gprs_apn_info_t apn_info;

    memset(&apn_info, 0, sizeof(apn_info));
    apn_info.using_proxy = USING_PROXY;
    strcpy(apn_info.apn, APN);
    strcpy(apn_info.proxy_address, PROXY_IP);
    apn_info.proxy_port = PROXY_PORT;
    ret = vm_gsm_gprs_set_customized_apn_info(&apn_info);
}

static void do_connect(void)
{
    set_custom_apn();
	g_tcp_handle = vm_tcp_connect(ADDR, PORT, VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN, NULL, tcp_cb);
}

static void start_doing(VM_TIMER_ID_NON_PRECISE tid)
{
    /* Deletes the non-precise timer */
    vm_timer_delete_non_precise(tid);
    
    /* Starts the connection */
    do_connect();
}

/* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param) 
{
    switch (message) 
    {
    case VM_EVENT_CREATE:
        /* Creates a non-precise timer for the board to initialize itself. */
        vm_timer_create_non_precise(START_TIMER, start_doing, NULL);
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

