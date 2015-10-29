/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
 This example demonstrates how to create precise, non-precise and hisr timer. 
 Furthermore it shows how to compute the precise timers every period,
 and how to delete them.

 This example creates a precise timer(vm_timer_create_precise) and then creates a non-precise timer(vm_timer_create_non_precise).
 In the end it creates a hisr timer(vm_timer_create_hisr and vm_timer_set_hisr).

 */
#include "vmtype.h" 
#include "vmsystem.h"
#include "vmlog.h" 
#include "vmtimer.h"
#include "vmdatetime.h"

/* timer id of precise timer */
VM_TIMER_ID_PRECISE g_precise_id = 0;
/* the interval length of precise timer */
#define PRECISE_DELAY_TIME (20)
/* the start time of each period of the precise timer  */
VM_TIME_UST_COUNT g_precise_start_count = 0;
/* the time out for each period of the precise timer */
VM_TIME_UST_COUNT g_precise_stop_count = 0;

/* a non precise timer */
VM_TIMER_ID_NON_PRECISE g_non_precise_id = 0;
/* the interval length of the non precise timer*/
#define NON_PRECISE_DELAY_TIME (1000)
/*  the start time of each period of the non precise timer  */
VM_TIME_UST_COUNT g_non_precise_start_count = 0;
/*  the time out for each period of the non precise timer */
VM_TIME_UST_COUNT g_non_precise_stop_count = 0;

/* the timer id of hisr timer */
VM_TIMER_ID_HISR g_hisr_id = NULL;
/* the interval length of hisr timer */
#define HISR_DELAY_TIME (10)
/* the flag count the number hisr timer time out */
VMINT g_hisr_count = 0;

/* precise timer time out function */
void customer_timer_precise_proc(VM_TIMER_ID_PRECISE timer_id, void* user_data){
    VMUINT32 duration = 0;
    /* get ust count */
    g_precise_stop_count = vm_time_ust_get_count();
    /* compute ust duration between start and time out */
    duration = vm_time_ust_get_duration(g_non_precise_start_count, g_precise_stop_count);
    vm_log_debug("precise proc duration %d stop count %d", duration, g_precise_stop_count);
    g_non_precise_start_count = g_precise_stop_count;
}

/* non precise timer time out function */
void customer_timer_non_precise_proc(VM_TIMER_ID_NON_PRECISE timer_id, void* user_data){
    VMUINT32 duration = 0;
    g_non_precise_stop_count = vm_time_ust_get_count();
    duration = vm_time_ust_get_duration(g_precise_start_count, g_non_precise_stop_count);
    vm_log_debug("non precise proc duration %d stop count %d", duration, g_non_precise_stop_count);
    g_precise_start_count = g_non_precise_stop_count;
}

/* hisr timer time out function */
void customer_timer_hisr_proc(void* user_data){
    g_hisr_count++;
}

/* how to create precise, non precise and hisr timer */
void customer_timer_create_timer(void){
    g_precise_id = vm_timer_create_precise(PRECISE_DELAY_TIME, customer_timer_precise_proc, NULL);
    vm_log_debug("customer timer g_precise_id = %d", g_precise_id);
    g_precise_start_count = vm_time_ust_get_count();
    vm_log_debug("customer timer precise start count %d", g_precise_start_count);
    //use code below to delete precise timer
    //vm_timer_delete_precise(g_precise_id);

    g_non_precise_id = vm_timer_create_non_precise(NON_PRECISE_DELAY_TIME, customer_timer_non_precise_proc, NULL);
    vm_log_debug("customer timer g_non_precise_id = %d", g_non_precise_id);
    g_non_precise_start_count = vm_time_ust_get_count();
    vm_log_debug("customer timer non precise start count %d", g_non_precise_start_count);
    // use code below to delete non precise timer
    //vm_timer_delete_non_precise(g_non_precise_id);


    g_hisr_id = vm_timer_create_hisr("HISR Timer");
    if(g_hisr_id == NULL){
    	vm_log_debug("create hisr fail");
    	return;
    }
    vm_timer_set_hisr(g_hisr_id, customer_timer_hisr_proc, NULL, HISR_DELAY_TIME, HISR_DELAY_TIME);
    //use code below to delete hisr timer
    //vm_timer_delete_hisr(g_hisr_id);
}

/* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param) {
    switch (message) {
        case VM_EVENT_CREATE:
        /* create timer */
        customer_timer_create_timer();
        break;

        case VM_EVENT_QUIT:
        break;
    }
}

/* Entry point */
void vm_main(void){
    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
