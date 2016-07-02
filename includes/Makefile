#################
#
#
#################
#################
# 'make' build parameters
#################
MAKEFILE      = Makefile
THIS	      = includes
#################
# Compiler parameters
#################
# passed in by build target in $(OBJDIR)/Makefile
######################

INCLUDES      = ../includes
CC            =
CFLAGS        =
INCS          =
OBJDIR        = 
MKSTR         = 
GFILE        = glew.cpp

#################
# Program parameters
#################
#################
# RCS files
#################
HDR_FILES = AdaptOptions.h \
			Bitmap.h \
			ColorClass.h \
			Craters.h \
			Effects.h \
			Erode.h \
			FileUtil.h \
			Fractal.h \
			FrameBuffer.h \
			FrameClass.h \
			GLSLMgr.h \
			GLSLProgram.h \
			GLSLVars.h \
			GLSupport.h \
			GLglue.h \
			GLheaders.h \
			ImageClass.h \
			ImageMgr.h \
			KeyIF.h \
			Layers.h \
			LightClass.h \
			ListClass.h \
			MapClass.h \
			MapLink.h \
			MapNode.h \
			MatrixClass.h \
			ModelClass.h \
			NodeData.h \
			NodeIF.h \
			NoiseClass.h \
			NoiseFuncs.h \
			ObjectClass.h \
			Octree.h \
			OrbitalClass.h \
			Placements.h \
			PointClass.h \
			PreviewRenderer.h \
			Raster.h \
			RenderOptions.h \
			Renderer.h \
			Rocks.h \
			SceneClass.h \
			TerrainClass.h \
			TerrainData.h \
			TerrainMgr.h \
			TerrainNode.h \
			TextureClass.h \
		    TrackBall.h \
			UniverseModel.h \
			Util.h \
			ViewClass.h \
			defs.h \
			matrix.h \
			star_data.h

SRC_FILES=  AdaptOptions.cpp \
			Bitmap.cpp \
			ColorClass.cpp \
			Craters.cpp \
			Effects.cpp \
			Erode.cpp \
			FileUtil.cpp \
			Fractal.cpp \
			FrameBuffer.cpp \
			FrameClass.cpp \
			GLSLMgr.cpp \
			GLSLProgram.cpp \
			GLSLVars.cpp \
			GLSupport.cpp \
			ImageClass.cpp \
			ImageMgr.cpp \
			Jpeg.cpp \
			KeyIF.cpp \
			Layers.cpp \
			LightClass.cpp \
			ListClass.cpp \
			MapClass.cpp \
			MapLink.cpp \
			MapNode.cpp \
			MatrixClass.cpp \
			ModelClass.cpp \
			NodeData.cpp \
			NoiseClass.cpp \
			NoiseFuncs.cpp \
			ObjectClass.cpp \
			Octree.cpp \
			OrbitalClass.cpp \
			Perlin.cpp \
			Placements.cpp \
			PointClass.cpp \
			PreviewRenderer.cpp \
			Raster.cpp \
			Renderer.cpp \
			Rocks.cpp \
			SceneClass.cpp \
			TerrainClass.cpp \
			TerrainData.cpp \
			TerrainMgr.cpp \
			TerrainNode.cpp \
			TextureClass.cpp \
		    TrackBall.cpp \
			UniverseModel.cpp \
			ViewClass.cpp \
			matrix.cpp \
			star_data.cpp \
			cloud_data.cpp

#################
# Object files
#################
#			GLee.cpp \
#			glew.cpp \

COBJS = $(SRC_FILES:%.cpp=$(OBJDIR)/%.o)
GOBJS = $(GFILE:%.cpp=$(OBJDIR)/%.o)
OBJS  = $(COBJS) $(GOBJS) 
COMPILE.cc   = $(CC) $(CFLAGS) $(INCS) -c
#################
# Target Rules
#################
local:;        @echo "local build from this directory not supported"

build:		    $(OBJS)

$(OBJDIR)/%.o: %.cpp
	$(COMPILE.cc) -o $(@) $(@F:%.o=%.cpp)

clean:;			@rm -fr $(OBJS) .sb

depend:;	
				@makedepend -Y./  $(SRC_FILES) -p"../Debug/" 2>/dev/null; \
                 makedepend -a  -Y./  $(SRC_FILES) -p"../Release/" 2>/dev/null; \
                 rm -f tmp; \
                 for file in $(SRC_FILES:%.cpp=%); do \
		              echo "../Debug/$$file.o:	$$file.cpp" >> tmp; \
	             done; \
                 for file in $(SRC_FILES:%.cpp=%); do \
		              echo "../Release/$$file.o:	$$file.cpp" >> tmp; \
	             done; \
	             cat tmp >> Makefile; \
	             rm tmp;
                 
#################
# dependencies
#################

# DO NOT DELETE THIS LINE -- make depend depends on it.

