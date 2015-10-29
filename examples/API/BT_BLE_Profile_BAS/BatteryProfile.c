#include "vmstdlib.h"
#include "vmlog.h"
#include "vmfs.h"

#include "vmbt_gatt.h"
#include "vmbt_cm.h"

#include "BAS.h"

/* ---------------------------------------------------------------------------
* global variables
* ------------------------------------------------------------------------ */
typedef enum {
	GATT_STATUS_DISABLED,
	GATT_STATUS_ENABLING,
	GATT_STATUS_ENABLED,
	GATT_STATUS_DISABLING,
}gatt_status_enum;

typedef enum {
	GATT_DISCONNECTED,
	GATT_CONNECTING,
	GATT_CONNECTED,
	GATT_DISCONNECTING,
}gatt_connection_enum;

typedef enum {
	GATT_OP_INIT,
	GATT_OP_GET_SERVICE,
	GATT_OP_ADD_SERVICE,
	GATT_OP_DEL_SERVICE,
    GATT_OP_ADD_CHARACTERISTIC,
    GATT_OP_DEL_CHARACTERISTIC,
    GATT_OP_ADD_DESC,
    GATT_OP_START_SERVICE,
    GATT_OP_STOP_SERVICE,
	GATT_OP_DEINIT
}gatt_operation_enum;

typedef struct {
	void                   *context_handle;
	gatt_operation_enum    gatt_op_flag;
    gatt_status_enum       gatt_status;
	VMUINT8                 uuid[16];
}gatt_cntx_struct;

typedef struct {
	void                   *connection_handle;
	gatt_connection_enum   conn_status;
	VMCHAR                 bdaddr[VM_BT_GATT_ADDRESS_SIZE];
}gatt_conn_cntx_struct;

gatt_cntx_struct  g_gatt_cntx = {0};
static gatt_conn_cntx_struct *g_conn_cntx = NULL;
vm_bt_gatt_service_info_t   *g_battery_srv_uuid_ptr = NULL;
vm_bt_gatt_attribute_uuid_t   *g_battery_char_uuid_ptr = NULL;
VM_BT_GATT_ATTRIBUTE_HANDLE g_battery_srvc_handle = 0;
VM_BT_GATT_ATTRIBUTE_HANDLE g_battery_char_handle = 0;

VMUINT8 g_gatt_uuid[] = {
    0x19, 0xA0, 0x1F, 0x49, 0xFF, 0xE5, 0x40, 0x56,
    0x84, 0x5B, 0x6D, 0xF1, 0xF1, 0xB1, 0x6E, 0x9D
};

#if 0
VMUINT8 g_battery_srv_uuid[] = {
    0x00, 0x00, 0x18, 0x0F, 0x00, 0x00, 0x10, 0x00,
    0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
};

VMUINT8 g_battery_char_uuid[] = {
    0x00, 0x00, 0x2A, 0x19, 0x00, 0x00, 0x10, 0x00,
    0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
};
#endif
VMUINT8 g_battery_srv_uuid[] = {
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0x0F, 0x18, 0x00, 0x00
};

VMUINT8 g_battery_char_uuid[] = {
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0x19, 0x2A, 0x00, 0x00
};

static void gatt_start(void);
static void btcm_cb(VMUINT evt, void* param, void* user_data);

static void gatts_register_server_cb(void *context_handle, VMBOOL status, VMUINT8 app_uuid[16]);
static void gatts_connect_cb(vm_bt_gatt_connection_t *conn, VMBOOL connected, vm_bt_gatt_address_t *bd_addr);
static void gatts_listen_cb(void *context_handle, VMBOOL status);
static void gatts_add_service_cb(VMBOOL status, void *context_handle, vm_bt_gatt_service_info_t *srvc_id,
                                 VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);
static void gatts_add_included_service_cb(VMBOOL status, void *context_handle, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle,
                                          VM_BT_GATT_ATTRIBUTE_HANDLE incl_srvc_handle);

static void gatts_add_characteristic_cb(VMBOOL status, void *context_handle, vm_bt_gatt_attribute_uuid_t *uuid,
                                        VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE char_handle);
static void gatts_add_descriptor_cb(VMBOOL status, void *context_handle, vm_bt_gatt_attribute_uuid_t *uuid,
                                    VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE descr_handle);

