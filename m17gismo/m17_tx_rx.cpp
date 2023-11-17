#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <memory.h>
#include "m17defines.h"
#include "codec2.h"

typedef enum{PTT_RX,PTT_TX,PTT_CA,PTT_DP}PttType;
typedef enum{STATE_RX,STATE_TX,STATE_DP}NetState;

static PttType m_ptt;
static struct CODEC2 *m_codec2;
static int m_nsam;
static int16_t m_amicsamples[160];
static int16_t m_aspksamples[160];
static scmplx  m_rsamples[1920];
static M17Type m_type;
static uint8_t m_meta[14];
static bool m_running;
static NetState m_net_state = STATE_RX;

void m17_txrx_spkr_audio( uint8_t *data ){

   codec2_decode(m_codec2, m_aspksamples, data);
   audio_output(m_aspksamples, m_nsam);
}

void *m17_txrx_net_thread(void *arg){

	m_net_state = STATE_RX;
    radio_receive();

    while(m_running == true){

    	if(m_net_state == STATE_RX){
		    int len = radio_receive_samples( m_rsamples, N_SAMPLES );
		    m17_dsp_rx( m_rsamples, len );
			radio_rssi_update();
			int n = buff_size();
			if( n > 15 ){
				uint8_t *b = buff_get();
				if(b != NULL){
					// We have something to transmit
			        radio_transmit();
			        m17_send_carrier();
			        m17_send_preamble();
			        m17_send_preamble();
			        m17_send_link_setup_frame_fm_net(b);
				    // Send Sub frame
				    m17_send_stream_frame_fm_net(b);
				    m_net_state = STATE_TX;
			    }
			}
        }

		if(m_net_state == STATE_TX){
			uint8_t *b = buff_get();
			if(b != NULL){
				// We have something to transmit
			    // Send Sub frame
			    m17_send_stream_frame_fm_net(b);
			    radio_power_update();
			}else{
				// Back to receive
			    // Send EOT
			    m17_send_eot();
			    usleep(40000);
			    // Go to receive
			    radio_receive();
			    m_net_state = STATE_RX;
			}
        }

        if(m_net_state == STATE_DP){

        }

    }
     // Close the system
    return arg;
}

void *m17_txrx_thread(void *arg){
    uint8_t payload[16];

    while(m_running == true){
        // Simplex transmit
		if( m_ptt == PTT_TX){
		    audio_mic_open();
		    usleep(120000);// Buffer 6 sound blocks
		    // Go to transmit
		    //radio_duplex();
		    radio_transmit();
	        // Send preamble
		    m17_send_carrier();
		    m17_send_preamble();
		    m17_send_preamble();
		    // Send link setup
		    m17_send_link_setup_frame(m17_db_get_dst(), m17_db_get_src(), m_type, m_meta);
		    // Send sub frames of voice, 25 sub frames per sec
		    while((m_ptt == PTT_TX)&&(m_running == true)){
			    // Read Audio samples and encode using Codec 2
			    // Audio blocks are 20 ms but M17 chunks are 40 ms so 2 codec frames per m17 sub frame
			    audio_input(m_amicsamples, m_nsam);
			    codec2_encode(m_codec2, &payload[0], m_amicsamples);
			    // Read Audio samples and encode using Codec 2
			    audio_input(m_amicsamples, m_nsam);
			    codec2_encode(m_codec2, &payload[8], m_amicsamples);
			    // Send Sub frame
			    m17_send_stream_frame( payload );
			    radio_power_update();
		    }
		    // Send EOT
		    m17_send_eot();
		    usleep(40000);
		    audio_mic_close();
		    // Go to receive
		    radio_receive();
        }

		// Full Duplex
		if( m_ptt == PTT_DP){
		    audio_mic_open();
		    audio_spk_open();
		    usleep(120000);// Buffer 6 sound blocks
		    // Go to transmit
		    radio_duplex();
	        // Send preamble
		    m17_send_carrier();
		    m17_send_preamble();
		    m17_send_preamble();
		    // Send link setup
		    m17_send_link_setup_frame(m17_db_get_dst(), m17_db_get_src(), m_type, m_meta);
		    // Send sub frames of voice, 25 sub frames per sec
		    while((m_ptt == PTT_DP)&&(m_running == true)){
			    // Read Audio samples and encode using Codec 2
			    // Audio blocks are 20 ms but M17 chunks are 40 ms so 2 codec frames per m17 sub frame
			    audio_input(m_amicsamples, m_nsam);
			    codec2_encode(m_codec2, &payload[0], m_amicsamples);
			    // Read Audio samples and encode using Codec 2
			    audio_input(m_amicsamples, m_nsam);
			    codec2_encode(m_codec2, &payload[8], m_amicsamples);
			    // Send Sub frame
			    m17_send_stream_frame( payload );
			    // Receive audio
			    int len = radio_receive_samples( m_rsamples, N_SAMPLES );
			    m17_dsp_rx( m_rsamples, len);
				radio_rssi_update();
		    }
		    // Send EOT
		    m17_send_eot();
		    usleep(40000);
		    audio_mic_close();
		    audio_spk_close();
		    // Go to receive
		    radio_receive();
        }

        // Simplex receive
		if( m_ptt == PTT_RX){
		    radio_receive();
		    audio_spk_open();
			while((m_ptt == PTT_RX)&&(m_running == true)){
			    int len = radio_receive_samples( m_rsamples, N_SAMPLES );
			    m17_dsp_rx( m_rsamples, len );
			    // Throw input samples away
				radio_rssi_update();
			}
		    audio_spk_close();
		}

        // Simplex transmit carrier
		if( m_ptt == PTT_CA){
		    audio_spk_close();
		    audio_mic_close();
		    radio_transmit();
			while((m_ptt == PTT_CA)&&(m_running == true)){
			    m17_send_carrier();
			}
		}
	}
    // Close the system
    return arg;
}

