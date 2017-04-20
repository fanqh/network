################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/high_speed_uart_demo2/interrupt.c \
../vendor/high_speed_uart_demo2/main.c 

OBJS += \
./vendor/high_speed_uart_demo2/interrupt.o \
./vendor/high_speed_uart_demo2/main.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/high_speed_uart_demo2/%.o: ../vendor/high_speed_uart_demo2/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -I"D:\Thread_MeshCop\Intel_Mesh\sensor_mesh_txrx_demo\sensor_mesh_pallet" -Wall -O2 -std=c99 -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


