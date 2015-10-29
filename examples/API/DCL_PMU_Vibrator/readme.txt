This example shows how to control vibrator on/off on LinkIt Assist 2502.

In this example, we will use vibrator LDO to control vibrator on/off. Use vm_dcl_open to open LDO and vm_dcl_control to make LDO enable or disable. This example run on LinkIt Assist 2502 board or others that have vibrator. LinkIt ONE does not have a vibrator.

When launching the example, you can send command in monitor tool to control the flow like below:

AT+[1000]Test01: vibrator on
AT+[1000]Test02: vibrator off
