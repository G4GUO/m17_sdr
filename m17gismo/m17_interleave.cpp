#include "m17defines.h"

void m17_interleave( uint8_t *in, uint8_t *out, int len){
	for( int i = 0; i < len; i++){
		out[((i*45) + (92*i*i))%368] = in[i];
	}
}
void m17_de_interleave( float *in, float *out, int len){
	for( int i = 0; i < len; i++){
		out[((i*45) + (92*i*i))%368] = in[i];
	}
}
