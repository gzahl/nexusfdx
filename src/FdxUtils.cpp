#include "FdxUtils.h"

namespace FdxUtils {
    float toFloat16(const int* data) {
        return float((data[0] + 256 * data[1])) / 100;
    }
    
    float toDegrees16(const int* data) {
        return float((data[0] + 256 * data[1])) * DEGREES_SCALE;
    }
    
    float toSignedDegrees16(const int* data) {
        float angle = toDegrees16(data);
        return angle > 180 ? -(360 - angle) : angle;
    }

    float readSpeed(const int* data) {
        return toFloat16(data);
    }

    float readAngle(const int* data) {
        return toSignedDegrees16(data);
    }
}