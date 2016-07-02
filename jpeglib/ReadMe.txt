

/////////////////////////////////////////////////////////////////////////////
Jpeg support
/////////////////////////////////////////////////////////////////////////////

Jtag compression and decompression routines are contained in a static library file:

jpeg.lib

1. to build the library:

   - run the Visual C++ project file (jtagproject.dsw) in the jtag directory
     this will create jtag.lib

2. to include the library in a project
   - create the following directory:

     C:\Program Files\Microsoft Visual Studio\VC98\Include\jpeg

   - copy the following include files from .\jpeg to ..\VC98\Include\jpeg 
     jinclude.h
	 jerror.h
	 jmorecfg.h
	 jpeglib.h

   - copy jtag.lib to C:\Program Files\Microsoft Visual Studio\VC98\LIB 
   
    - in project/settings/link/general        
        add  jpeg.lib to "Object/Library modules" list:
		
		e.g. opengl32.lib glu32.lib glaux.lib jpeg.lib

3. Application

   - library is added to application code (increases code size 216 KBytes)

     
