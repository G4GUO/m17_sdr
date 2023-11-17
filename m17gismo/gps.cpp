#include <math.h>
#include <string.h>
#include <pthread.h>
#include "m17defines.h"

GpsMsg m_gps;

void gps_decode(uint8_t *b, GpsMsg *msg){

	uint16_t n;
    uint48_t w;

	msg->lat = (int8_t)b[0];// lat degrees
    n = pack_8_to_16(&b[1]);// lat fraction
    msg->lat += n/65536.0;

    msg->lon  = (int16_t)pack_8_to_16(&b[3]); // long deg int
    n    = pack_8_to_16(&b[5]);// long fraction
    msg->lon += n/65536.0;

    msg->alt = pack_8_to_16(&b[7]) - 1500;// Alt

    w = pack_8_to_48(&b[9]);
    msg->course =  w>>38;
    msg->speed  = (w>>28)&0x3FF;
    msg->object =  w&0xFFFFF;
}

void gps_encode(uint8_t *b, GpsMsg *msg){
	double frac,ip;
	uint16_t n;
    uint48_t w;

    // Lattitude
	frac = modf(msg->lat,&ip);
    b[0] = ip;
    n = frac*65536;
    pack_16_to_8(n,&b[1]);

    // Longitude
	frac = modf(msg->lon,&ip);
    pack_16_to_8(n,&b[3]);
    n = frac*65536;
    pack_16_to_8(n,&b[5]);

    // Altitude
    pack_16_to_8(msg->alt+1500,&b[7]);
    // Course
    w = msg->course;
    w <<= 10;
    w |= msg->speed;
    w <<= 20;
}
void gps_nmea_parse(char *nmea){
	int nt = 0;
	char *token[25];
	token[nt] = strtok(nmea,",");
	while(token[nt] != NULL){
		if( ++nt == 25 ) break;
		token[nt] = strtok(NULL,",");
	}

	//MSG

	// TIME
	// Latidude
	// N/S
	// Longitude
	// W/E
	// WE
	// Quality
	// Number of satellites
	// HDOP
	// Altitude
	// Units M eters or feet
	// Geoidal seperation
	// Units

}
static bool m_gps_running;
static pthread_t m_tid;

