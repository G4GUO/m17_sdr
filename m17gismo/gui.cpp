#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "m17defines.h"

#define DISPLAY_WIDTH     80
#define MODE_LINE         0
#define BAR_LINE          1
#define DEST_LINE         2
#define SRC_LINE          3
#define RX_FRQ_LINE       5
#define TX_FRQ_LINE       6

#define CURSOR_CMD_LINE   8
#define CURSOR_RSP_LINE   7
#define HELP_LINE         15

static uint48_t m_src_add;
static uint48_t m_dst_add;
static char m_mode[15];

#define RX 0
#define TX 1
#define DP 2

static int m_tx_rx;
static int m_last_bar;

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

void display_tx_rx(void){
	if( m_tx_rx == RX ){
	    move(MODE_LINE,35);
	    attrset(COLOR_PAIR(2));
		printw("RX");
	    attroff(COLOR_PAIR(2));
	}
	if( m_tx_rx == TX ){
	    move(MODE_LINE,35);
	    attrset(COLOR_PAIR(4));
		printw("TX");
	    attroff(COLOR_PAIR(4));
	}
	if( m_tx_rx == DP ){
	    move(MODE_LINE,35);
	    attrset(COLOR_PAIR(3));
		printw("DX");
	    attroff(COLOR_PAIR(3));
	}
}
void clear_callsigns(void){
    move(DEST_LINE,5);
    clrtoeol();
    move(SRC_LINE,5);
    clrtoeol();
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
	printw("tx           (transmit in simplex)\n");
	printw("td           (transmit in duplex)\n");
	printw("tc           (transmit carrier)\n");
	printw("rx           (receive)\n");
	printw("sa xxxxxxxxx (source  address (len <= 9))\n");
	printw("ga xxxxxxxxx (gateway address (len <= 9))\n");
	printw("da xxxxxxxxx (destination address (len <= 9)))\n");
	printw("ba           (destination broadcast)\n");
	printw("conn M17 C   (connect to M17-M17 reflector module C)\n");
	printw("disc         (disconnect from current reflector)\n");
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
    init_pair(4, COLOR_RED, COLOR_BLACK);
}
void gui_close(void){
	endwin();
}
void gui_status_line(void){

	// Mode
    move(MODE_LINE,0);
    attrset(COLOR_PAIR(1));
	printw("MODE:");
    attrset(COLOR_PAIR(2));
    move(MODE_LINE,6);
    printw("%s",m_mode);

    move(MODE_LINE,12);
    if(m17_net_connected()==true){
        attrset(COLOR_PAIR(3));
    	printw("CONN");
    	char call[15];
    	m17_net_ref_address(call);
    	move(MODE_LINE,18);
        attrset(COLOR_PAIR(1));
    	printw("%s",call);
	    attroff(COLOR_PAIR(1));
    }else{
        attrset(COLOR_PAIR(3));
	    printw(" ");
	    attroff(COLOR_PAIR(3));
    }
    attroff(COLOR_PAIR(3));
	// TX RX status
	display_tx_rx();
}
void gui_update(void){
//	int x;
	char call[15];
	const M17_Dbase *db = m17_get_db();
//	clear();
	gui_status_line();

	// Destination Address
	m17_decode_call(m_dst_add, call);
    move(DEST_LINE,0);
    attrset(COLOR_PAIR(1));
	printw("DEST:");
    attrset(COLOR_PAIR(3));
    move(DEST_LINE,5);
	printw(" %s",call);

	// Source Address
	m17_decode_call(m_src_add, call);
    move(SRC_LINE,0);
    attrset(COLOR_PAIR(1));
	printw("SRC :");
    attrset(COLOR_PAIR(3));
    move(SRC_LINE,5);
	printw(" %s",call);

	// Transmit and Receive Frequencies
    attrset(COLOR_PAIR(1));
    move(RX_FRQ_LINE,0);
	printw("RX: %lud Hz",db->rx_freq);
    move(TX_FRQ_LINE,0);
	printw("TX: %lud Hz",db->tx_freq);

    gui_cmd_prompt();
    refresh();
}
void gui_update_clear(void){
	clear();
	gui_update();
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
	int m = v*DISPLAY_WIDTH;
	if(m != m_last_bar){
        attrset(COLOR_PAIR(2));
        move(BAR_LINE,0);
        clrtoeol();
        for( int i = 0; i < m; i++){
    	    if(i == 15) attrset(COLOR_PAIR(3));
    	    if(i == 25) attrset(COLOR_PAIR(4));
    	    printw("|");
        }
        attrset(COLOR_PAIR(1));
	    gui_cmd_prompt();
	}
	m_last_bar = m;
}
void gui_los(void){
	clear_callsigns();
    gui_cmd_prompt();
    refresh();
}
void gui_aos(void){
}
void gui_save_src_address(uint48_t add){
    m_src_add = add;
}
void gui_save_dest_address(uint48_t add){
	m_dst_add = add;
}
void gui_save_mode(const char *mode){
	for(uint16_t i = 0; i < strlen(mode);i++){
		m_mode[i]   = toupper(mode[i]);
		m_mode[i+1] = 0;
	}
}
void gui_tx(void){
	clear_callsigns();
	m_tx_rx = TX;
	gui_update();
}
void gui_rx(void){
	clear_callsigns();
	m_tx_rx = RX;
	gui_update();
}
void gui_dp(void){
	m_tx_rx = DP;
	gui_update();
}
