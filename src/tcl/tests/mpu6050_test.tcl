proc mpu6050_config {} {
	source drivers/mpu6050_regs.tcl
	
	mpu6050_write_reg $MPU6050_PWR_MGMT_1 0x80;
	
	# 解除休眠
	mpu6050_write_reg $MPU6050_PWR_MGMT_1 0x00;
	after 200;
	
	# 陀螺仪采样率，1khz效果不错
	mpu6050_write_reg $MPU6050_SMPLRT_DIV 0x00
	
	# 加速度44hz滤波，陀螺仪42hz滤波
	mpu6050_write_reg $MPU6050_CONFIG 0x06
	
	#陀螺仪最大量程 +-2000度每秒
	mpu6050_write_reg $MPU6050_GYRO_CONFIG 0x10
	
	#加速度度最大量程 +-4G
	mpu6050_write_reg $MPU6050_ACCEL_CONFIG 0x08
}

proc mpu6050_get_s16 {li index} {
	set hbyte	[lindex $li [expr $index]]
	set lbyte	[lindex $li [expr $index+1]]
	
	set dat16	[expr ($hbyte<<8) | $lbyte]
	set dat16	[expr ($dat16 & 0x8000) ?($dat16-0x10000) :$dat16]
	
	return $dat16
}

proc mpu6050_get_current_data { __fAccelX __fAccelY __fAccelZ __fGyroX __fGyroY __fGyroZ __fTemp } {
	source drivers/mpu6050_regs.tcl
	upvar $__fAccelX	fAccelX
	upvar $__fAccelY	fAccelY
	upvar $__fAccelZ	fAccelZ
	upvar $__fGyroX		fGyroX
	upvar $__fGyroY		fGyroY
	upvar $__fGyroZ		fGyroZ
	upvar $__fTemp		fTemp
	
	set li [mpu6050_read_reg $MPU6050_ACCEL_XOUT_H 14]
	
	set fGyroPrecision 	[ expr 2000.0 / 65536.0]
	set fAccelPrecision	[ expr 8.0 * $MPU6050_GRAVITY_ACCEL / 65536.0]
	
	set fAccelX	[expr $fGyroPrecision * [mpu6050_get_s16 $li 0] ]
	set fAccelY	[expr $fGyroPrecision * [mpu6050_get_s16 $li 2] ]
	set fAccelZ	[expr $fGyroPrecision * [mpu6050_get_s16 $li 4] ]
	set fGyroX	[expr $fAccelPrecision * [mpu6050_get_s16 $li 8] ]
	set fGyroY	[expr $fAccelPrecision * [mpu6050_get_s16 $li 10]]
	set fGyroZ	[expr $fAccelPrecision * [mpu6050_get_s16 $li 12]]
	
	set fTemp	[mpu6050_get_s16 $li 6]
	set fTemp	[expr $fTemp / 340.0 + 36.53]
}

proc mpu6050_show_current_data {} {
	set fAccelX	0
	set fAccelY	0
	set fAccelZ	0
	set fGyroX	0
	set fGyroY	0
	set fGyroZ	0
	set fTemp	0
	
	mpu6050_get_current_data fAccelX fAccelY fAccelZ fGyroX fGyroY fGyroZ fTemp
	
	puts [format "%-7s : %.3f"  "AccelX"  $fAccelX ]
	puts [format "%-7s : %.3f"  "AccelY"  $fAccelY ]
    puts [format "%-7s : %.3f"  "AccelZ"  $fAccelZ ]
    puts [format "%-7s : %.3f"  "GyroX "  $fGyroX  ]
    puts [format "%-7s : %.3f"  "GyroY "  $fGyroY  ]
    puts [format "%-7s : %.3f"  "GyroZ "  $fGyroZ  ]
    puts [format "%-7s : %.3f"  "Temp  "  $fTemp   ]

}