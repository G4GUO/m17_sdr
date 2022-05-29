#include "m17defines.h"

#define NF  40
#define FN  31
#define THS 50

static int m_clk;
static int m_epo;
static int m_thr;
static int m_index;

static float m_buff[FN];
static float m_mf[NF][FN];
static float m_md[NF][FN];

void display_filters(void){
	printf("RRC\n");
	for( int i = 0; i < NF; i++){
		printf("N: %d\n",i);
		for( int j = 0; j < FN; j++){
			printf("%f\n",m_mf[i][j]);
		}
	}
}
float rx_sync_filter(float *in, float *c, int len){
	float sum = in[0]*c[0];
	for( int i = 1; i < len; i++){
        sum += in[i]*c[i];
	}
	return sum;
}
void rx_sync_update( float in ){
	for( int i = 0; i < FN-1; i++){
		m_buff[i] = m_buff[i+1];
	}
	m_buff[FN-1] = in;
}
void sync_update( float sum, float dif){
	if( sum < 0 ) dif = -dif;
	if( dif > 0 ) m_thr++;
	if( dif < 0 ) m_thr--;
}
static int m_idx;

void m17_sync_adjust( int thresh, float *out ){
	if( m_thr > thresh ){
		// Move the sync point forwards (mod NF)
		m_index = (m_index+1)%NF;
		m_thr = 0;
#ifdef __TRACE__
		printf("Sync + %d\n",m_index);
#endif
	    if( m_index == 0 ){
	    	m_clk = 1;
	    	//bit slip
	    	out[m_idx++] = 0;// Set to unknown
	    }
	}
	if( m_thr < -thresh ){
		m_thr = 0;
		// Move the sync point backwards (mod NF)
		m_index = (m_index+NF-1)%NF;
#ifdef __TRACE__
		printf("Sync - %d\n",m_index);
#endif
	    if( m_index == (NF-1) ){
	    	m_clk = 1;
	    	// bit slip
	    	m_idx--;
	    }
	}
}
//
// input an array of samples at 2x symbol rate
// out put symbols clock aligned
//
int m17_rx_sync_samples( float *in, float *out, int len){
	static float dif,sum;
	m_idx = 0;
	for( int i = 0; i < len; i++){
		rx_sync_update(in[i]);
		m_clk = (m_clk+1)%2;
		if( m_clk ){
			sum = rx_sync_filter(m_buff, m_mf[m_index], FN);
			dif = rx_sync_filter(m_buff, m_md[m_index], FN);
			out[m_idx++] = sum;
		   // printf("%+2.4f\n",sum);
		}else{
			// Decide whether we need to move the optimum sync index
			sync_update( sum, dif );
			// Sync threshold is slower when locked
			if(m17_rx_lock() == false)
			    m17_sync_adjust(10, out);
			else
			    m17_sync_adjust(80, out);
		}
	}
	return m_idx;
}

void m17_rx_sync_init(void){
    // build matched filter
	// 2 samples per symbol, 40 filters, 17 taps per filter
	// number of taps for mother filter 40x17 =680
	// samples per symbol 2x40 = 80
	float mf[NF*FN];
	m17_dsp_build_rrc_filter(mf, 0.5, NF*FN, NF*2);
	// Build differential filter
	float md[NF*FN];
	for( int i = 0; i < NF*FN; i++){
		md[i] = mf[(i+1)%(NF*FN)] - mf[(i+(NF*FN)-1)%(NF*FN)];
	}
	// Partition both filters
    for( int i = 0; i < NF; i++){
    	for(int j = 0; j < FN; j++){
    		m_mf[i][j] = mf[i+(j*NF)];
    	    m_md[i][j] = md[i+(j*NF)];
    	}
    }
    for( int i = 0; i < NF; i++){
        m17_dsp_set_filter_gain(m_mf[i], 1.0, 1,FN);
    }
    m_clk = 1;
    m_epo = 0;
    m_thr = 0;
    m_index = 10;

    //display_filters();
}
