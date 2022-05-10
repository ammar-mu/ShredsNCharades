/*	SoloRack SDK v0.11 Beta
	Copyright 2017-2022 Ammar Muqaddas
*/

#include "Modules.h"

//----------------------------------------------------------
// Exported functions
extern "C"
{

__declspec(dllexport) bool GetDllModule(DllModule *mi, char *vendorname)
{	// mi->name and mi->vendorname are inputs to this function.
	// They are read by SoloRack from the xxxx.ini file, were xxxx.dll is the dll contaniing this requested module
	// Returnes true if the operation is successful, otherwise false.

	if (strcmp(mi->name,Clouds::name)==0 && strcmp(vendorname,Clouds::vendorname)==0)
	{	mi->sdk_version = SDK_VERSION;
		mi->name_len = Clouds::name_len;		// Not neccessary.
		
		mi->InitializePtr = Clouds::Initialize;
		mi->EndPtr = Clouds::End;
		mi->Constructor = (Module *(*)(CFrame *,CControlListener *, const SynthComm*, const int)) Clouds::Constructor;				// (Module *(__cdecl *)(CFrame *,CControlListener *)) 
		mi->GetTypePtr = Clouds::GetType;
		mi->ActivatePtr = Clouds::Activate;
		mi->IsActivePtr = Clouds::IsActive;
		mi->GetProductNamePtr = Clouds::GetProductName;

		return true;
	}
	else if (strcmp(mi->name, Rings::name) == 0 && strcmp(vendorname, Rings::vendorname) == 0)
	{
		mi->sdk_version = SDK_VERSION;
		mi->name_len = Rings::name_len;		// Not neccessary.

		mi->InitializePtr = Rings::Initialize;
		mi->EndPtr = Rings::End;
		mi->Constructor = (Module * (*)(CFrame*, CControlListener*, const SynthComm*, const int)) Rings::Constructor;				// (Module *(__cdecl *)(CFrame *,CControlListener *)) 
		mi->GetTypePtr = Rings::GetType;
		mi->ActivatePtr = Rings::Activate;
		mi->IsActivePtr = Rings::IsActive;
		mi->GetProductNamePtr = Rings::GetProductName;

		return true;
	}
	else if (strcmp(mi->name, Braids::name) == 0 && strcmp(vendorname, Braids::vendorname) == 0)
	{
		mi->sdk_version = SDK_VERSION;
		mi->name_len = Braids::name_len;		// Not neccessary.

		mi->InitializePtr = Braids::Initialize;
		mi->EndPtr = Braids::End;
		mi->Constructor = (Module * (*)(CFrame*, CControlListener*, const SynthComm*, const int)) Braids::Constructor;				// (Module *(__cdecl *)(CFrame *,CControlListener *)) 
		mi->GetTypePtr = Braids::GetType;
		mi->ActivatePtr = Braids::Activate;
		mi->IsActivePtr = Braids::IsActive;
		mi->GetProductNamePtr = Braids::GetProductName;

		return true;
	}
	else if (strcmp(mi->name, Tides::name) == 0 && strcmp(vendorname, Tides::vendorname) == 0)
	{
		mi->sdk_version = SDK_VERSION;
		mi->name_len = Tides::name_len;		// Not neccessary.

		mi->InitializePtr = Tides::Initialize;
		mi->EndPtr = Tides::End;
		mi->Constructor = (Module * (*)(CFrame*, CControlListener*, const SynthComm*, const int)) Tides::Constructor;				// (Module *(__cdecl *)(CFrame *,CControlListener *)) 
		mi->GetTypePtr = Tides::GetType;
		mi->ActivatePtr = Tides::Activate;
		mi->IsActivePtr = Tides::IsActive;
		mi->GetProductNamePtr = Tides::GetProductName;

		return true;
	}
	else if (strcmp(mi->name, Branches::name) == 0 && strcmp(vendorname, Branches::vendorname) == 0)
	{
		mi->sdk_version = SDK_VERSION;
		mi->name_len = Branches::name_len;		// Not neccessary.

		mi->InitializePtr = Branches::Initialize;
		mi->EndPtr = Branches::End;
		mi->Constructor = (Module * (*)(CFrame*, CControlListener*, const SynthComm*, const int)) Branches::Constructor;				// (Module *(__cdecl *)(CFrame *,CControlListener *)) 
		mi->GetTypePtr = Branches::GetType;
		mi->ActivatePtr = Branches::Activate;
		mi->IsActivePtr = Branches::IsActive;
		mi->GetProductNamePtr = Branches::GetProductName;

		return true;
	}
	else return false;
}

// The following functions are used to enumerate all modules in the Dll, they are only used when scanning the dll instead of reading an .INI file. Which is not the default behavior.
// You should provide an accurate .INI file for your dll.
__declspec(dllexport) int GetDllNumberOfModules()
{	// Returnes total number of modules in this Dll.
	return 5;
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
		default:
			return -1;
	}
}

