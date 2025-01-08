//
// FileUtil.cpp : platform dependent file operations
//
//************************************************************
// Windows implementation
//************************************************************
#ifdef WIN32
#include <windows.h>
#include "ModelClass.h"
#include "FileUtil.h"
#include <io.h>
// Windows file separator

char *FileUtil::separator="\\";
char *FileUtil::basename=(char*)"vtx.exe";

#else // assume Unix etc.

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include <fstream.h>
#include <dirent.h>
#include "ModelClass.h"
#include "FileUtil.h"
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

// Unix file separator

char *FileUtil::separator=(char*)"/";
char *FileUtil::basename=(char*)"vtx";

#endif

FileUtil File;

char *FileUtil::ext=(char*)".spx";
char *FileUtil::system=(char*)"System";
char *FileUtil::saves=(char*)"Saves";
char *FileUtil::objects=(char*)"Objects";
char *FileUtil::movies=(char*)"Movies";
char *FileUtil::bitmaps=(char*)"Bitmaps";
char *FileUtil::shaders=(char*)"Shaders";
char *FileUtil::textures=(char*)"Textures";
char *FileUtil::images=(char*)"Images";
char *FileUtil::maps=(char*)"Maps";
char *FileUtil::sprites=(char*)"Sprites";
char *FileUtil::plants=(char*)"Plants";
char *FileUtil::branches=(char*)"Branch";
char *FileUtil::leaves=(char*)"Leaf";
char *FileUtil::tmp=(char*)"tmp";
char *FileUtil::imports=(char*)"Images";
char *FileUtil::hmaps=(char*)"Hmaps";

//-------------------------------------------------------------
// FileUtil::FileUtil() constructor
//-------------------------------------------------------------

void FileUtil::setBaseName(char *s)	{
	basename=s;
}
char *FileUtil::getBaseName(){
	return basename;
}
//-------------------------------------------------------------
// FileUtil::getSystemDir get system directory
//-------------------------------------------------------------
void FileUtil::getSystemDir(char *dir)
{
	char base[MAXSTR];
  	getBaseDirectory(base);
	makeSubDirectory(base,system,dir);
}
//-------------------------------------------------------------
// FileUtil::getBitmapsDir get bitmaps directory
//-------------------------------------------------------------
void FileUtil::getBitmapsDir(char *dir)
{
	char path[MAXSTR];
	char base[MAXSTR];
	char tmp[MAXSTR];
  	getBaseDirectory(base);
	makeSubDirectory(base,bitmaps,path);
	strcat(path,separator);
	strcpy(dir,path);
}
//-------------------------------------------------------------
// FileUtil::getBitmapsDir get bitmaps directory
//-------------------------------------------------------------
void FileUtil::getTmpDir(char *dir)
{
	char base[MAXSTR];;
	char path[MAXSTR];
	getBitmapsDir(base);
	makeSubDirectory(base,tmp,path);
	strcat(path,separator);
	//cout<<"TMP ="<<path<<endl;
	strcpy(dir,path);
}
//-------------------------------------------------------------
// FileUtil::getImagesDir get images directory
//-------------------------------------------------------------
void FileUtil::getImportsDir(char *dir)
{
	char path[MAXSTR];
	char sdir[MAXSTR];
  	getBaseDirectory(path);
	addToPath(path,textures);
	addToPath(path,imports);
	strcat(path,separator);
	strcpy(dir,path);
}
//-------------------------------------------------------------
// FileUtil::getHmapsDir get hmaps directory
//-------------------------------------------------------------
void FileUtil::getHmapsDir(char *dir)
{
	char path[MAXSTR];
	char sdir[MAXSTR];
  	getBaseDirectory(path);
	addToPath(path,textures);
	addToPath(path,hmaps);
	strcat(path,separator);
	strcpy(dir,path);
}
//-------------------------------------------------------------
// FileUtil::getMapsDir get Maps directory
//-------------------------------------------------------------
void FileUtil::getMapsDir(char *dir)
{
	char path[MAXSTR];
	char sdir[MAXSTR];
  	getBaseDirectory(path);
	addToPath(path,textures);
	addToPath(path,maps);
	strcat(path,separator);
	strcpy(dir,path);
}

