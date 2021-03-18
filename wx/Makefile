#################
#
#
#################
#################
# 'make' build parameters
#################
MAKEFILE      = Makefile
THIS	      = glut
#################
# Compiler parameters
#################
# passed in by build target in $(OBJDIR)/Makefile
######################
INCS          = -I../ -I../includes  -I../Resources/UI 
OPTLVL        = 
GFLAGS        = 
CFLAGS		  = -g
CC            = g++
OBJDIR        =
#################
# Program parameters
#################
SRC_FILES        = \
			 VtxBandsTabs.cpp \
			 VtxCloudsTabs.cpp \
			 VtxColorTabs.cpp \
			 VtxControls.cpp \
			 VtxCoronaTabs.cpp \
			 VtxCratersFunct.cpp \
			 VtxDefaultTabs.cpp \
             VtxErodeFunct.cpp \
			 VtxExprEdit.cpp \
			 VtxFogTabs.cpp \
             VtxFrame.cpp \
             VtxFractalFunct.cpp \
             VtxFractalTabs.cpp \
			 VtxFunctDialog.cpp \
			 VtxFunctMgr.cpp \
             VtxGalaxyTabs.cpp \
             VtxGlossTabs.cpp \
			 VtxImageWindow.cpp \
			 VtxImportTabs.cpp \
			 VtxImageDialog.cpp \
			 VtxMovieDialog.cpp \
			 VtxImageMgr.cpp \
			 VtxImageTabs.cpp \
			 VtxLayerTabs.cpp \
             VtxMain.cpp \
			 VtxMapTabs.cpp \
			 VtxMoonTabs.cpp \
			 VtxNoiseFunct.cpp \
 			 VtxPlanetTabs.cpp \
			 VtxPointTabs.cpp \
			 VtxRingTabs.cpp \
			 VtxRocksTabs.cpp \
             VtxScene.cpp \
             VtxSceneDialog.cpp \
             VtxSceneTabs.cpp \
             VtxSnowTabs.cpp \
             VtxStarTabs.cpp \
             VtxSkyTabs.cpp \
             VtxSurfaceTabs.cpp \
             VtxSystemTabs.cpp \
             VtxTabsMgr.cpp \
             VtxTexTabs.cpp \
             VtxBitmapPanel.cpp \
             VtxWaterTabs.cpp \

OBJS= $(SRC_FILES:%.cpp=$(OBJDIR)/%.o)

COMPILE.cc    = $(CC) $(INCS) `wx-config --cxxflags` -DWXWIN $(CFLAGS) -c
#################
# Target Rules
#################

build:	  $(OBJDIR)/GLglue.o    $(OBJS)  

$(OBJDIR)/%.o: %.cpp
	$(COMPILE.cc) -o $(@) $(@F:%.o=%.cpp)

$(OBJDIR)/GLglue.o:       ../includes/GLglue.cpp
			  $(COMPILE.cc)  -o $(@) ../includes/GLglue.cpp

clean:;		   @rm -fr *.o
showenv:;      @echo WXLIBS = `wx-config --libs --gl_libs`;
			   @echo WXFLGS = `wx-config --cxxflags`
			   
depend:;      @makedepend -Y./ -I../includes $(SRC_FILES) -p"../Debug/" 2>/dev/null; \
               makedepend -a  -Y./ -I../includes $(SRC_FILES) -p"../Release/" 2>/dev/null; \
                 rm -f tmp; \
                 for file in $(SRC_FILES:%.cpp=%); do \
		              echo "../Debug/$$file.o:	$$file.cpp" >> tmp; \
	             done; \
                 for file in $(SRC_FILES:%.cpp=%); do \
		              echo "../Release/$$file.o: $$file.cpp" >> tmp; \
	             done; \
	             cat tmp >> Makefile; \
	             rm tmp;
                 

#################
# dependencies
#################

# DO NOT DELETE THIS LINE -- make depend depends on it.

