################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../audio_io.cpp \
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
../m17_prbs9.cpp \
../m17_puncture.cpp \
../m17_rx_frame.cpp \
../m17_rx_parse.cpp \
../m17_rx_sync.cpp \
../m17_test.cpp \
../m17_tx_routines.cpp \
../m17_tx_rx.cpp \
../main.cpp \
../mmi.cpp 

OBJS += \
./audio_io.o \
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
./m17_prbs9.o \
./m17_puncture.o \
./m17_rx_frame.o \
./m17_rx_parse.o \
./m17_rx_sync.o \
./m17_test.o \
./m17_tx_routines.o \
./m17_tx_rx.o \
./main.o \
./mmi.o 

CPP_DEPS += \
./audio_io.d \
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
./m17_prbs9.d \
./m17_puncture.d \
./m17_rx_frame.d \
./m17_rx_parse.d \
./m17_rx_sync.d \
./m17_test.d \
./m17_tx_routines.d \
./m17_tx_rx.d \
./main.d \
./mmi.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/charles/gits/codec2/src -I/home/charles/gits/codec2/build -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


