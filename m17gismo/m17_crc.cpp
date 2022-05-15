#include <stdio.h>
#include "m17defines.h"

#define CRC_POLY 0x5935

static uint16_t crc_lu[256];

static void build_crc_lu(void){
	for( int i = 0; i < 256; i++){
		uint16_t x = i<<8;
		for( int n = 0; n < 8; n++ )
		{
			if((x & 0x8000)!=0){
			    x <<= 1;
			    x ^= CRC_POLY;
			}else{
			    x <<= 1;
			}
		}
		crc_lu[i] = x;
	}
}
//
// Globally visible functions
//
uint16_t m17_crc_array_encode( uint8_t *in, int len )
{
	uint16_t crc = 0xFFFF;

	for( int i = 0; i < len; i++){
		uint8_t pos = (crc>>8)^in[i];
		crc = (crc<<8)^crc_lu[pos];
	}
	return crc;
}

void m17_crc_init(void){
	build_crc_lu();
}
void m17_test_crc(void){
    uint8_t array[256];

    for( uint16_t i = 0; i <= 0xFF; i++){
		array[i] = i;
	}
    uint16_t crc = m17_crc_array_encode( array, 256 );
	printf("CRC %.4X\n",crc);

}
