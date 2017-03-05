################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../util/app.cpp \
../util/bytes.cpp \
../util/config.cpp \
../util/log.cpp \
../util/sorted_set.cpp \
../util/test_sorted_set.cpp 

O_SRCS += \
../util/app.o \
../util/bytes.o \
../util/config.o \
../util/log.o \
../util/sorted_set.o 

OBJS += \
./util/app.o \
./util/bytes.o \
./util/config.o \
./util/log.o \
./util/sorted_set.o \
./util/test_sorted_set.o 

CPP_DEPS += \
./util/app.d \
./util/bytes.d \
./util/config.d \
./util/log.d \
./util/sorted_set.d \
./util/test_sorted_set.d 


# Each subdirectory must supply rules for building sources it contributes
util/%.o: ../util/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


