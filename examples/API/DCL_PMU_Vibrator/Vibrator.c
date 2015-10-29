/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  This example show how to control vibrator on/off.

  In this example, we will use vibrator LDO to control vibrator on/off.
  Using vm_dcl_open to open LDO and vm_dcl_control to make LDO enable or disable.
  This example run on  LinkIt Assist 2502 board or others that have vibrator.
  When launching the example, you can use AT command in monitor tool
  to control the flow like below:
  AT+[1000]Test01: vibrator on
  AT+[1000]Test02: vibrator off
*/

#include "vmtype.h"
#include "vmsystem.h"
#include "vmcmd.h"
#include "vmlog.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_pmu.h"

#define COMMAND_PORT  1000 /* AT command port */

static VM_DCL_HANDLE g_pmu_handle;

/* AT command callback, which will be invoked when you send AT command from monitor tool */
static void at_callback(vm_cmd_command_t *param, void *user_data)
{
  vm_dcl_pmu_ld0_buck_enable_t val;

  if(strcmp("Test01",(char*)param->command_buffer) == 0)
  {
    /* enable vibrator LDO when receive AT command: AT+[1000]Test01 */
	/* vibrator will on */
    g_pmu_handle = vm_dcl_open(VM_DCL_PMU, 0);
    val.enable = TRUE;
    val.module = VM_DCL_PMU_VIBR;
    vm_dcl_control(g_pmu_handle, VM_DCL_PMU_CONTROL_LDO_BUCK_SET_ENABLE, (void *)&val);
    vm_dcl_close(g_pmu_handle);

  }
  else if(strcmp("Test02",(char*)param->command_buffer) == 0)
  {
    /* disable vibrator LDO when receive AT command: AT+[1000]Test02 */
	/* vibrator will off */
    g_pmu_handle = vm_dcl_open(VM_DCL_PMU, 0);
    val.enable = FALSE;
    val.module = VM_DCL_PMU_VIBR;
    vm_dcl_control(g_pmu_handle, VM_DCL_PMU_CONTROL_LDO_BUCK_SET_ENABLE, (void *)&val);
    vm_dcl_close(g_pmu_handle);

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
  vm_pmng_register_system_event_callback(handle_sysevt);
}
