################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../wx/VtxBandsTabs.cpp \
../wx/VtxCloudsTabs.cpp \
../wx/VtxColorTabs.cpp \
../wx/VtxControls.cpp \
../wx/VtxCoronaTabs.cpp \
../wx/VtxCratersFunct.cpp \
../wx/VtxDefaultTabs.cpp \
../wx/VtxExprEdit.cpp \
../wx/VtxFogTabs.cpp \
../wx/VtxFractalFunct.cpp \
../wx/VtxFrame.cpp \
../wx/VtxFunctDialog.cpp \
../wx/VtxFunctMgr.cpp \
../wx/VtxGalaxyTabs.cpp \
../wx/VtxGlossTabs.cpp \
../wx/VtxImageDialog.cpp \
../wx/VtxImageMgr.cpp \
../wx/VtxImageTabs.cpp \
../wx/VtxImageWindow.cpp \
../wx/VtxImportTabs.cpp \
../wx/VtxLayerTabs.cpp \
../wx/VtxMain.cpp \
../wx/VtxMapTabs.cpp \
../wx/VtxMoonTabs.cpp \
../wx/VtxNoiseFunct.cpp \
../wx/VtxPlanetTabs.cpp \
../wx/VtxPointTabs.cpp \
../wx/VtxRingTabs.cpp \
../wx/VtxRocksTabs.cpp \
../wx/VtxScene.cpp \
../wx/VtxSceneDialog.cpp \
../wx/VtxSceneTabs.cpp \
../wx/VtxSkyTabs.cpp \
../wx/VtxSnowTabs.cpp \
../wx/VtxStarTabs.cpp \
../wx/VtxSurfaceTabs.cpp \
../wx/VtxSystemTabs.cpp \
../wx/VtxTabsMgr.cpp \
../wx/VtxTexTabs.cpp \
../wx/VtxWaterTabs.cpp 

OBJS += \
./wx/VtxBandsTabs.o \
./wx/VtxCloudsTabs.o \
./wx/VtxColorTabs.o \
./wx/VtxControls.o \
./wx/VtxCoronaTabs.o \
./wx/VtxCratersFunct.o \
./wx/VtxDefaultTabs.o \
./wx/VtxExprEdit.o \
./wx/VtxFogTabs.o \
./wx/VtxFractalFunct.o \
./wx/VtxFrame.o \
./wx/VtxFunctDialog.o \
./wx/VtxFunctMgr.o \
./wx/VtxGalaxyTabs.o \
./wx/VtxGlossTabs.o \
./wx/VtxImageDialog.o \
./wx/VtxImageMgr.o \
./wx/VtxImageTabs.o \
./wx/VtxImageWindow.o \
./wx/VtxImportTabs.o \
./wx/VtxLayerTabs.o \
./wx/VtxMain.o \
./wx/VtxMapTabs.o \
./wx/VtxMoonTabs.o \
./wx/VtxNoiseFunct.o \
./wx/VtxPlanetTabs.o \
./wx/VtxPointTabs.o \
./wx/VtxRingTabs.o \
./wx/VtxRocksTabs.o \
./wx/VtxScene.o \
./wx/VtxSceneDialog.o \
./wx/VtxSceneTabs.o \
./wx/VtxSkyTabs.o \
./wx/VtxSnowTabs.o \
./wx/VtxStarTabs.o \
./wx/VtxSurfaceTabs.o \
./wx/VtxSystemTabs.o \
./wx/VtxTabsMgr.o \
./wx/VtxTexTabs.o \
./wx/VtxWaterTabs.o 

CPP_DEPS += \
./wx/VtxBandsTabs.d \
./wx/VtxCloudsTabs.d \
./wx/VtxColorTabs.d \
./wx/VtxControls.d \
./wx/VtxCoronaTabs.d \
./wx/VtxCratersFunct.d \
./wx/VtxDefaultTabs.d \
./wx/VtxExprEdit.d \
./wx/VtxFogTabs.d \
./wx/VtxFractalFunct.d \
./wx/VtxFrame.d \
./wx/VtxFunctDialog.d \
./wx/VtxFunctMgr.d \
./wx/VtxGalaxyTabs.d \
./wx/VtxGlossTabs.d \
./wx/VtxImageDialog.d \
./wx/VtxImageMgr.d \
./wx/VtxImageTabs.d \
./wx/VtxImageWindow.d \
./wx/VtxImportTabs.d \
./wx/VtxLayerTabs.d \
./wx/VtxMain.d \
./wx/VtxMapTabs.d \
./wx/VtxMoonTabs.d \
./wx/VtxNoiseFunct.d \
./wx/VtxPlanetTabs.d \
./wx/VtxPointTabs.d \
./wx/VtxRingTabs.d \
./wx/VtxRocksTabs.d \
./wx/VtxScene.d \
./wx/VtxSceneDialog.d \
./wx/VtxSceneTabs.d \
./wx/VtxSkyTabs.d \
./wx/VtxSnowTabs.d \
./wx/VtxStarTabs.d \
./wx/VtxSurfaceTabs.d \
./wx/VtxSystemTabs.d \
./wx/VtxTabsMgr.d \
./wx/VtxTexTabs.d \
./wx/VtxWaterTabs.d 


# Each subdirectory must supply rules for building sources it contributes
wx/%.o: ../wx/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DGLEW -DWXWIN -DGLEW_STATIC -I"C:\Users\dean\workspace\vtx\includes" -I"C:\Users\dean\workspace\vtx\Resources" -O0 -g3 -w -c -fmessage-length=0 `wx-config --cxxflags` -DWXWIN -fpermissive -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


