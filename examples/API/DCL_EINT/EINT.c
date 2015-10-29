/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  This example shows how to configure EINT and interact with EINT in the callback.

  In this example, we will open EINT using vm_dcl_open() and configure it using
  vm_dcl_control().  We register EINT callback using vm_dcl_register_callback().
  The pin of VM_PIN_SIMULATE is used to simulate the source of an external interrupt, and a
  timer is used to pull up and down this pin. The FALLING edge VM_PIN_EINT will be formed
  in every 1.6 seconds, and the counter of g_count is pegged for every FALLING
  edge until the g_count reaches to 10,000 and starts from 0 again. The number of
  g_count will be printed on the Monitor tool.
  
  This example needs to connect VM_PIN_SIMULATE and VM_PIN_EINT with wires.
  After launching the example, the following AT commands can be issued through
  the Monitor tool to control the flow:
  AT+[1000]Test01: opens EINT and prints "eint count = X", X will increase one by one.
  AT+[1000]Test02: closes EINT and stops updating the outputs.
*/
#include <string.h>
#include "vmtype.h"
#include "vmsystem.h"
#include "vmcmd.h"
#include "vmlog.h"
#include "vmboard.h"
#include "vmdcl.h"
#include "vmdcl_eint.h"
#include "vmdcl_gpio.h"
#include "vmtimer.h"

#if defined(__HDK_LINKIT_ONE_V1__)
#define VM_PIN_SIMULATE  VM_PIN_D7
#define VM_PIN_EINT  VM_PIN_D2
#elif defined(__HDK_LINKIT_ASSIST_2502__)
#define VM_PIN_SIMULATE  VM_PIN_P7
#define VM_PIN_EINT  VM_PIN_P0
#else
#error "Board not support"
#endif


#define COMMAND_PORT  1000 /* AT command port */

static VMUINT g_count = 0;
static VMINT g_flag = 0;

static VM_DCL_HANDLE g_eint_handle = VM_DCL_HANDLE_INVALID;
static VM_DCL_HANDLE g_gpio_handle = VM_DCL_HANDLE_INVALID;

/* The timer to simulate the source of external interrupt. */
static VM_TIMER_ID_PRECISE g_timer_id = 0;

/* EINT callback, to be invoked when EINT triggers. */
static void eint_callback(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
  if(g_count>10000)
    g_count = 0;

    vm_log_info("eint count = %d", ++g_count);
}

