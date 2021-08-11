#include <Audio.h>
#include <Wire.h>

#define DC_LOW  0
#define DC_HIGH 1

AudioSynthWaveformDc  dc_div1;
AudioSynthWaveformDc  dc_div2;
AudioSynthWaveformDc  dc_div3;
AudioSynthWaveformDc  dc_div4;
AudioSynthWaveformDc  dc_div5;
AudioSynthWaveformDc  dc_div6;
AudioSynthWaveformDc  dc_div7;
AudioSynthWaveformDc  dc_div8;

elapsedMillis ctr_millis = 0;
byte running_counter_index = 0;

// This runs from 0 to 144
byte running_counter[145] = {0,1,2,7,12,29,58,123,240,245,230,231,200,201,138,159,20,21,50,51,40,109,110,111,192,209,210,215,156,157,170,171,32,37,38,119,88,89,90,95,196,197,162,163,168,189,190,191,16,81,66,71,76,77,106,123,176,181,182,183,136,137,138,207,68,85,114,115,120,125,46,47,128,129,130,151,156,221,250,251,96,101,102,103,8,25,26,31,148,149,162,227,232,237,238,255,80,81,18,23,12,13,42,43,160,245,246,247,216,217,202,207,4,5,34,51,56,61,62,127,192,193,194,199,204,221,186,187,48,53,38,39,8,73,74,95,212,213,242,243,168,1,3,174,175};

byte wave_0;
byte wave_1;
byte wave_2;
byte wave_3;
byte wave_4;
byte wave_5;
byte wave_6;
byte wave_7;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
	// put your main code here, to run repeatedly:
	if(ctr_millis > 100)
	{
		byte value = running_counter[running_counter_index];

		// Bit 0
		if(value & 1)
		{
			wave_0 = 1;
			dc_div1.amplitude(DC_HIGH);
		}
		else
		{
			wave_0 = 0;
			dc_div1.amplitude(DC_LOW);
		}
		// Bit 1
		
		if(value & 2)
		{
			wave_1 = 1;
			dc_div2.amplitude(DC_HIGH);
		}
		else
		{
			wave_1 = 0;
			dc_div2.amplitude(DC_LOW);
		}
		
		// Bit 2
		if(value & 4)
		{
			wave_2 = 1;
			dc_div3.amplitude(DC_HIGH);
		}
		else
		{
			wave_2 = 0;
			dc_div3.amplitude(DC_LOW);
		}
		
		// Bit 3
		if(value & 8)
		{
			wave_3 = 1;
			dc_div4.amplitude(DC_HIGH);
		}
		else
		{
			wave_3 = 0;
			dc_div4.amplitude(DC_LOW);
		}
		// Bit 4
		
		if(value & 16)
		{
			wave_4 = 1;
			dc_div5.amplitude(DC_HIGH);
		}
		else
		{
			wave_4 = 0;
			dc_div5.amplitude(DC_LOW);
		}
		
		// Bit 5
		if(value & 32)
		{
			wave_5 = 1;
			dc_div6.amplitude(DC_HIGH);
		}
		else
		{
			wave_5 = 0;
			dc_div6.amplitude(DC_LOW);
		}
		
		// Bit 6
		if(value & 64)
		{
			wave_6 = 1;
			dc_div7.amplitude(DC_HIGH);
		}
		else
		{
			wave_6 = 0;
			dc_div7.amplitude(DC_LOW);
		}
		
		// Bit 7
		if(value & 128)
		{
			wave_7 = 1;
			dc_div8.amplitude(DC_HIGH);
		}
		else
		{
			wave_7 = 0;
			dc_div8.amplitude(DC_LOW);
		}
    
		// Debug Output
		Serial.print(value);
		Serial.print("\t");
		Serial.print(wave_0);
		Serial.print(wave_1);
		Serial.print(wave_2);
		Serial.print(wave_3);
		Serial.print(wave_4);
		Serial.print(wave_5);
		Serial.print(wave_6);
		Serial.println(wave_7);
    
		// Housekeeping
		running_counter_index++;
		if (running_counter_index == 145)
		{
			running_counter_index = 0;
		}
		ctr_millis = 0;
	}
}