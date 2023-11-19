# m17_sdr

SDR software for implementing M17 radio protocol with either a Pluto / Pluto+ or LimeSDR


On a fresh RPi install the following packages need to be installed

    sudo apt install pulseaudio libpulse-dev ncurses-dev libiio-dev limesuite liblimesuite-dev cmake
    
codec2

    git clone https://github.com/drowe67/codec2.git
    cd codec2
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE="Release" ..
    make
    sudo make install
    sudo ldconfig
