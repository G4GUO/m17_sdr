#include "m17defines.h"

typedef struct{
	uint48_t src;
	uint48_t dst;
    bool listen_flag;
}M17_Dbase;
M17_Dbase m17_db;

void m17_db_set_src( const char *add ){
	m17_db.src = encode_call(add);
}
void m17_db_set_dst( const char *add ){
	m17_db.dst = encode_call(add);
}
void m17_db_set_brd( void ){
	m17_db.dst = 0xFFFFFFFFFFFF;
}
uint48_t m17_db_get_src( void ){
	return(m17_db.src);
}
uint48_t m17_db_get_dst( void ){
	return(m17_db.dst);
}
void m17_db_listen_all( bool flag){
    m17_db.listen_flag = flag;
}
bool m17_db_is_for_me(uint48_t add ){
	bool r = false;
	if( m17_db.listen_flag == false ){
		if( add == 0xFFFFFFFFFFFF ) r = true;
		if( m17_db.src == add ) r = true;
	}
	if( m17_db.listen_flag == true ) r = true;
	return r;
}
