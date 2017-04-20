################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include vendor/high_speed_uart_demo/subdir.mk
-include drivers/8267/subdir.mk
-include common/subdir.mk
-include boot/8267/subdir.mk
-include subdir.mk
-include objects.mk

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LST += \
SensorMesh.lst \

FLASH_IMAGE += \
SensorMesh.bin \

SIZEDUMMY += \
sizedummy \


# All Target
all: SensorMesh.elf secondary-outputs

# Tool invocations
SensorMesh.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: TC32 C Linker'
	tc32-elf-ld  -T ../boot.link -o "SensorMesh.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

SensorMesh.lst: SensorMesh.elf
	@echo 'Invoking: TC32 Create Extended Listing'
	tc32-elf-objdump -x -D -l -S SensorMesh.elf  >"SensorMesh.lst"
	@echo 'Finished building: $@'
	@echo ' '

SensorMesh.bin: SensorMesh.elf
	@echo 'Create Flash image (binary format)'
	tc32-elf-objcopy -O binary SensorMesh.elf  "SensorMesh.bin"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: SensorMesh.elf
	@echo 'Invoking: Print Size'
	tc32-elf-size -t SensorMesh.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(LST)$(SIZEDUMMY) SensorMesh.elf
	-@echo ' '

secondary-outputs: $(LST) $(FLASH_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
