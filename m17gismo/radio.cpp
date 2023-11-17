//
// This is where you add new radios
//
#include <math.h>
#include <memory.h>
#include "m17defines.h"

static bool m_afc;
static double m_cor_fac;
static float m_afc_delta;
static double m_ss;
static double m_pwr;
static RadioType m_radio_type;
#define NFIL 31
static scmplx m_rx_buff[N_SAMPLES+NFIL];
static int16_t m_dec_filter[NFIL];

static scmplx sub_filter(scmplx *in, int16_t *coffs ) {
	int32_t real, imag;
	scmplx sum;

    real = in[NFIL/2].re * coffs[NFIL/2];
    imag = in[NFIL/2].im * coffs[NFIL/2];

	for (int i = 0; i < NFIL/2; i++) {
	    real += coffs[i]*(in[i].re + in[(NFIL-1)-i].re);
	    imag += coffs[i]*(in[i].im + in[(NFIL-1)-i].im);
	}
	sum.re = real >> 15;
	sum.im = imag >> 15;
	return sum;
}
//
// Filter and decimate by 10
//
static void rx_decimate_filter(scmplx *in, scmplx *out){
    for( int i = 0; i < N_SAMPLES/8; i++){
    	out[i] = sub_filter(&in[i*8], m_dec_filter);
    }
}
//
// Filter used to decimate the receive samples.
//
void build_pluto_rx_dec_filter(void){
	float filter[NFIL];

	m17_dsp_build_lpf_filter( filter, 0.125f, NFIL);
	m17_dsp_set_filter_gain(filter, 0.9, 1, NFIL);
	m17_dsp_float_to_short(filter, m_dec_filter, NFIL);

}
int radio_open(RadioType type){
	m_radio_type = type;

	m_cor_fac = 1.0;
	m_afc_delta = 0;
	if( m_radio_type == RADIO_TYPE_LIME ) lime_open();
	if( m_radio_type == RADIO_TYPE_PLUTO ){
		uint32_t srate = 48000;
		pluto_open(N_SAMPLES);
		pluto_set_rx_sample_rate(   srate*64 );
		pluto_set_tx_sample_rate(   srate*64 );
		pluto_configure_x8_int_dec( srate*8 );
		build_pluto_rx_dec_filter();
	}
	rpi_gpio_open();
	return 0;
}
void radio_close(void){
	if( m_radio_type == RADIO_TYPE_LIME ) lime_close();
	if( m_radio_type == RADIO_TYPE_PLUTO ) pluto_close();
	rpi_gpio_close();
}
void radio_transmit(void){
	if( m_radio_type == RADIO_TYPE_LIME ){
	    lime_got_to_transmit();
	    lime_ptt_tx(); // Use GPIO on lime
	}
	if( m_radio_type == RADIO_TYPE_PLUTO ){
	    pluto_stop_rx_stream();
	    pluto_start_tx_stream();
	}
	rpi_tx();      // Use GPIO on RPi
	gui_tx();
}
void radio_receive(void){
	if( m_radio_type == RADIO_TYPE_LIME ){
		lime_got_to_receive();
        lime_ptt_rx();
	}
	if( m_radio_type == RADIO_TYPE_PLUTO ){
	    pluto_stop_tx_stream();
	    pluto_start_rx_stream();
	}
	rpi_rx(); // Use GPIO on RPi
	gui_rx();
}
void radio_duplex(void){
	if( m_radio_type == RADIO_TYPE_LIME ){
		lime_got_to_duplex();
	    lime_ptt_tx(); // Use GPIO on lime
	}
	if( m_radio_type == RADIO_TYPE_PLUTO ){
	    pluto_start_rx_stream();
	    pluto_start_tx_stream();
	}
	rpi_tx();      // Use GPIO on RPi
	gui_dp();
}

bool radio_keyed(void){
	return lime_read_ptt(); // Use GPIO on lime
	return rpi_read_ptt();  // Use GPIO on RPi
}
void radio_set_tx_frequency(uint64_t freq){
	m17_db_set_tx_freq(freq);
	freq = freq * m_cor_fac;
	if( m_radio_type == RADIO_TYPE_LIME )  lime_set_tx_freq(freq);
	if( m_radio_type == RADIO_TYPE_PLUTO ) pluto_set_tx_freq(freq);
}
void radio_set_rx_frequency(uint64_t freq){
	m17_db_set_rx_freq(freq);
	freq = freq * m_cor_fac;
	if( m_radio_type == RADIO_TYPE_LIME )  lime_set_rx_freq(freq);
	if( m_radio_type == RADIO_TYPE_PLUTO ) pluto_set_rx_freq(freq);
}
void radio_set_tx_gain(float gain){
	m_pwr = gain;
	if( m_radio_type == RADIO_TYPE_LIME ) lime_set_tx_gain(gain);
	if( m_radio_type == RADIO_TYPE_PLUTO ){
		gain = -(1.0 - gain)*89.0;
		if( gain > 0 ) gain = 0;
		if( gain < -89.0 ) gain = -89;
		pluto_set_tx_level(gain);
	}
}
void radio_set_rx_gain(float gain){
	if( m_radio_type == RADIO_TYPE_LIME ) lime_set_rx_gain(gain);
	if( m_radio_type == RADIO_TYPE_PLUTO ){
		gain = -(1.0 - gain)*89.0;
		if( gain > 0 ) gain = 0;
		if( gain < -89.0 ) gain = -89;
	//	pluto_set_rx_level(gain);
	}
}
void radio_set_afc_on(void){
    m_afc = true;
}
void radio_set_afc_off(void){
	m_afc_delta = 0;
    m_afc = false;
}
bool radio_get_afc_status(void){
    return m_afc;
}

