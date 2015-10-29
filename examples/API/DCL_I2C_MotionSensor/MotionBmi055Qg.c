/*
  This sample code is in public domain.

  This sample code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "vmtype.h" 
#include "vmlog.h" 
#include "MotionMsgID.h"
#include "MotionSensor.h"
#include "MotionBmi055Qg.h"

/* Define to set SCL, SDA pins pull enable
   #define SET_PULL_ENABLE_ON_I2C_PINS */
VMUINT8 MS_sensor_buffer[10];

// ============================================================================
VM_DCL_HANDLE i2c_handle = VM_DCL_HANDLE_INVALID;
VM_DCL_HANDLE eint_gpio_handle = VM_DCL_HANDLE_INVALID;

VM_DCL_HANDLE eint_handle = VM_DCL_HANDLE_INVALID;

#if defined(SET_PULL_ENABLE_ON_I2C_PINS)
VM_DCL_HANDLE scl_handle = VM_DCL_HANDLE_INVALID;
VM_DCL_HANDLE sda_handle = VM_DCL_HANDLE_INVALID;
#endif //


MotionSensor_customize_function_struct ms_custom_func;

motion_custom_data_struct  ms_custom_data_def ={          
   /*X axis*/
    ACC_0G_X,
    ACC_1G_X,
    ACC_MINUS1G_X,     
    /*Y axis*/
    ACC_0G_Y,   
    ACC_1G_Y,
    ACC_MINUS1G_Y,     
    /*Z axis*/
    ACC_0G_Z,   
    ACC_1G_Z,
    ACC_MINUS1G_Z,
    /*support interrupt or not*/
    VM_FALSE,
    0,
    0/*channel*/    
};

motion_custom_data_struct * (ms_get_data)(void){
    return (&ms_custom_data_def);
} 

MotionSensor_customize_function_struct * (ms_GetFunc)(void){
    return &(ms_custom_func);
}


VMBOOL motion_i2c_init(void){
    vm_dcl_i2c_control_config_t conf_data;
    VM_DCL_STATUS status;
    if (VM_DCL_HANDLE_INVALID == i2c_handle){	
        vm_log_info("motion i2c_handle is null");
        i2c_handle = vm_dcl_open(VM_DCL_I2C, 0);
    }
    if (VM_DCL_HANDLE_INVALID == i2c_handle) {
        vm_log_info("motion i2c init failed 1");
        return VM_FALSE;
    }
    conf_data.reserved_0 = (VM_DCL_I2C_OWNER)0;
    conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
    conf_data.get_handle_wait = 0;
    conf_data.reserved_1 = 0;
    conf_data.delay_length = 0;
    conf_data.slave_address = MS_SLAVE_ADDR;
    conf_data.fast_mode_speed = 400;
    conf_data.high_mode_speed = 0;
    status = vm_dcl_control(i2c_handle, VM_DCL_I2C_CMD_CONFIG, (void *)&conf_data);
    if (VM_DCL_STATUS_OK != status){
        vm_log_info("motion i2c init failed 2, status: %d", status);
        return VM_FALSE;
    }
    vm_log_info("motion i2c init success, status: %d", status);
    return VM_TRUE;
}

VMBOOL motion_comm_init(void){
    VM_DCL_STATUS status;
    status = motion_i2c_init();
    if (VM_FALSE == status){
        vm_log_info("motion i2c init failed");
        return VM_FALSE;
    }
#if defined(SET_PULL_ENABLE_ON_I2C_PINS)
    scl_handle = vm_dcl_open(VM_DCL_GPIO, MS_GPIO_SCL);
    if (VM_DCL_HANDLE_INVALID == scl_handle){
        vm_log_info("motion comm_init failed 3");
        return VM_FALSE;
    }
    sda_handle = vm_dcl_open(VM_DCL_GPIO, MS_GPIO_SDA);
    if (VM_DCL_HANDLE_INVALID == sda_handle){
        vm_log_info("motion comm_init failed 4");
        return VM_FALSE;
    }
    vm_dcl_control(sda_handle, VM_GPIO_CMD_SET_PULL_HIGH, 0);
    vm_dcl_control(sda_handle, VM_GPIO_CMD_ENABLE_PULL, 0);
    vm_dcl_control(scl_handle, VM_GPIO_CMD_SET_PULL_HIGH, 0);
    vm_dcl_control(scl_handle, VM_GPIO_CMD_ENABLE_PULL, 0);
#endif
    return VM_TRUE;
}

