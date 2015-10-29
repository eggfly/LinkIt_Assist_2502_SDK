/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  This example demonstrates how to turn on BT service as client,
  scan for BT devices, connect to BT device via SPP profile and
  write/read data to/from remote SPP device.

  This example firstly starts a 5s timer. After 5s it will try to
  start BT service (vm_bt_cm_switch_on), scan and search for
  device whose name is BT_NAME (vm_bt_cm_search). If the device
  is found, start SPP service and connect to the LinkIt Assist 2502
  (vm_bt_spp_connect). Finally, read (vm_bt_spp_read) and write
  (vm_bt_spp_write) from/to the LinkIt Assist 2502.

  Find a BT device with SPP profile, enable its BT device and
  modify macro BT_NAME to match its name.
*/

#include "vmtype.h" 
#include "vmsystem.h"
#include "vmlog.h" 
#include "vmtimer.h"
#include "vmbt_cm.h"
#include "vmbt_spp.h"
#include "vmstdlib.h"
#include "string.h"
#include "vmmemory.h"

/* Remote device that this example try to connect */
#define BT_NAME "SPP"
/* Standard SPP UUID */
#define UUID 0x1101
/* Data that this example try to send to remote */
#define SPP_DATA "Hello SPP!"

VMINT g_tid_switch = -1; /* timer id used to post BT process */
VMINT g_btcm_hdl = -1; /* handle of BT service */
VMINT g_btspp_hdl = -1; /* handle of SPP service */

static vm_bt_cm_bt_address_t g_btspp_addr; /* Store BT mac address of BT_NAME device we found during search process */
static VMINT g_btspp_min_buf_size;  /* size of buffer */
static void *g_btspp_buf; /* buffer that store SPP received data */
static VMINT g_b_server_find; /* if BT_NAME device is founded or not duing search process */


/* SPP servie callback handler */
void app_btspp_cb(VM_BT_SPP_EVENT evt, vm_bt_spp_event_cntx_t* param, void* user_data){
    vm_bt_spp_event_cntx_t *cntx = (vm_bt_spp_event_cntx_t*)param;
    VMINT ret;
    switch(evt){
        case VM_BT_SPP_EVENT_AUTHORIZE:{
            memset(&g_btspp_addr, 0, sizeof(g_btspp_addr));
            ret = vm_bt_spp_get_device_address(cntx->connection_id, &g_btspp_addr);
        }
        break;

        case VM_BT_SPP_EVENT_READY_TO_WRITE:{
            /* write SPP_DATA example string to remote side */
            if (cntx->result)
            {
                ret = vm_bt_spp_write(cntx->connection_id, SPP_DATA, strlen(SPP_DATA));
            }
        }
        break;

        case VM_BT_SPP_EVENT_READY_TO_READ:{
            /* read data from remote side and print it out to log */
            ret = vm_bt_spp_read(cntx->connection_id, g_btspp_buf, g_btspp_min_buf_size);
            if (ret > 0){
        	      /* log the received data */
                ((VMCHAR*)g_btspp_buf)[ret] = 0;
                vm_log_debug("BTSPP vm_btspp_read[%s]", g_btspp_buf);

                /* end of example, clean up resources */
                ret = vm_bt_spp_close(g_btspp_hdl);
                vm_log_debug("BTSPP vm_bt_spp_close[%d]", ret);
                if (g_btspp_buf){
                    vm_free(g_btspp_buf);
                    g_btspp_buf = NULL;
                }
                g_b_server_find = 0;

                /* turn off BT */
                ret = vm_bt_cm_switch_off();
            }
        }
        break;
    }
}

/* Init SPP servie and related resources */
static void app_btspp_start(void){
    VMINT result;
    VMUINT evt_mask = VM_BT_SPP_EVENT_START	|
        VM_BT_SPP_EVENT_BIND_FAIL |
        VM_BT_SPP_EVENT_AUTHORIZE |
        VM_BT_SPP_EVENT_CONNECT	|
        VM_BT_SPP_EVENT_SCO_CONNECT	|
        VM_BT_SPP_EVENT_READY_TO_WRITE |
        VM_BT_SPP_EVENT_READY_TO_READ |
        VM_BT_SPP_EVENT_DISCONNECT |
        VM_BT_SPP_EVENT_SCO_DISCONNECT;

    g_btspp_hdl = vm_bt_spp_open(evt_mask, app_btspp_cb, NULL);
    if(g_btspp_hdl < 0){
        return;
    }
    result = vm_bt_spp_set_security_level(g_btspp_hdl, VM_BT_SPP_SECURITY_NONE);

    g_btspp_min_buf_size = vm_bt_spp_get_min_buffer_size();

    g_btspp_buf = vm_calloc(g_btspp_min_buf_size);
    g_btspp_min_buf_size = g_btspp_min_buf_size / 2;
}

