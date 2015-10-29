#include "vmchset.h"
#include "vmstdlib.h"
#include "vmfs.h"
#include "vmlog.h"
#include "vmwdt.h"
#include "vmcmd.h"
#include "ResID.h"
#include "DIS.h"
#include "vmbt_gatt.h"
#include "vmlog.h"
#include "vmbt_cm.h"
#include "vmfirmware.h"

/* ---------------------------------------------------------------------------
* global variables
* ------------------------------------------------------------------------ */
/* Define this macro if the application supports running in the background. */
#define        SUPPORT_BG
#define CATCHER_PORT  1000

#define MANUFACTURER_NAME_CHAR_UUID          0x2A29
#define FIRMWARE_REVISION_CHAR_UUID      0x2A26
#define MODEL_NUMBER_CHAR_UUID          0x2A24
#define SERIAL_NUMBER_CHAR_UUID      0x2A25
#define HARDWARE_REVISION_CHAR_UUID          0x2A27




#define CHAR_NUM  5


/* Define status */
typedef enum {
    DIS_STATUS_DISABLED = 0,
    DIS_STATUS_ENABLING,
    DIS_STATUS_ENABLED,
    DIS_STATUS_DISABLING,
}DIS_STATUS;
typedef enum {
    DIS_STATUS_DISCONNECTED = 0,
    DIS_STATUS_CONNECTING,
    DIS_STATUS_CONNECTED,
    DIS_STATUS_DISCONNECTING,
}DIS_CONNECTION_STATUS;

typedef enum {
    DIS_OP_INIT = 0,
    DIS_OP_GET_SERVICE,
    DIS_OP_ADD_SERVICE,
    DIS_OP_DEL_SERVICE,
    DIS_OP_ADD_CHARACTERISTIC,
    DIS_OP_DEL_CHARACTERISTIC,
    DIS_OP_START_SERVICE,
    DIS_OP_STOP_SERVICE,
    DIS_OP_DEINIT,
}DIS_OPERATION;


typedef struct {
    DIS_OPERATION        op_flag;
    DIS_STATUS           state;
    void                 *context_handle;
    VMUINT8        uid[16];
}DisCntx;

typedef struct {
    void                 *connection_handle;
    DIS_CONNECTION_STATUS conn_status;
    VMCHAR                bdaddr[VM_BT_GATT_ADDRESS_SIZE];
}DisConnCntx;


typedef struct {
    vm_bt_gatt_connection_t     gatt_conn;
    vm_bt_gatt_address_t                 bdaddr[VM_BT_GATT_ADDRESS_SIZE];
    VMUINT16                  trans_id;
    VM_BT_GATT_ATTRIBUTE_HANDLE           att_handle;
    VMUINT16                  offset;
    vm_bt_gatt_attribute_value_t             att_value;
}DisReq;

DisCntx  g_dis_cntx = {0,};
vm_bt_gatt_service_info_t   *gatt_srv_uuid = NULL;
vm_bt_gatt_attribute_uuid_t   gatt_char_uuid[CHAR_NUM] = {NULL,NULL,NULL,NULL,NULL};
VM_BT_GATT_ATTRIBUTE_HANDLE g_srvc_handle = 0;
VM_BT_GATT_ATTRIBUTE_HANDLE g_char_handle[CHAR_NUM] = {0,0,0,0,0};
DisReq *g_tx_power = NULL;

static DisConnCntx *g_conn_cntx = NULL;
vm_bt_gatt_attribute_value_t read_att_value;
vm_bt_gatt_attribute_value_t write_att_value;
vm_bt_gatt_connection_t gatt_conn;
vm_bt_gatt_address_t            *gatt_bd_addr;


VMUINT8 g_dis_uid[] = {
    0x18, 0xA0, 0x1F, 0x49, 0xFF, 0xE5, 0x40, 0x56,
    0X84, 0x5B, 0x6D, 0xF1, 0xF1, 0xB1, 0x6E, 0x9D
};
#if 0
VMUINT8 g_srv_uuid[] = {
    0x00, 0x00, 0x18, 0xA0, 0x00, 0x00, 0x10, 0x00,
    0X80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
};

