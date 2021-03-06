#ifndef __DS18B20_H__
#define __DS18B20_H__
#include "one_wire.h"
#include "math.h"
#include <stdlib.h>
#include <stdbool.h>

// Structure in which temperature is stored
typedef struct {
    int8_t integer;
    uint16_t fractional;
    bool is_valid;
} simple_float;

// Structure for returning list of devices on one wire
typedef struct {
    uint8_t size;
    one_wire_device *devices;
} ds18b20_devices;

void ds18b20_init(GPIO_TypeDef *gpio, uint16_t port);
void ds18b20_set_precission(uint8_t precission);
ds18b20_devices ds18b20_get_devices(bool scan);

void ds18b20_convert_temperature_simple(void);
simple_float ds18b20_read_temperature_simple(void);

void ds18b20_convert_temperature_all(void);
simple_float* ds18b20_read_temperature_all(void);

void ds18b20_wait_for_conversion(void);
simple_float ds18b20_decode_temperature(void);

extern simple_float ds18b20_GetTemp1(void);
extern simple_float ds18b20_GetTemp2(void);

#endif // __DS18B20_H__
