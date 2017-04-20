################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/rf_uart_debug/interrupt.c \
../vendor/rf_uart_debug/main.c 

OBJS += \
./vendor/rf_uart_debug/interrupt.o \
./vendor/rf_uart_debug/main.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/rf_uart_debug/%.o: ../vendor/rf_uart_debug/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"D:\Thread_MeshCop\Intel_Mesh\sensor_mesh_txrx_demo\sensor_mesh_pallet" -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