static void gatts_start_service_cb(VMBOOL status, void *context_handle, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);

static void gatts_stop_service_cb(VMBOOL status, void *context_handle, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);

static void gatts_delete_service_cb(VMBOOL status, void *context_handle, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle);

static void gatts_request_read_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                                  VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, VMUINT16 offset, VMBOOL is_long);

static void gatts_request_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                                   VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, vm_bt_gatt_attribute_value_t *value, VMUINT16 offset,
                                   VMBOOL need_rsp, VMBOOL is_prep);

static void gatts_request_exec_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id,
                                        vm_bt_gatt_address_t *bd_addr, VMBOOL cancel);

static void gatts_response_confirmation_cb(VMBOOL status, VM_BT_GATT_ATTRIBUTE_HANDLE handle);

vm_bt_gatt_server_callback_t g_gatts_cb;

void gatts_callback_init(vm_bt_gatt_server_callback_t *gatts_cb)
{
    vm_log_info("[Battery GATT] gatts_callback_init()");

    gatts_cb->register_server = gatts_register_server_cb;
    gatts_cb->connection = gatts_connect_cb;
    gatts_cb->listen = gatts_listen_cb;
    gatts_cb->service_added = gatts_add_service_cb;
    gatts_cb->included_service_added = gatts_add_included_service_cb;
    gatts_cb->characteristic_added = gatts_add_characteristic_cb;
    gatts_cb->descriptor_added = gatts_add_descriptor_cb;
    gatts_cb->service_started = gatts_start_service_cb;
    gatts_cb->service_stopped = gatts_stop_service_cb;
    gatts_cb->service_deleted = gatts_delete_service_cb;
    gatts_cb->request_read = gatts_request_read_cb;
    gatts_cb->request_write = gatts_request_write_cb;
    gatts_cb->request_exec_write = gatts_request_exec_write_cb;
    gatts_cb->response_confirmation = gatts_response_confirmation_cb;
    gatts_cb->read_tx_power = NULL;
}

void gatt_init(void)
{
    VM_BT_CM_POWER_STATUS bt_power_status;
    VMINT hdl;

    vm_log_info("[Battery GATT] gatt_init()");

    bt_power_status = vm_bt_cm_get_power_status();
    vm_log_info("[Battery GATT] bt_power_status:%d", bt_power_status);
    if (VM_BT_CM_POWER_ON != bt_power_status)
    {
        // BT is not on
        hdl = vm_bt_cm_init(btcm_cb, VM_BT_CM_EVENT_BLE_ACTIVATE, NULL);
        if (VM_BT_CM_POWER_ON == vm_bt_cm_get_power_status())
        {
            vm_log_debug("[Battery GATT] VM_BT_CM_POWER_ON already");
            gatt_start();
        }
        else
        {
            vm_log_debug("[Battery GATT] to vm_bt_cm_switch_on");
            vm_bt_cm_switch_on();
        }
    }
    else
    {
        // BT is on
        vm_log_debug("[Battery GATT] gatt_start");
        gatt_start();
    }
}

void gatt_start(void)
{
    vm_log_debug("[Battery GATT] gatt_start(), gatt_status:%d", g_gatt_cntx.gatt_status);

    if (g_gatt_cntx.gatt_status == GATT_STATUS_DISABLED)
	{
		g_gatt_cntx.gatt_status = GATT_STATUS_ENABLING;
        g_gatt_cntx.gatt_op_flag = GATT_OP_INIT;
        memset(g_gatt_cntx.uuid, 0x0, sizeof(g_gatt_cntx.uuid));
	    memcpy(g_gatt_cntx.uuid, g_gatt_uuid, sizeof(g_gatt_cntx.uuid));
	    gatts_callback_init(&g_gatts_cb);
	    vm_bt_gatt_server_register(&g_gatt_cntx.uuid, &g_gatts_cb);
    }
	else if (g_gatt_cntx.gatt_status == GATT_STATUS_DISABLING)
	{
		if(g_gatt_cntx.gatt_op_flag == GATT_OP_DEINIT)
		{
			g_gatt_cntx.gatt_op_flag = GATT_OP_INIT;
		}
        memset(g_gatt_cntx.uuid, 0x0, sizeof(g_gatt_cntx.uuid));
		memcpy(g_gatt_cntx.uuid, g_gatt_uuid, sizeof(g_gatt_cntx.uuid));
	}
}

