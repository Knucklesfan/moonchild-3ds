#include "Util.h"
#include <SDL.h>
#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <errno.h>
#include <frm_int.hpp>

#ifdef _WIN32
#  include <direct.h> // _mkdir
#endif

using std::string;

namespace {

char prefOrg[] = "moonchild";
char prefApp[] = "moonchild_shell";

}  // namespace

#ifdef _WIN32
#define PATH_SEPARATOR ("\\")
#define HOME_ENV_VAR ("USERPROFILE")
#else
#define PATH_SEPARATOR ("/")
#define HOME_ENV_VAR ("HOME")
#endif

static bool testfile(const string& path) {
  FILE* f = fopen(path.c_str(), "r");
  if (f) {
    fclose(f);
    return true;
  }
  return false;
}

static bool mkpath(const string& path) {
  if (path.empty()) {
    return false;
  }
#ifdef _WIN32
  return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
  return mkdir(path.c_str(), 0700) == 0 || errno == EEXIST;
#endif
}

// Called by the game to get the full path to a file
char *FullPath(char *filename) {
  string sfp;
  static string fullpath;

  if (!filename) {
    return nullptr;
  }

  sfp = filename;
  fullpath = string("romfs:/")+sfp;

  return (char*)fullpath.c_str();
 // this function does NOT need to be that complicated \/\/\/
//   // Check env override MOONCHILD_ASSETS_PATH
//   env = SDL_getenv("MOONCHILD_ASSETS_PATH");
//   if (env != nullptr) {
//     basepath = env;
//     fullpath = basepath + PATH_SEPARATOR + sfp;
//     if (testfile(fullpath)) {
//       return (char*)fullpath.c_str();
//     }
//   }

//   // Check XDG_DATA_HOME/moonchild/ (or default to HOME/.local/share/moonchild/)
//   env = SDL_getenv("XDG_DATA_HOME");
//   bool skipxdgdata = false;
//   if (env != nullptr) {
//     basepath = string(env);
//   } else {
//     env = SDL_getenv(HOME_ENV_VAR);
//     if (env != nullptr) {
//       basepath = string(env) + PATH_SEPARATOR + ".local" + PATH_SEPARATOR + "share";
//     } else {
//       skipxdgdata = true;
//     }
//   }
//   if (!skipxdgdata) {
//     fullpath = basepath + PATH_SEPARATOR + "moonchild" + PATH_SEPARATOR + sfp;
//     if (testfile(fullpath)) {
//       return (char*)fullpath.c_str();
//     }
//   }

// #ifndef _WIN32
//   // TODO check XDG_DATA_DIRS with default to /usr/local/share/:/usr/share/
//   const char* xdgDataDirsEnv = SDL_getenv("XDG_DATA_DIRS");
//   string xdgDataDirs;
//   if (xdgDataDirsEnv != nullptr) {
//     xdgDataDirs = xdgDataDirsEnv;
//   } else {
//     xdgDataDirs = "/usr/local/share:/usr/share";
//   }

//   const char delimiter = ':';

//   size_t start = 0;
//   size_t end = xdgDataDirs.find(delimiter);
//   while (end != string::npos) {
//     fullpath = xdgDataDirs.substr(start, end - start) + PATH_SEPARATOR + "moonchild" + PATH_SEPARATOR + sfp;
//     if (testfile(fullpath)) {
//       return (char*)fullpath.c_str();
//     }
//     start = end + 1;
//     end = xdgDataDirs.find(delimiter, start);
//   }
//   fullpath = xdgDataDirs.substr(start) + PATH_SEPARATOR + "moonchild" + PATH_SEPARATOR + sfp;
//   if (testfile(fullpath)) {
//     return (char*)fullpath.c_str();
//   }
// #endif

//   // Check SDL_GetBasePath/assets/
//   char* basePathCStr = SDL_GetBasePath();
//   if (basePathCStr) {
//     basepath = basePathCStr;
//     SDL_free(basePathCStr);
//     fullpath = basepath + PATH_SEPARATOR + "assets" + PATH_SEPARATOR + sfp;
//     if (testfile(fullpath)) {
//       return (char*)fullpath.c_str();
//     }
//   }

//   return filename;
}

// Called by the game to get the full path to an audio file
char *FullAudioPath(char *filename) {
  static string fullpath;
  fullpath = string("audio") + PATH_SEPARATOR + filename;
  return FullPath((char*)fullpath.c_str());
}

// Called by the game to get the full path to a writable file (Only hiscore file)
char *FullWritablePath(char *filename) {
  static string basedir;
  static string fullpath;
  char* env;

  if (!filename) {
    return nullptr;
  }

  basedir = {};
  fullpath = {};

  // 1. MOONCHILD_SAVE_PATH (use directly if env var is set)
  env = "sdmc:/moonchild/";
  if (env != nullptr) {
    basedir = string(env);
    mkpath(basedir);
  }

  fullpath = basedir + PATH_SEPARATOR + string(filename);
  return (char*)fullpath.c_str();
}

