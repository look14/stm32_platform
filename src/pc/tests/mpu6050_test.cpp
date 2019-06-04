#include "stdafx.h"
#include "mpu6050_test.h"
#include "mpu6050.h"
#include "common.h"

#include <cmath>

#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

/* Starting sampling rate. */
#define DEFAULT_MPU_HZ  (20)

#define q30  1073741824.0f

struct platform_data_s {
	signed char orientation[9];
};

static struct platform_data_s gyro_pdata = {
	1, 0, 0,
	0, 1, 0,
	0, 0, 1,
};

static void tap_cb(unsigned char direction, unsigned char count)
{
	switch (direction) {
	case TAP_X_UP:
		printf("Tap X+ ");
		break;
	case TAP_X_DOWN:
		printf("Tap X- ");
		break;
	case TAP_Y_UP:
		printf("Tap Y+ ");
		break;
	case TAP_Y_DOWN:
		printf("Tap Y- ");
		break;
	case TAP_Z_UP:
		printf("Tap Z+ ");
		break;
	case TAP_Z_DOWN:
		printf("Tap Z- ");
		break;
	default:
		return;
	}
	printf("x%d\n", count);
	return;
}

static void android_orient_cb(unsigned char orientation)
{
	switch (orientation) {
	case ANDROID_ORIENT_PORTRAIT:
		printf("Portrait\n");
		break;
	case ANDROID_ORIENT_LANDSCAPE:
		printf("Landscape\n");
		break;
	case ANDROID_ORIENT_REVERSE_PORTRAIT:
		printf("Reverse Portrait\n");
		break;
	case ANDROID_ORIENT_REVERSE_LANDSCAPE:
		printf("Reverse Landscape\n");
		break;
	default:
		return;
	}
}

static inline void run_self_test(void)
{
    int result;
    char test_packet[4] = {0};
    long gyro[3], accel[3];

    result = mpu_run_self_test(gyro, accel);
    if (result == 0x7) {
        /* Test passed. We can trust the gyro data here, so let's push it down
         * to the DMP.
         */
        float sens;
        unsigned short accel_sens;
        mpu_get_gyro_sens(&sens);
        gyro[0] = (long)(gyro[0] * sens);
        gyro[1] = (long)(gyro[1] * sens);
        gyro[2] = (long)(gyro[2] * sens);
        dmp_set_gyro_bias(gyro);
        mpu_get_accel_sens(&accel_sens);
        accel[0] *= accel_sens;
        accel[1] *= accel_sens;
        accel[2] *= accel_sens;
        dmp_set_accel_bias(accel);
    }
	else {
		printf("mpu_run_self_test is failed\n");
	}
}

void mpu6050_test(void)
{
	short dat16;
	int result;
	struct int_param_s int_param;

	mpu_config();

	result = mpu_init(&int_param);
	if (result) {
		printf("Could not initialize gyro.\n");
	}

	mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);

	/* Push both gyro and accel data into the FIFO. */
	mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
	mpu_set_sample_rate(DEFAULT_MPU_HZ);

	dmp_load_motion_driver_firmware();
	dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_pdata.orientation));
	dmp_register_tap_cb(tap_cb);
	dmp_register_android_orient_cb(android_orient_cb);

	u16 dmp_features = 
		DMP_FEATURE_6X_LP_QUAT | 
		DMP_FEATURE_TAP |
		DMP_FEATURE_ANDROID_ORIENT | 
		DMP_FEATURE_SEND_RAW_ACCEL | 
		DMP_FEATURE_SEND_CAL_GYRO |
		DMP_FEATURE_GYRO_CAL ;

	dmp_features |= DMP_FEATURE_PEDOMETER;

	dmp_enable_feature(dmp_features);
	dmp_set_fifo_rate(DEFAULT_MPU_HZ);

	run_self_test();

	mpu_set_dmp_state(1);

	unsigned char more = 0;
	unsigned long sensor_timestamp;

	double q0=1.0f, q1=0.0f, q2=0.0f, q3=0.0f;
	double Pitch=0, Roll=0, Yaw=0;

	u32 pedometer_step;
	u32 pedometer_walk_time;

	dmp_set_pedometer_step_count(0);
	dmp_set_pedometer_walk_time(0);

	while(1)
	{
		result = mpu_get_int_status(&dat16);

		if(result==0 && ( (dat16 & 0x0100) || more ) )
		{
			short gyro[3], accel_short[3], sensors;
			long accel[3], quat[4], temperature;
            /* This function gets new data from the FIFO when the DMP is in
             * use. The FIFO can contain any combination of gyro, accel,
             * quaternion, and gesture data. The sensors parameter tells the
             * caller which data fields were actually populated with new data.
             * For example, if sensors == (INV_XYZ_GYRO | INV_WXYZ_QUAT), then
             * the FIFO isn't being filled with accel data.
             * The driver parses the gesture data to determine if a gesture
             * event has occurred; on an event, the application will be notified
             * via a callback (assuming that a callback function was properly
             * registered). The more parameter is non-zero if there are
             * leftover packets in the FIFO.
             */
            dmp_read_fifo(gyro, accel_short, quat, &sensor_timestamp, &sensors, &more);

			if(sensors & INV_WXYZ_QUAT )
			{
				q0 = quat[0] / q30;	
				q1 = quat[1] / q30;
				q2 = quat[2] / q30;
				q3 = quat[3] / q30;

				Pitch = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3;	// pitch
				Roll  = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3;	// roll
				Yaw   = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;	//yaw

				dmp_get_pedometer_step_count(&pedometer_step);
				dmp_get_pedometer_walk_time(&pedometer_walk_time);

				printf("%.3f\t %.3f\t %.3f\n", Pitch, Roll, Yaw);
			}
			
            if (sensors & INV_XYZ_GYRO) {
                /* Push the new data to the MPL. */

                /* Temperature only used for gyro temp comp. */
				mpu_get_temperature(&temperature, &sensor_timestamp);
            }

            if (sensors & INV_XYZ_ACCEL) {
                accel[0] = (long)accel_short[0];
                accel[1] = (long)accel_short[1];
                accel[2] = (long)accel_short[2];
            }
        }
	}

	return;
}
