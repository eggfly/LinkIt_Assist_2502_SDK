This example shows how to use I2C driver module control bmi055 motion sensor.

It initializes motion sensor, and then creates a timer to get x,y,z data from sensor, and writes these data Monitor log. I2C operations are implemented in file MotionBmi055Qg.c. It calls vm_dcl_open() to open I2C, vm_dcl_control()to control I2C, and sends command VM_DCL_I2C_CMD_CONFIG to config I2C's slave address, transition mode, mode speed. It also sends command VM_DCL_I2C_CMD_SINGLE_WRITE and VM_DCL_I2C_CMD_WRITE_AND_READ to communicate with bmi055.

To use this example, connect the sensor to I2C pins. Refer to pin-out diagram for the specification of I2C pins.Monitor shows "motion i2c init success" "motion i2c single write" "motion i2c write and read reg" 
