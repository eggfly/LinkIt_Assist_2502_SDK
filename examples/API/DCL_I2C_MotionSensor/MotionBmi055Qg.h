/*
  This sample code is in public domain.

  This sample code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _MOTION_BMI055_QG_H_
#define	_MOTION_BMI055_QG_H_

#include "vmdcl.h"
#include "vmdcl_i2c.h"
#include "vmdcl_gpio.h"
#include "vmdcl_eint.h"
#include "MotionSensor.h"


#define MOTION_SENSOR_BACK_180

#define MS_SLAVE_ADDR       0x30 /* 0x18 << 1 //68 for gyo */

/* BMI055 */
#define MS_REG_XOUT_EX_L 0x02
#define MS_REG_XOUT_EX_H 0x03
#define MS_REG_YOUT_EX_L 0x04
#define MS_REG_YOUT_EX_H 0x05
#define MS_REG_ZOUT_EX_L 0x06
#define MS_REG_ZOUT_EX_H 0x07

/* BMI055 chip ID */
#define MS_REG_CHIPID 0x00
#define MS_CHIPID_VALUE 0xFA

/* softreset */
#define MS_REG_RESET 0x14
#define MS_RESET_VALUE 0xb6

/* set mode */
#define MS_REG_MODE 0x11

#define MS_NORMAL_MODE 0x00    
#define MS_DEEP_SUSPEND_MODE 0x20
#define MS_LOW_POWER_MODE 0x40    
#define MS_SUSPEND_MODE 0x80    


/* RANGE */
#define MS_REG_RANGE 0x0f
#define MS_RANGE_2G 0x03
#define MS_RANGE_4G 0x05
#define MS_RANGE_8G 0x08
#define MS_RANGE_16G 0xc0

/* BAND WIDTH */
#define MS_REG_BANDWIDTH 0x10
#define MS_BW_7_81_HZ 0x08
#define MS_BW_15_63_HZ 0x09
#define MS_BW_31_25_HZ 0x0a
#define MS_BW_62_5_HZ 0x0b
#define MS_BW_125_HZ 0x0c
#define MS_BW_250_HZ 0x0d
#define MS_BW_500_HZ 0x0e
#define MS_BW_1000_HZ 0x0f


/* acceleration 12-Bits,+/-2g */
#define BMI055_RANGE       4	/* +/-1.5g */
#define BMI055_RESOLUTION  12	/* 8-Bits */

#define ACC_0G_X      2048
#define ACC_1G_X      (2048+1024)
#define ACC_MINUS1G_X (2048-1024)
#define ACC_0G_Y      2048   
#define ACC_1G_Y      (2048+1024)
#define ACC_MINUS1G_Y (2048-1024)
#define ACC_0G_Z      2048       
#define ACC_1G_Z      (2048+1024)
#define ACC_MINUS1G_Z (2048-1024)

#define MS_EINT        3
#define MS_GPIO_EINT   4 /* GPIO4 */

#define MS_GPIO_SCL  43  /* GPIO43 */
#define MS_GPIO_SDA  44  /* GPIO44 */

typedef struct {  
   motion_custom_data_struct * (*ms_get_data)(void);
   MOTION_STATUS_INT (*ms_enable)(VMBOOL enable);
   MOTION_STATUS_INT (*ms_custom_init)(void);
   MOTION_STATUS_INT (*ms_set_mode)(VMUCHAR mode);
   MOTION_STATUS_INT (*ms_get_mode)(VMUCHAR *return_mode);
   MOTION_STATUS_INT (*ms_read_adc)(motion_data_param *motion_data);
   MOTION_STATUS_INT (*ms_read_raw_data)(motion_data_param *motion_data);
   MOTION_STATUS_INT (*ms_reset)(void);
   MOTION_STATUS_INT (*ms_cali_enable)(void);
   MOTION_STATUS_INT (*ms_set_cali_data)(motion_data_param *cali_data);
   MOTION_STATUS_INT (*ms_get_cali_data)(motion_data_param *cali_data);
   MOTION_STATUS_INT (*ms_clear_cali_data)(void);
   MOTION_STATUS_INT (*ms_set_sample_period)(VMUINT32 sample_period);
   MOTION_STATUS_INT (*ms_get_sample_period)(VMUINT32 sample_period);
}MotionSensor_customize_function_struct;

extern MotionSensor_customize_function_struct ms_custom_func;

extern VMBOOL motion_i2c_init(void);
extern VMBOOL motion_comm_init(void);
extern VMBOOL motion_comm_deinit(void);
extern VMBOOL bmi055_match_id(void);
extern MOTION_STATUS_INT bmi055_sensor_init(void);
extern MOTION_STATUS_INT bmi055_sensor_deinit(void);
extern MOTION_STATUS_INT bmi055_sensor_softreset(void);
extern MOTION_STATUS_INT bmi055_sensor_set_mode(VMUCHAR mode);
extern MOTION_STATUS_INT bmi055_sensor_get_mode(VMUCHAR *return_mode);
extern void bmi055_sensor_set_range(VMUCHAR range);
extern void bmi055_sensor_set_bandwidth(VMUCHAR bw);
extern void bmi055_sensor_pwr_up(void);
extern void bmi055_sensor_pwr_down(void);
extern void bmi055_read_xyz(VMUINT16 *x,VMUINT16 *y,VMUINT16 *z);
extern MOTION_STATUS_INT bmi055_sensor_get_data(motion_data_param *motion_data);
extern void bmi055_intr_handler(void *parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle);
extern VMBOOL bmi055_writeRegister(VMUINT8 Addr, VMUINT8 Val);
extern VMUINT8 bmi055_readRegister(VMUINT8 Addr);
extern VMBOOL bmi055_readRegisterBurst(VMUINT8 Addr, VMUINT8 *data, VMUINT8 len);
extern void ms_custom_func_config(void);

#endif 
