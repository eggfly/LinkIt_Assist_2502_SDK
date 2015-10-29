/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  This example demonstrates how to use BLE GATT client.
  In this example, it will connect to the server which is search out first.
*/

#include "vmtype.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmlog.h"
#include "vmmemory.h"
#include "ResID.h"
#include "GATTClient.h"
#include "vmbt_gatt.h"
#include "vmlog.h"
#include "vmbt_cm.h"
#include "vmsystem.h"
#include "vmtimer.h"
#include "stdio.h"


static void app_client_check_bt_on_off(void);
static void app_client_bt_init_cb(VMUINT evt,void * param,void * user_data);
void app_client_init(void);
VMINT app_client_deinit(void);
static void app_client_register_client_callback(void *context_handle, VMBOOL status, VMUINT8 app_uuid[16]);

/* Callback for scan results */
static void app_client_vmt_scan_result_callback(void *context_handle, vm_bt_gatt_address_t *bd_addr, VMINT32 rssi, VMUINT8 eir_len, VMUINT8 *eir);

/*Callback indicating that a remote device has connected or been disconnected */
static void app_client_connection_callback(vm_bt_gatt_connection_t *conn, VMBOOL connected, vm_bt_gatt_address_t *bd_addr);

/* Callback triggered in response to listen */
static void app_client_listen_callback(void *context_handle, VMBOOL status);

/*Callback triggered in response to set_adv_data */
static void app_client_set_adv_data_callback(void *context_handle, VMBOOL status){}

/**
 * Invoked in response to search_service when the GATT service search
 * has been completed.
 */
static void app_client_search_complete_callback(void *context_handle, VMBOOL status);

/*Reports GATT services on a remote device */
static void app_client_search_result_callback(vm_bt_gatt_connection_t *conn, vm_bt_gatt_service_info_t *uuid);

/* GATT characteristic enumeration result callback */
static void app_client_get_characteristic_callback(vm_bt_gatt_connection_t *conn, VMBOOL status,
                                vm_bt_gatt_client_characteristic_t *ch, VM_BT_GATT_CHAR_PROPERTIES properties);

/*GATT descriptor enumeration result callback */
static void app_client_get_descriptor_callback(vm_bt_gatt_connection_t *conn, VMBOOL status, vm_bt_gatt_client_descriptor_t *descr){}

/* GATT included service enumeration result callback */
static void app_client_get_included_service_callback(vm_bt_gatt_connection_t *conn, VMBOOL status,
                                vm_bt_gatt_service_info_t *svc_uuid, vm_bt_gatt_service_info_t *incl_svc_uuid){}

/* Callback invoked in response to [de]register_for_notification */
static void app_client_register_for_notification_callback(void *context_handle, VMBOOL status,
                                vm_bt_gatt_address_t *bd_addr, vm_bt_gatt_client_characteristic_t *ch){}

/*
 * Remote device notification callback, invoked when a remote device sends
 * a notification or indication that a client has registered for.
 */
static void app_client_notify_callback(vm_bt_gatt_connection_t *conn, vm_bt_gatt_address_t *bd_addr,
                                vm_bt_gatt_client_characteristic_t *ch, vm_bt_gatt_attribute_value_t *value, VMBOOL is_notify){}

/* Reports result of a GATT read operation */
static void app_client_read_characteristic_callback(vm_bt_gatt_connection_t *conn, VMBOOL status,
                                vm_bt_gatt_client_characteristic_t *ch, vm_bt_gatt_attribute_value_t *value);

/* GATT write characteristic operation callback */
static void app_client_write_characteristic_callback(vm_bt_gatt_connection_t *conn, VMBOOL status,
                                vm_bt_gatt_client_characteristic_t *ch);

/* Callback invoked in response to read_descriptor */
static void app_client_read_descriptor_callback(vm_bt_gatt_connection_t *conn, VMBOOL status,
                                vm_bt_gatt_client_descriptor_t *descr, vm_bt_gatt_attribute_value_t *value){}

/* Callback invoked in response to write_descriptor */
static void app_client_write_descriptor_callback(vm_bt_gatt_connection_t *conn, VMBOOL status,
                                vm_bt_gatt_client_descriptor_t *descr){}

