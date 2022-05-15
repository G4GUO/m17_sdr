#include "m17defines.h"


const int P1 [61] = {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,
                     1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1,
                     0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1};

const int P2[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};

const int P3[8] = {1, 1, 1, 1, 1, 1, 1, 0};

int m17_punc_p1(uint8_t *in, uint8_t *out, int len){
	int idx = 0;
    for( int i = 0; i < len; i++){
    	if(P1[i%61]){
    		out[idx] = in[i];
    		idx++;
    	}
    }
    return idx;
}
int m17_punc_p2(uint8_t *in, uint8_t *out, int len){
	int idx = 0;
    for( int i = 0; i < len; i++){
    	if(P2[i%12]){
    		out[idx] = in[i];
    		idx++;
    	}
    }
    return idx;
}
int m17_punc_p3(uint8_t *in, uint8_t *out, int len){
	int idx = 0;
    for( int i = 0; i < len; i++){
    	if(P3[i%8]){
    		out[idx] = in[i];
    		idx++;
    	}
    }
    return idx;
}
//
// 0.5 means 50/50 i.e unknown
//
// len is the expected output length
//
int m17_de_punc_p1(float *in, float *out, int len){
	int odx = 0;
	int idx = 0;
    for( int i = 0; i < len; i++){
    	if(P1[i%61])
		    out[odx++] = in[idx++];
    	else
    		out[odx++] = 0.5f;
    }
    return odx;
}
int m17_de_punc_p2(float *in, float *out, int len){
	int odx = 0;
	int idx = 0;
    for( int i = 0; i < len; i++){
    	if(P2[i%12])
    		out[odx++] = in[idx++];
    	else
    		out[odx++] = 0.5f;
    }
    return odx;
}
int m17_de_punc_p3(float *in, float *out, int len){
	int idx = 0;
	int odx = 0;
    for( int i = 0; i < len; i++){
   	    if(P3[i%8])
    		out[odx++] = in[idx++];
    	else
    		out[odx++] = 0.5f;
    }
    return odx;
}
