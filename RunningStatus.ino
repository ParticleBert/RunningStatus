#include <Audio.h>
#include <Wire.h>

#define DEBUG				// ROEY Uncomment this line if you want to disable the debugging over the serial line.
// #define USES_AUDIOSHIELD	// ROEY If you use the audioshield, then keep this line active.
							// If you use only the DAC, then disable this line by making it a comment (//)
// 	DAC-Mode									Audioshield-Mode	
// 	CODEC (SGTL5000) will not be initialized	CODEC will be initialized
//	Pots A1, A2, A3, A6, A7 can be used			Pots A1, A2, A3, A6, A7 can be used
// 	Addtln. Pots on A4, A5, A6, A7 are avlbl.	Pots on A4, A5, A6, A7 are not available
// 	Values for A4, A5, A6, A7 are fix in code	Values for A4, A5, A6, A7 will be read from Potis

// For the fix values please search the code for ROEY

#define DC_LOW  			0
#define DC_HIGH 			1
#define BPM_LOWER_LIMIT   	30
#define BPM_UPPER_LIMIT   	200

								// ROEY These are the positions of the potis on the Teensy 3.2
								// A1, A2, A3, A6, A7 in both configurations
								// A4, A5, A8, A9 without the Audioshield
const int poti_on_a1	= A1;	// Grit (amounf of noise)
const int poti_on_a2	= A2;	// Body (the amount of carrier)
const int poti_on_a3	= A3;	// Tempo
const int poti_on_a6	= A6;	// Filter Cutoff
const int poti_on_a7	= A7;	// Release Time


#ifndef USES_AUDIOSHIELD
const int poti_on_a4	= A4;	// ONLY WIHOUTH AUDIO SHIELD	/4
const int poti_on_a5	= A5;	// ONLY WITHOUT AUDIO SHIELD	/5
const int poti_on_a8	= A8;	// ONLY WITHOUT AUDIO SHIELD	/7
const int poti_on_a9	= A9;	// ONLY WITHOUT AUDIO SHIELD	/8
#endif


// cut here -------------------------------------------------------------------
AudioSynthWaveformDc  		dc_div1;				//xy=868.3333206176758,1251.666561126709
AudioSynthWaveformDc  		dc_div2;				//xy=101.66668701171875,881.6665954589844
AudioSynthWaveformDc  		dc_div3;				//xy=101.66668701171875,853.3332214355469
AudioSynthWaveformDc  		dc_div4;				//xy=95.00003051757812,813.3332901000977
AudioSynthWaveformDc  		dc_div5;				//xy=343.3333625793457,1098.3332948684692
AudioSynthWaveformDc  		dc_div6;				//xy=93.33331680297852,989.9999495744705
AudioSynthWaveformDc  		dc_div7;				//xy=99.9999771118164,919.9999375343323
AudioSynthWaveformDc  		dc_div8;				//xy=348.33334732055664,1140.0001754760742

AudioSynthWaveformDc		dc_forfilter;			//xy=875.0000877380371,1176.6666812896729

AudioSynthWaveformModulated vco_a;				//xy=438.33325576782227,939.9998455047607
AudioSynthWaveformModulated vco_b;				//xy=698.3333168029785,1059.9998064041138

AudioSynthNoiseWhite  		noise1;				//xy=346.6666831970215,1058.33327293396

AudioEffectReverb     		reverb1;			//xy=1226.6668663024902,1073.3333444595337
AudioEffectEnvelope   		envelope_amplitude;	//xy=1066.6666946411133,849.9999046325684
AudioEffectEnvelope   		envelope_filter;	//xy=1078.333408355713,1178.333267211914

AudioFilterStateVariable  	filter1;			//xy=876.666633605957,1064.9998989105225

#ifdef USES_AUDIOSHIELD
AudioControlSGTL5000  		sgtl5000_1;			//xy=1221.6665802001953,1019.9999694824219
AudioOutputI2S        		i2s1;				//xy=1221.6667022705078,961.6666440963745
#endif

AudioOutputAnalog        	dac1;           	//xy=1270.0000190734863,1035.7143001556396

