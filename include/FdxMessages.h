#ifndef FDX_MESSAGES_H
#define FDX_MESSAGES_H

#include <unordered_map>
#include "FdxReader.h"
#include "FdxUtils.h"

namespace FdxMessages {
    const std::unordered_map<uint8_t, FdxMessage> messageMap = {
        // BSP - Boat Speed
        {0, {4, {{"BSP", "kn", FdxUtils::readSpeed}}}},
        
        // AWS/AWA - Apparent Wind
        {1, {6, {
            {"AWS", "m/s", FdxUtils::readSpeed},
            {"AWA", "°", [](const int* data) { return FdxUtils::readAngle(data + 2); }}
        }}},
        
        // HDC - Heading
        {2, {5, {{"HDC", "°", FdxUtils::readAngle}}}},
        
        // DBT - Depth
        {7, {5, {{"DBT", "m", FdxUtils::readSpeed}}}},
        
        // TMP - Temperature
        {8, {3, {{"TMP", "°C", [](const int* data) { return float(data[0]); }}}}},
        
        // BAT - Battery
        {9, {3, {{"BAT", "V", [](const int* data) { return float(data[0]) / 10; }}}}},
        
        // VMG - Velocity Made Good
        {17, {4, {{"VMG", "kn", [](const int* data) {
            int vmg_100 = data[0] + 256 * data[1];
            return vmg_100 > 32767 ? -float((vmg_100 ^ 65535) + 1) / 100 : float(vmg_100 + 1) / 100;
        }}}}},
        
        // TWS/TWA - True Wind
        {18, {6, {
            {"TWS", "m/s", FdxUtils::readSpeed},
            {"TWA", "°", [](const int* data) { return FdxUtils::readAngle(data + 2); }}
        }}},
        
        // SOG/COG - GPS Speed and Course
        {33, {6, {
            {"SOG", "kn", FdxUtils::readSpeed},
            {"COG", "°", [](const int* data) { return FdxUtils::toDegrees16(data + 2); }}
        }}},
        
        // BTW/DTW - Bearing and Distance to Waypoint
        {34, {9, {
            {"BTW", "°", FdxUtils::readAngle},
            {"DTW", "nm", [](const int* data) { 
                return float((data[4] + 256 * data[5] + 256 * 256 * data[6])) / 1000;
            }}
        }}},
        
        // TWD - True Wind Direction
        {98, {4, {{"TWD", "°", FdxUtils::readAngle}}}}
    };
}

#endif