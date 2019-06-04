proc mpu6050_is_exist {} {
	source drivers/mpu6050_regs.tcl
	set dat8 [mpu6050_read_reg $MPU6050_WHO_AM_I]
	
	if {$dat8==""} {
		return false;
	}
	
	return [expr (0x7E & $dat8)==0x68 ?true :false]
}