void btcm_cb(VMUINT evt, void* param, void* user_data)
{
    vm_log_info("[Battery GATT] btcm_cb(), evt:%d", evt);

    switch (evt)
    {
        case VM_BT_CM_EVENT_BLE_ACTIVATE:
        {
            // BT is turned on
            gatt_start();
            break;
        }
        /*
        case VM_BT_CM_POWER_OFF:
        {
            // BT is turned off
            gatt_deinit();
            break;
        }
        */
        default:
            break;
    }
}

void gatts_register_server_cb(void *context_handle, VMBOOL status, VMUINT8 app_uuid[16])
{
    vm_log_info("[Battery GATT] gatts_register_server_cb(), status: %d", status);

	if (memcmp(app_uuid, g_gatt_cntx.uuid, sizeof(g_gatt_cntx.uuid)) == 0)
	{
        if (g_gatt_cntx.gatt_status == GATT_STATUS_ENABLING)
		{
		    if (status == 0)
		    {
                if (context_handle == NULL)
                    return;

		        g_gatt_cntx.context_handle = context_handle;
			    if (g_gatt_cntx.gatt_op_flag == GATT_OP_INIT)
			    {
				    g_gatt_cntx.gatt_op_flag = GATT_OP_ADD_SERVICE;
                    g_battery_srv_uuid_ptr = (vm_bt_gatt_service_info_t *)vm_malloc(sizeof(vm_bt_gatt_service_info_t));
                    vm_log_info("[Battery GATT] g_battery_srv_uuid_ptr:%x", g_battery_srv_uuid_ptr);
				    memset(g_battery_srv_uuid_ptr, 0x0, sizeof(vm_bt_gatt_service_info_t));
                    g_battery_srv_uuid_ptr->is_primary = 1;
                    g_battery_srv_uuid_ptr->uuid.uuid.length = 16;
                    memcpy(g_battery_srv_uuid_ptr->uuid.uuid.uuid, &g_battery_srv_uuid, (sizeof(VMUINT8) * 16));
                    vm_log_info("[Battery GATT] add service");
				    vm_bt_gatt_server_add_service(context_handle, g_battery_srv_uuid_ptr, 10);
			    }
		    }
		    else
		    {
                g_gatt_cntx.context_handle = NULL;
			    g_gatt_cntx.gatt_op_flag = GATT_OP_DEINIT;
                g_gatt_cntx.gatt_status = GATT_STATUS_DISABLED;
		    }
        }
		else if (g_gatt_cntx.gatt_status == GATT_STATUS_DISABLING)
		{
			if (g_gatt_cntx.gatt_op_flag == GATT_OP_INIT)
			{
				g_gatt_cntx.gatt_status = GATT_STATUS_ENABLING;
                memset(g_gatt_cntx.uuid, 0x0, sizeof(g_gatt_cntx.uuid));
				memcpy(g_gatt_cntx.uuid, g_gatt_uuid, sizeof(g_gatt_cntx.uuid));
				vm_bt_gatt_server_register(&g_gatt_cntx.uuid, &g_gatts_cb);
			}
			else
			{
				g_gatt_cntx.context_handle = NULL;
				g_gatt_cntx.gatt_op_flag = GATT_OP_DEINIT;
				g_gatt_cntx.gatt_status = GATT_STATUS_DISABLED;
			}
		}
	}
}

