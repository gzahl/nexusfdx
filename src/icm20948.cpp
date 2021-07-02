#include "icm20948.h"

Icm20948::Icm20948() { eulerAngles.yaw = 0.0; }

void Icm20948::enable() {
  Serial.println("Enabling icm20948!");

  WIRE_PORT.begin();
  WIRE_PORT.setClock(400000);

  // Uncomment this line to enable helpful debug messages on Serial
  // icm.enableDebugging();

  bool initialized = false;
  while (!initialized) {
    // Initialize the ICM-20948
    // If the DMP is enabled, .begin performs a minimal startup. We need to
    // configure the sample mode etc. manually.
    icm.begin(WIRE_PORT, 1);

    SERIAL_PORT.print(F("Initialization of the sensor returned: "));
    SERIAL_PORT.println(icm.statusString());
    if (icm.status != ICM_20948_Stat_Ok) {
      SERIAL_PORT.println(F("Trying again..."));
      delay(500);
    } else {
      initialized = true;
    }
  }

  SERIAL_PORT.println(F("Device connected!"));

  bool success = true;

  success &= (icm.initializeDMP() == ICM_20948_Stat_Ok);
  success &= (icm.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) ==
              ICM_20948_Stat_Ok);
  success &=
      (icm.enableDMPSensor(INV_ICM20948_SENSOR_GYROSCOPE) == ICM_20948_Stat_Ok);
  success &= (icm.enableDMPSensor(INV_ICM20948_SENSOR_LINEAR_ACCELERATION) ==
              ICM_20948_Stat_Ok);

  // Configuring DMP to output data at multiple ODRs:
  // DMP is capable of outputting multiple sensor data at different rates to
  // FIFO. Setting value can be calculated as follows: Value = (DMP running rate
  // / ODR ) - 1 E.g. For a 5Hz ODR rate when DMP is running at 55Hz, value =
  // (55/5) - 1 = 10.
  int fusionrate = 4;
  success &=
      (icm.setDMPODRrate(DMP_ODR_Reg_Quat9, fusionrate) == ICM_20948_Stat_Ok);
  success &= (icm.setDMPODRrate(DMP_ODR_Reg_Gyro_Calibr, fusionrate) ==
              ICM_20948_Stat_Ok);
  success &=
      (icm.setDMPODRrate(DMP_ODR_Reg_Accel, fusionrate) == ICM_20948_Stat_Ok);

  success &= (icm.enableFIFO() == ICM_20948_Stat_Ok);
  success &= (icm.enableDMP() == ICM_20948_Stat_Ok);
  success &= (icm.resetDMP() == ICM_20948_Stat_Ok);
  success &= (icm.resetFIFO() == ICM_20948_Stat_Ok);

  // Check success
  if (success) {
    SERIAL_PORT.println(F("DMP enabled!"));
  } else {
    SERIAL_PORT.println(F("Enable DMP failed!"));
    SERIAL_PORT.println(
        F("Please check that you have uncommented line 29 (#define "
          "ICM_20948_USE_DMP) in ICM_20948_C.h..."));
    while (1)
      ;  // Do nothing more
  }

  auto gravity = findGravity();
  SERIAL_PORT.printf("Found gravity: %f, %f, %f\n", gravity.x, gravity.y,
                     gravity.z);
  auto down = mmath::Vector<3, double>(0., 0., 1.);
  auto up = mmath::Vector<3, double>(0., 0., -1.);
  auto rotateToGravity = rotationBetweenTwoVectors(gravity, down);
  auto correctToNorth = axisAngleQuaternion(up, mmath::Angles::DegToRad(5.));
  // calibration = correctToNorth * rotateToGravity;
  calibration = rotateToGravity;
  calibration = calibration / calibration.norm();
  Serial.printf("calibration quaternion: %f %f %f %f\n", calibration.w,
                calibration.x, calibration.y, calibration.z);

  app.onTick([this]() {
    while (available()) {
      if ((dmpData.header & DMP_header_bitmap_Quat9) > 0) {
        // Q0 value is computed from this equation: Q0^2 + Q1^2 + Q2^2 + Q3^2
        // = 1. In case of drift, the sum will not add to 1, therefore,
        // quaternion data need to be corrected with right bias values. The
        // quaternion data is scaled by 2^30.

        // SERIAL_PORT.printf("Quat9 data is: Q1:%ld Q2:%ld Q3:%ld
        // Accuracy:%d\r\n", data.Quat9.Data.Q1, data.Quat9.Data.Q2,
        // data.Quat9.Data.Q3, data.Quat9.Data.Accuracy);

        // Scale to +/- 1
        const double scale = 1.0 / pwrtwo(30);
        double q1 = ((double)dmpData.Quat9.Data.Q1) * scale;
        double q2 = ((double)dmpData.Quat9.Data.Q2) * scale;
        double q3 = ((double)dmpData.Quat9.Data.Q3) * scale;
        double q0 = -sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));

        mmath::Quaternion<double> quaternion(q0, q1, q2, q3);

        auto quat_calibrated = calibration * quaternion * conj(calibration);

        auto euler = quat_calibrated.ToEulerXYZ();

        data.pitch.emit(mmath::Angles::RadToDeg(-euler.x));
        data.roll.emit(mmath::Angles::RadToDeg(euler.y));
        data.yaw.emit(mmath::Angles::RadToDeg(euler.z));

        /*
        const double degMillisToDegMinutes = 60000.;
        data.rateOfTurn.emit((eulerAngles.yaw - yaw0) /
                             (eulerAngles.time - time0) *
                             degMillisToDegMinutes);
                             */
        data.accuracy.emit(dmpData.Quat9.Data.Accuracy);
      }

      if ((dmpData.header & DMP_header_bitmap_Gyro_Calibr) > 0) {
        const double scale = 1.0 / pwrtwo(15);
        double gyroX = (double)dmpData.Gyro_Calibr.Data.X * scale;
        double gyroY = (double)dmpData.Gyro_Calibr.Data.Y * scale;
        double gyroZ = (double)dmpData.Gyro_Calibr.Data.Z * scale;
        mmath::Vector<3, double> gyro(gyroX, gyroY, gyroZ);
      }
    }
  });
}

