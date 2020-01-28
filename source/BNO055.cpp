/***************************************************************************
  This is a library for the BNO055 orientation sensor
  Designed specifically to work with the Adafruit BNO055 Breakout.
  Pick one up today in the adafruit shop!

  ------> http://www.adafruit.com/products

  These sensors use I2C to communicate, 2 pins are required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by KTOWN for Adafruit Industries.
  Modified for operation on a linux platform by William Gerhard

  MIT license, all text above must be included in any redistribution
 ***************************************************************************/

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <stdio.h>      /* Standard I/O functions */
#include <fcntl.h>
#include <syslog.h>		/* Syslog functionallity */
#include <inttypes.h>
#include <errno.h>
#include <math.h>

#include "BNO055.h"

//! Constructor takes bus and address arguments
/*!
 \param bus the bus to use in /dev/i2c-%d.
 \param address the device address on bus
 */
BNO055::BNO055(int32_t sensorID, int address, int bus)
{
	i2c = new I2C(bus,address);
  _sensorID = sensorID;
  _address = address;
  delete i2c;

	i2c = new I2C(bus,address);
  _sensorID = sensorID;
  _address = address;
	

}

BNO055::~BNO055() {
	delete i2c;
}



bool BNO055::begin(bno055_opmode_t mode)
{

	setMode(OPERATION_MODE_CONFIG);
	i2c->write_byte(BNO055_PAGE_ID_ADDR, 0);

  /* Make sure we have the right device */
  uint8_t id = i2c->read_byte(BNO055_CHIP_ID_ADDR);
  if(id != BNO055_ID)
  {
    
    id = i2c->read_byte(BNO055_CHIP_ID_ADDR);
    if(id != BNO055_ID) {
    	rt_printf("Unexpected device ID (expected %i, got %i)\n", BNO055_ID, id);
      return false;  // still not? ok bail
    }
  }

  /* Switch to config mode (just in case since this is the default) */
  setMode(OPERATION_MODE_CONFIG);

  i2c->write_byte(BNO055_SYS_TRIGGER_ADDR, 0x20);
  
  int count = 0;
  
  while (i2c->read_byte(BNO055_CHIP_ID_ADDR) != BNO055_ID)
  {
    rt_printf("Waiting for chip to reset\n");
    usleep((int)(0.1 * 10e6));
    
    count++;
    
    if (count >= 10)
    {
    	rt_printf("Failed to read ID after reset\n");
    	return false;
    }
  }
  
  usleep(50000);

  i2c->write_byte(BNO055_PAGE_ID_ADDR, 0);

  // Trigger self test
  i2c->write_byte(BNO055_SYS_TRIGGER_ADDR, 0x1);
  usleep(50000);

  uint8_t status, result, error = 0;

    getSystemStatus(&status, &result, &error);
    rt_printf("Status: %i %i %i\n", status, result & 0x0F, error);
  
  if (error != 0)
  {
  	rt_printf("Self test failed\n");
    return false;
  }
    
  i2c->write_byte(BNO055_SYS_TRIGGER_ADDR, 0x0);
  
    /* Set to normal power mode */
  i2c->write_byte(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);
  
  /* Set the requested operating mode (see section 3.3) */
  setMode(mode);
 
  return true;
}



void BNO055::setMode(bno055_opmode_t mode)
{
  _mode = mode;
  i2c->write_byte(BNO055_OPR_MODE_ADDR, _mode);
  usleep(30000);
}


void BNO055::setExtCrystalUse(bool usextal)
{
  bno055_opmode_t modeback = _mode;

  /* Switch to config mode (just in case since this is the default) */
  setMode(OPERATION_MODE_CONFIG);
  
  i2c->write_byte(BNO055_PAGE_ID_ADDR, 0);
  if (usextal) {
    i2c->write_byte(BNO055_SYS_TRIGGER_ADDR, 0x80);
  } else {
    i2c->write_byte(BNO055_SYS_TRIGGER_ADDR, 0x00);
  }

  /* Set the requested operating mode (see section 3.3) */
  setMode(modeback);

}


