#ifndef FDX_UTILS_H
#define FDX_UTILS_H

namespace FdxUtils {
    static constexpr float DEGREES_SCALE = 0.005493164f; // 360/65536
    
    float toFloat16(const int* data);
    float toDegrees16(const int* data);
    float toSignedDegrees16(const int* data);
    
    // Common message type readers
    float readSpeed(const int* data);
    float readAngle(const int* data);
}

#endif