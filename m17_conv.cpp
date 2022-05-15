//
// Convolutional encoder and viterbi decoder
//
#include <stdio.h>
#include <memory.h>
#include "m17defines.h"

#define NR_CONVOLUTIONAL_STATES 16

// V = The Current state
// W = A Previous state
// X = Metric from state W to state V
// Y = A Previous state
// Z = Metric from state Y to state V
static int 	 m_hp=0;
static float m_acm[NR_CONVOLUTIONAL_STATES];
static int   m_path[NR_CONVOLUTIONAL_STATES][16400];// Large number to hold an entire packet

#define BF(v,w,x,y,z) tempa=m_acm[w]+metric[x];tempb=m_acm[y]+metric[z];if(tempa>tempb){tm[v]=tempa;m_path[v][m_hp]=w;}else{tm[v]=tempb;m_path[v][m_hp]=y;}
//#define BF(v,w,x,y,z) tempa=m_acm[w]*metric[x];tempb=m_acm[y]*metric[z];if(tempa>tempb){tm[v]=tempa;m_path[v][m_hp]=w;}else{tm[v]=tempb;m_path[v][m_hp]=y;}

static uint16_t clut[32][2];

void m17_build_clut_table(void){
	for( uint8_t i = 0; i < 32; i++){
		clut[i][0] = ((i>>4)&1)^((i>>1)&1)^(i&1);
		clut[i][1] = ((i>>4)&1)^((i>>3)&1)^((i>>2)&1)^(i&1);
	}
}
//
// Convolutional encode 1 bit input, 1 bit output
//
int m17_conv_encode_1(uint8_t *in, uint8_t *out, int len){
	int idx = 0;
	uint8_t sr = 0;
	for( int i = 0; i < len; i++){
		if(in[i]) sr |= 0x10;
		out[idx++] = clut[sr][0];
		out[idx++] = clut[sr][1];
		sr>>=1;
	}
	// Tail 0
	for( int i = 0; i < 4; i++){
		out[idx++] = clut[sr][0];
		out[idx++] = clut[sr][1];
		sr>>=1;
	}
	return idx;
}
//
// Convolutional encode 8 byte input, 1 bit output
//
int m17_conv_encode_8(uint8_t *in, uint8_t *out, int len){
	int idx = 0;
	uint8_t sr = 0;
	for( int i = 0; i < len; i++){
		for( uint8_t n = 0x80; n; n >>=1 ){
		    if(in[i]&n) sr |= 0x10;
		    out[idx++] = clut[sr][0];
		    out[idx++] = clut[sr][1];
		    sr>>=1;
		}
	}
	// Now do the 4 bit 0 tail
	for( int i = 0; i < 4; i++){
		out[idx++] = clut[sr][0];
		out[idx++] = clut[sr][1];
		sr>>=1;
	}
	return idx;
}

void m17_conv_new_metric(float m1, float m2){
	float tempa,tempb;
	float metricX0,metricX1,metric0X,metric1X;
	float metric[4];
	float tm[NR_CONVOLUTIONAL_STATES];
	int   i;

	/* Calculate the metric values */
	// The values passed are the probability of logic 1

	metric1X  =  m1;
	metric0X  = 1.0f - m1;
	metricX1  =  m2;
	metricX0  = 1.0f - m2;

	metric[0] = (metric0X + metricX0);
	metric[1] = (metric0X + metricX1);
	metric[2] = (metric1X + metricX0);
	metric[3] = (metric1X + metricX1);

	BF(0,0,0,1,3)
	BF(1,2,2,3,1)
	BF(2,4,1,5,2)
	BF(3,6,3,7,0)
	BF(4,8,1,9,2)
	BF(5,10,3,11,0)
	BF(6,12,0,13,3)
	BF(7,14,2,15,1)
	BF(8,0,3,1,0)
	BF(9,2,1,3,2)
	BF(10,4,2,5,1)
	BF(11,6,0,7,3)
	BF(12,8,2,9,1)
	BF(13,10,0,11,3)
	BF(14,12,3,13,0)
	BF(15,14,1,15,2)

	for( i = 0; i < NR_CONVOLUTIONAL_STATES; i++ ) m_acm[i] = tm[i];

	m_hp++;
}
/*
 * Used to generate the butterfly table in the decoder
 */
/*
void code_generate(void)
{
	int i;
	int last0;
	int last1;
	int dibit0;
	int dibit1;
	int temp1;
	int temp2;

	for(i=0; i< NR_CONVOLUTIONAL_STATES; i++ )
	{
		last0 = (i<<1)&0xF;
		last1 = ((i<<1)+1)&0xF;

		temp1 = clut[(i<<1)][0];
		temp2 = clut[(i<<1)][1];
		dibit0 = (temp1<<1)+temp2;

		temp1 = clut[(i<<1)+1][0];
		temp2 = clut[(i<<1)+1][1];
		dibit1 = (temp1<<1)+temp2;

		printf("\tBF(%d,%d,%d,%d,%d)\n",i,last0,dibit0,last1,dibit1);
	}
}
*/
/*
 * Need to look at conditioning the metrics so 0 is 0 and 1 is +1.0, unknown = 0.5
 */
int m17_viterbi_decode(float *in, uint8_t *out, int len){
    // Clear the accumulated metrics
	memset(m_acm,0,sizeof(float)*NR_CONVOLUTIONAL_STATES);
    m_hp = 0;
	// Set state zero as the most likely state m_acm[0] = 1.0;
    m_acm[0] = 1.0;
    for( int i = 0; i < len; i+=2){
		m17_conv_new_metric(in[i], in[i+1]);
	}
	// Do the path traceback
	int bit_length = m_hp;
//	printf("Bit len %d\n",bit_length);
	uint8_t state = 0; // The final state will be 0 (due to the zero stuffing in the tx)

	for( int i = bit_length-1; i >= 0; i-- )
	{
   		state     = m_path[state][i];
		out[i]    = state&0x08?1:0; // MSB of state
	}
	return bit_length;
}
void m17_init_conv(void){
	m17_build_clut_table();
	//code_generate();
}
