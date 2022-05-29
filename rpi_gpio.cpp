#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define PTT_OUT 10
#define PTT_IN  11

static int pin_direction( int pin, const void *dir ){
	char text[256];
	sprintf(text,"/sys/class/gpio/gpio%d/direction",pin);
    int fd = open(text, O_WRONLY);
    if (fd == -1) {
        // Unable to open /sys/class/gpio/gpio24/direction
    	return - 1;
    }
    int l = atoi((const char *)dir);
    if (write(fd, dir, l) != l) {
        // Error writing to /sys/class/gpio/gpio24/direction
    	return - 1;
    }
    close(fd);
    return 0;
}
static int pin_export(int pin){
	char text[10];
	int l;
    // Export the PTT pins
	int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1) {
        // Unable to open /sys/class/gpio/export
    	return - 1;
    }
    sprintf(text,"%d",pin);
    l = strlen(text);
    if (write(fd, text, l) != l) {
        // Error writing to /sys/class/gpio/export
    	return - 2;
    }
    close(fd);
	return 0;
}
static int pin_unexport(int pin){
	char text[10];
	int l;
    // Export the PTT pins
	int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) {
        // Unable to open /sys/class/gpio/export
    	return - 1;
    }
    sprintf(text,"%d",pin);
    l = strlen(text);
    if (write(fd, text, l) != l) {
        // Error writing to /sys/class/gpio/export
    	return - 2;
    }
    close(fd);
	return 0;
}

static int pin_set_value(int pin, int v ){
	char text[256];
	sprintf(text,"/sys/class/gpio/gpio%d/value",pin);
    int fd = open(text, O_WRONLY);
    if (fd == -1) {
    	return - 1;
    }
    sprintf(text,"%d", v);
    int l = strlen(text);
    if (write(fd, text, l) != l) {
    	return - 2;
    }
    close(fd);
    return 0;
}
static int pin_read_value( int pin, int &val ){
	char text[256];
	sprintf(text,"/sys/class/gpio/gpio%d/value",pin);
    int fd = open(text, O_RDONLY);
    if (fd == -1) {
    	return - 1;
    }
    if (read(fd, text, 5) < 0) {
    	return - 2;
    }
    val = atoi(text);
    close(fd);
    return 0;
}

//
// Globally visible functions
//

void rpi_tx( void ){
	pin_set_value(PTT_OUT, 1 );
}

void rpi_rx( void ){
	pin_set_value(PTT_OUT, 0 );
}

bool rpi_read_ptt(void){
	int val;
	if(pin_read_value( PTT_IN, val ) < 0){
	    return false;
    }
	if( val == 0 )
	    return true;
	else
		return false;
}

int rpi_gpio_open(void){

	if(pin_export(PTT_OUT)<0){
        return -1;
	}
	if(pin_export(PTT_IN)<0){
        return -2;
	}
    // Set the PIN directions
    if( pin_direction( PTT_OUT, "out" ) < 0 ){
        // Unable to open /sys/class/gpio/gpioPTT_OUT/direction
    	return - 3;
    }
    if( pin_direction( PTT_IN, "in" ) < 0 ){
        // Unable to open /sys/class/gpio/gpioPTT_IN/direction
    	return - 4;
    }
    return 0;
}

void rpi_gpio_close(void){
	pin_unexport(PTT_OUT);
	pin_unexport(PTT_IN);
}
