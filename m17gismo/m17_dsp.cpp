#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <memory.h>
#include "m17defines.h"

#define FA_N  63
#define HB_FN 63

// HB Filter weights
//static int16_t m_hbw[HB_FN/2];
// 5x decimating Filter weights
float m_df5w[DF5_FN];
float m_decb[N_SAMPLES+DF5_FN];

// Sample period

#define T (1.0/SRATE)
// K loop gain, stable between 0 and 2/PI*T, 30557
#define K 0.00000003

//static float m_fac[FA_N*N_OS];
static float m_k;
static double m_z;
static float m_gain_acc;
static float m_gain;
static float m_work[N_SAMPLES*2];
//
// De-map the symbol into 2 soft bits
// value is the 'probability' of receiving a 1
// 1.0 = 100%
// 0.5 = 50% (puncture value)
// 0   = 0%
//
void m17_dsp_demap_symbol(float in, float mag, float *out){
	// Normalise in relation to sync header
	float m;
	m = in*mag;

	out[0] = -m;
	out[1] = (fabs(m) - 0.6666);
}
void m17_dsp_demap_symbol_old(float in, float cor, float *out){
	// Normalise in relation to sync header
	float m,f0,f1;

	m = in*cor;
    f1 = fabs(m);
//    printf("VAL: %f\n",m);
	// limit the value
	if( f1 > 1.0) f1 = 1.0;
    f0 = f1 - 0.6666;

	if( m > 0){
		// Positive so msb = 0
		out[0] = 0.0;

		if( f0 > 0 ){
			// lsb = 1
			out[1] = 1.0;
		}else{
			// lsb = 0
			out[1] = 0.0;//
		}
	}else{
		// Negative so msb = 1
		out[0] = 1.0;

		if( f0 > 0 ){
			// lsb = 1
			out[1] = 1.0;
		}else{
			// lsb = 0
			out[1] = 0.0;
		}
	}
//	printf("in: %f out: %f %f\n",in,out[0],out[1]);
}
//
// Convert the frame symbols into bit probabilities
//
void m17_dsp_demap_frame( float *in, float *out){
	// Use the sync to estimate the reference magnitude
	float sum = 0;
	for( int i = 0; i < 8; i++){
		sum += fabs(in[i]);
	}
	float cor = 8.0/sum;
	// De-map the payload only, skip the sync
	int idx = 0;
	for( int i = 8; i < FRAME_SYM_LENGTH; i++){
		m17_dsp_demap_symbol(in[i], cor, &out[idx]);
		idx += 2;
	}
}

static float dsp_average(float *in, int len){
	float sum = 0;
	for( int i = 0; i < len; i++){
		sum += in[i];
	}
	sum = sum/len;
	return sum;
}
//static float dsp_sub_filter(float *samples){
//	float sum = samples[0]*m_fac[0];
//	for( int i = 1; i < FA_N*N_OS; i++){
//	    sum += 	samples[i]*m_fac[i];
//	}
//	return sum;
//}
//
// Set of functions to update buffers used by filters.
//
// flen = length of filter
// blen = length of buffer for new data
// flen + blen = total buffer size
// returns pointer to where new samples should be written

float *m17_dsp_update_buffer( float *in, int flen, int blen){
	memcpy(in,&in[blen],sizeof(float)*flen);
	return &in[flen];
}

fcmplx *m17_dsp_update_buffer( fcmplx *in, int flen, int blen){
	memcpy(in,&in[blen],sizeof(fcmplx)*flen);
	return &in[flen];
}

scmplx *m17_dsp_update_buffer( scmplx *in, int flen, int blen){
	memcpy(in,&in[blen],sizeof(scmplx)*flen);
	return &in[flen];
}


