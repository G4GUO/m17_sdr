#include <string.h>
#include <stdlib.h>
#include "m17defines.h"

void help(void){
	printf("\n");
	printf("f 437000000 (set frequency to 437 MHz)\n");
	printf("t           (transmit)\n");
	printf("r           (receive)\n");
	printf("q           (quit)\n");
	printf("h           (help)\n");
}

void mmi_parse(char *cmd){
	int nt = 0;
	bool valid = false;
	char *token[10];
	token[nt] = strtok(cmd," ");
	while(token[nt] != NULL){
		if( ++nt == 10 ) break;
		token[nt] = strtok(NULL," ");
	}
	if(strncmp(token[0],"g",1)==0){
		if(	nt == 2 ){
			float gain = atof(token[1]);
			if((gain <= 1.0)&&(gain>0)){
				//lime_set_gain(gain);
				valid = true;
			}
		}
	}
	if(strncmp(token[0],"f",1)==0){
		if(	nt == 2 ){
			uint64_t freq = atoll(token[1]);
			lime_set_freq(freq);
			valid = true;
		}
	}
	if(strncmp(token[0],"q",1)==0){
		m17_tx_rx_disable();
	}

	if(strncmp(token[0],"t",1)==0){
		m17_tx_rx_ptt_on();
		valid = true;
	}
	if(strncmp(token[0],"r",1)==0){
		m17_tx_rx_ptt_off();
		valid = true;
	}
	if(strncmp(token[0],"h",1)==0){
		help();
		valid = true;
	}

	if(valid == false) printf("Invalid command\n");
}
