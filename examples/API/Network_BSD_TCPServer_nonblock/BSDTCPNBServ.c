/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* 
  This example creates a non-blocking TCP server that listens to port 40000. 
  When data is received it will send the RESPONSE_STRING to client. 

  It opens the bearer by vm_bearer_open() and once the bearer is opened,
  it will create a sub thread by vm_thread_create(). Within the sub thread, it will
  bind the bearer to port 40000 by bind() and will start to listen for requests.
  Once requests are arriving, it will receive the data by recv() and send the response by send();

  You need change the WiFi SSID and PASSWORD to by modify the MACRO AP_SSID and AP_PASSWORD.
  You can see the local ip in log, if send a request to the port 40000 of this ip, it will
  print the first 20 bytes of the request data out to log.
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
#include "BSDTCPNBServ.h"
#include "vmsock.h"
#include "vmbearer.h"
#include "vmtimer.h"
#include "vmwlan.h"

#define MAX_BUF_LEN 1024
#define AP_SSID "SSID"
#define AP_PASSWORD "12345678"
#define AP_AUTH_MODE  VM_WLAN_AUTHORIZE_MODE_WPA2_ONLY_PSK
#define RESPONSE_STRING "Hello Client"

static VM_BEARER_HANDLE g_bearer_hdl;
static VM_THREAD_HANDLE g_thread_handle;
static VMINT g_soc_client;
static VMINT g_soc_server;

static VMINT32 soc_sub_thread(VM_THREAD_HANDLE thread_handle, void* user_data)
{
    SOCKADDR_IN addr_in = {0};
    SOCKADDR_IN client_addr_in = {0};
    char buf[MAX_BUF_LEN] = {0};
    int len = 0;
    int ret;
    timeval timeout;
    fd_set writefds;
    fd_set readfds;
    int option;

    g_soc_server = socket(PF_INET, SOCK_STREAM, 0);
    option = 1;
    ret = setsockopt(g_soc_server, VM_SOC_NBIO, &option, sizeof(option));
    addr_in.sin_family = PF_INET;
    addr_in.sin_addr.S_un.s_addr = 0;   /* INADDR_ANY */
    addr_in.sin_port = htons(40000);
    ret = bind(g_soc_server, (SOCKADDR*)&addr_in, sizeof(SOCKADDR), SOCK_STREAM);
    ret = listen(g_soc_server, 5);
    g_soc_client = accept(g_soc_server, (SOCKADDR*)&client_addr_in, &len);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000000;
    FD_ZERO(&writefds);
    FD_ZERO(&readfds);
    FD_SET(g_soc_client, &writefds);
    FD_SET(g_soc_client, &readfds);
    do
    {
        if (select(g_soc_client+1, &readfds, &writefds, 0, &timeout) >= 0)
        {
            if (FD_ISSET(g_soc_client, &readfds))
            {
                     //socket is ready for reading data
                int i = 0;
                ret = recv(g_soc_client, buf, MAX_BUF_LEN, 0);
                buf[20] = 0;
                vm_log_debug("First 20 bytes of received data:%s", buf);
            }        
            if (FD_ISSET(g_soc_client, &writefds))
            {
                     //socket is ready for writting data
                int j = 0;
                memset(buf, 0, MAX_BUF_LEN);
                strcpy(buf, RESPONSE_STRING);
                ret = send(g_soc_client, buf, strlen(RESPONSE_STRING), 0);
                break;
            }        
        }
    }while (1);
    
    closesocket(g_soc_client);
    closesocket(g_soc_server);

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
static void wlan_connect_callback(
            void *user_data,
            vm_wlan_connect_result_t *connect_result)
{
    vm_log_debug("wlan result:%d", connect_result->result);
}

static void wlan_noti_cb(void* user_data,
        vm_wlan_notification_t* notification)
{
    vm_wlan_ip_info_t ip_info;
    
    if(VM_WLAN_NOTIFICATION_IP_AVAILABLE == notification->type)
    {
        vm_wlan_deregister_notification_handler(VM_WLAN_NOTIFICATION_IP_AVAILABLE, wlan_noti_cb, NULL);
        memset(&ip_info, 0, sizeof(ip_info));
        vm_wlan_get_ip_info(&ip_info);
        vm_log_debug("local ip:%d.%d.%d.%d", ip_info.ip_address[0], ip_info.ip_address[1], ip_info.ip_address[2], ip_info.ip_address[3]);
        g_bearer_hdl = vm_bearer_open(VM_BEARER_DATA_ACCOUNT_TYPE_WLAN, NULL, bearer_callback, VM_BEARER_IPV4);
    }
}

static void do_connect(void)
{
    vm_wlan_ap_info_t ap_info;
    memset(&ap_info, 0, sizeof(ap_info));
    ap_info.authorize_mode = AP_AUTH_MODE;
    strcpy(ap_info.ssid, AP_SSID);
    strcpy(ap_info.password, AP_PASSWORD);
    vm_wlan_connect(&ap_info, wlan_connect_callback, NULL);

}

static void wlan_setmode_callback(void* user_data, VM_WLAN_REQUEST_RESULT result_type)
{
    if(VM_WLAN_REQUEST_DONE == result_type)
    {
        do_connect();
    }
}

static void connect_wlan(void)
{
    vm_wlan_register_notification_handler(VM_WLAN_NOTIFICATION_IP_AVAILABLE, wlan_noti_cb, NULL);
    vm_wlan_mode_set(VM_WLAN_MODE_STA, wlan_setmode_callback, NULL);
}
void handle_sysevt(VMINT message, VMINT param) 
{
    switch (message) 
    {
    case VM_EVENT_CREATE:
        connect_wlan();
        break;

    case VM_EVENT_QUIT:
        break;
    }
}

void vm_main(void) 
{
    vm_pmng_register_system_event_callback(handle_sysevt);
}

