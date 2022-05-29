#include <ncurses.h>
#include <string.h>
#include "m17defines.h"

#define DISPLAY_WIDTH     40
#define DEST_LINE         0
#define SRC_LINE          1
#define RX_FRQ_LINE       2
#define TX_FRQ_LINE       3
#define BAR_LINE          4

#define CURSOR_CMD_LINE   6
#define CURSOR_RSP_LINE   7
#define HELP_LINE         8

int center_offset(const char *txt ){
	int x = strlen(txt);
	int n = 0;
	for( int i = x-1; i >= 0; i--){
		if(txt[i] != ' '){
			n = i;
			break;
		}
	}
	n = (DISPLAY_WIDTH - n)/2;
	return n;
}
//
// Need to save a copy of the input line for when the command prompt is refreshed
//
#define MAX_CMD_LINE 80
static int  m_c;
static char m_cmd[MAX_CMD_LINE+2];

void gui_char(char c){
    if(m_c < MAX_CMD_LINE){
    	m_cmd[m_c++] = c;
    	m_cmd[m_c] = 0;
    }
}
//
// End of command line
//
void gui_char_end(void){
	m_c = 0;
    m_cmd[m_c] = 0;
}

void gui_help(void){
    move(HELP_LINE,0);
    attrset(COLOR_PAIR(1));
	printw("\n");
	printw("tf nnnnnnnnn (set transmit frequency in Hz)\n");
	printw("rf nnnnnnnnn (set receive  frequency in Hz)\n");
	printw("tg           (transmit gain {0 - 1.0})\n");
	printw("rg           (receive gain  {0 - 1.0})\n");
	printw("fc           (frequency correction {0.9 - 1.1})\n");
	printw("afc          (Auto Freq Control {on | off} )\n");
	printw("load file    (load a config file)\n");
	printw("tx           (transmit)\n");
	printw("td           (duplex)\n");
	printw("tc           (carrier)\n");
	printw("rx           (receive)\n");
	printw("sa xxxxxxxxx (source address (len <= 9))\n");
	printw("da xxxxxxxxx (destination address (len <= 9)))\n");
	printw("ba           (broadcast destination)\n");
	printw("q            (quit)\n");
	printw("h            (help)\n");
    attroff(COLOR_PAIR(1));
    gui_cmd_prompt();
}
void gui_open(void){
	initscr();
	cbreak();
	start_color();
	init_color(COLOR_BLACK, 0,0,0);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
}
void gui_close(void){
	endwin();
}
 void gui_update(void){
	int x;
	const M17_Dbase *db = m17_get_db();
	clear();
	// Destination Address
    attrset(COLOR_PAIR(2));
    x = center_offset(db->rx_dest_add);
    move(DEST_LINE,x);
	printw("%s",db->rx_dest_add);
    // Source Address
    attrset(COLOR_PAIR(3));
	x = center_offset(db->rx_src_add);
    move(SRC_LINE,x);
	printw("%s",db->rx_src_add);
    // Transmit and Receive Frequencies
    attrset(COLOR_PAIR(1));
    move(RX_FRQ_LINE,0);
	printw("RX: %lld",db->rx_freq);
    move(TX_FRQ_LINE,0);
	printw("TX: %lld",db->tx_freq);

    gui_cmd_prompt();
}
void gui_cmd_prompt(void){
    attrset(COLOR_PAIR(1));
    move(CURSOR_CMD_LINE,0);
    clrtoeol();
    printw(">> %s",m_cmd);
    attroff(COLOR_PAIR(1));
    refresh();
}
void gui_cmd_resp(const char *resp){
	gui_char_end();
    move(CURSOR_RSP_LINE,0);
    attrset(COLOR_PAIR(1));
    clrtoeol();
	printw("%s",resp);
    attroff(COLOR_PAIR(1));
    move(CURSOR_CMD_LINE,3);
    refresh();
}
void gui_bar(double v){
	char bar[50];
	int m = v*DISPLAY_WIDTH;
	for( int i = 0; i < m; i++){
		bar[i]   ='|';
		bar[i+1] = 0;
	}
    move(BAR_LINE,0);
    clrtoeol();
	printw("%s",bar);
	gui_cmd_prompt();
}
void gui_los(void){
    move(DEST_LINE,0);
    clrtoeol();
    move(SRC_LINE,0);
    clrtoeol();
    gui_cmd_prompt();
    refresh();
}
void gui_aos(void){
}