static void dsp_short_to_float(scmplx *in, fcmplx *out, int len){
	for( int i = 0; i < len; i++){
		out[i].re = in[i].re*0.00003;
		out[i].im = in[i].im*0.00003;
	}
}
void dsp_float_to_short(float *in, int16_t *out, int len){
	for( int i = 0; i < len; i++){
		out[i] = in[i]*0x3FFF;
	}
}
//
// Dicriminate using arctan
//
void dsp_arctan_disc(short *in){
	float u,v,re,im;
	static float z;
	for( int i = 0; i <  N_SAMPLES; i++){
	    re  = (in[(i*2) ] *  0.003)+0.0001;// Prevent large outputs
		im  =  in[(i*2)+1] * 0.003;
		v   = atan(im/re);
		u   = v - z;
		z = v;
		m_work[i] = m_work[i+N_SAMPLES];
		m_work[i+N_SAMPLES] = u;
	}
	// Calculate the DC offset
	float offset = dsp_average(&m_work[N_SAMPLES],N_SAMPLES);
//    printf("Offset %f\n",offset);
    // Remove the DC
	for( int i = 0; i < N_SAMPLES; i++){
		m_work[i+N_SAMPLES] = m_work[i+N_SAMPLES] - offset;
	}
}
int dsp_arctan_disc(fcmplx *in, float *out, int len){
	static int count;
	static float z;
	float  offset = 0;
	float u,v;
	int idx = 0;
	for( int i = 0; i <  len; i++){
		v   = atan(in[i].im/in[i].re);
		u   = v - z;
		z   = v;
		offset += u;
		count = (1+count)%5;
		if(count == 0) out[idx++] = u;
	}
	// Calculate the DC offset
	offset = offset/len;
	radio_afc(offset);
    // Remove the DC
	for( int i = 0; i < idx; i++){
		out[i] = out[i] - offset;
	}
	//printf("Offset %f\n",offset);
	return idx;
}
static int dsp_arctan_disc2(fcmplx *in, float *out, int len){
	static int count;
	static fcmplx z[2];
	float offset = 0;
	float a,b,u,c;
	c = 0.5;
	int idx = 0;
	for( int i = 0; i <  len; i++){
		a = z[0].im * (in[i].re - z[1].re);
		b = z[0].re * (in[i].im - z[1].im);
		u = b-a;
		z[1] = z[0];
		z[0] = in[i];
		count = (count+1)%5;
		if(count == 0){
			out[idx++] = u*c;
		}
		offset += u*c;
	}
	// Calculate the DC offset
	offset = offset/len;
	radio_afc(offset);
    // Remove the DC
	for( int i = 0; i < idx; i++){
		out[i] = out[i] - offset;
	}
	//printf("Offset %f\n",offset);
	return idx;
}
//
// PLL FM decoder
//
void dsp_pll_disc( short *in ){
	float val,re,im,rv,iv,cv,sv;
	m_gain_acc = 0;
	val = 0;
	for( int i = 0; i < N_SAMPLES; i++){
	    cv  = cos(m_z);
		sv  = sin(m_z);
		rv  = in[(i*2)];
		iv  = in[(i*2)+1];
	    re  = cv*rv - sv*iv;		m_z = m_z + val;

		im  = cv*iv + sv*rv;
		m_gain_acc += sqrt(rv*rv + iv*iv);
		val = re + im;
		//val *= m_gain;
		m_work[i+N_SAMPLES] = val;
		val = val * m_k;
		m_z = m_z + val;
	}
	m_gain = 1.0*N_SAMPLES/m_gain_acc;
	// Correct for any overflow in the accumulator
	m_z = m_z/(2.0*M_PI);
	double ip;
	m_z = modf(m_z,&ip);
	m_z = m_z*2.0*M_PI;
    //
	// Remove the DC offset. As the frame is scrambled it should have 0 average.
	float offset = dsp_average(&m_work[N_SAMPLES],N_SAMPLES);
    //printf("Offset %f\n",offset);
	for( int i = 0; i < N_SAMPLES; i++){
		m_work[i+N_SAMPLES] = m_work[i+N_SAMPLES] - offset;
	}
}
int dsp_pll_disc( fcmplx *in, float *out, int len ){
	static int count;
	float val,re,im,rv,iv,cv,sv;
	float offset = 0;
	int idx = 0;
	for( int i = 0; i < len; i++){
	    cv  = cos(m_z);
		sv  = sin(m_z);
		rv  = in[i].re;
		iv  = in[i].im;
	    re  = cv*rv - sv*iv;
		im  = cv*iv + sv*rv;
		val = re + im;
		count = (1+count)%5;
		if(count == 0) out[idx++] = val;
		offset += val;
		m_z += val * K;
	}
	// Correct for any overflow in the accumulator
	m_z = m_z/(2.0*M_PI);
	double ip;
	m_z = modf(m_z,&ip);
	m_z = m_z*2.0*M_PI;

	// Calculate the DC offset
	offset = offset / len;
	radio_afc(offset);
    // Remove the DC
	for( int i = 0; i < idx; i++){
		out[i] = out[i] - offset;
	}
	return idx;
}
//
// functions below here are externally visible
//
void m17_dsp_build_rrc_filter(float *filter, float rolloff, int ntaps, int samples_per_symbol) {
	double a, b, c, d;
	double B = (rolloff+0.0001);// Rolloff factor .0001 stops infinite filter coefficient with 0.25 rolloff
	double t = -(ntaps - 1) / 2;// First tap
	double Ts = samples_per_symbol;// Samples per symbol
	// Create the filter
	for (int i = 0; i < (ntaps); i++) {
		a = 2.0 * B / (M_PI*sqrt(Ts));
		b = cos((1.0 + B)*M_PI*t / Ts);
		// Check for infinity in calculation (computers don't have infinite precision)
		if (t == 0)
			c = (1.0 - B)*M_PI / (4 * B);
		else
			c = sin((1.0 - B)*M_PI*t / Ts) / (4.0*B*t / Ts);

		d = (1.0 - (4.0*B*t / Ts)*(4.0*B*t / Ts));
		//filter[i] = (float)(b+c)/(a*d);//beardy
		filter[i] = (float)(a*(b + c) / d);//nasa
		t = t + 1.0;
	}
}
//
// Filter program that takes advantage of the zero coefficients in a half band filter
//
void m17_halfband_filter(scmplx *in, scmplx *out, int16_t *coffs, int flen, int len) {
	int32_t real, imag;
	// Point to coefficients
    scmplx *mm;

	for (int i = 0; i < len; i++) {
		mm = &in[i];

		// Filter operation
		scmplx  *h = &mm[(flen / 2) + 1];
		scmplx  *l = &mm[(flen / 2) - 1];
		real  = coffs[0] * mm[flen / 2].re;
		imag  = coffs[0] * mm[flen / 2].im;
		real += coffs[1] * (l->re + h->re);
		imag += coffs[1] * (l->im + h->im);

		for (int j = 2; j < (flen / 2); j++) {
			l-=2; h+=2;
			real += coffs[j] * (l->re + h->re);
			imag += coffs[j] * (l->im + h->im);
		}
		out[i].re = real >> 15;
		out[i].im = imag >> 15;
	}
}
//
// Routines to create a LPF (Rectangular Window)
//
void m17_dsp_build_lpf_filter(float *filter, float bw, int ntaps) {
	double a;
	double B = bw;// filter bandwidth
	double t = -(ntaps - 1) / 2;// First tap
								// Create the filter
	for (int i = 0; i < ntaps; i++) {
		if (t == 0)
			a = 2.0 * B;
		else
			a = 2.0 * B * sin(M_PI * t * B) / (M_PI * t * B);
		filter[i] = (float)a;
		t = t + 1.0;
	}
}
//
// Create a window and also apply an int to float scaling
//
void m17_dsp_apply_window(float *window, int N){
	double n;
	n = -N/2;
	for( int i = 0; i < N; i++){
		window[i] *= 0.5*(1.0 + cos((n*2.0*M_PI)/N));
		n = n + 1.0;
	}
}

