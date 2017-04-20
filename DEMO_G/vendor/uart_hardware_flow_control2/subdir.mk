################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/uart_hardware_flow_control2/interrupt.c \
../vendor/uart_hardware_flow_control2/main.c 

OBJS += \
./vendor/uart_hardware_flow_control2/interrupt.o \
./vendor/uart_hardware_flow_control2/main.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/uart_hardware_flow_control2/%.o: ../vendor/uart_hardware_flow_control2/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"D:\Thread_MeshCop\Intel_Mesh\sensor_mesh_txrx_demo\sensor_mesh_pallet" -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


