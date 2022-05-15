#include <math.h>
#include "m17defines.h"

//
// This module is only for testing.
//
#ifdef __TEST__
#define FN 31
#define OS 2

static float m_rrc_c[FN*OS];
static float m_rrc_s[FN];

static float m_buffer[N_SAMPLES/5];

static float m_lu[4] = {0.3,1.0,-0.3,-1.0};

float m17_test_noise(void){
	float n = ((short)(rand()&0xFFFF)) * 0.000001;
	n = 0;
	return n;
}
void m17_test_update_sym(float sym){
	for( int i = 0; i < FN-1; i++){
		m_rrc_s[i] = m_rrc_s[i+1];
	}
	m_rrc_s[FN-1] = sym;
}
void m17_test_filter(float in, float *out){
	m17_test_update_sym(in);
	out[0] = 0;
	out[1] = 0;
	for( int i = 0; i < FN; i++){
		out[0] += m_rrc_c[(i*OS)+1] * m_rrc_s[i];
		out[1] += m_rrc_c[(i*OS)]   * m_rrc_s[i];
	}

	out[0] += m17_test_noise();
	out[1] += m17_test_noise();
//	printf("%+2.4f  %+2.4f\n",out[0],out[1]);
}
void m17_test_bb(uint8_t *dibit){
	int idx = 0;
	float tempb[N_SAMPLES/5];
	// Write new samples into the buffer
	for( int i = 0; i < 192; i++){
		m17_test_filter(m_lu[dibit[i]], &m_buffer[idx]);
		idx += 2;
	}
	int n = m17_rx_sync_samples( m_buffer, tempb, idx);
	m17_rx_symbols( tempb, n );
}
void m17_disp_float_test(float *in, int len){
	printf("\n");
	for( int i=0; i < len; i++)
		printf("%f\n",in[i]);
}
void m17_test_init(void){
	m17_dsp_build_rrc_filter(m_rrc_c, 0.5, FN*OS, OS);
    m17_dsp_set_filter_gain(m_rrc_c, 1.0, 1, FN*OS);
}
#endif
