/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* 

  This example gets the local address and peer address in network byte order.

  It opens the bearer by vm_bearer_open(), after the bearer is opened, 
  it will create a sub thread by vm_thread_create(). 
  Within the sub thread, it will establish a connection to CONNECT_ADDRESS 
  and after the connection is established, 
  it will get the local and peer address by getsockname() and getpeername().

  You can change the connect address by modify MACRO CONNECT_ADDRESS. 
  Change the APN information according to your SIM card. 
  You can see the local address and peer address by searching keywords 
  "local address" and "peer address" in the log about 2 minutes after bootup.
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
#include "BSDMISC.h"
#include "vmsock.h"
#include "vmbearer.h"
#include "vmtimer.h"
#include "vmgsm_gprs.h"

#define CONNECT_ADDRESS "180.76.3.151"
#define CONNECT_PORT 80
#define APN "cmwap"
#define USING_PROXY VM_TRUE
#define PROXY_IP    "10.0.0.172"
#define PROXY_PORT  80

#define START_TIMER 60000

static VM_BEARER_HANDLE g_bearer_hdl;
static VM_THREAD_HANDLE g_thread_handle;
static VMINT g_soc_sockname;

static VMINT32 soc_sub_thread(VM_THREAD_HANDLE g_thread_handle, void* user_data)
{
    SOCKADDR_IN addr_in = {0};
    int ret;
    int len;
    
    g_soc_sockname = socket(PF_INET, SOCK_STREAM, 0);
    addr_in.sin_family = PF_INET;
    addr_in.sin_addr.S_un.s_addr = inet_addr(CONNECT_ADDRESS);
    addr_in.sin_port = htons(CONNECT_PORT);
    ret = connect(g_soc_sockname, (SOCKADDR*)&addr_in, sizeof(SOCKADDR));
    ret = getsockname(g_soc_sockname, (SOCKADDR*)&addr_in, &len);
    vm_log_info("local address:%d.%d.%d.%d", addr_in.sin_addr.S_un.S_un_b.s_b1, addr_in.sin_addr.S_un.S_un_b.s_b2, 
        addr_in.sin_addr.S_un.S_un_b.s_b3, addr_in.sin_addr.S_un.S_un_b.s_b4);
    ret = getpeername(g_soc_sockname, (SOCKADDR*)&addr_in, &len);
    vm_log_info("peer address:%d.%d.%d.%d", addr_in.sin_addr.S_un.S_un_b.s_b1, addr_in.sin_addr.S_un.S_un_b.s_b2, 
        addr_in.sin_addr.S_un.S_un_b.s_b3, addr_in.sin_addr.S_un.S_un_b.s_b4);
    return 0;
}
static void bearer_callback(VM_BEARER_HANDLE handle, VM_BEARER_STATE event, VMUINT data_account_id, void *user_data)
{
    if (VM_BEARER_WOULDBLOCK == g_bearer_hdl)
    {
        g_bearer_hdl = handle;
    }
    if (handle == g_bearer_hdl)
    {
        switch (event)
        {
            case VM_BEARER_DEACTIVATED:
                break;
            case VM_BEARER_ACTIVATING:
                break;
            case VM_BEARER_ACTIVATED:
                g_thread_handle = vm_thread_create(soc_sub_thread, NULL, 0);

                break;
            case VM_BEARER_DEACTIVATING:
                break;
            default:
                break;
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

void start_doing(VM_TIMER_ID_NON_PRECISE tid, void* user_data)
{
    vm_timer_delete_non_precise(tid);
    set_custom_apn();
    g_bearer_hdl = vm_bearer_open(VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN, NULL, bearer_callback, VM_BEARER_IPV4);
}

void handle_sysevt(VMINT message, VMINT param) 
{
    switch (message) 
    {
        case VM_EVENT_CREATE:
            vm_timer_create_non_precise(START_TIMER, start_doing, NULL);
            break;
        case VM_EVENT_QUIT:
            break;
    }
}

void vm_main(void) 
{
    vm_pmng_register_system_event_callback(handle_sysevt);
}