int m17_dsp_compress_hbf(float *in, int n) {
    int idx = 2;
	for (int i = 3; i <= n; i+=2) {
		in[idx++] = in[i];
	}
	return idx;
}
void m17_dsp_float_to_short(float *in, int16_t *out, int len){
	for( int i = 0; i < len; i++){
		out[i] = (int16_t)(in[i]*0x7FFF);
	}
}

//
// NCO and complex mixer for shifting the frequency (samples in place)
// Used for AFC
//
static void dsp_nco_mixer(fcmplx *in, float delta, int len){
	static double acc;
	for( int i = 0; i < len; i++){
	    float c = cos(acc);
		float s = sin(acc);
		acc += delta;
		float re = (in[i].re * c) - (in[i].im * s);
		float im = (in[i].re * s) + (in[i].im * c);
	    in[i].re = re;
	    in[i].im = im;
	}
	// Correct for any overflow in the accumulator
	acc = acc/(2.0*M_PI);
	double ip;
	acc = modf(acc,&ip);
	acc = acc*2.0*M_PI;
	// Test for NaN
	if(acc != acc) acc = 0;
}
//
// Hard limit the signal
//
static void dsp_limit(fcmplx *in, int len){
	for( int i = 0; i < len; i++){
	    float m = sqrt(in[i].re*in[i].re + in[i].im*in[i].im);
	    float g = 1.0/m;
	    in[i].re *= g;
	    in[i].im *= g;
	}
}
void m17_dsp_set_filter_gain(float *filter, float gain, int stride, int ntaps) {
	float sum = 0;
	for (int i = 0; i < ntaps; i++) {
		sum += filter[i*stride];
	}
	gain = gain / sum;
	for (int i = 0; i < ntaps; i++) {
		filter[i*stride] = filter[i*stride] * gain;
	}
}
//
// Filter and decimate
// in = input samples
// out = output samples
// coffs = filter coefficients
// stride = decimation ratio
// flen = number of taps in filter
// len the number of input samples to process
// returns length of output buffer
//
int m17_dsp_decimating_filter(float *in, float *out, float *coffs, int stride, int flen, int len){
    int idx = 0;
	for( int i = 0; i < len; i+= stride){
		float sum = 0;
		float *s = &in[i];
		for( int j = 0; j < flen; j++){
            sum += s[j]*coffs[j];
		}
		out[idx++] = sum;
	}
	return idx;
}
void m17_dsp_display_coffs(float *in, int len){
	for( int i = 0; i < len; i++) printf("%2d %f\n",i,in[i]);
}

