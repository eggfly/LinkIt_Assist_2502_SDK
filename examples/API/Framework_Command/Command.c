/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  This example shows how to use AT command capability.

  In this example, we will open the command port first, using vm_cmd_open_port().
  Then the application of this example can receive AT commands and display the
  AT commends in the Monitor tool. The AT command with Test02 uses the
  vm_cmd_close_port() to close the AT command port. After launching the example,
  the AT command can be issued through the Monitor tool as below:
  AT+[1000]Test01: writes an AT command and log it through the Monitor tool.
  AT+[1000]Test02: closes the AT command port.
*/

#include "vmtype.h"
#include "vmsystem.h"
#include "vmcmd.h"
#include "vmlog.h"

#define COMMAND_PORT  1000 /* AT command port */

/* AT command callback, to be called when an AT command is sent from the Monitor tool. */
static void at_callback(vm_cmd_command_t* param, void* user_data)
{
  if(strcmp("Test01", (char*)param->command_buffer) == 0)
  {
    /* Log to the Monitor tool after receiving AT command: AT+[1000]Test01 */
    vm_log_info("cmd = %s", (char*)param->command_buffer);
  }
  else if(strcmp("Test02", (char*)param->command_buffer) == 0)
  {
    /* Closes the AT command port after receiving AT command: AT+[1000]Test02 */
    vm_log_info("close port");
    vm_cmd_close_port(COMMAND_PORT);
  }
}

/* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param) {
  switch (message) {
  case VM_EVENT_CREATE:
    vm_cmd_open_port(COMMAND_PORT, at_callback, NULL);
    break;
  case VM_EVENT_QUIT:
    vm_cmd_close_port(COMMAND_PORT);
    break;
  }
}

/* Entry point */
void vm_main(void) {
  /* Registers system event handler */
  vm_pmng_register_system_event_callback(handle_sysevt);
}

