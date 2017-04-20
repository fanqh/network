################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/internal32K_demo/interrupt.c \
../vendor/internal32K_demo/main.c 

OBJS += \
./vendor/internal32K_demo/interrupt.o \
./vendor/internal32K_demo/main.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/internal32K_demo/%.o: ../vendor/internal32K_demo/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -I"D:\Thread_MeshCop\Intel_Mesh\sensor_mesh_txrx_demo\sensor_mesh_pallet" -Wall -O2 -std=c99 -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


