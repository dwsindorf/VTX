################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../includes/AdaptOptions.cpp \
../includes/Bitmap.cpp \
../includes/ColorClass.cpp \
../includes/Craters.cpp \
../includes/Effects.cpp \
../includes/Erode.cpp \
../includes/FileUtil.cpp \
../includes/Fractal.cpp \
../includes/FrameBuffer.cpp \
../includes/FrameClass.cpp \
../includes/GLSLMgr.cpp \
../includes/GLSLProgram.cpp \
../includes/GLSLVars.cpp \
../includes/GLSupport.cpp \
../includes/GLglue.cpp \
../includes/ImageClass.cpp \
../includes/ImageMgr.cpp \
../includes/Jpeg.cpp \
../includes/KeyIF.cpp \
../includes/Layers.cpp \
../includes/LightClass.cpp \
../includes/ListClass.cpp \
../includes/MapClass.cpp \
../includes/MapLink.cpp \
../includes/MapNode.cpp \
../includes/MatrixClass.cpp \
../includes/ModelClass.cpp \
../includes/NodeData.cpp \
../includes/NoiseClass.cpp \
../includes/NoiseFuncs.cpp \
../includes/ObjectClass.cpp \
../includes/Octree.cpp \
../includes/OrbitalClass.cpp \
../includes/Perlin.cpp \
../includes/Placements.cpp \
../includes/PointClass.cpp \
../includes/PreviewRenderer.cpp \
../includes/Raster.cpp \
../includes/Renderer.cpp \
../includes/Rocks.cpp \
../includes/SceneClass.cpp \
../includes/TerrainClass.cpp \
../includes/TerrainData.cpp \
../includes/TerrainMgr.cpp \
../includes/TerrainNode.cpp \
../includes/TextureClass.cpp \
../includes/TrackBall.cpp \
../includes/UniverseModel.cpp \
../includes/ViewClass.cpp \
../includes/cloud_data.cpp \
../includes/glew.cpp \
../includes/matrix.cpp \
../includes/star_data.cpp \
../includes/test_data.cpp 

OBJS += \
./includes/AdaptOptions.o \
./includes/Bitmap.o \
./includes/ColorClass.o \
./includes/Craters.o \
./includes/Effects.o \
./includes/Erode.o \
./includes/FileUtil.o \
./includes/Fractal.o \
./includes/FrameBuffer.o \
./includes/FrameClass.o \
./includes/GLSLMgr.o \
./includes/GLSLProgram.o \
./includes/GLSLVars.o \
./includes/GLSupport.o \
./includes/GLglue.o \
./includes/ImageClass.o \
./includes/ImageMgr.o \
./includes/Jpeg.o \
./includes/KeyIF.o \
./includes/Layers.o \
./includes/LightClass.o \
./includes/ListClass.o \
./includes/MapClass.o \
./includes/MapLink.o \
./includes/MapNode.o \
./includes/MatrixClass.o \
./includes/ModelClass.o \
./includes/NodeData.o \
./includes/NoiseClass.o \
./includes/NoiseFuncs.o \
./includes/ObjectClass.o \
./includes/Octree.o \
./includes/OrbitalClass.o \
./includes/Perlin.o \
./includes/Placements.o \
./includes/PointClass.o \
./includes/PreviewRenderer.o \
./includes/Raster.o \
./includes/Renderer.o \
./includes/Rocks.o \
./includes/SceneClass.o \
./includes/TerrainClass.o \
./includes/TerrainData.o \
./includes/TerrainMgr.o \
./includes/TerrainNode.o \
./includes/TextureClass.o \
./includes/TrackBall.o \
./includes/UniverseModel.o \
./includes/ViewClass.o \
./includes/cloud_data.o \
./includes/glew.o \
./includes/matrix.o \
./includes/star_data.o \
./includes/test_data.o 

CPP_DEPS += \
./includes/AdaptOptions.d \
./includes/Bitmap.d \
./includes/ColorClass.d \
./includes/Craters.d \
./includes/Effects.d \
./includes/Erode.d \
./includes/FileUtil.d \
./includes/Fractal.d \
./includes/FrameBuffer.d \
./includes/FrameClass.d \
./includes/GLSLMgr.d \
./includes/GLSLProgram.d \
./includes/GLSLVars.d \
./includes/GLSupport.d \
./includes/GLglue.d \
./includes/ImageClass.d \
./includes/ImageMgr.d \
./includes/Jpeg.d \
./includes/KeyIF.d \
./includes/Layers.d \
./includes/LightClass.d \
./includes/ListClass.d \
./includes/MapClass.d \
./includes/MapLink.d \
./includes/MapNode.d \
./includes/MatrixClass.d \
./includes/ModelClass.d \
./includes/NodeData.d \
./includes/NoiseClass.d \
./includes/NoiseFuncs.d \
./includes/ObjectClass.d \
./includes/Octree.d \
./includes/OrbitalClass.d \
./includes/Perlin.d \
./includes/Placements.d \
./includes/PointClass.d \
./includes/PreviewRenderer.d \
./includes/Raster.d \
./includes/Renderer.d \
./includes/Rocks.d \
./includes/SceneClass.d \
./includes/TerrainClass.d \
./includes/TerrainData.d \
./includes/TerrainMgr.d \
./includes/TerrainNode.d \
./includes/TextureClass.d \
./includes/TrackBall.d \
./includes/UniverseModel.d \
./includes/ViewClass.d \
./includes/cloud_data.d \
./includes/glew.d \
./includes/matrix.d \
./includes/star_data.d \
./includes/test_data.d 


# Each subdirectory must supply rules for building sources it contributes
includes/%.o: ../includes/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DGLEW -DWXWIN -DGLEW_STATIC -I"/home/dean/workspace/VTX/includes" -I"/home/dean/workspace/VTX/Resources" -I"../`wx-config --cxxflags`" -I"/home/dean/wxWidgets/release/includes" -O0 -g3 -w -c -fmessage-length=0  -fpermissive -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


