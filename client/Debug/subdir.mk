################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../fde.cpp \
../fde_epoll.cpp \
../fde_select.cpp \
../link.cpp \
../msg.cpp \
../net.cpp \
../resp.cpp \
../server.cpp 

O_SRCS += \
../fde.o \
../link.o \
../msg.o \
../net.o \
../resp.o \
../server.o 

OBJS += \
./fde.o \
./fde_epoll.o \
./fde_select.o \
./link.o \
./msg.o \
./net.o \
./resp.o \
./server.o 

CPP_DEPS += \
./fde.d \
./fde_epoll.d \
./fde_select.d \
./link.d \
./msg.d \
./net.d \
./resp.d \
./server.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