// Internal method (only used here) to load a TGA file
typedef unsigned char BYTE;
unsigned short*LoadTGA(char *FileName)
{
	printf("loading: %s\n", FullPath(FileName));
//	LOG(logbuf);
    
	// load targa file
	BYTE* tgabuff = new BYTE[20];
	bool OK = true;
  FILE *tga = fopen( FullPath(FileName), "rb" );
  if (!tga) return 0;
  printf("tga opened?\n");
  fread(tgabuff, 20, 1, tga);
  fclose(tga);
	// gzFile tga = gzopen( FullPath(FileName), "rb" );
	// if (!tga) return 0; 
	// gzread(tga, tgabuff, 20);
	// gzclose( tga );
    
//	LOG("open succeeded\n");
    
	int TgaIDLen;
	int TgaCMapType;
	int TgaImgType;
	int TgaCMapOrig;
	int TgaCMapLen;
	int TgaCMapSize;
	int TgaXPos;
	int TgaYPos;
	int TgaWidth;
	int TgaHeight;
	int TgaPixSize;
	TgaIDLen		= *tgabuff;
	TgaCMapType	= *(tgabuff + 1);
	TgaImgType	= *(tgabuff + 2);
	TgaCMapOrig	= *(tgabuff + 3) + 256 * *(tgabuff + 4);
	TgaCMapLen	= *(tgabuff + 5) + 256 * *(tgabuff + 6);
	TgaCMapSize	= *(tgabuff + 7);
	TgaXPos		= *(tgabuff + 8) + 256 * *(tgabuff + 9);
	TgaYPos		= *(tgabuff + 10) + 256 * *(tgabuff + 11);
	TgaWidth	= *(tgabuff + 12) + 256 * *(tgabuff + 13);
	TgaHeight	= *(tgabuff + 14) + 256 * *(tgabuff + 15);
	TgaPixSize	= *(tgabuff + 16);
	delete [] tgabuff;
    
	int w,h;
	w = TgaWidth;
	h = TgaHeight;
	unsigned short *dest;
	int size = w * 4 * h + 20;
	tgabuff = new BYTE[size];
	dest = new unsigned short[w*h];  // hier komt uitgepakte plaatje
  
  // replace the gzip loading by normal loading
	// tga = gzopen( FullPath(FileName), "rb" );
	// if (!tga)
	// {
	// 	delete [] tgabuff;
	// 	delete [] dest;
	// 	return 0;
	// }
	// int read = gzread( tga, tgabuff, size );
	// gzclose( tga );
  tga = fopen( FullPath(FileName), "rb" );
	if (!tga)
	{
	 	delete [] tgabuff;
	 	delete [] dest;
	 	return 0;
	}
  fread(tgabuff, size, 1, tga);
  fclose(tga);
	
	if (TgaImgType == 1)
	{
		// Palettized image
		unsigned short* pal = new unsigned short[256];
		for ( int i = 0; i < 256; i++ )
		{
			int b = *(tgabuff + 18 + i * 3);
			int g = *(tgabuff + 18 + i * 3 + 1);
			int r = *(tgabuff + 18 + i * 3 + 2);
			pal[i] = (unsigned short)(((r >> 3) << 11) + ((g >> 2) << 5) + (b >> 3));
		}
		unsigned char* src = tgabuff + 18 + 768 + (h - 1) * w;
		unsigned short* dst = (unsigned short*)dest;
		for ( int y = 0; y < h; y++ )
		{
			for ( int x = 0; x < w; x++ )
			{
				int idx = *(src + x);
				*(dst + x) = pal[idx];
			}
			dst += w;
			src -= w;
		}
	}
	else
	{
		// Store the data at the specified target address
		unsigned char* src = (tgabuff + 18) + (((h - 1) * w)*4);
		unsigned short* dst = (unsigned short*)dest;
		for ( int i = 0; i < h; i++ )
		{
			for ( int x = 0; x < w; x++ )
			{
				int r,g,b,a;
				unsigned short rgba;
				b= *src++;
				g= *src++;
				r= *src++;
				a= *src++;
				rgba = ((r>>3)<<11)+((g>>2)<<5)+(b>>3);
				*(dst + x) = rgba; //*(src + x);
			}
			dst += w;
			src -= (w*8);
		}
	}
    
	delete [] tgabuff;
    
	printf("tga success\n");
	return dest;
}

// Called by the game to show a picture (tga)
void ShowPicture(char *FileName)
{
  unsigned short *TempPic;
	TempPic   = LoadTGA(FileName);
    
	video->DrawTempPic();
    
	delete [] TempPic;
}

