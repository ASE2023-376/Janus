/*
 * $Id$
 *
 * Copyright (C) 2010 Antoine Drouin <poinix@gmail.com>
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef IMU_ASPIRIN_H
#define IMU_ASPIRIN_H

#include "generated/airframe.h"
#include "subsystems/imu.h"

#include "mcu_periph/i2c.h"
#include "peripherals/itg3200.h"
#include "peripherals/hmc5843.h"
#include "peripherals/adxl345.h"


#ifdef IMU_ASPIRIN_VERSION_2_0
#define IMU_MAG_X_CHAN 2
#define IMU_MAG_Y_CHAN 0
#define IMU_MAG_Z_CHAN 1
#if !defined IMU_MAG_X_SIGN & !defined IMU_MAG_Y_SIGN & !defined IMU_MAG_Z_SIGN
#define IMU_MAG_X_SIGN 1
#define IMU_MAG_Y_SIGN -1
#define IMU_MAG_Z_SIGN 1
#endif
#endif

#if !defined IMU_GYRO_P_SIGN & !defined IMU_GYRO_Q_SIGN & !defined IMU_GYRO_R_SIGN
#define IMU_GYRO_P_SIGN   1
#define IMU_GYRO_Q_SIGN   1
#define IMU_GYRO_R_SIGN   1
#endif
#if !defined IMU_ACCEL_X_SIGN & !defined IMU_ACCEL_Y_SIGN & !defined IMU_ACCEL_Z_SIGN
#define IMU_ACCEL_X_SIGN  1
#define IMU_ACCEL_Y_SIGN  1
#define IMU_ACCEL_Z_SIGN  1
#endif

enum Aspirin2Status
  { Aspirin2StatusUninit,
    Aspirin2StatusIdle,
    Aspirin2StatusReading
  };

struct ImuAspirin2 {
  volatile enum Aspirin2Status status;
  volatile uint8_t imu_available;
  volatile uint8_t imu_tx_buf[64];
  volatile uint8_t imu_rx_buf[64];
  uint32_t time_since_last_reading;
};

extern struct ImuAspirin2 imu_aspirin2;


#define ASPIRIN2_TIMEOUT 3
/*

#define foo_handler() {}
#define ImuMagEvent(_mag_handler) {					\
      MagEvent(foo_handler); \
}


    if (hmc5843.data_available) {			\
      imu.mag_unscaled.x = hmc5843.data.value[IMU_MAG_X_CHAN];		\
      imu.mag_unscaled.y = hmc5843.data.value[IMU_MAG_Y_CHAN];		\
      imu.mag_unscaled.z = hmc5843.data.value[IMU_MAG_Z_CHAN];		\
      _mag_handler();							\
      hmc5843.data_available = FALSE;		\
    }									\
*/

/* underlying architecture */
#include "subsystems/imu/imu_aspirin2_arch.h"
/* must be implemented by underlying architecture */
extern void imu_aspirin2_arch_init(void);

/*
static inline void gyro_copy_i2c(void)
{
  int16_t gp = imu_aspirin.i2c_trans_gyro.buf[0]<<8 | imu_aspirin.i2c_trans_gyro.buf[1];
  int16_t gq = imu_aspirin.i2c_trans_gyro.buf[2]<<8 | imu_aspirin.i2c_trans_gyro.buf[3];
  int16_t gr = imu_aspirin.i2c_trans_gyro.buf[4]<<8 | imu_aspirin.i2c_trans_gyro.buf[5];
  RATES_ASSIGN(imu.gyro_unscaled, gp, gq, gr);
}

static inline void accel_copy_spi(void)
{
  const int16_t ax = imu_aspirin.accel_rx_buf[1] | (imu_aspirin.accel_rx_buf[2]<<8);
  const int16_t ay = imu_aspirin.accel_rx_buf[3] | (imu_aspirin.accel_rx_buf[4]<<8);
  const int16_t az = imu_aspirin.accel_rx_buf[5] | (imu_aspirin.accel_rx_buf[6]<<8);
  VECT3_ASSIGN(imu.accel_unscaled, ax, ay, az);
}


static inline void imu_gyro_event(void (* _gyro_handler)(void))
{

}
*/

static inline void imu_aspirin2_event(void (* _gyro_handler)(void), void (* _accel_handler)(void), void (* _mag_handler)(void))
{
  if (imu_aspirin2.status == Aspirin2StatusUninit) return;

  imu_aspirin2_arch_int_disable();
  if (imu_aspirin2.imu_available) {
    imu_aspirin2.time_since_last_reading = 0;
    imu_aspirin2.imu_available = FALSE;
    //accel_copy_spi();
    _accel_handler();
  }
  imu_aspirin2_arch_int_enable();

  // Reset everything if we've been waiting too long
  if (imu_aspirin2.time_since_last_reading > ASPIRIN2_TIMEOUT) {
    imu_aspirin2.time_since_last_reading = 0;
    return;
  }

}

#define ImuEvent(_gyro_handler, _accel_handler, _mag_handler) do {		\
  imu_aspirin2_event(_gyro_handler, _accel_handler, _mag_handler); \
} while(0);

#endif /* IMU_ASPIRIN_H */