../Debug/VtxBandsTabs.o: VtxImageDialog.h VtxControls.h VtxExprEdit.h
../Debug/VtxBandsTabs.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Debug/VtxBandsTabs.o: VtxImageTabs.h ../includes/ImageClass.h
../Debug/VtxBandsTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxBandsTabs.o: ../includes/PointClass.h ../includes/defs.h
../Debug/VtxBandsTabs.o: ../includes/ListClass.h ../includes/TextureClass.h
../Debug/VtxBandsTabs.o: ../includes/GLSupport.h ../includes/GLglue.h
../Debug/VtxBandsTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxBandsTabs.o: ../includes/TerrainData.h ../includes/NodeIF.h
../Debug/VtxBandsTabs.o: VtxImageWindow.h VtxImportTabs.h VtxBandsTabs.h
../Debug/VtxBandsTabs.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxBandsTabs.o: ../includes/ObjectClass.h ../includes/OrbitalClass.h
../Debug/VtxBandsTabs.o: ../includes/Renderer.h ../includes/ViewClass.h
../Debug/VtxBandsTabs.o: ../includes/Util.h ../includes/MatrixClass.h
../Debug/VtxBandsTabs.o: ../includes/matrix.h ../includes/TrackBall.h
../Debug/VtxBandsTabs.o: ../includes/ModelClass.h ../includes/FileUtil.h
../Debug/VtxBandsTabs.o: ../includes/RenderOptions.h
../Debug/VtxCloudsTabs.o: VtxCloudsTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxCloudsTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxCloudsTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxCloudsTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxCloudsTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxCloudsTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxCloudsTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxCloudsTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxCloudsTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxCloudsTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxCloudsTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxCloudsTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxCloudsTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxCloudsTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxCloudsTabs.o: ../includes/UniverseModel.h
../Debug/VtxCloudsTabs.o: ../includes/TerrainClass.h
../Debug/VtxColorTabs.o: VtxColorTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxColorTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxColorTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxColorTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxColorTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxColorTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxColorTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxColorTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxColorTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxColorTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxColorTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxColorTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxColorTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxColorTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxColorTabs.o: ../includes/TerrainClass.h ../includes/FileUtil.h
../Debug/VtxControls.o: VtxControls.h VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxControls.o: ../includes/GLheaders.h ../includes/TerrainMgr.h
../Debug/VtxControls.o: ../includes/NoiseClass.h ../includes/PointClass.h
../Debug/VtxControls.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxControls.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxControls.o: ../includes/GLglue.h ../includes/ImageMgr.h
../Debug/VtxControls.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Debug/VtxControls.o: ../includes/NodeIF.h ../includes/SceneClass.h
../Debug/VtxControls.o: ../includes/LightClass.h ../includes/ObjectClass.h
../Debug/VtxControls.o: ../includes/OrbitalClass.h ../includes/Renderer.h
../Debug/VtxControls.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxControls.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxControls.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxCoronaTabs.o: VtxCoronaTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxCoronaTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxCoronaTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxCoronaTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxCoronaTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxCoronaTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxCoronaTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxCoronaTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxCoronaTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxCoronaTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxCoronaTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxCoronaTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxCoronaTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxCoronaTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxCoronaTabs.o: ../includes/UniverseModel.h
../Debug/VtxCratersFunct.o: VtxCratersFunct.h VtxFunctMgr.h VtxControls.h
../Debug/VtxCratersFunct.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxCratersFunct.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxCratersFunct.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxCratersFunct.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxCratersFunct.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxCratersFunct.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxCratersFunct.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxCratersFunct.o: ../includes/TextureClass.h
../Debug/VtxCratersFunct.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Debug/VtxCratersFunct.o: ../includes/TerrainNode.h
../Debug/VtxCratersFunct.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxCratersFunct.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxCratersFunct.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxCratersFunct.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxCratersFunct.o: VtxFunctDialog.h ../includes/Craters.h
../Debug/VtxCratersFunct.o: ../includes/Placements.h
../Debug/VtxDefaultTabs.o: VtxDefaultTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxDefaultTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxDefaultTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxDefaultTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxDefaultTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxDefaultTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxDefaultTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxDefaultTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxDefaultTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxDefaultTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxDefaultTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxDefaultTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxDefaultTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxDefaultTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxDefaultTabs.o: ../includes/RenderOptions.h ../includes/GLSLMgr.h
../Debug/VtxDefaultTabs.o: ../includes/GLSLProgram.h ../includes/GLSLVars.h
../Debug/VtxDefaultTabs.o: ../includes/FrameBuffer.h
../Debug/VtxExprEdit.o: ../includes/UniverseModel.h ../includes/ModelClass.h
../Debug/VtxExprEdit.o: ../includes/ListClass.h ../includes/defs.h
../Debug/VtxExprEdit.o: ../includes/NodeIF.h ../includes/SceneClass.h
../Debug/VtxExprEdit.o: ../includes/LightClass.h ../includes/ColorClass.h
../Debug/VtxExprEdit.o: ../includes/GLheaders.h ../includes/PointClass.h
../Debug/VtxExprEdit.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxExprEdit.o: ../includes/OrbitalClass.h ../includes/TerrainMgr.h
../Debug/VtxExprEdit.o: ../includes/NoiseClass.h ../includes/TextureClass.h
../Debug/VtxExprEdit.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Debug/VtxExprEdit.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Debug/VtxExprEdit.o: ../includes/Renderer.h ../includes/ViewClass.h
../Debug/VtxExprEdit.o: ../includes/Util.h ../includes/MatrixClass.h
../Debug/VtxExprEdit.o: ../includes/matrix.h ../includes/TrackBall.h
../Debug/VtxExprEdit.o: VtxExprEdit.h VtxFunctDialog.h VtxFunctMgr.h
../Debug/VtxExprEdit.o: VtxControls.h
../Debug/VtxFogTabs.o: VtxFogTabs.h VtxTabsMgr.h VtxControls.h VtxExprEdit.h
../Debug/VtxFogTabs.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Debug/VtxFogTabs.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxFogTabs.o: ../includes/PointClass.h ../includes/defs.h
../Debug/VtxFogTabs.o: ../includes/ListClass.h ../includes/ObjectClass.h
../Debug/VtxFogTabs.o: ../includes/GLglue.h ../includes/NodeIF.h
../Debug/VtxFogTabs.o: ../includes/OrbitalClass.h ../includes/TerrainMgr.h
../Debug/VtxFogTabs.o: ../includes/NoiseClass.h ../includes/TextureClass.h
../Debug/VtxFogTabs.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Debug/VtxFogTabs.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Debug/VtxFogTabs.o: ../includes/Renderer.h ../includes/ViewClass.h
../Debug/VtxFogTabs.o: ../includes/Util.h ../includes/MatrixClass.h
../Debug/VtxFogTabs.o: ../includes/matrix.h ../includes/TrackBall.h
../Debug/VtxFogTabs.o: ../includes/ModelClass.h ../includes/TerrainClass.h
../Debug/VtxFogTabs.o: VtxSceneDialog.h ../includes/RenderOptions.h
../Debug/VtxFrame.o: ../includes/GLglue.h ../includes/GLheaders.h VtxFrame.h
../Debug/VtxFrame.o: VtxMain.h VtxScene.h ../includes/FileUtil.h
../Debug/VtxFrame.o: ../includes/ListClass.h ../includes/defs.h
../Debug/VtxFrame.o: ../includes/KeyIF.h ../includes/ViewClass.h
../Debug/VtxFrame.o: ../includes/LightClass.h ../includes/ColorClass.h
../Debug/VtxFrame.o: ../includes/PointClass.h ../includes/ObjectClass.h
../Debug/VtxFrame.o: ../includes/NodeIF.h ../includes/Util.h
../Debug/VtxFrame.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxFrame.o: ../includes/TrackBall.h
../Debug/VtxFractalFunct.o: VtxFractalFunct.h VtxFunctMgr.h VtxControls.h
../Debug/VtxFractalFunct.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxFractalFunct.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxFractalFunct.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxFractalFunct.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxFractalFunct.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxFractalFunct.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxFractalFunct.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxFractalFunct.o: ../includes/TextureClass.h
../Debug/VtxFractalFunct.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Debug/VtxFractalFunct.o: ../includes/TerrainNode.h
../Debug/VtxFractalFunct.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxFractalFunct.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxFractalFunct.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxFractalFunct.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxFractalFunct.o: VtxFunctDialog.h ../includes/Fractal.h
../Debug/VtxFunctDialog.o: VtxFunctDialog.h VtxFunctMgr.h VtxControls.h
../Debug/VtxFunctDialog.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxFunctDialog.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxFunctDialog.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxFunctDialog.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxFunctDialog.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxFunctDialog.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxFunctDialog.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxFunctDialog.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxFunctDialog.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxFunctDialog.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxFunctDialog.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxFunctDialog.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxFunctDialog.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxFunctDialog.o: VtxNoiseFunct.h VtxCratersFunct.h
../Debug/VtxFunctDialog.o: VtxFractalFunct.h ../includes/UniverseModel.h
../Debug/VtxFunctMgr.o: VtxFunctMgr.h VtxControls.h VtxExprEdit.h
../Debug/VtxFunctMgr.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Debug/VtxFunctMgr.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxFunctMgr.o: ../includes/PointClass.h ../includes/defs.h
../Debug/VtxFunctMgr.o: ../includes/ListClass.h ../includes/ObjectClass.h
../Debug/VtxFunctMgr.o: ../includes/GLglue.h ../includes/NodeIF.h
../Debug/VtxFunctMgr.o: ../includes/OrbitalClass.h ../includes/TerrainMgr.h
../Debug/VtxFunctMgr.o: ../includes/NoiseClass.h ../includes/TextureClass.h
../Debug/VtxFunctMgr.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Debug/VtxFunctMgr.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Debug/VtxFunctMgr.o: ../includes/Renderer.h ../includes/ViewClass.h
../Debug/VtxFunctMgr.o: ../includes/Util.h ../includes/MatrixClass.h
../Debug/VtxFunctMgr.o: ../includes/matrix.h ../includes/TrackBall.h
../Debug/VtxFunctMgr.o: ../includes/ModelClass.h
../Debug/VtxGalaxyTabs.o: VtxGalaxyTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxGalaxyTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxGalaxyTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxGalaxyTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxGalaxyTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxGalaxyTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxGalaxyTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxGalaxyTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxGalaxyTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxGalaxyTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxGalaxyTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxGalaxyTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxGalaxyTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxGalaxyTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxGalaxyTabs.o: ../includes/Octree.h ../includes/RenderOptions.h
../Debug/VtxGalaxyTabs.o: VtxSceneDialog.h
../Debug/VtxGlossTabs.o: VtxGlossTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxGlossTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxGlossTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxGlossTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxGlossTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxGlossTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxGlossTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxGlossTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxGlossTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxGlossTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxGlossTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxGlossTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxGlossTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxGlossTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxGlossTabs.o: ../includes/TerrainClass.h
../Debug/VtxImageWindow.o: VtxImageWindow.h ../includes/ImageMgr.h
../Debug/VtxImageWindow.o: ../includes/GLheaders.h ../includes/defs.h
../Debug/VtxImageWindow.o: ../includes/ColorClass.h ../includes/ListClass.h
../Debug/VtxImportTabs.o: VtxImageDialog.h VtxControls.h VtxExprEdit.h
../Debug/VtxImportTabs.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Debug/VtxImportTabs.o: VtxImageTabs.h ../includes/ImageClass.h
../Debug/VtxImportTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxImportTabs.o: ../includes/PointClass.h ../includes/defs.h
../Debug/VtxImportTabs.o: ../includes/ListClass.h ../includes/TextureClass.h
../Debug/VtxImportTabs.o: ../includes/GLSupport.h ../includes/GLglue.h
../Debug/VtxImportTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxImportTabs.o: ../includes/TerrainData.h ../includes/NodeIF.h
../Debug/VtxImportTabs.o: VtxImageWindow.h VtxImportTabs.h VtxBandsTabs.h
../Debug/VtxImportTabs.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxImportTabs.o: ../includes/ObjectClass.h
../Debug/VtxImportTabs.o: ../includes/OrbitalClass.h ../includes/Renderer.h
../Debug/VtxImportTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxImportTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxImportTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxImportTabs.o: ../includes/FileUtil.h ../includes/RenderOptions.h
../Debug/VtxImageDialog.o: VtxImageDialog.h VtxControls.h VtxExprEdit.h
../Debug/VtxImageDialog.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Debug/VtxImageDialog.o: VtxImageTabs.h ../includes/ImageClass.h
../Debug/VtxImageDialog.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxImageDialog.o: ../includes/PointClass.h ../includes/defs.h
../Debug/VtxImageDialog.o: ../includes/ListClass.h ../includes/TextureClass.h
../Debug/VtxImageDialog.o: ../includes/GLSupport.h ../includes/GLglue.h
../Debug/VtxImageDialog.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxImageDialog.o: ../includes/TerrainData.h ../includes/NodeIF.h
../Debug/VtxImageDialog.o: VtxImageWindow.h VtxImportTabs.h VtxBandsTabs.h
../Debug/VtxImageDialog.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxImageDialog.o: ../includes/ObjectClass.h
../Debug/VtxImageDialog.o: ../includes/OrbitalClass.h ../includes/Renderer.h
../Debug/VtxImageDialog.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxImageDialog.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxImageDialog.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxImageDialog.o: ../includes/UniverseModel.h
../Debug/VtxImageMgr.o: ../includes/GLheaders.h ../includes/defs.h
../Debug/VtxImageMgr.o: VtxImageMgr.h
../Debug/VtxImageTabs.o: VtxImageDialog.h VtxControls.h VtxExprEdit.h
../Debug/VtxImageTabs.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Debug/VtxImageTabs.o: VtxImageTabs.h ../includes/ImageClass.h
../Debug/VtxImageTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxImageTabs.o: ../includes/PointClass.h ../includes/defs.h
../Debug/VtxImageTabs.o: ../includes/ListClass.h ../includes/TextureClass.h
../Debug/VtxImageTabs.o: ../includes/GLSupport.h ../includes/GLglue.h
../Debug/VtxImageTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxImageTabs.o: ../includes/TerrainData.h ../includes/NodeIF.h
../Debug/VtxImageTabs.o: VtxImageWindow.h VtxImportTabs.h VtxBandsTabs.h
../Debug/VtxImageTabs.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxImageTabs.o: ../includes/ObjectClass.h ../includes/OrbitalClass.h
../Debug/VtxImageTabs.o: ../includes/Renderer.h ../includes/ViewClass.h
../Debug/VtxImageTabs.o: ../includes/Util.h ../includes/MatrixClass.h
../Debug/VtxImageTabs.o: ../includes/matrix.h ../includes/TrackBall.h
../Debug/VtxImageTabs.o: ../includes/ModelClass.h ../includes/FileUtil.h
../Debug/VtxImageTabs.o: ../includes/RenderOptions.h
../Debug/VtxLayerTabs.o: VtxLayerTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxLayerTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxLayerTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxLayerTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxLayerTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxLayerTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxLayerTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxLayerTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxLayerTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxLayerTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxLayerTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxLayerTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxLayerTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxLayerTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxLayerTabs.o: ../includes/Layers.h VtxSceneDialog.h
../Debug/VtxMain.o: ../includes/GLglue.h ../includes/GLheaders.h VtxMain.h
../Debug/VtxMain.o: VtxFrame.h VtxScene.h ../includes/FileUtil.h
../Debug/VtxMain.o: ../includes/ListClass.h ../includes/defs.h
../Debug/VtxMapTabs.o: VtxMapTabs.h VtxTabsMgr.h VtxControls.h VtxExprEdit.h
../Debug/VtxMapTabs.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Debug/VtxMapTabs.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxMapTabs.o: ../includes/PointClass.h ../includes/defs.h
../Debug/VtxMapTabs.o: ../includes/ListClass.h ../includes/ObjectClass.h
../Debug/VtxMapTabs.o: ../includes/GLglue.h ../includes/NodeIF.h
../Debug/VtxMapTabs.o: ../includes/OrbitalClass.h ../includes/TerrainMgr.h
../Debug/VtxMapTabs.o: ../includes/NoiseClass.h ../includes/TextureClass.h
../Debug/VtxMapTabs.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Debug/VtxMapTabs.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Debug/VtxMapTabs.o: ../includes/Renderer.h ../includes/ViewClass.h
../Debug/VtxMapTabs.o: ../includes/Util.h ../includes/MatrixClass.h
../Debug/VtxMapTabs.o: ../includes/matrix.h ../includes/TrackBall.h
../Debug/VtxMapTabs.o: ../includes/ModelClass.h ../includes/Layers.h
../Debug/VtxMoonTabs.o: VtxMoonTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxMoonTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxMoonTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxMoonTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxMoonTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxMoonTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxMoonTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxMoonTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxMoonTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxMoonTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxMoonTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxMoonTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxMoonTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxMoonTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxMoonTabs.o: ../includes/UniverseModel.h
../Debug/VtxNoiseFunct.o: VtxNoiseFunct.h VtxFunctMgr.h VtxControls.h
../Debug/VtxNoiseFunct.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxNoiseFunct.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxNoiseFunct.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxNoiseFunct.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxNoiseFunct.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxNoiseFunct.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxNoiseFunct.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxNoiseFunct.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxNoiseFunct.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxNoiseFunct.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxNoiseFunct.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxNoiseFunct.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxNoiseFunct.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxNoiseFunct.o: VtxFunctDialog.h ../includes/TerrainClass.h
../Debug/VtxPlanetTabs.o: VtxPlanetTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxPlanetTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxPlanetTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxPlanetTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxPlanetTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxPlanetTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxPlanetTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxPlanetTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxPlanetTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxPlanetTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxPlanetTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxPlanetTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxPlanetTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxPlanetTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxPlanetTabs.o: ../includes/UniverseModel.h
../Debug/VtxPointTabs.o: VtxPointTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxPointTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxPointTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxPointTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxPointTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxPointTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxPointTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxPointTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxPointTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxPointTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxPointTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxPointTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxPointTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxPointTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxPointTabs.o: ../includes/TerrainClass.h ../includes/FileUtil.h
../Debug/VtxRingTabs.o: VtxRingTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxRingTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxRingTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxRingTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxRingTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxRingTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxRingTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxRingTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxRingTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxRingTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxRingTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxRingTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxRingTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxRingTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxRingTabs.o: ../includes/UniverseModel.h
../Debug/VtxRocksTabs.o: VtxRocksTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxRocksTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxRocksTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxRocksTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxRocksTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxRocksTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxRocksTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxRocksTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxRocksTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxRocksTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxRocksTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxRocksTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxRocksTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxRocksTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxRocksTabs.o: ../includes/Rocks.h ../includes/Placements.h
../Debug/VtxScene.o: ../includes/GLglue.h ../includes/GLheaders.h
../Debug/VtxScene.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxScene.o: ../includes/ColorClass.h ../includes/PointClass.h
../Debug/VtxScene.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxScene.o: ../includes/ObjectClass.h ../includes/NodeIF.h
../Debug/VtxScene.o: ../includes/OrbitalClass.h ../includes/TerrainMgr.h
../Debug/VtxScene.o: ../includes/NoiseClass.h ../includes/TextureClass.h
../Debug/VtxScene.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Debug/VtxScene.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Debug/VtxScene.o: ../includes/Renderer.h ../includes/ViewClass.h
../Debug/VtxScene.o: ../includes/Util.h ../includes/MatrixClass.h
../Debug/VtxScene.o: ../includes/matrix.h ../includes/TrackBall.h
../Debug/VtxScene.o: ../includes/ModelClass.h ../includes/UniverseModel.h
../Debug/VtxScene.o: ../includes/TerrainClass.h ../includes/RenderOptions.h
../Debug/VtxScene.o: ../includes/AdaptOptions.h ../includes/Effects.h
../Debug/VtxScene.o: ../includes/Raster.h ../includes/KeyIF.h
../Debug/VtxScene.o: ../includes/GLSLMgr.h ../includes/GLSLProgram.h
../Debug/VtxScene.o: ../includes/GLSLVars.h ../includes/FrameBuffer.h
../Debug/VtxScene.o: ../includes/FileUtil.h VtxScene.h ../includes/Bitmap.h
../Debug/VtxScene.o: ../includes/FrameClass.h VtxSceneDialog.h VtxControls.h
../Debug/VtxScene.o: VtxExprEdit.h VtxTabsMgr.h VtxImageDialog.h
../Debug/VtxScene.o: VtxImageTabs.h ../includes/ImageClass.h VtxImageWindow.h
../Debug/VtxScene.o: VtxImportTabs.h VtxBandsTabs.h VtxFunctDialog.h
../Debug/VtxScene.o: VtxFunctMgr.h VtxImageMgr.h
../Debug/VtxSceneDialog.o: VtxSceneDialog.h VtxControls.h VtxExprEdit.h
../Debug/VtxSceneDialog.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Debug/VtxSceneDialog.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxSceneDialog.o: ../includes/PointClass.h ../includes/defs.h
../Debug/VtxSceneDialog.o: ../includes/ListClass.h ../includes/ObjectClass.h
../Debug/VtxSceneDialog.o: ../includes/GLglue.h ../includes/NodeIF.h
../Debug/VtxSceneDialog.o: ../includes/OrbitalClass.h
../Debug/VtxSceneDialog.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxSceneDialog.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxSceneDialog.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxSceneDialog.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxSceneDialog.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxSceneDialog.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxSceneDialog.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxSceneDialog.o: VtxTabsMgr.h ../includes/UniverseModel.h
../Debug/VtxSceneDialog.o: ../includes/FileUtil.h VtxSceneTabs.h
../Debug/VtxSceneDialog.o: ../includes/RenderOptions.h ../includes/Effects.h
../Debug/VtxSceneDialog.o: ../includes/Raster.h VtxPlanetTabs.h VtxMoonTabs.h
../Debug/VtxSceneDialog.o: VtxStarTabs.h VtxCoronaTabs.h VtxTexTabs.h
../Debug/VtxSceneDialog.o: ../includes/ImageClass.h VtxImageWindow.h
../Debug/VtxSceneDialog.o: VtxDefaultTabs.h VtxCloudsTabs.h VtxColorTabs.h
../Debug/VtxSceneDialog.o: ../includes/TerrainClass.h VtxSkyTabs.h
../Debug/VtxSceneDialog.o: VtxPointTabs.h VtxGlossTabs.h VtxLayerTabs.h
../Debug/VtxSceneDialog.o: ../includes/Layers.h VtxMapTabs.h VtxFogTabs.h
../Debug/VtxSceneDialog.o: VtxWaterTabs.h VtxSurfaceTabs.h VtxRocksTabs.h
../Debug/VtxSceneDialog.o: ../includes/Rocks.h ../includes/Placements.h
../Debug/VtxSceneDialog.o: VtxRingTabs.h VtxSnowTabs.h VtxSystemTabs.h
../Debug/VtxSceneDialog.o: VtxGalaxyTabs.h ../includes/Octree.h
../Debug/VtxSceneTabs.o: VtxSceneTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxSceneTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxSceneTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxSceneTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxSceneTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxSceneTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxSceneTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxSceneTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxSceneTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxSceneTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxSceneTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxSceneTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxSceneTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxSceneTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxSceneTabs.o: ../includes/RenderOptions.h ../includes/Effects.h
../Debug/VtxSceneTabs.o: ../includes/Raster.h ../includes/GLSLMgr.h
../Debug/VtxSceneTabs.o: ../includes/GLSLProgram.h ../includes/GLSLVars.h
../Debug/VtxSceneTabs.o: ../includes/FrameBuffer.h
../Debug/VtxSnowTabs.o: VtxSnowTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxSnowTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxSnowTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxSnowTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxSnowTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxSnowTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxSnowTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxSnowTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxSnowTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxSnowTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxSnowTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxSnowTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxSnowTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxSnowTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxSnowTabs.o: ../includes/TerrainClass.h
../Debug/VtxStarTabs.o: VtxStarTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxStarTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxStarTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxStarTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxStarTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxStarTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxStarTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxStarTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxStarTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxStarTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxStarTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxStarTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxStarTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxStarTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxSkyTabs.o: VtxSkyTabs.h VtxTabsMgr.h VtxControls.h VtxExprEdit.h
../Debug/VtxSkyTabs.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Debug/VtxSkyTabs.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxSkyTabs.o: ../includes/PointClass.h ../includes/defs.h
../Debug/VtxSkyTabs.o: ../includes/ListClass.h ../includes/ObjectClass.h
../Debug/VtxSkyTabs.o: ../includes/GLglue.h ../includes/NodeIF.h
../Debug/VtxSkyTabs.o: ../includes/OrbitalClass.h ../includes/TerrainMgr.h
../Debug/VtxSkyTabs.o: ../includes/NoiseClass.h ../includes/TextureClass.h
../Debug/VtxSkyTabs.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Debug/VtxSkyTabs.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Debug/VtxSkyTabs.o: ../includes/Renderer.h ../includes/ViewClass.h
../Debug/VtxSkyTabs.o: ../includes/Util.h ../includes/MatrixClass.h
../Debug/VtxSkyTabs.o: ../includes/matrix.h ../includes/TrackBall.h
../Debug/VtxSkyTabs.o: ../includes/ModelClass.h ../includes/RenderOptions.h
../Debug/VtxSurfaceTabs.o: VtxSurfaceTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxSurfaceTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxSurfaceTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxSurfaceTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxSurfaceTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxSurfaceTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxSurfaceTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxSurfaceTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxSurfaceTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxSurfaceTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxSurfaceTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxSurfaceTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxSurfaceTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxSurfaceTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxSurfaceTabs.o: ../includes/RenderOptions.h VtxSceneDialog.h
../Debug/VtxSystemTabs.o: VtxSystemTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxSystemTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxSystemTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxSystemTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxSystemTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxSystemTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxSystemTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxSystemTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxSystemTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxSystemTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxSystemTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxSystemTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxSystemTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxSystemTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxSystemTabs.o: ../includes/RenderOptions.h VtxSceneDialog.h
../Debug/VtxTabsMgr.o: VtxTabsMgr.h VtxControls.h VtxExprEdit.h
../Debug/VtxTabsMgr.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Debug/VtxTabsMgr.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxTabsMgr.o: ../includes/PointClass.h ../includes/defs.h
../Debug/VtxTabsMgr.o: ../includes/ListClass.h ../includes/ObjectClass.h
../Debug/VtxTabsMgr.o: ../includes/GLglue.h ../includes/NodeIF.h
../Debug/VtxTabsMgr.o: ../includes/OrbitalClass.h ../includes/TerrainMgr.h
../Debug/VtxTabsMgr.o: ../includes/NoiseClass.h ../includes/TextureClass.h
../Debug/VtxTabsMgr.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Debug/VtxTabsMgr.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Debug/VtxTabsMgr.o: ../includes/Renderer.h ../includes/ViewClass.h
../Debug/VtxTabsMgr.o: ../includes/Util.h ../includes/MatrixClass.h
../Debug/VtxTabsMgr.o: ../includes/matrix.h ../includes/TrackBall.h
../Debug/VtxTabsMgr.o: ../includes/ModelClass.h VtxSceneDialog.h
../Debug/VtxTexTabs.o: VtxTexTabs.h VtxTabsMgr.h VtxControls.h VtxExprEdit.h
../Debug/VtxTexTabs.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Debug/VtxTexTabs.o: ../includes/SceneClass.h ../includes/LightClass.h
../Debug/VtxTexTabs.o: ../includes/PointClass.h ../includes/defs.h
../Debug/VtxTexTabs.o: ../includes/ListClass.h ../includes/ObjectClass.h
../Debug/VtxTexTabs.o: ../includes/GLglue.h ../includes/NodeIF.h
../Debug/VtxTexTabs.o: ../includes/OrbitalClass.h ../includes/TerrainMgr.h
../Debug/VtxTexTabs.o: ../includes/NoiseClass.h ../includes/TextureClass.h
../Debug/VtxTexTabs.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Debug/VtxTexTabs.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Debug/VtxTexTabs.o: ../includes/Renderer.h ../includes/ViewClass.h
../Debug/VtxTexTabs.o: ../includes/Util.h ../includes/MatrixClass.h
../Debug/VtxTexTabs.o: ../includes/matrix.h ../includes/TrackBall.h
../Debug/VtxTexTabs.o: ../includes/ModelClass.h ../includes/ImageClass.h
../Debug/VtxTexTabs.o: VtxImageWindow.h VtxSceneDialog.h VtxImageDialog.h
../Debug/VtxTexTabs.o: VtxImageTabs.h VtxImportTabs.h VtxBandsTabs.h
../Debug/VtxTexTabs.o: ../includes/FileUtil.h
../Debug/VtxWaterTabs.o: VtxWaterTabs.h VtxTabsMgr.h VtxControls.h
../Debug/VtxWaterTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Debug/VtxWaterTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Debug/VtxWaterTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Debug/VtxWaterTabs.o: ../includes/defs.h ../includes/ListClass.h
../Debug/VtxWaterTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Debug/VtxWaterTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Debug/VtxWaterTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Debug/VtxWaterTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Debug/VtxWaterTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Debug/VtxWaterTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Debug/VtxWaterTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Debug/VtxWaterTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Debug/VtxWaterTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Debug/VtxWaterTabs.o: ../includes/TerrainClass.h VtxSceneDialog.h
../Debug/VtxWaterTabs.o: ../includes/RenderOptions.h

