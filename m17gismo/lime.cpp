/**
    @file   basicRX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  minimal RX example
 */
#include "LimeSuite.h"
#include <fftw3.h>
#include <iostream>
#include <chrono>
#include <math.h>
#include <unistd.h>

#include "m17defines.h"
using namespace std;

#define REAL 0
#define IMAG 1
//Device structure, should be initialize to NULL
lms_device_t* m_device = NULL;
lms_stream_t m_tx_streamId; //stream structure
lms_stream_t m_rx_streamId; //stream structure
lms_stream_meta_t m_metadata;


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
void lime_show_agc(void){
	uint16_t val0,val1;
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
}
void lime_set_agc(uint32_t gain, uint16_t des, uint16_t mode, uint16_t avg){
    uint16_t val;
	// loop gain
	val = gain&0x0FFFF;// LSB
	LMS_WriteLMSReg(m_device, 0x0408, val);
	val = (gain>>16)&0x03;
	// Desired value
	val |= (des<<4);
	LMS_WriteLMSReg(m_device, 0x0409, val);
	val = (mode<<12);
    val |= avg&0x07;
	LMS_WriteLMSReg(m_device, 0x040A, val);
}

int lime_open(void)
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
    if (LMS_Open(&m_device, list[0], NULL))
        error();

    //Initialize device with default configuration
    //Do not use if you want to keep existing configuration
    //Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
    if (LMS_Init(m_device) != 0)
        error();

    //Enable RX channel
    //Channels are numbered starting at 0
    if (LMS_EnableChannel(m_device, LMS_CH_RX, 0, true) != 0)
        error();

    //Set center frequency to 434 MHz
    if (LMS_SetLOFrequency(m_device, LMS_CH_RX, 0, 434000000) != 0)
        error();

    //Set sample rate to 48K, ask to use 10x oversampling in RF
    //This set sampling rate for all channels SRATE
    if (LMS_SetSampleRate(m_device, 48020, 64) != 0)
        error();

    LMS_SetNormalizedGain(m_device, LMS_CH_RX, 0, 0.5);
    LMS_SetAntenna(m_device,LMS_CH_RX,0,1);
    //LMS_SetAntenna(m_device,LMS_CH_RX,0,3); // Lime USB Wideband port
    LMS_SetLPFBW(m_device, LMS_CH_RX,0,5000000);

    //Streaming Setup
    //Initialize data buffers

    //Initialize RX stream
    m_rx_streamId.channel = 0; //channel number
    m_rx_streamId.fifoSize = 1; //fifo size
    m_rx_streamId.throughputVsLatency = 0.01; //optimize for max throughput
    m_rx_streamId.isTx = false; //RX channel
    m_rx_streamId.dataFmt = lms_stream_t::LMS_FMT_I16; //16-bit integers
    if (LMS_SetupStream(m_device, &m_rx_streamId) != 0)
        error();
    //
    // Do the same but for the tx stream
    //

    //Channels are numbered starting at 0
    if (LMS_EnableChannel(m_device, LMS_CH_TX, 0, true) != 0)
        error();

    //Set center frequency to 434 MHz
    if (LMS_SetLOFrequency(m_device, LMS_CH_TX, 0, 434000000) != 0)
        error();


    LMS_SetNormalizedGain(m_device, LMS_CH_TX, 0, 0.8);
    LMS_SetAntenna(m_device,LMS_CH_TX,0,1); // Lime USB Wideband port
    //LMS_SetAntenna(m_device,LMS_CH_TX,0,3); // Lime USB Wideband port
    LMS_SetLPFBW(m_device, LMS_CH_TX,0,5000000);

    //Streaming Setup

    //Initialize TX stream
    m_tx_streamId.channel = 0; //channel number
    m_tx_streamId.fifoSize = 1; //fifo size
    m_tx_streamId.throughputVsLatency = 0.01; //optimize for max throughput
    m_tx_streamId.isTx = true; //TX channel
    m_tx_streamId.dataFmt = lms_stream_t::LMS_FMT_I16; //16-bit integers
    if (LMS_SetupStream(m_device, &m_tx_streamId) != 0)
        error();
    // Calibrate receive and transmit
    LMS_StartStream(&m_rx_streamId);
    LMS_StartStream(&m_tx_streamId);
    LMS_Calibrate(m_device,LMS_CH_RX,0,4000000,0);
    LMS_Calibrate(m_device,LMS_CH_TX,0,4000000,0);
    LMS_StopStream(&m_rx_streamId);
    LMS_StopStream(&m_tx_streamId);

    lime_set_agc(0x3FFFF, 0xFFF, 0, 0x07);
   // lime_show_agc();


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
   LMS_StartStream(&m_rx_streamId); //stream is stopped but can be started again with LMS_StartStream()
   LMS_StartStream(&m_tx_streamId);
}
void lime_got_to_transmit(void){
   LMS_StopStream(&m_rx_streamId); //stream is stopped but can be started again with LMS_StartStream()
   LMS_StartStream(&m_tx_streamId);
}
void lime_got_to_receive(void){
   LMS_StopStream(&m_tx_streamId); //stream is stopped but can be started again with LMS_StartStream()
   LMS_StartStream(&m_rx_streamId);
}
void lime_set_freq(uint64_t freq){
    LMS_SetLOFrequency(m_device, LMS_CH_TX, 0, freq);
    LMS_SetLOFrequency(m_device, LMS_CH_RX, 0, freq);
}
void lime_close(void){
    LMS_StopStream(&m_rx_streamId);
    LMS_DestroyStream(m_device, &m_rx_streamId); //stream is deallocated and can no longer be used
    LMS_EnableChannel(m_device, LMS_CH_RX, 0, false);
    LMS_StopStream(&m_tx_streamId);
    LMS_DestroyStream(m_device, &m_tx_streamId);
    LMS_EnableChannel(m_device, LMS_CH_TX, 0, false);
    //Close device
    usleep(10000);
    LMS_Close(m_device);
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