../Debug/AdaptOptions.o: AdaptOptions.h defs.h ColorClass.h GLheaders.h
../Debug/Bitmap.o: Bitmap.h GLheaders.h
../Debug/ColorClass.o: defs.h ColorClass.h GLheaders.h
../Debug/Craters.o: Craters.h Placements.h TerrainMgr.h NoiseClass.h
../Debug/Craters.o: PointClass.h defs.h ListClass.h ColorClass.h GLheaders.h
../Debug/Craters.o: TextureClass.h GLSupport.h GLglue.h ImageMgr.h
../Debug/Craters.o: TerrainNode.h TerrainData.h NodeIF.h SceneClass.h
../Debug/Craters.o: LightClass.h ObjectClass.h OrbitalClass.h Renderer.h
../Debug/Craters.o: ViewClass.h Util.h MatrixClass.h matrix.h TrackBall.h
../Debug/Craters.o: ModelClass.h RenderOptions.h
../Debug/Effects.o: Effects.h Raster.h SceneClass.h LightClass.h ColorClass.h
../Debug/Effects.o: GLheaders.h PointClass.h defs.h ListClass.h ObjectClass.h
../Debug/Effects.o: GLglue.h NodeIF.h OrbitalClass.h TerrainMgr.h
../Debug/Effects.o: NoiseClass.h TextureClass.h GLSupport.h ImageMgr.h
../Debug/Effects.o: TerrainNode.h TerrainData.h Renderer.h ViewClass.h Util.h
../Debug/Effects.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Debug/Effects.o: RenderOptions.h AdaptOptions.h MapNode.h NodeData.h
../Debug/Effects.o: MapClass.h KeyIF.h GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h
../Debug/Effects.o: ./FrameBuffer.h
../Debug/Erode.o: NodeData.h GLheaders.h TerrainData.h NoiseClass.h
../Debug/Erode.o: PointClass.h defs.h ListClass.h ColorClass.h NodeIF.h
../Debug/Erode.o: TextureClass.h GLSupport.h GLglue.h ImageMgr.h MapClass.h
../Debug/Erode.o: SceneClass.h LightClass.h ObjectClass.h OrbitalClass.h
../Debug/Erode.o: TerrainMgr.h TerrainNode.h Renderer.h ViewClass.h Util.h
../Debug/Erode.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h Erode.h
../Debug/FileUtil.o: ModelClass.h ListClass.h defs.h NodeIF.h FileUtil.h
../Debug/Fractal.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Debug/Fractal.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Debug/Fractal.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Debug/Fractal.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Debug/Fractal.o: TerrainData.h Renderer.h ViewClass.h Util.h MatrixClass.h
../Debug/Fractal.o: matrix.h TrackBall.h ModelClass.h MapNode.h NodeData.h
../Debug/Fractal.o: MapClass.h Fractal.h AdaptOptions.h
../Debug/FrameBuffer.o: GLheaders.h ./FrameBuffer.h
../Debug/FrameClass.o: FrameClass.h ViewClass.h LightClass.h ColorClass.h
../Debug/FrameClass.o: GLheaders.h PointClass.h defs.h ListClass.h
../Debug/FrameClass.o: ObjectClass.h GLglue.h NodeIF.h Util.h MatrixClass.h
../Debug/FrameClass.o: matrix.h TrackBall.h
../Debug/GLSLMgr.o: ./NoiseFuncs.h GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h
../Debug/GLSLMgr.o: GLheaders.h ./FrameBuffer.h ListClass.h defs.h
../Debug/GLSLMgr.o: PointClass.h Effects.h Raster.h SceneClass.h LightClass.h
../Debug/GLSLMgr.o: ColorClass.h ObjectClass.h GLglue.h NodeIF.h
../Debug/GLSLMgr.o: OrbitalClass.h TerrainMgr.h NoiseClass.h TextureClass.h
../Debug/GLSLMgr.o: GLSupport.h ImageMgr.h TerrainNode.h TerrainData.h
../Debug/GLSLMgr.o: Renderer.h ViewClass.h Util.h MatrixClass.h matrix.h
../Debug/GLSLMgr.o: TrackBall.h ModelClass.h FileUtil.h
../Debug/GLSLProgram.o: GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h GLheaders.h
../Debug/GLSLProgram.o: ./FrameBuffer.h ListClass.h defs.h PointClass.h
../Debug/GLSLVars.o: ./GLSLVars.h GLheaders.h ./FrameBuffer.h ListClass.h
../Debug/GLSLVars.o: defs.h GLSLMgr.h ./GLSLProgram.h PointClass.h
../Debug/GLSupport.o: GLSupport.h GLglue.h GLheaders.h
../Debug/ImageClass.o: TerrainData.h NoiseClass.h PointClass.h defs.h
../Debug/ImageClass.o: ListClass.h ColorClass.h GLheaders.h NodeIF.h
../Debug/ImageClass.o: TerrainClass.h TerrainMgr.h TextureClass.h GLSupport.h
../Debug/ImageClass.o: GLglue.h ImageMgr.h TerrainNode.h ImageClass.h
../Debug/ImageClass.o: AdaptOptions.h SceneClass.h LightClass.h ObjectClass.h
../Debug/ImageClass.o: OrbitalClass.h Renderer.h ViewClass.h Util.h
../Debug/ImageClass.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Debug/ImageClass.o: RenderOptions.h GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h
../Debug/ImageClass.o: ./FrameBuffer.h
../Debug/ImageMgr.o: ImageMgr.h GLheaders.h defs.h ColorClass.h ListClass.h
../Debug/ImageMgr.o: Bitmap.h FileUtil.h Util.h PointClass.h TerrainClass.h
../Debug/ImageMgr.o: TerrainMgr.h NoiseClass.h TextureClass.h GLSupport.h
../Debug/ImageMgr.o: GLglue.h TerrainNode.h TerrainData.h NodeIF.h
../Debug/ImageMgr.o: AdaptOptions.h ModelClass.h SceneClass.h LightClass.h
../Debug/ImageMgr.o: ObjectClass.h OrbitalClass.h Renderer.h ViewClass.h
../Debug/ImageMgr.o: MatrixClass.h matrix.h TrackBall.h ImageClass.h
../Debug/Jpeg.o: GLheaders.h SceneClass.h LightClass.h ColorClass.h
../Debug/Jpeg.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Debug/Jpeg.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Debug/Jpeg.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Debug/Jpeg.o: TerrainData.h Renderer.h ViewClass.h Util.h MatrixClass.h
../Debug/Jpeg.o: matrix.h TrackBall.h ModelClass.h Bitmap.h
../Debug/KeyIF.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Debug/KeyIF.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Debug/KeyIF.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Debug/KeyIF.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Debug/KeyIF.o: TerrainData.h Renderer.h ViewClass.h Util.h MatrixClass.h
../Debug/KeyIF.o: matrix.h TrackBall.h ModelClass.h TerrainClass.h
../Debug/KeyIF.o: AdaptOptions.h RenderOptions.h Effects.h Raster.h KeyIF.h
../Debug/KeyIF.o: FileUtil.h FrameClass.h Octree.h MapClass.h
../Debug/Layers.o: TerrainMgr.h NoiseClass.h PointClass.h defs.h ListClass.h
../Debug/Layers.o: ColorClass.h GLheaders.h TextureClass.h GLSupport.h
../Debug/Layers.o: GLglue.h ImageMgr.h TerrainNode.h TerrainData.h NodeIF.h
../Debug/Layers.o: Layers.h MapNode.h NodeData.h MapClass.h SceneClass.h
../Debug/Layers.o: LightClass.h ObjectClass.h OrbitalClass.h Renderer.h
../Debug/Layers.o: ViewClass.h Util.h MatrixClass.h matrix.h TrackBall.h
../Debug/Layers.o: ModelClass.h AdaptOptions.h FileUtil.h
../Debug/LightClass.o: LightClass.h ColorClass.h GLheaders.h PointClass.h
../Debug/LightClass.o: defs.h ListClass.h ObjectClass.h GLglue.h NodeIF.h
../Debug/LightClass.o: NoiseClass.h
../Debug/ListClass.o: ListClass.h defs.h
../Debug/MapClass.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Debug/MapClass.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Debug/MapClass.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Debug/MapClass.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Debug/MapClass.o: TerrainData.h Renderer.h ViewClass.h Util.h
../Debug/MapClass.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Debug/MapClass.o: MapClass.h MapLink.h MapNode.h NodeData.h AdaptOptions.h
../Debug/MapClass.o: RenderOptions.h Effects.h Raster.h GLSLMgr.h
../Debug/MapClass.o: ./GLSLProgram.h ./GLSLVars.h ./FrameBuffer.h FileUtil.h
../Debug/MapLink.o: MapLink.h MapNode.h NodeData.h GLheaders.h TerrainData.h
../Debug/MapLink.o: NoiseClass.h PointClass.h defs.h ListClass.h ColorClass.h
../Debug/MapLink.o: NodeIF.h TextureClass.h GLSupport.h GLglue.h ImageMgr.h
../Debug/MapLink.o: MapClass.h SceneClass.h LightClass.h ObjectClass.h
../Debug/MapLink.o: OrbitalClass.h TerrainMgr.h TerrainNode.h Renderer.h
../Debug/MapLink.o: ViewClass.h Util.h MatrixClass.h matrix.h TrackBall.h
../Debug/MapLink.o: ModelClass.h
../Debug/MapNode.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Debug/MapNode.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Debug/MapNode.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Debug/MapNode.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Debug/MapNode.o: TerrainData.h Renderer.h ViewClass.h Util.h MatrixClass.h
../Debug/MapNode.o: matrix.h TrackBall.h ModelClass.h MapNode.h NodeData.h
../Debug/MapNode.o: MapClass.h MapLink.h AdaptOptions.h RenderOptions.h
../Debug/MapNode.o: GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h ./FrameBuffer.h
../Debug/MapNode.o: Effects.h Raster.h
../Debug/MatrixClass.o: defs.h MatrixClass.h matrix.h PointClass.h
../Debug/ModelClass.o: ModelClass.h ListClass.h defs.h NodeIF.h ObjectClass.h
../Debug/ModelClass.o: GLglue.h GLheaders.h PointClass.h ColorClass.h
../Debug/ModelClass.o: FileUtil.h
../Debug/NodeData.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Debug/NodeData.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Debug/NodeData.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Debug/NodeData.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Debug/NodeData.o: TerrainData.h Renderer.h ViewClass.h Util.h
../Debug/NodeData.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Debug/NodeData.o: MapNode.h NodeData.h MapClass.h MapLink.h AdaptOptions.h
../Debug/NodeData.o: RenderOptions.h Effects.h Raster.h
../Debug/NoiseClass.o: NoiseClass.h PointClass.h defs.h ListClass.h
../Debug/NoiseClass.o: ColorClass.h GLheaders.h Perlin.h Util.h
../Debug/NoiseFuncs.o: ./NoiseFuncs.h Perlin.h
../Debug/ObjectClass.o: ViewClass.h LightClass.h ColorClass.h GLheaders.h
../Debug/ObjectClass.o: PointClass.h defs.h ListClass.h ObjectClass.h
../Debug/ObjectClass.o: GLglue.h NodeIF.h Util.h MatrixClass.h matrix.h
../Debug/ObjectClass.o: TrackBall.h
../Debug/Octree.o: Octree.h defs.h ObjectClass.h GLglue.h GLheaders.h
../Debug/Octree.o: PointClass.h ColorClass.h ListClass.h NodeIF.h
../Debug/Octree.o: NoiseClass.h Util.h RenderOptions.h SceneClass.h
../Debug/Octree.o: LightClass.h OrbitalClass.h TerrainMgr.h TextureClass.h
../Debug/Octree.o: GLSupport.h ImageMgr.h TerrainNode.h TerrainData.h
../Debug/Octree.o: Renderer.h ViewClass.h MatrixClass.h matrix.h TrackBall.h
../Debug/Octree.o: ModelClass.h ImageClass.h TerrainClass.h AdaptOptions.h
../Debug/OrbitalClass.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Debug/OrbitalClass.o: PointClass.h defs.h ListClass.h ObjectClass.h
../Debug/OrbitalClass.o: GLglue.h NodeIF.h OrbitalClass.h TerrainMgr.h
../Debug/OrbitalClass.o: NoiseClass.h TextureClass.h GLSupport.h ImageMgr.h
../Debug/OrbitalClass.o: TerrainNode.h TerrainData.h Renderer.h ViewClass.h
../Debug/OrbitalClass.o: Util.h MatrixClass.h matrix.h TrackBall.h
../Debug/OrbitalClass.o: ModelClass.h RenderOptions.h Effects.h Raster.h
../Debug/OrbitalClass.o: MapClass.h MapNode.h NodeData.h Octree.h
../Debug/OrbitalClass.o: TerrainClass.h ImageClass.h GLSLMgr.h
../Debug/OrbitalClass.o: ./GLSLProgram.h ./GLSLVars.h ./FrameBuffer.h
../Debug/OrbitalClass.o: AdaptOptions.h FileUtil.h star_data.h
../Debug/Placements.o: TerrainMgr.h NoiseClass.h PointClass.h defs.h
../Debug/Placements.o: ListClass.h ColorClass.h GLheaders.h TextureClass.h
../Debug/Placements.o: GLSupport.h GLglue.h ImageMgr.h TerrainNode.h
../Debug/Placements.o: TerrainData.h NodeIF.h Placements.h AdaptOptions.h
../Debug/Placements.o: Util.h SceneClass.h LightClass.h ObjectClass.h
../Debug/Placements.o: OrbitalClass.h Renderer.h ViewClass.h MatrixClass.h
../Debug/Placements.o: matrix.h TrackBall.h ModelClass.h RenderOptions.h
../Debug/PointClass.o: PointClass.h defs.h
../Debug/PreviewRenderer.o: PreviewRenderer.h LightClass.h ColorClass.h
../Debug/PreviewRenderer.o: GLheaders.h PointClass.h defs.h ListClass.h
../Debug/PreviewRenderer.o: ObjectClass.h GLglue.h NodeIF.h OrbitalClass.h
../Debug/PreviewRenderer.o: TerrainMgr.h NoiseClass.h TextureClass.h
../Debug/PreviewRenderer.o: GLSupport.h ImageMgr.h TerrainNode.h
../Debug/PreviewRenderer.o: TerrainData.h Renderer.h ViewClass.h Util.h
../Debug/PreviewRenderer.o: MatrixClass.h matrix.h TrackBall.h MapClass.h
../Debug/PreviewRenderer.o: SceneClass.h ModelClass.h TerrainClass.h
../Debug/PreviewRenderer.o: MapNode.h NodeData.h UniverseModel.h
../Debug/PreviewRenderer.o: AdaptOptions.h Effects.h Raster.h
../Debug/Raster.o: Effects.h Raster.h SceneClass.h LightClass.h ColorClass.h
../Debug/Raster.o: GLheaders.h PointClass.h defs.h ListClass.h ObjectClass.h
../Debug/Raster.o: GLglue.h NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Debug/Raster.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Debug/Raster.o: TerrainData.h Renderer.h ViewClass.h Util.h MatrixClass.h
../Debug/Raster.o: matrix.h TrackBall.h ModelClass.h RenderOptions.h
../Debug/Raster.o: AdaptOptions.h MapNode.h NodeData.h MapClass.h KeyIF.h
../Debug/Renderer.o: Renderer.h ObjectClass.h GLglue.h GLheaders.h
../Debug/Renderer.o: PointClass.h defs.h ColorClass.h ListClass.h NodeIF.h
../Debug/Renderer.o: ViewClass.h LightClass.h Util.h MatrixClass.h matrix.h
../Debug/Renderer.o: TrackBall.h GLSupport.h AdaptOptions.h RenderOptions.h
../Debug/Renderer.o: GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h ./FrameBuffer.h
../Debug/Rocks.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Debug/Rocks.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Debug/Rocks.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Debug/Rocks.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Debug/Rocks.o: TerrainData.h Renderer.h ViewClass.h Util.h MatrixClass.h
../Debug/Rocks.o: matrix.h TrackBall.h ModelClass.h RenderOptions.h Rocks.h
../Debug/Rocks.o: Placements.h MapNode.h NodeData.h MapClass.h AdaptOptions.h
../Debug/SceneClass.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Debug/SceneClass.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Debug/SceneClass.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Debug/SceneClass.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Debug/SceneClass.o: TerrainData.h Renderer.h ViewClass.h Util.h
../Debug/SceneClass.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Debug/SceneClass.o: AdaptOptions.h RenderOptions.h Effects.h Raster.h
../Debug/SceneClass.o: Octree.h FileUtil.h FrameClass.h TerrainClass.h
../Debug/SceneClass.o: KeyIF.h GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h
../Debug/SceneClass.o: ./FrameBuffer.h
../Debug/TerrainClass.o: TerrainData.h NoiseClass.h PointClass.h defs.h
../Debug/TerrainClass.o: ListClass.h ColorClass.h GLheaders.h NodeIF.h
../Debug/TerrainClass.o: TerrainClass.h TerrainMgr.h TextureClass.h
../Debug/TerrainClass.o: GLSupport.h GLglue.h ImageMgr.h TerrainNode.h
../Debug/TerrainClass.o: MapNode.h NodeData.h MapClass.h SceneClass.h
../Debug/TerrainClass.o: LightClass.h ObjectClass.h OrbitalClass.h Renderer.h
../Debug/TerrainClass.o: ViewClass.h Util.h MatrixClass.h matrix.h
../Debug/TerrainClass.o: TrackBall.h ModelClass.h AdaptOptions.h Effects.h
../Debug/TerrainClass.o: Raster.h GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h
../Debug/TerrainClass.o: ./FrameBuffer.h RenderOptions.h
../Debug/TerrainData.o: TerrainData.h NoiseClass.h PointClass.h defs.h
../Debug/TerrainData.o: ListClass.h ColorClass.h GLheaders.h NodeIF.h
../Debug/TerrainData.o: NodeData.h TextureClass.h GLSupport.h GLglue.h
../Debug/TerrainData.o: ImageMgr.h MapClass.h SceneClass.h LightClass.h
../Debug/TerrainData.o: ObjectClass.h OrbitalClass.h TerrainMgr.h
../Debug/TerrainData.o: TerrainNode.h Renderer.h ViewClass.h Util.h
../Debug/TerrainData.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Debug/TerrainData.o: ImageClass.h TerrainClass.h GLSLMgr.h ./GLSLProgram.h
../Debug/TerrainData.o: ./GLSLVars.h ./FrameBuffer.h RenderOptions.h
../Debug/TerrainMgr.o: TerrainData.h NoiseClass.h PointClass.h defs.h
../Debug/TerrainMgr.o: ListClass.h ColorClass.h GLheaders.h NodeIF.h
../Debug/TerrainMgr.o: TerrainMgr.h TextureClass.h GLSupport.h GLglue.h
../Debug/TerrainMgr.o: ImageMgr.h TerrainNode.h ImageClass.h SceneClass.h
../Debug/TerrainMgr.o: LightClass.h ObjectClass.h OrbitalClass.h Renderer.h
../Debug/TerrainMgr.o: ViewClass.h Util.h MatrixClass.h matrix.h TrackBall.h
../Debug/TerrainMgr.o: ModelClass.h
../Debug/TerrainNode.o: TerrainMgr.h NoiseClass.h PointClass.h defs.h
../Debug/TerrainNode.o: ListClass.h ColorClass.h GLheaders.h TextureClass.h
../Debug/TerrainNode.o: GLSupport.h GLglue.h ImageMgr.h TerrainNode.h
../Debug/TerrainNode.o: TerrainData.h NodeIF.h SceneClass.h LightClass.h
../Debug/TerrainNode.o: ObjectClass.h OrbitalClass.h Renderer.h ViewClass.h
../Debug/TerrainNode.o: Util.h MatrixClass.h matrix.h TrackBall.h
../Debug/TerrainNode.o: ModelClass.h
../Debug/TextureClass.o: TextureClass.h GLSupport.h GLglue.h GLheaders.h
../Debug/TextureClass.o: ImageMgr.h defs.h ColorClass.h ListClass.h
../Debug/TextureClass.o: PointClass.h NoiseClass.h TerrainNode.h
../Debug/TextureClass.o: TerrainData.h NodeIF.h TerrainClass.h TerrainMgr.h
../Debug/TextureClass.o: GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h
../Debug/TextureClass.o: ./FrameBuffer.h Effects.h Raster.h SceneClass.h
../Debug/TextureClass.o: LightClass.h ObjectClass.h OrbitalClass.h Renderer.h
../Debug/TextureClass.o: ViewClass.h Util.h MatrixClass.h matrix.h
../Debug/TextureClass.o: TrackBall.h ModelClass.h RenderOptions.h
../Debug/TrackBall.o: TrackBall.h
../Debug/UniverseModel.o: UniverseModel.h ModelClass.h ListClass.h defs.h
../Debug/UniverseModel.o: NodeIF.h OrbitalClass.h LightClass.h ColorClass.h
../Debug/UniverseModel.o: GLheaders.h PointClass.h ObjectClass.h GLglue.h
../Debug/UniverseModel.o: TerrainMgr.h NoiseClass.h TextureClass.h
../Debug/UniverseModel.o: GLSupport.h ImageMgr.h TerrainNode.h TerrainData.h
../Debug/UniverseModel.o: TerrainClass.h ImageClass.h FileUtil.h
../Debug/ViewClass.o: ViewClass.h LightClass.h ColorClass.h GLheaders.h
../Debug/ViewClass.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Debug/ViewClass.o: NodeIF.h Util.h MatrixClass.h matrix.h TrackBall.h
../Debug/ViewClass.o: RenderOptions.h NoiseClass.h
../Debug/matrix.o: defs.h