VMUINT8 g_char_uuid[] = {
    0x00, 0x00, 0x2A, 0xA0, 0x00, 0x00, 0x10, 0x00,
    0X80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
};
#endif
VMUINT8 g_disrv_uuid[] = {
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0X80,
    0x00, 0x10, 0x00, 0x00, 0x0A, 0x18, 0x00, 0x00
};
/*Modify for DIS*/
VMUINT8 g_char_uuid[CHAR_NUM][16] = {
    {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0X80,
    0x00, 0x10, 0x00, 0x00, 0x24, 0x2A, 0x00, 0x00},
    {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0X80,
    0x00, 0x10, 0x00, 0x00, 0x25, 0x2A, 0x00, 0x00},
    {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0X80,
    0x00, 0x10, 0x00, 0x00, 0x26, 0x2A, 0x00, 0x00},
    {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0X80,
    0x00, 0x10, 0x00, 0x00, 0x27, 0x2A, 0x00, 0x00},
    {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0X80,
    0x00, 0x10, 0x00, 0x00, 0x29, 0x2A, 0x00, 0x00}
};
/*Modify for DIS*/


vm_bt_gatt_server_callback_t dis_gatts_cb;
#if 0
= {
    app_register_server_cb,
    app_connection_cb,
    app_listen_cb,
    app_service_added_cb,
    app_included_service_added_cb,
    app_characteristic_added_cb,
    app_descriptor_added_cb,
    app_service_started_cb,
    app_service_stopped_cb,
    app_service_deleted_cb,
    app_request_read_cb,
    app_request_write_cb,
    app_request_exec_write_cb,
    app_response_confirmation_cb,
    app_read_tx_power_cb,
};
#endif


/* ---------------------------------------------------------------------------
* FUNCTION DECLARATION
* ------------------------------------------------------------------------ */
void dis_check_bt_on_off(void);
static void dis_bt_init_cb(VMUINT evt,void * param,void * user_data);

//static void dis_init(void);
//static void dis_deinit(void);
static VMUINT16 dis_convert_array_to_uuid16(vm_bt_uuid_with_length_t uu);
static void dis_register_server_cb(void *context_handle, VMBOOL status, VMUINT8 app_uuid[16]);
static void dis_connection_cb(vm_bt_gatt_connection_t *conn, VMBOOL connected, vm_bt_gatt_address_t *bd_addr);
static void dis_listen_cb(void *context_handle, VMBOOL status);
static void dis_service_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_service_info_t *srvc_id, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);
static void dis_included_service_added_cb(VMBOOL status, void *context_handle,
                VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE incl_srvc_handle);
static void dis_characteristic_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_attribute_uuid_t *uuid, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE char_handle);
static void dis_descriptor_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_attribute_uuid_t *uuid, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE descr_handle);
static void dis_service_started_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);
static void dis_service_stopped_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);
static void dis_service_deleted_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);
static void dis_request_read_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                                      VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, VMUINT16 offset, VMBOOL is_long);
static void dis_request_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                                       VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, vm_bt_gatt_attribute_value_t *value, VMUINT16 offset,
                                       VMBOOL need_rsp, VMBOOL is_prep);
static void dis_request_exec_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id,
                                            vm_bt_gatt_address_t *bd_addr, VMBOOL cancel);
static void dis_response_confirmation_cb(VMBOOL status, VM_BT_GATT_ATTRIBUTE_HANDLE handle);
static void dis_read_tx_power_cb(void *context_handle, VMBOOL status, vm_bt_gatt_address_t *bd_addr, VMUINT8 tx_power);

