/*
 * I2C_APDS9900.cpp
 *
 *  
 */


#include "I2C_APDS9900.h"
#include <cmath>

I2C_APDS9900::I2C_APDS9900() 
{

}

bool I2C_APDS9900::begin(uint8_t bus, uint8_t i2cAddress) 
{
  _i2cAddress = i2cAddress;
  
  if(initI2C_RW(bus, i2cAddress, 0) > 0)
    return false;

  writeRegister(APDS9900_ENABLE, 0x00); // disable and power down
  writeRegister(APDS9900_CONTROL, 0x20); // diode select
  writeRegister(APDS9900_ATIME, 0xFF); // 2.7 ms – minimum ALS integration time
  writeRegister(APDS9900_PTIME, 0xFF); // 2.7 ms – minimum Wait time
  writeRegister(APDS9900_WTIME, 0xFF); // 2.7 ms – minimum Prox integration time
  writeRegister(APDS9900_PPCOUNT, 0x01); // Minimum prox pulse count
  writeRegister(APDS9900_ENABLE, 0x0F); // enable and power-on

  //delay(100);
  
  uint8_t c = readRegisterByte(APDS9900_ENABLE);
  
  if (c != 0x0F) {
    rt_printf("APDS9900 read 0x%x instead of 0x0F\n", c);
    return false;
  }

  initialized = true;
  return true;
}

uint8_t I2C_APDS9900::readRegisterByte(uint8_t reg)
{
    i2c_char_t inbuf, outbuf;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    /*
     * In order to read a register, we first do a "dummy write" by writing
     * 0 bytes to the register we want to read from.  This is similar to
     * the packet in set_i2c_register, except it's 1 byte rather than 2.
     */
    outbuf = (0x80 | reg);
    messages[0].addr  = APDS9900_I2CADDR_DEFAULT;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = &outbuf;

    /* The data will get returned in this structure */
    messages[1].addr  = APDS9900_I2CADDR_DEFAULT;
    messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
    messages[1].len   = sizeof(inbuf);
    messages[1].buf   = &inbuf;

    /* Send the request to the kernel and get the result back */
    packets.msgs      = messages;
    packets.nmsgs     = 2;
    if(ioctl(i2C_file, I2C_RDWR, &packets) < 0)
    {
        rt_printf("Unable to send data\n");
        return 0;
    }

    return inbuf;
}

bool I2C_APDS9900::readRegisterWord(uint8_t reg, uint16_t *out) {
    i2c_char_t inbuf[2], outbuf;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    /*
     * In order to read a register, we first do a "dummy write" by writing
     * 0 bytes to the register we want to read from.  This is similar to
     * the packet in set_i2c_register, except it's 1 byte rather than 2.
     */
    outbuf = (0xA0 | reg);
    messages[0].addr  = _i2cAddress;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = &outbuf;

    /* The data will get returned in this structure */
    messages[1].addr  = _i2cAddress;
    messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
    messages[1].len   = sizeof(inbuf);
    messages[1].buf   = inbuf;

    /* Send the request to the kernel and get the result back */
    packets.msgs      = messages;
    packets.nmsgs     = 2;
    
    if(ioctl(i2C_file, I2C_RDWR, &packets) < 0)
    {
        return false;
    }

    *out = (uint16_t)inbuf[0] | (((uint16_t)inbuf[1]) << 8);
    return true;
}

/**************************************************************************/
/*!
    @brief  Writes 8-bits to the specified destination register
*/
/**************************************************************************/
void I2C_APDS9900::writeRegister(uint8_t reg, uint8_t value) {
  uint8_t buf[2] = { (uint8_t)(0x80 | reg), value };

  if(write(i2C_file, buf, 2) != 2)
  {
    cout << "Failed to write register " << (int)reg << " on APDS9900\n";
    return;
  }
}

bool I2C_APDS9900::readProximity(float *out)
{
  uint16_t val;
  float r = 27;
  
  if (readRegisterWord(APDS9900_PDATAL, &val))
  {
    r = dampening * lastProximity + (1.0 - dampening) * val;
    lastProximity = r;
  }
  else
  {
    return false;
    r = lastProximity;
  }


 	if (val < minValue)
 	{
 		minValue = val;
 	}
 	else if (val > maxValue)
 	{
 		maxValue = val;
 	}

  // Compute distance from measurement
  float x = logf((r - minValue) / 3199.609003) / -0.469204;
  *out = r;
  return true;
}