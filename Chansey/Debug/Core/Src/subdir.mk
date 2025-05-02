################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/bodytemp.c \
../Core/Src/communicate.c \
../Core/Src/display.c \
../Core/Src/distance.c \
../Core/Src/heartRate.c \
../Core/Src/heart_rate_reader.c \
../Core/Src/main.c \
../Core/Src/max30105.c \
../Core/Src/motors.c \
../Core/Src/rfid.c \
../Core/Src/stm32l4xx_hal_msp.c \
../Core/Src/stm32l4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l4xx.c \
../Core/Src/temp_humid.c 

OBJS += \
./Core/Src/bodytemp.o \
./Core/Src/communicate.o \
./Core/Src/display.o \
./Core/Src/distance.o \
./Core/Src/heartRate.o \
./Core/Src/heart_rate_reader.o \
./Core/Src/main.o \
./Core/Src/max30105.o \
./Core/Src/motors.o \
./Core/Src/rfid.o \
./Core/Src/stm32l4xx_hal_msp.o \
./Core/Src/stm32l4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l4xx.o \
./Core/Src/temp_humid.o 

C_DEPS += \
./Core/Src/bodytemp.d \
./Core/Src/communicate.d \
./Core/Src/display.d \
./Core/Src/distance.d \
./Core/Src/heartRate.d \
./Core/Src/heart_rate_reader.d \
./Core/Src/main.d \
./Core/Src/max30105.d \
./Core/Src/motors.d \
./Core/Src/rfid.d \
./Core/Src/stm32l4xx_hal_msp.d \
./Core/Src/stm32l4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l4xx.d \
./Core/Src/temp_humid.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4R5xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/bodytemp.cyclo ./Core/Src/bodytemp.d ./Core/Src/bodytemp.o ./Core/Src/bodytemp.su ./Core/Src/communicate.cyclo ./Core/Src/communicate.d ./Core/Src/communicate.o ./Core/Src/communicate.su ./Core/Src/display.cyclo ./Core/Src/display.d ./Core/Src/display.o ./Core/Src/display.su ./Core/Src/distance.cyclo ./Core/Src/distance.d ./Core/Src/distance.o ./Core/Src/distance.su ./Core/Src/heartRate.cyclo ./Core/Src/heartRate.d ./Core/Src/heartRate.o ./Core/Src/heartRate.su ./Core/Src/heart_rate_reader.cyclo ./Core/Src/heart_rate_reader.d ./Core/Src/heart_rate_reader.o ./Core/Src/heart_rate_reader.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/max30105.cyclo ./Core/Src/max30105.d ./Core/Src/max30105.o ./Core/Src/max30105.su ./Core/Src/motors.cyclo ./Core/Src/motors.d ./Core/Src/motors.o ./Core/Src/motors.su ./Core/Src/rfid.cyclo ./Core/Src/rfid.d ./Core/Src/rfid.o ./Core/Src/rfid.su ./Core/Src/stm32l4xx_hal_msp.cyclo ./Core/Src/stm32l4xx_hal_msp.d ./Core/Src/stm32l4xx_hal_msp.o ./Core/Src/stm32l4xx_hal_msp.su ./Core/Src/stm32l4xx_it.cyclo ./Core/Src/stm32l4xx_it.d ./Core/Src/stm32l4xx_it.o ./Core/Src/stm32l4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l4xx.cyclo ./Core/Src/system_stm32l4xx.d ./Core/Src/system_stm32l4xx.o ./Core/Src/system_stm32l4xx.su ./Core/Src/temp_humid.cyclo ./Core/Src/temp_humid.d ./Core/Src/temp_humid.o ./Core/Src/temp_humid.su

.PHONY: clean-Core-2f-Src

