#include <string.h>
#include <stdio.h>
#include "m17defines.h"

int pack_8_to_1(uint8_t *in, uint8_t *out, int len){
    int idx = 0;
    for( int i = 0; i < len; i++){
    	out[idx++] = in[i]&0x80 ? 1 : 0;
    	out[idx++] = in[i]&0x40 ? 1 : 0;
    	out[idx++] = in[i]&0x20 ? 1 : 0;
    	out[idx++] = in[i]&0x10 ? 1 : 0;
    	out[idx++] = in[i]&0x08 ? 1 : 0;
    	out[idx++] = in[i]&0x04 ? 1 : 0;
    	out[idx++] = in[i]&0x02 ? 1 : 0;
    	out[idx++] = in[i]&0x01 ? 1 : 0;
    }
    return idx;
}
int pack_1_to_2(uint8_t *in, uint8_t *out, int len){
    int idx = 0;
    for( int i = 0; i < len; i+=2){
    	out[idx++] = (in[i]<<1) | in[i+1];
    }
    return idx;
}
int pack_1_to_8(uint8_t *in, uint8_t *out, int len){
    int idx = 0;
    for( int i = 0; i < len; i+=8){
    	out[idx++] = (in[i]<<7) | (in[i+1]<<6) | (in[i+2]<<5) | (in[i+3]<<4) | (in[i+4]<<3) | (in[i+5]<<2) | (in[i+6]<<1) | (in[i+7]);
    }
    return idx;
}
int pack_48_to_8(uint64_t in, uint8_t *out){
	out[0] = (in>>40)&0xFF;
	out[1] = (in>>32)&0xFF;
	out[2] = (in>>24)&0xFF;
	out[3] = (in>>16)&0xFF;
	out[4] = (in>>8)&0xFF;
	out[5] = in&0xFF;
	return 6;
}
int pack_24_to_8(uint24_t in, uint8_t *out){
	out[0] = (in>>16)&0xFF;
	out[1] = (in>>8)&0xFF;
	out[2] = in&0xFF;
	return 3;
}
int pack_24_to_2(uint24_t in, uint8_t *out){
	out[0]  = (in>>22)&0x3;
	out[1]  = (in>>20)&0x3;
	out[2]  = (in>>18)&0x3;
	out[3]  = (in>>16)&0x3;
	out[4]  = (in>>14)&0x3;
	out[5]  = (in>>12)&0x3;
	out[6]  = (in>>10)&0x3;
	out[7]  = (in>>8)&0x3;
	out[8]  = (in>>6)&0x3;
	out[9]  = (in>>4)&0x3;
	out[10] = (in>>2)&0x3;
	out[11] = in&0x3;
	return 12;
}
int pack_24_to_1(uint24_t in, uint8_t *out){
	int idx = 0;
	for( uint32_t i = 0x800000; i ; i >>= 1){
		out[idx++] = (in&i) ? 1: 0;
	}
	return 24;
}
int pack_16_to_8(uint16_t in, uint8_t *out){
	out[0] = (in>>8)&0xFF;
	out[1] = in&0xFF;
	return 2;
}
int pack_16_to_2(uint16_t in, uint8_t *out){
	out[0] = (in>>14)&0x03;
	out[1] = (in>>12)&0x03;
	out[2] = (in>>10)&0x03;
	out[3] = (in>>8)&0x03;
	out[4] = (in>>6)&0x03;
	out[5] = (in>>4)&0x03;
	out[6] = (in>>2)&0x03;
	out[7] = in&0x03;
	return 8;
}
int pack_8_to_48(uint8_t *in, uint48_t &out){
	out = in[0];
	out <<=8;
	out |= in[1];
	out <<=8;
	out |= in[2];
	out <<=8;
	out |= in[3];
	out <<=8;
	out |= in[4];
	out <<=8;
	out |= in[5];
	return 1;
}
uint48_t pack_8_to_48(uint8_t *in){
	uint48_t out;
	out = in[0];
	out <<=8;
	out |= in[1];
	out <<=8;
	out |= in[2];
	out <<=8;
	out |= in[3];
	out <<=8;
	out |= in[4];
	out <<=8;
	out |= in[5];
	return out;
}

