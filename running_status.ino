#include <Audio.h>
#include <Wire.h>

#define DC_LOW  0
#define DC_HIGH 1
#define BPM_LOWER_LIMIT   (30)
#define BPM_UPPER_LIMIT   (200)
// cut here -------------------------------------------------------------------
AudioSynthWaveformDc  dc_div1;
AudioSynthWaveformDc  dc_div2;
AudioSynthWaveformDc  dc_div3;
AudioSynthWaveformDc  dc_div4;
AudioSynthWaveformDc  dc_div5;
AudioSynthWaveformDc  dc_div6;
AudioSynthWaveformDc  dc_div7;
AudioSynthWaveformDc  dc_div8;

AudioSynthWaveformModulated vco_a;
AudioSynthWaveformModulated vco_b;

AudioSynthNoiseWhite  		noise1;

AudioEffectReverb     		reverb1;
AudioEffectEnvelope   		envelope1;

AudioFilterStateVariable  	filter1;

AudioOutputI2S        		i2s1;
AudioControlSGTL5000  		sgtl5000_1;

// mixer A collects four clock divider
AudioMixer4           	mixer_a;
AudioConnection       	patchCordMixerA_1(dc_div4, 		0, mixer_a, 0);
AudioConnection       	patchCordMixerA_2(dc_div3, 		0, mixer_a, 1);
AudioConnection       	patchCordMixerA_3(dc_div2, 		0, mixer_a, 2);
AudioConnection       	patchCordMixerA_4(dc_div7, 		0, mixer_a, 3);
AudioConnection       	mixer_a_to_vco_a(mixer_a, vco_a);

// mixer B collects the carrier, the noise and two additional clocks
AudioMixer4           	mixer_b;
AudioConnection       	patchCordMixerB_1(vco_a, 		0, mixer_b, 0);
AudioConnection       	patchCordMixerB_2(noise1, 		0, mixer_b, 1);
AudioConnection       	patchCordMixerB_3(dc_div5, 		0, mixer_b, 2);
AudioConnection       	patchCordMixerB_4(dc_div8, 		0, mixer_b, 3);
AudioConnection			mixer_b_to_vco_b(mixer_b, 		0, vco_b, 0);	
AudioConnection			vco_b_to_filter1(vco_b, 		0, filter1, 0);
AudioConnection     	filter1_to_envelope1(filter1, 	0, envelope1, 0);

AudioConnection       	output_L(envelope1, 			0, i2s1, 0);
AudioConnection       	output_R(envelope1, 			0, i2s1, 1);
// cut here -------------------------------------------------------------------

// Global Variables
elapsedMillis ctr_millis = 0;
unsigned int running_counter_index = 0;