//-------------------------------------------------------------
// FileUtil::getSpritesDir get sprites images directory
//-------------------------------------------------------------
void FileUtil::getSpritesDir(char *dir)
{
	char path[MAXSTR];
  	getBaseDirectory(path);
	addToPath(path,textures);
	addToPath(path,sprites);
	strcat(path,separator);
	strcpy(dir,path);
}

//-------------------------------------------------------------
// FileUtil::getSpritesDir get branches images directory
//-------------------------------------------------------------
void FileUtil::getBranchesDir(char *dir)
{	
	char path[MAXSTR];
  	getBaseDirectory(path);
	addToPath(path,textures);
	addToPath(path,plants);
	addToPath(path,branches);
	strcat(path,separator);
	strcpy(dir,path);
}

//-------------------------------------------------------------
// FileUtil::getSpritesDir get branches images directory
//-------------------------------------------------------------
void FileUtil::getLeavesDir(char *dir)
{
	char path[MAXSTR];
  	getBaseDirectory(path);
	addToPath(path,textures);
	addToPath(path,plants);
	addToPath(path,leaves);
	strcat(path,separator);
	strcpy(dir,path);
}

//-------------------------------------------------------------
// FileUtil::getMoviesDir get movies directory
//-------------------------------------------------------------
void FileUtil::getMoviesDir(char *dir)
{
	char base[MAXSTR];
  	getBaseDirectory(base);
	makeSubDirectory(base,movies,dir);
}

//-------------------------------------------------------------
// FileUtil::getSavesDir get saves directory
//-------------------------------------------------------------
void FileUtil::getSavesDir(char *dir)
{
	char base[MAXSTR];
  	getBaseDirectory(base);
	makeSubDirectory(base,saves,dir);
}
//-------------------------------------------------------------
// FileUtil::getShadersDir get shaders directory
//-------------------------------------------------------------
void FileUtil::getShadersDir(char *dir)
{
	char base[MAXSTR];
  	getBaseDirectory(base);
	makeSubDirectory(base,shaders,dir);
}

//-------------------------------------------------------------
// FileUtil::return path to file
//-------------------------------------------------------------
void FileUtil::addToPath(char *pdir, char *cdir){
    strcat(pdir,separator);
    strcat(pdir,cdir);
}

//-------------------------------------------------------------
// FileUtil::fileExists() return true if file exists
//-------------------------------------------------------------
int FileUtil::fileExists(char *dir,char *fn)
{
    char path[MAXSTR];
    sprintf(path,"%s%s%s",dir,separator,fn);
	return fileExists(path);
}

//-------------------------------------------------------------
// FileUtil::read a file
//-------------------------------------------------------------
FILE *FileUtil::readFile(char *dir, char *fn){
    char path[MAXSTR];
    char base[MAXSTR];
    getBaseDirectory(base);
    sprintf(path,"%s/%s/%s",base,dir,fn);
    return fopen(path, "rb");
}

//-------------------------------------------------------------
// FileUtil::write a  file
//-------------------------------------------------------------
void FileUtil::makeFilePath(char *dir, char *fn, char *path){
    char base[MAXSTR];
    getBaseDirectory(base);
    makeSubDirectory(base,dir,path);
    if(fn)
    	addToPath(path,fn);
}

//-------------------------------------------------------------
// FileUtil::write a  file
//-------------------------------------------------------------
FILE *FileUtil::writeFile(char *dir, char *fn){
    char path[MAXSTR];
    makeFilePath(dir,fn,path);
    return fopen(path, "wb");
}

//-------------------------------------------------------------
// FileUtil::getFilePath return path to file (remove filename)
//-------------------------------------------------------------
void FileUtil::getFilePath(char *dir, char *path){
	char name[512];
	strcpy(name,dir);
	char *start=name;
	char *cptr=start+strlen(name)-1;

	// remove name & ext

	while(*cptr != *separator && *cptr != ':' && cptr>start)
		cptr--;

	if(*cptr==*separator)
		cptr[0]=0;
	strcpy(path,name);
}

