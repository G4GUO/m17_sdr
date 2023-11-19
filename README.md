# m17_sdr

SDR software for implementing M17 radio protocol with either a Pluto / Pluto+ or LimeSDR


On a fresh RPi install the following packages need to be installed

pulseaudio and libpulse-dev 

    sudo apt install pulseaudio 
    sudo apt install libpulse-dev 

ncurses

    sudo apt install ncurses-dev

libiio_dev

    sudo apt install libiio-dev

Limesuite

    sudo apt install limesuite limesuite-dev

cmake

    sudo apt install cmake

codec2

    git clone https://github.com/drowe67/codec2.git
    cd codec2
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
