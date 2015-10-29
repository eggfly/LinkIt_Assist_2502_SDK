/*
  This sample code is in public domain.

  This sample code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _MOTION_SENSOR_DRIVER_H_
#define	_MOTION_SENSOR_DRIVER_H_

#define MOTION_SENSOR_CALI_SAMPLE_NUM 10
#define MOTION_SENSOR_MILLI_ACC_CALI_THRESHOLD 100 // 0.1G
#define MOTION_SENSOR_CALIBRATION_FILE "pedometer\\MOTION_SENSOR.dat"
#define TS_MOTION_SENSOR_FILE          "pedometer\\TsMotionSensor.vtp"

typedef VMINT MOTION_STATUS_INT;

typedef struct {
    VMUINT16 acc_data_x;
    VMUINT16 acc_data_y;
    VMUINT16 acc_data_z;
}motion_data_param;

typedef enum{
    MOTION_STATUS_OK = 0,
    MOTION_STATUS_FAILED = -1,
    MOTION_STATUS_TOO_MANY_USERS = -2,
    MOTION_STATUS_NOT_CONNECTED = -3,
    MOTION_STATUS_NOT_SUPPORTED = -4,
    MOTION_STATUS_HW_INIT_FAILED = -5
}motion_status_enum;

typedef struct {
   /*ADC*/
    VMUINT16 x_0g_adc;
    VMUINT16 x_1g_adc;
    VMUINT16 x_minus1g_adc;
    VMUINT16 y_0g_adc;
    VMUINT16 y_1g_adc;
    VMUINT16 y_minus1g_adc;
    VMUINT16 z_0g_adc;
    VMUINT16 z_1g_adc;
    VMUINT16 z_minus1g_adc;		
    VMBOOL   int_support;	
    VMUCHAR  int_level;
    VMUCHAR  int_chan;
} motion_custom_data_struct;

typedef struct{
    VMINT16	x_gain;
    VMINT16 	y_gain;
    VMINT16 	z_gain;
    VMINT16	x_offset;
    VMINT16 	y_offset;
    VMINT16 	z_offset;
}motion_cali_struct;

/*Gusture porting start*/

typedef enum{
    SRV_SENSOR_MOTION_NORMAL_TILT,       /*Filter noise*/
    SRV_SENSOR_MOTION_RAW_TILT,          /*May have noise*/
}motion_tilt_sensitive_enum;

/*sensor_type = SRV_SENSOR_MOTION_TILT*/
typedef struct{
    motion_tilt_sensitive_enum  sensitive;
}motion_tilt_cfg_struct;

typedef struct{
    VMINT32 gx;
    VMINT32 gy; 
    VMINT32 gz;
}motion_tilt_angle_struct;

typedef struct{
    VMINT16 x;
    VMINT16 y; 
    VMINT16 z;
}motion_tilt_acc_struct;


typedef struct{
    motion_tilt_angle_struct angle;
    motion_tilt_acc_struct   acc;
}motion_tilt_struct;


typedef struct{
    motion_tilt_struct          tilt;                /*tilt data*/
    motion_tilt_cfg_struct      tilt_sen;
    VMBOOL    is_tilt;
} motion_app_struct;

/*Detail infomation of tilt detection*/
typedef struct {
    VMINT32 angle_gx;   /*X angle related to acceleration of gravity, 0-180 and less then 0 means not support*/
    VMINT32 angle_gy;   /*Y angle related to acceleration of gravity, 0-180 and less then 0 means not support*/
    VMINT32 angle_gz;   /*Z angle 0-90(for hardware not support) and less then 0 means can't detect this time*/
    VMINT16 acc_x;      /*acc*10 at x axis*/
    VMINT16 acc_y;      /*acc*10 at y axis*/
    VMINT16 acc_z;      /*acc*10 at z axis*/
} motion_detail_struct;

/*Gusture porting end*/


typedef struct{
    motion_app_struct       app;
    motion_detail_struct    detail_info;
    VMBOOL                             is_msg_send;	
} motion_cntx_struct;

#endif