/* ---------------------------------------------------------------------------
* CONTENT
* ------------------------------------------------------------------------ */
void dis_gatts_callback_init(vm_bt_gatt_server_callback_t *srv_gatts_callback)
{
    srv_gatts_callback->characteristic_added = dis_characteristic_added_cb;
    srv_gatts_callback->connection = dis_connection_cb;
    srv_gatts_callback->descriptor_added = dis_descriptor_added_cb;
    srv_gatts_callback->included_service_added = dis_included_service_added_cb;
    srv_gatts_callback->listen = dis_listen_cb;
    srv_gatts_callback->read_tx_power = dis_read_tx_power_cb;
    srv_gatts_callback->register_server = dis_register_server_cb;
    srv_gatts_callback->request_exec_write = dis_request_exec_write_cb;
    srv_gatts_callback->request_read = dis_request_read_cb;
    srv_gatts_callback->request_write = dis_request_write_cb;
    srv_gatts_callback->response_confirmation = dis_response_confirmation_cb;
    srv_gatts_callback->service_added = dis_service_added_cb;
    srv_gatts_callback->service_deleted = dis_service_deleted_cb;
    srv_gatts_callback->service_started = dis_service_started_cb;
    srv_gatts_callback->service_stopped = dis_service_stopped_cb;
    return;
}
void dis_check_bt_on_off(void)
{
    vm_bt_cm_init(dis_bt_init_cb,VM_BT_CM_EVENT_BLE_ACTIVATE,NULL);
    if (VM_BT_CM_POWER_ON == vm_bt_cm_get_power_status())
    {
        dis_init();
    }
    else
    {
        vm_bt_cm_switch_on();
    }

}
void dis_bt_init_cb(VMUINT evt,void * param,void * user_data)
{
    if (VM_BT_CM_EVENT_BLE_ACTIVATE == evt)
    {
        dis_init();
    }
}

void dis_init(void)
{
    vm_log_debug("[DiSrv] dis_init_begain g_dis_cntx.state= %d g_dis_cntx.op_flag = %d",g_dis_cntx.state, g_dis_cntx.op_flag);
    if(g_dis_cntx.state == DIS_STATUS_DISABLED)
    {
        g_dis_cntx.state = DIS_STATUS_ENABLING;
        g_dis_cntx.op_flag = DIS_OP_INIT;
        memset(g_dis_cntx.uid, 0x0, sizeof(g_dis_cntx.uid));
        memcpy(g_dis_cntx.uid, g_dis_uid, sizeof(g_dis_cntx.uid));
        dis_gatts_callback_init(&dis_gatts_cb);
        vm_bt_gatt_server_register(g_dis_cntx.uid, &dis_gatts_cb);
    }
    else if(g_dis_cntx.state == DIS_STATUS_DISABLING)
    {
        if(g_dis_cntx.op_flag == DIS_OP_DEINIT)
        {
            g_dis_cntx.op_flag = DIS_OP_INIT;
        }
        memset(g_dis_cntx.uid,0x0,sizeof(g_dis_cntx.uid));
        memcpy(g_dis_cntx.uid, g_dis_uid, sizeof(g_dis_cntx.uid));
    }
    vm_log_debug("[DiSrv] dis_init_end");
    return;
}