//-------------------------------------------------------------
// FileUtil::getFileName() get name from path
//-------------------------------------------------------------
void FileUtil::getFileName(char *s, char *d)
{
	char name[512];
	strcpy(name,s);
	char *start=name;
	char *cptr=start+strlen(name)-1;

	// remove extension

	while(*cptr != '.' && cptr>start && *cptr != separator[0]){
		//cptr[0]=0;
		cptr--;
	}
	if(cptr[0]=='.'){
		cptr[0]=0;

		// remove path

		while(*cptr != *separator && *cptr != ':' && cptr>start)
			cptr--;
	}
	if(*cptr==*separator)
		cptr++;
	strcpy(d,cptr);
}

//-------------------------------------------------------------
// FileUtil::getParentDirectory() get parent directory path
//-------------------------------------------------------------
void FileUtil::getParentDirectory(char *s, char *buff)
{
	char path[512];
	strcpy(path,s);
	char *cptr=path+strlen(path)-1;
	while(*cptr != *separator && *cptr != ':'  && cptr>path)
		cptr--;
	cptr[0]=0;
	strcpy(buff,path);
}

//-------------------------------------------------------------
// FileUtil::return 1 if directory is empty
//-------------------------------------------------------------
int FileUtil::isEmptyDirectory(char *dir)
{
	ValueList<FileData*>list;
	getDirectoryList(dir,list);
	if(list.size==0)
		return 0;
	list.free();
	return 1;
}

#ifdef WIN32
//-------------------------------------------------------------
// FileUtil::moveFile move or rename a file
//-------------------------------------------------------------
int FileUtil::moveFile(char *p, char *f)
{
	if(!fileExists(p))
		return 0;
	if(fileExists(f))
		return 0;
	MoveFile(p,f);
	return fileExists(f);
}

//-------------------------------------------------------------
// FileUtil::getCurrentDirectory() get current directory path
//-------------------------------------------------------------
void FileUtil::getCurrentDirectory(char *dir)
{
    static int init=1;
    static char sdir[MAXSTR];

	GetCurrentDirectory(MAXSTR,sdir);
	strcpy(dir,sdir);
}

//bool FileUtil::setCurrentDirectory(char *dir)
//{
//	::SetCurrentDirectory(dir);
//	return true;
//}

//-------------------------------------------------------------
// FileUtil::getBaseDirectory() get application directory path
//-------------------------------------------------------------
void FileUtil::getBaseDirectory(char *bdir)
{
    static int init=1;
    static char sdir[MAXSTR];

    if(init){
	    char cdir[MAXSTR];
	    char dir[MAXSTR];
	    char fdir[MAXSTR];
	    char *fname;
	    bdir[0]=0;
	    getCurrentDirectory(cdir);

	    int found=SearchPath(NULL,basename, 0, MAXSTR,fdir,&fname);
		if(found){
			getFilePath(fdir, dir);
		    getParentDirectory(dir, sdir);
		}

	    ::SetCurrentDirectory(cdir);
	    init=0;
	}
	strcpy(bdir,sdir);
}

//-------------------------------------------------------------
// FileUtil::getSubDirectory() get sub-directory path
//-------------------------------------------------------------
int FileUtil::getSubDirectory(char *pdir, char *sdir,char *path)
{
	char *fname;
	int found=SearchPath(pdir,sdir, 0, MAXSTR,path,&fname);
	if(!found)
		path[0]=0;
	return found;
}

//-------------------------------------------------------------
// FileUtil::makeSubDirectory() insure sub-directory exists
//-------------------------------------------------------------
int FileUtil::makeSubDirectory(char *pdir, char *sdir,char *path)
{
	char *fname;
	int found=SearchPath(pdir, sdir, 0, MAXSTR,path,&fname);
	if(!found){
		sprintf(path,"%s%s%s",pdir,separator,sdir);
		found=CreateDirectory(path,(LPSECURITY_ATTRIBUTES)NULL);
	}
	if(!found)
		path[0]=0;
	return found;
}

