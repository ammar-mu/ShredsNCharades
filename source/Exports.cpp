/*	SoloRack SDK v0.11 Beta
	Copyright 2017-2022 Ammar Muqaddas
*/

#include "ShredsNCharades.h"

//----------------------------------------------------------
// Exported functions
extern "C"
{
// The following functions are used to enumerate all modules in the Dll, they are only used when scanning the dll instead of reading an .INI file. Which is not the default behavior.
// You should provide an accurate .INI file for your dll.
__declspec(dllexport) int GetDllNumberOfModules()
{	// Returnes total number of modules in this Dll.
	return 13;
}

__declspec(dllexport) const char *GetDllVendorName()
{	return "ShredsNCharades";
}

__declspec(dllexport) int GetDllModuleNameLenByIndex(DllModule *mi, int index)
{	// index range is from 0 to (Number Of Modules-1)
	// Returnes the module name length (exclusing terminating null).
	// If the module is not found, -1 is returned

	switch (index)
	{	case 0:
			return Clouds::name_len;	
		case 1:
			return Rings::name_len;
		case 2:
			return Braids::name_len;
		case 3:
			return Tides::name_len;
		case 4:
			return Branches::name_len;
		case 5:
			return Warps::name_len;
		case 6:
			return Stages::name_len;
		case 7:
			return Elements::name_len;
		case 8:
			return Marbles::name_len;
		case 9:
			return Frames::name_len;
		case 10:
			return Streams::name_len;
		case 11:
			return Shelves::name_len;
		case 12:
			return Plaits::name_len;
		default:
			return -1;
	}
}

__declspec(dllexport) bool GetDllModuleByIndex(DllModule* mi, int index)
{	// index range is from 0 to (Number Of Modules-1)
	// Returnes true if the operation is successful, otherwise false.
	// Note, caller (SoloRack) should allocate the memory for name. enough space can be ensured by calling GetDllModuleNameLenByIndex() 

	//#define CHOOSEMACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,NAME,...) NAME
	//#define NOCASE(cn)
	//#define MODCASE(...) CHOOSEMACRO(__VA_ARGS__,MODCASEN,MODCASEN,MODCASEN,MODCASEN,MODCASEN,MODCASEN,MODCASEN,MODCASEN,NOCASE)(__VA_ARGS__)

	#define MODCASE(cn,classname)	\
	case cn:								\
	mi->sdk_version = SDK_VERSION;			\
	strcpy(mi->name, classname::name);			\
	mi->name_len = classname::name_len;		\
	mi->InitializePtr = classname::Initialize;	\
	mi->EndPtr = classname::End;				\
	mi->Constructor = (Module * (*)(CFrame*, CControlListener*, const SynthComm*, const int)) classname::Constructor;	\
	mi->GetTypePtr = classname::GetType;		\
	mi->ActivatePtr = classname::Activate;		\
	mi->IsActivePtr = classname::IsActive;		\
	mi->GetProductNamePtr = classname::GetProductName;		\
	break;

	switch (index)
	{	MODCASE(0, Clouds)
		MODCASE(1, Rings)
		MODCASE(2, Braids)
		MODCASE(3, Tides)
		MODCASE(4, Branches)
		MODCASE(5, Warps)
		MODCASE(6, Stages)
		MODCASE(7, Elements)
		MODCASE(8, Marbles)
		MODCASE(9, Frames)
		MODCASE(10, Streams)
		MODCASE(11, Shelves)
		MODCASE(12, Plaits)

		default:
			return false;
	}
	return true;
}

__declspec(dllexport) bool GetDllModule(DllModule* mi, char* vendorname)
{	// mi->name and mi->vendorname are inputs to this function.
	// They are read by SoloRack from the xxxx.ini file, were xxxx.dll is the dll contaniing this requested module
	// Returnes true if the operation is successful, otherwise false.

	//#define CHOOSEMACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,NAME,...) NAME
	//#define NOMODIF() return false;
	//#define MODIF(...) CHOOSEMACRO(__VA_ARGS__,MODIFN,MODIFN,MODIFN,MODIFN,MODIFN,MODIFN,MODIFN,MODIFN,MODIF1)(__VA_ARGS__)

	#define IFMOD(classname)	\
	if (strcmp(mi->name,classname::name)==0 && strcmp(vendorname,classname::vendorname)==0)	\
	{	mi->sdk_version = SDK_VERSION;						\
		mi->name_len = classname::name_len;					\
		mi->InitializePtr = classname::Initialize;				\
		mi->EndPtr = classname::End;							\
		mi->Constructor = (Module *(*)(CFrame *,CControlListener *, const SynthComm*, const int)) classname::Constructor;	\
		mi->GetTypePtr = classname::GetType;					\
		mi->ActivatePtr = classname::Activate;					\
		mi->IsActivePtr = classname::IsActive;					\
		mi->GetProductNamePtr = classname::GetProductName;		\
		return true;										\
	}

	IFMOD(Clouds)
	else IFMOD(Rings)
	else IFMOD(Braids)
	else IFMOD(Tides)
	else IFMOD(Branches)
	else IFMOD(Warps)
	else IFMOD(Stages)
	else IFMOD(Elements)
	else IFMOD(Marbles)
	else IFMOD(Frames)
	else IFMOD(Streams)
	else IFMOD(Shelves)
	else IFMOD(Plaits)
	else return false;
}

__declspec(dllexport) bool DllInitialize(const DllInit *init)
{	int i, len;

	// Setup some global variables
	// Skin and UI size variable
	Module::skindir = (char *) malloc((strlen(init->skindir)+2)*sizeof(*(init->skindir)));
	strcpy((char *) Module::skindir,init->skindir);
	Module::defskindir = (char *) malloc((strlen(init->defskindir)+2)*sizeof(*(init->defskindir)));
	strcpy((char *) Module::defskindir,init->defskindir);
	Module::vp = init->vp;
	Module::vp_5 = Module::vp/5; Module::vp_3 = Module::vp/3;
	Module::uiscale = init->uiscale;
	Module::hp = Module::uiscale*BASE_HP;

	// Choose dll skin size based on solorack's skin size (indicated by the name of the skin folder Small/Medium/Large
	// Alternativly you can use a diffrent way, for example, you can base the size upon the Module::uiscale or the Module::vp
	i = strlen(Module::skindir)-2;
	while (i>=0)
	{	if (Module::skindir[i]=='\\') 
		{	Module::dllskindir = (char *) malloc((strlen(Module::dlldatadir)+strlen("Skin\\")+strlen(&(Module::skindir[i+1]))+2)*sizeof(*Module::dllskindir));
			strcpy(Module::dllskindir,Module::dlldatadir);
			strcat(Module::dllskindir,"Skin\\");
			strcat(Module::dllskindir,&(Module::skindir[i+1])); 
			break; 
		}
		i--;
	}
	
	Module::Initialize();
	Clouds::Initialize();
	Rings::Initialize();
	Braids::Initialize();
	Tides::Initialize();
	Branches::Initialize();
	Warps::Initialize();
	Stages::Initialize();
	Elements::Initialize();
	Marbles::Initialize();
	Frames::Initialize();
	Streams::Initialize();
	Shelves::Initialize();
	Plaits::Initialize();
	// Remmember to call your Initialize() here for each of your modules
	// ....
	return true;
}
}