void dis_deinit(void)
{
    vm_log_debug("[DiSrv] dis_deinit state %d, op_flag %d!\n", g_dis_cntx.state, g_dis_cntx.op_flag);
    if((g_dis_cntx.state == DIS_STATUS_DISABLED)
        || (g_dis_cntx.state == DIS_STATUS_DISABLING))
        return;

    vm_bt_gatt_server_listen(g_dis_cntx.context_handle, FALSE);
    if(g_dis_cntx.state == DIS_STATUS_ENABLED)
    {
        gatt_conn.context_handle = g_dis_cntx.context_handle;
        gatt_conn.connection_handle = g_conn_cntx->connection_handle;
        if((g_conn_cntx->conn_status == DIS_STATUS_CONNECTED)
                || (g_conn_cntx->conn_status == DIS_STATUS_CONNECTING))
        {
            gatt_bd_addr = (vm_bt_gatt_address_t *)vm_malloc(sizeof(vm_bt_gatt_address_t));
            memset(gatt_bd_addr, 0x0, sizeof(vm_bt_gatt_address_t));
            memcpy(gatt_bd_addr->data, g_conn_cntx->bdaddr, VM_BT_GATT_ADDRESS_SIZE);
            vm_bt_gatt_server_disconnect(&gatt_conn, gatt_bd_addr);
        }
    }
    vm_bt_gatt_server_stop_service(g_dis_cntx.context_handle, g_srvc_handle);
    vm_bt_gatt_server_delete_service(g_dis_cntx.context_handle, g_srvc_handle);
    vm_bt_gatt_server_deregister(g_dis_cntx.context_handle);
    g_dis_cntx.state = DIS_STATUS_DISABLED;
    g_dis_cntx.op_flag = DIS_OP_DEINIT;
    gatt_conn.context_handle = NULL;
    gatt_conn.connection_handle = NULL;
    if (gatt_srv_uuid)
    {
        vm_free(gatt_srv_uuid);
    }
    /*
    if (gatt_char_uuid)
    {
        vm_free(gatt_char_uuid);
    }
    */
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

void dis_register_server_cb(void *context_handle, VMBOOL status, VMUINT8 app_uuid[16])
{
    vm_log_debug("[DiSrv] dis_register_server_callback status %d, op_flag %d!\n", status, g_dis_cntx.op_flag);
    if(memcmp(app_uuid, g_dis_cntx.uid, sizeof(g_dis_cntx.uid)) == 0)
    {
        if(g_dis_cntx.state == DIS_STATUS_ENABLING)
        {
            if(status == 0)
            {
                if(context_handle == NULL)
                    return;

                g_dis_cntx.context_handle = context_handle;
                if(g_dis_cntx.op_flag == DIS_OP_INIT)
                {
                    g_dis_cntx.op_flag = DIS_OP_ADD_SERVICE;
                    //malloc
                    gatt_srv_uuid = (vm_bt_gatt_service_info_t *)vm_malloc(sizeof(vm_bt_gatt_service_info_t));
                    memset(gatt_srv_uuid, 0x0, sizeof(vm_bt_gatt_service_info_t));
                    gatt_srv_uuid->is_primary = TRUE;
                    //?? why len == 16?
                    gatt_srv_uuid->uuid.uuid.length = 16;
                    memcpy(gatt_srv_uuid->uuid.uuid.uuid, &g_disrv_uuid, (sizeof(VMUINT8) * 16));
                    vm_bt_gatt_server_add_service(context_handle,gatt_srv_uuid,20);
                }
            }
            else
            {
                g_dis_cntx.context_handle = NULL;
                g_dis_cntx.op_flag = DIS_OP_DEINIT;
                g_dis_cntx.state = DIS_STATUS_DISABLED;

            }
        }
        else if(g_dis_cntx.state == DIS_STATUS_DISABLING)
        {
            if(g_dis_cntx.op_flag == DIS_OP_INIT)
            {
                g_dis_cntx.state = DIS_STATUS_ENABLING;
                memset(g_dis_cntx.uid,0x0,sizeof(g_dis_cntx.uid));
                memcpy(g_dis_cntx.uid, g_dis_uid, sizeof(g_dis_cntx.uid));
                vm_bt_gatt_server_register(g_dis_cntx.uid, &dis_gatts_cb);
            }
            else
            {
                g_dis_cntx.context_handle = NULL;
                g_dis_cntx.op_flag = DIS_OP_DEINIT;
                g_dis_cntx.state = DIS_STATUS_DISABLED;
            }
        }

    }
    vm_log_debug("[DiSrv] dis_register_server_callback end -!\n");
    return;
}

void dis_connection_cb(vm_bt_gatt_connection_t *conn, VMBOOL connected, vm_bt_gatt_address_t *bd_addr)
{
    vm_log_debug("[DiSrv] dis_connection_callback connected %d!\n", connected);

    if(memcmp(bd_addr->data, g_conn_cntx->bdaddr, VM_BT_GATT_ADDRESS_SIZE) == 0)
    {
        g_conn_cntx->connection_handle = conn->connection_handle;
        if(connected  && (g_conn_cntx->conn_status != DIS_STATUS_CONNECTED))
        {
            //Do next step to discover all
            vm_log_debug("[DiSrv] connect success, but find in list!\n");
            vm_bt_gatt_server_listen(conn->connection_handle, FALSE);
            vm_bt_gatt_server_connect(conn->connection_handle, bd_addr, FALSE);
            g_conn_cntx->conn_status = DIS_STATUS_CONNECTED;
        }
        else if(!connected)
        {
            vm_log_debug("[DiSrv] connect fail!\n");
            vm_bt_gatt_server_listen(conn->connection_handle, TRUE);
            //vm_free(g_conn_cntx);
        }
        return;
    }
    if(connected)
    {
        vm_log_debug("[DiSrv] connect success, and not find in list!\n");
        //alloc asm
        if (g_conn_cntx == NULL)
        {
            g_conn_cntx = (DisConnCntx *)vm_malloc(sizeof(DisConnCntx));
        }
        memset(g_conn_cntx,0x0,sizeof(DisConnCntx));
        memcpy(g_conn_cntx->bdaddr, bd_addr->data, VM_BT_GATT_ADDRESS_SIZE);
        g_conn_cntx->connection_handle = conn->connection_handle;
        vm_bt_gatt_server_listen(conn->context_handle, VM_FALSE);
        vm_bt_gatt_server_connect(conn->context_handle, bd_addr, VM_FALSE);
        g_conn_cntx->conn_status = DIS_STATUS_CONNECTED;
        //Note the service status
    }
    return;
}
void dis_listen_cb(void *context_handle, VMBOOL status)
{
    vm_log_debug("[DiSrv] dis_listen_cb status %x!\n", status);
    return;
}
void dis_service_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_service_info_t *srvc_id, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{

    vm_log_debug("[DiSrv] dis_add_services_callback status %d, state %d, op_flag %d!\n",
                     status, g_dis_cntx.state, g_dis_cntx.op_flag);

    if((g_dis_cntx.context_handle == context_handle) && (memcmp(srvc_id, gatt_srv_uuid, sizeof(vm_bt_gatt_service_info_t)) == 0) )
    {
        if(g_dis_cntx.state == DIS_STATUS_ENABLING)
        {
            if(status == 0)
            {
                if(g_dis_cntx.op_flag == DIS_OP_ADD_SERVICE)
                {
                    int i;
                    g_dis_cntx.op_flag = DIS_OP_ADD_CHARACTERISTIC;
                    g_srvc_handle = srvc_handle;
                    /*Modify for DIS*/
                    for (i=0;i < CHAR_NUM;i++)
                    {
                        gatt_char_uuid[i].uuid.length = 16;
                        memcpy(gatt_char_uuid[i].uuid.uuid, &g_char_uuid[i], (sizeof(VMUINT8) * 16));
                        //srv_gatts_start_service(context_handle, svc_list->handle, GATT_TRANSPORT_LE);
                        vm_log_debug("[DiSrv] dis_add_characteristic");
                        vm_bt_gatt_server_add_characteristic(context_handle,
                                srvc_handle,
                                &(gatt_char_uuid[i].uuid),
                                VM_BT_GATT_CHAR_PROPERTY_READ,
                                VM_BT_GATT_PERMISSION_READ);
                    }
                    /*Modify for DIS*/
                }
            }
            else
            {
                dis_deinit();
            }
        }
    }
    vm_log_debug("[DiSrv] dis_add_services_callback end");
    return;
}

void dis_included_service_added_cb(VMBOOL status, void *context_handle,
                VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE incl_srvc_handle)
{
    vm_log_debug("[DiSrv] dis_included_service_added_cb status %x!\n", status);
    return;
}

void dis_characteristic_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_attribute_uuid_t *uuid, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE char_handle)
{
    vm_log_debug("[DiSrv] dis_characteristic_added_cb status %d, state %d, op_flag %d!\n",
                     status, g_dis_cntx.state, g_dis_cntx.op_flag);
    if (g_dis_cntx.context_handle == context_handle)
    {
        VMBOOL ret = VM_FALSE;
        int i;
        for (i = 0; i < CHAR_NUM; i++)
        {
            if (memcmp(uuid, &gatt_char_uuid[i], sizeof(vm_bt_gatt_attribute_uuid_t)) == 0)
            {
                g_char_handle[i] = char_handle;
                vm_log_debug("[Disrv] dis_characteristic_added_cb = %d!\n", g_char_handle[i]);
                ret = VM_TRUE;
                break;
            }
        }
        if((g_dis_cntx.state == DIS_STATUS_ENABLING) && (VM_TRUE == ret))
        {
            if(status == 0)
            {
                if((g_dis_cntx.op_flag == DIS_OP_ADD_CHARACTERISTIC) && (g_srvc_handle == srvc_handle))
                {
                    g_dis_cntx.op_flag = DIS_OP_START_SERVICE;
                    //g_char_handle = char_handle;
                    //srv_gatts_start_service(context_handle, svc_list->handle, GATT_TRANSPORT_LE);
                    vm_log_debug("[DiSrv] dis_add_start_service -!\n");
                    vm_bt_gatt_server_start_service(context_handle,srvc_handle);
                }
            }
            else
            {
                dis_deinit();
            }
        }
    }
    vm_log_debug("[DiSrv] dis_characteristic_added_cb end -!\n");
    return;
}