// mixer A collects four clock divider
AudioMixer4           	mixer_a;				//xy=283.3334503173828,884.9998874664307
AudioConnection       	patchCordMixerA_1				(dc_div4, 	0, mixer_a, 0);
AudioConnection       	patchCordMixerA_2				(dc_div3, 	0, mixer_a, 1);
AudioConnection       	patchCordMixerA_3				(dc_div2, 	0, mixer_a, 2);
AudioConnection       	patchCordMixerA_4				(dc_div7, 	0, mixer_a, 3);
AudioConnection       	mixer_a_to_vco_a				(mixer_a, 	vco_a);	

// mixer B collects the carrier, the noise and two additional clocks
AudioMixer4           	mixer_b;				//xy=535.0000419616699,1061.6666240692139
AudioConnection       	patchCordMixerB_1				(vco_a, 	0, mixer_b, 0);
AudioConnection       	patchCordMixerB_2				(noise1, 	0, mixer_b, 1);
AudioConnection       	patchCordMixerB_3				(dc_div5, 	0, mixer_b, 2);
AudioConnection       	patchCordMixerB_4				(dc_div8, 	0, mixer_b, 3);
AudioConnection			mixer_b_to_vco_b				(mixer_b, 	0, vco_b, 0);	

AudioConnection			vco_b_to_filter					(vco_b, 	0, filter1, 0);
AudioConnection			dc_to_envelope_filter			(dc_forfilter, 0, envelope_filter, 0);
AudioConnection			envelope_filter_to_filter		(envelope_filter, 0, filter1, 1);

AudioConnection     	filter1_to_envelope_amplitude	(filter1, 	0, envelope_amplitude, 0);

// AudioConnection       	output_L						(envelope_amplitude, 0, i2s1, 0);
// AudioConnection       	output_R						(envelope_amplitude, 0, i2s1, 1);
#ifdef USES_AUDIOSHIELD
AudioConnection			envelope_amplitude_to_i2s_L		(envelope_amplitude, 0, i2s1, 0);
AudioConnection			envelope_amplitude_to_i2s_R		(envelope_amplitude, 0, i2s1, 1);
#endif

AudioConnection			envelope_amplitude_to_DAC		(envelope_amplitude, 0, dac1, 0);
// cut here -------------------------------------------------------------------

// Global Variables
elapsedMillis ctr_millis = 0;
unsigned int running_counter_index = 0;

// The difference between these arrays is, that array_square generates the sync-pulses in a square-wave-shape, 
// while array_pulse generates them as pulses. The behaviour of array_pulse is more close to the clock generator
// module of the VCV rack.

// array_square:	1010101010101010
//					1100110011001100
//					111000111000111000

// array_pulse:		1010101010101010
//					1000100010001000
//					100000100000100000

