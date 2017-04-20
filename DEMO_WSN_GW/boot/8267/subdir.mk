################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../boot/8267/cstartup_8267.S 

OBJS += \
./boot/8267/cstartup_8267.o 


# Each subdirectory must supply rules for building sources it contributes
boot/8267/%.o: ../boot/8267/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 CC/Assembler'
	tc32-elf-gcc  -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