void dis_descriptor_added_cb(VMBOOL status, void *context_handle,
                vm_bt_gatt_attribute_uuid_t *uuid, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE descr_handle)
{
    vm_log_debug("[DiSrv] dis_descriptor_added_cb status %x!\n", status);
    return;
}

void dis_service_started_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{
    vm_log_debug("[DiSrv] dis_service_started_cb status %d, state %d, op_flag %d!\n",
                     status, g_dis_cntx.state, g_dis_cntx.op_flag);

    if((g_dis_cntx.context_handle == context_handle) && (g_srvc_handle == srvc_handle ))
    {
        if(g_dis_cntx.state == DIS_STATUS_ENABLING)
        {
            if(status == 0)
            {
                if(g_dis_cntx.op_flag == DIS_OP_START_SERVICE)
                {
                    g_dis_cntx.state = DIS_STATUS_ENABLED;
                    vm_log_debug("[DiSrv] dis_listen status %x!\n", status);
                    vm_bt_gatt_server_listen(context_handle,VM_TRUE);
                }
            }
            else
            {
                dis_deinit();
            }
        }

    }
    vm_log_debug("[DiSrv] dis_service_started_cb status %x!\n", status);
    return;
}

void dis_service_stopped_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{
    vm_log_debug("[DiSrv] dis_service_stopped_cb status %x!\n", status);
    return;
}

