#ifndef __OBD_PID_H_
#define __OBD_PID_H_

#define OBD2_PID_ENGINE_RPM 0x0C
#define OBD2_PID_VEHICLE_SPEED 0x0D
#define OBD2_PID_OIL_TEMP 0x5C
#define OBD2_PID_FUEL_LEVEL 0x2F

#define OBD2_PID_ENGINE_RPM_CONV(a, b) ((((uint16_t)(a) * 256) + (b)) / 4)
#define OBD2_PID_VEHICLE_SPEED_CONV(a) (a)
#define OBD2_PID_OIL_TEMP_CONV(a) ((a) - 40)
#define OBD2_PID_FUEL_LEVEL_CONV(a) ((uint16_t)(a) * 100 / 255)

#endif