#include "icm20948.h"

Icm20948::Icm20948() {
    eulerAngles.yaw = 0.0;
    eulerAngles.time = millis();
}

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

  // Configuring DMP to output data at multiple ODRs:
  // DMP is capable of outputting multiple sensor data at different rates to
  // FIFO. Setting value can be calculated as follows: Value = (DMP running rate
  // / ODR ) - 1 E.g. For a 5Hz ODR rate when DMP is running at 55Hz, value =
  // (55/5) - 1 = 10.
  success &= (icm.setDMPODRrate(DMP_ODR_Reg_Quat9, 0) ==
              ICM_20948_Stat_Ok);  // Set to the maximum

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

  app.onTick([this]() {
    while (available()) {
      if ((dmpData.header & DMP_header_bitmap_Quat9) >
          0)  // We have asked for orientation data so we should receive Quat9
      {
        // Q0 value is computed from this equation: Q0^2 + Q1^2 + Q2^2 + Q3^2
        // = 1. In case of drift, the sum will not add to 1, therefore,
        // quaternion data need to be corrected with right bias values. The
        // quaternion data is scaled by 2^30.

        // SERIAL_PORT.printf("Quat9 data is: Q1:%ld Q2:%ld Q3:%ld
        // Accuracy:%d\r\n", data.Quat9.Data.Q1, data.Quat9.Data.Q2,
        // data.Quat9.Data.Q3, data.Quat9.Data.Accuracy);

        // Scale to +/- 1
        double q1 = ((double)dmpData.Quat9.Data.Q1) /
                    1073741824.0;  // Convert to double. Divide by 2^30
        double q2 = ((double)dmpData.Quat9.Data.Q2) /
                    1073741824.0;  // Convert to double. Divide by 2^30
        double q3 = ((double)dmpData.Quat9.Data.Q3) /
                    1073741824.0;  // Convert to double. Divide by 2^30
        double q0 = -sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));

        Quaternion quaternion;
        quaternion.w = q0;
        quaternion.x = q1;
        quaternion.y = q2;
        quaternion.z = q3;

        double yaw0 = eulerAngles.yaw;
        double time0 = eulerAngles.time;

        eulerAngles = toEuler(quaternion);
        eulerAngles.time = millis();

        data.pitch.emit(-eulerAngles.pitch);
        data.roll.emit(eulerAngles.roll);
        data.yaw.emit(eulerAngles.yaw);

        const double degMillisToDegMinutes = 1.0/60000.;
        data.rateOfTurn.emit((eulerAngles.yaw - yaw0)/(eulerAngles.time - time0)*degMillisToDegMinutes);
        data.accuracy.emit(dmpData.Quat9.Data.Accuracy);
      }
    }
  });
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

EulerAngles Icm20948::toEuler(Quaternion q) {
  EulerAngles ret;
  double sqw = q.w * q.w;
  double sqx = q.x * q.x;
  double sqy = q.y * q.y;
  double sqz = q.z * q.z;

  ret.yaw =
      atan2(2.0 * (q.x * q.y + q.z * q.w), (sqx - sqy - sqz + sqw)) * RAD2DEG;
  ret.pitch =
      atan2(2.0 * (q.y * q.z + q.x * q.w), (-sqx - sqy + sqz + sqw)) * RAD2DEG;
  double siny = -2.0 * (q.x * q.z - q.y * q.w) / (sqx + sqy + sqz + sqw);

  // prevent NaN and use 90 deg when out of range
  if (fabs(siny) >= 1.0)
    ret.roll = copysign(90.0, siny);
  else
    ret.roll = asin(siny) * RAD2DEG;
  return ret;
}