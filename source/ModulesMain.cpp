// ../../../../..;../../../../../vstgui.sf/vstgui;../../../../../public.sdk/source/vst2.x;../../SoloRack/source
//#include "audioeffect.h"

//#ifndef __Configuration__
//#include ".\Configuration.h"
//#endif

#include "Modules.h"

#if WIN32
#include <windows.h>
void* hInstance;

int GetDataDirectory(char* dst, int nsize)
{	// Finds dll data folder WITH trailing '\\' followed by '\0'
	// This directory is direct parent folder of the dll file. So we just have to remove the "what_ever_name.dll" from the file name
	// Returns length of the path string excluding trailing null character

	int i, rsize;

	// ** think of using UNICODE version instead
	rsize = GetModuleFileNameA((HMODULE)hInstance, (LPCH)dst, nsize);		//** what if nsize is not enough.

	// Ignore file extension name
	for (i = rsize - 1; i >= 0; i--)
		if (dst[i] == '\\') return i + 1;
		else dst[i] = '\0';
	return -1;
}

extern "C" {
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{	int len;
	hInstance = hInst;

	if (dwReason==DLL_PROCESS_ATTACH)
	{	Module::dlldatadir = (char *) malloc(2*MAX_PATH*sizeof(*Module::dlldatadir));
		len = GetDataDirectory(Module::dlldatadir,2*MAX_PATH);
		Module::dlldatadir = (char *) realloc(Module::dlldatadir,(len+1)*sizeof(*Module::dlldatadir));

		// Remmember to call your Initialize() in DllInitialize() located in Exports.cpp file
	}
	else if (dwReason==DLL_PROCESS_DETACH)
	{	Clouds::End();
		Rings::End();
		Braids::End();
		Tides::End();
		Branches::End();
		Module::End();

		free(Module::skindir); free(Module::defskindir);
		free(Module::dlldatadir); free(Module::dllskindir);
		Module::skindir=NULL; Module::defskindir=NULL;
		Module::dlldatadir=NULL; Module::dllskindir=NULL;
	}

	return 1;
}
} 
#else
// MAC case
#endif