void dis_service_deleted_cb(VMBOOL status, void *context_handle,
                                         VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{
    vm_log_debug("[DiSrv] dis_service_stopped_cb status %x!\n", status);
    return;
}

VMUINT16 dis_convert_array_to_uuid16(vm_bt_uuid_with_length_t uu)
{
    VMUINT16 uuid = 0;

    if(uu.length == 2)
    {
        uuid = ((VMUINT16)uu.uuid[1]) << 8 | uu.uuid[0];
    }
    else if(uu.length == 16)
    {
        uuid = ((VMUINT16)uu.uuid[13]) << 8 | uu.uuid[12];
    }

    return uuid;
}

void dis_request_read_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                                      VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, VMUINT16 offset, VMBOOL is_long)
{
    VMUINT16 uuid;
    int i;
    VMBOOL ret = VM_FALSE;
    vm_log_debug("[DiSrv] dis_request_read_callback attr_handle %x!\n", attr_handle);

    vm_log_debug("g_char_handle[0] = %d,g_char_handle[1] = %d,g_char_handle[2] = %d,g_char_handle[3] = %d,g_char_handle[4] = %d!\n",
        g_char_handle[0],g_char_handle[1],g_char_handle[2],g_char_handle[3],g_char_handle[4]);
    for (i = 0; i < CHAR_NUM; i++)
    {
        if (attr_handle == g_char_handle[i])
        {
            uuid = dis_convert_array_to_uuid16(gatt_char_uuid[i].uuid);
            vm_log_debug("[DiSrv] dis_request_read_callback attr_handle %x, uuid %x!\n", attr_handle, uuid);
            ret = VM_TRUE;
            break;
        }
    }
    if(ret == VM_TRUE)
    {
        vm_log_debug("[TestApp] app_request_read_callback attr_handle %x, uuid %x!\n", attr_handle, uuid);
        if (MODEL_NUMBER_CHAR_UUID == uuid )/*Modul number*/
        {
            int i;
            VMCHAR value[60] = {0};
            //VMUINT valueNum = vm_get_sys_property(MRE_SYS_RELEASE_BRANCH, value, 60);
            VMUINT valueNum = vm_firmware_get_info(value, 60, VM_FIRMWARE_RELEASE_BRANCH);
            read_att_value.length = strlen(value);
            if (read_att_value.length < VM_BT_GATT_ATTRIBUTE_MAX_VALUE_LENGTH)
            {
                for (i = 0; i < read_att_value.length; i++ )
                {
                    read_att_value.data[i] = value[i];
                }
            }
            else
            {
                for (i =0; i < VM_BT_GATT_ATTRIBUTE_MAX_VALUE_LENGTH; i++ )
                {
                    read_att_value.data[i] = value[i];
                }
            }
            vm_bt_gatt_server_send_response(conn,trans_id,0,attr_handle,&read_att_value);
            vm_log_debug("[DiSrv] dis_request_read_callback_success attr_handle %x, uuid %x!\n", attr_handle, uuid);
            return;
        }
        else if (SERIAL_NUMBER_CHAR_UUID == uuid)/*Serial Number*/
        {
            read_att_value.length = 1;
            read_att_value.data[offset] = 0;
            vm_bt_gatt_server_send_response(conn,trans_id,0,attr_handle,&read_att_value);
            vm_log_debug("[DiSrv] dis_request_read_callback_success attr_handle %x, uuid %x!\n", attr_handle, uuid);
            return;
        }
        else if (uuid == FIRMWARE_REVISION_CHAR_UUID)/*Firmware version*/
        {
            int i;
            VMCHAR value[60] = {0};
            //VMUINT valueNum = vm_get_sys_property(MRE_SYS_HOST_VERSION, value, 60);
            VMUINT valueNum = vm_firmware_get_info(value, 60, VM_FIRMWARE_HOST_VERSION);
            read_att_value.length = strlen(value);

            if (read_att_value.length < VM_BT_GATT_ATTRIBUTE_MAX_VALUE_LENGTH)
            {
                for (i =0; i < read_att_value.length; i++ )
                {
                    read_att_value.data[i] = value[i];
                }
            }
            else
            {
                for (i =0; i < VM_BT_GATT_ATTRIBUTE_MAX_VALUE_LENGTH; i++ )
                {
                    read_att_value.data[i] = value[i];
                }
            }
            vm_bt_gatt_server_send_response(conn,trans_id,0,attr_handle,&read_att_value);
            vm_log_debug("[DiSrv] dis_request_read_callback_success attr_handle %x, uuid %x!\n", attr_handle, uuid);
            return;
        }
        else if (uuid == HARDWARE_REVISION_CHAR_UUID)/*Handware version*/
        {
            int i;
            VMCHAR value[60] = {0};
            //VMUINT valueNum = vm_get_sys_property(MRE_SYS_VERSION, value, 60);
            VMUINT valueNum = vm_firmware_get_info(value, 60, VM_FIRMWARE_RESERVED_3);
            read_att_value.length = strlen(value);
            if (read_att_value.length < VM_BT_GATT_ATTRIBUTE_MAX_VALUE_LENGTH)
            {
                for (i =0; i < read_att_value.length; i++ )
                {
                    read_att_value.data[i] = value[i];
                }
            }
            else
            {
                for (i =0; i < VM_BT_GATT_ATTRIBUTE_MAX_VALUE_LENGTH; i++ )
                {
                    read_att_value.data[i] = value[i];
                }
            }

            vm_bt_gatt_server_send_response(conn,trans_id,0,attr_handle,&read_att_value);
            vm_log_debug("[DiSrv] dis_request_read_callback_success attr_handle %x, uuid %x!\n", attr_handle, uuid);
            return;
        }
        else if (uuid == MANUFACTURER_NAME_CHAR_UUID) /*MANUFACTURER NAME*/
        {
            int i;
            VMCHAR value[60] = {0};
            VMCHAR MANU_NAME[] = "Linkit";
            //VMUINT valueNum = vm_get_customer_name(value, 60);
            VMUINT valueNum = vm_firmware_get_info(value, 60, VM_FIRMWARE_RELEASE_BRANCH);
            if (valueNum == -1)
            {
                read_att_value.length = strlen(MANU_NAME);
                memcpy(read_att_value.data, MANU_NAME, strlen(MANU_NAME));
            }
            else
            {
                read_att_value.length = strlen(value);
                if (read_att_value.length < VM_BT_GATT_ATTRIBUTE_MAX_VALUE_LENGTH)
                {
                    for (i =0; i < read_att_value.length; i++ )
                    {
                        read_att_value.data[i] = value[i];
                    }
                }
                else
                {
                    for (i =0; i < VM_BT_GATT_ATTRIBUTE_MAX_VALUE_LENGTH; i++ )
                    {
                        read_att_value.data[i] = value[i];
                    }
                }
            }
            vm_bt_gatt_server_send_response(conn,trans_id,0,attr_handle,&read_att_value);
            vm_log_debug("[DiSrv] dis_request_read_callback_success attr_handle %x, uuid %x!\n", attr_handle, uuid);
            return;
        }
        else
        {
            vm_bt_gatt_server_send_response(conn, trans_id, 1, attr_handle, NULL);
            vm_log_debug("[DiSrv] di_request_read_callback_fail attr_handle %x, uuid %x!\n", attr_handle, uuid);
        }
    }
    vm_log_debug("[DiSrv] dis_request_read_callback_end -!\n");
    return;

}
/*Modify for DIS*/


