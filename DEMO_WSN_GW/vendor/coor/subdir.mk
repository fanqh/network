################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/coor/interrupt.c \
../vendor/coor/main.c 

OBJS += \
./vendor/coor/interrupt.o \
./vendor/coor/main.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/coor/%.o: ../vendor/coor/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -DPA_MODE=1 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


