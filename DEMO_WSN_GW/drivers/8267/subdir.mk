################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../drivers/8267/adc.o \
../drivers/8267/aes_128.o \
../drivers/8267/bsp.o \
../drivers/8267/div_mod.o \
../drivers/8267/emitest.o \
../drivers/8267/flash.o \
../drivers/8267/gpio.o \
../drivers/8267/i2c.o \
../drivers/8267/irq.o \
../drivers/8267/pa.o \
../drivers/8267/pm.o \
../drivers/8267/pwm.o \
../drivers/8267/rf.o \
../drivers/8267/spi.o \
../drivers/8267/uart.o 

C_SRCS += \
../drivers/8267/adc.c \
../drivers/8267/aes_128.c \
../drivers/8267/audio.c \
../drivers/8267/bsp.c \
../drivers/8267/emitest.c \
../drivers/8267/flash.c \
../drivers/8267/gpio.c \
../drivers/8267/i2c.c \
../drivers/8267/irq.c \
../drivers/8267/pa.c \
../drivers/8267/pm.c \
../drivers/8267/pwm.c \
../drivers/8267/rf.c \
../drivers/8267/spi.c \
../drivers/8267/uart.c \
../drivers/8267/usb_hw.c 

S_UPPER_SRCS += \
../drivers/8267/div_mod.S 

OBJS += \
./drivers/8267/adc.o \
./drivers/8267/aes_128.o \
./drivers/8267/audio.o \
./drivers/8267/bsp.o \
./drivers/8267/div_mod.o \
./drivers/8267/emitest.o \
./drivers/8267/flash.o \
./drivers/8267/gpio.o \
./drivers/8267/i2c.o \
./drivers/8267/irq.o \
./drivers/8267/pa.o \
./drivers/8267/pm.o \
./drivers/8267/pwm.o \
./drivers/8267/rf.o \
./drivers/8267/spi.o \
./drivers/8267/uart.o \
./drivers/8267/usb_hw.o 


# Each subdirectory must supply rules for building sources it contributes
drivers/8267/%.o: ../drivers/8267/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=c99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/8267/%.o: ../drivers/8267/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 CC/Assembler'
	tc32-elf-gcc  -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


