################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/u64_multi_div/MulandDiv.c \
../vendor/u64_multi_div/interrupt.c \
../vendor/u64_multi_div/main.c 

OBJS += \
./vendor/u64_multi_div/MulandDiv.o \
./vendor/u64_multi_div/interrupt.o \
./vendor/u64_multi_div/main.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/u64_multi_div/%.o: ../vendor/u64_multi_div/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"D:\Thread_MeshCop\Intel_Mesh\sensor_mesh_txrx_demo\sensor_mesh_pallet" -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


