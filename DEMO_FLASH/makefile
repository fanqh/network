################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include vendor/internal_flash_read_write_demo/subdir.mk
-include print/subdir.mk
-include drivers/8267/subdir.mk
-include debug/subdir.mk
-include common/subdir.mk
-include boot/8267/subdir.mk
-include DEMO_WSN_ED/wsn/subdir.mk
-include DEMO_WSN_ED/vendor/end_dev/subdir.mk
-include DEMO_WSN_ED/drivers/8267/subdir.mk
-include DEMO_WSN_ED/common/subdir.mk
-include DEMO_WSN_ED/boot/8267/subdir.mk
-include objects.mk

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LST += \
SensorMesh_20170418.lst \

FLASH_IMAGE += \
SensorMesh_20170418.bin \

SIZEDUMMY += \
sizedummy \


# All Target
all: SensorMesh_20170418.elf secondary-outputs

# Tool invocations
SensorMesh_20170418.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: TC32 C Linker'
	tc32-elf-ld  -T ../boot.link -o"SensorMesh_20170418.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

SensorMesh_20170418.lst: SensorMesh_20170418.elf
	@echo 'Invoking: TC32 Create Extended Listing'
	tc32-elf-objdump -x -D -l -S SensorMesh_20170418.elf  >"SensorMesh_20170418.lst"
	@echo 'Finished building: $@'
	@echo ' '

SensorMesh_20170418.bin: SensorMesh_20170418.elf
	@echo 'Create Flash image (binary format)'
	tc32-elf-objcopy -O binary SensorMesh_20170418.elf  "SensorMesh_20170418.bin"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: SensorMesh_20170418.elf
	@echo 'Invoking: Print Size'
	tc32-elf-size -t SensorMesh_20170418.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(FLASH_IMAGE)$(ELFS)$(LST)$(SIZEDUMMY) SensorMesh_20170418.elf
	-@echo ' '

secondary-outputs: $(LST) $(FLASH_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
