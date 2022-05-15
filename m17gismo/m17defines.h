#ifndef M17DEFINES_H
#define M17DEFINES_H
#include <stdint.h>
#include <stdio.h>
#include "codec2.h"

//#define __TEST__
// Over sample rate
#define N_OS 10
// Size of decimating filter in receiver
#define DF5_FN 71

// Length of preamble in samples
#define N_SAMPLES 1920
#define SRATE 48000

typedef uint16_t uint12_t;
typedef uint32_t uint24_t;
typedef uint64_t uint48_t;
/*
 * Type field definitions
 *
   0 	Packet/stream indicator, 0=packet, 1=stream
   1..2 	Data type indicator, 01 =data (D), 10 =voice (V), 11 =V+D, 00 =reserved
   3..4 	Encryption type, 00 = none, 01 =AES, 10 =scrambling, 11 =other/reserved
   5..6 	Encryption subtype (meaning of values depends on encryption type)
   7..10 	Channel Access Number (CAN)
   11..15 	Reserved (don’t care)
*/

typedef struct{
	uint8_t p_s;
	uint8_t dt;
	uint8_t et;
	uint8_t est;
	uint8_t can;
	uint8_t reserved;
}M17Type;

#define CCT_PACKET   0
#define CCT_STREAM   1
#define DATA_DATA    1
#define DATA_VOICE   2
#define DATA_DANDV   3
#define DATA_RES     0
#define ENC_NONE     0
#define ENC_AES      1
#define ENC_SCR      2
#define ENC_OTH      3
#define ENC_SUB      0
#define CAN_NUM      0
#define RES_RES      0

#define FRAME_SYM_LENGTH 192
#define FRAME_BIT_LEN    384

typedef struct{
	float re;
	float im;
}fcmplx;

typedef struct {
	int16_t re;
	int16_t im;
}scmplx;

// MMI
void mmi_parse(char *cmd);

// Lime specific
int  lime_main(void);
int  lime_open(void);
void lime_got_to_duplex(void);
void lime_got_to_transmit(void);
void lime_got_to_receive(void);
void lime_set_freq(uint64_t freq);
void lime_close(void);
int  lime_transmit_samples( int16_t *in, int len);
int  lime_receive_samples(int16_t *samples, int len);


void m17_dsp_process_samples(short *buffer, int samplesRead);
void m17_dsp_build_rrc_filter(float *filter, float rolloff, int ntaps, int samples_per_symbol);
void m17_dsp_init(void);
void m17_dsp_enable_bs(void);
void m17_dsp_disable_bs(void);
void m17_dsp_set_filter_gain(float *filter, float gain, int stride, int ntaps);
void m17_dsp_rx(scmplx *in, int len);
void m17_dsp_display_coffs(float *in, int len);
int m17_dsp_decimating_filter(float *in, float *out, float *coffs, int stride, int flen, int len);
float  *m17_dsp_update_buffer( float *in, int flen, int blen);
fcmplx *m17_dsp_update_buffer( fcmplx *in, int flen, int blen);
scmplx *m17_dsp_update_buffer( scmplx *in, int flen, int blen);

//
// De-map the symbol into 2 soft bits
// value is the 'probability' of receiving a 1
// 1.0 = 100%
// 0.5 = 50% (puncture value)
// 0   = 0%
//

void m17_dsp_demap_symbol(float in, float mag, float *out);
void m17_dsp_demap_frame(float *in, float *out);

int audio_main(void);
void audio_output(int16_t *s, int len);
void audio_input( int16_t *s, int len);
void audio_input_flush(void);
void audio_output_flush(void);
void audio_open(void);
void audio_close(void);

//
// Puncture / De puncture routines
//
int m17_punc_p1(uint8_t *in, uint8_t *out, int len);
int m17_punc_p2(uint8_t *in, uint8_t *out, int len);
int m17_punc_p3(uint8_t *in, uint8_t *out, int len);
int m17_de_punc_p1(float *in, float *out, int len);
int m17_de_punc_p2(float *in, float *out, int len);
int m17_de_punc_p3(float *in, float *out, int len);

//
// bit packing etc
// len is the input length
//
int pack_8_to_1(uint8_t *in, uint8_t *out, int len);
int pack_1_to_2(uint8_t *in, uint8_t *out, int len);
int pack_1_to_8(uint8_t *in, uint8_t *out, int len);
int pack_48_to_8(uint64_t in, uint8_t *out);
int pack_24_to_8(uint24_t in, uint8_t *out);
int pack_24_to_2(uint24_t in, uint8_t *out);
int pack_24_to_1(uint24_t in, uint8_t *out);
int pack_16_to_8(uint16_t in, uint8_t *out);
int pack_16_to_2(uint16_t in, uint8_t *out);
int pack_12_to_8_x4x6(uint12_t *in, uint8_t *out);
int pack_8_to_48(uint8_t *in, uint48_t &out);
int pack_8_to_24(uint8_t *in, uint24_t &out);
int pack_8_to_16(uint8_t *in, uint16_t &out);
int pack_8_to_12_x4(uint8_t *in, uint12_t *out);
void prbs9(void);

