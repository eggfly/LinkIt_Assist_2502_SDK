/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  This example shows how to use BLE GATT server.
  In this example, you can use the example GATTClient to connect this server.
*/

#include "vmtype.h" 
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmfs.h"
#include "vmlog.h" 
#include "vmwdt.h"
#include "vmcmd.h" 
#include "vmmemory.h"
#include "ResID.h"
#include "GATTServer.h"
#include "vmbt_gatt.h"
#include "vmlog.h"
#include "vmbt_cm.h"
#include "vmsystem.h"
#include "vmtimer.h"
#include <stdio.h>

typedef enum {
    APP_STATUS_DISABLED = 0,
    APP_STATUS_ENABLING,
    APP_STATUS_ENABLED,
    APP_STATUS_DISABLING,
}APP_STATUS;

typedef enum {
    APP_STATUS_DISCONNECTED = 0,
    APP_STATUS_CONNECTING,
    APP_STATUS_CONNECTED,
    APP_STATUS_DISCONNECTING,
}APP_CONNECTION_STATUS;

typedef enum {
    APP_OP_INIT = 0,
    APP_OP_GET_SERVICE,
    APP_OP_ADD_SERVICE,
    APP_OP_DEL_SERVICE,
    APP_OP_ADD_CHARACTERISTIC,
    APP_OP_DEL_CHARACTERISTIC,
    APP_OP_START_SERVICE,
    APP_OP_STOP_SERVICE,
    APP_OP_DEINIT,
}APP_OPERATION;

typedef struct {
    APP_OPERATION        op_flag;
    APP_STATUS           state;
    void                 *context_handle;
    VMUINT8        uid[16];
}AppCntx;

typedef struct {
    void                 *connection_handle;
    APP_CONNECTION_STATUS                   conn_status;
    VMCHAR                  bdaddr[VM_BT_GATT_ADDRESS_SIZE];
}AppConnCntx;


typedef struct {
    vm_bt_gatt_connection_t     gatt_conn;
    vm_bt_gatt_address_t                 bdaddr[VM_BT_GATT_ADDRESS_SIZE];
    VMUINT16                  trans_id;
    VM_BT_GATT_ATTRIBUTE_HANDLE           att_handle;
    VMUINT16                  offset;
    vm_bt_gatt_attribute_value_t             att_value;
}AppReq;


AppCntx  g_app_cntx = {0,};
vm_bt_gatt_service_info_t   *gatt_srv_uuid = NULL;
vm_bt_gatt_attribute_uuid_t   *gatt_char_uuid = NULL;
VM_BT_GATT_ATTRIBUTE_HANDLE g_srvc_handle = 0;
VM_BT_GATT_ATTRIBUTE_HANDLE g_char_handle = 0;
AppReq *g_tx_power = NULL;

AppConnCntx *g_conn_cntx = NULL;
vm_bt_gatt_attribute_value_t read_att_value;
vm_bt_gatt_attribute_value_t write_att_value;
vm_bt_gatt_connection_t gatt_conn;
vm_bt_gatt_address_t            *gatt_bd_addr;


VMUINT8 g_app_uid[] = {
    0x18, 0xA0, 0x1F, 0x49, 0xFF, 0xE5, 0x40, 0x56,
    0X84, 0x5B, 0x6D, 0xF1, 0xF1, 0xB1, 0x6E, 0x9D
};

VMUINT8 g_srv_uuid[] = {
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0X80,
    0x00, 0x10, 0x00, 0x00, 0xFF, 0x18, 0x00, 0x00
};
VMUINT8 g_char_uuid[] = {
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0X80,
    0x00, 0x10, 0x00, 0x00, 0xA0, 0x2A, 0x00, 0x00
};


vm_bt_gatt_server_callback_t app_gatts_cb;
static void app_check_bt_on_off(void);
static void app_bt_init_cb(VMUINT evt,void * param,void * user_data);

static void app_init(void);
static void app_deinit(void);
static VMUINT16 app_convert_array_to_uuid16(vm_bt_uuid_with_length_t *uu);
static void app_register_server_cb(void *context_handle, VMBOOL status, VMUINT8 app_uuid[16]);
static void app_connection_cb(const vm_bt_gatt_connection_t *conn, VMBOOL connected, const vm_bt_gatt_address_t *bd_addr);
static void app_listen_cb(void *context_handle, VMBOOL status);
static void app_service_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_service_info_t *srvc_id, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);
static void app_included_service_added_cb(VMBOOL status, void *context_handle,
                VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE incl_srvc_handle);
