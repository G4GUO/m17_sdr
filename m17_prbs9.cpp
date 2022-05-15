#include "m17defines.h"

#define PRBS9_LEN 511

static uint8_t m_prbs9[PRBS9_LEN];
static uint16_t m_tx_idx;
static uint16_t m_rx_idx;
static int m_rx_state;
static uint16_t m_rx_bad;
static uint16_t m_rx_good;
static uint16_t m_rx_eq_cnt;
static uint16_t m_rx_dif_cnt;
#define NO_SYNC 0
#define IN_SYNC 1

void generate_prbs9(void){
	uint16_t sr;
	uint16_t st;
	uint16_t cnt = 0;
	sr = st = 0x01;
	do{
		uint8_t bit = ((sr>>8)^(sr>>4))&1;
		sr = ((sr<<1)|bit)&0x1FF;
		m_prbs9[cnt++] = bit;
	}while( sr != st );
}
void m17_prbs9_tx_load(uint8_t *out, int len){
	for( int i = 0; i < len; i++){
	    out[i] = m_prbs9[m_tx_idx];
	    m_tx_idx = (m_tx_idx + 1)%PRBS9_LEN;
	}
}
void m17_prbs9_tx_reset(void){
	m_tx_idx = 0;
}
void m17_prbs9_rx_reset(void){
	m_rx_idx   = 0;
	m_rx_state = NO_SYNC;
}
int m17_prbs9_rx_check(uint8_t bit){
    uint8_t d = bit ^ m_prbs9[m_rx_idx];

	if( d ){
		m_rx_dif_cnt++;
		m_rx_eq_cnt = 0;
	}else{
		m_rx_eq_cnt++;
		m_rx_dif_cnt = 0;
	}
    m_rx_idx = (m_rx_idx + 1)%PRBS9_LEN;

    if( m_rx_state == NO_SYNC ){
    	// We are hunting for sync
    	m_rx_bad  = 0;
    	m_rx_good = 0;
	if(m_rx_eq_cnt >= 18 ) m_rx_state = IN_SYNC;
		if(d) m17_prbs9_rx_reset();// Bits not the same, reset the sync vector
    }else{
		if(m_rx_dif_cnt >= 18 ) m_rx_state = NO_SYNC;
    	if(d) m_rx_bad++;
    	if(d == 9) m_rx_good++;
    }
	return 0;
}
void m17_prbs9_init(void){
	m17_prbs9_tx_reset();
	m17_prbs9_rx_reset();
	m_tx_idx = 0;
	generate_prbs9();
}
