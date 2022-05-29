#include <math.h>
#include "m17defines.h"

// Frame types
static const float sframe[6][8]={
		{ 1,-1, 1,-1, 1,-1, 1,-1}, // Preamble
		{ 1, 1, 1, 1,-1,-1, 1,-1}, // Link
		{-1,-1,-1,-1, 1, 1,-1, 1}, // Stream
		{ 1,-1, 1, 1,-1,-1,-1,-1}, // Packet
		{-1, 1,-1,-1, 1, 1, 1, 1}, // Bert
		{ 1, 1, 1, 1, 1, 1,-1, 1}  // EOT
};

static float m_f_sym[192];// Symbols

static bool  m_flock;// Status of framer
static int   m_fclk;// framer clock
static int   m_frame_errors;
//
// Find the variance in the magnitude
//
float find_variance( float *in, int len ){
	float v;
	float mmin;
	float mmax;

    mmin = fabs(in[0]);
    mmax = mmin;

	for( int i = 1; i < len; i++){
		v = fabs(in[i]);
		if(v > mmax )
			mmax = v;
		else{
			if(v < mmin) mmin = v;
		}
	}
	v = (mmax-mmin)/mmax;
	// NaN removal
	if(v != v ) v = 1.0;
//	printf("var: %f\n",v);
	return v;
}
//
// correlate against sync vectors
//
void m17_sync_check(float *vect, M17Sync *sync ){
    float sums[6];
    sums[0] = vect[0]*sframe[0][0];
    sums[1] = vect[0]*sframe[1][0];
    sums[2] = vect[0]*sframe[2][0];
    sums[3] = vect[0]*sframe[3][0];
    sums[4] = vect[0]*sframe[4][0];
    sums[5] = vect[0]*sframe[5][0];
    for( int i = 1; i < 8; i++){
    	sums[0] += vect[i]*sframe[0][i];
	    sums[1] += vect[i]*sframe[1][i];
	    sums[2] += vect[i]*sframe[2][i];
	    sums[3] += vect[i]*sframe[3][i];
	    sums[4] += vect[i]*sframe[4][i];
	    sums[5] += vect[i]*sframe[5][i];
    }
    sync->variance = find_variance( vect, 8 );
    //
    // Find the maximum likely vector
    //
    float mmax = 0;
    int   nmax = 0;
    for( int i = 0; i < 6; i++){
	    if(sums[i] > mmax ){
		    mmax = sums[i];
		    nmax = i;
	    }
    }
    sync->type = nmax;
    // We know the most likely sync do a vote
    sync->votes = 0;
    for( int i = 0; i < 8; i++){
        if(vect[i]*sframe[nmax][i] < 0 ) sync->votes++;
    }
}
bool m17_unlocked_sync_check( M17Sync *sync){
	if(sync->votes > 0 ){
		return false;
	}
	if((sync->type == 1) || (sync->type == 2) || (sync->type == 3) || (sync->type == 4)){
		if( sync->variance < 0.3){
			return true;
		}
	}
	return false;
}
bool m17_locked_sync_check( M17Sync *sync){
	if(sync->votes > 1 ){
		return false;
	}
	if((sync->type == 1) || (sync->type == 2) || (sync->type == 3) || (sync->type == 4)){
		if( sync->variance < 0.5){
			return true;
		}
	}
	return false;
}
static float m_sync[8];

static void update_sync(float sm){
	for( int i = 0; i < 7; i++){
		m_sync[i] = m_sync[i+1];
	}
	m_sync[7] = sm;
}
static void copy_sync(void){
	for( int i = 0; i < 8; i++) m_f_sym[i] = m_sync[i];
}
static void reset_sync(void){
	for( int i = 0; i < 8; i++) m_sync[i] = 0;
}

//
// Functions below here are globally visible
//
#define N_FERROR 5
//
// New symbol received
//
void m17_rx_sym(float sym){
	M17Sync sync;
	if( m_flock == true ){
		// In lock wait until frame received
		m_f_sym[m_fclk] = sym;
		m_fclk = (m_fclk + 1)%FRAME_SYM_LENGTH;

		if(m_fclk == 0){
			// See what type of frame we have received
			m17_sync_check( m_f_sym, &sync );
			if(sync.type == M17_EOT){
			    m_flock = false;
			    reset_sync();
			    m17_los();
			}else{
			    if(m17_locked_sync_check( &sync ) == true){
			        m17_rx_parse(  m_f_sym, sync.type );
				    m_frame_errors = 0;
			    }else{
				    m_frame_errors++;
				    if( m_frame_errors > N_FERROR ){
					    m_flock = false;
					    reset_sync();
					    m17_los();
				    }else{
			            m17_rx_parse( m_f_sym, sync.type );
				    }
			    }
			}
		}
	}else{
		// Not in lock hunt for sync
		update_sync(sym);
		m17_sync_check( m_sync, &sync );
		if(m17_unlocked_sync_check( &sync ) == true){
			// Potential sync
			// Set the clock to point to the first symbol after the sync
			// Signal frame lock
			// copy the received sync to the start of the frame
			copy_sync();
			m_fclk = 8;
			m_frame_errors = 0;
			m_flock = true;
			m17_aos();
		}
	}
}
void m17_rx_symbols(float *sym, int len){
	for( int i = 0; i < len; i++){
		m17_rx_sym(sym[i]);
	}
}

void m17_rx_lost(void){
	reset_sync();
	m_flock = false;
}
void m17_rx_init(void){
	reset_sync();
	m_flock = false;
}
bool m17_rx_lock(void){
	return m_flock;
}
