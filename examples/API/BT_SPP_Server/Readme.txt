This example demonstrates how to setup a Bluetooth SPP server.

It turns on BT service as server, accepts BT device via SPP profile and write/read data to/from remote SPP device. Initially this example starts a 5s timer. After 5s it will try to start BT service with vm_bt_cm_switch_on, set host name with vm_bt_cm_set_host_name, and set visibility with vm_bt_cm_set_visibility. If other SPP device connects to the server, it will accept. After establishing a connection, it reads (vm_bt_spp_read) and writes (vm_bt_spp_write) from/to the device.

Find a BT device with SPP profile, pair with the device and modify the macro BT_NAME to match its name. You can check the result log it will show "Hello SPP!"
