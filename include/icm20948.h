#ifndef ICM20948_H
#define ICM20948_H

#define _USE_MATH_DEFINES
#include <mmath/mmath.h>

#include <cmath>

#include "ICM_20948.h"
#include "pwrUtility.hpp"
#include "sensors/sensor.h"
#include "system/observablevalue.h"
#include "sensesp.h"

#define WIRE_PORT Wire

struct EulerAngles {
  double pitch, roll, yaw;
};

struct IcmData {
  ObservableValue<double> pitch;
  ObservableValue<double> roll;
  ObservableValue<double> yaw;
  ObservableValue<double> rateOfTurn;
  ObservableValue<int16_t> accuracy;
};

class Icm20948 : Sensor {
 public:
  Icm20948();
  virtual void enable() override final;
  IcmData data;

 private:
  ICM_20948_I2C icm;  // Otherwise create an ICM_20948_I2C object
  EulerAngles eulerAngles;
  icm_20948_DMP_data_t dmpData;
  mmath::Quaternion<double> calibration;
  boolean available();
  mmath::Vector<3, double> findGravity();
  mmath::Quaternion<double> rotationBetweenTwoVectors(
      mmath::Vector<3, double> a, mmath::Vector<3, double> b);
  mmath::Vector<3, double> crossProduct(mmath::Vector<3, double> a,
                                        mmath::Vector<3, double> b);
  mmath::Quaternion<double> conj(mmath::Quaternion<double> q);

  mmath::Quaternion<double> axisAngleQuaternion(mmath::Vector<3, double> axis,
                                                double angle);
};

#endif