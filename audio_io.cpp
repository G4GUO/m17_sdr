/* gcc -W -Wall -O2 $(shell pkg-config --cflags libpulse-simple) -o sine sine.c -lm $(shell pkg-config --libs libpulse-simple) */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#define RATE 8000

pa_simple *m_os;
pa_simple *m_is;

static const pa_sample_spec spec = {
	.format		= PA_SAMPLE_S16LE,
	.rate		= RATE,
	.channels	= 1,
};

void audio_open(void){
	m_os = pa_simple_new(NULL, "m17", PA_STREAM_PLAYBACK, NULL, "m17", &spec, NULL, NULL, NULL);
	if (!m_os) printf("Audio Output failed\n");
	m_is = pa_simple_new(NULL, "m17", PA_STREAM_RECORD, NULL, "m17", &spec, NULL, NULL, NULL);
	if (!m_is) printf("Audio Input failed\n");
	//pa_usec_t latency = pa_simple_get_latency(m_is, NULL);
	//printf("Audio Latency %lu \n",latency);

}
void audio_close(void){
	pa_simple_drain(m_os, NULL);
	pa_simple_free(m_os);
	pa_simple_free(m_is);
}
void audio_output(int16_t *s, int len){
	pa_simple_write(m_os, s, len*sizeof(int16_t), NULL);

}
void audio_input(int16_t *s, int len){
	pa_simple_read(m_is, s, len*sizeof(int16_t), NULL);
}
void audio_input_flush(void){
	int e;
    pa_simple_flush(m_is, &e);
}
void audio_output_flush(void){
	int e;
	pa_simple_flush(m_os, &e);
}

