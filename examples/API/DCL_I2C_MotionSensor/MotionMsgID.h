/*
  This sample code is in public domain.

  This sample code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _MOTION_MSGID_
#define	_MOTION_MSGID_

/* when ap mode application launch success, it will receive this message */
#define VM_MSG_LOAD_TINY                                12

#define MSG_MOTION_CONN_SERVER                          1000   /* APP -> Driver */
#define MSG_MOTION_DISCONN_SERVER                       1001   /* APP -> Driver */

#define MSG_MOTION_ENABLE                               1002   /* APP -> Driver */
#define MSG_MOTION_INIT									1003   /* APP -> Driver */
#define MSG_MOTION_GET_CHIP_INFO						1004   /* APP -> Driver */
#define MSG_MOTION_SET_MODE								1005   /* APP -> Driver */
#define MSG_MOTION_GET_MODE								1006   /* APP -> Driver */
#define MSG_MOTION_GET_DATA								1007   /* APP -> Driver */
#define MSG_MOTION_GET_RAW_DATA							1008   /* APP -> Driver */
#define MSG_MOTION_RESET								1009   /* APP -> Driver */
#define MSG_MOTION_CALIBRATION_ENABLE                   1010   /* APP -> Driver */
#define MSG_MOTION_SET_CALIBRATION_DATA					1011   /* APP -> Driver */
#define MSG_MOTION_GET_CALIBRATION_DATA					1012   /* APP -> Driver */
#define MSG_MOTION_CLEAR_CALIBRATION_DATA				1013   /* APP -> Driver */
#define MSG_MOTION_SET_SAMPLING_PERIOD                  1014   /* APP -> Driver */
#define MSG_MOTION_GET_SAMPLING_PERIOD                  1015   /* APP -> Driver */

#define MSG_MOTION_RUN_CALIBRATION                      1016
#define MSG_MOTION_FINISH_CALIBRATION                   1017



#define MSG_MOTION_CUSTOM_START	                        2000



#define MSG_MOTION_CONN_SERVER_CNF                      3000   /* Driver -> APP */
#define MSG_MOTION_DISCONN_SERVER_CNF                   3001   /* Driver -> APP */
#define MSG_MOTION_ENABLE_CNF                           3002   /* Driver -> APP */
#define MSG_MOTION_INIT_CNF					            3003   /* Driver -> APP */
#define MSG_MOTION_GET_CHIP_INFO_CNF		            3004   /* Driver -> APP */
#define MSG_MOTION_SET_MODE_CNF				            3005   /* Driver -> APP */
#define MSG_MOTION_GET_MODE_CNF			                3006   /* Driver -> APP */
#define MSG_MOTION_GET_DATA_CNF     		            3007   /* APP -> Driver (Optional) */
#define MSG_MOTION_GET_RAW_DATA_CNF 					3008   /* Driver -> APP */
#define MSG_MOTION_RESET_CNF       						3009   /* Driver -> APP */
#define MSG_MOTION_CALIBRATION_ENABLE_CNF			    3010   /* APP -> Driver (Optional) */
#define MSG_MOTION_SET_CALIBRATION_DATA_CNF             3011   /* Driver -> APP */
#define MSG_MOTION_GET_CALIBRATION_DATA_CNF             3012   /* Driver -> APP */
#define MSG_MOTION_CLEAR_CALIBRATION_CNF                3013   /* Driver -> APP */
#define MSG_MOTION_SET_SAMPLING_PERIOD_CNF              3014   /* Driver -> APP */
#define MSG_HRS_GET_WORKING_MODE_CNF         		    3015   /* Driver -> APP */
#define MSG_MOTION_GET_SAMPLING_PERIOD_CNF              3016   /* APP -> Driver */
/*Gusture porting start*/
#define MSG_MOTION_GET_SENSOR_TILTE_REQ                 3017   /* APP -> APP */
#define MSG_MOTION_GET_SENSOR_TILTE_CNF                 3018   /* APP -> APP */
/*Gusture porting end*/

#define MSG_MOTION_RUN_CALIBRATION_CNF                  3019   /* APP -> APP */
#define MSG_MOTION_FINISH_CALIBRATION_CNF               3020

#define MSG_MOTION_CUSTOM_CNF_START                     4000

#endif
