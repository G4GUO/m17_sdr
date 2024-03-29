#include <math.h>
#include <memory.h>
#include "m17defines.h"

static uint8_t m_lsf[2][30];
static uint8_t m_packet[800];
static int     m_packet_idx;
static uint16_t m_frame_id = 0xDEAD;

void generate_new_frame_id(void){
	m_frame_id = rand()&0xFFFF;
}
void copy_lich(void){
	memcpy(m_lsf[1],m_lsf[0],30);
}
void valid_packet_received(uint8_t *packet, int len){

}

static void valid_lsf_received(uint48_t src_add, uint48_t dst_add, uint8_t *meta, M17Type type){
	m17_db_set_rx_src( src_add );
	m17_db_set_rx_dst( dst_add );
    gui_update();
}

static void sound_data_received(uint8_t *data, int len){
	const M17_Dbase *db = m17_get_db();
	if(m17_db_is_for_me( db->rx_dest ) == true ){
		m17_txrx_spkr_audio( &data[0] );
		m17_txrx_spkr_audio( &data[len/2]);
	}
}

void parse_packet( uint8_t *data, uint8_t eof, uint8_t fn){
    if(eof){
    	// FN is the number of valid bytes in the frame
        memcpy(&m_packet[m_packet_idx],data,fn);
        // Calculate the length of the packet
        m_packet_idx += fn;
        // Do a CRC on the packet
    	uint16_t crc = m17_crc_array_encode( m_packet, m_packet_idx );
    	if( crc == 0x0000 ){
    		// valid Packet of data
    		valid_packet_received(m_packet, m_packet_idx - 2);// Discard the CRC
    	}
        m_packet_idx = 0;
    }else{
        memcpy(&m_packet[fn*25],data,25);
        m_packet_idx = fn*25;
    }
}
void parse_lsf( uint8_t *in ){
	uint16_t tw;
	uint48_t dst_add;
	uint48_t src_add;
	uint8_t meta[14];
	// Extract the destination address
	dst_add = pack_8_to_48(in);
	// Extract the src address
	src_add = pack_8_to_48(&in[6]);
	// Extract the type
	tw = pack_8_to_16( &in[12]);
	M17Type type = m17_upack_type(tw);
	// Extract the meta data
	memcpy(meta,&in[14],sizeof(uint8_t)*14);
	// Send this information to a higher layer
	valid_lsf_received( src_add, dst_add, meta, type);
	gui_save_dest_address(dst_add);
	gui_save_src_address(src_add);
}
void update_lich(uint8_t *in){
	// extract seq field
	int seq = in[5]>>5;// Mod 6 counter
	if( seq < 6 ){
		// refresh the data
		memcpy(&(m_lsf[0][seq*5]),in,sizeof(uint8_t)*5);
	    // Check the CRC
	    uint16_t crc = m17_crc_array_encode( m_lsf[0], 30 );
	    if( crc == 0x0000 ){
		    // valid LSF, save a good copy
	    	copy_lich();
		    parse_lsf(m_lsf[1]);
	    }
	}
}
void decode_link_frame(float *sb){
	float so[2][488];
	uint8_t bits[244];
	uint8_t bytes[31];
	m17_de_correlate_1( sb, sb, 368);
	m17_de_interleave( sb, so[0], 368);
	// de puncture
	m17_de_punc_p1(so[0], so[1], 488);
	// Viterbi
	m17_viterbi_decode( so[1], bits, 488);
	// Pack LSF
	pack_1_to_8(&bits[1], bytes, 240);// Discard 4 tail bits
	if(m17_crc_array_encode( m_packet, 30 )==0){
	    parse_lsf(bytes);
	}
}
//
// Soft bits start at the payload (skip the 16 bit sync)
//
void decode_stream_frame(float *sb){
	float so[2][368];
	uint24_t gw[4];
	uint12_t w[4];
	uint8_t  lich[6];
	uint8_t  bits[148];
	uint8_t  pld[19];
	uint16_t fn;
	uint16_t e;

	m17_de_correlate_1( sb, sb,    368);
	m17_de_interleave(  sb, so[0], 368);
	// Extract Golay words
	gw[0] = hard_decode_24_bits(&so[0][0]);
	gw[1] = hard_decode_24_bits(&so[0][24]);
	gw[2] = hard_decode_24_bits(&so[0][48]);
	gw[3] = hard_decode_24_bits(&so[0][72]);
	// Error correct them
	e  = m_17_golay_decode( gw[0], w[0] );
	e += m_17_golay_decode( gw[1], w[1] );
	e += m_17_golay_decode( gw[2], w[2] );
	e += m_17_golay_decode( gw[3], w[3] );
	// Report number of errors
	m17_db_golay_errors( e );
#ifdef __TRACE__
	printf(" Golay errors %d ",e);
#endif
	// Pack into an 8 bit array 6 bytes long
	pack_12_to_8_x4x6(w, lich);
	// If CRC is correct it will save a good copy
	update_lich( lich );
	// Now do the convolutional bits
	// de puncture
	m17_de_punc_p2(&so[0][96], so[1], 296);
	// Viterbi
	m17_viterbi_decode( so[1], bits, 296);
	// Pack bits into byte array
	pack_1_to_8(&bits[1], pld, 144);// discard 4 tail bits ???????
	// Extract seq
    fn = pack_8_to_16( pld );
    m17_db_stream_seq_number(fn);
//     printf("FN %d \n",fn);
    // Only pass data to higher layer if a valid lich has been received
    if(m17_crc_array_encode( m_lsf[1], 30 ) == 0){
        // Point to start of data
        uint8_t *data = &pld[2];
        if(m17_db_get_chan_type() == DRTODN ){
        	// Send to the network
     	    m17_net_new_rx_data( m_frame_id, m_lsf[1], fn, data);
        }
        if(m17_db_get_chan_type() == DRTOAS ){
 	        // Play M17 over speakers
 	        sound_data_received(data, 16);
 	    }
    }
}
void decode_packet_frame(float *sb){
	float so[2][420];
	uint8_t bits[240];
	uint8_t bytes[240];
	m17_de_correlate_1( sb, sb, 368);
	m17_de_interleave( sb, so[0], 368);
	// de puncture
	m17_de_punc_p3(so[0], so[1], 420);
	// Viterbi
	m17_viterbi_decode( so[1], bits, 420);
	pack_1_to_8(&bits[1], bytes, 208);// Discard 2 tail bits
	// Extract EOF file
	uint8_t eof = bytes[25]>>7;
	// Extract fn field
	uint8_t fn = (bytes[25]>>2)&0x1F;
	parse_packet( bytes, eof, fn);
}
void decode_bert_frame(float *sb){

}

//
// Parse an incoming frame of symbols
//
void m17_rx_parse(float *s, uint8_t type){
	float sb[384];// soft bits
	//float m_f_sb[384];// soft bits
#ifdef __TRACE__
    printf("Type: %d ",type);
#endif
	switch(type){
	case 0:
		// Preamble
		generate_new_frame_id();
		break;
	case 1:
		// Link
		m17_dsp_demap_frame(s, sb);
		decode_link_frame(sb);
		break;
	case 2:
		// Stream
		m17_dsp_demap_frame(s, sb);
		decode_stream_frame(sb);
		break;
	case 3:
		// Packet
		m17_dsp_demap_frame(s, sb);
		decode_packet_frame(sb);
		break;
	case 4:
		// Bert
		m17_dsp_demap_frame(s, sb);
		decode_bert_frame(sb);
		break;
	case 5:
		// EOT
		generate_new_frame_id();
		break;
	default:
		break;
	}
#ifdef __TRACE__
	printf("\n");
#endif
}
void m17_rx_parse_aos(void){
	generate_new_frame_id();
}
void m17_rx_parse_los(void){
	generate_new_frame_id();
}

