#include <queue>
#include <deque>
#include <list>
#include <semaphore.h>
#include <pthread.h>
#include <malloc.h>
#include "m17defines.h"

using namespace std;

#define MAX_Q_LEN 200
// tx buffer queue
static queue <uint8_t *> m_tx_q;
// spare buffer pool
static queue <uint8_t *> m_po_q;
// Mutexes
static pthread_mutex_t mutex;
//
// Allocate a new buffer
//
uint8_t *buff_alloc(void)
{
    uint8_t *b;
    pthread_mutex_lock( &mutex );
    if(m_po_q.size() > 0 )
    {
        b = m_po_q.front();
        m_po_q.pop();
    }
    else
    {
        // No available memory so allocate some
        b = (uint8_t *)malloc(54);
    }
    pthread_mutex_unlock( &mutex );
    return b;
}
int buff_size(void){
    pthread_mutex_lock( &mutex );
	int n = m_tx_q.size();
    pthread_mutex_unlock( &mutex );
    return n;
}
//
// Release a buffer
//
void buff_rel(uint8_t *b)
{
    pthread_mutex_lock( &mutex );
    m_po_q.push(b);
    pthread_mutex_unlock( &mutex );
}
//
// Post a buffer to the tx queue
//
void buff_post( uint8_t *b)
{
    pthread_mutex_lock( &mutex );
    if(m_tx_q.size() < MAX_Q_LEN)
        m_tx_q.push(b);
    else
    	m_po_q.push(b);// Queue overflow
    pthread_mutex_unlock( &mutex );
}
//
// Get a buffer from the tx queue
//
uint8_t *buff_get(void)
{
    uint8_t *b;

    pthread_mutex_lock( &mutex );
    if(m_tx_q.size() > 0 )
    {
        b = m_tx_q.front();
        m_tx_q.pop();
    }
    else
    {
        b = NULL;
    }
    pthread_mutex_unlock( &mutex );
    return b;
}
//
// Initialise the buffers
//
void buff_open(void)
{
    // Create the mutex
    pthread_mutex_init( &mutex, NULL );
}
void buff_close(void)
{
}