void gatts_add_service_cb(VMBOOL status, void *context_handle, vm_bt_gatt_service_info_t *srvc_id,
                          VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{
    vm_log_info("[Battery GATT] gatts_add_service_cb(), status: %d, context_handle:%x, srvc_id:%x, srvc_handle:%d",
        status, context_handle, srvc_id, srvc_handle);

    vm_log_info("[Battery GATT] g_gatt_cntx.context_handle:%x, g_battery_srv_uuid_ptr:%x", g_gatt_cntx.context_handle, g_battery_srv_uuid_ptr);

    if ((g_gatt_cntx.context_handle == context_handle) && (memcmp(srvc_id, g_battery_srv_uuid_ptr, sizeof(vm_bt_gatt_service_info_t)) == 0))
	{
        vm_log_info("[Battery GATT] context_handle and srvc_id matched, gatt_status:%d", g_gatt_cntx.gatt_status);
		if (g_gatt_cntx.gatt_status == GATT_STATUS_ENABLING)
		{
			if (status == 0)
			{
                vm_log_info("[Battery GATT] gatt_op_flag:%d", g_gatt_cntx.gatt_op_flag);
				if(g_gatt_cntx.gatt_op_flag == GATT_OP_ADD_SERVICE)
				{
					g_gatt_cntx.gatt_op_flag = GATT_OP_ADD_CHARACTERISTIC;
					g_battery_srvc_handle = srvc_handle;
					g_battery_char_uuid_ptr = (vm_bt_gatt_attribute_uuid_t *)vm_malloc(sizeof(vm_bt_gatt_attribute_uuid_t));
                    vm_log_info("[Battery GATT] g_battery_char_uuid_ptr:%x", g_battery_char_uuid_ptr);
					memset(g_battery_char_uuid_ptr, 0x0, sizeof(vm_bt_gatt_attribute_uuid_t));
					g_battery_char_uuid_ptr->uuid.length = 16;
					memcpy(g_battery_char_uuid_ptr->uuid.uuid, &g_battery_char_uuid, (sizeof(VMUINT8) * 16));
                    vm_log_info("[Battery GATT] add characteristic");
					vm_bt_gatt_server_add_characteristic(context_handle, srvc_handle, &(g_battery_char_uuid_ptr->uuid),
                        VM_BT_GATT_CHAR_PROPERTY_READ, VM_BT_GATT_PERMISSION_READ | VM_BT_GATT_PERMISSION_READ_ENCRYPTED | VM_BT_GATT_PERMISSION_READ_ENC_MITM);
				}
			}
			else
			{
				gatt_deinit();
			}
		}
	}
}

void gatts_add_included_service_cb(VMBOOL status, void *context_handle, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle,
                                   VM_BT_GATT_ATTRIBUTE_HANDLE incl_srvc_handle)
{
    vm_log_info("[Battery GATT] gatts_add_included_service_cb(), status: %d", status);
}

void gatts_add_characteristic_cb(VMBOOL status, void *context_handle, vm_bt_gatt_attribute_uuid_t *uuid,
                                 VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE char_handle)
{
    vm_log_info("[Battery GATT] gatts_add_characteristic_cb(), status: %d", status);

    if ((g_gatt_cntx.context_handle == context_handle) && (memcmp(uuid, g_battery_char_uuid_ptr, sizeof(vm_bt_gatt_attribute_uuid_t)) == 0))
	{
		if (g_gatt_cntx.gatt_status == GATT_STATUS_ENABLING)
		{
			if (status == 0)
			{
				if((g_gatt_cntx.gatt_op_flag == GATT_OP_ADD_CHARACTERISTIC) && (g_battery_srvc_handle == srvc_handle))
				{
                    g_gatt_cntx.gatt_op_flag = GATT_OP_START_SERVICE;
				    g_battery_char_handle = char_handle;
                    vm_log_info("[Battery GATT] start service");
					vm_bt_gatt_server_start_service(context_handle, srvc_handle);
				}
			}
			else
			{
				gatt_deinit();
			}
		}
	}
}

void gatts_add_descriptor_cb(VMBOOL status, void *context_handle, vm_bt_gatt_attribute_uuid_t *uuid,
                             VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle, VM_BT_GATT_ATTRIBUTE_HANDLE descr_handle)
{
    vm_log_info("[Battery GATT] gatts_add_descriptor_cb(), status: %d", status);
}

void gatts_start_service_cb(VMBOOL status, void *context_handle, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{
    vm_log_info("[Battery GATT] gatts_start_service_cb(), status: %d", status);
    vm_log_info("[Battery GATT] gatt_status: %d, gatt_op_flag:%d", g_gatt_cntx.gatt_status, g_gatt_cntx.gatt_op_flag);

    if ((g_gatt_cntx.context_handle == context_handle) && (g_battery_srvc_handle == srvc_handle))
	{
		if (g_gatt_cntx.gatt_status == GATT_STATUS_ENABLING)
		{
			if (status == 0)
			{
				if (g_gatt_cntx.gatt_op_flag == GATT_OP_START_SERVICE)
				{
					g_gatt_cntx.gatt_status = GATT_STATUS_ENABLED;
					vm_bt_gatt_server_listen(context_handle, VM_TRUE);
				}
			}
			else
			{
				gatt_deinit();
			}
		}

	}
}

void gatts_listen_cb(void *context_handle, VMBOOL status)
{
    vm_log_info("[Battery GATT] gatts_listen_cb(), status: %d", status);
}

void gatts_connect_cb(vm_bt_gatt_connection_t *conn, VMBOOL connected, vm_bt_gatt_address_t *bd_addr)
{
    vm_log_info("[Battery GATT] gatts_connect_cb(), connected: %d, conn_status:%d", connected, g_conn_cntx->conn_status);

    if (memcmp(bd_addr->data, g_conn_cntx->bdaddr, VM_BT_GATT_ADDRESS_SIZE) == 0)
	{
	    vm_log_info("[Battery GATT] bdaddr matched!\n");
		g_conn_cntx->connection_handle = conn->connection_handle;
		if (connected  && (g_conn_cntx->conn_status != GATT_CONNECTED))
		{
			//Do next step to discovery all devices
			vm_bt_gatt_server_listen(conn->connection_handle, VM_FALSE);
			//vm_bt_gatt_server_connect(conn->connection_handle, bd_addr, VM_FALSE);
			g_conn_cntx->conn_status = GATT_CONNECTED;
		}
		else if (!connected)
		{
			vm_bt_gatt_server_listen(conn->connection_handle, VM_TRUE);
			//vm_free(g_conn_cntx);
		}
		return;
	}

	if (connected)
	{
		vm_log_info("[Battery GATT] not find in list!\n");
		//alloc asm
        if (g_conn_cntx == NULL)
        {
		    g_conn_cntx = (gatt_conn_cntx_struct *)vm_malloc(sizeof(gatt_conn_cntx_struct));
            vm_log_info("[Battery GATT] g_conn_cntx:%x\n", g_conn_cntx);
        }
        memset(g_conn_cntx, 0x0, sizeof(gatt_conn_cntx_struct));
        memcpy(g_conn_cntx->bdaddr, bd_addr->data, VM_BT_GATT_ADDRESS_SIZE);
		g_conn_cntx->connection_handle = conn->connection_handle;
		vm_bt_gatt_server_listen(conn->context_handle, VM_FALSE);
		//vm_bt_gatt_server_connect(conn->context_handle, bd_addr, VM_FALSE);
		g_conn_cntx->conn_status = GATT_CONNECTED;
		//Note the service status
	}
}

VMUINT16 gatt_convert_uuid_to_u16(vm_bt_uuid_with_length_t uu)
{
	VMUINT16 uuid = 0;

	if (uu.length == 2)
	{
		uuid = ((VMUINT16)uu.uuid[1]) << 8 | uu.uuid[0];
	}
	else if (uu.length == 16)
	{
		uuid = ((VMUINT16)uu.uuid[13]) << 8 | uu.uuid[12];
	}

	return uuid;
}


static void gatts_read_battery_response(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, VMUINT16 offset)
{
    vm_bt_gatt_attribute_value_t att_value;
    VMINT battery_level = 0;

    vm_log_info("[Battery GATT] gatts_read_battery_response()");

    battery_level = vm_pwr_get_battery_level();
    vm_log_info("[Battery GATT] battery_level:%d", battery_level);
    memcpy(&(att_value.data[offset]), &battery_level, sizeof(battery_level));
    att_value.length = sizeof(battery_level);

    vm_bt_gatt_server_send_response(conn, trans_id, 0, attr_handle, &att_value);
}

void gatts_request_read_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                           VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, VMUINT16 offset, VMBOOL is_long)
{
    vm_log_info("[Battery GATT] gatts_request_read_cb(), trans_id: %d, offset: %d", trans_id, offset);

    if (g_battery_char_handle == attr_handle)
	{
        // TODO: check bdaddr?
		VMUINT16 uuid = gatt_convert_uuid_to_u16(g_battery_char_uuid_ptr->uuid);
		vm_log_info("[Battery GATT] attr_handle %d, uuid %d!\n", attr_handle, uuid);
		if (uuid == 0x2A19)
		{
            gatts_read_battery_response(conn, trans_id, attr_handle, offset);
		}
	}
    else
    {
        // TODO
    }
}

void gatts_request_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id, vm_bt_gatt_address_t *bd_addr,
                            VM_BT_GATT_ATTRIBUTE_HANDLE attr_handle, vm_bt_gatt_attribute_value_t *value, VMUINT16 offset,
                            VMBOOL need_rsp, VMBOOL is_prep)
{
    vm_bt_gatt_attribute_value_t att_value;

    vm_log_info("[Battery GATT] gatts_request_write_cb(), offset:%d, need_rsp: %d", offset, need_rsp);

    if (g_battery_char_handle == attr_handle)
	{
        vm_log_info("[Battery GATT] character handle matched");
		if (memcmp(bd_addr->data, g_conn_cntx->bdaddr, VM_BT_GATT_ADDRESS_SIZE) == 0)
		{
            vm_log_info("[Battery GATT] find in list! len:%d, value0:%d, value1:%d, value2:%d",
                value->length, value->data[0], value->data[1], value->data[2]);
		}
	}
	else
	{
        // TODO
		vm_log_info("[Battery GATT] character handle not matched");
	}
}

