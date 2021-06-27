#ifndef ICM20948_H
#define ICM20948_H

#define _USE_MATH_DEFINES
#include <cmath>

#include "ICM_20948.h"
#include "sensors/sensor.h"
#include "system/observablevalue.h"

#define SERIAL_PORT Serial

#define WIRE_PORT Wire

/**
 * The value of the last bit of the I2C address.
 * On the SparkFun 9DoF IMU breakout the default is 1, and when
 * the ADR jumper is closed the value becomes 0#define AD0_VAL 1
 */
struct EulerAngles {
  double roll, pitch, yaw;
};

struct Quaternion {
  double w, x, y, z;
};

struct IcmData {
  ObservableValue<double> pitch;
  ObservableValue<double> roll;
  ObservableValue<double> yaw;
  ObservableValue<int16_t> accuracy;
};

class Icm20948 : Sensor {
 public:
  Icm20948();
  virtual void enable() override final;
  IcmData data;

 private:
  ICM_20948_I2C icm;  // Otherwise create an ICM_20948_I2C object
  EulerAngles toEuler(Quaternion quaternion);
  const double RAD2DEG = (180.0 / M_PI);
  const double DEG2RAD = (M_PI / 180.0);
  boolean available();
  icm_20948_DMP_data_t dmpData;
};

#endif