// This runs from 0 to 1680
byte running_counter[1681] = {0,1,2,7,12,29,58,123,240,245,230,231,200,201,138,159,20,21,50,51,40,45,46,47,128,145,210,215,156,157,170,171,32,37,38,119,88,89,90,95,196,197,162,163,168,189,190,191,16,81,66,71,76,77,106,123,176,181,182,183,136,137,138,207,68,85,114,115,120,125,46,47,128,129,130,151,156,221,250,251,96,101,102,103,8,25,26,31,148,149,162,227,232,237,238,255,80,81,18,23,12,13,42,43,160,245,246,247,216,217,202,207,4,5,34,51,56,61,62,127,192,193,194,199,204,221,186,187,48,53,38,39,8,73,74,95,212,213,242,243,168,173,174,175,0,17,18,87,92,93,106,107,224,229,166,183,152,153,154,159,4,69,98,99,104,125,126,127,144,145,130,135,140,141,170,251,112,117,118,119,72,73,10,15,132,149,178,179,184,253,238,239,64,65,66,87,28,29,58,59,160,165,166,231,200,217,218,223,84,85,34,35,40,45,46,63,144,209,210,215,204,205,234,235,32,53,54,55,24,25,10,79,196,197,226,243,248,253,190,191,0,1,2,7,12,93,122,123,240,245,230,231,136,137,138,159,20,21,50,115,104,109,110,111,192,209,146,151,156,157,170,171,32,101,102,119,88,89,90,95,132,133,162,163,168,189,190,255,80,81,66,71,76,77,42,59,176,181,182,183,136,201,202,207,68,85,114,115,56,61,46,47,128,129,130,215,220,221,250,251,96,101,38,39,8,25,26,31,148,213,226,227,232,237,238,255,16,17,18,23,12,13,42,107,224,245,246,247,216,217,138,143,4,5,34,51,56,125,126,127,192,193,194,199,140,157,186,187,48,53,38,103,72,73,74,95,212,213,178,179,168,173,174,175,0,81,82,87,92,93,106,107,160,165,166,183,152,153,154,223,68,69,98,99,104,125,62,63,144,145,130,135,140,205,234,251,112,117,118,119,8,9,10,15,132,149,178,243,248,253,238,239,64,65,2,23,28,29,58,59,160,229,230,231,200,217,218,223,20,21,34,35,40,45,46,127,208,209,210,215,204,205,170,171,32,53,54,55,24,89,74,79,196,197,226,243,184,189,190,191,0,1,2,71,76,93,122,123,240,245,166,167,136,137,138,159,20,85,114,115,104,109,110,111,128,145,146,151,156,157,170,235,96,101,102,119,88,89,26,31,132,133,162,163,168,253,254,255,80,81,66,71,12,13,42,59,176,181,182,247,200,201,202,207,68,85,50,51,56,61,46,47,128,193,194,215,220,221,250,251,32,37,38,39,8,25,26,95,212,213,226,227,232,237,174,191,16,17,18,23,12,77,106,107,224,245,246,247,152,153,138,143,4,5,34,115,120,125,126,127,192,193,130,135,140,157,186,187,48,117,102,103,72,73,74,95,148,149,178,179,168,173,174,239,64,81,82,87,92,93,42,43,160,165,166,183,152,217,218,223,68,69,98,99,40,61,62,63,144,145,130,199,204,205,234,251,112,117,54,55,8,9,10,15,132,213,242,243,248,253,238,239,0,1,2,23,28,29,58,123,224,229,230,231,200,217,154,159,20,21,34,35,40,109,110,127,208,209,210,215,140,141,170,171,32,53,54,119,88,89,74,79,196,197,162,179,184,189,190,191,0,65,66,71,76,93,122,123,176,181,166,167,136,137,138,223,84,85,114,115,104,109,46,47,128,145,146,151,156,221,234,235,96,101,102,119,24,25,26,31,132,133,162,227,232,253,254,255,80,81,2,7,12,13,42,59,176,245,246,247,200,201,202,207,4,21,50,51,56,61,46,111,192,193,194,215,220,221,186,187,32,37,38,39,8,89,90,95,212,213,226,227,168,173,174,191,16,17,18,87,76,77,106,107,224,245,182,183,152,153,138,143,4,69,98,115,120,125,126,127,128,129,130,135,140,157,186,251,112,117,102,103,72,73,10,31,148,149,178,179,168,237,238,239,64,81,82,87,28,29,42,43,160,165,166,247,216,217,218,223,68,69,34,35,40,61,62,63,144,209,194,199,204,205,234,251,48,53,54,55,8,9,10,79,196,213,242,243,248,253,174,175,0,1,2,23,28,93,122,123,224,229,230,231,136,153,154,159,20,21,34,99,104,109,110,127,208,209,146,151,140,141,170,171,32,117,118,119,88,89,74,79,132,133,162,179,184,189,190,255,64,65,66,71,76,93,58,59,176,181,166,167,136,201,202,223,84,85,114,115,40,45,46,47,128,145,146,215,220,221,234,235,96,101,38,55,24,25,26,31,132,197,226,227,232,253,254,255,16,17,2,7,12,13,42,123,240,245,246,247,200,201,138,143,4,21,50,51,56,125,110,111,192,193,194,215,156,157,186,187,32,37,38,103,72,89,90,95,212,213,162,163,168,173,174,191,16,81,82,87,76,77,106,107,160,181,182,183,152,153,138,207,68,69,98,115,120,125,62,63,128,129,130,135,140,221,250,251,112,117,102,103,8,9,10,31,148,149,178,243,232,237,238,239,64,81,18,23,28,29,42,43,160,229,230,247,216,217,218,223,4,5,34,35,40,61,62,127,208,209,194,199,204,205,170,187,48,53,54,55,8,73,74,79,196,213,242,243,184,189,174,175,0,1,2,87,92,93,122,123,224,229,166,167,136,153,154,159,20,85,98,99,104,109,110,127,144,145,146,151,140,141,170,235,96,117,118,119,88,89,10,15,132,133,162,179,184,253,254,255,64,65,66,71,12,29,58,59,176,181,166,231,200,201,202,223,84,85,50,51,40,45,46,47,128,209,210,215,220,221,234,235,32,37,38,55,24,25,26,95,196,197,226,227,232,253,190,191,16,17,2,7,12,77,106,123,240,245,246,247,136,137,138,143,4,21,50,115,120,125,110,111,192,193,130,151,156,157,186,187,32,101,102,103,72,89,90,95,148,149,162,163,168,173,174,255,80,81,82,87,76,77,42,43,160,181,182,183,152,217,202,207,68,69,98,115,56,61,62,63,128,129,130,199,204,221,250,251,112,117,38,39,8,9,10,31,148,213,242,243,232,237,238,239,0,17,18,23,28,29,42,107,224,229,230,247,216,217,154,159,4,5,34,35,40,125,126,127,208,209,194,199,140,141,170,187,48,53,54,119,72,73,74,79,196,213,178,179,184,189,174,175,0,65,66,87,92,93,122,123,160,165,166,167,136,153,154,223,84,85,98,99,104,109,46,63,144,145,146,151,140,205,234,235,96,117,118,119,24,25,10,15,132,133,162,243,248,253,254,255,64,65,2,7,12,29,58,59,176,245,230,231,200,201,202,223,20,21,50,51,40,45,46,111,192,209,210,215,220,221,170,171,32,37,38,55,24,89,90,95,196,197,226,227,168,189,190,191,16,17,2,71,76,77,106,123,240,245,182,183,136,137,138,143,4,85,114,115,120,125,110,111,128,129,130,151,156,157,186,251,96,101,102,103,72,89,26,31,148,149,162,163,168,237,238,255,80,81,82,87,12,13,42,43,160,181,182,247,216,217,202,207,68,69,34,51,56,61,62,63,128,193,194,199,204,221,250,251,48,53,38,39,8,9,10,95,212,213,242,243,232,237,174,175,0,17,18,23,28,93,106,107,224,229,230,247,152,153,154,159,4,5,34,99,104,125,126,127,208,209,130,135,140,141,170,187,48,117,118,119,72,73,74,79,132,149,178,179,184,189,174,239,64,65,66,87,92,93,58,59,160,165,166,167,136,217,218,223,84,85,98,99,40,45,46,63,144,145,146,215,204,205,234,235,96,117,54,55,24,25,10,15,132,197,226,243,248,253,254,255};