//-------------------------------------------------------------
// FileUtil::fileExists() return true if file exists
//-------------------------------------------------------------
int FileUtil::fileExists(char *f)
{
    WIN32_FIND_DATA data;
	HANDLE hndl=FindFirstFile(f,&data);
	if(hndl==INVALID_HANDLE_VALUE)
		return 0;
	FindClose(hndl);
	return 1;
}

//-------------------------------------------------------------
// FileUtil::getDirectoryList() get list of sub-directories
//-------------------------------------------------------------
void FileUtil::getDirectoryList(char *dir,ValueList<FileData*>&list)
{
	char path[MAXSTR];
	struct _finddata_t c_file;
    long long hFile;

	sprintf(path,"%s%s%s",dir,separator,"*");
    if((hFile = _findfirst(path, &c_file)) == -1)
		return;
    
	if(c_file.attrib & _A_SUBDIR && c_file.name[0]!='.'){
		sprintf(path,"%s%s%s",dir,separator,c_file.name);
		list.add(new FileData(c_file.name,path));
	}
	while(_findnext(hFile, &c_file) == 0){
		if((c_file.attrib & _A_SUBDIR  && c_file.name[0]!='.')){
			sprintf(path,"%s%s%s",dir,separator,c_file.name);			
			list.add(new FileData(c_file.name,path));
		}
	}
	_findclose(hFile);
}

//-------------------------------------------------------------
// FileUtil::getFileNameList() get list of files in directory
//-------------------------------------------------------------
void FileUtil::getFileNameList(char *dir,const char *xt,NameList<ModelSym*>&list)
{
	char cdir[MAXSTR];
	char name[MAXSTR];
	char path[MAXSTR];
	GetCurrentDirectory(MAXSTR,cdir);
	::SetCurrentDirectory(dir);
	WIN32_FIND_DATA data;

	HANDLE hndl=FindFirstFile(xt,&data);
	int morefiles=(hndl!=INVALID_HANDLE_VALUE);
	while(morefiles){
		getFileName(data.cFileName,name);
		if(strlen(name) && name[0] !='.'){
			char c=dir[strlen(dir)-1];
			if(c==separator[0])
				sprintf(path,"%s%s",dir,data.cFileName);
			else				
				sprintf(path,"%s%s%s",dir,separator,data.cFileName);
			if(!list.inlist(name)){
				ModelSym *sym=new ModelSym(name,path);
				list.add(sym);
			}
		}
		morefiles=FindNextFile(hndl,&data);
	}
	FindClose(hndl);
	::SetCurrentDirectory(cdir);
	list.sort();
}
//-------------------------------------------------------------
// FileUtil::getFileNameList() get list of files in directory
//-------------------------------------------------------------
void FileUtil::getFileNameList(char *dir,const char *xt,LinkedList<ModelSym*>&list)
{
	char cdir[MAXSTR];
	char name[MAXSTR];
	char path[MAXSTR];
	GetCurrentDirectory(MAXSTR,cdir);
	::SetCurrentDirectory(dir);
	WIN32_FIND_DATA data;

	HANDLE hndl=FindFirstFile(xt,&data);
	int morefiles=(hndl!=INVALID_HANDLE_VALUE);
	while(morefiles){
		getFileName(data.cFileName,name);
		if(strlen(name) && name[0] !='.'){
			char c=dir[strlen(dir)-1];
			if(c==separator[0])
				sprintf(path,"%s%s",dir,data.cFileName);
			else				
				sprintf(path,"%s%s%s",dir,separator,data.cFileName);
			ModelSym *sym=new ModelSym(name,path);
			list.add(sym);
		}
		morefiles=FindNextFile(hndl,&data);
	}
	FindClose(hndl);
	::SetCurrentDirectory(cdir);
}