VMBOOL motion_comm_deinit(void){
#if defined(SET_PULL_ENABLE_ON_I2C_PINS)
    if (scl_handle != VM_DCL_HANDLE_INVALID){
        vm_dcl_close(scl_handle);
    }
    if (sda_handle != VM_DCL_HANDLE_INVALID){
        vm_dcl_close(sda_handle);
    }
#endif
    if (eint_gpio_handle != VM_DCL_HANDLE_INVALID){
        vm_dcl_close(eint_gpio_handle);
    }
    if (eint_handle != VM_DCL_HANDLE_INVALID){
        vm_dcl_close(eint_handle);
    }
    if (i2c_handle != VM_DCL_HANDLE_INVALID){
        vm_dcl_close(i2c_handle);
    }
    return VM_TRUE;
}


VMBOOL bmi055_match_id(void){
    VMUINT8 id;
    id = bmi055_readRegister(MS_REG_CHIPID);
    vm_log_info("motion driver match_id: id = 0x%x", id);
    if (id == MS_CHIPID_VALUE){
        return VM_TRUE;
    }
    return VM_FALSE;
}

MOTION_STATUS_INT bmi055_sensor_init(void){
    VMBOOL ret;
    ret = motion_comm_init();
    if (VM_FALSE == ret){
        vm_log_info("motion_comm_init falied");
        return MOTION_STATUS_FAILED;
    }
    /* reset */
    bmi055_sensor_softreset();
    /* check chipid */
    ret = bmi055_match_id();
    if (VM_FALSE == ret){
        vm_log_info("chip ID don't match");
        return MOTION_STATUS_FAILED;
    }
    /* set range */
    bmi055_sensor_set_range(MS_RANGE_2G);
    /* set bandwidth */
    bmi055_sensor_set_bandwidth(MS_BW_125_HZ);
    /* power up */
    bmi055_sensor_pwr_up();
    /* power down */
    bmi055_sensor_pwr_down();
    return MOTION_STATUS_OK;
}


MOTION_STATUS_INT bmi055_sensor_enable(VMBOOL enable){
    if(enable)
        bmi055_sensor_pwr_up();
    else
        bmi055_sensor_pwr_down();
    return MOTION_STATUS_OK;
}

MOTION_STATUS_INT bmi055_sensor_deinit(void){
    VMBOOL ret;
    ret = motion_comm_deinit();
    if (VM_FALSE == ret){
        return VM_FALSE;
    }
    /* reset */
    bmi055_sensor_softreset();
    bmi055_sensor_pwr_down();
    return MOTION_STATUS_OK;
}

MOTION_STATUS_INT bmi055_sensor_softreset(void){
    vm_log_info("bmi055_sensor_softreset");
    bmi055_writeRegister(MS_REG_RESET, MS_RESET_VALUE);
    return MOTION_STATUS_OK;
}

MOTION_STATUS_INT bmi055_sensor_set_mode(VMUCHAR mode){
    vm_log_info("bmi055_sensor_set_mode");
    bmi055_writeRegister(MS_REG_MODE, mode);
    return MOTION_STATUS_OK;
}

MOTION_STATUS_INT bmi055_sensor_get_mode(VMUCHAR *return_mode){
    VMUINT8 mode;
    vm_log_info("bmi055_sensor_get_mode");
    mode = bmi055_readRegister(MS_REG_MODE);
    return_mode = &mode;
    return MOTION_STATUS_OK;
}


void bmi055_sensor_set_range(VMUCHAR range){
    vm_log_info("bmi055_sensor_set_range");
    bmi055_writeRegister(MS_REG_RANGE, range);
}


