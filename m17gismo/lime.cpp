/**
    @file   basicRX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  minimal RX example
 */
#include "LimeSuite.h"
#include <iostream>
#include <chrono>
#include <math.h>
#include <unistd.h>
#include <string.h>

#include "m17defines.h"
using namespace std;

#define REAL 0
#define IMAG 1
#define CHAN_A 0
#define CHAN_B 1

//Device structure, should be initialize to NULL
static lms_device_t* m_device = NULL;
static lms_stream_t m_tx_streamId; //stream structure
static lms_stream_t m_rx_streamId; //stream structure
static lms_stream_meta_t m_metadata;
static size_t m_tx_chan_n;
static size_t m_rx_chan_n;
static double m_tx_lvl;

int error()
{
    if (m_device != NULL)
        LMS_Close(m_device);
    exit(-1);
}

/*
int lime_main(void)
{

    //Find devices
    int n;
    lms_info_str_t list[8]; //should be large enough to hold all detected devices
    if ((n = LMS_GetDeviceList(list)) < 0) //NULL can be passed to only get number of devices
        error();

    cout << "Devices found: " << n << endl; //print number of devices
    if (n < 1)
        return -1;

    //open the first device
    if (LMS_Open(&device, list[0], NULL))
        error();

    //Initialize device with default configuration
    //Do not use if you want to keep existing configuration
    //Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
    if (LMS_Init(device) != 0)
        error();

    //Enable RX channel
    //Channels are numbered starting at 0
    if (LMS_EnableChannel(device, LMS_CH_RX, 0, true) != 0)
        error();

    //Set center frequency to 434 MHz
    if (LMS_SetLOFrequency(device, LMS_CH_RX, 0, 434000000) != 0)
        error();

    //Set sample rate to 48K, ask to use 10x oversampling in RF
    //This set sampling rate for all channels
    if (LMS_SetSampleRate(device, SRATE, 32) != 0)
        error();

    LMS_SetNormalizedGain(device, LMS_CH_RX, 0, 0.999);
    //LMS_SetAntenna(device,LMS_CH_RX,0,1);
    LMS_SetAntenna(device,LMS_CH_RX,0,3); // Lime USB Wideband port
    LMS_SetLPFBW(device, LMS_CH_RX,0,5000000);
    //Enable test signal generation
    //To receive data from RF, remove this line or change signal to LMS_TESTSIG_NONE
//    if (LMS_SetTestSignal(device, LMS_CH_RX, 0, LMS_TESTSIG_NCODIV8, 0, 0) != 0)
//        error();

    //Streaming Setup
    //Initialize data buffers
    const int sampleCnt = N_SAMPLES; //complex samples per buffer

    //Initialize stream
    m_rx_streamId.channel = 0; //channel number
    m_rx_streamId.fifoSize = 1; //fifo size in samples
    m_rx_streamId.throughputVsLatency = 1.0; //optimize for max throughput
    m_rx_streamId.isTx = false; //RX channel
    m_rx_streamId.dataFmt = lms_stream_t::LMS_FMT_I16; //16-bit integers
    if (LMS_SetupStream(device, &m_rx_streamId) != 0)
        error();

    //Start streaming
    LMS_StartStream(&m_rx_streamId);
    LMS_Calibrate(device,LMS_CH_RX,0,4000000,0);

    //Streaming
    while (1)
    {
        //Receive samples
        int samplesRead = LMS_RecvStream(&m_rx_streamId, buffer, sampleCnt, NULL, 100);
	//I and Q samples are interleaved in buffer: IQIQIQ...
       // printf("Received %d samples\n", samplesRead);

		INSERT CODE FOR PROCESSING RECEIVED SAMPLES

    m17_process_samples(buffer, samplesRead);

    }
    //Stop streaming
    LMS_StopStream(&m_rx_streamId); //stream is stopped but can be started again with LMS_StartStream()
    LMS_DestroyStream(device, &m_rx_streamId); //stream is deallocated and can no longer be used

    //Close device
    LMS_Close(device);

    return 0;
}
*/
void lime_sel_ab_chan(int chan){
	uint16_t v;

	if(chan == CHAN_A){
    	// A channel
		// MAC[1:0]
		LMS_ReadLMSReg( m_device, 0x0020, &v);
		v = (v&0xFFFC)| 0x01;
		LMS_WriteLMSReg(m_device, 0x0020, v);

    }else{
    	// Chan B
		LMS_ReadLMSReg( m_device, 0x0020, &v);
		v = (v&0xFFFC)| 0x02;
		LMS_WriteLMSReg(m_device, 0x0020, v);
    }
}
void lime_desel_ab_chan(void){
	uint16_t v;
	// MAC[1:0]
	LMS_ReadLMSReg( m_device, 0x0020, &v);
	v = (v&0xFFFC)| 0x00;
	LMS_WriteLMSReg(m_device, 0x0020, v);

}
void lime_setup_rssi(void){
	uint16_t v;

	lime_sel_ab_chan(CHAN_A);

	//Dont bypass AGC
	LMS_ReadLMSReg( m_device, 0x040C, &v);
	v = v&0xFFBF; // [6] = 0
	LMS_WriteLMSReg(m_device, 0x040C, v);

	// Set to RSSI
	LMS_ReadLMSReg( m_device, 0x040A, &v);
	v = (v&0xCFFF)|(0x1000); //[13:12] = 1
	LMS_WriteLMSReg(m_device, 0x040A, v);

	//RSSI to be captured registsers [14:13] = 0
	LMS_ReadLMSReg( m_device, 0x0400, &v);
	v = v&0x9FFF; // [14:13] = 0
	LMS_WriteLMSReg(m_device, 0x0400, v);

	// Set RSSI Period [2:0] = 0;
	LMS_ReadLMSReg( m_device, 0x040A, &v);
	v = (v&0xFFF8)|0x03;// Set to 0
	LMS_WriteLMSReg(m_device, 0x040A, v);

	lime_desel_ab_chan();

}
uint32_t lime_read_rssi(void){
	uint16_t v;
    uint32_t rssi = 0;
	if(m_device != NULL ){
	    lime_sel_ab_chan(CHAN_A);

	    // Toggle MSB
	    LMS_ReadLMSReg( m_device, 0x0400, &v);
	    v = v&0x7FFF;//0
	    LMS_WriteLMSReg(m_device, 0x0400, v);
	    // Delay !!
	    //usleep(50);
	    LMS_ReadLMSReg( m_device, 0x0400, &v);
	    v = v | 0x8000;//1
	    LMS_WriteLMSReg(m_device, 0x0400, v);

	    // Read the value
	    LMS_ReadLMSReg( m_device, 0x040F, &v);
	    rssi = v;
	    rssi <<= 2;
	    LMS_ReadLMSReg( m_device, 0x040E, &v);
	    rssi |= v&0x0003;

	    lime_desel_ab_chan();
	}
	return rssi;
}
void lime_show_agc(void){
	uint16_t val0,val1;

	lime_sel_ab_chan(CHAN_A);

	LMS_ReadLMSReg(m_device, 0x0408, &val0);
	LMS_ReadLMSReg(m_device, 0x0409, &val1);
	printf("AGC Desired %.3x\n",val1>>4);
	uint32_t gain = (val1&0x03);
	gain <<= 16;
	gain |= val0;
	printf("AGC loop gain %.5x\n",gain);
	LMS_ReadLMSReg(m_device, 0x040A, &val0);
	printf("AGC Mode %d\n",(val0>>12)&0x03);
	printf("AGC AVG %.1x\n",val0&0x07);

	lime_desel_ab_chan();
}
void lime_set_agc(uint32_t gain, uint16_t des, uint16_t avg){
    uint16_t v;
    lime_sel_ab_chan(CHAN_A);

    // Set to AGC
    LMS_ReadLMSReg( m_device, 0x040A, &v);
	v = (v&0xCFF8)|(0x0000); //[13:12] = 00
    v = v | (avg&0x07);//[2:0]
	LMS_WriteLMSReg(m_device, 0x040A, v);

    // loop gain MSB [15:0]
	v = gain&0x0FFFF;// LSB
	LMS_WriteLMSReg(m_device, 0x0408, v);

    // loop gain MSB [17:16]
	v = (gain>>16)&0x03;
	// Desired value [11:0]
	v |= (des<<4);
	LMS_WriteLMSReg(m_device, 0x0409, v);

	lime_desel_ab_chan();
}
void lime_set_vctcxo_correction(uint16_t val ){
	LMS_VCTCXOWrite(m_device, val);
}
void lime_get_vctcxo_correction( void ){
	uint16_t val;
	if(LMS_VCTCXORead(m_device, &val)==0){
		printf("Lime VCTXCO %.4x\n",val);
	}
}

