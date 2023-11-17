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
				m17_txrx_disable();
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
void command_line(int c, char **argv){

	for( int i = 1; i < c; i++){
	}
}
int main( int c, char **argv ){
	M17Type type;
//	gps_open();
//    while(1){
//    	sleep(1);
//    }
	type.p_s      = CCT_STREAM; // A stream
	type.dt       = DATA_VOICE; // of voice
	type.et       = ENC_NONE ;  // With no encryption
	type.est      = ENC_SUB ;   // empty
	type.can      = CAN_NUM;    // empty
	type.reserved = RES_RES;    // not used

	radio_open(RADIO_TYPE_PLUTO);

	m17_prbs9_init();
	m17_crc_init();
	m17_init_conv();
	m17_init_de_correlate();
	m17_dsp_init();
	m17_fmt_init();
	m17_golay_init();
	m17_txrx_init();
	m17_rx_sync_init();
	buff_open();
	gps_open();

#ifdef __TEST__
	printf("TEST MODE TEST MODE TEST MODE TEST MODE\n");
	m17_test_init();
#endif
	m17_mod_init();

	// Default configuration, basic radio
	m17_db_set_chan_type(DRTOAS);
	m17_txrx_set_src_add( "YOURCALL ");
	m17_txrx_set_dest_add("HISCALL  ");
	m17_txrx_set_brd_add();
	m17_txrx_set_type(type);
	m17_txrx_set_meta(m_meta);

	radio_set_rx_frequency(434000000);
	radio_set_tx_frequency(434000000);

	// Start up in receive mode
	m17_txrx_ptt_off();

	// Initialise the GUI
	gui_open();
    gui_update();

    // Load any command line parameters
	command_line( c, argv);

	// Load initialisation
	mmi_load_file("config.txt");

   	if(pthread_create( &m_m17_threads[0], NULL, m17_txrx_threads, NULL ) != 0 )
    {
        printf("Unable to start m17 net threads\n");
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

    m17_net_disconnect_from_reflector();
    sleep(1);
	m17_net_close();
	radio_close();
	gui_close();
	gps_close();
	buff_close();
	return(0);
}