byte wave_0;
byte wave_1;
byte wave_2;
byte wave_3;
byte wave_4;
byte wave_5;
byte wave_6;
byte wave_7;

byte trigger_envelope = 0;

void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	AudioMemory(20);

    sgtl5000_1.enable();
    sgtl5000_1.volume(1);

	mixer_a.gain(0,1.5);                       	// /4
	mixer_a.gain(1,0);   				       	// /3
	mixer_a.gain(2,0.4);  						// /2
	mixer_a.gain(3,0);   						// /7

	vco_a.frequencyModulation(8);			    // 12 Octaves Pitch Modulation
	vco_a.begin(0.8, 80, WAVEFORM_SINE);

	mixer_b.gain(0,1);   						// Carrier
	mixer_b.gain(1,1);   						// Noise
	mixer_b.gain(2,0.8);   						// /5
	mixer_b.gain(3,0.5);   						// /8

	vco_b.frequencyModulation(8);			// 12 Octaves Pitch Modulation
	vco_b.begin(0.8, 150, WAVEFORM_SINE);

	envelope1.attack(0);
	envelope1.decay(0);
	envelope1.sustain(1);
	envelope1.release(180);	
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
    
		// Trigger the Envelope
		envelope1.noteOn();
	
		// Housekeeping
		running_counter_index++;
		if (running_counter_index == 1681)
		{
			running_counter_index = 0;
		}
		ctr_millis = 0;
	}
		// --------------------------------------------------------------------------
	// Read the potis
	// --------------------------------------------------------------------------
	// 	A6			A7			A1 		A2		A3
	//	log			log			lin		lin		lin  
	// decay+filter	nothing		noise	body	tempo
	//							grit

	float various3_poti_raw = (float)analogRead(A6) / 1023;		// On my HW this poti is logarithmic

	float various2_poti_raw = (float)analogRead(A1) / 1023;	
	float various1_poti_raw = (float)analogRead(A2) / 1023;
	float bpm_poti_raw = (float)analogRead(A3) / 1023;

  
	// ------------------------------------------------------------------------
	// Do the poti calculations
	// ------------------------------------------------------------------------
  
	// BPM poti
	// Scale the value to the allowed BPM maximum and minimum
	float bpm_scaled_value = bpm_poti_raw * (BPM_UPPER_LIMIT - BPM_LOWER_LIMIT);
	bpm_scaled_value = bpm_scaled_value + BPM_LOWER_LIMIT;
  
	// Calculate the frequency which corresponds to 1/16th note at the chosen BPM
	float time_16th = 60 / (bpm_scaled_value * 2) * 1000;   // in ms      FIXME Why 2?
	float hertz_16th = (bpm_scaled_value * 2) / 60;         // In Hertz   FIXME Why 2?

	// envelope1.release(time_16th*4);
	// Timer1.setPeriod(time_16th);

	// Frequency poti
	float freq_a = various3_poti_raw * 10000;

	// Body
	mixer_a.gain(0,various1_poti_raw);
	
	// Grit / Noise
	noise1.amplitude(various2_poti_raw);

	filter1.frequency(freq_a);

	if(envelope1.isSustain() == true)
	{
		envelope1.noteOff();
	}
}