int radio_receive_samples( scmplx *s, uint32_t n){
	int nr = 1920;
	if( m_radio_type == RADIO_TYPE_LIME ){
		nr = lime_receive_samples((int16_t *)s, n);
	}
	if( m_radio_type == RADIO_TYPE_PLUTO ){
		// THe Pluto requires a high sample rate to work
		// but crashes if it's buffers are too big
		// So we do it in small chunks
		int op = 0;
		for( int i = 0; i < 8; i++){
			// Update buffer
			memcpy(m_rx_buff, &m_rx_buff[N_SAMPLES],sizeof(scmplx)*NFIL);
			nr = pluto_rx_samples( &m_rx_buff[NFIL] );
			// Filter and decimate the samples
			rx_decimate_filter(m_rx_buff, &s[op]);
			op += 240;
		}
	}
	return nr;
}
int radio_transmit_samples( scmplx *s, uint32_t n){
	int nr = 0;
	if( m_radio_type == RADIO_TYPE_LIME ){
	    nr = lime_transmit_samples((int16_t *)s, n);
	}
	if( m_radio_type == RADIO_TYPE_PLUTO ){
	    pluto_tx_samples( s, n);
	}
	return nr;
}
void radio_set_freq_correction_factor( double fac){
	if((fac > 0.9)&&(fac < 1.1)){
	    m_cor_fac = fac;
	}
}
//
// Automatic frequency control
//
void radio_afc(float mean){
	if((m_afc == true)&&(m17_db_in_frame()==true)){
		m_afc_delta -= mean * 0.1;
	}
}
float radio_get_afc_delta(void){
	if((m_afc == true)&&(m17_db_in_frame()==true)){
		return m_afc_delta;
	}else{
		m_afc_delta = 0;
		return 0;
	}
}
// return the required over sampling for the SDR

int radio_get_oversample(void){
	if( m_radio_type == RADIO_TYPE_LIME ){
	    return 10;
	}
	if( m_radio_type == RADIO_TYPE_PLUTO ){
	    return 80;
	}
	return 10;
}

#define RSSI_DES 4000
#define RSSI_TOL 1500

void radio_rssi_update(void){

    if( m_radio_type == RADIO_TYPE_LIME ){
        uint32_t  rssi = lime_read_rssi();
//    printf("RSSI %.8X\n",rssi);
//    printf("RSSI %d\n",rssi);
        double gain = 1.0;
        gain = lime_get_rx_gain();

        if((rssi > (RSSI_DES-RSSI_TOL))&&(rssi < (RSSI_DES+RSSI_TOL))){
    	    //RSSI is OK
            if( rssi > RSSI_DES ) gain -= 0.0001;
            if( rssi < RSSI_DES ) gain += 0.0001;
        }else{
            // Need to rapid adjust the gain
            if( rssi > RSSI_DES ) gain -= 0.1;
            if( rssi < RSSI_DES ) gain += 0.1;
        }
        if( gain > 1.0 )      gain  = 1.0;
        if( gain < 0.1 )      gain  = 0.1;
        // Psuedo signal strength
        m_ss = 1.0 - gain;
    //    printf("GAIN %f    %f\n",gain,m_ss);
	    if( m_radio_type == RADIO_TYPE_LIME ) lime_set_rx_gain(gain);
    //    printf("Signal %f \n",m_ss);
    }

    if( m_radio_type == RADIO_TYPE_PLUTO ){
    	long long int rssi;
    	pluto_read_rssi_value(&rssi);
    	// Max signal == 40
    	// Min signal == 125
    	double v = rssi;
    	//printf("%f\n\r",v);
    	v = v/130.0;
    	v = 1.0 - v;
    	m_ss = v;
    }
	if(m_ss > 1.0 ) m_ss = 1.0;
	if(m_ss < 0.0 ) m_ss = 0.0;
    gui_bar(m_ss);
}
void radio_power_update(void){
    gui_bar(m_pwr);
}