uint24_t pack_8_to_24(uint8_t *in){
	uint24_t out;
	out = in[0];
	out <<=8;
	out |= in[1];
	out <<=8;
	out |= in[2];
	return out;
}
uint16_t pack_8_to_16(uint8_t *in ){
	uint16_t out;
	out = in[0];
	out <<=8;
	out |= in[1];
	return out;
}

int pack_8_to_12_x4(uint8_t *in, uint12_t *out){
	out[0] =   in[0];
	out[0] <<= 4;
	out[0] |=  (in[1]>>4)&0x0F;

	out[1] =   in[1]&0x0F;
	out[1] <<= 8;
	out[1] |=  in[2];

	out[2] =   in[3];
	out[2] <<= 4;
	out[2] |=  (in[4]>>4)&0x0F;

	out[3] =   in[4]&0x0F;
	out[3] <<= 8;
	out[3] |=  in[5];

	return 4;
}
int pack_12_to_8_x4x6(uint12_t *in, uint8_t *out){
	uint24_t w;

	w  = in[0];
	w <<= 12;
	w |= in[1];

	out[0] =   (w>>16)&0xFF;
	out[1] =   (w>>8)&0xFF;
	out[2] =   w&0xFF;

	w  = in[2];
	w <<= 12;
	w |= in[3];

	out[3] =   (w>>16)&0xFF;
	out[4] =   (w>>8)&0xFF;
	out[5] =   w&0xFF;

	return 6;
}

int pack_8_to_8(uint8_t *in, uint8_t *out, int len){
	for( int i = 0; i < len; i++){
		out[i] = in[i];
	}
	return len;
}
uint24_t hard_decode_24_bits(float *in){
	uint24_t word = 0;
	for( int i = 0; i < 24; i++){
		word <<= 1;
		word |= in[i] >= 0 ? 1 : 0;
	}
	return word;
}
//
// Callsign needs to be padded out to 9 characters
//
uint48_t m17_encode_call(const char *call){
	uint48_t word = 0;
	for( int i = 8; i >= 0; i--){
		word *= 40;
		if(call[i] >= 'A' && call[i]<='Z'){
			word += call[i] - 'A' + 1;
		}else{
			if(call[i] >= '0' && call[i]<='9'){
				word += call[i] - '0' + 27;
			}else{
				if(call[i] == '-') word += 37;
				if(call[i] == '/') word += 38;
				if(call[i] == '.') word += 39;
			}
		}
	}
	return word;
}
char *m17_decode_call(uint48_t word, char *call){
	if(word == 0xFFFFFFFFFFFF){
	    strncpy(call,"BROADCAST",10);
	    return call;
	}
	for( int i = 8; i >= 0; i--){
		char c = word%40;
		if(c ==  0) call[8-i] = ' ';
		if(c == 37) call[8-i] = '-';
		if(c == 38) call[8-i] = '/';
		if(c == 39) call[8-i] = '.';
		if((c >= 1) && (c <=26))  call[8-i] = c + 'A' - 1;
		if((c >= 27) && (c <=36)) call[8-i] = c + '0' - 27;
		word = word/40;
	}
	call[9] = 0;// Null terminate string
	return call;
}
//
// Pack Type array into type field
//
uint16_t m17_pack_type(M17Type type){
	uint16_t word = 0;
	word = type.reserved;
	word <<= 4;
	word |= type.can;
	word <<= 2;
	word |= type.est;
	word <<= 2;
	word |= type.et;
	word <<= 2;
	word |= type.dt;
	word <<= 1;
	word |= type.p_s;
	return word;
}
M17Type m17_upack_type(uint16_t word){
	M17Type type;
	type.reserved = (word>>11)&0x1F;
	type.can = (word>>7)&0xF;
	type.est = (word>>5)&0x3;
	type.et  = (word>>3)&0x3;
	type.dt  = (word>>1)&0x3;
	type.p_s = word&0x1;
	return type;
}

void test_encode(void){
	char text[10];
	text[9] = 0;
	uint64_t word = m17_encode_call("G4GUO/P  ");
	m17_decode_call(word,text);
	printf("Call was %s!\n",text);
}
