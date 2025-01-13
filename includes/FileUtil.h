
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

class FileData {
	char   _name[256];
	char   _path[1024];
	long   _hash_name;
	void getHashName(){
		_hash_name = 0;
		for (size_t i = 0; i < strlen(_name); ++i)
			_hash_name += (_name[i] - '0') * static_cast<int>(i + 1);
	}

public:
	FileData(const char *n) {
		strcpy(_name,n);
		_path[0]=0;
		getHashName();
	}
	FileData(const char *n, const char *p) {
		strcpy(_name,n);
		strcpy(_path,p);
		getHashName();
	}
	double value(){return _hash_name;}
	int isFile() { return _path[0];}
	char *name() { return _name;}
	char *path() { return _path;}
	void setPath(char *p) {strcpy(_path,p);}
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
	static char *movies;
	static char *bitmaps;
	static char *shaders;
	static char *textures;
	static char *images;
	static char *imports;
	static char *hmaps;
	static char *maps;
	static char *sprites;
	static char *plants;
	static char *branches;
	static char *leaves;

	static void getSystemDir(char *dir);
	static void getSavesDir(char *dir);
	static void getObjectsDir(char *dir);
	static void getMoviesDir(char *dir);
	static void getBitmapsDir(char *dir);
	static void getShadersDir(char *dir);
	static void getImportsDir(char *dir);
	static void getHmapsDir(char *dir);
	static void getMapsDir(char *dir);
	static void getSpritesDir(char *dir);
	static void getBranchesDir(char *dir);
	static void getLeavesDir(char *dir);

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
	static void getFileNameList(char *dir,const char *ext,NameList<ModelSym*>&list);
	static void getDirectoryList(char *dir,ValueList<FileData*>&list);

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
