#include "m17defines.h"

#define KN 5

static float m_c[KN];
static float m_g[KN];
static float m_u[KN][KN];
static float m_d[KN];
static float m_E;
static float m_q;
static float m_y;
static float m_fbr;

void eq_k_reset_coffs(void)
{
	int i;

	for (i = 0; i < KN; i++)
	{
		m_c[i] = 0.0;
	}
}
void eq_k_reset_ud(void)
{
	int i, j;

	for (j = 0; j < KN; j++)
	{
		for (i = 0; i < j; i++)

		{
			m_u[i][j] = 0.0;
		}
		m_d[j] = 0.1f;
	}
}
//
// Modified Root Kalman gain Vector estimator
//
void eq_k_calculate(float *x)
{
	int    i, j;
	float  B0;
	float  hq;
	float  B;
	float  ht;
	float  f[KN];
	float  h[KN];
	float  a[KN];

	f[0] = x[0];               // 6.2

	for (j = 1; j < KN; j++)              // 6.3
	{
		f[j] = m_u[0][j]*x[0] + x[j];
		for (i = 1; i < j; i++)
		{
			f[j] += m_u[i][j]*x[i];
		}
	}

	for (j = 0; j < KN; j++)                      // 6.4
	{
		m_g[j] = m_d[j] * f[j];
	}

	a[0] = m_E + m_g[0]*f[0]; 	                  // 6.5

	for (j = 1; j < KN; j++)                      // 6.6
	{
		a[j] = a[j - 1] + m_g[j]*f[j];
	}
	hq = 1 + m_q;                                  // 6.7
	ht = a[KN - 1] * m_q;

	m_y = (float)1.0 / (a[0] + ht);                // 6.19

	m_d[0] = m_d[0] * hq * (m_E + ht) * m_y;       // 6.20

	// 6.10 - 6.16 (Calculate recursively)

	for (j = 1; j < KN; j++)
	{
		B = a[j - 1] + ht;                         // 6.21

		h[j] = -f[j]*m_y;                          // 6.11

		m_y = (float)1.0 / (a[j] + ht);            // 6.22

		m_d[j] = m_d[j] * hq*B*m_y;                // 6.13

		for (i = 0; i < j; i++)
		{
			B0 = m_u[i][j];
			m_u[i][j] = B0 + h[j] * m_g[i]; // 6.15

			m_g[i] += m_g[j] * B0;               // 6.16
		}
	}
}
//
// Update the filter coefficients using the Kalman gain vector and
// the error
//
void eq_k_update(float *s, float error, float symbol )
{
	int i;
	//
	// Calculate the new Kalman gain vector
	//
	eq_k_calculate(s);
	//
	// Update the filter coefficients using the gain vector
	// and the error.
	//
	error *= m_y;

	for (i = 0; i < KN; i++)
	{
		m_c[i] += error * m_g[i];
	}
}
float eq_equalize(float *s)
{
	float symbol;

	/* Calculate the new symbol */

	symbol = s[0]*m_c[0];

	for (int i = 1; i < KN; i++)
	{
		symbol += s[i]*m_c[i];
	}
	return symbol;
}
void eq_reset(void)
{
	eq_k_reset_ud();
	eq_k_reset_coffs();
}
void eq_restart(void)
{
	eq_k_reset_ud();
}

//
// Sample array, 2 samples per symbol
//
static float m_samples[KN];

void eq_update_samples(float *s){

	for( int i = 0; i < KN-2; i++){
		m_samples[i] = m_samples[i+2];
	}
	m_samples[KN-2] = s[0];
	m_samples[KN-1] = s[1];
}
//
// Train the equaliser using known symbol
//
float eq_train_known( float *in, float train )
{
	float error;
	float symbol;

	// Update the samples
	eq_update_samples(in);
    // Equalise
	symbol = eq_equalize(m_samples);
	/* Calculate error */
	error = train - symbol;
	/* Update the coefficients */
	eq_k_update( m_samples, error, symbol );
	/* Update the FB data */
	m_fbr = train;
	return(symbol);
}
//
// Train the equaliser using unknown symbol
// derived from decision
//
float eq_train_unknown( float *in )
{
	float error;
	float symbol;
	float train;

	// Update the samples
	eq_update_samples(in);
	// Equalise
	symbol = eq_equalize(m_samples);
    /* Make a decision */
	if( symbol > 0){
		if( symbol >= 0.66)
			train = 1.0f;
		else
			train = 0.333f;
	}else{
		if( symbol <= -0.66)
			train = -1.0f;
		else
			train = -0.333f;
	}
	/* Calculate error */
	error = train - symbol;
	/* Update the coefficients */
	eq_k_update( m_samples, error, symbol );
	/* Update the FB data */
	m_fbr = train;
	return(symbol);
}
/*
* Initialise this module
*/
void eq_open(void)
{
	m_q = (float)0.08;
	m_E = (float)0.01;

	eq_k_reset_ud();
	eq_k_reset_coffs();
}