static void app_characteristic_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_attribute_uuid_t *uuid, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE char_handle);
static void app_descriptor_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_attribute_uuid_t *uuid, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE descr_handle);
static void app_service_started_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);
static void app_service_stopped_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);
static void app_service_deleted_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);
static void app_request_read_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                                      VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, VMUINT16 offset, VMBOOL is_long);
static void app_request_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                                       VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, vm_bt_gatt_attribute_value_t *value, VMUINT16 offset,
                                       VMBOOL need_rsp, VMBOOL is_prep);
static void app_request_exec_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id,
                                            vm_bt_gatt_address_t *bd_addr, VMBOOL cancel);
static void app_response_confirmation_cb(VMBOOL status, VM_BT_GATT_ATTRIBUTE_HANDLE handle);
static void app_read_tx_power_cb(void *context_handle, VMBOOL status, vm_bt_gatt_address_t *bd_addr, VMUINT8 tx_power);


void timer_cb(VM_TIMER_ID_NON_PRECISE tid, void* user_data)
{
    app_check_bt_on_off();
    vm_timer_delete_non_precise(tid);
}


void vm_main(void)
{
    VM_TIMER_ID_NON_PRECISE timer_id = 0;
    timer_id = vm_timer_create_non_precise(1000, (vm_timer_non_precise_callback)timer_cb, NULL);
    vm_log_debug("create timer [%d]", timer_id);
}


void app_gatts_callback_init(vm_bt_gatt_server_callback_t *app_gatts_callback)
{

    app_gatts_callback->characteristic_added = app_characteristic_added_cb;
    app_gatts_callback->connection = app_connection_cb;
    app_gatts_callback->descriptor_added = app_descriptor_added_cb;
    app_gatts_callback->included_service_added = app_included_service_added_cb;
    app_gatts_callback->listen = app_listen_cb;
    app_gatts_callback->read_tx_power = app_read_tx_power_cb;
    app_gatts_callback->register_server = app_register_server_cb;
    app_gatts_callback->request_exec_write = app_request_exec_write_cb;
    app_gatts_callback->request_read = app_request_read_cb;
    app_gatts_callback->request_write = app_request_write_cb;
    app_gatts_callback->response_confirmation = app_response_confirmation_cb;
    app_gatts_callback->service_added = app_service_added_cb;
    app_gatts_callback->service_deleted = app_service_deleted_cb;
    app_gatts_callback->service_started = app_service_started_cb;
    app_gatts_callback->service_stopped = app_service_stopped_cb;

    return;

}
void app_check_bt_on_off(void)
{
    vm_bt_cm_init(app_bt_init_cb,0x00080000 | 0x00000001,NULL);
    vm_log_debug("[TestApp] app_check_bt_on_off");
    if (VM_BT_CM_POWER_ON == vm_bt_cm_get_power_status())
    {
        vm_log_debug("[TestApp] VM_BT_CM_POWER_ON already");
        app_init();
    }
    else
    {
        vm_log_debug("[TestApp] to vm_bt_cm_switch_on");
        vm_bt_cm_switch_on();
    }

}
void app_bt_init_cb(VMUINT evt,void * param,void * user_data)
{
    if (0x00000001 == evt || 0x00080000 == evt)
    {
        app_init();
    }
}

void app_init(void)
{
    vm_log_debug("[TestApp] app_init_begain g_app_cntx.state= %d g_app_cntx.op_flag = %d",g_app_cntx.state, g_app_cntx.op_flag);
    if(g_app_cntx.state == APP_STATUS_DISABLED)
    {
        g_app_cntx.state = APP_STATUS_ENABLING;
        g_app_cntx.op_flag = APP_OP_INIT;
        memset(g_app_cntx.uid,0x0,sizeof(g_app_cntx.uid));
        memcpy(g_app_cntx.uid, &g_app_uid, sizeof(g_app_cntx.uid));
        app_gatts_callback_init(&app_gatts_cb);
        vm_bt_gatt_server_register(g_app_cntx.uid, &app_gatts_cb);
    }
    else if(g_app_cntx.state == APP_STATUS_DISABLING)
    {
        if(g_app_cntx.op_flag == APP_OP_DEINIT)
        {
            g_app_cntx.op_flag = APP_OP_INIT;
        }
        memset(g_app_cntx.uid,0x0,sizeof(g_app_cntx.uid));
        memcpy(g_app_cntx.uid, &g_app_uid, sizeof(g_app_cntx.uid));
    }
    vm_log_debug("[TestApp] app_init_end");
    return;
}