/**
 * Calculate quaternion for rotation from vector a to vector b
 * http://www.euclideanspace.com/maths/algebra/vectors/angleBetween/
 */
mmath::Quaternion<double> Icm20948::rotationBetweenTwoVectors(
    mmath::Vector<3, double> a, mmath::Vector<3, double> b) {
  double norm = 1.0 / (mmath::Length<double, double>(a) *
                       mmath::Length<double, double>(b));

  auto c = a * b;
  auto d = norm * crossProduct(a, b);

  mmath::Quaternion<double> res;
  res.w = 1.0 + (c.x + c.y + c.z) * norm;
  res.x = d.x;
  res.y = d.y;
  res.z = d.z;
  return res;
}

/**
 * q = cos(a/2) + i ( x * sin(a/2)) + j (y * sin(a/2)) + k ( z * sin(a/2))
 * https://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/index.htm
 */
mmath::Quaternion<double> Icm20948::axisAngleQuaternion(
    mmath::Vector<3, double> axis, double angle) {
  const auto sin2 = sin(angle / 2.);
  return mmath::Quaternion<double>(cos(angle / 2.), axis.x * sin2,
                                   axis.y * sin2, axis.z * sin2);
}

mmath::Vector<3, double> Icm20948::crossProduct(mmath::Vector<3, double> a,
                                                mmath::Vector<3, double> b) {
  return mmath::Vector<3, double>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                                  a.x * b.y - a.y * b.x);
}

mmath::Quaternion<double> Icm20948::conj(mmath::Quaternion<double> q) {
  return mmath::Quaternion<double>(q.w, -q.x, -q.y, -q.z);
}

mmath::Vector<3, double> Icm20948::findGravity() {
  Serial.println(
      "Calibrating gravity vector - keep the sensor still or this might give "
      "inaccurate results");
  // Do exponential moving average over a certain time period and wait until
  // vector settles at around 1g. Takes around 8 seconds. double totalAcc =
  // sqrt(g_x*g_x + g_y*g_y+ g_z*g_z);
  // Average the first xxx readings.DMP seems to take a few seconds to settle
  // down anyhow.
  int counter = 750;
  int i = 0;
  double grav_x = 0, grav_y = 0, grav_z = 0;

  while (i < counter) {
    if (available()) {
      if ((dmpData.header & DMP_header_bitmap_Accel) > 0) {
        grav_x += (float)dmpData.Raw_Accel.Data.X;
        grav_y += (float)dmpData.Raw_Accel.Data.Y;
        grav_z += (float)dmpData.Raw_Accel.Data.Z;
        i++;
        if (i % (counter / 10) == 0) {
          Serial.printf("gravity[%i/%i]: %f %f %f\n", i, counter, grav_x / i,
                        grav_y / i, grav_z / i);
        }
      }
    } else {
      delay(1);
    }
  }

  return mmath::Vector<3, double>(grav_x / i, grav_y / i, grav_z / i);
}

boolean Icm20948::available() {
  // Note:
  //    readDMPdataFromFIFO will return ICM_20948_Stat_FIFONoDataAvail if no
  //    data is available. If data is available, readDMPdataFromFIFO will
  //    attempt to read _one_ frame of DMP data. readDMPdataFromFIFO will
  //    return ICM_20948_Stat_FIFOIncompleteData if a frame was present but
  //    was incomplete readDMPdataFromFIFO will return ICM_20948_Stat_Ok if a
  //    valid frame was read. readDMPdataFromFIFO will return
  //    ICM_20948_Stat_FIFOMoreDataAvail if a valid frame was read _and_ the
  //    FIFO contains more (unread) data.
  icm.readDMPdataFromFIFO(&dmpData);

  return (icm.status == ICM_20948_Stat_Ok) ||
         (icm.status == ICM_20948_Stat_FIFOMoreDataAvail);
}