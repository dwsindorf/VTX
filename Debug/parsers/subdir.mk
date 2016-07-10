################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../parsers/sx_l.cpp \
../parsers/sx_y.cpp 

OBJS += \
./parsers/sx_l.o \
./parsers/sx_y.o 

CPP_DEPS += \
./parsers/sx_l.d \
./parsers/sx_y.d 


# Each subdirectory must supply rules for building sources it contributes
parsers/%.o: ../parsers/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DGLEW -DWXWIN -DGLEW_STATIC -I"/home/dean/workspace/VTX/includes" -I"/home/dean/workspace/VTX/Resources" -I"../`wx-config --cxxflags`" -I"/home/dean/wxWidgets/release/includes" -O0 -g3 -w -c -fmessage-length=0 -fpermissive -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