void app_deinit(void)
{

    vm_log_debug("[TestApp] app_deinit state %d, op_flag %d!\n", g_app_cntx.state, g_app_cntx.op_flag);
    if((g_app_cntx.state == APP_STATUS_DISABLED)
        || (g_app_cntx.state == APP_STATUS_DISABLING))
        return;

    vm_bt_gatt_server_listen(g_app_cntx.context_handle, FALSE);
    if(g_app_cntx.state == APP_STATUS_ENABLED)
    {
        gatt_conn.context_handle = g_app_cntx.context_handle;
        gatt_conn.connection_handle = g_conn_cntx->connection_handle;
        if((g_conn_cntx->conn_status == APP_STATUS_CONNECTED)
                || (g_conn_cntx->conn_status == APP_STATUS_CONNECTING))
        {
            gatt_bd_addr = (vm_bt_gatt_address_t *)vm_malloc(sizeof(vm_bt_gatt_address_t));
            memset(gatt_bd_addr, 0x0, sizeof(vm_bt_gatt_address_t));
            memcpy(gatt_bd_addr->data, g_conn_cntx->bdaddr, VM_BT_GATT_ADDRESS_SIZE);
            vm_bt_gatt_server_disconnect(&gatt_conn, gatt_bd_addr);
        }
    }
    vm_bt_gatt_server_stop_service(g_app_cntx.context_handle, g_srvc_handle);
    vm_bt_gatt_server_delete_service(g_app_cntx.context_handle, g_srvc_handle);
    vm_bt_gatt_server_deregister(g_app_cntx.context_handle);
    g_app_cntx.state = APP_STATUS_DISABLED;
    g_app_cntx.op_flag = APP_OP_DEINIT;
    gatt_conn.context_handle = NULL;
    gatt_conn.connection_handle = NULL;
    if (gatt_srv_uuid)
    {
        vm_free(gatt_srv_uuid);
    }
    if (gatt_char_uuid)
    {
        vm_free(gatt_char_uuid);
    }
    if(g_tx_power)
    {
        vm_free(g_tx_power);
    }
    if(gatt_bd_addr)
    {
        vm_free(gatt_bd_addr);
    }
    if (g_conn_cntx)
    {
        vm_free(g_conn_cntx);
    }
    return;
}


void app_register_server_cb(void *context_handle, VMBOOL status, VMUINT8 app_uuid[16])
{
    vm_log_debug("[TestApp] app_register_server_callback status %d, op_flag %d!\n", status, g_app_cntx.op_flag);
    if(memcmp(app_uuid, g_app_cntx.uid, sizeof(g_app_cntx.uid)) == 0)
    {
        if(g_app_cntx.state == APP_STATUS_ENABLING)
        {
            if(status == 0)
            {
                if(context_handle == NULL)
                    return;

                g_app_cntx.context_handle = context_handle;
                if(g_app_cntx.op_flag == APP_OP_INIT)
                {
                    g_app_cntx.op_flag = APP_OP_ADD_SERVICE;

                    gatt_srv_uuid = (vm_bt_gatt_service_info_t *)vm_malloc(sizeof(vm_bt_gatt_service_info_t));
                    memset(gatt_srv_uuid, 0x0, sizeof(vm_bt_gatt_service_info_t));
                    gatt_srv_uuid->is_primary = TRUE;

                    gatt_srv_uuid->uuid.uuid.length = 16;
                    memcpy(gatt_srv_uuid->uuid.uuid.uuid, &g_srv_uuid, (sizeof(VMUINT8) * 16));
                    vm_bt_gatt_server_add_service(context_handle,gatt_srv_uuid,10);
                }
            }
            else
            {
                g_app_cntx.context_handle = NULL;
                g_app_cntx.op_flag = APP_OP_DEINIT;
                g_app_cntx.state = APP_STATUS_DISABLED;

            }
        }
        else if(g_app_cntx.state == APP_STATUS_DISABLING)
        {
            if(g_app_cntx.op_flag == APP_OP_INIT)
            {
                g_app_cntx.state = APP_STATUS_ENABLING;
                memset(g_app_cntx.uid,0x0,sizeof(g_app_cntx.uid));
                memcpy(g_app_cntx.uid, &g_app_uid, sizeof(g_app_cntx.uid));
                vm_bt_gatt_server_register(g_app_cntx.uid, &app_gatts_cb);
            }
            else
            {
                g_app_cntx.context_handle = NULL;
                g_app_cntx.op_flag = APP_OP_DEINIT;
                g_app_cntx.state = APP_STATUS_DISABLED;
            }
        }

    }
    vm_log_debug("[TestApp] app_register_server_callback end -!\n");
    return;
}

