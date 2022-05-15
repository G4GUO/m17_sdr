################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../audio_io.cpp \
../lime_rx.cpp \
../m17_bit_utils.cpp \
../m17_conv.cpp \
../m17_correlate.cpp \
../m17_crc.cpp \
../m17_dsp.cpp \
../m17_golay.cpp \
../m17_interleave.cpp \
../m17_modulate.cpp \
../m17_puncture.cpp \
../m17_rx_frame.cpp \
../m17_rx_parse.cpp \
../m17_tests.cpp \
../m17_tx_routines.cpp \
../main.cpp 

OBJS += \
./audio_io.o \
./lime_rx.o \
./m17_bit_utils.o \
./m17_conv.o \
./m17_correlate.o \
./m17_crc.o \
./m17_dsp.o \
./m17_golay.o \
./m17_interleave.o \
./m17_modulate.o \
./m17_puncture.o \
./m17_rx_frame.o \
./m17_rx_parse.o \
./m17_tests.o \
./m17_tx_routines.o \
./main.o 

CPP_DEPS += \
./audio_io.d \
./lime_rx.d \
./m17_bit_utils.d \
./m17_conv.d \
./m17_correlate.d \
./m17_crc.d \
./m17_dsp.d \
./m17_golay.d \
./m17_interleave.d \
./m17_modulate.d \
./m17_puncture.d \
./m17_rx_frame.d \
./m17_rx_parse.d \
./m17_tests.d \
./m17_tx_routines.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/charles/gits/codec2/src -I/home/charles/gits/codec2/build -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


