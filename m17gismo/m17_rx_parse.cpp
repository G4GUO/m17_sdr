#include <math.h>
#include <memory.h>
#include "m17defines.h"

static uint8_t m_lsf[30];
static uint8_t m_packet[800];
static int     m_packet_idx;
static uint48_t m_src_add;
static uint48_t m_dst_add;

void valid_packet_received(uint8_t *packet, int len){

}

static void valid_lsf_received(uint48_t src_add, uint48_t dst_add, uint8_t *meta, M17Type type){
	char dst[10];
	char src[10];
	decode_call(src_add, src);
	decode_call(dst_add, dst);
    printf(" src: %s dest: %s ", src, dst);
    m_src_add = src_add;
    m_dst_add = dst_add;
}

static void data_received(uint8_t *data, int len){
	if(m17_db_is_for_me( m_dst_add ) == true ){
	    m17_tx_rx_rx_spkr_audio( &data[0] );
	    m17_tx_rx_rx_spkr_audio( &data[len/2]);
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
	pack_8_to_48( in, dst_add );
	// Extract the src address
	pack_8_to_48( &in[6], src_add );
	// Extract the type
	pack_8_to_16( &in[12], tw);
	M17Type type = m17_upack_type(tw);
	// Extract the meta data
	memcpy(meta,&in[14],sizeof(uint8_t)*14);
	// Send this information to a higher layer
	valid_lsf_received( src_add, dst_add, meta, type);
}
void update_lich(uint8_t *in){
	// extract seq field
	int seq = in[5]>>5;// Mod 6 counter
	if( seq < 6 ){
		// refresh the data
		memcpy(&m_lsf[seq*5],in,sizeof(uint8_t)*5);
	    // Check the CRC
	    uint16_t crc = m17_crc_array_encode( m_lsf, 30 );
	    if( crc == 0x0000 ){
		    // valid LSF
		    parse_lsf(m_lsf);
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
	parse_lsf(bytes);
}
//
// Soft bits start at the payload (skip the 16 bit sync)
//
void decode_stream_frame(float *sb){
	float so[2][368];
	uint24_t gw[4];
	uint12_t w[4];
	uint8_t lich[6];
	uint8_t bits[148];
	uint8_t pld[19];
	uint16_t fn;
	int e;

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
	printf(" Golay errors %d ",e);
	// Pack into an 8 bit array 6 bytes long
	pack_12_to_8_x4x6( w, lich);
	update_lich(lich);
	// Now do the convolutional bits
	// de puncture
	m17_de_punc_p2(&so[0][96], so[1], 296);
	// Viterbi
	m17_viterbi_decode( so[1], bits, 296);
	// Pack bits into byte array
	pack_1_to_8(&bits[1], pld, 144);// discard 4 tail bits ???????
	// Extract seq
     pack_8_to_16( pld, fn);
//     printf("FN %d \n",fn);
     // Point to start of data
     uint8_t *data = &pld[2];
     data_received(data, 16);
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
void m17_rx_parse(float *s, int type){
	float sb[384];// soft bits
	//float m_f_sb[384];// soft bits
printf("Type: %d ",type);
	switch(type){
	case 0:
		// Preamble
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
		break;
	default:
		break;
	}
	printf("\n");
}