void app_connection_cb(const vm_bt_gatt_connection_t *conn, VMBOOL connected, const vm_bt_gatt_address_t *bd_addr)
{
    vm_log_debug("[TestApp] app_connection_callback connected %d!\n", connected);

    if(memcmp(bd_addr->data, g_conn_cntx->bdaddr, VM_BT_GATT_ADDRESS_SIZE) == 0)
    {
        g_conn_cntx->connection_handle = conn->connection_handle;
        if(connected  && (g_conn_cntx->conn_status != APP_STATUS_CONNECTED))
        {
            vm_log_debug("[TestApp] connect success, but find in list!\n");
            vm_bt_gatt_server_listen(conn->connection_handle, FALSE);
            vm_bt_gatt_server_connect(conn->connection_handle, bd_addr, FALSE);
            g_conn_cntx->conn_status = APP_STATUS_CONNECTED;
        }
        else if(!connected)
        {
            vm_log_debug("[TestApp] connect fail!\n");
            vm_bt_gatt_server_listen(conn->connection_handle, TRUE);

        }
        return;
    }
    if(connected)
    {
        vm_log_debug("[TestApp] connect success, and not find in list!\n");
        if (g_conn_cntx == NULL)
        {
            g_conn_cntx = (AppConnCntx *)vm_malloc(sizeof(AppConnCntx));
        }
        memset(g_conn_cntx,0x0,sizeof(AppConnCntx));
        memcpy(g_conn_cntx->bdaddr, bd_addr->data, VM_BT_GATT_ADDRESS_SIZE);
        g_conn_cntx->connection_handle = conn->connection_handle;
        vm_bt_gatt_server_listen(conn->context_handle, VM_FALSE);
        vm_bt_gatt_server_connect(conn->context_handle, bd_addr, VM_FALSE);
        g_conn_cntx->conn_status = APP_STATUS_CONNECTED;
    }
    return;
}
void app_listen_cb(void *context_handle, VMBOOL status)
{
    vm_log_debug("[TestApp] app_listen_cb status %x!\n", status);
    return;
}
void app_service_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_service_info_t *srvc_id, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{

    vm_log_debug("[TestApp] app_add_services_callback status %d, state %d, op_flag %d!\n",
                     status, g_app_cntx.state, g_app_cntx.op_flag);

    if((g_app_cntx.context_handle == context_handle) && (memcmp(srvc_id, gatt_srv_uuid, sizeof(vm_bt_gatt_service_info_t)) == 0) )
    {
        if(g_app_cntx.state == APP_STATUS_ENABLING)
        {
            if(status == 0)
            {
                if(g_app_cntx.op_flag == APP_OP_ADD_SERVICE)
                {
                    g_app_cntx.op_flag = APP_OP_ADD_CHARACTERISTIC;
                    g_srvc_handle = srvc_handle;
                    gatt_char_uuid = (vm_bt_gatt_attribute_uuid_t *)vm_malloc(sizeof(vm_bt_gatt_attribute_uuid_t));
                    memset(gatt_char_uuid, 0x0, sizeof(vm_bt_gatt_attribute_uuid_t));
                    gatt_char_uuid->uuid.length = 16;
                    memcpy(gatt_char_uuid->uuid.uuid, &g_char_uuid, (sizeof(VMUINT8) * 16));
                    vm_log_debug("[TestApp] app_add_characteristic");
                    vm_bt_gatt_server_add_characteristic(context_handle,
                        srvc_handle,
                        &(gatt_char_uuid->uuid),
                        0XFF,
                        (VM_BT_GATT_PERMISSION_READ |
                        VM_BT_GATT_PERMISSION_READ_ENCRYPTED |
                        VM_BT_GATT_PERMISSION_WRITE |
                        VM_BT_GATT_PERMISSION_WRITE_ENCRYPTED |
                        VM_BT_GATT_PERMISSION_WRITE_ENC_MITM |
                        VM_BT_GATT_PERMISSION_WRITE_SIGNED |
                        VM_BT_GATT_PERMISSION_WRITE_SIGNED_MITM));

                }
            }
            else
            {
                app_deinit();
            }
        }
    }
    vm_log_debug("[TestApp] app_add_services_callback end");
    return;
}