void *m17_txrx_client_thread(void *arg){
    uint8_t payload[16];

    while(m_running == true){
        // Simplex transmit
		if( m_ptt == PTT_TX){
		    audio_mic_open();
		    // Send link setup
		    m17_send_link_setup_frame_to_net(m17_db_get_dst(), m17_db_get_src(), m_type, m_meta);
		    // Send sub frames of voice, 25 sub frames per sec
		    while((m_ptt == PTT_TX)&&(m_running == true)){
			    // Read Audio samples and encode using Codec 2
			    // Audio blocks are 20 ms but M17 chunks are 40 ms so 2 codec frames per m17 sub frame
			    audio_input(m_amicsamples, m_nsam);
			    codec2_encode(m_codec2, &payload[0], m_amicsamples);
			    // Read Audio samples and encode using Codec 2
			    audio_input(m_amicsamples, m_nsam);
			    codec2_encode(m_codec2, &payload[8], m_amicsamples);
			    // Send Sub frame
			    m17_send_stream_frame_to_net( payload );
			    radio_power_update();
		    }
		    audio_mic_close();
        }

        // Simplex receive
		if( m_ptt == PTT_RX){
		    audio_spk_open();
			while((m_ptt == PTT_RX)&&(m_running == true)){
				usleep(1000);
			}
		}
	}
    return arg;
}
void *m17_txrx_audio_loopback(void *arg){
    uint8_t payload[16];
    while(m_running == true){
        // Simplex transmit
		if( m_ptt == PTT_TX){
	        audio_input(m_amicsamples,  m_nsam);
		    codec2_encode(m_codec2, &payload[0], m_amicsamples);
		    m17_txrx_spkr_audio( payload );
        }else{
	        audio_input(m_amicsamples,  m_nsam);
        }
    }
    return arg;
}
//
// This is the thread function
//
void *m17_txrx_threads(void *arg){
    m_running   = true;
    m_net_state = STATE_RX;
    m_ptt = PTT_RX;

    audio_mic_open();
    audio_spk_open();

    while(m_running == true){
        // decide which sub thread should be running
    	if(m17_db_get_chan_type() == DRTOAS) m17_txrx_thread(NULL);
    	if(m17_db_get_chan_type() == ASTODN) m17_txrx_client_thread(NULL);
    	if(m17_db_get_chan_type() == DRTODN) m17_txrx_net_thread(NULL);
    	if(m17_db_get_chan_type() == ASTOAS) m17_txrx_audio_loopback(NULL);
    }
    // Close the system
    audio_spk_close();
    audio_mic_close();
	return arg;
}
//
// If listen all is true then it decodes all addresses
// if false then only decodes own and broadcast address
//
void m17_txrx_listen_all( bool b ){
    m17_db_listen_all( b );
}
//
// Set the own address
//
void m17_txrx_set_src_add(const char *add){
	m17_db_set_src( add );
}
//
// Set the local gateway address
//
void m17_txrx_set_gate_add(const char *add){
	m17_db_set_gate( add );
}
//
// Set the address of station you are calling
//
void m17_txrx_set_dest_add(const char *add){
	m17_db_set_dst( add );
}
//
// Set address to call everyone
//
void m17_txrx_set_brd_add(void){
	m17_db_set_brd();
}

void m17_txrx_set_type(M17Type type){
    m_type = type;
}
void m17_txrx_set_meta(uint8_t *meta){
    memcpy(m_meta,meta,sizeof(uint8_t)*14);
}
//
// Go to transmit
//
void m17_txrx_ptt_on(void){
    m_ptt = PTT_TX;
    m17_db_set_ptt(false);
}
//
// Go to receive
//
void m17_txrx_ptt_off(void){
    m_ptt = PTT_RX;
    m17_db_set_ptt(false);
}
//
// Transmit a carrier
//
void m17_txrx_ptt_carrier(void){
    m_ptt = PTT_CA;
    m17_db_set_ptt(true);
}
void m17_txrx_ptt_duplex(void){
    m_ptt = PTT_DP;
}
void m17_txrx_enable(void){
	m_running = true;
}
// Close the system
void m17_txrx_disable(void){
	m_running = false;
}
// Init this module
void m17_txrx_init(void){
	m_codec2 = codec2_create(CODEC2_MODE_3200);
	m_nsam   = codec2_samples_per_frame(m_codec2);
	m17_db_listen_all( false );
}

