#include "Icm20948.h"

Icm20948::Icm20948(String config_path) : Sensor(config_path) {
  eulerAngles.yaw = 0.0;
  heading_offset_ = 274. - 360 - 16.;
  calibrate_ = false;
  load_configuration();
}

void Icm20948::enable() {
  debugI("Enabling icm20948!");

  WIRE_PORT.begin();
  WIRE_PORT.setClock(400000);

  // Uncomment this line to enable helpful debug messages on Serial
  // icm.enableDebugging();

  bool success = true;
  // Initialize the ICM-20948
  // If the DMP is enabled, .begin performs a minimal startup. We need to
  // configure the sample mode etc. manually.
  icm.begin(WIRE_PORT, 1);
  if (icm.status != ICM_20948_Stat_Ok) {
    debugE("Initialization of the icm20948 failed with: %s",
           icm.statusString());
    success = false;
  } else {
    debugI("Device connected!");
  }

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
    debugI("DMP enabled!");
  } else {
    debugE("Enable DMP failed!");
    debugE(
        "Please check that you have uncommented line 29 (#define "
        "ICM_20948_USE_DMP) in ICM_20948_C.h...");
  }

  if (success) {
    gravity_ = findGravity();
    debugI("Found gravity: %f, %f, %f", gravity_.x, gravity_.y, gravity_.z);
    auto down = mmath::Vector<3, double>(0., 0., 1.);
    auto rotateToGravity = rotationBetweenTwoVectors(gravity_, down);
    mmath::Quaternion<double> correctToNorth(
        down, mmath::Angles::DegToRad(heading_offset_));
    debugD("rotateToGravity quaternion: %f %f %f %f", rotateToGravity.w,
           rotateToGravity.x, rotateToGravity.y, rotateToGravity.z);
    debugD("correctToNorth quaternion: %f %f %f %f", correctToNorth.w,
           correctToNorth.x, correctToNorth.y, correctToNorth.z);

    calibration = correctToNorth * rotateToGravity;
    // calibration = rotateToGravity;
    calibration = calibration / calibration.norm();
    debugD("calibration quaternion: %f %f %f %f", calibration.w, calibration.x,
           calibration.y, calibration.z);

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

          // auto quat_calibrated = calibration * quaternion *
          // conj(calibration);
          auto quat_calibrated = calibration * quaternion;

          auto euler = quat_calibrated.ToEulerXYZ();

          data.pitch.emit(mmath::Angles::RadToDeg(-euler.x));
          data.roll.emit(mmath::Angles::RadToDeg(euler.y));
          data.yaw.emit(mmath::Angles::RadToDeg(euler.z));

          data.accuracy.emit(dmpData.Quat9.Data.Accuracy);
        }

        if ((dmpData.header & DMP_header_bitmap_Gyro_Calibr) > 0) {
          const double scale = 1.0 / pwrtwo(15);
          double gyroX = (double)dmpData.Gyro_Calibr.Data.X * scale;
          double gyroY = (double)dmpData.Gyro_Calibr.Data.Y * scale;
          double gyroZ = (double)dmpData.Gyro_Calibr.Data.Z * scale;
          // mmath::Vector<3, double> gyro(gyroX, gyroY, gyroZ);
          // auto gyro_calibrated = calibration * gyro * conj(calibration);

          double w = -sqrt(
              1.0 - ((gyroX * gyroX) + (gyroY * gyroY) + (gyroZ * gyroZ)));
          mmath::Quaternion<double> gyro(w, gyroX, gyroY, gyroZ);

          auto gyro_calibrated = calibration * gyro;
          auto gyro_euler = gyro_calibrated.ToEulerXYZ();

          // const double degMillisToDegMinutes = 60000.;
          data.pitch_rate.emit(mmath::Angles::RadToDeg(-gyro_euler.x));
          data.roll_rate.emit(mmath::Angles::RadToDeg(gyro_euler.y));
          data.yaw_rate.emit(mmath::Angles::RadToDeg(gyro_euler.z));
        }
      }
    });
  }
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

mmath::Vector<3, double> Icm20948::crossProduct(mmath::Vector<3, double> a,
                                                mmath::Vector<3, double> b) {
  return mmath::Vector<3, double>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                                  a.x * b.y - a.y * b.x);
}

mmath::Quaternion<double> Icm20948::conj(mmath::Quaternion<double> q) {
  return mmath::Quaternion<double>(q.w, -q.x, -q.y, -q.z);
}

mmath::Vector<3, double> Icm20948::findGravity() {
  debugI(
      "Calibrating gravity vector - keep the sensor still or this might give "
      "inaccurate results");
  // Do exponential moving average over a certain time period and wait until
  // vector settles at around 1g. Takes around 8 seconds. double totalAcc =
  // sqrt(g_x*g_x + g_y*g_y+ g_z*g_z);
  // Average the first xxx readings.DMP seems to take a few seconds to settle
  // down anyhow.
  int counter = 250;
  int i = 0;
  mmath::Vector<3, double> mean(0., 0., 0.), m2(0., 0., 0), delta1, delta2,
      grav;

  while (i < counter) {
    if (available()) {
      if ((dmpData.header & DMP_header_bitmap_Accel) > 0) {
        grav.x = (float)dmpData.Raw_Accel.Data.X;
        grav.y = (float)dmpData.Raw_Accel.Data.Y;
        grav.z = (float)dmpData.Raw_Accel.Data.Z;
        i++;
        delta1 = grav - mean;
        mean += delta1 / (double)i;
        delta2 = grav - mean;
        m2 += delta1 * delta2;
        if (i % (counter / 10) == 0) {
          debugD("gravity[%i/%i]: %f +- %f, %f +- %f, %f +- %f", i, counter,
                 mean.x, sqrt(m2.x / i), mean.y, sqrt(m2.y / i), mean.z,
                 sqrt(m2.z / i));
        }
      }
    } else {
      delay(1);
    }
  }

  return mean;
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

void Icm20948::get_configuration(JsonObject& root) {
  root["heading_offset"] = heading_offset_;
  root["calibrate"] = calibrate_;
}

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "heading_offset": { "title": "Heading offset", "type": "number", "description": "The offset of the heading, because the sensor is not aligned to the boat axis." },
        "calibrate" : { "title" : "Run calibration and find gravity.", "type": "boolean", "description": "Run the calibration for gravity again. The sensor should be stationary."}
    }
  })###";

String Icm20948::get_config_schema() { return FPSTR(SCHEMA); }

bool Icm20948::set_configuration(const JsonObject& config) {
  String expected[] = {"heading_offset", "calibrate"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  heading_offset_ = config["heading_offset"];
  calibrate_ = config["calibrate"];
  if(calibrate_) {
    debugI("Calibrate now!");
    calibrate_ = false; 
  }
  return true;
}