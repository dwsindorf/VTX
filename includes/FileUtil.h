
//  FileUtil.h

#ifndef _FILEUTIL
#define _FILEUTIL

#include "ListClass.h"
class ModelSym;

#ifndef MAXSTR
#define MAXSTR 1024
#endif
enum {
   FILE_JPG = 0,
   FILE_BMP = 1,
   FILE_MP4 = 2,
   FILE_AVI = 3
};

class FileUtil
{
public:
    static char *ext;
    static char *separator;
	static char *basename;
	static char *system;
	static char *saves;
	static char *objects;
	static char *images;
	static char *movies;
	static char *bitmaps;
	static char *shaders;

	static void getSystemDir(char *dir);
	static void getSavesDir(char *dir);
	static void getObjectsDir(char *dir);
	static void getImagesDir(char *dir);
	static void getMoviesDir(char *dir);
	static void getBitmapsDir(char *dir);
	static void getShadersDir(char *dir);

	static void getCurrentDirectory(char *dir);
	static bool setCurrentDirectory(char *dir);
	static void setBaseName(char *s);
	static char *getBaseName();
	static void addToPath(char *pdir, char *cdir);
	static void getBaseDirectory(char *dir);
	static int getSubDirectory(char *pdir, char *sdir, char *buff);
	static int makeSubDirectory(char *pdir, char *sdir, char *buff);
	static void getParentDirectory(char *s,char *d);
	static void getFileNameList(char *dir,const char *ext,LinkedList<ModelSym*>&list);
	static void getDirectoryList(char *dir,LinkedList<ModelSym*>&list);
	static int isEmptyDirectory(char *dir);
	static void getFileName(char *s,char *d);
	static void getFilePath(char *dir,char *path);
	static int deleteFile(char *s);
	static int deleteDirectory(char *p);
	static int deleteDirectoryFiles(char *p);
	static int deleteDirectoryFiles(char *p,char *type);
	static int fileExists(char *f);
	static int fileExists(char *p,char *f);
	static FILE *readFile(char *p, char *f);
	static FILE *writeFile(char *p, char *f);
	static int moveFile(char *p, char *f);
	static void makeFilePath(char *dir, char *fn, char *path);
};

extern FileUtil File;

#endif