//
// New received samples are processed here
//
//

void m17_dsp_rx(scmplx *in, int len){
	fcmplx  tempa[N_SAMPLES];
	float   tempd[N_SAMPLES/5];
	float   tempc[N_SAMPLES/2];// larger than required to cope with bit slips
	int n;
	//dsp_pll_disc(in);
	dsp_short_to_float( in, tempa, len );
	if(radio_get_afc_status() == true ) dsp_nco_mixer(tempa, radio_get_afc_delta(), len);
	dsp_limit( tempa, len);
    // Run the discriminator
    //n = dsp_pll_disc(tempa, tempd, len);
	n = dsp_arctan_disc2( tempa, tempd, len );
	// do the symbol tracking
	n = m17_rx_sync_samples( tempd, tempc, n );
	m17_rx_symbols( tempc, n);
}
//
// Initialisation routine
//
void m17_dsp_init(void){
//	float temp[HB_FN];

//	m17_dsp_build_rrc_filter(m_fac, 0.5, FA_N*N_OS, N_OS);
//	m17_dsp_set_filter_gain(m_fac, 1.0, 1, FA_N);
	m_k = 2*M_PI*K*T;
	// Build a half band filter
    //m17_dsp_build_lpf_filter(temp, 0.5f,  HB_FN);
    //m17_dsp_apply_window(temp, HB_FN);
    //m17_dsp_set_filter_gain(temp, 1.0, 1, HB_FN);
    //int len = m17_dsp_compress_hbf(&temp[HB_FN/2],HB_FN/2);
//    m17_dsp_display_coffs(&temp[HB_FN/2], len);
//	dsp_float_to_short(&temp[HB_FN/2], m_hbw, len);
/*
	// Build 5x decimating filter (used on the output of the discriminator
    m17_dsp_build_lpf_filter(m_df5w, 0.2f,  DF5_FN);
    m17_dsp_apply_window(m_df5w, DF5_FN);
    m17_dsp_set_filter_gain(m_df5w, 1.0, 1, DF5_FN);
*/
}
