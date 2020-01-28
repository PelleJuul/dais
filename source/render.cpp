#include <Bela.h>
#include <cmath>
#include <iostream>
#include <Midi.h>

#include "I2C_APDS9900.h"
#include "ExpCal.h"
#include "BowedString.h"
#include "BNO055.h"
#include "imumaths.h"
#include <vector>

#define NUM_SENSORS (3)
#define NUM_VIBROS (15)

void midiMessageCallback(MidiChannelMessage message, void* arg);

// Midi input
Midi midi;
const char* gMidiPort0 = "hw:1,0,0";

// Instance for the proximity sensor
I2C_APDS9900 proximitySensor;

// Calibration algorithm for the proximity sensor
ExpCal proximityCalibration;

bool firstProximity = true;

float minProximity = 6;

int proximityCount = 0;

// Instance for imuSensor
BNO055 imuSensor(BNO055_ID, BNO055_ADDRESS_A, 1);

// Flag indicating whether the IMU has been initialized.
bool imuInitialized = false;

// How long to wait before reading the next sensor
float switchTime = 0.005;

// Time until we should perform the next measurement.
float nextMeasurement = switchTime;

// Previous measured distance (for filtering).
float d = 9;

float note = 60;
float bend = 0;

imu::Vector<3> orientOffset;
int imuCount = 0;

void measureProximity(void*);
AuxiliaryTask measureProximityTask;

BowedString bowedString(40, 44100);

int blockNo = 0;

bool setup(BelaContext *context, void *userData)
{
	pinMode(context, 0, 0, OUTPUT);
	pinMode(context, 0, 1, OUTPUT);
	
	midi.readFrom(gMidiPort0);
	midi.writeTo(gMidiPort0);
	midi.enableParser(true);
	midi.setParserCallback(midiMessageCallback, (void*) gMidiPort0);

	measureProximityTask = Bela_createAuxiliaryTask(
		&measureProximity, 95, "measure proximity task", nullptr);

	proximityCalibration.init(3.500000, 3.290660, -0.004086, 155.000000);

    return true;
}

void render(BelaContext *context, void *userData)
{
	for(unsigned int n = 0; n < context->audioFrames; n++) 
	{
		nextMeasurement -= 1.0 / (float)context->digitalSampleRate;
		
		if (nextMeasurement <= 0.0)
		{
			nextMeasurement = switchTime;
			Bela_scheduleAuxiliaryTask(measureProximityTask);
		}

		float x = audioRead(context, n, 0);
		bowedString.wavespeed = powf(2, ((note + bend) - 69) / 12.0f) * 440;
		float out = 0.5 * 1e3 * bowedString.getNextSample(1000 * x);
		
		audioWrite(context, n, 0, out);
		audioWrite(context, n, 1, out);
	} 
}

float mapRange(float x, float oldMin, float oldMax, float newMin, float newMax)
{
    float r = (x - oldMin) / (oldMax - oldMin);
	return newMin + r * (newMax - newMin);
}

void measureProximity(void*)
{
	// rt_printf("Reading sensors\n");
	
	if (!proximitySensor.isInitialized())
	{
		
		if (proximitySensor.begin())
		{
			rt_printf("Proximity sensor initialized\n");
		}
		else
		{
			rt_printf("Failed to initialize proximity sensor\n");
		}
	}

	// Do proximity sensor stuff
	float proximity = 0;
	
	if (proximitySensor.isInitialized() &&
		proximitySensor.readProximity(&proximity))
	{
		float calibrated = proximityCalibration.compute(proximity);
		d = 0.1 * d + 0.9 * calibrated;
		
		if (proximityCount > 2 && proximityCount < 20)
		{
			d = calibrated;
			minProximity = 0.5 * minProximity + 0.5 * calibrated;
			minProximity = calibrated;
			firstProximity = false;
		}
		
		float s = 5;
		float p = mapRange(calibrated, 3.5, minProximity - 0.5, 1, 0);
		
		p = constrain(p, 0, 1.0);
		
		bowedString.Fb = 10000 * (exp(s * p) - 1) / (exp(s) - 1);
		// rt_printf("%.0f %.2f %.2f %.2f %.2f\n", proximity, d, minProximity, p, bowedString.Fb);
		
		proximityCount++;
	}
	else
	{
		rt_printf("Failed to read sensor\n");
		return;
	}
	
	// Do IMU stuff
	if (!imuInitialized)
	{
		imuSensor.setMode(BNO055::OPERATION_MODE_NDOF);
		/*
		if (imuSensor.begin(BNO055::OPERATION_MODE_NDOF))
		{
			imuInitialized = true;
			rt_printf("IMU initialized\n");
		}
		else
		{
			rt_printf("Failed to initialize IMU\n");
		}
		*/
	}

	imuInitialized = true;

	if (imuInitialized)
	{
		// TODO: Remove this read, it is not necessary if stuff is working.
		uint8_t status, result, error = 0;
		imuSensor.getSystemStatus(&status, &result, &error);

		imu::Vector<3> orient = imuSensor.getVector(BNO055::VECTOR_EULER);

		// TODO: Map measurement to some parameter in bowedString e.g. bow
		//       velocity.
		
		imuCount++;
		
		if (imuCount == 20)
		{
			orientOffset = orient;
		}
		
		if (imuCount > 20)
		{
			// orientOffset = orientOffset * 0.999 + orient * 0.001;
			orient = orient - orientOffset;
			/*
			rt_printf(
				"(%i %i %i) %.2f, %.2f, %.2f\n",
				status, result, error,
				orient.x(), orient.y(), orient.z());
				*/
				
			// rt_printf("%f\n", orient.y());
			float y = orient.y() + 1;
			bowedString.vb = y > 0 ? 0.0 : fabs(orient.y() / 15.0);
			
			float z = orient.z();
			
			if (z > 180)
			{
				z = z - 360;
			}
			else if (z < -180)
			{
				z = z + 360;
			}
			
			bend = z / 10.0f;
		}
	}
	
	rt_printf("fb: %.0f, vb: %.2f, bend: %f\n\n", bowedString.Fb, bowedString.vb, bend);
}

void cleanup(BelaContext *context, void *userData)
{
	
}

void midiMessageCallback(MidiChannelMessage message, void* arg)
{
	if(arg != NULL){
		// rt_printf("Message from midi port %s ", (const char*) arg);
	}

	message.prettyPrint();

	if(message.getType() == kmmNoteOn)
	{
        float newNote = message.getDataByte(0);
        
        if (newNote <= 84)
        {
        	rt_printf("Note is now: %.0f\n", newNote);
        	note = newNote;
        }
	}
	
	if (message.getType() == kmmControlChange && message.getDataByte(0) == 1 && message.getDataByte(1) == 0)
	{
		imuCount = 0;
	}
}