const byte array_square[1681] = {0,1,2,7,12,29,58,123,240,245,230,231,200,201,138,159,20,21,50,51,40,45,46,47,128,145,210,215,156,157,170,171,32,37,38,119,88,89,90,95,196,197,162,163,168,189,190,191,16,81,66,71,76,77,106,123,176,181,182,183,136,137,138,207,68,85,114,115,120,125,46,47,128,129,130,151,156,221,250,251,96,101,102,103,8,25,26,31,148,149,162,227,232,237,238,255,80,81,18,23,12,13,42,43,160,245,246,247,216,217,202,207,4,5,34,51,56,61,62,127,192,193,194,199,204,221,186,187,48,53,38,39,8,73,74,95,212,213,242,243,168,173,174,175,0,17,18,87,92,93,106,107,224,229,166,183,152,153,154,159,4,69,98,99,104,125,126,127,144,145,130,135,140,141,170,251,112,117,118,119,72,73,10,15,132,149,178,179,184,253,238,239,64,65,66,87,28,29,58,59,160,165,166,231,200,217,218,223,84,85,34,35,40,45,46,63,144,209,210,215,204,205,234,235,32,53,54,55,24,25,10,79,196,197,226,243,248,253,190,191,0,1,2,7,12,93,122,123,240,245,230,231,136,137,138,159,20,21,50,115,104,109,110,111,192,209,146,151,156,157,170,171,32,101,102,119,88,89,90,95,132,133,162,163,168,189,190,255,80,81,66,71,76,77,42,59,176,181,182,183,136,201,202,207,68,85,114,115,56,61,46,47,128,129,130,215,220,221,250,251,96,101,38,39,8,25,26,31,148,213,226,227,232,237,238,255,16,17,18,23,12,13,42,107,224,245,246,247,216,217,138,143,4,5,34,51,56,125,126,127,192,193,194,199,140,157,186,187,48,53,38,103,72,73,74,95,212,213,178,179,168,173,174,175,0,81,82,87,92,93,106,107,160,165,166,183,152,153,154,223,68,69,98,99,104,125,62,63,144,145,130,135,140,205,234,251,112,117,118,119,8,9,10,15,132,149,178,243,248,253,238,239,64,65,2,23,28,29,58,59,160,229,230,231,200,217,218,223,20,21,34,35,40,45,46,127,208,209,210,215,204,205,170,171,32,53,54,55,24,89,74,79,196,197,226,243,184,189,190,191,0,1,2,71,76,93,122,123,240,245,166,167,136,137,138,159,20,85,114,115,104,109,110,111,128,145,146,151,156,157,170,235,96,101,102,119,88,89,26,31,132,133,162,163,168,253,254,255,80,81,66,71,12,13,42,59,176,181,182,247,200,201,202,207,68,85,50,51,56,61,46,47,128,193,194,215,220,221,250,251,32,37,38,39,8,25,26,95,212,213,226,227,232,237,174,191,16,17,18,23,12,77,106,107,224,245,246,247,152,153,138,143,4,5,34,115,120,125,126,127,192,193,130,135,140,157,186,187,48,117,102,103,72,73,74,95,148,149,178,179,168,173,174,239,64,81,82,87,92,93,42,43,160,165,166,183,152,217,218,223,68,69,98,99,40,61,62,63,144,145,130,199,204,205,234,251,112,117,54,55,8,9,10,15,132,213,242,243,248,253,238,239,0,1,2,23,28,29,58,123,224,229,230,231,200,217,154,159,20,21,34,35,40,109,110,127,208,209,210,215,140,141,170,171,32,53,54,119,88,89,74,79,196,197,162,179,184,189,190,191,0,65,66,71,76,93,122,123,176,181,166,167,136,137,138,223,84,85,114,115,104,109,46,47,128,145,146,151,156,221,234,235,96,101,102,119,24,25,26,31,132,133,162,227,232,253,254,255,80,81,2,7,12,13,42,59,176,245,246,247,200,201,202,207,4,21,50,51,56,61,46,111,192,193,194,215,220,221,186,187,32,37,38,39,8,89,90,95,212,213,226,227,168,173,174,191,16,17,18,87,76,77,106,107,224,245,182,183,152,153,138,143,4,69,98,115,120,125,126,127,128,129,130,135,140,157,186,251,112,117,102,103,72,73,10,31,148,149,178,179,168,237,238,239,64,81,82,87,28,29,42,43,160,165,166,247,216,217,218,223,68,69,34,35,40,61,62,63,144,209,194,199,204,205,234,251,48,53,54,55,8,9,10,79,196,213,242,243,248,253,174,175,0,1,2,23,28,93,122,123,224,229,230,231,136,153,154,159,20,21,34,99,104,109,110,127,208,209,146,151,140,141,170,171,32,117,118,119,88,89,74,79,132,133,162,179,184,189,190,255,64,65,66,71,76,93,58,59,176,181,166,167,136,201,202,223,84,85,114,115,40,45,46,47,128,145,146,215,220,221,234,235,96,101,38,55,24,25,26,31,132,197,226,227,232,253,254,255,16,17,2,7,12,13,42,123,240,245,246,247,200,201,138,143,4,21,50,51,56,125,110,111,192,193,194,215,156,157,186,187,32,37,38,103,72,89,90,95,212,213,162,163,168,173,174,191,16,81,82,87,76,77,106,107,160,181,182,183,152,153,138,207,68,69,98,115,120,125,62,63,128,129,130,135,140,221,250,251,112,117,102,103,8,9,10,31,148,149,178,243,232,237,238,239,64,81,18,23,28,29,42,43,160,229,230,247,216,217,218,223,4,5,34,35,40,61,62,127,208,209,194,199,204,205,170,187,48,53,54,55,8,73,74,79,196,213,242,243,184,189,174,175,0,1,2,87,92,93,122,123,224,229,166,167,136,153,154,159,20,85,98,99,104,109,110,127,144,145,146,151,140,141,170,235,96,117,118,119,88,89,10,15,132,133,162,179,184,253,254,255,64,65,66,71,12,29,58,59,176,181,166,231,200,201,202,223,84,85,50,51,40,45,46,47,128,209,210,215,220,221,234,235,32,37,38,55,24,25,26,95,196,197,226,227,232,253,190,191,16,17,2,7,12,77,106,123,240,245,246,247,136,137,138,143,4,21,50,115,120,125,110,111,192,193,130,151,156,157,186,187,32,101,102,103,72,89,90,95,148,149,162,163,168,173,174,255,80,81,82,87,76,77,42,43,160,181,182,183,152,217,202,207,68,69,98,115,56,61,62,63,128,129,130,199,204,221,250,251,112,117,38,39,8,9,10,31,148,213,242,243,232,237,238,239,0,17,18,23,28,29,42,107,224,229,230,247,216,217,154,159,4,5,34,35,40,125,126,127,208,209,194,199,140,141,170,187,48,53,54,119,72,73,74,79,196,213,178,179,184,189,174,175,0,65,66,87,92,93,122,123,160,165,166,167,136,153,154,223,84,85,98,99,104,109,46,63,144,145,146,151,140,205,234,235,96,117,118,119,24,25,10,15,132,133,162,243,248,253,254,255,64,65,2,7,12,29,58,59,176,245,230,231,200,201,202,223,20,21,50,51,40,45,46,111,192,209,210,215,220,221,170,171,32,37,38,55,24,89,90,95,196,197,226,227,168,189,190,191,16,17,2,71,76,77,106,123,240,245,182,183,136,137,138,143,4,85,114,115,120,125,110,111,128,129,130,151,156,157,186,251,96,101,102,103,72,89,26,31,148,149,162,163,168,237,238,255,80,81,82,87,12,13,42,43,160,181,182,247,216,217,202,207,68,69,34,51,56,61,62,63,128,193,194,199,204,221,250,251,48,53,38,39,8,9,10,95,212,213,242,243,232,237,174,175,0,17,18,23,28,93,106,107,224,229,230,247,152,153,154,159,4,5,34,99,104,125,126,127,208,209,130,135,140,141,170,187,48,117,118,119,72,73,74,79,132,149,178,179,184,189,174,239,64,65,66,87,92,93,58,59,160,165,166,167,136,217,218,223,84,85,98,99,40,45,46,63,144,145,146,215,204,205,234,235,96,117,54,55,24,25,10,15,132,197,226,243,248,253,254,255};
const byte array_pulse[1681] = {0,255,254,255,252,255,250,255,244,255,238,255,216,255,190,255,116,255,250,255,236,255,254,255,208,255,254,255,188,255,234,255,116,255,254,255,216,255,254,255,228,255,186,255,252,255,254,255,80,255,238,255,252,255,250,255,180,255,254,255,200,255,254,255,116,255,250,255,252,255,174,255,208,255,254,255,252,255,250,255,100,255,254,255,152,255,254,255,244,255,234,255,252,255,254,255,80,255,190,255,236,255,250,255,244,255,254,255,216,255,238,255,52,255,250,255,252,255,254,255,192,255,254,255,252,255,186,255,116,255,238,255,216,255,254,255,244,255,250,255,172,255,254,255,80,255,254,255,252,255,234,255,244,255,190,255,216,255,254,255,100,255,250,255,252,255,254,255,144,255,238,255,252,255,250,255,116,255,254,255,200,255,190,255,244,255,250,255,252,255,238,255,80,255,254,255,188,255,250,255,228,255,254,255,216,255,254,255,116,255,170,255,252,255,254,255,208,255,254,255,236,255,250,255,52,255,254,255,216,255,238,255,244,255,250,255,252,255,190,255,64,255,254,255,252,255,250,255,244,255,238,255,152,255,254,255,116,255,250,255,236,255,254,255,208,255,190,255,252,255,234,255,116,255,254,255,216,255,254,255,164,255,250,255,252,255,254,255,80,255,238,255,252,255,186,255,244,255,254,255,200,255,254,255,116,255,250,255,188,255,238,255,208,255,254,255,252,255,250,255,100,255,190,255,216,255,254,255,244,255,234,255,252,255,254,255,16,255,254,255,236,255,250,255,244,255,254,255,216,255,174,255,116,255,250,255,252,255,254,255,192,255,254,255,188,255,250,255,116,255,238,255,216,255,254,255,244,255,186,255,236,255,254,255,80,255,254,255,252,255,234,255,180,255,254,255,216,255,254,255,100,255,250,255,252,255,190,255,208,255,238,255,252,255,250,255,116,255,254,255,136,255,254,255,244,255,250,255,252,255,238,255,80,255,190,255,252,255,250,255,228,255,254,255,216,255,254,255,52,255,234,255,252,255,254,255,208,255,254,255,236,255,186,255,116,255,254,255,216,255,238,255,244,255,250,255,188,255,254,255,64,255,254,255,252,255,250,255,244,255,174,255,216,255,254,255,116,255,250,255,236,255,254,255,144,255,254,255,252,255,234,255,116,255,254,255,216,255,190,255,228,255,250,255,252,255,254,255,80,255,238,255,188,255,250,255,244,255,254,255,200,255,254,255,116,255,186,255,252,255,238,255,208,255,254,255,252,255,250,255,36,255,254,255,216,255,254,255,244,255,234,255,252,255,190,255,80,255,254,255,236,255,250,255,244,255,254,255,152,255,238,255,116,255,250,255,252,255,254,255,192,255,190,255,252,255,250,255,116,255,238,255,216,255,254,255,180,255,250,255,236,255,254,255,80,255,254,255,252,255,170,255,244,255,254,255,216,255,254,255,100,255,250,255,188,255,254,255,208,255,238,255,252,255,250,255,116,255,190,255,200,255,254,255,244,255,250,255,252,255,238,255,16,255,254,255,252,255,250,255,228,255,254,255,216,255,190,255,116,255,234,255,252,255,254,255,208,255,254,255,172,255,250,255,116,255,254,255,216,255,238,255,244,255,186,255,252,255,254,255,64,255,254,255,252,255,250,255,180,255,238,255,216,255,254,255,116,255,250,255,236,255,190,255,208,255,254,255,252,255,234,255,116,255,254,255,152,255,254,255,228,255,250,255,252,255,254,255,80,255,174,255,252,255,250,255,244,255,254,255,200,255,254,255,52,255,250,255,252,255,238,255,208,255,254,255,252,255,186,255,100,255,254,255,216,255,254,255,244,255,234,255,188,255,254,255,80,255,254,255,236,255,250,255,244,255,190,255,216,255,238,255,116,255,250,255,252,255,254,255,128,255,254,255,252,255,250,255,116,255,238,255,216,255,190,255,244,255,250,255,236,255,254,255,80,255,254,255,188,255,234,255,244,255,254,255,216,255,254,255,100,255,186,255,252,255,254,255,208,255,238,255,252,255,250,255,52,255,254,255,200,255,254,255,244,255,250,255,252,255,174,255,80,255,254,255,252,255,250,255,228,255,254,255,152,255,254,255,116,255,234,255,252,255,254,255,208,255,190,255,236,255,250,255,116,255,254,255,216,255,238,255,180,255,250,255,252,255,254,255,64,255,254,255,252,255,186,255,244,255,238,255,216,255,254,255,116,255,250,255,172,255,254,255,208,255,254,255,252,255,234,255,116,255,190,255,216,255,254,255,228,255,250,255,252,255,254,255,16,255,238,255,252,255,250,255,244,255,254,255,200,255,190,255,116,255,250,255,252,255,238,255,208,255,254,255,188,255,250,255,100,255,254,255,216,255,254,255,244,255,170,255,252,255,254,255,80,255,254,255,236,255,250,255,180,255,254,255,216,255,238,255,116,255,250,255,252,255,190,255,192,255,254,255,252,255,250,255,116,255,238,255,152,255,254,255,244,255,250,255,236,255,254,255,80,255,190,255,252,255,234,255,244,255,254,255,216,255,254,255,36,255,250,255,252,255,254,255,208,255,238,255,252,255,186,255,116,255,254,255,200,255,254,255,244,255,250,255,188,255,238,255,80,255,254,255,252,255,250,255,228,255,190,255,216,255,254,255,116,255,234,255,252,255,254,255,144,255,254,255,236,255,250,255,116,255,254,255,216,255,174,255,244,255,250,255,252,255,254,255,64,255,254,255,188,255,250,255,244,255,238,255,216,255,254,255,116,255,186,255,236,255,254,255,208,255,254,255,252,255,234,255,52,255,254,255,216,255,254,255,228,255,250,255,252,255,190,255,80,255,238,255,252,255,250,255,244,255,254,255,136,255,254,255,116,255,250,255,252,255,238,255,208,255,190,255,252,255,250,255,100,255,254,255,216,255,254,255,180,255,234,255,252,255,254,255,80,255,254,255,236,255,186,255,244,255,254,255,216,255,238,255,116,255,250,255,188,255,254,255,192,255,254,255,252,255,250,255,116,255,174,255,216,255,254,255,244,255,250,255,236,255,254,255,16,255,254,255,252,255,234,255,244,255,254,255,216,255,190,255,100,255,250,255,252,255,254,255,208,255,238,255,188,255,250,255,116,255,254,255,200,255,254,255,244,255,186,255,252,255,238,255,80,255,254,255,252,255,250,255,164,255,254,255,216,255,254,255,116,255,234,255,252,255,190,255,208,255,254,255,236,255,250,255,116,255,254,255,152,255,238,255,244,255,250,255,252,255,254,255,64,255,190,255,252,255,250,255,244,255,238,255,216,255,254,255,52,255,250,255,236,255,254,255,208,255,254,255,252,255,170,255,116,255,254,255,216,255,254,255,228,255,250,255,188,255,254,255,80,255,238,255,252,255,250,255,244,255,190,255,200,255,254,255,116,255,250,255,252,255,238,255,144,255,254,255,252,255,250,255,100,255,254,255,216,255,190,255,244,255,234,255,252,255,254,255,80,255,254,255,172,255,250,255,244,255,254,255,216,255,238,255,116,255,186,255,252,255,254,255,192,255,254,255,252,255,250,255,52,255,238,255,216,255,254,255,244,255,250,255,236,255,190,255,80,255,254,255,252,255,234,255,244,255,254,255,152,255,254,255,100,255,250,255,252,255,254,255,208,255,174,255,252,255,250,255,116,255,254,255,200,255,254,255,180,255,250,255,252,255,238,255,80,255,254,255,252,255,186,255,228,255,254,255,216,255,254,255,116,255,234,255,188,255,254,255,208,255,254,255,236,255,250,255,116,255,190,255,216,255,238,255,244,255,250,255,252,255,254,255};

