#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>
#include "m17defines.h"

static uint8_t m_meta[16];
static pthread_t m_m17_threads[2];
static bool m_running;
//
// Alternative gets
//
char *mfgets(char *b,int n,FILE *fp){
	int i = 0;
	int r;
	for(;;){
		if((r=getch())!= EOF){
			b[i++] = (char)r&0xFF;;
			b[i] = 0;
			gui_char((char)r&0xFF);
			if(r == '\n')  return b;
			if(i >= n - 1) return b;
		}
	}
	return b;
}
void *wait_command_thread(void *arg){
	//
    // Just wait for commands
	//
	char cmd[80];
	m_running = true;

	FILE *fp = stdin;
	gui_cmd_prompt();
	while(m_running == true){
		usleep(10);
		if((mfgets(cmd,80,fp))!= NULL){
			if(cmd[0] == 'q'){
				m17_tx_rx_disable();
				m_running = false;
			}else{
				if(strncmp(cmd,"load",4)==0){
					cmd[strlen(cmd)-1] = 0;// remove line ending
					mmi_load_file(&cmd[5]);
				}else{
				    mmi_parse(cmd);
				}
			}
			gui_cmd_prompt();
		}
	}
	return arg;
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
	m17_rx_sync_init();
#ifdef __TEST__
	printf("TEST MODE TEST MODE TEST MODE TEST MODE\n");
	m17_test_init();
#endif
	radio_open();

	m17_tx_rx_set_src_add("G4GUO    ");
	m17_tx_rx_set_dest_add("G4GUO   ");
	m17_tx_rx_set_brd_add();
	m17_tx_rx_set_type(type);
	m17_tx_rx_set_meta(m_meta);

	radio_set_rx_frequency(434000000);
	radio_set_tx_frequency(434000000);

	m17_tx_rx_ptt_off();

	gui_open();
    gui_update();

	if(pthread_create( &m_m17_threads[0], NULL, m17_tx_rx_thread, NULL ) != 0 )
    {
        printf("Unable to start m17 tx rx thread\n");
        return(0);
    }
	if(pthread_create( &m_m17_threads[1], NULL, wait_command_thread, NULL ) != 0 )
    {
        printf("Unable to start m17 command thread\n");
        return(0);
    }
	// Make sure we don't terminate until all threads have completed
    pthread_join(m_m17_threads[0], NULL);
    pthread_join(m_m17_threads[1], NULL);
	radio_close();
	gui_close();
	return(0);
}
