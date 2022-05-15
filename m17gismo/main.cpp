#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#include "m17defines.h"

static uint8_t m_meta[16];
static pthread_t m_m17_thread;
static bool m_running;

void *m17_thread( void *arg ){
    m17_tx_rx_loop();
	lime_close();
	audio_close();
    return arg;
}
//
// Alternative gets
//
char *mfgets(char *b,int n,FILE *fp){
	int i = 0;
	int r;
	for(;;){
		if((r=fgetc(fp))!= EOF){
			b[i++] = (char)r&0xFF;;
			b[i] =0;
			if(r == '\n')  return b;
			if(i >= n - 1) return b;
		}else{
			usleep(1000);
		}
	}
	return b;
}
void wait_command(void){
	//
    // Just wait for commands
	//
	char cmd[80];

	FILE *fp = stdin;
	m_running = true;
	printf(">> ");
	while(m_running == true){
		usleep(10);
		if((mfgets(cmd,80,fp))!= NULL){
			if(cmd[0] == 'q'){
				m17_tx_rx_disable();
				sleep(1);
				m_running = false;
			}else{
				mmi_parse(cmd);
			    printf(">> ");
			}
		}
	}
}
int main( int c, char **argv ){
	M17Type type;

	type.p_s      = CCT_STREAM; // A stream
	type.dt       = DATA_VOICE; // of voice
	type.et       = ENC_NONE ;  // With no encryption
	type.est      = ENC_SUB ;   // empty
	type.can      = CAN_NUM;    // empty
	type.reserved = RES_RES;    // not used

	m17_prbs9_init();
	m17_crc_init();
	m17_init_conv();
	m17_init_de_correlate();
	m17_dsp_init();
	m17_mod_init();
	m17_fmt_init();
	m17_golay_init();
	m17_tx_rx_init();
	audio_open();
	m17_rx_sync_init();
#ifdef __TEST__
	printf("TEST MODE TEST MODE TEST MODE TEST MODE\n");
	m17_test_init();
#endif
	lime_open();

	m17_tx_rx_set_src_add("G4GUO    ");
	m17_tx_rx_set_dest_add("G4GUO   ");
	m17_tx_rx_set_brd_add();
	m17_tx_rx_set_type(type);
	m17_tx_rx_set_meta(m_meta);

	m17_tx_rx_ptt_off();
	m17_tx_rx_enable();

	if(pthread_create( &m_m17_thread, NULL, m17_thread, NULL ) != 0 )
    {
        printf("Unable to start m17 thread\n");
        m_running = false;
        return(0);
    }
	wait_command();

	return(0);
}
