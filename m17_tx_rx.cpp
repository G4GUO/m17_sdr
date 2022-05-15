#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <memory.h>
#include "m17defines.h"
#include "codec2.h"

static bool m_ptt;
static struct CODEC2 *m_codec2;
static int m_nsam;
static int16_t m_amicsamples[160];
static int16_t m_aspksamples[160];
static scmplx  m_rsamples[1920];
static M17Type m_type;
static uint8_t m_meta[14];
static bool m_running;

void m17_tx_rx_rx_spkr_audio( uint8_t *data ){

   codec2_decode(m_codec2, m_aspksamples, data);
   audio_output(m_aspksamples, m_nsam);
//   printf("Audio ");
//   for(int i = 0; i < 8; i++ ) printf("%.2X ",data[i]);
//   printf("\n");

}
void m17_tx_rx_loop(void){
    uint8_t payload[16];

	while(m_running){

		if( m_ptt == true){
		    // Go to transmit
			audio_input_flush();
			lime_got_to_duplex();
		    //lime_got_to_transmit();
	       // Send preamble
		    m17_send_carrier();
		    m17_send_carrier();
		    m17_send_preamble();
		    m17_send_preamble();
		    // Send link setup
		    m17_send_link_setup_frame(m17_db_get_dst(), m17_db_get_src(), m_type, m_meta);
		    // Send sub frames of voice, 25 sub frames per sec
		    while(m_ptt == true ){
			    // Read Audio samples and encode using Codec 2
			    // Audio blocks are 20 ms but M17 chunks are 40 ms so 2 codec frames per m17 sub frame
			    audio_input(m_amicsamples, m_nsam);
			    codec2_encode(m_codec2, &payload[0], m_amicsamples);
			    // Read Audio samples and encode using Codec 2
			    audio_input(m_amicsamples, m_nsam);
			    codec2_encode(m_codec2, &payload[8], m_amicsamples);
			    // Send Sub frame
			    //memset(payload,0x55,16);
			    m17_send_stream_frame( payload );
			    int len = lime_receive_samples( (short*)m_rsamples, N_SAMPLES );
			    m17_dsp_rx( m_rsamples, len);
		    }
		    // Send EOT
		    m17_send_eot();
		    m17_send_eot();
		    usleep(100);
		    // Go to receive
		    lime_got_to_receive();
        }

		if( m_ptt == false){
			audio_output_flush();
		    lime_got_to_receive();
			while(m_ptt == false){
			    int len = lime_receive_samples( (short*)m_rsamples, N_SAMPLES );
			    m17_dsp_rx( m_rsamples, len);
			}
		    lime_got_to_transmit();
		}
	}
}
//
// If listen all is true then it decodes all addresses
// if false then only decodes own and broadcast address
//
void m17_tx_rx_listen_all( bool b ){
    m17_db_listen_all( b );
}
//
// Set the own address
//
void m17_tx_rx_set_src_add(const char *add){
	m17_db_set_src( add );
}
//
// Set the address of station you are calling
//
void m17_tx_rx_set_dest_add(const char *add){
	m17_db_set_dst( add );
}
//
// Set address to call everyone
//
void m17_tx_rx_set_brd_add(void){
	m17_db_set_brd();
}

void m17_tx_rx_set_type(M17Type type){
    m_type = type;
}
void m17_tx_rx_set_meta(uint8_t *meta){
    memcpy(m_meta,meta,sizeof(uint8_t)*14);
}
//
// Go to transmit
//
void m17_tx_rx_ptt_on(void){
    m_ptt = true;
}
//
// Go to receive
//
void m17_tx_rx_ptt_off(void){
    m_ptt = false;
}
void m17_tx_rx_enable(void){
	m_running = true;
}
void m17_tx_rx_disable(void){
	m_running = false;
}
void m17_tx_rx_init(void){
	m_codec2 = codec2_create(CODEC2_MODE_3200);
	m_nsam   = codec2_samples_per_frame(m_codec2);
	m17_db_listen_all( false );

}

