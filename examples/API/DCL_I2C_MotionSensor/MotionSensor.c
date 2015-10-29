/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  show how to use I2C driver module control bmi055 motion sensor

  initialize motion sensor,and then create a timer to get x,y,z data from sensor.
  this data will be output by Catch log.i2c operator in file MotionBmi055Qg.c,call vm_dcl_open() to open I2C,call
  vm_dcl_control()to control I2C, command VM_DCL_I2C_CMD_CONFIG is to config I2C's
  slave address,transition mode,mode speed.use VM_DCL_I2C_CMD_SINGLE_WRITE,
  VM_DCL_I2C_CMD_WRITE_AND_READ to communicate with bmi055.

  use Aster Watch board(for this board support bmi055 motion sensor)

*/
#include "vmtype.h" 
#include "vmfs.h"
#include "vmlog.h"
#include "vmsystem.h"
#include "vmtimer.h"
#include "MotionMsgID.h"
#include "MotionSensor.h"
#include "MotionBmi055Qg.h"

#define MAX_USER 1
/*Gusture porting start*/
#define PI     (3.1415926)
#ifndef abs
#define abs(a)   (((a) < 0) ? -(a) : (a))
#endif
#define INT_NEGATIVE_RIGHT_SHIFT(val32, num)  {\
	val32 = -val32;\
	val32 = (VMINT32)(((VMUINT32)val32) >> num);\
	val32 = -val32;\
}

#define INT_POSITIVE_RIGHT_SHIFT(val32, num)  {\
	val32 = (VMINT32)(((VMUINT32)val32) >> num);\
}

#define MOTION_SENSOR_ABS(a,b) (a>=b?a-b:b-a)

motion_data_param acc_data;
motion_data_param acc_raw_data;


VMUINT32 user_count;
VMUINT32 enable_count;

VMINT16 old_x = 0, old_y = 0, old_z = 0;
VMINT16 new_x = -10000, new_y = -10000, new_z = -10000;
/*Gusture porting end*/

VMINT32 motion_x_cali_milli_acc, motion_y_cali_milli_acc, motion_z_cali_milli_acc;
VMINT16 pre_x_milli_acc, pre_y_milli_acc, pre_z_milli_acc;
VMINT16 motion_cali_count;
static motion_cali_struct motion_sensor_cali_data;
static motion_data_param motion_sensor_cali_milli_acc_data={0, 0, 0};
static motion_custom_data_struct *motion_sensor_custom_dp;
VM_TIMER_ID_PRECISE g_ptimer_id = 0;

/*
* FUNCTION                                                            
*	motion_sensor_get_acc
*
* DESCRIPTION                                                           
*   	This function is to get acceleration
*
* CALLS  
*
* PARAMETERS
*	acc_data: acceleration data pointer
*  x_adc: ADC value in X-axis
*  y_adc: ADC value in Y-axis
*  z_adc: ADC value in Z-axis
*	
* RETURNS
*	None
*/

VMINT16 motion_sensor_get_acc_func(VMUINT16 adc, VMINT16 offset, VMINT16 gain){
    VMINT32 val32;
    val32 = MOTION_SENSOR_ABS(adc, offset)*gain;
    if (val32 < 0){
        INT_NEGATIVE_RIGHT_SHIFT(val32, 10);
    }
    else{
        INT_POSITIVE_RIGHT_SHIFT(val32, 10);
    }
    if(adc>=offset)
        return (VMINT16)val32;
    else
        return (VMINT16)-val32;
}

void motion_sensor_get_acc(motion_data_param *acc_data,
                           VMUINT16 x_adc, VMUINT16 y_adc, VMUINT16 z_adc){  
    acc_data->acc_data_x = motion_sensor_get_acc_func(x_adc, motion_sensor_cali_data.x_offset, motion_sensor_cali_data.x_gain);
    acc_data->acc_data_y = motion_sensor_get_acc_func(y_adc, motion_sensor_cali_data.y_offset, motion_sensor_cali_data.y_gain);
    acc_data->acc_data_z = motion_sensor_get_acc_func(z_adc, motion_sensor_cali_data.z_offset, motion_sensor_cali_data.z_gain);
}

VMINT motion_get_calibration(motion_data_param *acc_cali_data){
    VM_FS_HANDLE f_handle = -1;
    VMUINT nread = 0;
    VMWCHAR szwFilePath[128] = {0};
    vm_log_info("motion_get_calibration()");
    vm_chset_ascii_to_ucs2(szwFilePath, 128, "C:\\@BTMre\\installed\\MOTION_SENSOR.dat");
    f_handle = vm_fs_open((VMWSTR)szwFilePath, VM_FS_MODE_READ, FALSE);
    if (f_handle < 0){
        vm_fs_close(f_handle);
        return VM_FALSE;
    }
    if (vm_fs_read(f_handle, acc_cali_data, sizeof(motion_data_param), &nread) == 0){
        vm_fs_close(f_handle);
        return VM_FALSE;
    }
    vm_fs_close(f_handle);
    return VM_TRUE;
}

