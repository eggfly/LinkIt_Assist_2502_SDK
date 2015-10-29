#ifndef _LINKIT_APP_WIZARDTEMPLATE_
#define	_LINKIT_APP_WIZARDTEMPLATE_

#include "vmbt_gatt.h"

/* status of client application */
typedef enum {
    APPC_STATUS_DISABLED = 0,
    APPC_STATUS_ENABLING,
    APPC_STATUS_ENABLED,
    APPC_STATUS_DISABLING,
}APPC_STATUS;

/* context of client application */
typedef struct {
    void* context_handle;
    APPC_STATUS state;
    VMUINT8 uid[16];
}AppClientCntx;

typedef enum {
    APPC_STATUS_DISCONNECTED = 0,
    APPC_STATUS_CONNECTING,
    APPC_STATUS_CONNECTED,
    APPC_STATUS_DISCONNECTING,
}APP_CLIENT_CONNECTION_STATUS;

typedef struct {
    void* connection_handle;
    APP_CLIENT_CONNECTION_STATUS conn_status;
    VMCHAR bdaddr[VM_BT_GATT_ADDRESS_SIZE];
}AppClientConnCntx;


AppClientConnCntx *g_appc_conn_cntx = NULL;
AppClientCntx  g_appc_cntx = {0};

vm_bt_gatt_client_callback_t g_appc_cb;
vm_bt_gatt_address_t g_app_client_bd_addr_list[10];
vm_bt_gatt_connection_t g_client_gatt_conn;
vm_bt_gatt_address_t* g_client_bd_addr;

VMUINT8 g_appc_uid[] = {
    0xB4, 0x73, 0x1F, 0x49, 0xFF, 0xE5, 0x40, 0x56,
    0X84, 0x5B, 0x6D, 0xF1, 0xF1, 0xB1, 0x6E, 0x9D
};

VMUINT8 g_appc_srv_uuid[] = {
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0X80,
    0x00, 0x10, 0x00, 0x00, 0xFF, 0x18, 0x00, 0x00
};
VMUINT8 g_appc_char_uuid[] = {
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0X80,
    0x00, 0x10, 0x00, 0x00, 0x09, 0x2A, 0x00, 0x10
};

vm_bt_gatt_attribute_value_t g_char_value;

#endif