uint24_t hard_decode_24_bits(float *in);

int pack_8_to_8(uint8_t *in, uint8_t *out, int len);
// Pack Type array into type field
uint16_t m17_pack_type(M17Type type);
M17Type m17_upack_type(uint16_t word);

uint48_t encode_call(const char *call);
char *decode_call(uint48_t word, char *call);
void test_encode(void);

// Convolutional code
void m17_init_conv(void);
int m17_conv_encode_1(uint8_t *in, uint8_t *out, int len);
int m17_conv_encode_8(uint8_t *in, uint8_t *out, int len);
int m17_viterbi_decode(float *in, uint8_t *out, int len);

//
// Interleaver / Deinterleaver
//
void m17_interleave( uint8_t *in, uint8_t *out, int len);
void m17_de_interleave( float *in, float *out, int len);

//
// Golay encoder / decoder
//
uint24_t m17_golay_encode(uint12_t data);
int      m_17_golay_decode(uint24_t word, uint12_t &odata);
void m17_golay_init(void);

//
// CRC calculation
//
uint16_t m17_crc_array_encode( uint8_t *in, int len );
void m17_crc_init(void);
void m17_test_crc(void);

//
// de-Correlator
//
void m17_de_correlate_8( uint8_t *in, int len);
void m17_de_correlate_1( uint8_t *in, uint8_t *out, int len);
void m17_de_correlate_1( float *in, float *out, int len);
void m17_init_de_correlate(void);

//
// Tx frames
//
void m17_send_preamble(void);
void m17_send_link_setup_frame(uint48_t dest, uint48_t src, M17Type type, uint8_t *meta);
void m17_send_stream_frame( uint8_t *payload);
void m17_send_bert_frame( void );
void m17_send_eot(void);
// Blank carrier
void m17_send_carrier(void);
void m17_fmt_init(void);

// tx modulator
void m17_mod_tx(int16_t *s, int len);
// initialise
void m17_mod_init(void);
// Send stream of dibits
void m17_mod_dibits(uint8_t *dibits, int len);
// Send carrier
void m17_mod_carrier(void);

// m17_tests
void m17_send_test_frames( CODEC2 *codec, uint8_t *meta, int n_sams);

// rx frame
void m17_rx_sym(float sym);
void m17_rx_symbols(float *sym, int len);
void m17_rx_lost(void);
void m17_rx_init(void);
bool m17_rx_lock(void);

// rx parser
void m17_rx_parse(float *sb, int type);

// rx sync
int m17_rx_sync_samples( float *in, float *out, int len);
void m17_rx_sync_init(void);
int m17_sync_check(float *vect, int &type, int &e );

// Tx RX routines
void m17_tx_rx_init(void);
void m17_tx_rx_loop(void);
void m17_tx_rx_set_src_add(const char *add);
void m17_tx_rx_set_dest_add(const char *add);
void m17_tx_rx_set_brd_add(void);
void m17_tx_rx_set_type(M17Type type);
void m17_tx_rx_set_meta(uint8_t *meta);
void m17_tx_rx_rx_spkr_audio( uint8_t *data );

void m17_tx_rx_ptt_on(void);
void m17_tx_rx_ptt_off(void);
void m17_tx_rx_enable(void);
void m17_tx_rx_disable(void);

// PRBS9
void m17_prbs9_tx_load(uint8_t *out, int len);
void m17_prbs9_tx_reset(void);
void m17_prbs9_rx_reset(void);
int  m17_prbs9_rx_check(uint8_t bit);
void m17_prbs9_init(void);

// Equaliser
void eq_reset(void);
void eq_restart(void);
float eq_train_known(float *in, float train);
float eq_train_unknown( float *in );
void eq_open(void);

// Database routines
void m17_db_set_src( const char *add );
void m17_db_set_dst( const char *add );
void m17_db_set_brd( void );
uint48_t m17_db_get_src( void );
uint48_t m17_db_get_dst( void );
void m17_db_listen_all( bool flag );
bool m17_db_is_for_me(uint48_t add );

#ifdef __TEST__
// test routines
void m17_test_bb(uint8_t *dibit);
void m17_disp_float_test(float *in, int len);
void m17_test_init(void);
#endif
#endif
