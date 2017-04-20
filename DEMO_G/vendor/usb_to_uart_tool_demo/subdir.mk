################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/usb_to_uart_tool_demo/interrupt.c \
../vendor/usb_to_uart_tool_demo/main.c \
../vendor/usb_to_uart_tool_demo/usb_cdc.c \
../vendor/usb_to_uart_tool_demo/usb_ctrl.c \
../vendor/usb_to_uart_tool_demo/usb_desc.c 

OBJS += \
./vendor/usb_to_uart_tool_demo/interrupt.o \
./vendor/usb_to_uart_tool_demo/main.o \
./vendor/usb_to_uart_tool_demo/usb_cdc.o \
./vendor/usb_to_uart_tool_demo/usb_ctrl.o \
./vendor/usb_to_uart_tool_demo/usb_desc.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/usb_to_uart_tool_demo/%.o: ../vendor/usb_to_uart_tool_demo/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"D:\Thread_MeshCop\Intel_Mesh\sensor_mesh_txrx_demo\sensor_mesh_pallet" -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


