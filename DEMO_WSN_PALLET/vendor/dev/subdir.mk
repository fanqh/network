################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/dev/interrupt.c \
../vendor/dev/main.c 

OBJS += \
./vendor/dev/interrupt.o \
./vendor/dev/main.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/dev/%.o: ../vendor/dev/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"C:\Users\Administrator\AppData\Roaming\Skype\My Skype Received Files\sensor_mesh_pallet(2)\sensor_mesh_pallet" -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


