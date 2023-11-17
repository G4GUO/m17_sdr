#include "m17defines.h"

const uint8_t ctab[46] = { 0xD6, 0xB5, 0xE2, 0x30, 0x82, 0xFF, 0x84, 0x62, 0xBA, 0x4E,
                           0x96, 0x90, 0xD8, 0x98, 0xDD, 0x5D, 0x0C, 0xC8, 0x52, 0x43,
						   0x91, 0x1D, 0xF8, 0x6E, 0x68, 0x2F, 0x35, 0xDA, 0x14, 0xEA,
						   0xCD, 0x76, 0x19, 0x8D, 0xD5, 0x80, 0xD1, 0x33, 0x87, 0x13,
						   0x57, 0x18, 0x2D, 0x29, 0x78, 0xC3 };

static uint8_t m_bit_ctab[46*8];

void m17_de_correlate_8( uint8_t *in, int len){
	for( int i = 0 ; i < len; i++){
		in[i] = in[i]^ctab[i%46];
	}
}
void m17_de_correlate_1( uint8_t *in, uint8_t *out, int len){
	for( int i = 0 ; i < len; i++){
		out[i] = (in[i]^m_bit_ctab[i%368])&1;
	}
}//
// Soft bits are stored as probabilities where
// 1.0 means 100% certain it is a one
// 0   means 50% certain
// -1 means 0% certain
// to flip a bit change it's sign
//
void m17_de_correlate_1( float *in, float *out, int len){
	for( int i = 0 ; i < len; i++){
		out[i] = m_bit_ctab[i%368] ? -in[i] : in[i];
	}
}
//
// turn the 8 bit table into a 368 bit sequence
//
void m17_init_de_correlate(void){
	int idx = 0;
	for( int i = 0; i < 46; i++){
		for( uint8_t n = 0x80; n; n>>=1){
			m_bit_ctab[idx++] = (ctab[i]&n) ? 1 : 0;
		}
	}
}