byte wave_0;
byte wave_1;
byte wave_2;
byte wave_3;
byte wave_4;
byte wave_5;
byte wave_6;
byte wave_7;

byte trigger_envelope = 0;
float time_16th = 100;

void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	AudioMemory(20);

	#ifdef USES_AUDIOSHIELD
    sgtl5000_1.enable();
    sgtl5000_1.volume(1);
	#endif

	mixer_a.gain(0,1.5);                       	// ROEY This is the default value for divide-by-4. Change the value behind the comma
	mixer_a.gain(1,0);   				       	// /3
	mixer_a.gain(2,0.4);  						// /2
	mixer_a.gain(3,0);   						// ROEY This is the default vaule for divide-by-7

	vco_a.frequencyModulation(8);			    // 8 Octaves Pitch Modulation
	vco_a.begin(0.8, 80, WAVEFORM_SINE);

	mixer_b.gain(0,1);   						// Carrier
	mixer_b.gain(1,1);   						// Noise
	mixer_b.gain(2,0.8);   						// ROEY This is the default value for divide-by-5
	mixer_b.gain(3,0.5);   						// ROEY This is the default vaule for divide-by-8

	vco_b.frequencyModulation(8);				// 8 Octaves Pitch Modulation
	vco_b.begin(0.8, 150, WAVEFORM_SINE);

	envelope_amplitude.attack(0);
	envelope_amplitude.decay(2);
	envelope_amplitude.sustain(1);
	envelope_amplitude.release(220);			
													
	envelope_filter.attack(3);
	envelope_filter.decay(2);
	envelope_filter.sustain(1);
	envelope_filter.release(90);
	
	dc_forfilter.amplitude(1);
	
	filter1.octaveControl(7);
	filter1.resonance(1.3);
	filter1.frequency(100);
}