void BNO055::getSystemStatus(uint8_t *system_status, uint8_t *self_test_result, uint8_t *system_error)
{
  i2c->write_byte(BNO055_PAGE_ID_ADDR, 0);

  /* System Status (see section 4.3.58)
     ---------------------------------
     0 = Idle
     1 = System Error
     2 = Initializing Peripherals
     3 = System Iniitalization
     4 = Executing Self-Test
     5 = Sensor fusio algorithm running
     6 = System running without fusion algorithms */

  if (system_status != 0)
    *system_status    = i2c->read_byte(BNO055_SYS_STAT_ADDR);

  /* Self Test Results (see section )
     --------------------------------
     1 = test passed, 0 = test failed
     Bit 0 = Accelerometer self test
     Bit 1 = Magnetometer self test
     Bit 2 = Gyroscope self test
     Bit 3 = MCU self test
     0x0F = all good! */

  if (self_test_result != 0)
    *self_test_result = i2c->read_byte(BNO055_SELFTEST_RESULT_ADDR);

  /* System Error (see section 4.3.59)
     ---------------------------------
     0 = No error
     1 = Peripheral initialization error
     2 = System initialization error
     3 = Self test result failed
     4 = Register map value out of range
     5 = Register map address out of range
     6 = Register map write error
     7 = BNO low power mode not available for selected operat ion mode
     8 = Accelerometer power mode not available
     9 = Fusion algorithm configuration error
     A = Sensor configuration error */

  if (system_error != 0)
    *system_error     = i2c->read_byte(BNO055_SYS_ERR_ADDR);

  
}


void BNO055::getRevInfo(bno055_rev_info_t* info)
{
  uint8_t a, b;

  memset(info, 0, sizeof(bno055_rev_info_t));

  /* Check the accelerometer revision */
  info->accel_rev = i2c->read_byte(BNO055_ACCEL_REV_ID_ADDR);

  /* Check the magnetometer revision */
  info->mag_rev   = i2c->read_byte(BNO055_MAG_REV_ID_ADDR);

  /* Check the gyroscope revision */
  info->gyro_rev  = i2c->read_byte(BNO055_GYRO_REV_ID_ADDR);

  /* Check the SW revision */
  info->bl_rev    = i2c->read_byte(BNO055_BL_REV_ID_ADDR);

  a = i2c->read_byte(BNO055_SW_REV_ID_LSB_ADDR);
  b = i2c->read_byte(BNO055_SW_REV_ID_MSB_ADDR);
  info->sw_rev = (((uint16_t)b) << 8) | ((uint16_t)a);
}




void BNO055::getCalibration(uint8_t* sys, uint8_t* gyro, uint8_t* accel, uint8_t* mag) {
  uint8_t calData =  i2c->read_byte(BNO055_CALIB_STAT_ADDR);
  if (sys != NULL) {
    *sys = (calData >> 6) & 0x03;
  }
  if (gyro != NULL) {
    *gyro = (calData >> 4) & 0x03;
  }
  if (accel != NULL) {
    *accel = (calData >> 2) & 0x03;
  }
  if (mag != NULL) {
    *mag = calData & 0x03;
  }
}


int8_t BNO055::getTemp(void)
{
  int8_t temp = (int8_t)(i2c->read_byte(BNO055_TEMP_ADDR));
  return temp;
}


