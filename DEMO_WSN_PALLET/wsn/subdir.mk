################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../wsn/frame.c \
../wsn/message_queue.c \
../wsn/pallet.c 

OBJS += \
./wsn/frame.o \
./wsn/message_queue.o \
./wsn/pallet.o 


# Each subdirectory must supply rules for building sources it contributes
wsn/%.o: ../wsn/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