void app_included_service_added_cb(VMBOOL status, void *context_handle,
                VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE incl_srvc_handle)
{
    vm_log_debug("[TestApp] app_included_service_added_cb status %x!\n", status);
    return;
}

void app_characteristic_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_attribute_uuid_t *uuid, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE char_handle)
{
    vm_log_debug("[TestApp] app_characteristic_added_cb status %d, state %d, op_flag %d!\n",
                     status, g_app_cntx.state, g_app_cntx.op_flag);
    if((g_app_cntx.context_handle == context_handle) && (memcmp(uuid, gatt_char_uuid, sizeof(vm_bt_gatt_attribute_uuid_t)) == 0))
    {
        if(g_app_cntx.state == APP_STATUS_ENABLING)
        {
            if(status == 0)
            {
                if((g_app_cntx.op_flag == APP_OP_ADD_CHARACTERISTIC) && (g_srvc_handle == srvc_handle))
                {
                    g_app_cntx.op_flag = APP_OP_START_SERVICE;
                    g_char_handle = char_handle;
                    //srv_gatts_start_service(context_handle, svc_list->handle, GATT_TRANSPORT_LE);
                    vm_log_debug("[TestApp] app_add_start_service -!\n");
                    vm_bt_gatt_server_start_service(context_handle,srvc_handle);
                }
            }
            else
            {
                app_deinit();
            }
        }
    }
    vm_log_debug("[TestApp] app_characteristic_added_cb end -!\n");
    return;
}

void app_descriptor_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_attribute_uuid_t *uuid, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE descr_handle)
{
    vm_log_debug("[TestApp] app_descriptor_added_cb status %x!\n", status);
    return;
}

void app_service_started_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{
    vm_log_debug("[TestApp] app_service_started_cb status %d, state %d, op_flag %d!\n",
                     status, g_app_cntx.state, g_app_cntx.op_flag);

    if((g_app_cntx.context_handle == context_handle) && (g_srvc_handle == srvc_handle ))
    {
        if(g_app_cntx.state == APP_STATUS_ENABLING)
        {
            if(status == 0)
            {
                if(g_app_cntx.op_flag == APP_OP_START_SERVICE)
                {
                    g_app_cntx.state = APP_STATUS_ENABLED;
                    vm_log_debug("[TestApp] app_listen status %x!\n", status);
                    vm_bt_gatt_server_listen(context_handle,VM_TRUE);
                }
            }
            else
            {
                app_deinit();
            }
        }

    }
    vm_log_debug("[TestApp] app_service_started_cb status %x!\n", status);
    return;
}

void app_service_stopped_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{
    vm_log_debug("[TestApp] app_service_stopped_cb status %x!\n", status);
    return;
}

void app_service_deleted_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{
    vm_log_debug("[TestApp] app_service_stopped_cb status %x!\n", status);
    return;
}

VMUINT16 app_convert_array_to_uuid16(vm_bt_uuid_with_length_t *uu)
{
    VMUINT16 uuid = 0;

    if(uu->length == 2)
    {
        uuid = ((VMUINT16)uu->uuid[1]) << 8 | uu->uuid[0];
    }
    else if(uu->length == 16)
    {
        uuid = ((VMUINT16)uu->uuid[13]) << 8 | uu->uuid[12];
    }

    return uuid;
}