imu::Vector<3> BNO055::getVector(vector_type_t vector_type)
{
  imu::Vector<3> xyz;
  uint8_t buffer[6];
  memset (buffer, 0, 6);

  int16_t x, y, z;
  x = y = z = 0;

  /* Read vector data (6 bytes) */
  i2c->read_length((bno055_reg_t)vector_type, 6, buffer);

  x = ((int16_t)buffer[0]) | (((int16_t)buffer[1]) << 8);
  y = ((int16_t)buffer[2]) | (((int16_t)buffer[3]) << 8);
  z = ((int16_t)buffer[4]) | (((int16_t)buffer[5]) << 8);

  /* Convert the value to an appropriate range (section 3.6.4) */
  /* and assign the value to the Vector type */
  switch(vector_type)
  {
    case VECTOR_MAGNETOMETER:
      /* 1uT = 16 LSB */
      xyz[0] = ((double)x)/16.0;
      xyz[1] = ((double)y)/16.0;
      xyz[2] = ((double)z)/16.0;
      break;
    case VECTOR_GYROSCOPE:
      /* 1dps = 16 LSB */
      xyz[0] = ((double)x)/16.0;
      xyz[1] = ((double)y)/16.0;
      xyz[2] = ((double)z)/16.0;
      break;
    case VECTOR_EULER:
      /* 1 degree = 16 LSB */
      xyz[0] = ((double)x)/16.0;
      xyz[1] = ((double)y)/16.0;
      xyz[2] = ((double)z)/16.0;
      break;
    case VECTOR_ACCELEROMETER:
    case VECTOR_LINEARACCEL:
    case VECTOR_GRAVITY:
      /* 1m/s^2 = 100 LSB */
      xyz[0] = ((double)x)/100.0;
      xyz[1] = ((double)y)/100.0;
      xyz[2] = ((double)z)/100.0;
      break;
  }

  return xyz;
}


imu::Quaternion BNO055::getQuat(void)
{
  uint8_t buffer[8];
  memset (buffer, 0, 8);

  int16_t x, y, z, w;
  x = y = z = w = 0;

  /* Read quat data (8 bytes) */
  i2c->read_length(BNO055_QUATERNION_DATA_W_LSB_ADDR, 8, buffer);
  w = (((uint16_t)buffer[1]) << 8) | ((uint16_t)buffer[0]);
  x = (((uint16_t)buffer[3]) << 8) | ((uint16_t)buffer[2]);
  y = (((uint16_t)buffer[5]) << 8) | ((uint16_t)buffer[4]);
  z = (((uint16_t)buffer[7]) << 8) | ((uint16_t)buffer[6]);

  /* Assign to Quaternion */
  /* See http://ae-bst.resource.bosch.com/media/products/dokumente/bno055/BST_BNO055_DS000_12~1.pdf
     3.6.5.5 Orientation (Quaternion)  */
  const double scale = (1.0 / (1<<14));
  imu::Quaternion quat(scale * w, scale * x, scale * y, scale * z);

  return quat;
}


bool BNO055::getSensorOffsets(uint8_t* calibData)
{
    if (isFullyCalibrated())
    {
        bno055_opmode_t lastMode = _mode;
        setMode(OPERATION_MODE_CONFIG);

        i2c->read_length(ACCEL_OFFSET_X_LSB_ADDR, NUM_BNO055_OFFSET_REGISTERS, calibData);

        setMode(lastMode);
        return true;
    }
    return false;
}