../Release/VtxBandsTabs.o: VtxImageDialog.h VtxControls.h VtxExprEdit.h
../Release/VtxBandsTabs.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Release/VtxBandsTabs.o: VtxImageTabs.h ../includes/ImageClass.h
../Release/VtxBandsTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxBandsTabs.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxBandsTabs.o: ../includes/ListClass.h ../includes/TextureClass.h
../Release/VtxBandsTabs.o: ../includes/GLSupport.h ../includes/GLglue.h
../Release/VtxBandsTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxBandsTabs.o: ../includes/TerrainData.h ../includes/NodeIF.h
../Release/VtxBandsTabs.o: VtxImageWindow.h VtxImportTabs.h VtxBandsTabs.h
../Release/VtxBandsTabs.o: ../includes/SceneClass.h ../includes/LightClass.h
../Release/VtxBandsTabs.o: ../includes/ObjectClass.h
../Release/VtxBandsTabs.o: ../includes/OrbitalClass.h ../includes/Renderer.h
../Release/VtxBandsTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxBandsTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxBandsTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxBandsTabs.o: ../includes/FileUtil.h ../includes/RenderOptions.h
../Release/VtxCloudsTabs.o: VtxCloudsTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxCloudsTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxCloudsTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxCloudsTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxCloudsTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxCloudsTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxCloudsTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxCloudsTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxCloudsTabs.o: ../includes/TextureClass.h
../Release/VtxCloudsTabs.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxCloudsTabs.o: ../includes/TerrainNode.h
../Release/VtxCloudsTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxCloudsTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxCloudsTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxCloudsTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxCloudsTabs.o: ../includes/UniverseModel.h
../Release/VtxCloudsTabs.o: ../includes/TerrainClass.h
../Release/VtxColorTabs.o: VtxColorTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxColorTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxColorTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxColorTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxColorTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxColorTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxColorTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxColorTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxColorTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxColorTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxColorTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxColorTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxColorTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxColorTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxColorTabs.o: ../includes/TerrainClass.h ../includes/FileUtil.h
../Release/VtxControls.o: VtxControls.h VtxExprEdit.h
../Release/VtxControls.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Release/VtxControls.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxControls.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxControls.o: ../includes/ListClass.h ../includes/TextureClass.h
../Release/VtxControls.o: ../includes/GLSupport.h ../includes/GLglue.h
../Release/VtxControls.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxControls.o: ../includes/TerrainData.h ../includes/NodeIF.h
../Release/VtxControls.o: ../includes/SceneClass.h ../includes/LightClass.h
../Release/VtxControls.o: ../includes/ObjectClass.h
../Release/VtxControls.o: ../includes/OrbitalClass.h ../includes/Renderer.h
../Release/VtxControls.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxControls.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxControls.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxCoronaTabs.o: VtxCoronaTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxCoronaTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxCoronaTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxCoronaTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxCoronaTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxCoronaTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxCoronaTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxCoronaTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxCoronaTabs.o: ../includes/TextureClass.h
../Release/VtxCoronaTabs.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxCoronaTabs.o: ../includes/TerrainNode.h
../Release/VtxCoronaTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxCoronaTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxCoronaTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxCoronaTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxCoronaTabs.o: ../includes/UniverseModel.h
../Release/VtxCratersFunct.o: VtxCratersFunct.h VtxFunctMgr.h VtxControls.h
../Release/VtxCratersFunct.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxCratersFunct.o: ../includes/GLheaders.h
../Release/VtxCratersFunct.o: ../includes/SceneClass.h
../Release/VtxCratersFunct.o: ../includes/LightClass.h
../Release/VtxCratersFunct.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxCratersFunct.o: ../includes/ListClass.h
../Release/VtxCratersFunct.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxCratersFunct.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxCratersFunct.o: ../includes/TerrainMgr.h
../Release/VtxCratersFunct.o: ../includes/NoiseClass.h
../Release/VtxCratersFunct.o: ../includes/TextureClass.h
../Release/VtxCratersFunct.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxCratersFunct.o: ../includes/TerrainNode.h
../Release/VtxCratersFunct.o: ../includes/TerrainData.h
../Release/VtxCratersFunct.o: ../includes/Renderer.h ../includes/ViewClass.h
../Release/VtxCratersFunct.o: ../includes/Util.h ../includes/MatrixClass.h
../Release/VtxCratersFunct.o: ../includes/matrix.h ../includes/TrackBall.h
../Release/VtxCratersFunct.o: ../includes/ModelClass.h VtxFunctDialog.h
../Release/VtxCratersFunct.o: ../includes/Craters.h ../includes/Placements.h
../Release/VtxDefaultTabs.o: VtxDefaultTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxDefaultTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxDefaultTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxDefaultTabs.o: ../includes/LightClass.h
../Release/VtxDefaultTabs.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxDefaultTabs.o: ../includes/ListClass.h
../Release/VtxDefaultTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxDefaultTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxDefaultTabs.o: ../includes/TerrainMgr.h
../Release/VtxDefaultTabs.o: ../includes/NoiseClass.h
../Release/VtxDefaultTabs.o: ../includes/TextureClass.h
../Release/VtxDefaultTabs.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxDefaultTabs.o: ../includes/TerrainNode.h
../Release/VtxDefaultTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxDefaultTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxDefaultTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxDefaultTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxDefaultTabs.o: ../includes/RenderOptions.h
../Release/VtxDefaultTabs.o: ../includes/GLSLMgr.h ../includes/GLSLProgram.h
../Release/VtxDefaultTabs.o: ../includes/GLSLVars.h ../includes/FrameBuffer.h
../Release/VtxExprEdit.o: ../includes/UniverseModel.h
../Release/VtxExprEdit.o: ../includes/ModelClass.h ../includes/ListClass.h
../Release/VtxExprEdit.o: ../includes/defs.h ../includes/NodeIF.h
../Release/VtxExprEdit.o: ../includes/SceneClass.h ../includes/LightClass.h
../Release/VtxExprEdit.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Release/VtxExprEdit.o: ../includes/PointClass.h ../includes/ObjectClass.h
../Release/VtxExprEdit.o: ../includes/GLglue.h ../includes/OrbitalClass.h
../Release/VtxExprEdit.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxExprEdit.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxExprEdit.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxExprEdit.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxExprEdit.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxExprEdit.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxExprEdit.o: ../includes/TrackBall.h VtxExprEdit.h
../Release/VtxExprEdit.o: VtxFunctDialog.h VtxFunctMgr.h VtxControls.h
../Release/VtxFogTabs.o: VtxFogTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxFogTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxFogTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxFogTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxFogTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxFogTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxFogTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxFogTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxFogTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxFogTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxFogTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxFogTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxFogTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxFogTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxFogTabs.o: ../includes/TerrainClass.h VtxSceneDialog.h
../Release/VtxFogTabs.o: ../includes/RenderOptions.h
../Release/VtxFrame.o: ../includes/GLglue.h ../includes/GLheaders.h
../Release/VtxFrame.o: VtxFrame.h VtxMain.h VtxScene.h ../includes/FileUtil.h
../Release/VtxFrame.o: ../includes/ListClass.h ../includes/defs.h
../Release/VtxFrame.o: ../includes/KeyIF.h ../includes/ViewClass.h
../Release/VtxFrame.o: ../includes/LightClass.h ../includes/ColorClass.h
../Release/VtxFrame.o: ../includes/PointClass.h ../includes/ObjectClass.h
../Release/VtxFrame.o: ../includes/NodeIF.h ../includes/Util.h
../Release/VtxFrame.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxFrame.o: ../includes/TrackBall.h
../Release/VtxFractalFunct.o: VtxFractalFunct.h VtxFunctMgr.h VtxControls.h
../Release/VtxFractalFunct.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxFractalFunct.o: ../includes/GLheaders.h
../Release/VtxFractalFunct.o: ../includes/SceneClass.h
../Release/VtxFractalFunct.o: ../includes/LightClass.h
../Release/VtxFractalFunct.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxFractalFunct.o: ../includes/ListClass.h
../Release/VtxFractalFunct.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxFractalFunct.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxFractalFunct.o: ../includes/TerrainMgr.h
../Release/VtxFractalFunct.o: ../includes/NoiseClass.h
../Release/VtxFractalFunct.o: ../includes/TextureClass.h
../Release/VtxFractalFunct.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxFractalFunct.o: ../includes/TerrainNode.h
../Release/VtxFractalFunct.o: ../includes/TerrainData.h
../Release/VtxFractalFunct.o: ../includes/Renderer.h ../includes/ViewClass.h
../Release/VtxFractalFunct.o: ../includes/Util.h ../includes/MatrixClass.h
../Release/VtxFractalFunct.o: ../includes/matrix.h ../includes/TrackBall.h
../Release/VtxFractalFunct.o: ../includes/ModelClass.h VtxFunctDialog.h
../Release/VtxFractalFunct.o: ../includes/Fractal.h
../Release/VtxFunctDialog.o: VtxFunctDialog.h VtxFunctMgr.h VtxControls.h
../Release/VtxFunctDialog.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxFunctDialog.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxFunctDialog.o: ../includes/LightClass.h
../Release/VtxFunctDialog.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxFunctDialog.o: ../includes/ListClass.h
../Release/VtxFunctDialog.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxFunctDialog.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxFunctDialog.o: ../includes/TerrainMgr.h
../Release/VtxFunctDialog.o: ../includes/NoiseClass.h
../Release/VtxFunctDialog.o: ../includes/TextureClass.h
../Release/VtxFunctDialog.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxFunctDialog.o: ../includes/TerrainNode.h
../Release/VtxFunctDialog.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxFunctDialog.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxFunctDialog.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxFunctDialog.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxFunctDialog.o: VtxNoiseFunct.h VtxCratersFunct.h
../Release/VtxFunctDialog.o: VtxFractalFunct.h ../includes/UniverseModel.h
../Release/VtxFunctMgr.o: VtxFunctMgr.h VtxControls.h VtxExprEdit.h
../Release/VtxFunctMgr.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Release/VtxFunctMgr.o: ../includes/SceneClass.h ../includes/LightClass.h
../Release/VtxFunctMgr.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxFunctMgr.o: ../includes/ListClass.h ../includes/ObjectClass.h
../Release/VtxFunctMgr.o: ../includes/GLglue.h ../includes/NodeIF.h
../Release/VtxFunctMgr.o: ../includes/OrbitalClass.h ../includes/TerrainMgr.h
../Release/VtxFunctMgr.o: ../includes/NoiseClass.h ../includes/TextureClass.h
../Release/VtxFunctMgr.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxFunctMgr.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Release/VtxFunctMgr.o: ../includes/Renderer.h ../includes/ViewClass.h
../Release/VtxFunctMgr.o: ../includes/Util.h ../includes/MatrixClass.h
../Release/VtxFunctMgr.o: ../includes/matrix.h ../includes/TrackBall.h
../Release/VtxFunctMgr.o: ../includes/ModelClass.h
../Release/VtxGalaxyTabs.o: VtxGalaxyTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxGalaxyTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxGalaxyTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxGalaxyTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxGalaxyTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxGalaxyTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxGalaxyTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxGalaxyTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxGalaxyTabs.o: ../includes/TextureClass.h
../Release/VtxGalaxyTabs.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxGalaxyTabs.o: ../includes/TerrainNode.h
../Release/VtxGalaxyTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxGalaxyTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxGalaxyTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxGalaxyTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxGalaxyTabs.o: ../includes/Octree.h ../includes/RenderOptions.h
../Release/VtxGalaxyTabs.o: VtxSceneDialog.h
../Release/VtxGlossTabs.o: VtxGlossTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxGlossTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxGlossTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxGlossTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxGlossTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxGlossTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxGlossTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxGlossTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxGlossTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxGlossTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxGlossTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxGlossTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxGlossTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxGlossTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxGlossTabs.o: ../includes/TerrainClass.h
../Release/VtxImageWindow.o: VtxImageWindow.h ../includes/ImageMgr.h
../Release/VtxImageWindow.o: ../includes/GLheaders.h ../includes/defs.h
../Release/VtxImageWindow.o: ../includes/ColorClass.h ../includes/ListClass.h
../Release/VtxImportTabs.o: VtxImageDialog.h VtxControls.h VtxExprEdit.h
../Release/VtxImportTabs.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Release/VtxImportTabs.o: VtxImageTabs.h ../includes/ImageClass.h
../Release/VtxImportTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxImportTabs.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxImportTabs.o: ../includes/ListClass.h
../Release/VtxImportTabs.o: ../includes/TextureClass.h
../Release/VtxImportTabs.o: ../includes/GLSupport.h ../includes/GLglue.h
../Release/VtxImportTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxImportTabs.o: ../includes/TerrainData.h ../includes/NodeIF.h
../Release/VtxImportTabs.o: VtxImageWindow.h VtxImportTabs.h VtxBandsTabs.h
../Release/VtxImportTabs.o: ../includes/SceneClass.h ../includes/LightClass.h
../Release/VtxImportTabs.o: ../includes/ObjectClass.h
../Release/VtxImportTabs.o: ../includes/OrbitalClass.h ../includes/Renderer.h
../Release/VtxImportTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxImportTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxImportTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxImportTabs.o: ../includes/FileUtil.h
../Release/VtxImportTabs.o: ../includes/RenderOptions.h
../Release/VtxImageDialog.o: VtxImageDialog.h VtxControls.h VtxExprEdit.h
../Release/VtxImageDialog.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Release/VtxImageDialog.o: VtxImageTabs.h ../includes/ImageClass.h
../Release/VtxImageDialog.o: ../includes/TerrainMgr.h
../Release/VtxImageDialog.o: ../includes/NoiseClass.h
../Release/VtxImageDialog.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxImageDialog.o: ../includes/ListClass.h
../Release/VtxImageDialog.o: ../includes/TextureClass.h
../Release/VtxImageDialog.o: ../includes/GLSupport.h ../includes/GLglue.h
../Release/VtxImageDialog.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxImageDialog.o: ../includes/TerrainData.h ../includes/NodeIF.h
../Release/VtxImageDialog.o: VtxImageWindow.h VtxImportTabs.h VtxBandsTabs.h
../Release/VtxImageDialog.o: ../includes/SceneClass.h
../Release/VtxImageDialog.o: ../includes/LightClass.h
../Release/VtxImageDialog.o: ../includes/ObjectClass.h
../Release/VtxImageDialog.o: ../includes/OrbitalClass.h
../Release/VtxImageDialog.o: ../includes/Renderer.h ../includes/ViewClass.h
../Release/VtxImageDialog.o: ../includes/Util.h ../includes/MatrixClass.h
../Release/VtxImageDialog.o: ../includes/matrix.h ../includes/TrackBall.h
../Release/VtxImageDialog.o: ../includes/ModelClass.h
../Release/VtxImageDialog.o: ../includes/UniverseModel.h
../Release/VtxImageMgr.o: ../includes/GLheaders.h ../includes/defs.h
../Release/VtxImageMgr.o: VtxImageMgr.h
../Release/VtxImageTabs.o: VtxImageDialog.h VtxControls.h VtxExprEdit.h
../Release/VtxImageTabs.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Release/VtxImageTabs.o: VtxImageTabs.h ../includes/ImageClass.h
../Release/VtxImageTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxImageTabs.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxImageTabs.o: ../includes/ListClass.h ../includes/TextureClass.h
../Release/VtxImageTabs.o: ../includes/GLSupport.h ../includes/GLglue.h
../Release/VtxImageTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxImageTabs.o: ../includes/TerrainData.h ../includes/NodeIF.h
../Release/VtxImageTabs.o: VtxImageWindow.h VtxImportTabs.h VtxBandsTabs.h
../Release/VtxImageTabs.o: ../includes/SceneClass.h ../includes/LightClass.h
../Release/VtxImageTabs.o: ../includes/ObjectClass.h
../Release/VtxImageTabs.o: ../includes/OrbitalClass.h ../includes/Renderer.h
../Release/VtxImageTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxImageTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxImageTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxImageTabs.o: ../includes/FileUtil.h ../includes/RenderOptions.h
../Release/VtxLayerTabs.o: VtxLayerTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxLayerTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxLayerTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxLayerTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxLayerTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxLayerTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxLayerTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxLayerTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxLayerTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxLayerTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxLayerTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxLayerTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxLayerTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxLayerTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxLayerTabs.o: ../includes/Layers.h VtxSceneDialog.h
../Release/VtxMain.o: ../includes/GLglue.h ../includes/GLheaders.h VtxMain.h
../Release/VtxMain.o: VtxFrame.h VtxScene.h ../includes/FileUtil.h
../Release/VtxMain.o: ../includes/ListClass.h ../includes/defs.h
../Release/VtxMapTabs.o: VtxMapTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxMapTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxMapTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxMapTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxMapTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxMapTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxMapTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxMapTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxMapTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxMapTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxMapTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxMapTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxMapTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxMapTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxMapTabs.o: ../includes/Layers.h
../Release/VtxMoonTabs.o: VtxMoonTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxMoonTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxMoonTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxMoonTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxMoonTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxMoonTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxMoonTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxMoonTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxMoonTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxMoonTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxMoonTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxMoonTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxMoonTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxMoonTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxMoonTabs.o: ../includes/UniverseModel.h
../Release/VtxNoiseFunct.o: VtxNoiseFunct.h VtxFunctMgr.h VtxControls.h
../Release/VtxNoiseFunct.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxNoiseFunct.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxNoiseFunct.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxNoiseFunct.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxNoiseFunct.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxNoiseFunct.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxNoiseFunct.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxNoiseFunct.o: ../includes/TextureClass.h
../Release/VtxNoiseFunct.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxNoiseFunct.o: ../includes/TerrainNode.h
../Release/VtxNoiseFunct.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxNoiseFunct.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxNoiseFunct.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxNoiseFunct.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxNoiseFunct.o: VtxFunctDialog.h ../includes/TerrainClass.h
../Release/VtxPlanetTabs.o: VtxPlanetTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxPlanetTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxPlanetTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxPlanetTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxPlanetTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxPlanetTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxPlanetTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxPlanetTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxPlanetTabs.o: ../includes/TextureClass.h
../Release/VtxPlanetTabs.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxPlanetTabs.o: ../includes/TerrainNode.h
../Release/VtxPlanetTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxPlanetTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxPlanetTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxPlanetTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxPlanetTabs.o: ../includes/UniverseModel.h
../Release/VtxPointTabs.o: VtxPointTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxPointTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxPointTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxPointTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxPointTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxPointTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxPointTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxPointTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxPointTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxPointTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxPointTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxPointTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxPointTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxPointTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxPointTabs.o: ../includes/TerrainClass.h ../includes/FileUtil.h
../Release/VtxRingTabs.o: VtxRingTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxRingTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxRingTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxRingTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxRingTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxRingTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxRingTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxRingTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxRingTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxRingTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxRingTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxRingTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxRingTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxRingTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxRingTabs.o: ../includes/UniverseModel.h
../Release/VtxRocksTabs.o: VtxRocksTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxRocksTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxRocksTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxRocksTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxRocksTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxRocksTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxRocksTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxRocksTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxRocksTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxRocksTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxRocksTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxRocksTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxRocksTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxRocksTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxRocksTabs.o: ../includes/Rocks.h ../includes/Placements.h
../Release/VtxScene.o: ../includes/GLglue.h ../includes/GLheaders.h
../Release/VtxScene.o: ../includes/SceneClass.h ../includes/LightClass.h
../Release/VtxScene.o: ../includes/ColorClass.h ../includes/PointClass.h
../Release/VtxScene.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxScene.o: ../includes/ObjectClass.h ../includes/NodeIF.h
../Release/VtxScene.o: ../includes/OrbitalClass.h ../includes/TerrainMgr.h
../Release/VtxScene.o: ../includes/NoiseClass.h ../includes/TextureClass.h
../Release/VtxScene.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxScene.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Release/VtxScene.o: ../includes/Renderer.h ../includes/ViewClass.h
../Release/VtxScene.o: ../includes/Util.h ../includes/MatrixClass.h
../Release/VtxScene.o: ../includes/matrix.h ../includes/TrackBall.h
../Release/VtxScene.o: ../includes/ModelClass.h ../includes/UniverseModel.h
../Release/VtxScene.o: ../includes/TerrainClass.h ../includes/RenderOptions.h
../Release/VtxScene.o: ../includes/AdaptOptions.h ../includes/Effects.h
../Release/VtxScene.o: ../includes/Raster.h ../includes/KeyIF.h
../Release/VtxScene.o: ../includes/GLSLMgr.h ../includes/GLSLProgram.h
../Release/VtxScene.o: ../includes/GLSLVars.h ../includes/FrameBuffer.h
../Release/VtxScene.o: ../includes/FileUtil.h VtxScene.h ../includes/Bitmap.h
../Release/VtxScene.o: ../includes/FrameClass.h VtxSceneDialog.h
../Release/VtxScene.o: VtxControls.h VtxExprEdit.h VtxTabsMgr.h
../Release/VtxScene.o: VtxImageDialog.h VtxImageTabs.h
../Release/VtxScene.o: ../includes/ImageClass.h VtxImageWindow.h
../Release/VtxScene.o: VtxImportTabs.h VtxBandsTabs.h VtxFunctDialog.h
../Release/VtxScene.o: VtxFunctMgr.h VtxImageMgr.h
../Release/VtxSceneDialog.o: VtxSceneDialog.h VtxControls.h VtxExprEdit.h
../Release/VtxSceneDialog.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Release/VtxSceneDialog.o: ../includes/SceneClass.h
../Release/VtxSceneDialog.o: ../includes/LightClass.h
../Release/VtxSceneDialog.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxSceneDialog.o: ../includes/ListClass.h
../Release/VtxSceneDialog.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxSceneDialog.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxSceneDialog.o: ../includes/TerrainMgr.h
../Release/VtxSceneDialog.o: ../includes/NoiseClass.h
../Release/VtxSceneDialog.o: ../includes/TextureClass.h
../Release/VtxSceneDialog.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxSceneDialog.o: ../includes/TerrainNode.h
../Release/VtxSceneDialog.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxSceneDialog.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxSceneDialog.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxSceneDialog.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxSceneDialog.o: VtxTabsMgr.h ../includes/UniverseModel.h
../Release/VtxSceneDialog.o: ../includes/FileUtil.h VtxSceneTabs.h
../Release/VtxSceneDialog.o: ../includes/RenderOptions.h
../Release/VtxSceneDialog.o: ../includes/Effects.h ../includes/Raster.h
../Release/VtxSceneDialog.o: VtxPlanetTabs.h VtxMoonTabs.h VtxStarTabs.h
../Release/VtxSceneDialog.o: VtxCoronaTabs.h VtxTexTabs.h
../Release/VtxSceneDialog.o: ../includes/ImageClass.h VtxImageWindow.h
../Release/VtxSceneDialog.o: VtxDefaultTabs.h VtxCloudsTabs.h VtxColorTabs.h
../Release/VtxSceneDialog.o: ../includes/TerrainClass.h VtxSkyTabs.h
../Release/VtxSceneDialog.o: VtxPointTabs.h VtxGlossTabs.h VtxLayerTabs.h
../Release/VtxSceneDialog.o: ../includes/Layers.h VtxMapTabs.h VtxFogTabs.h
../Release/VtxSceneDialog.o: VtxWaterTabs.h VtxSurfaceTabs.h VtxRocksTabs.h
../Release/VtxSceneDialog.o: ../includes/Rocks.h ../includes/Placements.h
../Release/VtxSceneDialog.o: VtxRingTabs.h VtxSnowTabs.h VtxSystemTabs.h
../Release/VtxSceneDialog.o: VtxGalaxyTabs.h ../includes/Octree.h
../Release/VtxSceneTabs.o: VtxSceneTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxSceneTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxSceneTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxSceneTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxSceneTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxSceneTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxSceneTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxSceneTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxSceneTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxSceneTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxSceneTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxSceneTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxSceneTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxSceneTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxSceneTabs.o: ../includes/RenderOptions.h ../includes/Effects.h
../Release/VtxSceneTabs.o: ../includes/Raster.h ../includes/GLSLMgr.h
../Release/VtxSceneTabs.o: ../includes/GLSLProgram.h ../includes/GLSLVars.h
../Release/VtxSceneTabs.o: ../includes/FrameBuffer.h
../Release/VtxSnowTabs.o: VtxSnowTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxSnowTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxSnowTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxSnowTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxSnowTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxSnowTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxSnowTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxSnowTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxSnowTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxSnowTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxSnowTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxSnowTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxSnowTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxSnowTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxSnowTabs.o: ../includes/TerrainClass.h
../Release/VtxStarTabs.o: VtxStarTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxStarTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxStarTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxStarTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxStarTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxStarTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxStarTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxStarTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxStarTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxStarTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxStarTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxStarTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxStarTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxStarTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxSkyTabs.o: VtxSkyTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxSkyTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxSkyTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxSkyTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxSkyTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxSkyTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxSkyTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxSkyTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxSkyTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxSkyTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxSkyTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxSkyTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxSkyTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxSkyTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxSkyTabs.o: ../includes/RenderOptions.h
../Release/VtxSurfaceTabs.o: VtxSurfaceTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxSurfaceTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxSurfaceTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxSurfaceTabs.o: ../includes/LightClass.h
../Release/VtxSurfaceTabs.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxSurfaceTabs.o: ../includes/ListClass.h
../Release/VtxSurfaceTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxSurfaceTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxSurfaceTabs.o: ../includes/TerrainMgr.h
../Release/VtxSurfaceTabs.o: ../includes/NoiseClass.h
../Release/VtxSurfaceTabs.o: ../includes/TextureClass.h
../Release/VtxSurfaceTabs.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxSurfaceTabs.o: ../includes/TerrainNode.h
../Release/VtxSurfaceTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxSurfaceTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxSurfaceTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxSurfaceTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxSurfaceTabs.o: ../includes/RenderOptions.h VtxSceneDialog.h
../Release/VtxSystemTabs.o: VtxSystemTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxSystemTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxSystemTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxSystemTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxSystemTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxSystemTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxSystemTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxSystemTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxSystemTabs.o: ../includes/TextureClass.h
../Release/VtxSystemTabs.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxSystemTabs.o: ../includes/TerrainNode.h
../Release/VtxSystemTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxSystemTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxSystemTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxSystemTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxSystemTabs.o: ../includes/RenderOptions.h VtxSceneDialog.h
../Release/VtxTabsMgr.o: VtxTabsMgr.h VtxControls.h VtxExprEdit.h
../Release/VtxTabsMgr.o: ../includes/ColorClass.h ../includes/GLheaders.h
../Release/VtxTabsMgr.o: ../includes/SceneClass.h ../includes/LightClass.h
../Release/VtxTabsMgr.o: ../includes/PointClass.h ../includes/defs.h
../Release/VtxTabsMgr.o: ../includes/ListClass.h ../includes/ObjectClass.h
../Release/VtxTabsMgr.o: ../includes/GLglue.h ../includes/NodeIF.h
../Release/VtxTabsMgr.o: ../includes/OrbitalClass.h ../includes/TerrainMgr.h
../Release/VtxTabsMgr.o: ../includes/NoiseClass.h ../includes/TextureClass.h
../Release/VtxTabsMgr.o: ../includes/GLSupport.h ../includes/ImageMgr.h
../Release/VtxTabsMgr.o: ../includes/TerrainNode.h ../includes/TerrainData.h
../Release/VtxTabsMgr.o: ../includes/Renderer.h ../includes/ViewClass.h
../Release/VtxTabsMgr.o: ../includes/Util.h ../includes/MatrixClass.h
../Release/VtxTabsMgr.o: ../includes/matrix.h ../includes/TrackBall.h
../Release/VtxTabsMgr.o: ../includes/ModelClass.h VtxSceneDialog.h
../Release/VtxTexTabs.o: VtxTexTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxTexTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxTexTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxTexTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxTexTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxTexTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxTexTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxTexTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxTexTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxTexTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxTexTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxTexTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxTexTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxTexTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxTexTabs.o: ../includes/ImageClass.h VtxImageWindow.h
../Release/VtxTexTabs.o: VtxSceneDialog.h VtxImageDialog.h VtxImageTabs.h
../Release/VtxTexTabs.o: VtxImportTabs.h VtxBandsTabs.h
../Release/VtxTexTabs.o: ../includes/FileUtil.h
../Release/VtxWaterTabs.o: VtxWaterTabs.h VtxTabsMgr.h VtxControls.h
../Release/VtxWaterTabs.o: VtxExprEdit.h ../includes/ColorClass.h
../Release/VtxWaterTabs.o: ../includes/GLheaders.h ../includes/SceneClass.h
../Release/VtxWaterTabs.o: ../includes/LightClass.h ../includes/PointClass.h
../Release/VtxWaterTabs.o: ../includes/defs.h ../includes/ListClass.h
../Release/VtxWaterTabs.o: ../includes/ObjectClass.h ../includes/GLglue.h
../Release/VtxWaterTabs.o: ../includes/NodeIF.h ../includes/OrbitalClass.h
../Release/VtxWaterTabs.o: ../includes/TerrainMgr.h ../includes/NoiseClass.h
../Release/VtxWaterTabs.o: ../includes/TextureClass.h ../includes/GLSupport.h
../Release/VtxWaterTabs.o: ../includes/ImageMgr.h ../includes/TerrainNode.h
../Release/VtxWaterTabs.o: ../includes/TerrainData.h ../includes/Renderer.h
../Release/VtxWaterTabs.o: ../includes/ViewClass.h ../includes/Util.h
../Release/VtxWaterTabs.o: ../includes/MatrixClass.h ../includes/matrix.h
../Release/VtxWaterTabs.o: ../includes/TrackBall.h ../includes/ModelClass.h
../Release/VtxWaterTabs.o: ../includes/TerrainClass.h VtxSceneDialog.h
../Release/VtxWaterTabs.o: ../includes/RenderOptions.h
../Debug/VtxBandsTabs.o:	VtxBandsTabs.cpp
../Debug/VtxCloudsTabs.o:	VtxCloudsTabs.cpp
../Debug/VtxColorTabs.o:	VtxColorTabs.cpp
../Debug/VtxControls.o:	VtxControls.cpp
../Debug/VtxCoronaTabs.o:	VtxCoronaTabs.cpp
../Debug/VtxCratersFunct.o:	VtxCratersFunct.cpp
../Debug/VtxDefaultTabs.o:	VtxDefaultTabs.cpp
../Debug/VtxExprEdit.o:	VtxExprEdit.cpp
../Debug/VtxFogTabs.o:	VtxFogTabs.cpp
../Debug/VtxFrame.o:	VtxFrame.cpp
../Debug/VtxFractalFunct.o:	VtxFractalFunct.cpp
../Debug/VtxFunctDialog.o:	VtxFunctDialog.cpp
../Debug/VtxFunctMgr.o:	VtxFunctMgr.cpp
../Debug/VtxGalaxyTabs.o:	VtxGalaxyTabs.cpp
../Debug/VtxGlossTabs.o:	VtxGlossTabs.cpp
../Debug/VtxImageWindow.o:	VtxImageWindow.cpp
../Debug/VtxImportTabs.o:	VtxImportTabs.cpp
../Debug/VtxImageDialog.o:	VtxImageDialog.cpp
../Debug/VtxImageMgr.o:	VtxImageMgr.cpp
../Debug/VtxImageTabs.o:	VtxImageTabs.cpp
../Debug/VtxLayerTabs.o:	VtxLayerTabs.cpp
../Debug/VtxMain.o:	VtxMain.cpp
../Debug/VtxMapTabs.o:	VtxMapTabs.cpp
../Debug/VtxMoonTabs.o:	VtxMoonTabs.cpp
../Debug/VtxNoiseFunct.o:	VtxNoiseFunct.cpp
../Debug/VtxPlanetTabs.o:	VtxPlanetTabs.cpp
../Debug/VtxPointTabs.o:	VtxPointTabs.cpp
../Debug/VtxRingTabs.o:	VtxRingTabs.cpp
../Debug/VtxRocksTabs.o:	VtxRocksTabs.cpp
../Debug/VtxScene.o:	VtxScene.cpp
../Debug/VtxSceneDialog.o:	VtxSceneDialog.cpp
../Debug/VtxSceneTabs.o:	VtxSceneTabs.cpp
../Debug/VtxSnowTabs.o:	VtxSnowTabs.cpp
../Debug/VtxStarTabs.o:	VtxStarTabs.cpp
../Debug/VtxSkyTabs.o:	VtxSkyTabs.cpp
../Debug/VtxSurfaceTabs.o:	VtxSurfaceTabs.cpp
../Debug/VtxSystemTabs.o:	VtxSystemTabs.cpp
../Debug/VtxTabsMgr.o:	VtxTabsMgr.cpp
../Debug/VtxTexTabs.o:	VtxTexTabs.cpp
../Debug/VtxWaterTabs.o:	VtxWaterTabs.cpp
../Release/VtxBandsTabs.o: VtxBandsTabs.cpp
../Release/VtxCloudsTabs.o: VtxCloudsTabs.cpp
../Release/VtxColorTabs.o: VtxColorTabs.cpp
../Release/VtxControls.o: VtxControls.cpp
../Release/VtxCoronaTabs.o: VtxCoronaTabs.cpp
../Release/VtxCratersFunct.o: VtxCratersFunct.cpp
../Release/VtxDefaultTabs.o: VtxDefaultTabs.cpp
../Release/VtxErodeFunct.o: VtxErodeFunct.cpp
../Release/VtxExprEdit.o: VtxExprEdit.cpp
../Release/VtxFogTabs.o: VtxFogTabs.cpp
../Release/VtxFrame.o: VtxFrame.cpp
../Release/VtxFractalFunct.o: VtxFractalFunct.cpp
../Release/VtxFunctDialog.o: VtxFunctDialog.cpp
../Release/VtxFunctMgr.o: VtxFunctMgr.cpp
../Release/VtxGalaxyTabs.o: VtxGalaxyTabs.cpp
../Release/VtxGlossTabs.o: VtxGlossTabs.cpp
../Release/VtxImageWindow.o: VtxImageWindow.cpp
../Release/VtxImportTabs.o: VtxImportTabs.cpp
../Release/VtxImageDialog.o: VtxImageDialog.cpp
../Release/VtxImageMgr.o: VtxImageMgr.cpp
../Release/VtxImageTabs.o: VtxImageTabs.cpp
../Release/VtxLayerTabs.o: VtxLayerTabs.cpp
../Release/VtxMain.o: VtxMain.cpp
../Release/VtxMapTabs.o: VtxMapTabs.cpp
../Release/VtxMoonTabs.o: VtxMoonTabs.cpp
../Release/VtxNoiseFunct.o: VtxNoiseFunct.cpp
../Release/VtxPlanetTabs.o: VtxPlanetTabs.cpp
../Release/VtxPointTabs.o: VtxPointTabs.cpp
../Release/VtxRingTabs.o: VtxRingTabs.cpp
../Release/VtxRocksTabs.o: VtxRocksTabs.cpp
../Release/VtxScene.o: VtxScene.cpp
../Release/VtxSceneDialog.o: VtxSceneDialog.cpp
../Release/VtxSceneTabs.o: VtxSceneTabs.cpp
../Release/VtxSnowTabs.o: VtxSnowTabs.cpp
../Release/VtxStarTabs.o: VtxStarTabs.cpp
../Release/VtxSkyTabs.o: VtxSkyTabs.cpp
../Release/VtxSurfaceTabs.o: VtxSurfaceTabs.cpp
../Release/VtxSystemTabs.o: VtxSystemTabs.cpp
../Release/VtxTabsMgr.o: VtxTabsMgr.cpp
../Release/VtxTexTabs.o: VtxTexTabs.cpp
../Release/VtxWaterTabs.o: VtxWaterTabs.cpp
../Debug/VtxBandsTabs.o:	VtxBandsTabs.cpp
../Debug/VtxCloudsTabs.o:	VtxCloudsTabs.cpp
../Debug/VtxColorTabs.o:	VtxColorTabs.cpp
../Debug/VtxControls.o:	VtxControls.cpp
../Debug/VtxCoronaTabs.o:	VtxCoronaTabs.cpp
../Debug/VtxCratersFunct.o:	VtxCratersFunct.cpp
../Debug/VtxDefaultTabs.o:	VtxDefaultTabs.cpp
../Debug/VtxErodeFunct.o:	VtxErodeFunct.cpp
../Debug/VtxExprEdit.o:	VtxExprEdit.cpp
../Debug/VtxFogTabs.o:	VtxFogTabs.cpp
../Debug/VtxFrame.o:	VtxFrame.cpp
../Debug/VtxFractalFunct.o:	VtxFractalFunct.cpp
../Debug/VtxFunctDialog.o:	VtxFunctDialog.cpp
../Debug/VtxFunctMgr.o:	VtxFunctMgr.cpp
../Debug/VtxGalaxyTabs.o:	VtxGalaxyTabs.cpp
../Debug/VtxGlossTabs.o:	VtxGlossTabs.cpp
../Debug/VtxImageWindow.o:	VtxImageWindow.cpp
../Debug/VtxImportTabs.o:	VtxImportTabs.cpp
../Debug/VtxImageDialog.o:	VtxImageDialog.cpp
../Debug/VtxImageMgr.o:	VtxImageMgr.cpp
../Debug/VtxImageTabs.o:	VtxImageTabs.cpp
../Debug/VtxLayerTabs.o:	VtxLayerTabs.cpp
../Debug/VtxMain.o:	VtxMain.cpp
../Debug/VtxMapTabs.o:	VtxMapTabs.cpp
../Debug/VtxMoonTabs.o:	VtxMoonTabs.cpp
../Debug/VtxNoiseFunct.o:	VtxNoiseFunct.cpp
../Debug/VtxPlanetTabs.o:	VtxPlanetTabs.cpp
../Debug/VtxPointTabs.o:	VtxPointTabs.cpp
../Debug/VtxRingTabs.o:	VtxRingTabs.cpp
../Debug/VtxRocksTabs.o:	VtxRocksTabs.cpp
../Debug/VtxScene.o:	VtxScene.cpp
../Debug/VtxSceneDialog.o:	VtxSceneDialog.cpp
../Debug/VtxSceneTabs.o:	VtxSceneTabs.cpp
../Debug/VtxSnowTabs.o:	VtxSnowTabs.cpp
../Debug/VtxStarTabs.o:	VtxStarTabs.cpp
../Debug/VtxSkyTabs.o:	VtxSkyTabs.cpp
../Debug/VtxSurfaceTabs.o:	VtxSurfaceTabs.cpp
../Debug/VtxSystemTabs.o:	VtxSystemTabs.cpp
../Debug/VtxTabsMgr.o:	VtxTabsMgr.cpp
../Debug/VtxTexTabs.o:	VtxTexTabs.cpp
../Debug/VtxWaterTabs.o:	VtxWaterTabs.cpp
../Release/VtxBandsTabs.o: VtxBandsTabs.cpp
../Release/VtxCloudsTabs.o: VtxCloudsTabs.cpp
../Release/VtxColorTabs.o: VtxColorTabs.cpp
../Release/VtxControls.o: VtxControls.cpp
../Release/VtxCoronaTabs.o: VtxCoronaTabs.cpp
../Release/VtxCratersFunct.o: VtxCratersFunct.cpp
../Release/VtxDefaultTabs.o: VtxDefaultTabs.cpp
../Release/VtxErodeFunct.o: VtxErodeFunct.cpp
../Release/VtxExprEdit.o: VtxExprEdit.cpp
../Release/VtxFogTabs.o: VtxFogTabs.cpp
../Release/VtxFrame.o: VtxFrame.cpp
../Release/VtxFractalFunct.o: VtxFractalFunct.cpp
../Release/VtxFunctDialog.o: VtxFunctDialog.cpp
../Release/VtxFunctMgr.o: VtxFunctMgr.cpp
../Release/VtxGalaxyTabs.o: VtxGalaxyTabs.cpp
../Release/VtxGlossTabs.o: VtxGlossTabs.cpp
../Release/VtxImageWindow.o: VtxImageWindow.cpp
../Release/VtxImportTabs.o: VtxImportTabs.cpp
../Release/VtxImageDialog.o: VtxImageDialog.cpp
../Release/VtxImageMgr.o: VtxImageMgr.cpp
../Release/VtxImageTabs.o: VtxImageTabs.cpp
../Release/VtxLayerTabs.o: VtxLayerTabs.cpp
../Release/VtxMain.o: VtxMain.cpp
../Release/VtxMapTabs.o: VtxMapTabs.cpp
../Release/VtxMoonTabs.o: VtxMoonTabs.cpp
../Release/VtxNoiseFunct.o: VtxNoiseFunct.cpp
../Release/VtxPlanetTabs.o: VtxPlanetTabs.cpp
../Release/VtxPointTabs.o: VtxPointTabs.cpp
../Release/VtxRingTabs.o: VtxRingTabs.cpp
../Release/VtxRocksTabs.o: VtxRocksTabs.cpp
../Release/VtxScene.o: VtxScene.cpp
../Release/VtxSceneDialog.o: VtxSceneDialog.cpp
../Release/VtxSceneTabs.o: VtxSceneTabs.cpp
../Release/VtxSnowTabs.o: VtxSnowTabs.cpp
../Release/VtxStarTabs.o: VtxStarTabs.cpp
../Release/VtxSkyTabs.o: VtxSkyTabs.cpp
../Release/VtxSurfaceTabs.o: VtxSurfaceTabs.cpp
../Release/VtxSystemTabs.o: VtxSystemTabs.cpp
../Release/VtxTabsMgr.o: VtxTabsMgr.cpp
../Release/VtxTexTabs.o: VtxTexTabs.cpp
../Release/VtxWaterTabs.o: VtxWaterTabs.cpp
../Debug/VtxBandsTabs.o:	VtxBandsTabs.cpp
../Debug/VtxCloudsTabs.o:	VtxCloudsTabs.cpp
../Debug/VtxColorTabs.o:	VtxColorTabs.cpp
../Debug/VtxControls.o:	VtxControls.cpp
../Debug/VtxCoronaTabs.o:	VtxCoronaTabs.cpp
../Debug/VtxCratersFunct.o:	VtxCratersFunct.cpp
../Debug/VtxDefaultTabs.o:	VtxDefaultTabs.cpp
../Debug/VtxErodeFunct.o:	VtxErodeFunct.cpp
../Debug/VtxExprEdit.o:	VtxExprEdit.cpp
../Debug/VtxFogTabs.o:	VtxFogTabs.cpp
../Debug/VtxFrame.o:	VtxFrame.cpp
../Debug/VtxFractalFunct.o:	VtxFractalFunct.cpp
../Debug/VtxFunctDialog.o:	VtxFunctDialog.cpp
../Debug/VtxFunctMgr.o:	VtxFunctMgr.cpp
../Debug/VtxGalaxyTabs.o:	VtxGalaxyTabs.cpp
../Debug/VtxGlossTabs.o:	VtxGlossTabs.cpp
../Debug/VtxImageWindow.o:	VtxImageWindow.cpp
../Debug/VtxImportTabs.o:	VtxImportTabs.cpp
../Debug/VtxImageDialog.o:	VtxImageDialog.cpp
../Debug/VtxMovieDialog.o:	VtxMovieDialog.cpp
../Debug/VtxImageMgr.o:	VtxImageMgr.cpp
../Debug/VtxImageTabs.o:	VtxImageTabs.cpp
../Debug/VtxLayerTabs.o:	VtxLayerTabs.cpp
../Debug/VtxMain.o:	VtxMain.cpp
../Debug/VtxMapTabs.o:	VtxMapTabs.cpp
../Debug/VtxMoonTabs.o:	VtxMoonTabs.cpp
../Debug/VtxNoiseFunct.o:	VtxNoiseFunct.cpp
../Debug/VtxPlanetTabs.o:	VtxPlanetTabs.cpp
../Debug/VtxPointTabs.o:	VtxPointTabs.cpp
../Debug/VtxRingTabs.o:	VtxRingTabs.cpp
../Debug/VtxRocksTabs.o:	VtxRocksTabs.cpp
../Debug/VtxScene.o:	VtxScene.cpp
../Debug/VtxSceneDialog.o:	VtxSceneDialog.cpp
../Debug/VtxSceneTabs.o:	VtxSceneTabs.cpp
../Debug/VtxSnowTabs.o:	VtxSnowTabs.cpp
../Debug/VtxStarTabs.o:	VtxStarTabs.cpp
../Debug/VtxSkyTabs.o:	VtxSkyTabs.cpp
../Debug/VtxSurfaceTabs.o:	VtxSurfaceTabs.cpp
../Debug/VtxSystemTabs.o:	VtxSystemTabs.cpp
../Debug/VtxTabsMgr.o:	VtxTabsMgr.cpp
../Debug/VtxTexTabs.o:	VtxTexTabs.cpp
../Debug/VtxWaterTabs.o:	VtxWaterTabs.cpp
../Release/VtxBandsTabs.o: VtxBandsTabs.cpp
../Release/VtxCloudsTabs.o: VtxCloudsTabs.cpp
../Release/VtxColorTabs.o: VtxColorTabs.cpp
../Release/VtxControls.o: VtxControls.cpp
../Release/VtxCoronaTabs.o: VtxCoronaTabs.cpp
../Release/VtxCratersFunct.o: VtxCratersFunct.cpp
../Release/VtxDefaultTabs.o: VtxDefaultTabs.cpp
../Release/VtxErodeFunct.o: VtxErodeFunct.cpp
../Release/VtxExprEdit.o: VtxExprEdit.cpp
../Release/VtxFogTabs.o: VtxFogTabs.cpp
../Release/VtxFrame.o: VtxFrame.cpp
../Release/VtxFractalFunct.o: VtxFractalFunct.cpp
../Release/VtxFunctDialog.o: VtxFunctDialog.cpp
../Release/VtxFunctMgr.o: VtxFunctMgr.cpp
../Release/VtxGalaxyTabs.o: VtxGalaxyTabs.cpp
../Release/VtxGlossTabs.o: VtxGlossTabs.cpp
../Release/VtxImageWindow.o: VtxImageWindow.cpp
../Release/VtxImportTabs.o: VtxImportTabs.cpp
../Release/VtxImageDialog.o: VtxImageDialog.cpp
../Release/VtxMovieDialog.o: VtxMovieDialog.cpp
../Release/VtxImageMgr.o: VtxImageMgr.cpp
../Release/VtxImageTabs.o: VtxImageTabs.cpp
../Release/VtxLayerTabs.o: VtxLayerTabs.cpp
../Release/VtxMain.o: VtxMain.cpp
../Release/VtxMapTabs.o: VtxMapTabs.cpp
../Release/VtxMoonTabs.o: VtxMoonTabs.cpp
../Release/VtxNoiseFunct.o: VtxNoiseFunct.cpp
../Release/VtxPlanetTabs.o: VtxPlanetTabs.cpp
../Release/VtxPointTabs.o: VtxPointTabs.cpp
../Release/VtxRingTabs.o: VtxRingTabs.cpp
../Release/VtxRocksTabs.o: VtxRocksTabs.cpp
../Release/VtxScene.o: VtxScene.cpp
../Release/VtxSceneDialog.o: VtxSceneDialog.cpp
../Release/VtxSceneTabs.o: VtxSceneTabs.cpp
../Release/VtxSnowTabs.o: VtxSnowTabs.cpp
../Release/VtxStarTabs.o: VtxStarTabs.cpp
../Release/VtxSkyTabs.o: VtxSkyTabs.cpp
../Release/VtxSurfaceTabs.o: VtxSurfaceTabs.cpp
../Release/VtxSystemTabs.o: VtxSystemTabs.cpp
../Release/VtxTabsMgr.o: VtxTabsMgr.cpp
../Release/VtxTexTabs.o: VtxTexTabs.cpp
../Release/VtxWaterTabs.o: VtxWaterTabs.cpp
../Debug/VtxBandsTabs.o:	VtxBandsTabs.cpp
../Debug/VtxCloudsTabs.o:	VtxCloudsTabs.cpp
../Debug/VtxColorTabs.o:	VtxColorTabs.cpp
../Debug/VtxControls.o:	VtxControls.cpp
../Debug/VtxCoronaTabs.o:	VtxCoronaTabs.cpp
../Debug/VtxCratersFunct.o:	VtxCratersFunct.cpp
../Debug/VtxDefaultTabs.o:	VtxDefaultTabs.cpp
../Debug/VtxErodeFunct.o:	VtxErodeFunct.cpp
../Debug/VtxExprEdit.o:	VtxExprEdit.cpp
../Debug/VtxFogTabs.o:	VtxFogTabs.cpp
../Debug/VtxFrame.o:	VtxFrame.cpp
../Debug/VtxFractalFunct.o:	VtxFractalFunct.cpp
../Debug/VtxFunctDialog.o:	VtxFunctDialog.cpp
../Debug/VtxFunctMgr.o:	VtxFunctMgr.cpp
../Debug/VtxGalaxyTabs.o:	VtxGalaxyTabs.cpp
../Debug/VtxGlossTabs.o:	VtxGlossTabs.cpp
../Debug/VtxImageWindow.o:	VtxImageWindow.cpp
../Debug/VtxImportTabs.o:	VtxImportTabs.cpp
../Debug/VtxImageDialog.o:	VtxImageDialog.cpp
../Debug/VtxMovieDialog.o:	VtxMovieDialog.cpp
../Debug/VtxImageMgr.o:	VtxImageMgr.cpp
../Debug/VtxImageTabs.o:	VtxImageTabs.cpp
../Debug/VtxLayerTabs.o:	VtxLayerTabs.cpp
../Debug/VtxMain.o:	VtxMain.cpp
../Debug/VtxMapTabs.o:	VtxMapTabs.cpp
../Debug/VtxMoonTabs.o:	VtxMoonTabs.cpp
../Debug/VtxNoiseFunct.o:	VtxNoiseFunct.cpp
../Debug/VtxPlanetTabs.o:	VtxPlanetTabs.cpp
../Debug/VtxPointTabs.o:	VtxPointTabs.cpp
../Debug/VtxRingTabs.o:	VtxRingTabs.cpp
../Debug/VtxRocksTabs.o:	VtxRocksTabs.cpp
../Debug/VtxScene.o:	VtxScene.cpp
../Debug/VtxSceneDialog.o:	VtxSceneDialog.cpp
../Debug/VtxSceneTabs.o:	VtxSceneTabs.cpp
../Debug/VtxSnowTabs.o:	VtxSnowTabs.cpp
../Debug/VtxStarTabs.o:	VtxStarTabs.cpp
../Debug/VtxSkyTabs.o:	VtxSkyTabs.cpp
../Debug/VtxSurfaceTabs.o:	VtxSurfaceTabs.cpp
../Debug/VtxSystemTabs.o:	VtxSystemTabs.cpp
../Debug/VtxTabsMgr.o:	VtxTabsMgr.cpp
../Debug/VtxTexTabs.o:	VtxTexTabs.cpp
../Debug/VtxWaterTabs.o:	VtxWaterTabs.cpp
../Release/VtxBandsTabs.o: VtxBandsTabs.cpp
../Release/VtxCloudsTabs.o: VtxCloudsTabs.cpp
../Release/VtxColorTabs.o: VtxColorTabs.cpp
../Release/VtxControls.o: VtxControls.cpp
../Release/VtxCoronaTabs.o: VtxCoronaTabs.cpp
../Release/VtxCratersFunct.o: VtxCratersFunct.cpp
../Release/VtxDefaultTabs.o: VtxDefaultTabs.cpp
../Release/VtxErodeFunct.o: VtxErodeFunct.cpp
../Release/VtxExprEdit.o: VtxExprEdit.cpp
../Release/VtxFogTabs.o: VtxFogTabs.cpp
../Release/VtxFrame.o: VtxFrame.cpp
../Release/VtxFractalFunct.o: VtxFractalFunct.cpp
../Release/VtxFunctDialog.o: VtxFunctDialog.cpp
../Release/VtxFunctMgr.o: VtxFunctMgr.cpp
../Release/VtxGalaxyTabs.o: VtxGalaxyTabs.cpp
../Release/VtxGlossTabs.o: VtxGlossTabs.cpp
../Release/VtxImageWindow.o: VtxImageWindow.cpp
../Release/VtxImportTabs.o: VtxImportTabs.cpp
../Release/VtxImageDialog.o: VtxImageDialog.cpp
../Release/VtxMovieDialog.o: VtxMovieDialog.cpp
../Release/VtxImageMgr.o: VtxImageMgr.cpp
../Release/VtxImageTabs.o: VtxImageTabs.cpp
../Release/VtxLayerTabs.o: VtxLayerTabs.cpp
../Release/VtxMain.o: VtxMain.cpp
../Release/VtxMapTabs.o: VtxMapTabs.cpp
../Release/VtxMoonTabs.o: VtxMoonTabs.cpp
../Release/VtxNoiseFunct.o: VtxNoiseFunct.cpp
../Release/VtxPlanetTabs.o: VtxPlanetTabs.cpp
../Release/VtxPointTabs.o: VtxPointTabs.cpp
../Release/VtxRingTabs.o: VtxRingTabs.cpp
../Release/VtxRocksTabs.o: VtxRocksTabs.cpp
../Release/VtxScene.o: VtxScene.cpp
../Release/VtxSceneDialog.o: VtxSceneDialog.cpp
../Release/VtxSceneTabs.o: VtxSceneTabs.cpp
../Release/VtxSnowTabs.o: VtxSnowTabs.cpp
../Release/VtxStarTabs.o: VtxStarTabs.cpp
../Release/VtxSkyTabs.o: VtxSkyTabs.cpp
../Release/VtxSurfaceTabs.o: VtxSurfaceTabs.cpp
../Release/VtxSystemTabs.o: VtxSystemTabs.cpp
../Release/VtxTabsMgr.o: VtxTabsMgr.cpp
../Release/VtxTexTabs.o: VtxTexTabs.cpp
../Release/VtxWaterTabs.o: VtxWaterTabs.cpp
../Debug/VtxBandsTabs.o:	VtxBandsTabs.cpp
../Debug/VtxCloudsTabs.o:	VtxCloudsTabs.cpp
../Debug/VtxColorTabs.o:	VtxColorTabs.cpp
../Debug/VtxControls.o:	VtxControls.cpp
../Debug/VtxCoronaTabs.o:	VtxCoronaTabs.cpp
../Debug/VtxCratersFunct.o:	VtxCratersFunct.cpp
../Debug/VtxDefaultTabs.o:	VtxDefaultTabs.cpp
../Debug/VtxErodeFunct.o:	VtxErodeFunct.cpp
../Debug/VtxExprEdit.o:	VtxExprEdit.cpp
../Debug/VtxFogTabs.o:	VtxFogTabs.cpp
../Debug/VtxFrame.o:	VtxFrame.cpp
../Debug/VtxFractalFunct.o:	VtxFractalFunct.cpp
../Debug/VtxFunctDialog.o:	VtxFunctDialog.cpp
../Debug/VtxFunctMgr.o:	VtxFunctMgr.cpp
../Debug/VtxGalaxyTabs.o:	VtxGalaxyTabs.cpp
../Debug/VtxGlossTabs.o:	VtxGlossTabs.cpp
../Debug/VtxImageWindow.o:	VtxImageWindow.cpp
../Debug/VtxImportTabs.o:	VtxImportTabs.cpp
../Debug/VtxImageDialog.o:	VtxImageDialog.cpp
../Debug/VtxMovieDialog.o:	VtxMovieDialog.cpp
../Debug/VtxImageMgr.o:	VtxImageMgr.cpp
../Debug/VtxImageTabs.o:	VtxImageTabs.cpp
../Debug/VtxLayerTabs.o:	VtxLayerTabs.cpp
../Debug/VtxMain.o:	VtxMain.cpp
../Debug/VtxMapTabs.o:	VtxMapTabs.cpp
../Debug/VtxMoonTabs.o:	VtxMoonTabs.cpp
../Debug/VtxNoiseFunct.o:	VtxNoiseFunct.cpp
../Debug/VtxPlanetTabs.o:	VtxPlanetTabs.cpp
../Debug/VtxPointTabs.o:	VtxPointTabs.cpp
../Debug/VtxRingTabs.o:	VtxRingTabs.cpp
../Debug/VtxRocksTabs.o:	VtxRocksTabs.cpp
../Debug/VtxScene.o:	VtxScene.cpp
../Debug/VtxSceneDialog.o:	VtxSceneDialog.cpp
../Debug/VtxSceneTabs.o:	VtxSceneTabs.cpp
../Debug/VtxSnowTabs.o:	VtxSnowTabs.cpp
../Debug/VtxStarTabs.o:	VtxStarTabs.cpp
../Debug/VtxSkyTabs.o:	VtxSkyTabs.cpp
../Debug/VtxSurfaceTabs.o:	VtxSurfaceTabs.cpp
../Debug/VtxSystemTabs.o:	VtxSystemTabs.cpp
../Debug/VtxTabsMgr.o:	VtxTabsMgr.cpp
../Debug/VtxTexTabs.o:	VtxTexTabs.cpp
../Debug/VtxWaterTabs.o:	VtxWaterTabs.cpp
../Release/VtxBandsTabs.o: VtxBandsTabs.cpp
../Release/VtxCloudsTabs.o: VtxCloudsTabs.cpp
../Release/VtxColorTabs.o: VtxColorTabs.cpp
../Release/VtxControls.o: VtxControls.cpp
../Release/VtxCoronaTabs.o: VtxCoronaTabs.cpp
../Release/VtxCratersFunct.o: VtxCratersFunct.cpp
../Release/VtxDefaultTabs.o: VtxDefaultTabs.cpp
../Release/VtxErodeFunct.o: VtxErodeFunct.cpp
../Release/VtxExprEdit.o: VtxExprEdit.cpp
../Release/VtxFogTabs.o: VtxFogTabs.cpp
../Release/VtxFrame.o: VtxFrame.cpp
../Release/VtxFractalFunct.o: VtxFractalFunct.cpp
../Release/VtxFunctDialog.o: VtxFunctDialog.cpp
../Release/VtxFunctMgr.o: VtxFunctMgr.cpp
../Release/VtxGalaxyTabs.o: VtxGalaxyTabs.cpp
../Release/VtxGlossTabs.o: VtxGlossTabs.cpp
../Release/VtxImageWindow.o: VtxImageWindow.cpp
../Release/VtxImportTabs.o: VtxImportTabs.cpp
../Release/VtxImageDialog.o: VtxImageDialog.cpp
../Release/VtxMovieDialog.o: VtxMovieDialog.cpp
../Release/VtxImageMgr.o: VtxImageMgr.cpp
../Release/VtxImageTabs.o: VtxImageTabs.cpp
../Release/VtxLayerTabs.o: VtxLayerTabs.cpp
../Release/VtxMain.o: VtxMain.cpp
../Release/VtxMapTabs.o: VtxMapTabs.cpp
../Release/VtxMoonTabs.o: VtxMoonTabs.cpp
../Release/VtxNoiseFunct.o: VtxNoiseFunct.cpp
../Release/VtxPlanetTabs.o: VtxPlanetTabs.cpp
../Release/VtxPointTabs.o: VtxPointTabs.cpp
../Release/VtxRingTabs.o: VtxRingTabs.cpp
../Release/VtxRocksTabs.o: VtxRocksTabs.cpp
../Release/VtxScene.o: VtxScene.cpp
../Release/VtxSceneDialog.o: VtxSceneDialog.cpp
../Release/VtxSceneTabs.o: VtxSceneTabs.cpp
../Release/VtxSnowTabs.o: VtxSnowTabs.cpp
../Release/VtxStarTabs.o: VtxStarTabs.cpp
../Release/VtxSkyTabs.o: VtxSkyTabs.cpp
../Release/VtxSurfaceTabs.o: VtxSurfaceTabs.cpp
../Release/VtxSystemTabs.o: VtxSystemTabs.cpp
../Release/VtxTabsMgr.o: VtxTabsMgr.cpp
../Release/VtxTexTabs.o: VtxTexTabs.cpp
../Release/VtxWaterTabs.o: VtxWaterTabs.cpp
../Debug/VtxBandsTabs.o:	VtxBandsTabs.cpp
../Debug/VtxCloudsTabs.o:	VtxCloudsTabs.cpp
../Debug/VtxColorTabs.o:	VtxColorTabs.cpp
../Debug/VtxControls.o:	VtxControls.cpp
../Debug/VtxCoronaTabs.o:	VtxCoronaTabs.cpp
../Debug/VtxCratersFunct.o:	VtxCratersFunct.cpp
../Debug/VtxDefaultTabs.o:	VtxDefaultTabs.cpp
../Debug/VtxErodeFunct.o:	VtxErodeFunct.cpp
../Debug/VtxExprEdit.o:	VtxExprEdit.cpp
../Debug/VtxFogTabs.o:	VtxFogTabs.cpp
../Debug/VtxFrame.o:	VtxFrame.cpp
../Debug/VtxFractalFunct.o:	VtxFractalFunct.cpp
../Debug/VtxFunctDialog.o:	VtxFunctDialog.cpp
../Debug/VtxFunctMgr.o:	VtxFunctMgr.cpp
../Debug/VtxGalaxyTabs.o:	VtxGalaxyTabs.cpp
../Debug/VtxGlossTabs.o:	VtxGlossTabs.cpp
../Debug/VtxImageWindow.o:	VtxImageWindow.cpp
../Debug/VtxImportTabs.o:	VtxImportTabs.cpp
../Debug/VtxImageDialog.o:	VtxImageDialog.cpp
../Debug/VtxMovieDialog.o:	VtxMovieDialog.cpp
../Debug/VtxImageMgr.o:	VtxImageMgr.cpp
../Debug/VtxImageTabs.o:	VtxImageTabs.cpp
../Debug/VtxLayerTabs.o:	VtxLayerTabs.cpp
../Debug/VtxMain.o:	VtxMain.cpp
../Debug/VtxMapTabs.o:	VtxMapTabs.cpp
../Debug/VtxMoonTabs.o:	VtxMoonTabs.cpp
../Debug/VtxNoiseFunct.o:	VtxNoiseFunct.cpp
../Debug/VtxPlanetTabs.o:	VtxPlanetTabs.cpp
../Debug/VtxPointTabs.o:	VtxPointTabs.cpp
../Debug/VtxRingTabs.o:	VtxRingTabs.cpp
../Debug/VtxRocksTabs.o:	VtxRocksTabs.cpp
../Debug/VtxScene.o:	VtxScene.cpp
../Debug/VtxSceneDialog.o:	VtxSceneDialog.cpp
../Debug/VtxSceneTabs.o:	VtxSceneTabs.cpp
../Debug/VtxSnowTabs.o:	VtxSnowTabs.cpp
../Debug/VtxStarTabs.o:	VtxStarTabs.cpp
../Debug/VtxSkyTabs.o:	VtxSkyTabs.cpp
../Debug/VtxSurfaceTabs.o:	VtxSurfaceTabs.cpp
../Debug/VtxSystemTabs.o:	VtxSystemTabs.cpp
../Debug/VtxTabsMgr.o:	VtxTabsMgr.cpp
../Debug/VtxTexTabs.o:	VtxTexTabs.cpp
../Debug/VtxWaterTabs.o:	VtxWaterTabs.cpp
../Debug/VtxBitmapPanel.o:	VtxBitmapPanel.cpp
../Release/VtxBandsTabs.o: VtxBandsTabs.cpp
../Release/VtxCloudsTabs.o: VtxCloudsTabs.cpp
../Release/VtxColorTabs.o: VtxColorTabs.cpp
../Release/VtxControls.o: VtxControls.cpp
../Release/VtxCoronaTabs.o: VtxCoronaTabs.cpp
../Release/VtxCratersFunct.o: VtxCratersFunct.cpp
../Release/VtxDefaultTabs.o: VtxDefaultTabs.cpp
../Release/VtxErodeFunct.o: VtxErodeFunct.cpp
../Release/VtxExprEdit.o: VtxExprEdit.cpp
../Release/VtxFogTabs.o: VtxFogTabs.cpp
../Release/VtxFrame.o: VtxFrame.cpp
../Release/VtxFractalFunct.o: VtxFractalFunct.cpp
../Release/VtxFunctDialog.o: VtxFunctDialog.cpp
../Release/VtxFunctMgr.o: VtxFunctMgr.cpp
../Release/VtxGalaxyTabs.o: VtxGalaxyTabs.cpp
../Release/VtxGlossTabs.o: VtxGlossTabs.cpp
../Release/VtxImageWindow.o: VtxImageWindow.cpp
../Release/VtxImportTabs.o: VtxImportTabs.cpp
../Release/VtxImageDialog.o: VtxImageDialog.cpp
../Release/VtxMovieDialog.o: VtxMovieDialog.cpp
../Release/VtxImageMgr.o: VtxImageMgr.cpp
../Release/VtxImageTabs.o: VtxImageTabs.cpp
../Release/VtxLayerTabs.o: VtxLayerTabs.cpp
../Release/VtxMain.o: VtxMain.cpp
../Release/VtxMapTabs.o: VtxMapTabs.cpp
../Release/VtxMoonTabs.o: VtxMoonTabs.cpp
../Release/VtxNoiseFunct.o: VtxNoiseFunct.cpp
../Release/VtxPlanetTabs.o: VtxPlanetTabs.cpp
../Release/VtxPointTabs.o: VtxPointTabs.cpp
../Release/VtxRingTabs.o: VtxRingTabs.cpp
../Release/VtxRocksTabs.o: VtxRocksTabs.cpp
../Release/VtxScene.o: VtxScene.cpp
../Release/VtxSceneDialog.o: VtxSceneDialog.cpp
../Release/VtxSceneTabs.o: VtxSceneTabs.cpp
../Release/VtxSnowTabs.o: VtxSnowTabs.cpp
../Release/VtxStarTabs.o: VtxStarTabs.cpp
../Release/VtxSkyTabs.o: VtxSkyTabs.cpp
../Release/VtxSurfaceTabs.o: VtxSurfaceTabs.cpp
../Release/VtxSystemTabs.o: VtxSystemTabs.cpp
../Release/VtxTabsMgr.o: VtxTabsMgr.cpp
../Release/VtxTexTabs.o: VtxTexTabs.cpp
../Release/VtxWaterTabs.o: VtxWaterTabs.cpp
../Release/VtxBitmapPanel.o: VtxBitmapPanel.cpp
../Debug/VtxBandsTabs.o:	VtxBandsTabs.cpp
../Debug/VtxCloudsTabs.o:	VtxCloudsTabs.cpp
../Debug/VtxColorTabs.o:	VtxColorTabs.cpp
../Debug/VtxControls.o:	VtxControls.cpp
../Debug/VtxCoronaTabs.o:	VtxCoronaTabs.cpp
../Debug/VtxCratersFunct.o:	VtxCratersFunct.cpp
../Debug/VtxDefaultTabs.o:	VtxDefaultTabs.cpp
../Debug/VtxErodeFunct.o:	VtxErodeFunct.cpp
../Debug/VtxExprEdit.o:	VtxExprEdit.cpp
../Debug/VtxFogTabs.o:	VtxFogTabs.cpp
../Debug/VtxFrame.o:	VtxFrame.cpp
../Debug/VtxFractalFunct.o:	VtxFractalFunct.cpp
../Debug/VtxFunctDialog.o:	VtxFunctDialog.cpp
../Debug/VtxFunctMgr.o:	VtxFunctMgr.cpp
../Debug/VtxGalaxyTabs.o:	VtxGalaxyTabs.cpp
../Debug/VtxGlossTabs.o:	VtxGlossTabs.cpp
../Debug/VtxImageWindow.o:	VtxImageWindow.cpp
../Debug/VtxImportTabs.o:	VtxImportTabs.cpp
../Debug/VtxImageDialog.o:	VtxImageDialog.cpp
../Debug/VtxMovieDialog.o:	VtxMovieDialog.cpp
../Debug/VtxImageMgr.o:	VtxImageMgr.cpp
../Debug/VtxImageTabs.o:	VtxImageTabs.cpp
../Debug/VtxLayerTabs.o:	VtxLayerTabs.cpp
../Debug/VtxMain.o:	VtxMain.cpp
../Debug/VtxMapTabs.o:	VtxMapTabs.cpp
../Debug/VtxMoonTabs.o:	VtxMoonTabs.cpp
../Debug/VtxNoiseFunct.o:	VtxNoiseFunct.cpp
../Debug/VtxPlanetTabs.o:	VtxPlanetTabs.cpp
../Debug/VtxPointTabs.o:	VtxPointTabs.cpp
../Debug/VtxRingTabs.o:	VtxRingTabs.cpp
../Debug/VtxRocksTabs.o:	VtxRocksTabs.cpp
../Debug/VtxScene.o:	VtxScene.cpp
../Debug/VtxSceneDialog.o:	VtxSceneDialog.cpp
../Debug/VtxSceneTabs.o:	VtxSceneTabs.cpp
../Debug/VtxSnowTabs.o:	VtxSnowTabs.cpp
../Debug/VtxStarTabs.o:	VtxStarTabs.cpp
../Debug/VtxSkyTabs.o:	VtxSkyTabs.cpp
../Debug/VtxSurfaceTabs.o:	VtxSurfaceTabs.cpp
../Debug/VtxSystemTabs.o:	VtxSystemTabs.cpp
../Debug/VtxTabsMgr.o:	VtxTabsMgr.cpp
../Debug/VtxTexTabs.o:	VtxTexTabs.cpp
../Debug/VtxBitmapPanel.o:	VtxBitmapPanel.cpp
../Debug/VtxWaterTabs.o:	VtxWaterTabs.cpp
../Release/VtxBandsTabs.o: VtxBandsTabs.cpp
../Release/VtxCloudsTabs.o: VtxCloudsTabs.cpp
../Release/VtxColorTabs.o: VtxColorTabs.cpp
../Release/VtxControls.o: VtxControls.cpp
../Release/VtxCoronaTabs.o: VtxCoronaTabs.cpp
../Release/VtxCratersFunct.o: VtxCratersFunct.cpp
../Release/VtxDefaultTabs.o: VtxDefaultTabs.cpp
../Release/VtxErodeFunct.o: VtxErodeFunct.cpp
../Release/VtxExprEdit.o: VtxExprEdit.cpp
../Release/VtxFogTabs.o: VtxFogTabs.cpp
../Release/VtxFrame.o: VtxFrame.cpp
../Release/VtxFractalFunct.o: VtxFractalFunct.cpp
../Release/VtxFunctDialog.o: VtxFunctDialog.cpp
../Release/VtxFunctMgr.o: VtxFunctMgr.cpp
../Release/VtxGalaxyTabs.o: VtxGalaxyTabs.cpp
../Release/VtxGlossTabs.o: VtxGlossTabs.cpp
../Release/VtxImageWindow.o: VtxImageWindow.cpp
../Release/VtxImportTabs.o: VtxImportTabs.cpp
../Release/VtxImageDialog.o: VtxImageDialog.cpp
../Release/VtxMovieDialog.o: VtxMovieDialog.cpp
../Release/VtxImageMgr.o: VtxImageMgr.cpp
../Release/VtxImageTabs.o: VtxImageTabs.cpp
../Release/VtxLayerTabs.o: VtxLayerTabs.cpp
../Release/VtxMain.o: VtxMain.cpp
../Release/VtxMapTabs.o: VtxMapTabs.cpp
../Release/VtxMoonTabs.o: VtxMoonTabs.cpp
../Release/VtxNoiseFunct.o: VtxNoiseFunct.cpp
../Release/VtxPlanetTabs.o: VtxPlanetTabs.cpp
../Release/VtxPointTabs.o: VtxPointTabs.cpp
../Release/VtxRingTabs.o: VtxRingTabs.cpp
../Release/VtxRocksTabs.o: VtxRocksTabs.cpp
../Release/VtxScene.o: VtxScene.cpp
../Release/VtxSceneDialog.o: VtxSceneDialog.cpp
../Release/VtxSceneTabs.o: VtxSceneTabs.cpp
../Release/VtxSnowTabs.o: VtxSnowTabs.cpp
../Release/VtxStarTabs.o: VtxStarTabs.cpp
../Release/VtxSkyTabs.o: VtxSkyTabs.cpp
../Release/VtxSurfaceTabs.o: VtxSurfaceTabs.cpp
../Release/VtxSystemTabs.o: VtxSystemTabs.cpp
../Release/VtxTabsMgr.o: VtxTabsMgr.cpp
../Release/VtxTexTabs.o: VtxTexTabs.cpp
../Release/VtxBitmapPanel.o: VtxBitmapPanel.cpp
../Release/VtxWaterTabs.o: VtxWaterTabs.cpp
../Debug/VtxBandsTabs.o:	VtxBandsTabs.cpp
../Debug/VtxCloudsTabs.o:	VtxCloudsTabs.cpp
../Debug/VtxColorTabs.o:	VtxColorTabs.cpp
../Debug/VtxControls.o:	VtxControls.cpp
../Debug/VtxCoronaTabs.o:	VtxCoronaTabs.cpp
../Debug/VtxCratersFunct.o:	VtxCratersFunct.cpp
../Debug/VtxDefaultTabs.o:	VtxDefaultTabs.cpp
../Debug/VtxErodeFunct.o:	VtxErodeFunct.cpp
../Debug/VtxExprEdit.o:	VtxExprEdit.cpp
../Debug/VtxFogTabs.o:	VtxFogTabs.cpp
../Debug/VtxFrame.o:	VtxFrame.cpp
../Debug/VtxFractalFunct.o:	VtxFractalFunct.cpp
../Debug/VtxFractalTabs.o:	VtxFractalTabs.cpp
../Debug/VtxFunctDialog.o:	VtxFunctDialog.cpp
../Debug/VtxFunctMgr.o:	VtxFunctMgr.cpp
../Debug/VtxGalaxyTabs.o:	VtxGalaxyTabs.cpp
../Debug/VtxGlossTabs.o:	VtxGlossTabs.cpp
../Debug/VtxImageWindow.o:	VtxImageWindow.cpp
../Debug/VtxImportTabs.o:	VtxImportTabs.cpp
../Debug/VtxImageDialog.o:	VtxImageDialog.cpp
../Debug/VtxMovieDialog.o:	VtxMovieDialog.cpp
../Debug/VtxImageMgr.o:	VtxImageMgr.cpp
../Debug/VtxImageTabs.o:	VtxImageTabs.cpp
../Debug/VtxLayerTabs.o:	VtxLayerTabs.cpp
../Debug/VtxMain.o:	VtxMain.cpp
../Debug/VtxMapTabs.o:	VtxMapTabs.cpp
../Debug/VtxMoonTabs.o:	VtxMoonTabs.cpp
../Debug/VtxNoiseFunct.o:	VtxNoiseFunct.cpp
../Debug/VtxPlanetTabs.o:	VtxPlanetTabs.cpp
../Debug/VtxPointTabs.o:	VtxPointTabs.cpp
../Debug/VtxRingTabs.o:	VtxRingTabs.cpp
../Debug/VtxRocksTabs.o:	VtxRocksTabs.cpp
../Debug/VtxScene.o:	VtxScene.cpp
../Debug/VtxSceneDialog.o:	VtxSceneDialog.cpp
../Debug/VtxSceneTabs.o:	VtxSceneTabs.cpp
../Debug/VtxSnowTabs.o:	VtxSnowTabs.cpp
../Debug/VtxStarTabs.o:	VtxStarTabs.cpp
../Debug/VtxSkyTabs.o:	VtxSkyTabs.cpp
../Debug/VtxSurfaceTabs.o:	VtxSurfaceTabs.cpp
../Debug/VtxSystemTabs.o:	VtxSystemTabs.cpp
../Debug/VtxTabsMgr.o:	VtxTabsMgr.cpp
../Debug/VtxTexTabs.o:	VtxTexTabs.cpp
../Debug/VtxBitmapPanel.o:	VtxBitmapPanel.cpp
../Debug/VtxWaterTabs.o:	VtxWaterTabs.cpp
../Release/VtxBandsTabs.o: VtxBandsTabs.cpp
../Release/VtxCloudsTabs.o: VtxCloudsTabs.cpp
../Release/VtxColorTabs.o: VtxColorTabs.cpp
../Release/VtxControls.o: VtxControls.cpp
../Release/VtxCoronaTabs.o: VtxCoronaTabs.cpp
../Release/VtxCratersFunct.o: VtxCratersFunct.cpp
../Release/VtxDefaultTabs.o: VtxDefaultTabs.cpp
../Release/VtxErodeFunct.o: VtxErodeFunct.cpp
../Release/VtxExprEdit.o: VtxExprEdit.cpp
../Release/VtxFogTabs.o: VtxFogTabs.cpp
../Release/VtxFrame.o: VtxFrame.cpp
../Release/VtxFractalFunct.o: VtxFractalFunct.cpp
../Release/VtxFractalTabs.o: VtxFractalTabs.cpp
../Release/VtxFunctDialog.o: VtxFunctDialog.cpp
../Release/VtxFunctMgr.o: VtxFunctMgr.cpp
../Release/VtxGalaxyTabs.o: VtxGalaxyTabs.cpp
../Release/VtxGlossTabs.o: VtxGlossTabs.cpp
../Release/VtxImageWindow.o: VtxImageWindow.cpp
../Release/VtxImportTabs.o: VtxImportTabs.cpp
../Release/VtxImageDialog.o: VtxImageDialog.cpp
../Release/VtxMovieDialog.o: VtxMovieDialog.cpp
../Release/VtxImageMgr.o: VtxImageMgr.cpp
../Release/VtxImageTabs.o: VtxImageTabs.cpp
../Release/VtxLayerTabs.o: VtxLayerTabs.cpp
../Release/VtxMain.o: VtxMain.cpp
../Release/VtxMapTabs.o: VtxMapTabs.cpp
../Release/VtxMoonTabs.o: VtxMoonTabs.cpp
../Release/VtxNoiseFunct.o: VtxNoiseFunct.cpp
../Release/VtxPlanetTabs.o: VtxPlanetTabs.cpp
../Release/VtxPointTabs.o: VtxPointTabs.cpp
../Release/VtxRingTabs.o: VtxRingTabs.cpp
../Release/VtxRocksTabs.o: VtxRocksTabs.cpp
../Release/VtxScene.o: VtxScene.cpp
../Release/VtxSceneDialog.o: VtxSceneDialog.cpp
../Release/VtxSceneTabs.o: VtxSceneTabs.cpp
../Release/VtxSnowTabs.o: VtxSnowTabs.cpp
../Release/VtxStarTabs.o: VtxStarTabs.cpp
../Release/VtxSkyTabs.o: VtxSkyTabs.cpp
../Release/VtxSurfaceTabs.o: VtxSurfaceTabs.cpp
../Release/VtxSystemTabs.o: VtxSystemTabs.cpp
../Release/VtxTabsMgr.o: VtxTabsMgr.cpp
../Release/VtxTexTabs.o: VtxTexTabs.cpp
../Release/VtxBitmapPanel.o: VtxBitmapPanel.cpp
../Release/VtxWaterTabs.o: VtxWaterTabs.cpp
