#include <math.h>
#include <memory.h>
#include <stdio.h>
#include "m17defines.h"

#define TX_FN 31
static float m_tx_c[TX_FN*N_OS];
static float m_tx_s[TX_FN];
static float m_tx_lu[4] = {M_PI/30.0,M_PI/10.0,-M_PI/30,-M_PI/10.0};
// large enough to hold a 40 ms subframe
static scmplx m_tx_samples[N_SAMPLES];
static int m_tx_idx;

static float m_acc;

// Radio specific interface
void m17_mod_tx(scmplx *s, int len){
	lime_transmit_samples((short *)s, len);
}

static void mod_fsk(float *s, int len){
    for( int i = 0;  i < len; i++){
    	m_acc += s[i];
   	    m_tx_samples[m_tx_idx].re =  cos(m_acc)*0x3FFF;
    	m_tx_samples[m_tx_idx].im =  sin(m_acc)*0x3FFF;
    	m_tx_idx++;
    }
    // Check for phase accumulator saturation
	m_acc = m_acc/(2.0*M_PI);
	double ip;
	m_acc = modf(m_acc,&ip);
	m_acc = m_acc*2.0*M_PI;
	// Now check if we have 40 ms of samples to send

	if(m_tx_idx >= N_SAMPLES){
#ifdef __TEST__
		m17_dsp_rx((scmplx*)m_tx_samples, m_tx_idx);
#else
		m17_mod_tx(m_tx_samples,m_tx_idx);// Number of (IQ) samples
#endif
		m_tx_idx = 0;
	}
}
//
// RRC filters and upsamples by 10
//
static float sub_filter(float *s, float *c){
	float sum = s[0]*c[0];
	for( int i = 1; i < TX_FN; i++){
		sum += s[i]*c[i*N_OS];
	}
	return sum;
}
static void mod_filter(float sample){
	// update samples
    for( int i = 0; i < TX_FN-1; i++){
    	m_tx_s[i] = m_tx_s[i+1];
    }
    // Add new sample
    m_tx_s[TX_FN-1] = sample;
    // Do the filtering
    float sum[N_OS];
    sum[0]  = sub_filter(m_tx_s,&m_tx_c[9]);
    sum[1]  = sub_filter(m_tx_s,&m_tx_c[8]);
    sum[2]  = sub_filter(m_tx_s,&m_tx_c[7]);
    sum[3]  = sub_filter(m_tx_s,&m_tx_c[6]);
    sum[4]  = sub_filter(m_tx_s,&m_tx_c[5]);
    sum[5]  = sub_filter(m_tx_s,&m_tx_c[4]);
    sum[6]  = sub_filter(m_tx_s,&m_tx_c[3]);
    sum[7]  = sub_filter(m_tx_s,&m_tx_c[2]);
    sum[8]  = sub_filter(m_tx_s,&m_tx_c[1]);
    sum[9]  = sub_filter(m_tx_s,&m_tx_c[0]);
    mod_fsk(sum, N_OS);
}
//
// Publically visible functions
//
void m17_mod_init(void){
	// Build an oversamples tx filter
	m17_dsp_build_rrc_filter(m_tx_c, 0.5, TX_FN*N_OS, N_OS);
	m17_dsp_set_filter_gain(&m_tx_c[0], 10, 1, TX_FN*N_OS);
	return;
	float m = 0.9;
	m17_dsp_set_filter_gain(&m_tx_c[0], m, N_OS, TX_FN);
	m17_dsp_set_filter_gain(&m_tx_c[1], m, N_OS, TX_FN);
	m17_dsp_set_filter_gain(&m_tx_c[2], m, N_OS, TX_FN);
	m17_dsp_set_filter_gain(&m_tx_c[3], m, N_OS, TX_FN);
	m17_dsp_set_filter_gain(&m_tx_c[4], m, N_OS, TX_FN);
	m17_dsp_set_filter_gain(&m_tx_c[5], m, N_OS, TX_FN);
	m17_dsp_set_filter_gain(&m_tx_c[6], m, N_OS, TX_FN);
	m17_dsp_set_filter_gain(&m_tx_c[7], m, N_OS, TX_FN);
	m17_dsp_set_filter_gain(&m_tx_c[8], m, N_OS, TX_FN);
	m17_dsp_set_filter_gain(&m_tx_c[9], m, N_OS, TX_FN);
}


void m17_mod_dibits(uint8_t *dibits, int len){
    for( int i = 0; i < len; i++){
    	mod_filter(m_tx_lu[dibits[i]]);
    }
#ifdef __TEST__
//    m17_test_bb(dibits);
#endif
}
// Blank carrier
void m17_mod_carrier(void){
    for( int i = 0; i < 192; i++){
    	mod_filter(0);
    }
}