bool BNO055::getSensorOffsets(bno055_offsets_t &offsets_type)
{
    if (isFullyCalibrated())
    {
        bno055_opmode_t lastMode = _mode;
        setMode(OPERATION_MODE_CONFIG);
        

        offsets_type.accel_offset_x = (i2c->read_byte(ACCEL_OFFSET_X_MSB_ADDR) << 8) | (i2c->read_byte(ACCEL_OFFSET_X_LSB_ADDR));
        offsets_type.accel_offset_y = (i2c->read_byte(ACCEL_OFFSET_Y_MSB_ADDR) << 8) | (i2c->read_byte(ACCEL_OFFSET_Y_LSB_ADDR));
        offsets_type.accel_offset_z = (i2c->read_byte(ACCEL_OFFSET_Z_MSB_ADDR) << 8) | (i2c->read_byte(ACCEL_OFFSET_Z_LSB_ADDR));

        offsets_type.gyro_offset_x = (i2c->read_byte(GYRO_OFFSET_X_MSB_ADDR) << 8) | (i2c->read_byte(GYRO_OFFSET_X_LSB_ADDR));
        offsets_type.gyro_offset_y = (i2c->read_byte(GYRO_OFFSET_Y_MSB_ADDR) << 8) | (i2c->read_byte(GYRO_OFFSET_Y_LSB_ADDR));
        offsets_type.gyro_offset_z = (i2c->read_byte(GYRO_OFFSET_Z_MSB_ADDR) << 8) | (i2c->read_byte(GYRO_OFFSET_Z_LSB_ADDR));

        offsets_type.mag_offset_x = (i2c->read_byte(MAG_OFFSET_X_MSB_ADDR) << 8) | (i2c->read_byte(MAG_OFFSET_X_LSB_ADDR));
        offsets_type.mag_offset_y = (i2c->read_byte(MAG_OFFSET_Y_MSB_ADDR) << 8) | (i2c->read_byte(MAG_OFFSET_Y_LSB_ADDR));
        offsets_type.mag_offset_z = (i2c->read_byte(MAG_OFFSET_Z_MSB_ADDR) << 8) | (i2c->read_byte(MAG_OFFSET_Z_LSB_ADDR));

        offsets_type.accel_radius = (i2c->read_byte(ACCEL_RADIUS_MSB_ADDR) << 8) | (i2c->read_byte(ACCEL_RADIUS_LSB_ADDR));
        offsets_type.mag_radius = (i2c->read_byte(MAG_RADIUS_MSB_ADDR) << 8) | (i2c->read_byte(MAG_RADIUS_LSB_ADDR));

        setMode(lastMode);
        return true;
    }
    return false;
}


void BNO055::setSensorOffsets(const uint8_t* calibData)
{
    bno055_opmode_t lastMode = _mode;
    setMode(OPERATION_MODE_CONFIG);
    

    /* A writeLen() would make this much cleaner */
    i2c->write_byte(ACCEL_OFFSET_X_LSB_ADDR, calibData[0]);
    i2c->write_byte(ACCEL_OFFSET_X_MSB_ADDR, calibData[1]);
    i2c->write_byte(ACCEL_OFFSET_Y_LSB_ADDR, calibData[2]);
    i2c->write_byte(ACCEL_OFFSET_Y_MSB_ADDR, calibData[3]);
    i2c->write_byte(ACCEL_OFFSET_Z_LSB_ADDR, calibData[4]);
    i2c->write_byte(ACCEL_OFFSET_Z_MSB_ADDR, calibData[5]);

    i2c->write_byte(GYRO_OFFSET_X_LSB_ADDR, calibData[6]);
    i2c->write_byte(GYRO_OFFSET_X_MSB_ADDR, calibData[7]);
    i2c->write_byte(GYRO_OFFSET_Y_LSB_ADDR, calibData[8]);
    i2c->write_byte(GYRO_OFFSET_Y_MSB_ADDR, calibData[9]);
    i2c->write_byte(GYRO_OFFSET_Z_LSB_ADDR, calibData[10]);
    i2c->write_byte(GYRO_OFFSET_Z_MSB_ADDR, calibData[11]);

    i2c->write_byte(MAG_OFFSET_X_LSB_ADDR, calibData[12]);
    i2c->write_byte(MAG_OFFSET_X_MSB_ADDR, calibData[13]);
    i2c->write_byte(MAG_OFFSET_Y_LSB_ADDR, calibData[14]);
    i2c->write_byte(MAG_OFFSET_Y_MSB_ADDR, calibData[15]);
    i2c->write_byte(MAG_OFFSET_Z_LSB_ADDR, calibData[16]);
    i2c->write_byte(MAG_OFFSET_Z_MSB_ADDR, calibData[17]);

    i2c->write_byte(ACCEL_RADIUS_LSB_ADDR, calibData[18]);
    i2c->write_byte(ACCEL_RADIUS_MSB_ADDR, calibData[19]);

    i2c->write_byte(MAG_RADIUS_LSB_ADDR, calibData[20]);
    i2c->write_byte(MAG_RADIUS_MSB_ADDR, calibData[21]);

    setMode(lastMode);
}




