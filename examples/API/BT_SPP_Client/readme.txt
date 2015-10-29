This example demonstrate how to setup a BT SPP client.

It scans for BT devices, connects to BT device via SPP profile and write/read data to/from remote SPP device. This example starts a 5s timer. After 5s it will try to start BT service (vm_bt_cm_switch_on), scan and search for device whose name is BT_NAME (vm_bt_cm_search). If the device is found, start SPP service and connect to the device (vm_bt_spp_connect). Finally, it reads (vm_bt_spp_read) and writes(vm_bt_spp_write) from/to the device.
Find a BT device with SPP profile, enable its BT device and modify the macro BT_NAME to match its name. You can check the result log it will show "Hello SPP!"