void gatts_request_exec_write_cb(vm_bt_gatt_connection_t *conn, VMUINT16 trans_id,
                                 vm_bt_gatt_address_t *bd_addr, VMBOOL cancel)
{
    vm_log_info("[Battery GATT] gatts_request_exec_write_cb(), trans_id: %d", trans_id);
}

void gatts_response_confirmation_cb(VMBOOL status, VM_BT_GATT_ATTRIBUTE_HANDLE handle)
{
    vm_log_info("[Battery GATT] gatts_response_confirmation_cb(), status: %d", status);
}

void gatt_deinit(void)
{
    vm_bt_gatt_connection_t gatt_conn;
	vm_bt_gatt_address_t bd_addr;

    vm_log_info("[Battery GATT] gatt_deinit(), gatt_status:%d", g_gatt_cntx.gatt_status);

    if(g_gatt_cntx.gatt_status == GATT_STATUS_DISABLED || g_gatt_cntx.gatt_status == GATT_STATUS_DISABLING)
		return;

	vm_bt_gatt_server_listen(g_gatt_cntx.context_handle, VM_FALSE);
	if(g_gatt_cntx.gatt_status == GATT_STATUS_ENABLED)
	{
		gatt_conn.context_handle = g_gatt_cntx.context_handle;
		gatt_conn.connection_handle = g_conn_cntx->connection_handle;
		if(g_conn_cntx->conn_status == GATT_CONNECTED || g_conn_cntx->conn_status == GATT_CONNECTING)
		{
            memcpy(bd_addr.data, g_conn_cntx->bdaddr, VM_BT_GATT_ADDRESS_SIZE);
			vm_bt_gatt_server_disconnect(&gatt_conn, &bd_addr);
		}
	}

    vm_bt_gatt_server_stop_service(g_gatt_cntx.context_handle, g_battery_srvc_handle);
	vm_bt_gatt_server_delete_service(g_gatt_cntx.context_handle, g_battery_srvc_handle);
	vm_bt_gatt_server_deregister(g_gatt_cntx.context_handle);
	g_gatt_cntx.gatt_status = GATT_STATUS_DISABLED;
	g_gatt_cntx.gatt_op_flag = GATT_OP_DEINIT;
    gatt_conn.context_handle = NULL;
	gatt_conn.connection_handle = NULL;
    if (g_battery_srv_uuid_ptr)
    {
        vm_free(g_battery_srv_uuid_ptr);
        g_battery_srv_uuid_ptr = NULL;
    }
    if (g_battery_char_uuid_ptr)
    {
	    vm_free(g_battery_char_uuid_ptr);
        g_battery_char_uuid_ptr = NULL;
    }
    if (g_conn_cntx)
    {
        vm_free(g_conn_cntx);
        g_conn_cntx = NULL;
    }
}

void gatts_stop_service_cb(VMBOOL status, void *context_handle, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{
    vm_log_info("[Battery GATT] gatts_stop_service_cb(), status: %d", status);
}

void gatts_delete_service_cb(VMBOOL status, void *context_handle, VM_BT_GATT_ATTRIBUTE_HANDLE srvc_handle)
{
    vm_log_info("[Battery GATT] gatts_delete_service_cb(), status: %d", status);
}
