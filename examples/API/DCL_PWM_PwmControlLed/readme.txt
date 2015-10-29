This example shows how to use PWM module to control LED's brightness.

1) Configure pin mode with vm_dcl_config_pin_mode() and PIN2PWM to get the device.
2) Use vm_dcl_open() open the device to get the PWM device handle.
3) Call vm_dcl_control to control the device, use VM_PWM_CMD_START, VM_PWM_CMD_SET_CLOCK to prepare the initial state, and VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD to control brightness of the LED.

To use this example,For LinkIt Assist 2502, connect P0 to an LED. Then upload this app and observe the LED.
