# m17_sdr

SDR software for implementing M17 radio protocol with either a Pluto / Pluto+ or LimeSDR


On a fresh RPi install the following packages need to be installed

pulseaudio and libpulse_dev 

    sudo apt-get install pulseaudio 
    sudo aptitude install libpulse-dev 

ncurses

    sudo apt-get install ncurses_dev

libiio_dev

    sudo apt-get install libiio_dev

Limesuite

    sudo apt-get install limesuite limesuite_dev

cmake

    sudo apt-get install cmake

codec2

    git clone https://github.com/drowe67/codec2.git
    cd codec2
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