void bmi055_sensor_set_bandwidth(VMUCHAR bw){
    vm_log_info("bmi055_sensor_set_range");
    bmi055_writeRegister(MS_REG_BANDWIDTH, bw);
}


void bmi055_sensor_pwr_up(void){
    vm_log_info("bmi055_sensor_pwr_up");
    bmi055_sensor_set_mode(MS_NORMAL_MODE);
}
void bmi055_sensor_pwr_down(void){
    vm_log_info("bmi055_sensor_pwr_down");
    bmi055_sensor_set_mode(MS_DEEP_SUSPEND_MODE);
}

void bmi055_read_xyz(VMUINT16 *x,VMUINT16 *y,VMUINT16 *z){	
    bmi055_readRegisterBurst(MS_REG_XOUT_EX_L,&MS_sensor_buffer[0],6);
    *x = ((MS_sensor_buffer[1])<<4) + (MS_sensor_buffer[0]>>4);
    *y = ((MS_sensor_buffer[3])<<4) + (MS_sensor_buffer[2]>>4);
    *z = ((MS_sensor_buffer[5])<<4) + (MS_sensor_buffer[4]>>4);
    vm_log_info("RAW_DATA *x = %x, *y=%x,*z=%x",*x,*y,*z);
}


MOTION_STATUS_INT bmi055_sensor_get_raw_data(motion_data_param *motion_data){
    motion_data_param get_data;
    VMUINT16 x,y,z;
    bmi055_read_xyz(&x,&y,&z);
    get_data.acc_data_x = x;
    get_data.acc_data_y = y;
    get_data.acc_data_z = z;
    *motion_data = get_data;
    return MOTION_STATUS_OK;
}


MOTION_STATUS_INT bmi055_sensor_get_data(motion_data_param *motion_data){
    VMUINT16 x_temp,y_temp,z_temp;
    VMUINT16 x,y,z;
    motion_data_param get_data;
    bmi055_readRegisterBurst(MS_REG_CHIPID,&MS_sensor_buffer[0],1);
    vm_log_info("bmi055 sensor chip ID = %d",MS_sensor_buffer[0]);
    vm_log_info("bmi055_sensor_get_data");
    bmi055_read_xyz(&x_temp,&y_temp,&z_temp);	 
    if(x_temp<ACC_0G_X)     
        x=x_temp+ACC_0G_X;
    else
        x=x_temp-ACC_0G_X;
    if(y_temp<ACC_0G_Y)
        y=y_temp+ACC_0G_Y;
    else
        y=y_temp-ACC_0G_Y;
    if(z_temp<ACC_0G_Z)
        z=z_temp+ACC_0G_Z;
    else
        z=z_temp-ACC_0G_Z;

#if defined(MOTION_SENSOR_BACK_0)	
    get_data.acc_data_x = x;
    get_data.acc_data_y = y;
    get_data.acc_data_z = z;
#elif defined(MOTION_SENSOR_BACK_90)
    get_data.acc_data_x = y;
    get_data.acc_data_y = 2*ACC_0G_X-x;
    get_data.acc_data_z = z;
#elif defined(MOTION_SENSOR_BACK_180)
    get_data.acc_data_x = 2*ACC_0G_X-x;
    get_data.acc_data_y = 2*ACC_0G_Y-y;
    get_data.acc_data_z = z;
#elif defined(MOTION_SENSOR_BACK_270)
    get_data.acc_data_x = 2*ACC_0G_Y-y;
    get_data.acc_data_y = x;
    get_data.acc_data_z = z;
#elif defined(MOTION_SENSOR_FRONT_0)
    get_data.acc_data_x = 2*ACC_0G_X-x;
    get_data.acc_data_y = y;
    get_data.acc_data_z = 2*ACC_0G_Z-z;
#elif defined(MOTION_SENSOR_FRONT_90)
    get_data.acc_data_x = 2*ACC_0G_Y-y;
    get_data.acc_data_y = 2*ACC_0G_X-x;
    get_data.acc_data_z = 2*ACC_0G_Z-z;
#elif defined(MOTION_SENSOR_FRONT_180)
    get_data.acc_data_x = x;
    get_data.acc_data_y= 2*ACC_0G_Y-y;
    get_data.acc_data_z = 2*ACC_0G_Z-z;
#elif defined(MOTION_SENSOR_FRONT_270)
    get_data.acc_data_x = y;
    get_data.acc_data_y = x;
    get_data.acc_data_z = 2*ACC_0G_Z-z;
#endif

    *motion_data = get_data;
    vm_log_info("Motion Sensor Readed and value is x:%d  ,y:%d   ,z:%d  ",motion_data->acc_data_x,motion_data->acc_data_y,motion_data->acc_data_z);
    return MOTION_STATUS_OK;
}

