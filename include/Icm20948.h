#ifndef ICM20948_H
#define ICM20948_H

#define _USE_MATH_DEFINES
#include <mmath/mmath.h>

#include <cmath>

#include "ICM_20948.h"
#include "pwrUtility.hpp"
#include "sensesp.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/lambda_consumer.h"

#include "transforms/moving_average.h"

#define WIRE_PORT Wire

struct EulerAngles {
  double pitch, roll, yaw;
};

struct IcmData {
  sensesp::ObservableValue<double> pitch;
  sensesp::ObservableValue<double> roll;
  sensesp::ObservableValue<double> yaw;
  sensesp::ObservableValue<double> pitch_rate;
  sensesp::ObservableValue<double> roll_rate;
  sensesp::ObservableValue<double> yaw_rate;
  sensesp::ObservableValue<int16_t> accuracy;
  sensesp::ObservableValue<mmath::Vector<3, double>> gravity;
};

class Icm20948 : sensesp::Sensor {
 public:
  Icm20948(String config_path);
  virtual void start() override final;
  IcmData data;

 private:
  ICM_20948_I2C icm;  // Otherwise create an ICM_20948_I2C object
  EulerAngles eulerAngles;
  icm_20948_DMP_data_t dmpData;
  mmath::Quaternion<double> calibration_;
  double heading_offset_;
  bool calibrate_;
  bool enabled;
  mmath::Vector<3, double> gravity_;
  mmath::Vector<3, double> mean_gravity;

  boolean available();
  mmath::Vector<3, double> findGravity();
  mmath::Quaternion<double> rotationBetweenTwoVectors(
      mmath::Vector<3, double> a, mmath::Vector<3, double> b);
  mmath::Vector<3, double> crossProduct(mmath::Vector<3, double> a,
                                        mmath::Vector<3, double> b);
  mmath::Quaternion<double> conj(mmath::Quaternion<double> q);

  mmath::Quaternion<double> axisAngleQuaternion(mmath::Vector<3, double> axis,
                                                double angle);
  mmath::Quaternion<double> calculateCalibration(
      mmath::Vector<3, double>& gravity, double heading_offset);

  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

#endif