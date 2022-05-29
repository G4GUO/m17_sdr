//
// This is where you add new radios
//
#include <math.h>
#include "m17defines.h"

static bool m_afc;
static double m_cor_fac;
static float m_afc_delta;
static double m_ss;

void radio_open(void){
	m_cor_fac = 1.0;
	m_afc_delta = 0;
	lime_open();
	rpi_gpio_open();
}
void radio_close(void){
	lime_close();
	rpi_gpio_close();
}
void radio_transmit(void){
	lime_got_to_transmit();
	lime_ptt_tx(); // Use GPIO on lime
	rpi_tx();      // Use GPIO on RPi
}
void radio_receive(void){
	lime_got_to_receive();
	lime_ptt_rx();
	rpi_rx(); // Use GPIO on RPi
}
void radio_duplex(void){
	lime_got_to_duplex();
	lime_ptt_tx(); // Use GPIO on lime
	rpi_tx();      // Use GPIO on RPi
}

bool radio_keyed(void){
	return lime_read_ptt(); // Use GPIO on lime
	return rpi_read_ptt();  // Use GPIO on RPi
}
void radio_set_tx_frequency(uint64_t freq){
	m17_db_set_tx_freq(freq);
	freq = freq * m_cor_fac;
	lime_set_tx_freq(freq);
}
void radio_set_rx_frequency(uint64_t freq){
	m17_db_set_rx_freq(freq);
	freq = freq * m_cor_fac;
	lime_set_rx_freq(freq);
}
void radio_set_tx_gain(float gain){
	lime_set_tx_gain(gain);
}
void radio_set_rx_gain(float gain){
	lime_set_rx_gain(gain);
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
	int nr = lime_receive_samples((int16_t *)s, n);
	return nr;
}
int radio_transmit_samples( scmplx *s, uint32_t n){
	int nr = lime_transmit_samples((int16_t *)s, n);
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

#define RSSI_DES 4000
#define RSSI_TOL 1500

void radio_rssi_update(void){
    uint32_t rssi = lime_read_rssi();
//    printf("RSSI %.8X\n",rssi);
//    printf("RSSI %d\n",rssi);
    double gain = lime_get_rx_gain();
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
    lime_set_rx_gain(gain);
//    printf("Signal %f \n",m_ss);
    gui_bar(m_ss);

}