void bmi055_intr_handler(void *parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle){
	/* do not support interrupt mode now */
    NULL;
}

VMBOOL bmi055_writeRegister(VMUINT8 Addr, VMUINT8 Val){
    vm_dcl_i2c_control_single_write_t write_data;
    VMUINT8 i2c_write_data[2];
    VM_DCL_STATUS status;
    i2c_write_data[0] = Addr;
    i2c_write_data[1] = Val;
    write_data.data_ptr = i2c_write_data;
    write_data.data_length = 2;
    status = vm_dcl_control(i2c_handle, VM_DCL_I2C_CMD_SINGLE_WRITE, (void *)&write_data);
    if (VM_DCL_STATUS_OK != status){
        vm_log_info("motion write reg: status = %d", status);
        return VM_FALSE;
    }
    vm_log_info("motion i2c single write Addr=%d,Val=%d,status = %d", Addr,Val,status);
    return VM_TRUE;
}

VMUINT8 bmi055_readRegister(VMUINT8 Addr){
    VMBOOL result;
    VMUINT8 data = 0xFF;
	
    result = bmi055_readRegisterBurst(Addr, &data, 1);
    return data;
}

VMBOOL bmi055_readRegisterBurst(VMUINT8 Addr, VMUINT8 *data, VMUINT8 len){
    vm_dcl_i2c_control_write_and_read_t write_and_read_data;
    VMUINT8 i2c_write_data[1];
    VM_DCL_STATUS status;

    i2c_write_data[0] = Addr;
	
    write_and_read_data.out_data_ptr = i2c_write_data;
    write_and_read_data.out_data_length = 1;
    write_and_read_data.in_data_ptr = data;
    write_and_read_data.in_data_length = len;
    status = vm_dcl_control(i2c_handle, VM_DCL_I2C_CMD_WRITE_AND_READ, (void *)&write_and_read_data);
    if (VM_DCL_STATUS_OK != status){
         vm_log_info("motion read reg burst: status = %d", status);
         return VM_FALSE;
    }
    vm_log_info("motion i2c write and read reg burst:Addr=%d,data[0]=%d,status = %d",Addr,data[0],status);
    return VM_TRUE;
}


/*customizaton data*/
void ms_custom_func_config(void){
    ms_custom_func.ms_get_data = ms_get_data;
    ms_custom_func.ms_enable = bmi055_sensor_enable;
    ms_custom_func.ms_custom_init = bmi055_sensor_init;
    ms_custom_func.ms_set_mode = bmi055_sensor_set_mode;
    ms_custom_func.ms_get_mode = bmi055_sensor_get_mode;
    ms_custom_func.ms_read_adc = bmi055_sensor_get_data;
    ms_custom_func.ms_read_raw_data= bmi055_sensor_get_raw_data;
    ms_custom_func.ms_reset = bmi055_sensor_softreset;
    ms_custom_func.ms_cali_enable= NULL;
    ms_custom_func.ms_set_cali_data= NULL;
    ms_custom_func.ms_get_cali_data= NULL;
    ms_custom_func.ms_clear_cali_data= NULL;
    ms_custom_func.ms_set_sample_period= NULL;
    ms_custom_func.ms_get_sample_period= NULL;
    vm_log_info("ms_custom_func_config");
}
