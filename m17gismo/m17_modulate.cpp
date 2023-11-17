#include <math.h>
#include <memory.h>
#include <stdio.h>
#include "m17defines.h"

#define TX_FN 31
static float *m_tx_c;
static float m_tx_s[TX_FN];
static float m_tx_lu[4] = {M_PI/30.0,M_PI/10.0,-M_PI/30,-M_PI/10.0};
// large enough to hold a 40 ms subframe
static scmplx *m_tx_samples;
static int m_tx_idx;
static float *m_sum;
static float m_acc;
static int m_os;

// Radio specific interface
void m17_mod_tx(scmplx *s, int len){
	radio_transmit_samples(s, len);
}

static void mod_fsk(float *s, int len){
    for( int i = 0;  i < len; i++){
    	m_acc += s[i];
   	    m_tx_samples[m_tx_idx].re =  cos(m_acc)*0x3FFF;
    	m_tx_samples[m_tx_idx].im =  sin(m_acc)*0x3FFF;
    	m_tx_idx++;
    	if(m_tx_idx >= N_SAMPLES){
    		m17_mod_tx(m_tx_samples,m_tx_idx);// Number of (IQ) samples
    		m_tx_idx = 0;
    	}
    }
    // Check for phase accumulator saturation
	m_acc = m_acc/(2.0*M_PI);
	double ip;
	m_acc = modf(m_acc,&ip);
	m_acc = m_acc*2.0*M_PI;
}
//
// RRC filters and upsamples by 10
//
static float sub_filter(float *s, float *c){
	float sum = s[0]*c[0];
	for( int i = 1; i < TX_FN; i++){
		sum += s[i]*c[i*m_os];
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
    for( int i = 0, n = m_os-1; i < m_os; i++, n--){
        m_sum[i] = sub_filter(m_tx_s,&m_tx_c[n]);
    }
    mod_fsk(m_sum, m_os);
}
//
// Publically visible functions
//
void m17_mod_init(void){
	// Build an oversamples tx filter
	m_os         = radio_get_oversample();
	m_sum        = (float*)malloc(sizeof(float)*m_os);
	m_tx_c       = (float*)malloc(sizeof(float)*TX_FN*m_os);
	m_tx_samples = (scmplx*)malloc(sizeof(scmplx)*N_SAMPLES);
	m_tx_idx     = 0;

	m17_dsp_build_rrc_filter(m_tx_c, 0.5, TX_FN*m_os, m_os);
	m17_dsp_set_filter_gain( m_tx_c, 10, 1, TX_FN*m_os);
	return;
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

