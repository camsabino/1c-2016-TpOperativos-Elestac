################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/lib/primitivasAnSISOP.c \
../src/lib/principalesCPU.c 

OBJS += \
./src/lib/primitivasAnSISOP.o \
./src/lib/principalesCPU.o 

C_DEPS += \
./src/lib/primitivasAnSISOP.d \
./src/lib/principalesCPU.d 


# Each subdirectory must supply rules for building sources it contributes
src/lib/%.o: ../src/lib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2016-1c-Cazadores-de-cucos/utilidades" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


