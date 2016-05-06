#include "one_wire.h"

static one_wire_device one_wire_devices[10];
uint8_t one_wire_device_count = 0;

GPIO_TypeDef *gpio;
uint16_t pin;
TIM_HandleTypeDef *timer;

#define __SET_BIT	 		HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_SET);
#define __RESET_BIT	 	HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_RESET);
#define __READ_BIT		HAL_GPIO_ReadPin(gpio, pin);

one_wire_state state;

// global search state
uint8_t last_mismatch;
uint8_t last_device_flag;
uint8_t crc8;
uint8_t ROM_NO[8];

static unsigned char crc_table[] = {
	0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
	157,195, 33,127,252,162, 64, 30, 95, 1,227,189, 62, 96,130,220,
	35,125,159,193, 66, 28,254,160,225,191, 93, 3,128,222, 60, 98,
	190,224, 2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
	70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89, 7,
	219,133,103, 57,186,228, 6, 88, 25, 71,165,251,120, 38,196,154,
	101, 59,217,135, 4, 90,184,230,167,249, 27, 69,198,152,122, 36,
	248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91, 5,231,185,
	140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
	17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
	175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
	50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
	202,148,118, 40,171,245, 23, 73, 8, 86,180,234,105, 55,213,139,
	87, 9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
	233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
	116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
};


static void one_wire_delay_us(unsigned int time) {
	__HAL_TIM_SET_COUNTER(timer, 0);
	time -= 3;
	while (__HAL_TIM_GET_COUNTER(timer) <= time) {}
}

void one_wire_init(GPIO_TypeDef *g, uint16_t p, TIM_HandleTypeDef *t) 
{
	gpio = g;
	pin = p;
	timer = t;
	state = ONE_WIRE_ERROR;

	// Use PC6 as bus master
//	GPIO_InitTypeDef GPIO_InitStructure;
//	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
//	GPIO_InitStructure.Pin = pin;
//	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
//	HAL_GPIO_Init(gpio, &GPIO_InitStructure);


//	// Setup clock
//	if (timer->Instance == TIM6);
//	//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
//	else
//		while(1){} // not implemented

////	TIM_TimeBaseInitTypeDef TIM_InitStructure;
////	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
////	TIM_InitStructure.TIM_Prescaler = 24 - 1;
////	TIM_InitStructure.TIM_Period = 10000 - 1; // Update event every 10000 us / 10 ms
////	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
////	TIM_InitStructure.TIM_RepetitionCounter = 0;
////	TIM_TimeBaseInit(timer, &TIM_InitStructure);
//////	TIM_ITConfig(timer, TIM_IT_Update, ENABLE);
////	TIM_Cmd(timer, ENABLE);
}

bool one_wire_reset_pulse() 
{
	GPIO_PinState bit;
	// Pull bus down for 500 us (min. 480 us)
	__RESET_BIT
	one_wire_delay_us(500);
	__SET_BIT

	// Wait 70 us, bus should be pulled up by resistor and then
	// pulled down by slave (15-60 us after detecting rising edge)
	one_wire_delay_us(70);

	bit = __READ_BIT
	if (bit == GPIO_PIN_RESET) {
//		GPIO_SetBits(GPIOC, GPIO_Pin_9);
//		GPIO_ResetBits(GPIOC, GPIO_Pin_9);
		__SET_BIT
		__RESET_BIT
		state = ONE_WIRE_SLAVE_PRESENT;
	} else {
		state = ONE_WIRE_ERROR;
		return false;
	}

	// Wait additional 430 us until slave keeps bus down (total 500 us, min. 480 us)
	one_wire_delay_us(430);
	return true;
}

void one_wire_write_1() {
	// Pull bus down for 15 us
//	GPIO_ResetBits(gpio, pin);
	__RESET_BIT
		one_wire_delay_us(15);
//	GPIO_SetBits(gpio, pin);
	__SET_BIT

	// Wait until end of timeslot (60 us) + 5 us for recovery
	one_wire_delay_us(50);
}

void one_wire_write_0() {
	// Pull bus down for 60 us
//	GPIO_ResetBits(gpio, pin);
	__RESET_BIT
	one_wire_delay_us(60);
//	GPIO_SetBits(gpio, pin);
	__SET_BIT
	// Wait until end of timeslot (60 us) + 5 us for recovery
	one_wire_delay_us(5);
}

void one_wire_write_bit(bool bit) 
{
	
	if (bit) {
		one_wire_write_1();
	} else {
		one_wire_write_0();
	}
}

bool one_wire_read_bit()
{
	static GPIO_PinState bit; 
	// Pull bus down for 5 us
__RESET_BIT
	one_wire_delay_us(5);
__SET_BIT
	// Wait 5 us and check bus state
	one_wire_delay_us(5);

//	static BitAction bit;
//	bit = GPIO_ReadInputDataBit(gpio, pin);
	bit = __READ_BIT
	
//	GPIO_WriteBit(GPIOC, GPIO_Pin_9, bit);
	if(bit) __SET_BIT
		else __RESET_BIT
	
	// Wait until end of timeslot (60 us) + 5 us for recovery
	one_wire_delay_us(55);

	if (bit == GPIO_PIN_SET) {
		return true;
	} else {
		return false;
	}
}

