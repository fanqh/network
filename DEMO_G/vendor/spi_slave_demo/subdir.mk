################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/spi_slave_demo/interrupt.c \
../vendor/spi_slave_demo/main.c 

OBJS += \
./vendor/spi_slave_demo/interrupt.o \
./vendor/spi_slave_demo/main.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/spi_slave_demo/%.o: ../vendor/spi_slave_demo/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -I"D:\Thread_MeshCop\Intel_Mesh\sensor_mesh_txrx_demo\sensor_mesh_pallet" -Wall -O3 -std=c99 -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


