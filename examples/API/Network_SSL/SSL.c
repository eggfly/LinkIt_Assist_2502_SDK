/*
  This sample code is in public domain.

  This sample code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* 
  This sample executes an HTTPS request and prints the content of the request to the log.

  It connects the port 443 of the host by API vm_ssl_connect(), a callback function
  is registered with this API. When the VM_SSL_EVENT_CAN_WRITE event is received, it
  will call vm_ssl_write() to send a HTTP request to get the URL. When the 
  VM_SSL_EVENT_CAN_READ even is received, it will call vm_ssl_read() to read the content.

  Before running this application, you need to put the certificate of the certification 
  authority of the host you want to connect to into the CERTIFICATE_PATH. You can change
  the URL, host and certificate path by modify the corresponding MACROs.
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
#include "SSL.h"
#include "vmssl.h"
#include "vmfs.h"
#include "vmtimer.h"
#include "vmgsm_gprs.h"

/* Configurable macros */
#define CUST_APN    "cmwap"         /* The APN of your test SIM */
#define USING_PROXY VM_TRUE         /* Whether your SIM uses proxy */
#define PROXY_ADDRESS   "10.0.0.172"    /* The proxy address */
#define PROXY_PORT  80              /* The proxy port */

#define CONNECT_URL "https://www.baidu.com/"
#define CONNECT_HOST    "www.baidu.com"
#define CERTIFICATE_PATH "C:\\Certificates\\Baidu.cer"

#define SSL_CERT_PATH_MAX_LENGTH 100
#define SSL_EXTERNAL_CERT_NUMBER    1
#define SSL_BUF_SIZE 100

static VM_SSL_HANDLE g_ssl_hd;
static VMCHAR g_ssl_write_buf[SSL_BUF_SIZE];
static VMCHAR g_ssl_read_buf[SSL_BUF_SIZE];

void set_cust_apn(void)
{
    vm_gsm_gprs_apn_info_t apn_info;
    
    memset(&apn_info, 0, sizeof(apn_info));
    strcpy(apn_info.apn, CUST_APN);
    strcpy(apn_info.proxy_address, PROXY_ADDRESS);
    apn_info.proxy_port = PROXY_PORT;
    apn_info.using_proxy = USING_PROXY;
    vm_gsm_gprs_set_customized_apn_info(&apn_info);
}

static void ssl_connection_callback(VMINT handle, VMINT event)
{
    VMINT read_size, write_size;
    VMCHAR log_buf[40];
    VM_SSL_VERIFY_RESULT verify_result;
    
    switch(event)
    {
        case VM_SSL_EVENT_CONNECTED:
            g_ssl_hd = handle;
        case VM_SSL_EVENT_CAN_WRITE:
            sprintf(g_ssl_write_buf, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", CONNECT_URL, CONNECT_HOST);
            write_size = vm_ssl_write(handle, g_ssl_write_buf, strlen(g_ssl_write_buf));
            break;
        case VM_SSL_EVENT_CAN_READ:
            read_size = vm_ssl_read(handle, g_ssl_read_buf, SSL_BUF_SIZE);
            while(read_size > 0)
            {
                vm_log_info(g_ssl_read_buf);
                read_size = vm_ssl_read(handle, g_ssl_read_buf, SSL_BUF_SIZE);
            }
            break;
        case VM_SSL_EVENT_CERTIFICATE_VALIDATION_FAILED:
        case VM_SSL_EVENT_HANDSHAKE_FAILED:
            verify_result = vm_ssl_get_verify_result(g_ssl_hd);
            break;
            
    }
}
void load_external_cert(void)
{
    VMWCHAR file_name[SSL_CERT_PATH_MAX_LENGTH + 1];
    VMSTR cert_paths[SSL_EXTERNAL_CERT_NUMBER];
    VM_FS_HANDLE file_handle;
    VMUINT8* cert_buffer;
    VMUINT file_size = 0;
    VMUINT read_size;
    VM_RESULT result;
    VMINT i;
    
    cert_paths[0] = CERTIFICATE_PATH;
    for( i = 0; i < SSL_EXTERNAL_CERT_NUMBER; i++)
    {
        memset(file_name, 0, sizeof(file_name));
        vm_chset_ascii_to_ucs2(file_name, SSL_CERT_PATH_MAX_LENGTH, cert_paths[i]);
        do
        {
            file_handle = vm_fs_open(file_name, VM_FS_MODE_READ, VM_TRUE);
            if(!(VM_IS_SUCCEEDED(file_handle)))
            {
                vm_log_info("open file failed");
                break;
            }
            result = vm_fs_get_size(file_handle, &file_size);
            if(!(VM_IS_SUCCEEDED(result)))
            {
                vm_log_info("get file size failed");
                break;
            }
            cert_buffer = vm_malloc(file_size);
            if(NULL == cert_buffer)
            {
                vm_log_info("allocate memory failed");
                break;
            }
            result = vm_fs_read(file_handle, cert_buffer, file_size, &read_size);
            if(read_size != file_size)
            {
                vm_log_info("read file failed, read_size:%d, file_size%d", read_size, file_size);
                break;
            }
            result = vm_ssl_load_ca_chain_certificate(g_ssl_hd, cert_buffer, file_size);
            if(!(VM_IS_SUCCEEDED(result)))
            {
                vm_log_info("load ca chain failed, result:%d", result);
            }
        }while(0);
        if(NULL != cert_buffer )
        {
            vm_free(cert_buffer);
            cert_buffer = NULL;
        }
        if(file_handle >= 0)
        {
            vm_fs_close(file_handle);
            file_handle = -1;
        }
    }
}
static void ssl_timer_callback(VM_TIMER_ID_NON_PRECISE timer_id, void* user_data)
{
    
    vm_ssl_context_t context;

    vm_timer_delete_non_precise(timer_id);
    
    context.host = CONNECT_HOST;
    context.port = 443;
    context.connection_callback = ssl_connection_callback;
    context.authorize_mode = VM_SSL_VERIFY_REQUIRED;
    context.user_agent = NULL;
    set_cust_apn();
    g_ssl_hd = vm_ssl_connect(&context);
    if(VM_IS_SUCCEEDED(g_ssl_hd))
    {
        load_external_cert();
    }
}

void handle_sysevt(VMINT message, VMINT param) 
{
    
    switch (message) 
    {
    case VM_EVENT_CREATE:
        /* wait a moment until the network and SSL modules have initiated successfully after boot up */
        vm_timer_create_non_precise(60 * 1000, ssl_timer_callback, NULL);
        break;

    case VM_EVENT_QUIT:
        break;
    }
}

void vm_main(void) 
{
    vm_pmng_register_system_event_callback(handle_sysevt);
}