int nmea_decimal(const char *in, int n){
	int v = 0;
	for( int i = 0; i < n; i++){
		v *= 10;
		v += in[i] -'0';
	}
	return v;
}
bool nmea_chksum( char *nmea){
	char c = 0;
	for( uint32_t i = 1; i < strlen(nmea); i++){
		if(nmea[i] != '*'){
			c ^= nmea[i];
	    }else{
	    	int s = strtol(&nmea[i+1],NULL,16);
	    	if( c == s ){
	    		return true;
	    	}else{
	    		return false;
	    	}
	    }
	}
	return false;
}
int gps_tokenise(char *nmea, char *list[]){
    int len = strlen(nmea);
    int n = 0;
    list[n++] = nmea;
    for( int i = 1; i < len;  i++){
    	if(nmea[i] == ','){
    		nmea[i] = 0;
    		list[n] = &nmea[i+1];
		    n++;
    	}
    }
    return n;
}
void gps_parse_nmea( char *nmea){
    char *token;
    char *list[25];
    const char s[2] = ",";
    if(nmea_chksum( nmea)==false) return;
    int nt = gps_tokenise( nmea, list);
    // Valid message
    if(strncmp(list[0],"$GPTXT",5)==0){
    	// Message about GPS device
    //	token = strtok( NULL, s);
    }
    if(strncmp(list[0],"$GPRMC",5)==0){
    	//$GPRMC,132334.00,A,5048.22218,N,00026.51640,W,0.039,,240722,,,A*6D
    	return;
    	token = strtok( NULL, s);// UTC
    	token = strtok( NULL, s);// STATUS
    	token = strtok( NULL, s);// LAT
    	token = strtok( NULL, s);// LAT DIR
    	token = strtok( NULL, s);// LON
    	token = strtok( NULL, s);// LON DIR
    	token = strtok( NULL, s);// Speed over ground Knots
    	token = strtok( NULL, s);// Track made good
    	token = strtok( NULL, s);// Date
     	token = strtok( NULL, s);// Magnetic variation
    	token = strtok( NULL, s);// Magnetic var direction
    	token = strtok( NULL, s);// Mode Indicator
    }
    if(strncmp(list[0],"$GPVTG",5)==0){
    	return;
    	//$GPVTG,,T,,M,0.039,N,0.072,K,A*2C
    	token = strtok( NULL, s);// Track made good degree true
    	token = strtok( NULL, s);// True track indicator
    	token = strtok( NULL, s);// Track made good, degrees magnetic
    	token = strtok( NULL, s);// Magnetic track indicator
    	token = strtok( NULL, s);// Speed over ground knots
    	token = strtok( NULL, s);// Nautical speed indicator N - knots
    	token = strtok( NULL, s);// Speed kph
    	//token = strtok( NULL, s);// Speed indicator
    	//token = strtok( NULL, s);// Mode indicator

    }
    if(strncmp(list[0],"$GPGGA",5)==0){
    	//$GPGGA,132334.00,5048.22218,N,00026.51640,W,1,07,1.14,11.4,M,45.8,M,,*74
    	// 1 UTC
    	// 2 LAT
    	// 3 LAT DIR
    	// 4 LON
    	// 5 LON DIR
    	// 6 Quality
    	// 7 Nr sats
    	m_gps.nsats = atoi(list[7]);
    	// 8 HDOP
    	// 9 Ant altitude
    	double alt = strtod(list[9],NULL);
    	// 10 A units
    	if(list[10][0] == 'M' ){
    		m_gps.alt = alt*3.28084;// Convert to feet
    	}
        // 11 Undulation
        // 12 Undulation units
        // 13 Age of correction
        // 14 Differential Base station ID
    }
    if(strncmp(list[0],"$GPGSA",5)==0){
    	return;
    	//$GPGSA,A,3,03,17,01,19,04,22,21,,,,,,2.09,1.14,1.75*05
    	token = strtok( NULL, s);// Mode MA
    	token = strtok( NULL, s);// Mode 123 (1D 2D 3D)
    	for( int i = 0; i < 12; i++){
    	    token = strtok( NULL, s);// PRN of satellites
    	}
    	token = strtok( NULL, s);// PDOP
    	token = strtok( NULL, s);// HDOP
    	token = strtok( NULL, s);// VDOP
//    	token = strtok( NULL, s);// System ID
    }
    if(strncmp(list[0],"$GPGSV",5)==0){
    	return;
    	//$GPGSV,2,1,08,01,74,097,28,03,74,243,39,04,23,176,34,17,46,297,31*73
    	token = strtok( NULL, s);// # msgs
    	int m = atoi(token);
    	token = strtok( NULL, s);// msg #
    	token = strtok( NULL, s);// # sats
    	int n = atoi(token);
    	// Calculate the number of satellites in this messages
    	for( int i = 0; i < n/m; i++){
 //   	    token = strtok( NULL, s);// Sat PRN
 //   	    token = strtok( NULL, s);// Elevation
 //   	    token = strtok( NULL, s);// Azimuth
 //   	    token = strtok( NULL, s);// SNR
    	}
 //   	token = strtok( NULL, s);// System ID
    }
    if(strncmp(list[0],"$GPGLL",5)==0){
    	//$GPGLL,5048.22247,N,00026.51350,W,191209.00,A,A*77
    	m_gps.lat = nmea_decimal(list[1],2);
    	m_gps.lat += strtod(&list[1][2],NULL)/60.0;
    	if(list[2][0] == 'N'){

    	}
    	if(list[2][0] == 'S'){
    		m_gps.lat = -m_gps.lat;
    	}
        m_gps.lon = nmea_decimal(list[3],3);
        m_gps.lon += strtod(&list[3][3],NULL)/60.0;
    	if(list[4][0] == 'E'){
    	}
    	if(list[4][0] == 'W'){
            m_gps.lon = -m_gps.lon;
    	}
    	// Time
    	m_gps.hour = nmea_decimal(list[5],2);
    	m_gps.min  = nmea_decimal(&list[5][2],2);
    	m_gps.sec  = nmea_decimal(&list[5][4],2);
        // Fix valid
        if(list[6][0] == 'A') m_gps.valid = true;
        if(list[6][0] == 'V') m_gps.valid = false;

        printf("Lat %f Lon %f Alt %d Time %.2d:%.2d:%.2d Nsats %d\n\r",m_gps.lat,m_gps.lon,m_gps.alt,m_gps.hour, m_gps.min, m_gps.sec,m_gps.nsats);
    }
}
void *gps_thread( void *arg){
    FILE *fp;
    char nmea[1025];
    if((fp=fopen("/dev/ttyACM0","rw"))!=NULL){
	    while(m_gps_running == true){
	    	if(fgets( nmea, 1024, fp)!= NULL){
	    		gps_parse_nmea(nmea);
	    	}
	    }
    }
    printf("File open failed \n\r");
	return arg;
}

int gps_open(void){
	m_gps_running = true;
	if(pthread_create(&m_tid, NULL, &gps_thread, NULL)!=0){
        printf("GPS thread cannot be started\n\r");
        return -1;
    }
	return 0;
}
void gps_close(void){
	m_gps_running = false;
}
