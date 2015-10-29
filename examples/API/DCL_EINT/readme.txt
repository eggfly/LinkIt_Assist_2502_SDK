This example shows how to configure EINT and interact with EINT in the callback.

In this example, we will open EINT using vm_dcl_open() and configure it using vm_dcl_control(). We register EINT callback using vm_dcl_register_callback(). The pin of VM_PIN_SIMULATE is used to simulate the source of an external interrupt, and a
timer is used to pull up and down this pin. The FALLING edge VM_PIN_EINT will be formed in every 1.6 seconds, and the counter of g_count is pegged for every FALLING edge until the g_count reaches to 10000 and starts from 0 again. 

The number of g_count will be printed on the Monitor tool.
 

This example runs on LinkIt ONE board and needs to connect VM_PIN_EINT and VM_PIN_SIMULATE with wires. After launching the example, the following AT commands can be issued through the Monitor tool to control the flow:


AT+[1000]Test01: opens EINT and prints "eint count = X", X will increase one by one.


AT+[1000]Test02: closes EINT and stops updating the outputs.