/* BT servie callback handler */
static void app_btcm_cb(VMUINT evt, void * param, void * user_data){
    VMINT ret;
    vm_log_debug("BTCM app_btcm_cb evt[0x%x]", evt);
    switch (evt){
        case VM_BT_CM_EVENT_ACTIVATE:{
            /* After activated, continue to scan for devices */
            vm_bt_cm_device_info_t dev_info = {0};
            /* display host info */
            ret = vm_bt_cm_get_host_device_info(&dev_info);
            vm_log_debug("BTCM vm_btcm_get_host_dev_info [%d]", ret);
            vm_log_debug("BTCM vm_btcm_get_host_dev_info[%s][0x%02x:%02x:%02x:%02x:%02x:%02x]", dev_info.name,
                ((dev_info.device_address.nap & 0xff00) >> 8),
                (dev_info.device_address.nap & 0x00ff),
                (dev_info.device_address.uap),
                ((dev_info.device_address.lap & 0xff0000) >> 16),
                ((dev_info.device_address.lap & 0x00ff00) >> 8),
                (dev_info.device_address.lap & 0x0000ff));
            /* init SPP services */
            app_btspp_start();
            break;
        }

        case VM_BT_CM_EVENT_DEACTIVATE:{
            ret = vm_bt_cm_exit(g_btcm_hdl);
            g_btcm_hdl = -1;
            break;
        }

        case VM_BT_CM_EVENT_INQUIRY_IND:{
        	/* check if we found the BT_NAME device. And if found, stop the scan */
            vm_bt_cm_inquiry_indication_t *ind = (vm_bt_cm_inquiry_indication_t *)param;
            if (ind->discovered_device_number > 0){
                VMUINT i = 0;
                vm_bt_cm_device_info_t dev_info = {0};
                for (i = 0; i < ind->discovered_device_number; i++){
                    vm_bt_cm_get_device_info_by_index(i, VM_BT_CM_DEVICE_DISCOVERED, &dev_info);
                    if (0 == strcmp(dev_info.name, BT_NAME)){
                        ret = vm_bt_cm_search_abort();
                        g_b_server_find = 1;
                        vm_log_debug("BTCM vm_btcm_search_abort [%d] [%s]", ret, ret, dev_info.name);
                        memcpy((void *)&g_btspp_addr, (void *)&dev_info.device_address, sizeof(g_btspp_addr));
                        break;
                    }
                }
            }
            break;
        }

        case VM_BT_CM_EVENT_INQUIRY_COMPLETE:{
            vm_bt_cm_inquiry_complete_t *cmpl = (vm_bt_cm_inquiry_complete_t *)param;
            if (g_b_server_find){
                ret = vm_bt_spp_connect(g_btspp_hdl,
                    &g_btspp_addr,
                    g_btspp_buf,
                    g_btspp_min_buf_size,
                    g_btspp_min_buf_size,
                    UUID);
            }
            ret = 0;
            break;
        }

        default:{
            break;
        }
    }
}

/* Init BT servie and turn on BT if necessary */
static void app_btcm_start(void){
    VMINT ret;
    g_btcm_hdl = vm_bt_cm_init(
        app_btcm_cb,
        VM_BT_CM_EVENT_ACTIVATE |
        VM_BT_CM_EVENT_DEACTIVATE |
        VM_BT_CM_EVENT_INQUIRY_IND |
        VM_BT_CM_EVENT_INQUIRY_COMPLETE,
        NULL);

    ret = vm_bt_cm_get_power_status();
    vm_log_debug("BTCM vm_btcm_get_power_status[%d]", ret);
    if (VM_BT_CM_POWER_OFF == ret){
    	/* Turn on BT if not yet on */
        ret = vm_bt_cm_switch_on();
    }
    else if (VM_BT_CM_POWER_ON == ret){
        /* if BT is already on */

        /* display host info */
        vm_bt_cm_device_info_t dev_info = {0};
        ret = vm_bt_cm_get_host_device_info(&dev_info);
        vm_log_debug("BTCM vm_btcm_get_host_dev_info [%d]", ret);
        vm_log_debug("BTCM vm_btcm_get_host_dev_info[%s][0x%02x:%02x:%02x:%02x:%02x:%02x]", dev_info.name,
            ((dev_info.device_address.nap & 0xff00) >> 8),
            (dev_info.device_address.nap & 0x00ff),
            (dev_info.device_address.uap),
            ((dev_info.device_address.lap & 0xff0000) >> 16),
            ((dev_info.device_address.lap & 0x00ff00) >> 8),
            (dev_info.device_address.lap & 0x0000ff));

        /* scan for devices */
        ret = vm_bt_cm_search(10, 50, 0xFFFFFFFF, 1);
        /* init SPP services */
        app_btspp_start();
    }
}

static void app_timer_cb(VMINT tid, void* user_data){
    if (tid == g_tid_switch){
    	/* start BT */
        app_btcm_start();

        /* stop timer */
        vm_timer_delete_precise(g_tid_switch);
        g_tid_switch = -1;
    }
}

/* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param){
    switch (message){
        case VM_EVENT_CREATE:
        /* wait 5 secs to start bt process because system init bt service */
        g_tid_switch = vm_timer_create_precise(5000, app_timer_cb, NULL);
        break;

        case VM_EVENT_QUIT:
        break;
    }
}

/* Entry point */
void vm_main(void){
    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