void BNO055::setSensorOffsets(const bno055_offsets_t &offsets_type)
{
    bno055_opmode_t lastMode = _mode;
    setMode(OPERATION_MODE_CONFIG);
    
    i2c->write_byte(ACCEL_OFFSET_X_LSB_ADDR, (offsets_type.accel_offset_x) & 0x0FF);
    i2c->write_byte(ACCEL_OFFSET_X_MSB_ADDR, (offsets_type.accel_offset_x >> 8) & 0x0FF);
    i2c->write_byte(ACCEL_OFFSET_Y_LSB_ADDR, (offsets_type.accel_offset_y) & 0x0FF);
    i2c->write_byte(ACCEL_OFFSET_Y_MSB_ADDR, (offsets_type.accel_offset_y >> 8) & 0x0FF);
    i2c->write_byte(ACCEL_OFFSET_Z_LSB_ADDR, (offsets_type.accel_offset_z) & 0x0FF);
    i2c->write_byte(ACCEL_OFFSET_Z_MSB_ADDR, (offsets_type.accel_offset_z >> 8) & 0x0FF);

    i2c->write_byte(GYRO_OFFSET_X_LSB_ADDR, (offsets_type.gyro_offset_x) & 0x0FF);
    i2c->write_byte(GYRO_OFFSET_X_MSB_ADDR, (offsets_type.gyro_offset_x >> 8) & 0x0FF);
    i2c->write_byte(GYRO_OFFSET_Y_LSB_ADDR, (offsets_type.gyro_offset_y) & 0x0FF);
    i2c->write_byte(GYRO_OFFSET_Y_MSB_ADDR, (offsets_type.gyro_offset_y >> 8) & 0x0FF);
    i2c->write_byte(GYRO_OFFSET_Z_LSB_ADDR, (offsets_type.gyro_offset_z) & 0x0FF);
    i2c->write_byte(GYRO_OFFSET_Z_MSB_ADDR, (offsets_type.gyro_offset_z >> 8) & 0x0FF);

    i2c->write_byte(MAG_OFFSET_X_LSB_ADDR, (offsets_type.mag_offset_x) & 0x0FF);
    i2c->write_byte(MAG_OFFSET_X_MSB_ADDR, (offsets_type.mag_offset_x >> 8) & 0x0FF);
    i2c->write_byte(MAG_OFFSET_Y_LSB_ADDR, (offsets_type.mag_offset_y) & 0x0FF);
    i2c->write_byte(MAG_OFFSET_Y_MSB_ADDR, (offsets_type.mag_offset_y >> 8) & 0x0FF);
    i2c->write_byte(MAG_OFFSET_Z_LSB_ADDR, (offsets_type.mag_offset_z) & 0x0FF);
    i2c->write_byte(MAG_OFFSET_Z_MSB_ADDR, (offsets_type.mag_offset_z >> 8) & 0x0FF);

    i2c->write_byte(ACCEL_RADIUS_LSB_ADDR, (offsets_type.accel_radius) & 0x0FF);
    i2c->write_byte(ACCEL_RADIUS_MSB_ADDR, (offsets_type.accel_radius >> 8) & 0x0FF);

    i2c->write_byte(MAG_RADIUS_LSB_ADDR, (offsets_type.mag_radius) & 0x0FF);
    i2c->write_byte(MAG_RADIUS_MSB_ADDR, (offsets_type.mag_radius >> 8) & 0x0FF);

    setMode(lastMode);
}

bool BNO055::isFullyCalibrated(void)
{
    uint8_t system, gyro, accel, mag;
    getCalibration(&system, &gyro, &accel, &mag);
    if (system < 3 || gyro < 3 || accel < 3 || mag < 3)
        return false;
    return true;
}