/* GATT execute prepared write callback */
static void app_client_execute_write_callback(vm_bt_gatt_connection_t *conn, VMBOOL status);

/* Callback triggered in response to read_remote_rssi */
static void app_client_read_remote_rssi_callback(void *context_handle, VMBOOL status, vm_bt_gatt_address_t *bd_addr, VMINT32 rssi){}

/* Callback triggered in response to get_device_type */
static void app_client_get_device_type_callback(void *context_handle, VMBOOL status, vm_bt_gatt_address_t *bd_addr, VM_BT_GATT_CLIENT_DEV_TYPE dev_type){}



void timer_cb(VM_TIMER_ID_NON_PRECISE tid, void* user_data)
{
    app_client_check_bt_on_off();
    vm_timer_delete_non_precise(tid);
}


void vm_main(void)
{

    VM_TIMER_ID_NON_PRECISE timer_id = 0;
    timer_id = vm_timer_create_non_precise(1000, (vm_timer_non_precise_callback)timer_cb, NULL);
    vm_log_debug("create timer [%d]", timer_id);
}


void app_client_callback_init(vm_bt_gatt_client_callback_t *gattc_cb)
{
    gattc_cb->register_client = app_client_register_client_callback;
    gattc_cb->scan_result = app_client_vmt_scan_result_callback ;
    gattc_cb->connection = app_client_connection_callback;
    gattc_cb->listen = app_client_listen_callback;
    gattc_cb->set_advertisement_data = app_client_set_adv_data_callback;
    gattc_cb->search_complete = app_client_search_complete_callback;
    gattc_cb->search_result = app_client_search_result_callback;
    gattc_cb->get_characteristic = app_client_get_characteristic_callback;
    gattc_cb->get_descriptor = app_client_get_descriptor_callback;
    gattc_cb->get_included_service = app_client_get_included_service_callback;
    gattc_cb->register_for_notification = app_client_register_for_notification_callback;
    gattc_cb->notify = app_client_notify_callback;
    gattc_cb->read_characteristic = app_client_read_characteristic_callback;
    gattc_cb->write_characteristic = app_client_write_characteristic_callback;
    gattc_cb->read_descriptor = app_client_read_descriptor_callback;
    gattc_cb->write_descriptor = app_client_write_descriptor_callback;
    gattc_cb->execute_write = app_client_execute_write_callback;
    gattc_cb->read_remote_rssi = app_client_read_remote_rssi_callback;
    gattc_cb->get_device_type = app_client_get_device_type_callback;
    return;
}

void app_client_check_bt_on_off(void)
{
    vm_bt_cm_init(app_client_bt_init_cb,0x00080000 | 0x00000001,NULL);
    if (VM_BT_CM_POWER_ON == vm_bt_cm_get_power_status())
    {
        app_client_init();
    }
    else
    {
        vm_bt_cm_switch_on();
    }

}
void app_client_bt_init_cb(VMUINT evt,void * param,void * user_data)
{
    if (0x00000001 == evt || 0x00080000 == evt)
    {
        app_client_init();
    }
}

void app_client_init(void)
{
    vm_log_debug("[AppClient] appc_init state %d!\n", g_appc_cntx.state);
    if(g_appc_cntx.state == APPC_STATUS_DISABLED)
    {
        g_appc_cntx.state = APPC_STATUS_ENABLING;
        memset(g_appc_cntx.uid,0x0,sizeof(g_appc_cntx.uid));
        memcpy(g_appc_cntx.uid, g_appc_uid, sizeof(g_appc_cntx.uid));
        app_client_callback_init(&g_appc_cb);
        vm_bt_gatt_client_register(g_appc_cntx.uid, &g_appc_cb);
        vm_log_debug("[AppClient] appc_init_end");
    }
    return;
}