void loop() {
	if(ctr_millis > time_16th)
	{
		byte value = array_pulse[running_counter_index];	// Here you can select which array to use.

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
    
		#ifdef DEBUG
		// Debug Output
		Serial.print(running_counter_index);
		Serial.print(": \t");
		Serial.print(value);
		Serial.print("\t");
		Serial.print(wave_0);
		Serial.print(wave_1);
		Serial.print(wave_2);
		Serial.print(wave_3);
		Serial.print(wave_4);
		Serial.print(wave_5);
		Serial.print(wave_6);
		Serial.print("\t");
		Serial.print(time_16th);
		Serial.println(wave_7);
		#endif
    
		// Trigger the Envelopes if value is dividable by 2 without remainder.
		// This is the cause when value == 0, 2, 4, 8 etc.
		if(value % 2 == 0)
		{
			envelope_amplitude.noteOn();
			envelope_filter.noteOn();
		}
		else
		{
			// I commented this out because the envelope gets too long.
			// Then envelope.noteOff gets send as soon as the envelope reaches the sustain phase.
			// envelope_amplitude.noteOff();
			// envelope_filter.noteOff();
		}
	
		// Housekeeping
		running_counter_index++;	// Handle the running counter and his overflow
		if (running_counter_index == 1680)
		{
			running_counter_index = 0;
		}
		ctr_millis = 0;				// Reset the milliseconds
	}
	
	// Read the potis and set the values
	float poti_grit_raw		= (float)analogRead(poti_on_a1) / 1023;	// Grit equals noise and is routed to the noise amplitude
	noise1.amplitude(poti_grit_raw);
	
	float poti_body_raw		= (float)analogRead(poti_on_a2) / 1023;	// Body is the carrier amount
	mixer_a.gain(0,poti_body_raw);
	
	float poti_tempo_raw 	= (float)analogRead(poti_on_a3) / 1023;					// Tempo is the tempo
	float bpm_scaled_value = poti_tempo_raw * (BPM_UPPER_LIMIT - BPM_LOWER_LIMIT);	// Scale the value to the selected BPM range.
	bpm_scaled_value = bpm_scaled_value + BPM_LOWER_LIMIT;
	time_16th = 60 / (bpm_scaled_value * 4) * 1000;   								// *1000 because ms
	
	float poti_cutoff_raw	= (float)analogRead(poti_on_a6) / 1023;	// The cutoff-frequency for the filter		
	float freq_a = poti_cutoff_raw * 2;
	freq_a = freq_a - 1;
	dc_forfilter.amplitude(freq_a);

	float poti_release_raw	= (float)analogRead(poti_on_a7) / 1023;
	float poti_release_calculated	= poti_release_raw * time_16th * 2;
	envelope_amplitude.release(poti_release_calculated);
	envelope_filter.release(poti_release_calculated);

	#ifndef USES_AUDIOSHIELD
	float poti_div4_raw		= (float)analogRead(poti_on_a4) / 1023;	// Div4
	mixer_a.gain(0, poti_div4_raw);
	float poti_div5_raw		= (float)analogRead(poti_on_a5) / 1023;	// Div5
	mixer_b.gain(2, poti_div5_raw);
	float poti_div7_raw		= (float)analogRead(poti_on_a8) / 1023;	// Div7
	mixer_a.gain(3, poti_div7_raw);
	float poti_div8_raw		= (float)analogRead(poti_on_a9) / 1023;	// Div8
	mixer_b.gain(3, poti_div8_raw);
	#endif

	// Again, as soon as the amplitude envelope reaches the sustain phase, the noteOff gets automatically sent.
	if(envelope_amplitude.isSustain() == true)
	{
		envelope_amplitude.noteOff();
		envelope_filter.noteOff();
	}
}
