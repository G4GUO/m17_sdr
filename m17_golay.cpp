#include <stdio.h>
#include "m17defines.h"
/*
 * This file contains the lookup tables and the Golay encoding and
 * decoding tables.
 *
 */
#define MASK_12_BIT 0xFFF

// Parity generator table (see M17 spec)
static const uint12_t gtab[12] = {0xC75,0x63B,0xF68,0x7B4,0x3DA,0xD99,0x6CD,0x367,0xDC6,0xA97,0x93E,0x8EB};

/*
 * the following array uses the 12 bit data as the index and returns
 * the parity value.
 *
 */
// The encoding table
static uint12_t g_enctab[0x1000];
/*
 * The error table is a 16 bit table, the index into the table
 * is the syndrome. The top 4 bits are the number of detected
 * errors, if the number = 4 it means 4 or more errors. The lower
 * 12 bits are the error vector for the data words. There is
 * really little point in correcting the errors in the parity field.
 *
 */
uint16_t g_errtab[0x1000];


static void golay_build_encode_table(void){
	for( int i = 0; i < 0x1000; i++){
		g_enctab[i] = 0;
		int m = 0x800;
		for( int n = 0; n < 12; n++){
			if(i&m) g_enctab[i] ^= gtab[n];
			m >>= 1;
		}
	}
}
static uint8_t count_bits(uint24_t word){
	int count = 0;
	for( int i = 0; i < 24; i++){
		if(word&1) count++;
		word >>= 1;
	}
	return count;
}
static void golay_build_error_table(void){
	uint8_t bits;
	uint24_t word;
	// Set output table to 4 errors, this indicates error is unrecoverable
	for(int i = 0 ; i < 0xFFF; i++){
		g_errtab[i]	= 0x400;
	}
	// Now fill in for 3 errors or less
	for( word = 0; word < 0x1000000; word++ ){
		// generate a 24 bit error vectors
		// If it has less than 4 bits, update the output table
	    bits = 	count_bits(word);
	    if( bits < 5){
	         uint12_t data     = word>>12;
	         uint12_t parity   = word&0xFFF;
	         uint12_t syndrome = parity^g_enctab[data];
	         // We are only interested in the error vector that affects the data bits
	         uint16_t vector = bits;
	         vector <<= 12;
	         vector |= word>>12;
	         g_errtab[syndrome] = vector;
	    }
	}
}
/*
static void test_error_recovery(void){
	// Test to see if can recover an error
	uint12_t data = 0xABC; // Data to send
	uint24_t word = 0;     // 24 bit Golay word
	uint24_t error = 0x111000; // 24 bit error vector
	word = m17_golay_encode(data);
	word ^= error; // Apply the error
	uint12_t odata;
	uint8_t errors;
	m_17_golay_decode(word, &odata, &errors);// Error correct
	if( odata == data)
		printf("Success ");
	else
		printf("Failed ");
	printf("Data in %.3X, Data out %.3X, Error applied %6X Errors repaired %d\n",data,odata,error,errors);
}*/
/* Functions start here */
//
// Encode 12 bit data into 24 bit code word
//
uint24_t m17_golay_encode(uint12_t data)
{
	 uint24_t word;
	 word = data;
	 word <<= 12;
	 word |= g_enctab[data&MASK_12_BIT];

	 return word;
}
int m_17_golay_decode(uint24_t word, uint12_t &odata )
{
   uint12_t syndrome;
   uint12_t data   = (word>>12)&MASK_12_BIT;
   uint12_t parity = word&MASK_12_BIT;

   syndrome = parity^g_enctab[data];

   /* Now correct the bits */

   odata   = data^(g_errtab[syndrome]&MASK_12_BIT);
   int e  = (g_errtab[syndrome]&0xF000)>>12;
   return e;
}
void m17_golay_init(void){
	golay_build_encode_table();
	golay_build_error_table();
	//test_error_recovery();
}