void app_client_register_client_callback(void *context_handle, VMBOOL status, VMUINT8 app_uuid[16])
{
    vm_log_debug("[AppClient] reg status %d state %d!\n", status, g_appc_cntx.state);
    if(memcmp(app_uuid, g_appc_cntx.uid, sizeof(g_appc_cntx.uid)) == 0)
    {
        if(g_appc_cntx.state == APPC_STATUS_ENABLING)
        {
            if(status == 0)
            {
                vm_log_debug("[AppClient] reg status_ok %d state_ok %d!\n", status, g_appc_cntx.state);
                g_appc_cntx.context_handle = context_handle;
                g_appc_cntx.state = APPC_STATUS_ENABLED;
                vm_bt_gatt_client_scan(context_handle, VM_TRUE);
            }
            else
            {
                g_appc_cntx.context_handle = NULL;
                g_appc_cntx.state = APPC_STATUS_DISABLED;
            }
        }
        else if(g_appc_cntx.state == APPC_STATUS_DISABLING)
        {
            if(status == 0)
            {
                g_appc_cntx.context_handle = NULL;
                g_appc_cntx.state = APPC_STATUS_DISABLED;
            }
            else
            {
                g_appc_cntx.state = APPC_STATUS_ENABLED;
            }
        }
    }
    vm_log_debug("[AppClient] reg -!\n");
}

void app_client_listen_callback(void *context_handle, VMBOOL status)
{
    vm_log_debug("[AppClient] app_client_listen_callback status %d\n", status);
    if ((g_appc_cntx.context_handle == context_handle) && (NULL != context_handle))
    {
        if(status == 0)
        {
            vm_log_debug("[AppClient] app_client_listen_callback statusok %d!\n", status);
            vm_bt_gatt_client_scan(context_handle, VM_TRUE);
        }
    }
}

static VMUINT32 app_client_get_free_index(void)
{
    VMUINT32 idx;

    for (idx = 0; idx < 10; idx++)
    {
        if (g_app_client_bd_addr_list[idx].data[0] == 0
         && g_app_client_bd_addr_list[idx].data[1] == 0
         && g_app_client_bd_addr_list[idx].data[2] == 0
         && g_app_client_bd_addr_list[idx].data[3] == 0
         && g_app_client_bd_addr_list[idx].data[4] == 0
         && g_app_client_bd_addr_list[idx].data[5] == 0)
        {
            return idx;
        }
    }
    return 100;
}

void app_client_vmt_scan_result_callback(void *context_handle, vm_bt_gatt_address_t *bd_addr, VMINT32 rssi, VMUINT8 eir_len, VMUINT8 *eir)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    VMUINT32 idx;
    VMUINT8 server_addr[VM_BT_GATT_ADDRESS_SIZE] = {0xC9,0x92,0x65,0x46,0x5B,0x7E};
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    vm_log_debug("[AppClient] app_client_vmt_scan_result_callback bd_addr %x:%x:%x:%x:%x:%x!\n",
        bd_addr->data[0],bd_addr->data[1],
    bd_addr->data[2],bd_addr->data[3],bd_addr->data[4],bd_addr->data[5]);
    if(g_appc_cntx.context_handle == context_handle)
    {
        idx = app_client_get_free_index();
        if (idx < 10)
        {
            memcpy(&g_app_client_bd_addr_list[idx], bd_addr, sizeof(vm_bt_gatt_address_t));

            vm_bt_gatt_client_scan(context_handle,VM_FALSE);
            vm_bt_gatt_client_connect(context_handle, bd_addr, VM_TRUE);
            vm_log_debug("[AppClient] dev index:%d\n", idx);
        }
    }
}


void app_client_connection_callback(vm_bt_gatt_connection_t *conn, VMBOOL connected, vm_bt_gatt_address_t *bd_addr)
{
    vm_log_debug("[AppClient] app_client_connection_callback connected %d!\n", connected);

    if((memcmp(bd_addr->data, g_appc_conn_cntx->bdaddr, VM_BT_GATT_ADDRESS_SIZE) == 0) && (NULL != bd_addr))
    {
        vm_log_debug("[AppClient] find in list!\n");
        g_appc_conn_cntx->connection_handle = conn->connection_handle;
        if(connected  && (g_appc_conn_cntx->conn_status != APPC_STATUS_CONNECTED))
        {
            /* do next step Discovery all */
            vm_log_debug("[AppClient] have connected,find in list!\n");
            g_appc_conn_cntx->conn_status = APPC_STATUS_CONNECTING;
            vm_bt_gatt_client_search_service(conn, NULL);
        }
        else if(!connected)
        {
            vm_log_debug("[AppClient] connection failed");
        }
        return;
    }
    if(connected)
    {
        vm_log_debug("[AppClient] connect success, and not find in list!\n");
        if (g_appc_conn_cntx == NULL)
        {
            g_appc_conn_cntx = (AppClientConnCntx *)vm_malloc(sizeof(AppClientConnCntx));
        }
        memset(g_appc_conn_cntx, 0x0, sizeof(AppClientConnCntx));
        memcpy(g_appc_conn_cntx->bdaddr, bd_addr->data, VM_BT_GATT_ADDRESS_SIZE);
        g_appc_conn_cntx->connection_handle = conn->connection_handle;
        g_appc_conn_cntx->conn_status = APPC_STATUS_CONNECTING;
        vm_bt_gatt_client_search_service(conn, NULL);
    }
}