void dis_request_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                                       VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, vm_bt_gatt_attribute_value_t *value, VMUINT16 offset,
                                       VMBOOL need_rsp, VMBOOL is_prep)
{
    vm_log_debug("[DiSrv] dis_request_read_callback attr_handle %x!\n", attr_handle);
    #if 0 /*Modify for DIS*/
    if(g_char_handle == attr_handle)
    {
        if((memcmp(bd_addr->addr, g_conn_cntx->bdaddr, VM_BT_GATT_ADDRESS_SIZE) == 0) && (need_rsp))
        {
            vm_log_debug("[DiSrv] write success find in list!\n");
            vm_bt_gatt_server_send_response(conn, trans_id, 0, attr_handle, value);
            return;
        }
    }
    else
    {
        vm_log_debug("[DiSrv] write fail because other type!\n");
        write_att_value.len = 1;
        write_att_value.value[offset] = 1;
        vm_bt_gatt_server_send_response(conn, trans_id, VMBOOL_FAILED, attr_handle, &write_att_value);
    }
    vm_log_debug("[DiSrv] dis_request_write_callback_end -!\n");
    #endif  /*Modify for DIS*/
    return;
}


void dis_request_exec_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id,
                                            vm_bt_gatt_address_t *bd_addr, VMBOOL cancel)
{
    vm_log_debug("[DiSrv] dis_request_exec_write_cb cancel %x!\n", cancel);
    return;
}

void dis_response_confirmation_cb(VMBOOL status, VM_BT_GATT_ATTRIBUTE_HANDLE handle)
{
    vm_log_debug("[DiSrv] dis_response_confirmation_callback handle %x!\n", handle);
    return;
}

void dis_read_tx_power_cb(void *context_handle, VMBOOL status, vm_bt_gatt_address_t *bd_addr, VMUINT8 tx_power)
{
    vm_log_debug("[DiSrv] dis_read_txpower_callback -!\n");
    return;
}



