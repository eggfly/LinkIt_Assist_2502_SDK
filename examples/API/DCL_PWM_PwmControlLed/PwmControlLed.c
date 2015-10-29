/*
  This example code is in public domain.
  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/
/*
  show how to use pwm driver module to control led's bright

  1)config pin mode,use vm_dcl_config_pin_mode(),and PIN2PWM to get the device.
  2)vm_dcl_open() open the device to get pwm handle
  3)vm_dcl_control control the device, use VM_PWM_CMD_START,VM_PWM_CMD_SET_CLOCK to
  prepare the initial state, VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD control the led light

  for LinkIt Assist 2502, connect VM_PIN_P0 to an led,run this app to see the led's bright.
 */
#include "vmtype.h" 
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmlog.h"
#include "vmsystem.h"
#include "vmtimer.h"
#include "vmboard.h"
#include "vmdcl.h"
#include "vmdcl_pwm.h"

VMINT g_tid = 0;
VM_DCL_HANDLE g_pwm_handle;

/*pwm timer callback, use VM_PWM_CMD_START to start pwm,  use VM_PWM_CMD_SET_CLOCK set clock,use VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD control bright*/
void pwm_timer_callback(VM_TIMER_ID_PRECISE timer_id,void * user_data){
    vm_dcl_pwm_config_t config;
    vm_dcl_pwm_set_clock_t clock;
    VMINT ret = -1;
    vm_dcl_pwm_set_counter_threshold_t counter1;
    VMINT i=0;
    VMINT offset = 10;
    VMINT duty = 0;
    vm_log_info("pwm handle=%d",g_pwm_handle);
    if(g_pwm_handle!=-1){
        ret = vm_dcl_control(g_pwm_handle,VM_PWM_CMD_START,0);
        vm_log_info("pwm start ret1 =%d",ret);
        clock.source_clock = 0;
        clock.source_clock_division = 3;
        ret = vm_dcl_control(g_pwm_handle,VM_PWM_CMD_SET_CLOCK,(void *)(&clock));
        vm_log_info("pwm set clock ret =%d",ret);
        for(i=0;i<1022;i++){
            counter1.counter = 1022;
            counter1.threshold = duty;
            ret = vm_dcl_control(g_pwm_handle,VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD,(void *)(&counter1));
            vm_thread_sleep(10);
            vm_log_info("pwm set count ret1 =%d",ret);
            duty += offset;
            if(duty>=1020){
                break;
            }
        }
    }
}

/* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param) {
    switch (message) {
    case VM_EVENT_CREATE:
        /* the message of creation of application */
		/* the graphic operation is not recommended as the response of the message*/
		break;
    case VM_EVENT_PAINT:{   /*for no screen app, code put on VM_EVENT_PAINT is the same as VM_EVENT_CREATE*/
        VM_DCL_PWM_DEVICE dev = VM_DCL_PWM_START;
        g_pwm_handle = -1;
			  /*set pin mode as PWM and get pin's pwm device for the input of vm_dcl_open*/
        vm_dcl_config_pin_mode(VM_PIN_P0,VM_DCL_PIN_MODE_PWM);
        dev = PIN2PWM(VM_PIN_P0);
        g_pwm_handle = vm_dcl_open(dev,0);
        /*create timer to see the led light from dim to bright again and again*/
        g_tid = vm_timer_create_precise(1000, pwm_timer_callback,NULL);
    }
        break;
    case VM_EVENT_QUIT:{
    /* the message of quit of application */
    /* Release all resource */
        vm_timer_delete_precise(g_tid);
        vm_dcl_close(g_pwm_handle);
        g_pwm_handle = -1;
    }
        break;
    }
}
/* Entry point */
void vm_main(void){
	/* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
