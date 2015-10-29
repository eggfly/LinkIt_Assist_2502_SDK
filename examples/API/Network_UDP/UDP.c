/*
  This sample code is in public domain.

  This sample code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* 
  This sample gets the time data from NTP server by UDP.

  It creates a udp handle by vm_udp_create() to receive the udp messages, then
  call vm_udp_send() to send a packet to a NTP server. When NTP server responsed
  it will call vm_udp_receive() to read the data.

  You can change the NTP server address by modify g_address. If the SIM card you
  use doesn't have proxy APN, you need change the NETWORK_APN to
  VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NON_PROXY_APN.
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
#include "UDP.h"
#include "vmsock.h"
#include "vmbearer.h"
#include "vmudp.h"
#include "vmtimer.h"
#include "vmgsm_gprs.h"

/* Configurable macros */
#define CUST_APN    "cmwap"         /* The APN of your test SIM */
#define USING_PROXY VM_TRUE         /* Whether your SIM uses proxy */
#define PROXY_ADDRESS   "10.0.0.172"    /* The proxy address */
#define PROXY_PORT  80              /* The proxy port */
#define TCP_SYNC_START_TIMER    60000   /* Start connection 60 seconds later after bootup */


#define NTP_PACKET_SIZE 48
static VM_UDP_HANDLE g_udp;
vm_soc_address_t g_address = {4, 123, {132, 163, 4, 101}};
VMUINT8 g_packetBuffer[NTP_PACKET_SIZE];

static void udp_callback(VM_UDP_HANDLE handle, VM_UDP_EVENT event)
{
    VMCHAR buf[100] = {0};
    VMINT ret = 0;
    VMINT nwrite;
    
    vm_log_debug("event:%d", event);
    switch (event)
    {
        case VM_UDP_EVENT_READ:
            ret = vm_udp_receive(g_udp, buf, 100, &g_address);
            if(ret > 0)
            {
                vm_log_debug("received data: %s", buf);
            }
            vm_udp_close(g_udp);
            break;
        case VM_UDP_EVENT_WRITE:
            nwrite = vm_udp_send(g_udp, g_packetBuffer, NTP_PACKET_SIZE, &g_address);
            break;
        default:
            break;
    }

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

static void send_udp(VM_TIMER_ID_NON_PRECISE timer_id, void* user_data)
{
    VMINT nwrite;

    vm_timer_delete_non_precise(timer_id);
    set_custom_apn();
    g_udp = vm_udp_create(1000, VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN, udp_callback, 0);
    memset(g_packetBuffer, 0, NTP_PACKET_SIZE);
    g_packetBuffer[0] = 0x11100011;   // LI, Version, Mode
    g_packetBuffer[1] = 0;     // Stratum, or type of clock
    g_packetBuffer[2] = 6;     // Polling Interval
    g_packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    g_packetBuffer[12]  = 49;
    g_packetBuffer[13]  = 0x4E;
    g_packetBuffer[14]  = 49;
    g_packetBuffer[15]  = 52;
    nwrite = vm_udp_send(g_udp, g_packetBuffer, NTP_PACKET_SIZE, &g_address);
}
void handle_sysevt(VMINT message, VMINT param) 
{
    switch (message) 
    {
        case VM_EVENT_CREATE:
            vm_timer_create_non_precise(60000, send_udp, NULL);
            break;

        case VM_EVENT_QUIT:
            break;
    }
}

void vm_main(void) 
{
    vm_pmng_register_system_event_callback(handle_sysevt);
}