//
// GPIO handling code
//

#define PTT_IN  0x01
#define PTT_OUT 0x02

static void lime_gpio_read( uint8_t *b){
	if(m_device != NULL) LMS_GPIORead(m_device, b, 1);
}
static void lime_gpio_write( uint8_t b){
	if(m_device != NULL) LMS_GPIOWrite(m_device, &b, 1);
}
static void lime_configure_gpio(void){
	// 0 = input 1 = output
	uint8_t b = PTT_OUT; // All input except PTT_OUT
	if(m_device != NULL) LMS_GPIODirWrite(m_device, &b, 1);
}
//
// Externally visible
//
bool lime_read_ptt(void){
	uint8_t b = 0;
	lime_gpio_read( &b);
	bool res = (b&PTT_IN) ? false : true;
	return res;
}
void lime_ptt_tx(void){
	uint8_t b = 0;
	lime_gpio_read(&b);
	b |= PTT_OUT;
	lime_gpio_write( b);
}
void lime_ptt_rx(void){
	uint8_t b = 0;
	uint8_t m;
	lime_gpio_read(&b);
	m = (0xFF ^ PTT_OUT);
	b &= m;
	lime_gpio_write(b);
}

int lime_open(void)
{
    //Find devices
    int n;
    m_device = NULL;
    m_tx_chan_n = 0;
    m_rx_chan_n = 0;

    lms_info_str_t list[8]; //should be large enough to hold all detected devices
    if ((n = LMS_GetDeviceList(list)) < 0) //NULL can be passed to only get number of devices
        error();

    cout << "Devices found: " << n << endl; //print number of devices
    if (n < 1)
        return -1;

    for( int i = 0; i < n; i++){
        cout << "Name: " << list[i] << endl; //print number of devices
    }

    if(strncmp((char*)list[0],"LimeSDR-USB", 11) == 0){
    	m_rx_chan_n = 0;
    	m_tx_chan_n = 1;
		//cout <<"USB Found" << endl;
    }

    if(strncmp((char*)list[0],"LimeSDR Mini", 12) == 0){
    	m_rx_chan_n = 0;
    	m_tx_chan_n = 0;
		//cout <<"USB Found" << endl;
    }

    //open the first device
    if (LMS_Open(&m_device, list[0], NULL))
        error();

    //Initialize device with default configuration
    //Do not use if you want to keep existing configuration
    //Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
    if (LMS_Init(m_device) != 0)
        error();

    //Enable RX channel
    //Channels are numbered starting at 0
    if (LMS_EnableChannel(m_device, LMS_CH_RX,  m_rx_chan_n, true) != 0)
        error();

    //Set center frequency to 434 MHz
    if (LMS_SetLOFrequency(m_device, LMS_CH_RX, m_rx_chan_n, 434000000) != 0)
        error();

    //Set sample rate to 48K, ask to use 10x oversampling in RF
    //This set sampling rate for all channels SRATE
    if (LMS_SetSampleRate(m_device, 48000, 32) != 0)
        error();

    LMS_SetNormalizedGain(m_device, LMS_CH_RX, m_rx_chan_n, 1.0);
    //LMS_SetAntenna(m_device,LMS_CH_RX,m_rx_chan_n,1); // Lime Mini Wideband port
    LMS_SetAntenna(m_device,LMS_CH_RX,m_rx_chan_n,3); // Lime USB Wideband port
    LMS_SetLPFBW(m_device, LMS_CH_RX,m_rx_chan_n,1500000);

    //Streaming Setup
    //Initialize data buffers

    //Initialize RX stream
    m_rx_streamId.channel = m_rx_chan_n; //channel number
    m_rx_streamId.fifoSize = 1; //fifo size
    m_rx_streamId.throughputVsLatency = 1.0; //optimize for max throughput
    m_rx_streamId.isTx = false; //RX channel
    m_rx_streamId.dataFmt = lms_stream_t::LMS_FMT_I16; //16-bit integers
    if (LMS_SetupStream(m_device, &m_rx_streamId) != 0)
        error();
    //
    // Do the same but for the tx stream
    //

    //Channels are numbered starting at 0
    if (LMS_EnableChannel(m_device, LMS_CH_TX, m_tx_chan_n, true) != 0)
        error();

    //Set center frequency to 434 MHz
    if (LMS_SetLOFrequency(m_device, LMS_CH_TX, m_tx_chan_n, 434000000) != 0)
        error();

    m_tx_lvl = 0.5;

    LMS_SetNormalizedGain(m_device, LMS_CH_TX, m_tx_chan_n, m_tx_lvl);
    //LMS_SetAntenna(m_device,LMS_CH_TX,m_tx_chan_n,1); // Lime Mini Wideband port
    LMS_SetAntenna(m_device,LMS_CH_TX,m_tx_chan_n,3); // Lime USB Wideband port
    LMS_SetLPFBW(m_device, LMS_CH_TX,m_tx_chan_n,5000000);

    //Streaming Setup

    //Initialize TX stream
    m_tx_streamId.channel = m_tx_chan_n; //channel number
    m_tx_streamId.fifoSize = 1; //fifo size
    m_tx_streamId.throughputVsLatency = 1.0; //optimize for max throughput
    m_tx_streamId.isTx = true; //TX channel
    m_tx_streamId.dataFmt = lms_stream_t::LMS_FMT_I16; //16-bit integers
    if (LMS_SetupStream(m_device, &m_tx_streamId) != 0)
        error();
    // Calibrate receive and transmit
    LMS_StartStream(&m_rx_streamId);
    LMS_StartStream(&m_tx_streamId);
    LMS_Calibrate(m_device,LMS_CH_RX,m_rx_chan_n,4000000,0);
    LMS_Calibrate(m_device,LMS_CH_TX,m_tx_chan_n,4000000,0);
    LMS_StopStream(&m_rx_streamId);
    LMS_StopStream(&m_tx_streamId);

    // Set the GPIO pins
    lime_configure_gpio();

    // Setup RSSI
    lime_setup_rssi();

//    lime_set_agc(0x3FFFF, 0x3FF, 0x00);
//    lime_show_agc();

//    lime_set_vctcxo_correction(0x0000);
//    lime_get_vctcxo_correction();



//    uint16_t val = 0xFFFF;
//    LMS_VCTCXOWrite(m_device, val);
//    if(LMS_VCTCXORead(m_device, &val)==0){
//        printf("VCTCXO value %d\n",val);
//    }
//    else
//    	printf("VCTCXO read Failed\n");

    return 0;
}
void lime_got_to_duplex(void){
    LMS_SetNormalizedGain(m_device, LMS_CH_TX, m_tx_chan_n, m_tx_lvl);
    LMS_StartStream(&m_rx_streamId); //stream is stopped but can be started again with LMS_StartStream()
    LMS_StartStream(&m_tx_streamId);
}
void lime_got_to_transmit(void){
    LMS_SetNormalizedGain(m_device, LMS_CH_TX, m_tx_chan_n, m_tx_lvl);
    LMS_StopStream(&m_rx_streamId); //stream is stopped but can be started again with LMS_StartStream()
    LMS_StartStream(&m_tx_streamId);
}
void lime_got_to_receive(void){
    // Set the gain in the transmitter chain to lowest to prevent leakage on receive
    LMS_SetNormalizedGain(m_device, LMS_CH_TX, m_tx_chan_n, 0.0);// Set lowest value
    LMS_StopStream(&m_tx_streamId); //stream is stopped but can be started again with LMS_StartStream()
    LMS_StartStream(&m_rx_streamId);
}
void lime_set_tx_freq(uint64_t freq){
    LMS_SetLOFrequency(m_device, LMS_CH_TX, m_tx_chan_n, freq);
}
void lime_set_rx_freq(uint64_t freq){
    LMS_SetLOFrequency(m_device, LMS_CH_RX, m_rx_chan_n, freq);
}
void lime_close(void){
	if(m_device != NULL ){
        LMS_StopStream(&m_rx_streamId);
        LMS_DestroyStream(m_device, &m_rx_streamId); //stream is deallocated and can no longer be used
        LMS_EnableChannel(m_device, LMS_CH_RX, m_rx_chan_n, false);
        LMS_StopStream(&m_tx_streamId);
        LMS_DestroyStream(m_device, &m_tx_streamId);
        LMS_EnableChannel(m_device, LMS_CH_TX, m_tx_chan_n, false);
        LMS_Close(m_device);
        m_device = NULL;
	}
}
int lime_transmit_samples( int16_t *in, int len){
	int samplesSent = LMS_SendStream(&m_tx_streamId, in, len, &m_metadata, 100);
	//printf("LIME sent %4X\n",in[0]);
	return samplesSent;
}
int lime_receive_samples( int16_t *samples, int len){
    //Receive samples
    int samplesRead = LMS_RecvStream(&m_rx_streamId, samples, len, NULL, 100);
    //I and Q samples are interleaved in buffer: IQIQIQ...
    //printf("Received %d samples\n", samplesRead);
    return samplesRead;
}
void lime_set_tx_gain(float gain){
    m_tx_lvl = gain;
    LMS_SetNormalizedGain(m_device, LMS_CH_TX, m_tx_chan_n, m_tx_lvl);
}
void lime_set_rx_gain(double gain){
    LMS_SetNormalizedGain(m_device, LMS_CH_RX, m_rx_chan_n, gain);
}
double lime_get_rx_gain(void){
	double gain;
    LMS_GetNormalizedGain(m_device, LMS_CH_RX, m_rx_chan_n, &gain);
    return gain;
}