../Release/AdaptOptions.o: AdaptOptions.h defs.h ColorClass.h GLheaders.h
../Release/Bitmap.o: Bitmap.h GLheaders.h
../Release/ColorClass.o: defs.h ColorClass.h GLheaders.h
../Release/Craters.o: Craters.h Placements.h TerrainMgr.h NoiseClass.h
../Release/Craters.o: PointClass.h defs.h ListClass.h ColorClass.h
../Release/Craters.o: GLheaders.h TextureClass.h GLSupport.h GLglue.h
../Release/Craters.o: ImageMgr.h TerrainNode.h TerrainData.h NodeIF.h
../Release/Craters.o: SceneClass.h LightClass.h ObjectClass.h OrbitalClass.h
../Release/Craters.o: Renderer.h ViewClass.h Util.h MatrixClass.h matrix.h
../Release/Craters.o: TrackBall.h ModelClass.h RenderOptions.h
../Release/Effects.o: Effects.h Raster.h SceneClass.h LightClass.h
../Release/Effects.o: ColorClass.h GLheaders.h PointClass.h defs.h
../Release/Effects.o: ListClass.h ObjectClass.h GLglue.h NodeIF.h
../Release/Effects.o: OrbitalClass.h TerrainMgr.h NoiseClass.h TextureClass.h
../Release/Effects.o: GLSupport.h ImageMgr.h TerrainNode.h TerrainData.h
../Release/Effects.o: Renderer.h ViewClass.h Util.h MatrixClass.h matrix.h
../Release/Effects.o: TrackBall.h ModelClass.h RenderOptions.h AdaptOptions.h
../Release/Effects.o: MapNode.h NodeData.h MapClass.h KeyIF.h GLSLMgr.h
../Release/Effects.o: ./GLSLProgram.h ./GLSLVars.h ./FrameBuffer.h
../Release/Erode.o: NodeData.h GLheaders.h TerrainData.h NoiseClass.h
../Release/Erode.o: PointClass.h defs.h ListClass.h ColorClass.h NodeIF.h
../Release/Erode.o: TextureClass.h GLSupport.h GLglue.h ImageMgr.h MapClass.h
../Release/Erode.o: SceneClass.h LightClass.h ObjectClass.h OrbitalClass.h
../Release/Erode.o: TerrainMgr.h TerrainNode.h Renderer.h ViewClass.h Util.h
../Release/Erode.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h Erode.h
../Release/FileUtil.o: ModelClass.h ListClass.h defs.h NodeIF.h FileUtil.h
../Release/Fractal.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Release/Fractal.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Release/Fractal.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Release/Fractal.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Release/Fractal.o: TerrainData.h Renderer.h ViewClass.h Util.h
../Release/Fractal.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Release/Fractal.o: MapNode.h NodeData.h MapClass.h Fractal.h
../Release/Fractal.o: AdaptOptions.h
../Release/FrameBuffer.o: GLheaders.h ./FrameBuffer.h
../Release/FrameClass.o: FrameClass.h ViewClass.h LightClass.h ColorClass.h
../Release/FrameClass.o: GLheaders.h PointClass.h defs.h ListClass.h
../Release/FrameClass.o: ObjectClass.h GLglue.h NodeIF.h Util.h MatrixClass.h
../Release/FrameClass.o: matrix.h TrackBall.h
../Release/GLSLMgr.o: ./NoiseFuncs.h GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h
../Release/GLSLMgr.o: GLheaders.h ./FrameBuffer.h ListClass.h defs.h
../Release/GLSLMgr.o: PointClass.h Effects.h Raster.h SceneClass.h
../Release/GLSLMgr.o: LightClass.h ColorClass.h ObjectClass.h GLglue.h
../Release/GLSLMgr.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Release/GLSLMgr.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Release/GLSLMgr.o: TerrainData.h Renderer.h ViewClass.h Util.h
../Release/GLSLMgr.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Release/GLSLMgr.o: FileUtil.h
../Release/GLSLProgram.o: GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h GLheaders.h
../Release/GLSLProgram.o: ./FrameBuffer.h ListClass.h defs.h PointClass.h
../Release/GLSLVars.o: ./GLSLVars.h GLheaders.h ./FrameBuffer.h ListClass.h
../Release/GLSLVars.o: defs.h GLSLMgr.h ./GLSLProgram.h PointClass.h
../Release/GLSupport.o: GLSupport.h GLglue.h GLheaders.h
../Release/ImageClass.o: TerrainData.h NoiseClass.h PointClass.h defs.h
../Release/ImageClass.o: ListClass.h ColorClass.h GLheaders.h NodeIF.h
../Release/ImageClass.o: TerrainClass.h TerrainMgr.h TextureClass.h
../Release/ImageClass.o: GLSupport.h GLglue.h ImageMgr.h TerrainNode.h
../Release/ImageClass.o: ImageClass.h AdaptOptions.h SceneClass.h
../Release/ImageClass.o: LightClass.h ObjectClass.h OrbitalClass.h Renderer.h
../Release/ImageClass.o: ViewClass.h Util.h MatrixClass.h matrix.h
../Release/ImageClass.o: TrackBall.h ModelClass.h RenderOptions.h GLSLMgr.h
../Release/ImageClass.o: ./GLSLProgram.h ./GLSLVars.h ./FrameBuffer.h
../Release/ImageMgr.o: ImageMgr.h GLheaders.h defs.h ColorClass.h ListClass.h
../Release/ImageMgr.o: Bitmap.h FileUtil.h Util.h PointClass.h TerrainClass.h
../Release/ImageMgr.o: TerrainMgr.h NoiseClass.h TextureClass.h GLSupport.h
../Release/ImageMgr.o: GLglue.h TerrainNode.h TerrainData.h NodeIF.h
../Release/ImageMgr.o: AdaptOptions.h ModelClass.h SceneClass.h LightClass.h
../Release/ImageMgr.o: ObjectClass.h OrbitalClass.h Renderer.h ViewClass.h
../Release/ImageMgr.o: MatrixClass.h matrix.h TrackBall.h ImageClass.h
../Release/Jpeg.o: GLheaders.h SceneClass.h LightClass.h ColorClass.h
../Release/Jpeg.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Release/Jpeg.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Release/Jpeg.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Release/Jpeg.o: TerrainData.h Renderer.h ViewClass.h Util.h MatrixClass.h
../Release/Jpeg.o: matrix.h TrackBall.h ModelClass.h Bitmap.h
../Release/KeyIF.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Release/KeyIF.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Release/KeyIF.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Release/KeyIF.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Release/KeyIF.o: TerrainData.h Renderer.h ViewClass.h Util.h MatrixClass.h
../Release/KeyIF.o: matrix.h TrackBall.h ModelClass.h TerrainClass.h
../Release/KeyIF.o: AdaptOptions.h RenderOptions.h Effects.h Raster.h KeyIF.h
../Release/KeyIF.o: FileUtil.h FrameClass.h Octree.h MapClass.h
../Release/Layers.o: TerrainMgr.h NoiseClass.h PointClass.h defs.h
../Release/Layers.o: ListClass.h ColorClass.h GLheaders.h TextureClass.h
../Release/Layers.o: GLSupport.h GLglue.h ImageMgr.h TerrainNode.h
../Release/Layers.o: TerrainData.h NodeIF.h Layers.h MapNode.h NodeData.h
../Release/Layers.o: MapClass.h SceneClass.h LightClass.h ObjectClass.h
../Release/Layers.o: OrbitalClass.h Renderer.h ViewClass.h Util.h
../Release/Layers.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Release/Layers.o: AdaptOptions.h FileUtil.h
../Release/LightClass.o: LightClass.h ColorClass.h GLheaders.h PointClass.h
../Release/LightClass.o: defs.h ListClass.h ObjectClass.h GLglue.h NodeIF.h
../Release/LightClass.o: NoiseClass.h
../Release/ListClass.o: ListClass.h defs.h
../Release/MapClass.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Release/MapClass.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Release/MapClass.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Release/MapClass.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Release/MapClass.o: TerrainData.h Renderer.h ViewClass.h Util.h
../Release/MapClass.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Release/MapClass.o: MapClass.h MapLink.h MapNode.h NodeData.h
../Release/MapClass.o: AdaptOptions.h RenderOptions.h Effects.h Raster.h
../Release/MapClass.o: GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h ./FrameBuffer.h
../Release/MapClass.o: FileUtil.h
../Release/MapLink.o: MapLink.h MapNode.h NodeData.h GLheaders.h
../Release/MapLink.o: TerrainData.h NoiseClass.h PointClass.h defs.h
../Release/MapLink.o: ListClass.h ColorClass.h NodeIF.h TextureClass.h
../Release/MapLink.o: GLSupport.h GLglue.h ImageMgr.h MapClass.h SceneClass.h
../Release/MapLink.o: LightClass.h ObjectClass.h OrbitalClass.h TerrainMgr.h
../Release/MapLink.o: TerrainNode.h Renderer.h ViewClass.h Util.h
../Release/MapLink.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Release/MapNode.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Release/MapNode.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Release/MapNode.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Release/MapNode.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Release/MapNode.o: TerrainData.h Renderer.h ViewClass.h Util.h
../Release/MapNode.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Release/MapNode.o: MapNode.h NodeData.h MapClass.h MapLink.h
../Release/MapNode.o: AdaptOptions.h RenderOptions.h GLSLMgr.h
../Release/MapNode.o: ./GLSLProgram.h ./GLSLVars.h ./FrameBuffer.h Effects.h
../Release/MapNode.o: Raster.h
../Release/MatrixClass.o: defs.h MatrixClass.h matrix.h PointClass.h
../Release/ModelClass.o: ModelClass.h ListClass.h defs.h NodeIF.h
../Release/ModelClass.o: ObjectClass.h GLglue.h GLheaders.h PointClass.h
../Release/ModelClass.o: ColorClass.h FileUtil.h
../Release/NodeData.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Release/NodeData.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Release/NodeData.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Release/NodeData.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Release/NodeData.o: TerrainData.h Renderer.h ViewClass.h Util.h
../Release/NodeData.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Release/NodeData.o: MapNode.h NodeData.h MapClass.h MapLink.h
../Release/NodeData.o: AdaptOptions.h RenderOptions.h Effects.h Raster.h
../Release/NoiseClass.o: NoiseClass.h PointClass.h defs.h ListClass.h
../Release/NoiseClass.o: ColorClass.h GLheaders.h Perlin.h Util.h
../Release/NoiseFuncs.o: ./NoiseFuncs.h Perlin.h
../Release/ObjectClass.o: ViewClass.h LightClass.h ColorClass.h GLheaders.h
../Release/ObjectClass.o: PointClass.h defs.h ListClass.h ObjectClass.h
../Release/ObjectClass.o: GLglue.h NodeIF.h Util.h MatrixClass.h matrix.h
../Release/ObjectClass.o: TrackBall.h
../Release/Octree.o: Octree.h defs.h ObjectClass.h GLglue.h GLheaders.h
../Release/Octree.o: PointClass.h ColorClass.h ListClass.h NodeIF.h
../Release/Octree.o: NoiseClass.h Util.h RenderOptions.h SceneClass.h
../Release/Octree.o: LightClass.h OrbitalClass.h TerrainMgr.h TextureClass.h
../Release/Octree.o: GLSupport.h ImageMgr.h TerrainNode.h TerrainData.h
../Release/Octree.o: Renderer.h ViewClass.h MatrixClass.h matrix.h
../Release/Octree.o: TrackBall.h ModelClass.h ImageClass.h TerrainClass.h
../Release/Octree.o: AdaptOptions.h
../Release/OrbitalClass.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Release/OrbitalClass.o: PointClass.h defs.h ListClass.h ObjectClass.h
../Release/OrbitalClass.o: GLglue.h NodeIF.h OrbitalClass.h TerrainMgr.h
../Release/OrbitalClass.o: NoiseClass.h TextureClass.h GLSupport.h ImageMgr.h
../Release/OrbitalClass.o: TerrainNode.h TerrainData.h Renderer.h ViewClass.h
../Release/OrbitalClass.o: Util.h MatrixClass.h matrix.h TrackBall.h
../Release/OrbitalClass.o: ModelClass.h RenderOptions.h Effects.h Raster.h
../Release/OrbitalClass.o: MapClass.h MapNode.h NodeData.h Octree.h
../Release/OrbitalClass.o: TerrainClass.h ImageClass.h GLSLMgr.h
../Release/OrbitalClass.o: ./GLSLProgram.h ./GLSLVars.h ./FrameBuffer.h
../Release/OrbitalClass.o: AdaptOptions.h FileUtil.h star_data.h
../Release/Placements.o: TerrainMgr.h NoiseClass.h PointClass.h defs.h
../Release/Placements.o: ListClass.h ColorClass.h GLheaders.h TextureClass.h
../Release/Placements.o: GLSupport.h GLglue.h ImageMgr.h TerrainNode.h
../Release/Placements.o: TerrainData.h NodeIF.h Placements.h AdaptOptions.h
../Release/Placements.o: Util.h SceneClass.h LightClass.h ObjectClass.h
../Release/Placements.o: OrbitalClass.h Renderer.h ViewClass.h MatrixClass.h
../Release/Placements.o: matrix.h TrackBall.h ModelClass.h RenderOptions.h
../Release/PointClass.o: PointClass.h defs.h
../Release/PreviewRenderer.o: PreviewRenderer.h LightClass.h ColorClass.h
../Release/PreviewRenderer.o: GLheaders.h PointClass.h defs.h ListClass.h
../Release/PreviewRenderer.o: ObjectClass.h GLglue.h NodeIF.h OrbitalClass.h
../Release/PreviewRenderer.o: TerrainMgr.h NoiseClass.h TextureClass.h
../Release/PreviewRenderer.o: GLSupport.h ImageMgr.h TerrainNode.h
../Release/PreviewRenderer.o: TerrainData.h Renderer.h ViewClass.h Util.h
../Release/PreviewRenderer.o: MatrixClass.h matrix.h TrackBall.h MapClass.h
../Release/PreviewRenderer.o: SceneClass.h ModelClass.h TerrainClass.h
../Release/PreviewRenderer.o: MapNode.h NodeData.h UniverseModel.h
../Release/PreviewRenderer.o: AdaptOptions.h Effects.h Raster.h
../Release/Raster.o: Effects.h Raster.h SceneClass.h LightClass.h
../Release/Raster.o: ColorClass.h GLheaders.h PointClass.h defs.h ListClass.h
../Release/Raster.o: ObjectClass.h GLglue.h NodeIF.h OrbitalClass.h
../Release/Raster.o: TerrainMgr.h NoiseClass.h TextureClass.h GLSupport.h
../Release/Raster.o: ImageMgr.h TerrainNode.h TerrainData.h Renderer.h
../Release/Raster.o: ViewClass.h Util.h MatrixClass.h matrix.h TrackBall.h
../Release/Raster.o: ModelClass.h RenderOptions.h AdaptOptions.h MapNode.h
../Release/Raster.o: NodeData.h MapClass.h KeyIF.h
../Release/Renderer.o: Renderer.h ObjectClass.h GLglue.h GLheaders.h
../Release/Renderer.o: PointClass.h defs.h ColorClass.h ListClass.h NodeIF.h
../Release/Renderer.o: ViewClass.h LightClass.h Util.h MatrixClass.h matrix.h
../Release/Renderer.o: TrackBall.h GLSupport.h AdaptOptions.h RenderOptions.h
../Release/Renderer.o: GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h ./FrameBuffer.h
../Release/Rocks.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Release/Rocks.o: PointClass.h defs.h ListClass.h ObjectClass.h GLglue.h
../Release/Rocks.o: NodeIF.h OrbitalClass.h TerrainMgr.h NoiseClass.h
../Release/Rocks.o: TextureClass.h GLSupport.h ImageMgr.h TerrainNode.h
../Release/Rocks.o: TerrainData.h Renderer.h ViewClass.h Util.h MatrixClass.h
../Release/Rocks.o: matrix.h TrackBall.h ModelClass.h RenderOptions.h Rocks.h
../Release/Rocks.o: Placements.h MapNode.h NodeData.h MapClass.h
../Release/Rocks.o: AdaptOptions.h
../Release/SceneClass.o: SceneClass.h LightClass.h ColorClass.h GLheaders.h
../Release/SceneClass.o: PointClass.h defs.h ListClass.h ObjectClass.h
../Release/SceneClass.o: GLglue.h NodeIF.h OrbitalClass.h TerrainMgr.h
../Release/SceneClass.o: NoiseClass.h TextureClass.h GLSupport.h ImageMgr.h
../Release/SceneClass.o: TerrainNode.h TerrainData.h Renderer.h ViewClass.h
../Release/SceneClass.o: Util.h MatrixClass.h matrix.h TrackBall.h
../Release/SceneClass.o: ModelClass.h AdaptOptions.h RenderOptions.h
../Release/SceneClass.o: Effects.h Raster.h Octree.h FileUtil.h FrameClass.h
../Release/SceneClass.o: TerrainClass.h KeyIF.h GLSLMgr.h ./GLSLProgram.h
../Release/SceneClass.o: ./GLSLVars.h ./FrameBuffer.h
../Release/TerrainClass.o: TerrainData.h NoiseClass.h PointClass.h defs.h
../Release/TerrainClass.o: ListClass.h ColorClass.h GLheaders.h NodeIF.h
../Release/TerrainClass.o: TerrainClass.h TerrainMgr.h TextureClass.h
../Release/TerrainClass.o: GLSupport.h GLglue.h ImageMgr.h TerrainNode.h
../Release/TerrainClass.o: MapNode.h NodeData.h MapClass.h SceneClass.h
../Release/TerrainClass.o: LightClass.h ObjectClass.h OrbitalClass.h
../Release/TerrainClass.o: Renderer.h ViewClass.h Util.h MatrixClass.h
../Release/TerrainClass.o: matrix.h TrackBall.h ModelClass.h AdaptOptions.h
../Release/TerrainClass.o: Effects.h Raster.h GLSLMgr.h ./GLSLProgram.h
../Release/TerrainClass.o: ./GLSLVars.h ./FrameBuffer.h RenderOptions.h
../Release/TerrainData.o: TerrainData.h NoiseClass.h PointClass.h defs.h
../Release/TerrainData.o: ListClass.h ColorClass.h GLheaders.h NodeIF.h
../Release/TerrainData.o: NodeData.h TextureClass.h GLSupport.h GLglue.h
../Release/TerrainData.o: ImageMgr.h MapClass.h SceneClass.h LightClass.h
../Release/TerrainData.o: ObjectClass.h OrbitalClass.h TerrainMgr.h
../Release/TerrainData.o: TerrainNode.h Renderer.h ViewClass.h Util.h
../Release/TerrainData.o: MatrixClass.h matrix.h TrackBall.h ModelClass.h
../Release/TerrainData.o: ImageClass.h TerrainClass.h GLSLMgr.h
../Release/TerrainData.o: ./GLSLProgram.h ./GLSLVars.h ./FrameBuffer.h
../Release/TerrainData.o: RenderOptions.h
../Release/TerrainMgr.o: TerrainData.h NoiseClass.h PointClass.h defs.h
../Release/TerrainMgr.o: ListClass.h ColorClass.h GLheaders.h NodeIF.h
../Release/TerrainMgr.o: TerrainMgr.h TextureClass.h GLSupport.h GLglue.h
../Release/TerrainMgr.o: ImageMgr.h TerrainNode.h ImageClass.h SceneClass.h
../Release/TerrainMgr.o: LightClass.h ObjectClass.h OrbitalClass.h Renderer.h
../Release/TerrainMgr.o: ViewClass.h Util.h MatrixClass.h matrix.h
../Release/TerrainMgr.o: TrackBall.h ModelClass.h
../Release/TerrainNode.o: TerrainMgr.h NoiseClass.h PointClass.h defs.h
../Release/TerrainNode.o: ListClass.h ColorClass.h GLheaders.h TextureClass.h
../Release/TerrainNode.o: GLSupport.h GLglue.h ImageMgr.h TerrainNode.h
../Release/TerrainNode.o: TerrainData.h NodeIF.h SceneClass.h LightClass.h
../Release/TerrainNode.o: ObjectClass.h OrbitalClass.h Renderer.h ViewClass.h
../Release/TerrainNode.o: Util.h MatrixClass.h matrix.h TrackBall.h
../Release/TerrainNode.o: ModelClass.h
../Release/TextureClass.o: TextureClass.h GLSupport.h GLglue.h GLheaders.h
../Release/TextureClass.o: ImageMgr.h defs.h ColorClass.h ListClass.h
../Release/TextureClass.o: PointClass.h NoiseClass.h TerrainNode.h
../Release/TextureClass.o: TerrainData.h NodeIF.h TerrainClass.h TerrainMgr.h
../Release/TextureClass.o: GLSLMgr.h ./GLSLProgram.h ./GLSLVars.h
../Release/TextureClass.o: ./FrameBuffer.h Effects.h Raster.h SceneClass.h
../Release/TextureClass.o: LightClass.h ObjectClass.h OrbitalClass.h
../Release/TextureClass.o: Renderer.h ViewClass.h Util.h MatrixClass.h
../Release/TextureClass.o: matrix.h TrackBall.h ModelClass.h RenderOptions.h
../Release/TrackBall.o: TrackBall.h
../Release/UniverseModel.o: UniverseModel.h ModelClass.h ListClass.h defs.h
../Release/UniverseModel.o: NodeIF.h OrbitalClass.h LightClass.h ColorClass.h
../Release/UniverseModel.o: GLheaders.h PointClass.h ObjectClass.h GLglue.h
../Release/UniverseModel.o: TerrainMgr.h NoiseClass.h TextureClass.h
../Release/UniverseModel.o: GLSupport.h ImageMgr.h TerrainNode.h
../Release/UniverseModel.o: TerrainData.h TerrainClass.h ImageClass.h
../Release/UniverseModel.o: FileUtil.h
../Release/ViewClass.o: ViewClass.h LightClass.h ColorClass.h GLheaders.h
../Release/ViewClass.o: PointClass.h defs.h ListClass.h ObjectClass.h
../Release/ViewClass.o: GLglue.h NodeIF.h Util.h MatrixClass.h matrix.h
../Release/ViewClass.o: TrackBall.h RenderOptions.h NoiseClass.h
../Release/matrix.o: defs.h
../Debug/AdaptOptions.o:	AdaptOptions.cpp
../Debug/Bitmap.o:	Bitmap.cpp
../Debug/ColorClass.o:	ColorClass.cpp
../Debug/Craters.o:	Craters.cpp
../Debug/Effects.o:	Effects.cpp
../Debug/Erode.o:	Erode.cpp
../Debug/FileUtil.o:	FileUtil.cpp
../Debug/Fractal.o:	Fractal.cpp
../Debug/FrameBuffer.o:	FrameBuffer.cpp
../Debug/FrameClass.o:	FrameClass.cpp
../Debug/GLSLMgr.o:	GLSLMgr.cpp
../Debug/GLSLProgram.o:	GLSLProgram.cpp
../Debug/GLSLVars.o:	GLSLVars.cpp
../Debug/GLSupport.o:	GLSupport.cpp
../Debug/ImageClass.o:	ImageClass.cpp
../Debug/ImageMgr.o:	ImageMgr.cpp
../Debug/Jpeg.o:	Jpeg.cpp
../Debug/KeyIF.o:	KeyIF.cpp
../Debug/Layers.o:	Layers.cpp
../Debug/LightClass.o:	LightClass.cpp
../Debug/ListClass.o:	ListClass.cpp
../Debug/MapClass.o:	MapClass.cpp
../Debug/MapLink.o:	MapLink.cpp
../Debug/MapNode.o:	MapNode.cpp
../Debug/MatrixClass.o:	MatrixClass.cpp
../Debug/ModelClass.o:	ModelClass.cpp
../Debug/NodeData.o:	NodeData.cpp
../Debug/NoiseClass.o:	NoiseClass.cpp
../Debug/NoiseFuncs.o:	NoiseFuncs.cpp
../Debug/ObjectClass.o:	ObjectClass.cpp
../Debug/Octree.o:	Octree.cpp
../Debug/OrbitalClass.o:	OrbitalClass.cpp
../Debug/Perlin.o:	Perlin.cpp
../Debug/Placements.o:	Placements.cpp
../Debug/PointClass.o:	PointClass.cpp
../Debug/PreviewRenderer.o:	PreviewRenderer.cpp
../Debug/Raster.o:	Raster.cpp
../Debug/Renderer.o:	Renderer.cpp
../Debug/Rocks.o:	Rocks.cpp
../Debug/SceneClass.o:	SceneClass.cpp
../Debug/TerrainClass.o:	TerrainClass.cpp
../Debug/TerrainData.o:	TerrainData.cpp
../Debug/TerrainMgr.o:	TerrainMgr.cpp
../Debug/TerrainNode.o:	TerrainNode.cpp
../Debug/TextureClass.o:	TextureClass.cpp
../Debug/TrackBall.o:	TrackBall.cpp
../Debug/UniverseModel.o:	UniverseModel.cpp
../Debug/ViewClass.o:	ViewClass.cpp
../Debug/matrix.o:	matrix.cpp
../Debug/star_data.o:	star_data.cpp
../Debug/cloud_data.o:	cloud_data.cpp
../Release/AdaptOptions.o:	AdaptOptions.cpp
../Release/Bitmap.o:	Bitmap.cpp
../Release/ColorClass.o:	ColorClass.cpp
../Release/Craters.o:	Craters.cpp
../Release/Effects.o:	Effects.cpp
../Release/Erode.o:	Erode.cpp
../Release/FileUtil.o:	FileUtil.cpp
../Release/Fractal.o:	Fractal.cpp
../Release/FrameBuffer.o:	FrameBuffer.cpp
../Release/FrameClass.o:	FrameClass.cpp
../Release/GLSLMgr.o:	GLSLMgr.cpp
../Release/GLSLProgram.o:	GLSLProgram.cpp
../Release/GLSLVars.o:	GLSLVars.cpp
../Release/GLSupport.o:	GLSupport.cpp
../Release/ImageClass.o:	ImageClass.cpp
../Release/ImageMgr.o:	ImageMgr.cpp
../Release/Jpeg.o:	Jpeg.cpp
../Release/KeyIF.o:	KeyIF.cpp
../Release/Layers.o:	Layers.cpp
../Release/LightClass.o:	LightClass.cpp
../Release/ListClass.o:	ListClass.cpp
../Release/MapClass.o:	MapClass.cpp
../Release/MapLink.o:	MapLink.cpp
../Release/MapNode.o:	MapNode.cpp
../Release/MatrixClass.o:	MatrixClass.cpp
../Release/ModelClass.o:	ModelClass.cpp
../Release/NodeData.o:	NodeData.cpp
../Release/NoiseClass.o:	NoiseClass.cpp
../Release/NoiseFuncs.o:	NoiseFuncs.cpp
../Release/ObjectClass.o:	ObjectClass.cpp
../Release/Octree.o:	Octree.cpp
../Release/OrbitalClass.o:	OrbitalClass.cpp
../Release/Perlin.o:	Perlin.cpp
../Release/Placements.o:	Placements.cpp
../Release/PointClass.o:	PointClass.cpp
../Release/PreviewRenderer.o:	PreviewRenderer.cpp
../Release/Raster.o:	Raster.cpp
../Release/Renderer.o:	Renderer.cpp
../Release/Rocks.o:	Rocks.cpp
../Release/SceneClass.o:	SceneClass.cpp
../Release/TerrainClass.o:	TerrainClass.cpp
../Release/TerrainData.o:	TerrainData.cpp
../Release/TerrainMgr.o:	TerrainMgr.cpp
../Release/TerrainNode.o:	TerrainNode.cpp
../Release/TextureClass.o:	TextureClass.cpp
../Release/TrackBall.o:	TrackBall.cpp
../Release/UniverseModel.o:	UniverseModel.cpp
../Release/ViewClass.o:	ViewClass.cpp
../Release/matrix.o:	matrix.cpp
../Release/star_data.o:	star_data.cpp
../Release/cloud_data.o:	cloud_data.cpp
