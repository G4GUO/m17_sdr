################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../audio_io.cpp \
../buffers.cpp \
../gps.cpp \
../gui.cpp \
../lime.cpp \
../m17_bit_utils.cpp \
../m17_conv.cpp \
../m17_correlate.cpp \
../m17_crc.cpp \
../m17_dbase.cpp \
../m17_dsp.cpp \
../m17_equalize.cpp \
../m17_golay.cpp \
../m17_interleave.cpp \
../m17_modulate.cpp \
../m17_net.cpp \
../m17_prbs9.cpp \
../m17_puncture.cpp \
../m17_rx_frame.cpp \
../m17_rx_parse.cpp \
../m17_rx_sync.cpp \
../m17_test.cpp \
../m17_tx_routines.cpp \
../m17_tx_rx.cpp \
../main.cpp \
../mmi.cpp \
../pluto.cpp \
../radio.cpp \
../rpi_gpio.cpp 

CPP_DEPS += \
./audio_io.d \
./buffers.d \
./gps.d \
./gui.d \
./lime.d \
./m17_bit_utils.d \
./m17_conv.d \
./m17_correlate.d \
./m17_crc.d \
./m17_dbase.d \
./m17_dsp.d \
./m17_equalize.d \
./m17_golay.d \
./m17_interleave.d \
./m17_modulate.d \
./m17_net.d \
./m17_prbs9.d \
./m17_puncture.d \
./m17_rx_frame.d \
./m17_rx_parse.d \
./m17_rx_sync.d \
./m17_test.d \
./m17_tx_routines.d \
./m17_tx_rx.d \
./main.d \
./mmi.d \
./pluto.d \
./radio.d \
./rpi_gpio.d 

OBJS += \
./audio_io.o \
./buffers.o \
./gps.o \
./gui.o \
./lime.o \
./m17_bit_utils.o \
./m17_conv.o \
./m17_correlate.o \
./m17_crc.o \
./m17_dbase.o \
./m17_dsp.o \
./m17_equalize.o \
./m17_golay.o \
./m17_interleave.o \
./m17_modulate.o \
./m17_net.o \
./m17_prbs9.o \
./m17_puncture.o \
./m17_rx_frame.o \
./m17_rx_parse.o \
./m17_rx_sync.o \
./m17_test.o \
./m17_tx_routines.o \
./m17_tx_rx.o \
./main.o \
./mmi.o \
./pluto.o \
./radio.o \
./rpi_gpio.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/codec2 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean--2e-

clean--2e-:
	-$(RM) ./audio_io.d ./audio_io.o ./buffers.d ./buffers.o ./gps.d ./gps.o ./gui.d ./gui.o ./lime.d ./lime.o ./m17_bit_utils.d ./m17_bit_utils.o ./m17_conv.d ./m17_conv.o ./m17_correlate.d ./m17_correlate.o ./m17_crc.d ./m17_crc.o ./m17_dbase.d ./m17_dbase.o ./m17_dsp.d ./m17_dsp.o ./m17_equalize.d ./m17_equalize.o ./m17_golay.d ./m17_golay.o ./m17_interleave.d ./m17_interleave.o ./m17_modulate.d ./m17_modulate.o ./m17_net.d ./m17_net.o ./m17_prbs9.d ./m17_prbs9.o ./m17_puncture.d ./m17_puncture.o ./m17_rx_frame.d ./m17_rx_frame.o ./m17_rx_parse.d ./m17_rx_parse.o ./m17_rx_sync.d ./m17_rx_sync.o ./m17_test.d ./m17_test.o ./m17_tx_routines.d ./m17_tx_routines.o ./m17_tx_rx.d ./m17_tx_rx.o ./main.d ./main.o ./mmi.d ./mmi.o ./pluto.d ./pluto.o ./radio.d ./radio.o ./rpi_gpio.d ./rpi_gpio.o

.PHONY: clean--2e-