//-------------------------------------------------------------
// FileUtil::deleteFile() delete a file
//-------------------------------------------------------------
int FileUtil::deleteFile(char *s)
{
 	return DeleteFile(s);
}

//-------------------------------------------------------------
// FileUtil::deleteDirectoryFiles() delete all files in directory
//-------------------------------------------------------------
int FileUtil::deleteDirectoryFiles(char *path)
{
    return deleteDirectoryFiles(path,(char*)"*");
}

//-------------------------------------------------------------
// FileUtil::deleteDirectoryFiles() delete files in directory
//-------------------------------------------------------------
int FileUtil::deleteDirectoryFiles(char *dir,char *xt)
{
    char path[MAXSTR];
	char ext[16];
	if(xt[0]=='*')
		strcpy(ext,xt+1);
	else
		strcpy(ext,xt);

	strcpy(path,dir);

	LinkedList<ModelSym*>flist;
	getFileNameList(dir,xt,flist);

	flist.ss();
    ModelSym*sym;
	while(sym=flist++){
		//sprintf(path,"%s%s%s%s",dir,File.separator,sym->name,ext);
		DeleteFile(sym->path);
	}
	flist.free();
	return 1;
}
//-------------------------------------------------------------
// FileUtil::deleteDirectory() delete a directory
//-------------------------------------------------------------
int FileUtil::deleteDirectory(char *path)
{
	deleteDirectoryFiles(path);
 	return RemoveDirectory(path);
}

#else // end WINDOWS

//************************************************************
// Unix implementation
//************************************************************

void FileUtil::getCurrentDirectory(char *dir)
{
    static char cdir[MAXSTR];
    static int init=1;
    cdir[0]=0;
    if(init){
	    ::system("pwd>tmp");
	    FILE *fp;
	     if((fp = fopen("tmp", "rb")) != NULL){
		    fgets(cdir,MAXSTR,fp);
		    fclose(fp);
	    }
	    ::system("rm tmp");
	    init=0;
	}
	strcpy(dir,cdir);
}

bool FileUtil::setCurrentDirectory(char *dir)
{
    if(!fileExists(dir))
        return false;
 	char buff[MAXSTR];
 	sprintf(buff,"cd %s",dir);
	::system(buff);
	return true;
}

void FileUtil::getBaseDirectory(char *dir)
{
    // assume that base dir=../ from sx executable
	static char tmp[MAXSTR]={0};
    static int init=1;
    if(init){
		getCurrentDirectory(tmp);
		getParentDirectory(tmp,tmp);
	    init=0;
	}
	strcpy(dir,tmp);
}

int FileUtil::getSubDirectory(char *pdir, char *sdir, char *buff)
{
	return 0;
}

//-------------------------------------------------------------
// FileUtil::getDirectoryList() get list of subdirectories
//-------------------------------------------------------------
void FileUtil::getDirectoryList(char *dir,LinkedList<ModelSym*>&list)
{

	char path[MAXSTR];

    DIR *dirp;
    struct dirent *dp;
    struct stat buf;

    //printf("\tDirectories:\n");
    //printf("\t----------- \n");
    dirp = opendir(".");
    while ((dp = readdir(dirp))>0) {
        if (stat(dp->d_name,&buf) == 0){
            if(buf.st_mode & S_IFDIR){
				sprintf(path,"%s%s%s",dir,separator,dp->d_name);
				list.add(new ModelSym(dp->d_name,path));
			}
		}
    }
    closedir(dirp);
}