void app_client_search_complete_callback(void *context_handle, VMBOOL status)
{
    vm_log_debug("[AppClient] Client Done [%d]", status);
    if(g_appc_cntx.context_handle == context_handle)
    {
        vm_log_debug("[AppClient] seach complete !\n");
        if(status == 0)
        {
            if(g_appc_conn_cntx->conn_status == APPC_STATUS_CONNECTING)
            {
                g_appc_conn_cntx->conn_status = APPC_STATUS_CONNECTED;
            }
        }
        return;
    }

}


void app_client_search_result_callback(vm_bt_gatt_connection_t *conn, vm_bt_gatt_service_info_t *uuid)
{

    vm_log_debug("[AppClient] scaning");
    if(memcmp(uuid->uuid.uuid.uuid,g_appc_srv_uuid, (sizeof(VMUINT8) * 16)) == 0)
    {
        vm_log_debug("[AppClient]service2 has been find !\n");
        if(g_appc_conn_cntx->connection_handle == conn->connection_handle)
        {
            vm_log_debug("[AppClient] to get character !\n");
            vm_bt_gatt_client_get_characteristic(conn, uuid, NULL);
            return;
        }
    }
}


void app_client_get_characteristic_callback(vm_bt_gatt_connection_t *conn, VMBOOL status,
                                    vm_bt_gatt_client_characteristic_t *ch, VM_BT_GATT_CHAR_PROPERTIES properties)
{
    vm_log_debug("[AppClient] app_client_get_characteristic_callback status %d!\n", status);
    if(g_appc_conn_cntx->connection_handle == conn->connection_handle)
    {
        if(memcmp((ch->svc_uuid->uuid.uuid.uuid),g_appc_srv_uuid, (sizeof(VMUINT8) * 16)) == 0)
        {
            if(memcmp(ch->ch_uuid->uuid.uuid, g_appc_char_uuid, (sizeof(VMUINT8) * 16)) == 0)
            {
                vm_log_debug("[AppClient]service2 has been find !\n");
                if(status == 0)
                {
                    if (properties & (VM_BT_GATT_CHAR_PROPERTY_READ))
                    {
                        vm_log_debug("[AppClient] read service2  !\n");
                        vm_bt_gatt_client_read_characteristic(conn,ch,VM_BT_GATT_CLIENT_AUTH_REQ_NONE);
                    }
                    else if (properties & (VM_BT_GATT_CHAR_PROPERTY_WRITE || VM_BT_GATT_CHAR_PROPERTY_WRITE_WO_RESPONSE))
                    {
                        g_char_value.data[0] = 0;
                        g_char_value.length = 1;
                        vm_log_debug("[AppClient] write service2 !\n");
                        vm_bt_gatt_client_write_characteristic(conn,
                           ch,
                           &g_char_value,
                           VM_BT_GATT_CLIENT_WRITE_TYPE_PREPARE,
                           VM_BT_GATT_CLIENT_AUTH_REQ_NONE);
                    }
                }
            }
        }
        else
        {
            if(status == 0)
            {
                vm_bt_gatt_client_get_characteristic(conn, ch->svc_uuid, ch->ch_uuid);
            }
        }
    }
    return ;
}