/* Attaches EINT */
static void eint_attach(void)
{
    vm_dcl_eint_control_config_t eint_config;
    vm_dcl_eint_control_sensitivity_t sens_data;
    vm_dcl_eint_control_hw_debounce_t deboun_time;
    VM_DCL_STATUS status;
  
    /* Resets the data structures */
    memset(&eint_config,0, sizeof(vm_dcl_eint_control_config_t));
    memset(&sens_data,0, sizeof(vm_dcl_eint_control_sensitivity_t));
    memset(&deboun_time,0, sizeof(vm_dcl_eint_control_hw_debounce_t));

    /* Opens and attaches VM_PIN_EINT EINT */
    g_eint_handle = vm_dcl_open(VM_DCL_EINT, PIN2EINT(VM_PIN_EINT));
  
    if(VM_DCL_HANDLE_INVALID == g_eint_handle)
    {
        vm_log_info("open EINT error");
        return;
    }

    /* Usually, before configuring the EINT, we mask it firstly. */
    status = vm_dcl_control(g_eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL);
    if(status != VM_DCL_STATUS_OK)
    {
      vm_log_info("VM_DCL_EINT_COMMAND_MASK  = %d", status);
    }

    /* Registers the EINT callback */
    status = vm_dcl_register_callback(g_eint_handle, VM_DCL_EINT_EVENT_TRIGGER, (vm_dcl_callback)eint_callback, (void*)NULL );
    if(status != VM_DCL_STATUS_OK)
    {
        vm_log_info("VM_DCL_EINT_EVENT_TRIGGER = %d", status);
    }

    /* Configures a FALLING edge to trigger */
    sens_data.sensitivity = 0;
    eint_config.act_polarity = 0;

    /* Sets the auto unmask for the EINT */
    eint_config.auto_unmask = 1;

    /* Sets the EINT sensitivity */
    status = vm_dcl_control(g_eint_handle, VM_DCL_EINT_COMMAND_SET_SENSITIVITY, (void*)&sens_data);
    if(status != VM_DCL_STATUS_OK)
    {
      vm_log_info("VM_DCL_EINT_COMMAND_SET_SENSITIVITY = %d", status);
    }

    /* Sets debounce time to 1ms */
    deboun_time.debounce_time = 1;
    /* Sets debounce time */
    status = vm_dcl_control(g_eint_handle, VM_DCL_EINT_COMMAND_SET_HW_DEBOUNCE, (void*)&deboun_time);
    if(status != VM_DCL_STATUS_OK)
    {
      vm_log_info("VM_DCL_EINT_COMMAND_SET_HW_DEBOUNCE = %d", status);
    }

    /* Usually, before configuring the EINT, we mask it firstly. */
    status = vm_dcl_control(g_eint_handle, VM_DCL_EINT_COMMAND_MASK, NULL);
    if(status != VM_DCL_STATUS_OK)
    {
       vm_log_info("VM_DCL_EINT_COMMAND_MASK  = %d", status);
    }
    
    /* 1 means enabling the HW debounce; 0 means disabling. */
    eint_config.debounce_enable = 1;

    /* Make sure to call this API at the end as the EINT will be unmasked in this statement. */
    status = vm_dcl_control(g_eint_handle, VM_DCL_EINT_COMMAND_CONFIG, (void*)&eint_config);
    if(status != VM_DCL_STATUS_OK)
    {
      vm_log_info("VM_DCL_EINT_COMMAND_CONFIG = %d", status);
    } 
}

/* The timer callback to simulate the EINT source. */
static void eint_timer_cb(VMINT tid, void* user_data)
{
  if(g_flag)
  {
    vm_dcl_control(g_gpio_handle,VM_DCL_GPIO_COMMAND_WRITE_LOW,0);
  }
  else
  {
    vm_dcl_control(g_gpio_handle,VM_DCL_GPIO_COMMAND_WRITE_HIGH,0);
  }
  g_flag = !g_flag;
}

/* AT command callback, to be invoked when an AT command is sent from the Monitor tool. */
static void at_callback(vm_cmd_command_t* param, void* user_data)
{    
  if(strcmp("Test01", (char*)param->command_buffer) == 0)
  {
    /* Attaches the EINT and creates a timer to trigger it after receiving the AT command: AT+[1000]Test01. */
    vm_dcl_config_pin_mode(VM_PIN_EINT, VM_DCL_PIN_MODE_EINT); /* Sets the pin VM_PIN_EINT to EINT mode */
    vm_dcl_config_pin_mode(VM_PIN_SIMULATE, VM_DCL_PIN_MODE_GPIO); /* Sets the pin VM_PIN_SIMULATE to GPIO mode */
    g_gpio_handle = vm_dcl_open(VM_DCL_GPIO, VM_PIN_SIMULATE);
    g_timer_id = vm_timer_create_precise(800, eint_timer_cb, NULL);
    eint_attach();
  }
  else if(strcmp("Test02", (char*)param->command_buffer) == 0)
  {
	/* Closes the EINT & GPIO, and deletes the timer after receiving AT command: AT+[1000]Test02. */
    if(g_eint_handle!=VM_DCL_HANDLE_INVALID)
    {
      vm_dcl_close(g_eint_handle);
      vm_dcl_close(g_gpio_handle);
      g_eint_handle = VM_DCL_HANDLE_INVALID;
      vm_timer_delete_precise(g_timer_id);
    }
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