void app_request_read_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                                      VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, VMUINT16 offset, VMBOOL is_long)
{
    vm_log_debug("[TestApp] app_request_read_callback attr_handle %x!\n", attr_handle);
    if(g_char_handle == attr_handle)
    {
        VMUINT16 uuid = app_convert_array_to_uuid16(&(gatt_char_uuid->uuid));
        vm_log_debug("[TestApp] app_request_read_callback attr_handle %x, uuid %x!\n", attr_handle, uuid);
        if(uuid == 0x2A07)
        {
            g_tx_power = (AppReq *)vm_malloc(sizeof(AppReq));
            g_tx_power->att_handle = attr_handle;
            memcpy(g_tx_power->bdaddr, bd_addr->data, VM_BT_GATT_ADDRESS_SIZE);
            g_tx_power->gatt_conn.context_handle = conn->context_handle;
            g_tx_power->gatt_conn.connection_handle = conn->connection_handle;
            g_tx_power->trans_id = trans_id;
            g_tx_power->offset = offset;
            vm_log_debug("[TestApp] app_request_read_tx_power attr_handle %x, uuid %x!\n", attr_handle, uuid);
            vm_bt_gatt_server_read_tx_power(conn->context_handle, bd_addr);
            return;
        }
        else if(uuid == 0x2AA0)
        {
            read_att_value.length = 1;
            read_att_value.data[offset] = 0;
            vm_bt_gatt_server_send_response(conn,trans_id,0,attr_handle,&read_att_value);
            vm_log_debug("[TestApp] app_request_read_callback_success attr_handle %x, uuid %x!\n", attr_handle, uuid);
            return;
        }
        read_att_value.length = 1;
        read_att_value.data[offset] = 1;
        vm_bt_gatt_server_send_response(conn, trans_id, 1, attr_handle, &read_att_value);
        vm_log_debug("[TestApp] app_request_read_callback_fail attr_handle %x, uuid %x!\n", attr_handle, uuid);
    }
    vm_log_debug("[TestApp] app_request_read_callback_end -!\n");
    return;

}


void app_request_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                                       VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, vm_bt_gatt_attribute_value_t *value, VMUINT16 offset,
                                       VMBOOL need_rsp, VMBOOL is_prep)
{
    vm_log_debug("[TestApp] app_request_read_callback attr_handle %x!\n", attr_handle);
    if(g_char_handle == attr_handle)
    {
        if((memcmp(bd_addr->data, g_conn_cntx->bdaddr, VM_BT_GATT_ADDRESS_SIZE) == 0) && (need_rsp))
        {
            vm_log_debug("[TestApp] write success find in list!\n");
            vm_bt_gatt_server_send_response(conn, trans_id, 0, attr_handle, value);
            return;
        }
    }
    else
    {
        vm_log_debug("[TestApp] write fail because other type!\n");
        write_att_value.length = 1;
        write_att_value.data[offset] = 1;
        vm_bt_gatt_server_send_response(conn, trans_id, 1, attr_handle, &write_att_value);
    }
    vm_log_debug("[TestApp] app_request_write_callback_end -!\n");
    return;
}


void app_request_exec_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id,
                                            vm_bt_gatt_address_t *bd_addr, VMBOOL cancel)
{
    vm_log_debug("[TestApp] app_request_exec_write_cb cancel %x!\n", cancel);
    return;
}

void app_response_confirmation_cb(VMBOOL status, VM_BT_GATT_ATTRIBUTE_HANDLE handle)
{
    vm_log_debug("[TestApp] app_response_confirmation_callback handle %x!\n", handle);
    return;
}

void app_read_tx_power_cb(void *context_handle, VMBOOL status, vm_bt_gatt_address_t *bd_addr, VMUINT8 tx_power)
{

    vm_log_debug("[TestApp] app_prxr_read_txpower_callback status %d, tx_power %x!\n", status, tx_power);
    if(g_app_cntx.context_handle == context_handle)
    {
        if(memcmp(g_conn_cntx->bdaddr, bd_addr->data, VM_BT_GATT_ADDRESS_SIZE) == 0)
        {
            if(status == 0)
            {
                g_tx_power->att_value.length = 1;
                g_tx_power->att_value.data[g_tx_power->offset] = tx_power;
                vm_bt_gatt_server_send_response(&g_tx_power->gatt_conn, g_tx_power->trans_id, 0, g_tx_power->att_handle, &g_tx_power->att_value);
                vm_log_debug("[TestApp] app_read_tx_power_cb_success tx_power %d!\n", tx_power);

            }
            else
            {
                g_tx_power->att_value.length = 1;
                g_tx_power->att_value.data[g_tx_power->offset] = 1;
                vm_bt_gatt_server_send_response(&g_tx_power->gatt_conn, g_tx_power->trans_id, 1, g_tx_power->att_handle, &g_tx_power->att_value);
                vm_log_debug("[TestApp] app_read_tx_power_cb_fail status %d!\n", status);
            }
        }
    }
    vm_log_debug("[TestApp] app_read_txpower_callback -!\n");
    return;
}