void app_client_write_characteristic_callback(vm_bt_gatt_connection_t *conn, VMBOOL status,
                                vm_bt_gatt_client_characteristic_t *ch)
{
    vm_log_debug("[AppClient] app_client_write_characteristic_callback status %d!\n", status);
    if(g_appc_conn_cntx->connection_handle == conn->connection_handle)
    {
        if(memcmp(ch->svc_uuid->uuid.uuid.uuid, g_appc_srv_uuid, (sizeof(VMUINT8) * 16)) == 0)
        {
            if(memcmp(ch->ch_uuid->uuid.uuid, g_appc_char_uuid, (sizeof(VMUINT8) * 16)) == 0)
            {
                if(status == 0)
                {
                    vm_bt_gatt_client_execute_write(conn,1);
                    vm_log_debug("[AppClient] service execute write execute=1 !\n");
                }
            }
        }

    }
    return;
}


void app_client_execute_write_callback(vm_bt_gatt_connection_t *conn, VMBOOL status)
{
    vm_log_debug("[AppClient] app_client_execute_write_callback !\n");

    if(g_appc_conn_cntx->connection_handle == conn->connection_handle)
    {
        if(status == 0)
        {
            vm_log_debug("[AppClient] Client Done !\n");
        }
    }
    return;
}


/* Reports result of a GATT read operation */
void app_client_read_characteristic_callback(vm_bt_gatt_connection_t *conn, VMBOOL status,
                                vm_bt_gatt_client_characteristic_t *ch, vm_bt_gatt_attribute_value_t *value)
{
    vm_log_debug("[AppClient] app_client_read_characteristic_callback status %d!\n", status);
    if(g_appc_conn_cntx->connection_handle == conn->connection_handle)
    {
        if(memcmp(ch->svc_uuid->uuid.uuid.uuid,g_appc_srv_uuid, (sizeof(VMUINT8) * 16)) == 0)
        {
            if(memcmp(ch->ch_uuid->uuid.uuid, g_appc_char_uuid, (sizeof(VMUINT8) * 16)) == 0)
            {
                if(status == 0)
                {
                    vm_log_debug("[AppClient] service read done value %x:%x:%x:%x:%x:%x!\n", value->data[0],value->data[0],
                                 value->data[2],value->data[3],value->data[4],value->data[5]);
                    /*to test write API*/
                    {
                        g_char_value.data[0] = 0;
                        g_char_value.length = 1;
                        vm_log_debug("[AppClient] write service2 !\n");
                        vm_bt_gatt_client_write_characteristic(conn,
                           ch,
                           &g_char_value,
                           VM_BT_GATT_CLIENT_WRITE_TYPE_PREPARE,
                           VM_BT_GATT_CLIENT_AUTH_REQ_NONE);
                    }
                }
            }
        }
    }
    return;
}


VMINT app_client_deinit(void)
{
    vm_log_debug("[AppClient] app_client_deinit state %d!\n", g_appc_cntx.state);
    if((g_appc_cntx.state == APPC_STATUS_DISABLED)
        || (g_appc_cntx.state == APPC_STATUS_DISABLING))
        return 0;

    if(g_appc_cntx.state == APPC_STATUS_ENABLED)
    {
        {
            g_client_gatt_conn.context_handle = g_appc_cntx.context_handle;
            g_client_gatt_conn.connection_handle = g_appc_conn_cntx->connection_handle;
            if((g_appc_conn_cntx->conn_status == APPC_STATUS_CONNECTED)
                || (g_appc_conn_cntx->conn_status == APPC_STATUS_CONNECTING))
            {
                g_client_bd_addr = (vm_bt_gatt_address_t *)vm_malloc(sizeof(vm_bt_gatt_address_t));
                memset(g_client_bd_addr, 0x0, sizeof(vm_bt_gatt_address_t));
                memcpy(g_client_bd_addr->data, g_appc_conn_cntx->bdaddr, VM_BT_GATT_ADDRESS_SIZE);
                vm_bt_gatt_client_disconnect(&g_client_gatt_conn, g_client_bd_addr);
            }
        }
    }

    vm_bt_gatt_client_deregister(g_appc_cntx.context_handle);
    g_appc_cntx.state = APPC_STATUS_DISABLED;

    if (g_appc_conn_cntx)
    {
        vm_free(g_appc_conn_cntx);
    }
    if (g_client_bd_addr)
    {
        vm_free(g_client_bd_addr);
    }
    return 1;
}