void FileUtil::getFileNameList(char *dir,const char *xt,LinkedList<ModelSym*>&list)
{
	NameList<ModelSym*> sorted;

	DIR *pdir;
	struct dirent *pfile;
	char extbuf[16];
	strcpy(extbuf,xt);
	char *sext=extbuf;
	if(*sext=='*')
	    sext++;
	if(*sext=='.')
	    sext++;
    // errno not defined in WIN7-GLEW build
	//errno=0; // reset before calling readdir
	pdir=opendir(dir);
	if(!pdir){
		perror(dir);
		return;
	}
	//errno=0; // reset before calling readdir
	while((pfile=readdir(pdir))>0) {
	    char name[MAXSTR];
    	strcpy(name,pfile->d_name);
    	if(strstr(name,"_alpha")!=NULL)
    		continue;

    	char *iname=strtok(name,".");
    	if(iname){
    	    char *fext=strtok(NULL,".");
    	    if(fext && !strcmp(fext,sext)){
    			char path[MAXSTR];
    			sprintf(path,"%s%s%s",dir,separator,pfile->d_name);
    		    sorted.add(new ModelSym(name,path));
    		}
    	}
    }
    //if (errno==0)
    	closedir(pdir);
    ModelSym* sym;
    sorted.sort();
    sorted.ss();
	while((sym=sorted++)>0)
		list.add(sym);
}

//-------------------------------------------------------------
// FileUtil::makeSubDirectory() insure sub-directory exists
//-------------------------------------------------------------
int FileUtil::makeSubDirectory(char *pdir, char *sdir,char *path)
{
    sprintf(path,"%s%s%s",pdir,separator,sdir);
    if(fileExists(path))
        return 1;
 	char buff[MAXSTR];
 	sprintf(buff,"mkdir %s",path);
	::system(buff);
	return fileExists(path);
}

//-------------------------------------------------------------
// FileUtil::moveFile move or rename a file
//-------------------------------------------------------------
int FileUtil::moveFile(char *p, char *f)
{
	if(!fileExists(p))
		return 0;
	if(!fileExists(f))
		return 0;
	char tmp[MAXSTR];
    sprintf(tmp,"mv %s %s",p,f);
	::system(tmp);

	return fileExists(f);
}

//-------------------------------------------------------------
// FileUtil::deleteFile() delete a file
//-------------------------------------------------------------
int FileUtil::deleteFile(char *s)
{
    if(fileExists(s)){
    	char tmp[MAXSTR];
    	sprintf(tmp,"rm %s",s);
		::system(tmp);
		return 1;
	}
	return 0;
}

//-------------------------------------------------------------
// FileUtil::deleteDirectoryFiles() delete all files in directory
//-------------------------------------------------------------
int FileUtil::deleteDirectoryFiles(char *path)
{
    return deleteDirectoryFiles(path,(char*)"*");
}

//-------------------------------------------------------------
// FileUtil::deleteDirectoryFiles() delete files in directory
//-------------------------------------------------------------
int FileUtil::deleteDirectoryFiles(char *path,char *type)
{
 	struct stat file_stat;
 	int err=stat(path,&file_stat);

    if(err<0){
     	printf("stat returned error %s\n",path);
     	return 0;
    }
    if((file_stat.st_mode & S_IFMT)==S_IFDIR){
     	printf("deleting (files) in %s\n",path);
    	char tmp[MAXSTR];
    	sprintf(tmp,"rm -f %s/%s",path,type);
    	::system(tmp);
    }
	return 1;
}

//-------------------------------------------------------------
// FileUtil::deleteDirectory() delete a file or directory
//-------------------------------------------------------------
int FileUtil::deleteDirectory(char *path)
{
 	struct stat file_stat;
 	int err=stat(path,&file_stat);

    if(err<0){
     	printf("stat returned error %s\n",path);
     	return 0;
    }
    if((file_stat.st_mode & S_IFMT)==S_IFDIR){
     	printf("deleting (dir) %s\n",path);
    	char tmp[MAXSTR];
    	sprintf(tmp,"rm -f %s/*.*",path);
    	::system(tmp);
    	sprintf(tmp,"rmdir %s",path);
    	::system(tmp);
    }
	return 1;
}

//-------------------------------------------------------------
// FileUtil::fileExists() return true if file exists
//-------------------------------------------------------------
int FileUtil::fileExists(char *path)
{
  	struct stat file_stat;
 	int err=stat(path,&file_stat);
	return err<0?0:1;
}
#endif // unix

//************************************************************

