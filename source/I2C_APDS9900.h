/*
 * APDS9900 proximity/ALS sensor driver
 * 
 * Based on Andrew McPherson's driver for the MPR121 capacitive touch sensor
 * Based on Adafruit library by Limor Fried/Ladyada
 */

#ifndef I2CTK_H_
#define I2CTK_H_

#include <I2c.h>
#include "Utilities.h"

// typedef bool boolean;

#define APDS9900_I2CADDR_DEFAULT 0x39
#define APDS9900_ENABLE 0x00
#define APDS9900_CONTROL 0x0F
#define APDS9900_ATIME 0x01
#define APDS9900_PTIME 0x02
#define APDS9900_WTIME 0x03
#define APDS9900_PPCOUNT 0x0E
#define APDS9900_PDATAL 0x18

class I2C_APDS9900 : public I2c
{
public:
	// Hardware I2C
	I2C_APDS9900();

	bool begin(uint8_t bus = 1, uint8_t i2cAddress = APDS9900_I2CADDR_DEFAULT);
	
	bool isInitialized() { return initialized; };
	
	bool readProximity(float *out);
	
private:

	uint8_t readRegisterByte(uint8_t reg);
	bool readRegisterWord(uint8_t reg, uint16_t *out);
	void writeRegister(uint8_t reg, uint8_t value);
	int readI2C() { return 0; } // Unused
	
	float dampening = 0.0;
	float lastProximity = 0;
	
	float minValue = 1023;
	float maxValue = 0;
	

	bool initialized = false;
	int _i2cAddress;
};


#endif /* I2CTK_H_ */
