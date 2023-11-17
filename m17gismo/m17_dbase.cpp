#include <string.h>
#include "m17defines.h"


M17_Dbase m17_db;

const M17_Dbase *m17_get_db( void ){
    return(&m17_db);
}
void m17_db_set_tx_freq(uint64_t freq){
	m17_db.tx_freq = freq;
}
void m17_db_set_rx_freq(uint64_t freq){
	m17_db.rx_freq = freq;
}
void m17_db_set_rx_src( uint48_t src ){
	m17_db.rx_src = src;
	m17_decode_call(src, m17_db.rx_src_add);
}
void m17_db_set_rx_dst(  uint48_t dest ){
	m17_db.rx_dest = dest;
	m17_decode_call(dest, m17_db.rx_dest_add);
}
void m17_db_set_src( const char *add ){
	m17_db.tx_src = m17_encode_call(add);
	strcpy(m17_db.tx_src_add,add);
}
void m17_db_set_gate( const char *add ){
	m17_db.tx_gate = m17_encode_call(add);
	strcpy(m17_db.tx_gate_add,add);
}
void m17_db_set_dst( const char *add ){
	m17_db.tx_dest = m17_encode_call(add);
	strcpy(m17_db.tx_dest_add,add);
}
void m17_db_set_brd( void ){
	m17_db.tx_dest = 0xFFFFFFFFFFFF;
}
uint48_t m17_db_get_src( void ){
	return(m17_db.tx_src);
}
uint48_t m17_db_get_gate( void ){
	return(m17_db.tx_gate);
}
uint48_t m17_db_get_dst( void ){
	return(m17_db.tx_dest);
}
void m17_db_listen_all( bool flag){
    m17_db.listen_flag = flag;
}
bool m17_db_is_for_me(uint48_t add ){
	bool r = false;
	if( m17_db.listen_flag == false ){
		if( add == 0xFFFFFFFFFFFF ) r = true;
		if( m17_db.tx_src == add ) r = true;
	}
	if( m17_db.listen_flag == true ) r = true;
	return r;
}
void m17_aos(void){
    m17_db.g_errors = 0;
    m17_db.n_frames = 0;
    m17_db.in_frame = true;
    gui_aos();
    m17_rx_parse_aos();
//    printf("AOS\n");
}
void m17_los(void){
//	float ber = (100.0*m17_db.g_errors)/(m17_db.n_frames*24.0);
    m17_db.in_frame = false;
    gui_los();
    m17_rx_parse_los();

//    printf("LOS %f \n",ber);
}
//
// Data from stream frame
//
void m17_db_golay_errors( uint16_t e ){
    m17_db.g_errors += e;
    m17_db.n_frames++;
}
void m17_db_stream_seq_number( uint16_t n){

}
bool m17_db_in_frame(void){
    return m17_db.in_frame;
}
void m17_db_set_chan_type(CircuitType type){
	m17_db.chan_type = type;
}
CircuitType m17_db_get_chan_type(void){
	return m17_db.chan_type;
}
void m17_db_set_ptt(bool ptt){
	m17_db.ptt = ptt;
}
bool m17_db_get_ptt(bool ptt){
	return m17_db.ptt;
}