__declspec(dllexport) bool GetDllModuleByIndex(DllModule *mi, int index)
{	// index range is from 0 to (Number Of Modules-1)
	// Returnes true if the operation is successful, otherwise false.
	// Note, caller (SoloRack) should allocate the memory for name. enough space can be ensured by calling GetDllModuleNameLenByIndex() 

	switch (index)
	{	case 0:
			mi->sdk_version = SDK_VERSION;
			strcpy(mi->name,Clouds::name);
			mi->name_len = Clouds::name_len;	

			mi->InitializePtr = Clouds::Initialize;
			mi->EndPtr = Clouds::End;
			mi->Constructor = (Module *(*)(CFrame *,CControlListener *, const SynthComm*, const int)) Clouds::Constructor;				// (Module *(__cdecl *)(CFrame *,CControlListener *)) 
			mi->GetTypePtr = Clouds::GetType;
			mi->ActivatePtr = Clouds::Activate;
			mi->IsActivePtr = Clouds::IsActive;
			mi->GetProductNamePtr = Clouds::GetProductName;
			break;

		case 1:
			mi->sdk_version = SDK_VERSION;
			strcpy(mi->name, Rings::name);
			mi->name_len = Rings::name_len;

			mi->InitializePtr = Rings::Initialize;
			mi->EndPtr = Rings::End;
			mi->Constructor = (Module * (*)(CFrame*, CControlListener*, const SynthComm*, const int)) Rings::Constructor;				// (Module *(__cdecl *)(CFrame *,CControlListener *)) 
			mi->GetTypePtr = Rings::GetType;
			mi->ActivatePtr = Rings::Activate;
			mi->IsActivePtr = Rings::IsActive;
			mi->GetProductNamePtr = Rings::GetProductName;
			break;

		case 2:
			mi->sdk_version = SDK_VERSION;
			strcpy(mi->name, Braids::name);
			mi->name_len = Braids::name_len;

			mi->InitializePtr = Braids::Initialize;
			mi->EndPtr = Braids::End;
			mi->Constructor = (Module * (*)(CFrame*, CControlListener*, const SynthComm*, const int)) Braids::Constructor;				// (Module *(__cdecl *)(CFrame *,CControlListener *)) 
			mi->GetTypePtr = Braids::GetType;
			mi->ActivatePtr = Braids::Activate;
			mi->IsActivePtr = Braids::IsActive;
			mi->GetProductNamePtr = Braids::GetProductName;
			break;

		case 3:
			mi->sdk_version = SDK_VERSION;
			strcpy(mi->name, Tides::name);
			mi->name_len = Tides::name_len;

			mi->InitializePtr = Tides::Initialize;
			mi->EndPtr = Tides::End;
			mi->Constructor = (Module * (*)(CFrame*, CControlListener*, const SynthComm*, const int)) Tides::Constructor;				// (Module *(__cdecl *)(CFrame *,CControlListener *)) 
			mi->GetTypePtr = Tides::GetType;
			mi->ActivatePtr = Tides::Activate;
			mi->IsActivePtr = Tides::IsActive;
			mi->GetProductNamePtr = Tides::GetProductName;
			break;

		case 4:
			mi->sdk_version = SDK_VERSION;
			strcpy(mi->name, Branches::name);
			mi->name_len = Branches::name_len;

			mi->InitializePtr = Branches::Initialize;
			mi->EndPtr = Branches::End;
			mi->Constructor = (Module * (*)(CFrame*, CControlListener*, const SynthComm*, const int)) Branches::Constructor;				// (Module *(__cdecl *)(CFrame *,CControlListener *)) 
			mi->GetTypePtr = Branches::GetType;
			mi->ActivatePtr = Branches::Activate;
			mi->IsActivePtr = Branches::IsActive;
			mi->GetProductNamePtr = Branches::GetProductName;
			break;

		default:
			return false;
	}
	return true;
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
	// Remmember to call your Initialize() here for each of your modules
	// ....
	return true;
}
}