void one_wire_write_byte(uint8_t data) {
	uint8_t i;
	for (i = 0; i < 8; ++i) {
		if ((data >> i) & 1) {
			one_wire_write_1();
		} else {
			one_wire_write_0();
		}
	}
}

uint8_t one_wire_read_byte() {
	uint8_t i;
	uint8_t data = 0;
	bool bit;
	for (i = 0; i < 8; ++i) {
		bit = one_wire_read_bit();
		data |= bit << i;
	}
	return data;
}

void one_wire_reset_crc() {
	crc8 = 0;
}

uint8_t one_wire_get_crc() {
	return crc8;
}

uint8_t one_wire_crc(uint8_t data) {
	crc8 = crc_table[crc8 ^ data];
	return crc8;
}

void one_wire_read_rom() {
	one_wire_reset_pulse();
	one_wire_write_byte(0x33);
	one_wire_read_byte();
	one_wire_read_byte();
	one_wire_read_byte();
	one_wire_read_byte();
	one_wire_read_byte();
	one_wire_read_byte();
	one_wire_read_byte();
	one_wire_read_byte();
}



int one_wire_search()
{
	uint8_t id_bit_number;
	int last_zero, rom_byte_number, search_result;
	bool id_bit, cmp_id_bit;
	uint8_t rom_byte_mask, search_direction;

	// initialize for search
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = 0;
	crc8 = 0;
	// if the last call was not the last one
	if (!last_device_flag)
	{
		// 1-Wire reset
		if (!one_wire_reset_pulse()) {
			// reset the search
			last_mismatch = 0;
			last_device_flag = false;
			return false;
		}
		// issue the search command
		one_wire_write_byte(0xF0);
		// loop to do the search
		do
		{
			// read a bit and its complement
			id_bit = one_wire_read_bit();
			cmp_id_bit = one_wire_read_bit();
			// check for no devices on 1-wire
			if ((id_bit == 1) && (cmp_id_bit == 1))
				break; // no devices

			// all devices coupled have 0 or 1
			if (id_bit != cmp_id_bit)
				search_direction = id_bit; // bit write value for search
			else
			{
				// if this discrepancy if before the Last Discrepancy
				// on a previous next then pick the same as last time
				if (id_bit_number < last_mismatch)
					search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
				else
					// if equal to last pick 1, if not then pick 0
					search_direction = (id_bit_number == last_mismatch);
				// if 0 was picked then record its position in LastZero
				if (search_direction == 0)
				{
					last_zero = id_bit_number;
				}
			}
			// set or clear the bit in the ROM byte rom_byte_number
			// with mask rom_byte_mask
			if (search_direction == 1)
				ROM_NO[rom_byte_number] |= rom_byte_mask;
			else
				ROM_NO[rom_byte_number] &= ~rom_byte_mask;
			// serial number search direction write bit
			one_wire_write_bit(search_direction);
			// increment the byte counter id_bit_number
			// and shift the mask rom_byte_mask
			id_bit_number++;
			rom_byte_mask <<= 1;
			// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
			if (rom_byte_mask == 0)
			{
				one_wire_crc(ROM_NO[rom_byte_number]); // accumulate the CRC
				rom_byte_number++;
				rom_byte_mask = 1;
			}

		}
		while(rom_byte_number < 8); // loop until through all ROM bytes 0-7
		// if the search was successful then
		if (!((id_bit_number < 65) || (crc8 != 0)))
		{
			// search successful so set last_mismatch, last_device_flag, search_result
			last_mismatch = last_zero;
			// check for last device
			if (last_mismatch == 0)
				last_device_flag = true;

			search_result = true;
		}
	}
	// if no device found then reset counters so next 'search' will be like a first
	if (!search_result || !ROM_NO[0])
	{
		last_mismatch = 0;
		last_device_flag = false;
		search_result = false;
	}
	return search_result;
}

bool one_wire_match_rom(one_wire_device device) {
	int i;
	one_wire_reset_pulse();
	one_wire_write_byte(0x55); // Match ROM command
	for (i = 0; i < 8; ++i) {
		one_wire_write_byte(device.address[i]);
	}

	return false;
}

int one_wire_first()
{
	// reset the search state
	last_mismatch = 0;
	last_device_flag = false;
	return one_wire_search();
}

int one_wire_next()
{
	// leave the search state alone
	return one_wire_search();
}

one_wire_device* one_wire_search_rom(uint8_t *devices) {
	int result, i;
	int count = 0;
	result = one_wire_first();
	while (result)
	{
		one_wire_device device;
		// print device found - CRC, ID, Family
		for (i = 7; i >= 0; i--) {
			printf("%02X", ROM_NO[i]);
		}

		for (i = 0; i < 8; ++i) {
			device.address[i] = ROM_NO[i];
		}

		one_wire_devices[one_wire_device_count++] = device;

		printf(" %d\r\n", ++count);
		result = one_wire_next();
	}
	*devices = one_wire_device_count;
	return one_wire_devices;
}