void motion_sensor_init(){
    VMINT ret;
    motion_data_param acc_cali_data;
    ret = motion_get_calibration(&acc_cali_data);
    if(ret == VM_TRUE) /*already has cali values in file*/
        motion_sensor_cali_milli_acc_data = acc_cali_data;
    motion_sensor_custom_dp = ms_custom_func.ms_get_data();
    motion_sensor_cali_data.x_offset=(VMINT16)motion_sensor_custom_dp->x_0g_adc;
    motion_sensor_cali_data.y_offset=(VMINT16)motion_sensor_custom_dp->y_0g_adc;
    motion_sensor_cali_data.z_offset=(VMINT16)motion_sensor_custom_dp->z_0g_adc;
    motion_sensor_cali_data.x_gain=
    (motion_sensor_custom_dp->x_1g_adc-motion_sensor_custom_dp->x_minus1g_adc)/2;
    motion_sensor_cali_data.y_gain=
    (motion_sensor_custom_dp->y_1g_adc-motion_sensor_custom_dp->y_minus1g_adc)/2;
    motion_sensor_cali_data.z_gain=
    (motion_sensor_custom_dp->z_1g_adc-motion_sensor_custom_dp->z_minus1g_adc)/2;
    if(motion_sensor_cali_data.x_gain==0){
        motion_sensor_cali_data.x_gain=0;
    }
    else
        motion_sensor_cali_data.x_gain=(1000<<10)/(motion_sensor_cali_data.x_gain);
    if(motion_sensor_cali_data.y_gain==0){
        motion_sensor_cali_data.y_gain=0;
    }
    else
        motion_sensor_cali_data.y_gain=(1000<<10)/(motion_sensor_cali_data.y_gain);
    if(motion_sensor_cali_data.z_gain==0){
        motion_sensor_cali_data.z_gain=0;
    }   
    else
        motion_sensor_cali_data.z_gain=(1000<<10)/(motion_sensor_cali_data.z_gain);
}
VMINT motion_app_message(VMUINT msg_id){
    VMINT32 i = 0, min;
    VMINT32 sender_idx = MAX_USER;
    MOTION_STATUS_INT hw_op_result = MOTION_STATUS_OK;
    VMUCHAR get_mode;
    motion_data_param *get_data;
    motion_data_param lparam = {120,100,400};
    motion_data_param *set_data = (motion_data_param *)&lparam;
    switch (msg_id){
		case MSG_MOTION_ENABLE:
        vm_log_info("deal with MSG_MOTION_ENABLE start");
        hw_op_result = ms_custom_func.ms_enable((VMBOOL)1);
        vm_log_info("deal with MSG_MOTION_ENABLE end %d",hw_op_result);
        break;/* MSG_MOTION_ENABLE */
    case MSG_MOTION_INIT:
        vm_log_info("deal with MSG_MOTION_INIT start");
        hw_op_result = ms_custom_func.ms_custom_init();
        motion_sensor_init();
        vm_log_info("deal with MSG_MOTION_INIT end %d",hw_op_result);
        break;/* MSG_MOTION_INIT */
    case MSG_MOTION_GET_DATA:
        vm_log_info("deal with MSG_MOTION_GET_DATA start");
        hw_op_result = ms_custom_func.ms_read_adc(&acc_data);
        vm_log_info("MSG_MOTION_GET_DATA data 0 %d %d %d ", acc_data.acc_data_x, acc_data.acc_data_y, acc_data.acc_data_z);
        motion_sensor_get_acc(&acc_data, acc_data.acc_data_x, acc_data.acc_data_y, acc_data.acc_data_z);
        vm_log_info("MSG_MOTION_GET_DATA data 1 %d %d %d ", acc_data.acc_data_x, acc_data.acc_data_y, acc_data.acc_data_z);
        acc_data.acc_data_x -= motion_sensor_cali_milli_acc_data.acc_data_x;
        acc_data.acc_data_y -= motion_sensor_cali_milli_acc_data.acc_data_y;
        acc_data.acc_data_z -= motion_sensor_cali_milli_acc_data.acc_data_z;
        vm_log_info("deal with MSG_MOTION_GET_DATA end %d",hw_op_result);
        get_data = (motion_data_param *)(VMINT)&acc_data;
        vm_log_info("MSG_MOTION_GET_DATA data 2 %d %d %d", acc_data.acc_data_x, acc_data.acc_data_y, acc_data.acc_data_z);
        break;/* MSG_MOTION_GET_DATA */
    case MSG_MOTION_GET_RAW_DATA:
        hw_op_result = ms_custom_func.ms_read_raw_data(&acc_raw_data);
        break;/* MSG_MOTION_GET_RAW_DATA */
    case MSG_MOTION_RESET:
        hw_op_result = ms_custom_func.ms_reset();
        break;/* MSG_MOTION_RESET */

    default:
        break;
    }
    return 0;
}




 void customer_precise_timer_example_proc(VM_TIMER_ID_PRECISE tid, void* user_data){
    motion_app_message(MSG_MOTION_GET_DATA);
    motion_app_message(MSG_MOTION_GET_RAW_DATA);
}

 /* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param) {
    vm_log_debug("Motion Driver handle_sysevt %d", message);
    switch (message) {
    case VM_EVENT_CREATE:
        ms_custom_func_config();
        vm_log_info("Motion driver VM_MSG_CREATE %d", memmove(0, 0, 0));
        motion_sensor_init();
        motion_app_message(MSG_MOTION_INIT);
        motion_app_message(MSG_MOTION_ENABLE);
        g_ptimer_id = vm_timer_create_precise(1000, (vm_timer_precise_callback)customer_precise_timer_example_proc, NULL);
        if(g_ptimer_id < 0){
            vm_log_debug("customer_run_timer create timer fail");
        }
        break;
    case VM_EVENT_QUIT:
        if(g_ptimer_id>0){
            vm_timer_delete_precise(g_ptimer_id);
        }
        vm_log_info("Motion driver VM_MSG_QUIT");
        break;
    }
}
/* Entry point */
void vm_main(void){
	/* register system events handler */
	vm_pmng_register_system_event_callback(handle_sysevt);
}

