/*	SoloRack SDK v0.2 Beta
	Copyright 2017-2022 Ammar Muqaddas
*/


#ifndef __Modules__
#define __Modules__


#ifndef __cvstguitimer__
#include "cvstguitimer.h"
#endif

#include "cscrollview.h"

#define __PI 3.14159265358979
#define __TWO_PI (2.0*__PI)



#define NO_TAG	2147483640
#define NOSAVE_TAG	2147483639
#define NEW_TAG	2147483638
#define MAX_INT		2147483646
#define SDK_VERSION		200
#define RAND_BITMAP		-1
#define ALL_MIDI_CHAN	-1
#define BASE_MHEIGHT	291.0
#define BASE_HP			12
#define SYNTH_EXIT		-2
#define SYSEX_MAXTAG	16

// Some more constants
#define SCREW_X	9
#define SCREW_Y	9
#define NUM_PP_BITMAPS	1
#define NUM_SCREW_BITMAPS	10
#define MAIN_KNOB_IMAGES 49
#define MAIN_KNOB_BIG_IMAGES 99
#define SMALL_KNOB_GRAY_IMAGES	31
#define SMALL_KNOB_BLUE_IMAGES	31
#define LED_BLUE_IMAGES	10
#define LED_RED_IMAGES	10
#define DIGITS_RED_IMAGES	10
#define DIGITS_RED_ALPHA_IMAGES	26
#define DIGITS_PLUS_MINUS_IMAGES	3
#define CLOCK_WIDTH			0.3					// Percentage of the total clock cycle
#define CLIPTIME 0.2							// Clip led pulse time in seconds
#define TRIGTIME 0.1							// Trigger led pulse time in seconds
#define LMIDITIME	0.1							// MIDI led pulse time in seconds
#define MAX_CLOCK_WIDTH		0.1					// Max clock pulse width is seconds
#define LED_PULSE_TIME		0.1					// Reasonably visible pulse time (sec)
#define SMOOTH_DELAY		35					// (was 45) Time in milliseconds to allow a knob to reach it's value smoothly.
#define HIGH_CV				0.25				
#define LOW_CV				0.15
#define MID_CV				((LOW_CV+HIGH_CV)/2.0)
#define VERY_HIGH_CV		4.0
#define MAX_LEVEL			(clip_level)
#define DEFAULT_MAX_LEVEL	4.0					// 12db
#define MIDI_TUNE_FACTOR	10.0				// The factor to achieve 0.1/oct tunning
#define MIDI_MAX_DELAY		0.002				// SMALL. For some Generators. In seconds. Dictates MIDI buffer sizes
#define MIDI_MAX_DELAY2		0.006				// MEDIUM. For mergers and such. In seconds. Dictates MIDI buffer sizes
#define LOG2				0.69314718055994529
#define LOG10				2.30258509299405
#undef NONE_CPP_DESTRUCTOR
#define VST2_FORMAT		0
#define VST3_FORMAT		1		// Not suported yet
#define CLAP_FORMAT		2

// Special isTypeOf that doesn't go to parent. to capture PatchPoint and not get an error with the frame. //** find another effcient method
#define CLASS_ISTYPEOF_SPECIFIC(name)             \
	virtual bool isTypeOf (const char* s) const \
		{ return (!strcmp (s, (#name))); } \

// We use macros instead of functions sometimes to avoid call overhead.
#define EXPRES(v1)	(v1*v1)
#define SCALE(x,a,b) ((x)*((b)-(a))+(a))
#define INVSCALE(v,a,b) (((v)-(a))/((b)-(a)))
#define CLIP(x,a,b) (((x)<(a))? (a):(((x)>(b))? (b):(x)))
#define AMPCLIP(x,a,b) ((x)<(a))? ((1-(a/x)*(a-x))*x):(((x)>(b))? ((1-(b/x)*(x-b))*x):(x))
#define QUADRCLIP(out,x,a,b,m)		\
{	float ftemp;					\
	if (x>m+b)						\
		out = b+0.5*m;				\
	else if (x<a-m)					\
		out = a-0.5*m;				\
	else if (x>b)					\
	{	ftemp=(x-b+m)/(2.0*m)-1; out = 0.5*m+b-2.0*m*ftemp*ftemp;	\
	}																\
	else if (x<a)													\
	{	ftemp=(a-x+m)/(2.0*m)-1; out = a-0.5*m+2.0*m*ftemp*ftemp;	\
	}								\
	else							\
		out = x;					\
}
#define MSBLSB(ms,ls) (((int)ms)<<7)+((int)ls)
#define GETMSB(msls) (((msls) & 0x3F80)>>7)				/* 0x3F80  = 11111110000000 */
#define GETLSB(msls) ((msls) & 0x7F)						/* 0x7F    = 00000001111111 */
#define FMOD(num,den) (((float) num/den) - ((int)(num/den)))*den
#define WETDRY(out,mix,wet,dry,amp)		\
{	if (mix>=0.5)					\
		out = amp*(1.5-mix)*((1-mix)*dry + mix*wet);			/*Original: (2-2*(ftemp-0.5))*/		\
	else																						\
		out = amp*(0.5+mix)*((1-mix)*dry + mix*wet);			/*Original: (2-2*(0.5-ftemp))*/		\
}

#define WETDRY_SIMPLE(out,mix,wet,dry) out = (1-mix)*dry + mix*wet

#define IS_ALMOST_DENORMAL(f) (((*(unsigned int *)&(f))&0x7f800000) < 0x08000000)

#define LED_CLIP(in,out,minl,maxl,clip_count,clip_time,lclip)			\
{	if (in>(maxl))														\
	{	out = (maxl); clip_count=clip_time; lclip->value=1.0;			\
	}																	\
	else if (in<(minl))													\
	{	out = (minl); clip_count=clip_time; lclip->value=1.0;			\
	}																	\
	else out=in;														\
}

#define LED_CLIP_COUNT(clip_count,lclip) if (clip_count==0) lclip->value=0.0; else clip_count--;

#define TAYLORSIN(out,x)						\
{	double t=(x/__PI-1)*3.14114830541267,t3;						\
	t3 = t*t*t;													\
	out = t3/6.0 - t3*t*t/120.0 + t3*t3*t/5040.0 - t;			\
	t3=t3*t3*t3;												\
	out -= t3/362880.0;							\
	out += t3*t*t/39916800.0;					\
}

// Park-Miller random generator
#define PMRAND() (zgen=(agen*zgen+cgen) % mgen,((double) zgen/mgen))
#define PMRANDZ(z) (z=(agen*z+cgen) % mgen,((double) z/mgen))
#define INT_PMRAND() (zgen=(agen*zgen+cgen) % mgen)

#define UNDENORM(unden)	(unden=-unden)
#define DENORM_VALUE	0.0000000000001
#define DENORMALIZE() (SCALE(PMRAND(),-DENORM_VALUE,DENORM_VALUE))
#define NO_DIV_ZERO_VALUE	0.00001

#undef MODULE_OVERSAMPLE
#ifdef MODULE_OVERSAMPLE
#defne IFMOVER(exp)	exp
// Per module oversampling settings. NOT completed yet. It has to be for each output patchpoint on the module
// I decided not to continue with this. It's not worth the effort.
typedef struct OversampleSet
{	float *ovin;				// taps array for FIR. Implemented as a circular Q
	float *cof;					// FIR coeficients.
	float overs_cut;			// LP Cutoff for FIR
	int iovin;					// Index in ovin where the most recent sample is.
	float bp,lp,bp_2,lp_2; //bp_3,lp_3,bp_4,lp_4;		// IIR variables
	int cofs;					// Nubmer of coeficients
	int overs_filter;			// Filter type.
	int overs;					// Oversampling factor. 2X, 4X, 8X, etc..
	int sovers;					// Number of real samples (not zero stuffs) in "overs" samples
	int overs_index;				// Oversampling factor menu index. (Just to make things a little faster) and more general for future uses.
};
#else
#define IFMOVER(exp)
#endif

// Stack definitions
template <typename TP>
struct cstack
{	TP *pbottom;
	int top;
};

template <typename TP>
void InitStack(cstack<TP> &stackv,int size);

template <typename TP>
void FreeStack(cstack<TP> &stackv);

#define PUSH(stackv,value) stackv.pbottom[++stackv.top]=value
#define POP(stackv) stackv.pbottom[stackv.top--]
#define FLUSHPOP(stackv) stackv.top--
#define NOTEMPTY(stackv) stackv.top!=-1


// DC blocker
struct dcblock
{	float in0,in1,out0;
};
#define DC_BLOCK_INIT(dcblk) { dcblk.in0=0; dcblk.in1=0; dcblk.out0=0; }
#define DC_BLOCK(dcblk,one_cut) dcblk.out0=dcblk.in0-dcblk.in1+one_cut*dcblk.out0;
#define DC_BLOCK_NEXT_SAMPLE(dcblk)	dcblk.in1=dcblk.in0;

#define SET_NO_MIDI(midi)	*(int*) &(midi) = 0

int gcd(int a, int b);
int IntPow(int base, int exp);
int IntLog(int v, int base);

enum
{	// Global
	kNumPrograms = 1,

	// Parameters Tags
	kNumParams = 1000,
	vscrolltag
};
class SynEditor : public CControlListener
{	
};

class SoloRack;

enum ModuleType
{	kMixerPanner		= 0,
	kFilter,
	kOscillatorSource,
	kAmplifier,
	kModulator,
	kModifierEffect,
	kClockGate,
	kSwitch,
	kCVSourceProcessor,
	kFromToDAW,
	kSequencer,
	kLogicBit,
	kMIDI,
	kOther,

	kModtypeCount
};


// Bandlimiting settings
enum BandlimitingSettings
{	kNoBandlimiting = -1,
	kAtOversamplingNyquist,
	kNearDAWNyquist,
	//kAtDAWNyquist								// Takes too much memmory and CPU at high oversampling. Might do it latter
};

// Clipping menu settings
enum ClipSettings
{	kNoClipLevel = -1,
	km12db = 0,
	km6db,
	km3db,
	k0db,
	k3db,
	k6db,
	k12db
	//kClipSettingsCount
};

class PatchPoint;
typedef struct Cable
{	Cable *llink;
	Cable *rlink;
	PatchPoint *pp1;
	PatchPoint *pp2;
	int colori;
	int voice;
};

typedef struct MIDIdata			// better be same size as float, because routing in processReplacing does'nt differentiate between protocols
{	char status;
	char data1;
	char data2;
	char data3;
};

template <typename FT>
struct Complex
{	FT re;			// Real part
	FT im;			// Imaginary part
};

//---------------------------------------------
// Patch Point Class
enum PatchPointType
{	ppTypeInput = 0,
	ppTypeOutput,
	ppTypeInOut,
	ppTypeUnused					// Used only with active_type variable to indicat the current type/function of an in/out port
};

enum PatchPointProtocol
{	ppAudioCV = 0,
	ppMIDI
};

class SynEditor;
class Product;
class Module;

//---------------------------------------------------
// External (3rd party) Dll modules stuff
struct DllInfo;
struct SynthComm;
typedef struct DllModule
{	
public:
	// SDK version used to develope the module
	int sdk_version;

	// Static members pointers. All these pointers has to point to functions. Not NULL.
	void (*InitializePtr)();
	void (*EndPtr)();
	Module *(*Constructor)(CFrame *pParent, CControlListener *listener, const SynthComm *psynth_comm, const int vvoice);
	const int (*GetTypePtr)();
	Product *(*ActivatePtr)(char *fullname, char *email, char *serial);
	bool (*IsActivePtr)(); 
	const char *(*GetProductNamePtr)();				// Returns a pointer to product name

	char *name;					// Module title name that will be displayed in the menu
	int name_len;				// Length of the name string excluding terminating NULL.
	int type;					// Module type
	bool active_hint;			// Read from the ini file. Indicates if the module is active, used for menu marks, usefull only before loading the Dll and module. Once the module is used/loaded. IsActive() function will be used instead.
	int order;					// Order number in ini file. (like M1, M2,,, M6 etc)
	DllInfo *parent_dll;		// Pointer to the dll structure containing that module
	CMenuItem *menu_item;		// Internal pointer to the menu item. Dlls Better not touch this
};

typedef struct DllInit
{	const char *skindir;		// SoloRack's Skin folder (path)
	const char *defskindir;		// SoloRack's Default Skin folder (path)
	long vp;					// Hight of one module
	float uiscale;				// UI Scale factor
};

typedef bool (*GetDllModulePtrType)(DllModule *mi, char *vendorname);
typedef bool (*DllInitializePtrType)(const DllInit *init);

typedef void* EventsHandle;
typedef void* EventHandle;

enum EventType
{	kMidiType = 1,
	kMidiSysExType = 6
};

#define INVALID_DINFO		DBL_MAX
enum DAWTimeflags
{	kTransportChanged       = 1,		// Set when play, loop or record state has changed
	kTransportIsPlaying     = 1 << 1,
	kTransportLoopIsActive  = 1 << 2,
	kTransportIsRecording   = 1 << 3,
	kAutomationIsWriting    = 1 << 6,	// Set when host is recording parameter changes)
	kAutomationIsReading    = 1 << 7,	// Set when host is playing parameter changes
};

typedef struct DAWTime
{	int flags;
	double tempo;					// Current tempo in BPM (Beats Per Minute)
	double tempo_inc;				// Tempo increment (or decrement when negative) per sample
	double ppq_pos;					// Musical Position in quarter note (1.0 equals 1 quarter note)
	double sample_pos;
	double last_bar_start_pos;		// Last bar start position in quarter notes
	double loop_start_pos;			// Loop start in quarter Notes
	double loop_end_pos;			// Loop end in quarter Notes
	int time_sig_num;				// Time signature numerator. (beats per bar)
	int time_sig_denom;				// Time signature denominator.
};


typedef struct DllInfo
{	GetDllModulePtrType GetDllModulePtr;
	DllInitializePtrType DllInitializePtr;
	char *filename;				// MAX_PATHSIZE
	HMODULE	dll_handle;			// Dll Module handle returned from LoadLibrary()
	char *vendorname;			// Module vendor name
	int vendorname_len;			// Length of the vendor name string excluding terminating NULL.
	int num_mods;				// number of module classes in thr dll
	DllModule *mods_info;
};

// SynthComm is a structure that allows modules to call functions in SoloRack. It's the "Module to SoloRack" interface.
class ModuleKnob;
class CMovieBitmapEx;
class SynEditor;
class SoloRack;
typedef struct SynthComm
{	friend SynEditor;

	private:
	SynEditor *peditor;

	public:
	int (*GetSynthSDKVersion)();
	SoloRack *(*GetSynth)(SynEditor *tthis);
	SynEditor *GetEditor() { return peditor; }
	int (*GetOversamplingFactor)(SynEditor *tthis);
	Module *(*GetVoiceModule)(SynEditor *tthis, Module *mod, int voice);
	int (*GetPolyphony)(SynEditor *tthis);
	bool (*ClearIfOrphaned)(Module *mod);
	int (*GetDAWBlockSize)(SynEditor *tthis);
	float (*GetDAWSampleRate)(SynEditor *tthis);

	// Events Handling (For events sent from SoloRack to modules)
	int (*GetNumberOfEvents)(EventsHandle handle);
	EventHandle *(*GetEventsArray)(EventsHandle handle);
	int (*GetEventType)(EventHandle handle);					// See EventType
	int (*GetEventTime)(EventHandle handle);					// Relative time in DAW samples. Often called "Delta Frames"
	char *(*GetMIDIEventMessage)(EventHandle handle);			// This returns char[4]. An 4 byte array containing the MIDI message
	int (*GetMIDISysExSize)(EventHandle handle);				// Size of SysEx message in bytes
	char *(*GetMIDISysExMessage)(EventHandle handle);

	bool (*SendEventsToDAW)(SynEditor *tthis, EventsHandle handle, void *callback);
	
	bool (*GetDAWTime)(SynEditor *tthis, DAWTime *pt);

	
	// PatchPoint
	CMouseEventResult (*PPOnMouseDownPtr)(PatchPoint *tthis, CPoint& where, const long& buttons);
	CMouseEventResult (*PPOnMouseUpPtr)(PatchPoint *tthis, CPoint& where, const long& buttons);
	CMouseEventResult (*PPOnMouseMovedPtr)(PatchPoint *tthis, CPoint& where, const long& buttons);
	CMessageResult (*PPNotify)(PatchPoint *tthis, CBaseObject* sender, const char* message);

	CMouseEventResult (*ModuleKnobOnMouseDownPtr)(ModuleKnob *tthis, CPoint &where, const long& buttons);
	CMouseEventResult (*ModuleKnobOnMouseUpPtr)(ModuleKnob *tthis, CPoint &where, const long& buttons);
	CMouseEventResult (*ModuleKnobOnMouseMovedPtr)(ModuleKnob *tthis, CPoint &where, const long& buttons);

	bool (*CMovieBitmapExIsDirtyPtr)(const CMovieBitmapEx *tthis);
	
	// Module
	CMouseEventResult (*ModuleOnMouseDownPtr)(Module *tthis, CPoint &where, const long& buttons);
	CMouseEventResult (*ModuleOnMouseUpPtr)(Module *tthis, CPoint &where, const long& buttons);
	CMouseEventResult (*ModuleOnMouseMovedPtr)(Module *tthis, CPoint &where, const long& buttons);

	long (*ModuleGetFreeTagPtr)(Module *tthis);
	bool (*ModuleRegisterTagPtr)(Module *tthis, CControl *pcon, long tag);
	bool (*ModuleUnRegisterTagPtr)(Module *tthis, CControl *pcon);
	bool (*ModuleCallProcessEventsPtr)(Module *tthis);
	bool (*ModuleCallStartOfBlockPtr)(Module *tthis);
	bool (*ModuleCallMouseObservePtr)(Module *tthis);
	bool (*ModuleUnRegisterOldTagPtr)(Module *tthis, CControl *pcon,long oldtag, long newtag);

	void (*ModuleSendAudioToDAW1Ptr)(Module *tthis, float left, float right);
	void (*ModuleSendAudioToDAW2Ptr)(Module *tthis, PatchPoint **pps_outputs);
	void (*ModuleSendAudioToDAW3Ptr)(Module *tthis, PatchPoint **pps_outputs, int first_output);
	void (*ModuleSendAudioToDAW4Ptr)(Module *tthis, float *outputs, int last_output);
	void (*ModuleSendAudioToDAW5Ptr)(Module *tthis, float *outputs, int first_output, int last_output);

	void (*ModuleReceiveAudioFromDAW1Ptr)(Module *tthis, float *left, float *right);
	void (*ModuleReceiveAudioFromDAW2Ptr)(Module *tthis, PatchPoint **pps_inputs);
	void (*ModuleReceiveAudioFromDAW3Ptr)(Module *tthis, PatchPoint **pps_inputs, int first_input);
	void (*ModuleReceiveAudioFromDAW4Ptr)(Module *tthis, float *inputs, int last_input);
	void (*ModuleReceiveAudioFromDAW5Ptr)(Module *tthis, float *inputs, int first_input, int last_input);

	int (*ModuleGetNumberOfAudioFromDAWPtr)(Module *tthis);
	int (*ModuleGetNumberOfAudioToDAWPtr)(Module *tthis);
	void (*ModuleEnterProcessingCriticalSectionPtr)(Module *tthis);
	void (*ModuleLeaveProcessingCriticalSectionPtr)(Module *tthis);

	// For Clap. Mainly to support poly modulation
	// Index here is the last data char of the MIDI note ON message. This is injected by SoloRack. in the MIDI message
	bool (*SetVoiceUsed)(SynEditor *tthis, int voice, int index);
	bool (*UnSetVoiceUsed)(SynEditor *tthis, int voice, int index);
	short (*GetPluginFormat)(SynEditor *tthis);
};

// Park miller random generator
unsigned int const agen=16807,cgen=0,mgen=2147483648;
extern unsigned int zgen;

inline void SetSeed(unsigned int seed)
{
	zgen = seed;
}

//__inline double PMRand()
//{	// Note: for our a,c,m values, it will never return exactly 0!!
//	// Park miller random generator
//
//	zgen=(a*zgen+c) % m; 
//	return ((double) zgen/m); 
//}

__inline int GenRand(int smallest, int largest)
{	// Generates a random integer between smallest and largest
	
	return PMRAND()*(largest-smallest+1)+smallest;
}

__inline double DGenRand(double smallest, double largest)
{	// Generates a random double between smallest and largest. Result can be smallest But NEVER reaching largest
	
	return SCALE(PMRAND(),smallest,largest);
}

inline long FRound(float r)
{	long i = (long) r;
	if (r-i>0.5) return i+1; else return i;
}

inline long FRound(double r)
{	long i = (long) r;
	if (r-i>0.5) return i+1; else return i;
}

//--------------------------------------------------
class ProductWrapper
{	
friend class SoloRack;
public:
	//Product::Product(void *parameter=NULL, int opt=1, Product *pparent=NULL, bool isactive=false, char* prname=NULL);
	//virtual Product::~Product();
	virtual char *GetName()=0;
};

class Product : public ProductWrapper
{	// This just an empty template class. You can create your own implementation here
	// It doesn't matter even if you completly change every thing, or add your own functions and data members
	// SoloRack doesn't depend on this class. This should be internal to your module

public:
	Product::Product(void *parameter=NULL, int opt=1, Product *pparent=NULL, bool isactive=false, char* prname=NULL);
	virtual ~Product();
	virtual char *GetName() { return name; }
	virtual void SetName(char* prname);
	virtual Product *Activate(char *fullname, char *email, char *serial);

private:
	char *name;
	int name_len;
};

//class PatchPointWrapper : public CMovieBitmap
//{	
//friend class Module;
//public:
//	PatchPointWrapper (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
//	//PatchPointWrapper (const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint (0, 0));
//	//PatchPointWrapper (const CMovieBitmap& movieBitmap);
//
//	virtual ~PatchPointWrapper()=0;
//};
//--------------------------------------------------
class PatchPoint : public CMovieBitmap
{
friend class Module;
public:
	PatchPoint::PatchPoint(const CRect& size, CControlListener* listener, long tag, CBitmap* background, int pptype);
	PatchPoint::PatchPoint(const CRect& size, CControlListener* listener, long tag, CBitmap* background, int pptype, char center_offset_x, char center_offset_y);
	void SetCenterOffset(char x, char y);
	virtual CMouseEventResult onMouseDown (CPoint& where, const long& buttons);
	CMouseEventResult onMouseMoved (CPoint &where, const long &buttons);
	CMouseEventResult onMouseUp (CPoint &where, const long &buttons);
	//virtual CMouseEventResult TTT1(CPoint &where, const long& buttons);

	// The following are used for Dll modules.
	static CMouseEventResult ClassOnMouseDown(PatchPoint *tthis, CPoint& where, const long& buttons);
	//static CMouseEventResult ClassOnMouseMoved (PatchPoint *tthis, CPoint &where, const long &buttons);
	static CMouseEventResult ClassOnMouseUp(PatchPoint *tthis, CPoint &where, const long &buttons);

	CMessageResult notify(CBaseObject* sender, const char* message);

	// The following is used for Dll modules.
	static CMessageResult ClassNotify(PatchPoint *tthis, CBaseObject* sender, const char* message);

	void SetType(int pptype);
	void SetProtocol(int ppprotocol);
	long GetPPTag() { return pptag; }

	//CLASS_ISTYPEOF_SPECIFIC(PatchPoint)						//** think of way to do this
	CLASS_METHODS(PatchPoint, CMovieBitmap)

	int type;
	int active_type;			// Used only for in/out type
	int protocol;				// CV, MIDI or any other future protocol like USB.
	union //TINU
	{	float in;				// If type is input
		MIDIdata midi_in;
	}; // in;
	union //TOUTU
	{	float out;				// If type is output
		MIDIdata midi_out;
	}; // out;
	Cable *pcable;				// Used to remove a cable from the cables linked list. Only for input type.
	float *cable_in;			// If type is input. Should point to another module's output
	int num_cables;				// Number of cables connected to this patch point.
	char coff_x;				// Center Offset X. Used to correct patch cable position
	char coff_y;				// Center Offset Y. Used to correct patch cable position
	PatchPoint *pnext;			// Used to customize polyphony connections. Points to a patch point in the same module that is next in a chain to be connectted to higher voices. Instead of connecting to the same patchpoint. Currently used for the poly chainer
								// By default, pnext will point to the 'this' PatchPoint it self. So higher voices will simply be connected to the same patchpoint
	bool force_mono;			// Used to force a patch point to be mono regardless if the module was mono or not.

private:
	SynEditor *peditor;
	long pptag;			// Tag used when saving to identify patch points for cabling.

};

//class PatchPoint2 : public PatchPoint
//{	
//friend class Module;
//public:
//	PatchPoint2(const CRect& size, CControlListener* listener, long tag, CBitmap* background, int pptype);
//	CMouseEventResult onMouseDown (CPoint &where, const long& buttons);
//	//PatchPointWrapper (const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint (0, 0));
//	//PatchPointWrapper (const CMovieBitmap& movieBitmap);
//	CMouseEventResult TTT1(CPoint &where, const long& buttons);
//	//virtual ~PatchPointWrapper()=0;
//};


//---------------------------------------------
// Module Knob Class - for smooth tweaking
class ModuleKnob : public CAnimKnob
{
public:
	ModuleKnob(const CRect& size, CControlListener* listener, long tag, CBitmap* background, Module *parent, const CPoint& offset = CPoint (0, 0));
	ModuleKnob(const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, Module *parent, const CPoint& offset = CPoint (0, 0));
	ModuleKnob(const ModuleKnob& knob);
	CMouseEventResult onMouseDown(CPoint& where, const long& buttons);
	CMouseEventResult onMouseMoved(CPoint& where, const long& buttons);

	static CMouseEventResult ClassOnMouseDown(ModuleKnob *tthis, CPoint& where, const long& buttons);
	static CMouseEventResult ClassOnMouseMoved(ModuleKnob *tthis, CPoint& where, const long& buttons);
	//void  setValue (float val) { value = val; }

	//float GetSValue() const { return svalue; }
	void setValue(float val);
	void UpdateQValue();

	// Some of the following smoothing functions are very often used. So it's worth trying to optimize them
	// and have different versions, each better for different cases instead of one general purpose function

	// Update Smoothed movment value
	inline void UpdateSValue()
	{	float a = value-svalue;
		if (a!=0.0)
		{	// Guanranteed not to skip the range as long as SMOOTH_DELAY time (or the delay set by SetSmoothDelay()) is more than 1000/sampling_rate time.
			// This is true no matter what the range is because the maximum change will be a/1 //** check again
			if (a<0.001 && a>-0.001) { svalue=value; return; }
			
			svalue+=a*smooth_samples_1;		// smooth_samples_1 = 1/smooth_samples. 
		}
	}

	inline void UpdateSValue(float const half_margin)
	{	float a = value-svalue;
		if (a!=0.0)
		{	// The lower half_margin is, the smoother the stop happens. This will not be apperent with low delay
			// By with high smoothing delay, it will make a difference.

			// To prevent skipping the margin, the following has to be met:
			// half_margin > a*1000/(samplw_rate*delay)-a
			// The caller must insure that. Otherwise svalue may go up and down fluctuating!!
			if (a<half_margin && a>-half_margin) { svalue=value; return; }
			
			svalue+=a*smooth_samples_1;
		}
	}

	// Another version that returns bolean to indicate whither or not value has already been reached
	inline bool UpdateSValueReached()
	{	// Update Smoothed movment value
		//float sv_change2;
		float a = value-svalue;
		if (a!=0.0)
		{	if (a<0.001 && a>-0.001) { svalue=value; return false; }		//if abs(a)< (a<=0.001 && a>=-0.001)
			//sv_change2=a/1000.0;
			//float b = abs(a);
			//while (b<=sv_change2) 
			//{	sv_change2/=2; 
			//	if (b<=0.000001) { svalue=value; sv_change2=sv_change; return; }
			//}
			svalue+=a*smooth_samples_1;
			return false;
		} else return true;
	}

	// Update Smoothed movment value relative to the quantized value
	inline void UpdateQuantizedSValue()
	{	float a = qvalue-svalue;
		if (a!=0.0)
		{	if (a<0.001 && a>-0.001) { svalue=qvalue; return; }
			//sv_change2=a/1000.0;
			//float b = abs(a);
			//while (b<=sv_change2) 
			//{	sv_change2/=2; 
			//	if (b<=0.000001) { svalue=value; sv_change2=sv_change; return; }
			//}
			svalue+=a*smooth_samples_1;
		}
	}

	// Update Smoothed movment value relative to the quantized value. Returns true if qvalue has been reached
	inline bool UpdateQuantizedSValueReached()
	{	float a = qvalue-svalue;
		if (a!=0.0)
		{	if (a<0.001 && a>-0.001) { svalue=qvalue; return false; }
			//sv_change2=a/1000.0;
			//float b = abs(a);
			//while (b<=sv_change2) 
			//{	sv_change2/=2; 
			//	if (b<=0.000001) { svalue=value; sv_change2=sv_change; return; }
			//}
			svalue+=a*smooth_samples_1;
			return false;
		} else return true;
	}

	// GetCurrentStep() simply gives the index of the currently shown sub-image/bitmap. This is usefull if the knob is STEPPING and is meant to be a selector of several choices.
	// This function uses float to integer convertion which is usually heavy on CPU, so better not use it at audio rate. A good place to use it is ValueChanged().
	inline long GetCurrentStep()
	{	//return qvalue*(subPixmaps - 1)+error_correction;
		// First step is ZERO (not 1)

		long temp = subPixmaps-1;
		if (bInverseBitmap)
			return (long) ((1.0-value)*(float)temp);
		else
			return (long) (value*(float)temp);
	}

	// The following smoothing macros have either sm_sam or sm_sam_1 variables
	// sm_sam = 1/sm_sam_1 = smoothing delay in samples

	// Just like the above functions but in macro form. Smooth using exponential decay (Well not really exponential)
	#define ExpUpdateSValue(value,svalue,sm_sam_1)		\
	{	float a = value-svalue;							\
		if (a!=0.0)										\
		{	if (a<0.001 && a>-0.001) svalue=value;		\
			else svalue+=a*sm_sam_1;					\
		}												\
	}
	#define ExpUpdateSValueQuick(value,svalue,sm_sam_1)		\
	{	svalue += (value-svalue)*sm_sam_1;					\
	}
	// Same as above but uses division instead of mutiplication
	#define ExpUpdateSValueDiv(value,svalue,sm_sam)		\
	{	float a = value-svalue;							\
		if (a!=0.0)										\
		{	if (a<0.001 && a>-0.001) svalue=value;		\
			else svalue+=a/sm_sam;						\
		}												\
	}

	// Smooth using constant Rate (not being used so far)
	#define RateUpdateSValue(value,svalue,sm_sam)			\
	{	/*Update Smoothed movment value*/				\
		float a = (value)-svalue;						\
		if (a!=0.0)										\
		{	if (a>0.001) svalue+=1.0/(sm_sam);			\
			else if (a<-0.001) svalue-=1.0/(sm_sam);	\
			else svalue=(value);						\
		}												\
	}

	//// Smooth Using Constant rate (Original)
	//#define ZRateUpdateSValue(value,svalue,sm_sam)			\
	//{	if ((sm_sam)!=0.0)									\
	//	{	float a = (value)-svalue,b;						\
	//		if (a!=0.0)										\
	//		{	b=1.0/(sm_sam);				\
	//			if (a>b) svalue+=b;			\
	//			else if (a<b) svalue-=b;	\
	//			else svalue=(value);						\
	//		}												\
	//	} else svalue=(value);								\
	//}

	// Smooth using Constant rate
	#define ZRateUpdateSValue(value,svalue,sm_sam)			\
	{	if ((sm_sam)!=0.0)									\
		{	float a = (value)-svalue,b;						\
			if (a>0.0)						\
			{	b=1.0/(sm_sam);				\
				if (a>b) svalue+=b;			\
				else svalue=(value);		\
			} else							\
			if (a<0.0)						\
			{	b=-1.0/(sm_sam);			\
				if (a<b) svalue+=b;			\
				else svalue=(value);		\
			}								\
		} else svalue=(value);				\
	}

	// Smooth using Constant time
	#define ZTimeUpdateSValue(value,svalue,sm_sam,sm_sam2)		\
	{	if ((sm_sam)!=0.0)									\
		{	/*Update Smoothed movment value*/				\
			float a = (value)-svalue;						\
			if (a!=0.0)										\
			{	float inc=a/(sm_sam2); 						\
				if ((a>0 && a<=inc) || (a<0 && a>=inc)) svalue=(value);		/*assuming a and b are always the same sign*/	\
				else svalue+=inc;											\
			}															\
		} else svalue=(value);								\
	}

	//// Used with the above macro. Must be called only when value has changed
	//#define ZTimeCalcInc(value,svalue,sm_sam2,inc,prevalue)		\
	//{	float a = (value)-svalue;					\
	//	inc=a/(sm_sam2); prevalue=value;				\
	//}

	//CMessageResult notify(CBaseObject* sender, const char* message);
	void SetSmoothDelay(float del, Module *parent = NULL);

	inline void InitValue(float v) { value=v; svalue=v; }

	double svalue;		// Smoothed movment value. I will define this as public to allow direct access if you want to avoid the overhead of the GetSValue()
	float qvalue;		// Quantized value. If the knob is a stepping knob
	float mvalue;		// Modulation value. This was added latter to support CLAP poly modulation. This gets added to svalue which is final value.
	bool is_stepping;	// is or not stepping knob. The number of steps is equal to the number of sub bitmaps (subPixmaps)
	float smooth_samples_1;		// Definition changed since SDK version 0.12 to be RESIPROCAL of smooth samples delay instead of smooth samples delay. 
								// This will avoid using division in UpdateSValue()
	float delay;
	//CVSTGUITimer timer;

	//CLASS_ISTYPEOF_SPECIFIC(ModuleKnob)
	CLASS_METHODS(ModuleKnob, CAnimKnob)
};

// Smooth using exponential decay (Well not really exponencial)
inline bool ExpUpdateSValueReached(const double value,double &svalue,double sm_sam_1)
{	double a = value-svalue;
	if (a!=0.0)	
	{	if (a<0.001 && a>-0.001) { svalue=value; return true; }
		else svalue+=a*sm_sam_1;
		return false;
	}
	else return true;
}


//---------------------------------------------
// Extended Module Knob Class - Has a extra feature. Currently: a pointer to Attached control
class ModuleKnobEx : public ModuleKnob
{
public:
	ModuleKnobEx(const CRect& size, CControlListener* listener, long tag, CBitmap* background, Module *parent, const CPoint& offset = CPoint (0, 0));
	ModuleKnobEx(const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, Module *parent, const CPoint& offset = CPoint (0, 0));
	ModuleKnobEx(const ModuleKnobEx& knob);
	CMouseEventResult onMouseMoved(CPoint& where, const long& buttons);
	CMouseEventResult onMouseDown(CPoint& where, const long& buttons);
	//void  setValue (float val) { value = val; }

	bool invalidate;
	CControl *attached1;	// Attached Control
	int tag1;				// Used for something like index or etc..
	bool auto_zoom;

	//CLASS_ISTYPEOF_SPECIFIC(ModuleKnob)
	CLASS_METHODS(ModuleKnobEx, ModuleKnob)
};


/* This class is solely meant for CPU performance. When you have too many knobs in your module that require 
   smoothing. It is more CPU efficient to use this pool class than the UpdateSValue() functions.
*/
template <int num>
class ModuleKnobExPool
{	
public:
	ModuleKnobEx *change_pool[num];	
	int last_changed;
	int last_knob;

	ModuleKnobExPool()
	{	int i;
		last_changed=-1; last_knob=-1;
		for (i=0; i<num; i++) change_pool[i]=NULL;
	}

	// This sohuld be called in costructors or so.
	bool AddPoolKnob(ModuleKnobEx *pknob)
	{	if (last_knob+1<num)
		{	last_knob++; change_pool[last_knob]=pknob; 
			pknob->tag1=0; return true;
		}
		else return false;
	}

	// The following function should be called in ProccessSample()
	inline void UpdatePoolSValues()
	{	int i=0, lc=last_changed;
		ModuleKnobEx *tknob;
		while (i<=lc)
		{	tknob = change_pool[i];
			if (tknob->UpdateSValueReached())
			{	tknob->tag1=0;
				change_pool[i]=change_pool[lc]; lc--;
			}
			else i++;
		}			
		last_changed=lc;	
	}

	bool IsKnobInPool(CControl *pcontrol)
	{	int i=0, lc=last_knob;
		while (i<=lc)
		{	if (pcontrol==change_pool[i])
				return true;
			else i++;
		}
		return false;
	}

	// The following two functions should be called whenever any of the knobs in the pool change in value. 
	// Typicaly called in ValueChanged()

	// Important: You must call EnterProcessingCriticalSection() before calling this.
	// Dont forget to LeaveProcessingCriticalSection() after that.
	void PoolKnobValueChanged(CControl *pcontrol)
	{	ModuleKnobEx *tknob = (ModuleKnobEx *) pcontrol;
		if (tknob->tag1!=1)
		{	tknob->tag1=1; last_changed++; change_pool[last_changed]=tknob;
		}
	}

	// This is the same as PoolKnobValueChanged() but it will automatically Enter/Leave
	// The audio critical section for you
	void LockPoolKnobValueChanged(CControl *pcontrol, Module *mod)
	{	mod->EnterProcessingCriticalSection();
		ModuleKnobEx *tknob = (ModuleKnobEx *) pcontrol;
		if (tknob->tag1!=1)
		{	tknob->tag1=1; last_changed++; change_pool[last_changed]=tknob;
		}
		mod->LeaveProcessingCriticalSection();
	}
};



//---------------------------------------------
// Extended CSpecialDigit Class - Has a extra feature. Currently: a pointer to Attached control
class CSpecialDigitEx : public CSpecialDigit
{
public:
	CSpecialDigitEx(const CRect& size, CControlListener* listener, long tag, long dwPos, long iNumbers, long* xpos, long* ypos, long width, long height, CBitmap* background, CBitmap* pblank=NULL);
	CSpecialDigitEx(const CSpecialDigitEx& digit);
	void draw (CDrawContext *pContext);
	//CMouseEventResult onMouseMoved(CPoint& where, const long& buttons);
	//void  setValue (float val) { value = val; }

	//bool invalidate;
	CControl *attached1;	// Attached Control
	int tag1;				// Used for something like index or etc..
	CBitmap* blank;

	//CLASS_ISTYPEOF_SPECIFIC(ModuleKnob)
	CLASS_METHODS(CSpecialDigitEx, CSpecialDigit)
};


//---------------------------------------------
// Extended CSpecialDigit Class - Has a extra feature. Currently: a pointer to Attached control
class CKickButtonEx : public CKickButton
{
public:
	CKickButtonEx (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CKickButtonEx (const CRect& size, CControlListener* listener, long tag, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CKickButtonEx (const CKickButtonEx& kickButton);

	virtual CMouseEventResult onMouseDown (CPoint& where, const long& buttons);
	virtual CMouseEventResult onMouseUp (CPoint& where, const long& buttons);
	//virtual CMouseEventResult onMouseMoved (CPoint& where, const long& buttons);
	
	//bool invalidate;
	CControl *attached1;	// Attached Control
	int tag1;				// Used for something like index or etc..
	bool immediate_valuechanged;			// Force a ValueChanged() to be called imidiatly after a mouse down and a mouse up, instead of the combined method in CKickButton when mouse is up.

	CLASS_METHODS(CKickButtonEx,CKickButton)
};


////---------------------------------------------
//// Extended COnOffButton Class - Has a extra feature. Currently: tag1 and a pointer to Attached control
//class COnOffButtonEx : public COnOffButton
//{
//public:
//	COnOffButtonEx (const CRect& size, CControlListener* listener, long tag, CBitmap* background, long style = kPreListenerUpdate);
//	COnOffButtonEx (const COnOffButton& onOffButton);
//	
//	CControl *attached1;	// Attached Control
//	int tag1;				// Used for something like index or etc..
//
//	CLASS_METHODS(COnOffButtonEx, COnOffButton)
//};


//-----------------------------------------------------------------------
// Extended CMovieBitmap. better for values that change slowly by smoothly in an none stepy fashion
// Must be parented directly by a module

#undef USE_NEW_OLD_VERTICAL
#define LED_DIF_THREASH (0.5/LED_RED_IMAGES)

class CMovieBitmapEx : public CMovieBitmap
{
public:
	CMovieBitmapEx (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CMovieBitmapEx (const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CMovieBitmapEx (const CMovieBitmapEx& movieBitmap);

	//virtual	~CMovieBitmapEx ();

	virtual void draw (CDrawContext*);
	#ifdef USE_NEW_OLD_VERTICAL
	bool isDirty () const;
	#else
	bool isDirty () const;
	#endif

	static bool ClassIsDirty(const CMovieBitmapEx *tthis);

	CLASS_METHODS(CMovieBitmapEx, CMovieBitmap)

	bool dirty_false_after_draw;	// probably will be deprecated
	
	#ifdef USE_NEW_OLD_VERTICAL
	CCoord old_vertical;				// The vertical point of the image at which the old value was drawn
	#endif
	long subPixmaps_1;
	int update_ticks;					// Custome update rate. How many calles to isdirty is skiped untill a return of true is allowed.
	int update_count;
};


//-----------------------------------------------------------------------------
// Extended CTextLabel Declaration
//-----------------------------------------------------------------------------
#define CTEXTLABEL_MAXTEXT_SIZE			1000000

class CTextLabelEx : public CTextLabel
{
public:
	CTextLabelEx (const CRect& size, const char* txt = 0, CBitmap* background = 0, const long style = 0);
	CTextLabelEx (const CTextLabelEx& textLabel);
	
	~CTextLabelEx();
	
	virtual void setText(const char* txt);
	virtual void SetTextPointer(const char* txt, int arsize);
	virtual bool AppendText(const char* txt, bool ignore_limit=false);				
	virtual void FindLineOffsets();	
	virtual void FindLineOffsets(int start);	
	virtual void SetCurrentLine(int line); 
	virtual int GetCurrentLine() { return cur_line; }
	virtual int GetLastLine() { return last_line; }
	virtual void SetMaxDrawLines(int maxl) { max_draw_lines = maxl; }
	void SetMaxTextSize(unsigned long size) { max_text_size=size; }
	void SetMaxCharsPerLine(unsigned long maxc) { max_chars_per_line=maxc; }
	
	virtual	void draw (CDrawContext* pContext);
	virtual void freeText();
	void InitCritSec();
	void DelCritSec();
	void EnterCritSec();
	void LeaveCritSec();

	CLASS_METHODS(CTextLabelEx, CTextLabel)

protected:
	int *line_offsets;
	int line_offsets_size;
	int last_line;				// last index in line_offsets array
	int cur_line;
	int max_draw_lines;
	unsigned long max_chars_per_line;		// Naive word wrap. Replaces spaces with \n.
											// FindLineOffsets() has to be called (without parameters) at least once for this to work
	unsigned long max_text_size;
	CRITICAL_SECTION critsec;
	bool use_critsec;
};


////-----------------------------------------------------------------------------
//// CFileSelectorEx Declaration
//// Extension for the CFileSelector. adds default filter index
////-----------------------------------------------------------------------------
//class CFileSelectorEx : public CFileSelector
//{
//public:
//	CFileSelectorEx(void* ptr);
//	//virtual ~CFileSelector ();
//
//	virtual long run(VstFileSelect *vstFileSelect, int filter_index = 1);
//};


//---------------------------------------------
// Base Module Class Wrapper!!. Just to make Dlls work.
#include "ModuleWrapper.h"


//---------------------------------------------
// Base Module Class
struct SynthComm;
class Module : public ModuleWrapper
{
friend class SynEditor;
friend class SoloRack;
friend class ModuleKnob;
friend class CMovieBitmapEx;
friend class PatchPoint;
public:
	virtual int GetSDKVersion();
	Module::Module(const CRect &size, CFrame *pParent, CBitmap *pBackground, const SynthComm *psynth_comm, const int vvoice=0);
	//virtual Module* VConstructor(CFrame *pParent, CControlListener *listener,const SynthComm *psynth_comm);
	Module::~Module();
	static void Module::Initialize();
	static void Module::End();
	virtual bool forget();

	CMouseEventResult onMouseDown (CPoint &where, const long& buttons);
	CMouseEventResult onMouseUp (CPoint &where, const long& buttons);
	CMouseEventResult onMouseMoved (CPoint &where, const long& buttons);
	virtual CMouseEventResult OnMouseMovedObserve(CPoint &where, const long& buttons);		// This is NOT in VSTGUI. Its for SoloRack

	// These are used for Dll modules.
	static CMouseEventResult ClassOnMouseDown(Module *tthis, CPoint &where, const long& buttons);
	static CMouseEventResult ClassOnMouseUp(Module *tthis, CPoint &where, const long& buttons);
	static CMouseEventResult ClassOnMouseMoved(Module *tthis, CPoint &where, const long& buttons);
	static bool TryEmptySpace(Module *tthis, const CPoint &where, const CPoint &po, const CRect &parect, bool right);
	static void SlideMultipleModules(Module *tthis, long &x, const CPoint &po, const CRect &parect, short right);
	static bool FixBadPosition(Module *tthis, const long& buttons);

	void PutLeftScrews(CMovieBitmap *&top_screw,CMovieBitmap *&bottom_screw, CControlListener *listener);
	void PutRightScrews(CMovieBitmap *&top_screw,CMovieBitmap *&bottom_screw, CControlListener *listener);
	
	void InitPatchPoints(float init);
	virtual void ProcessSample();							// Main audo processing function
	virtual const char *GetName();
	virtual const int GetNameLen();
	virtual const char *GetVendorName();
	virtual const int GetVendorNameLen();
	static const int GetType();								// This is menu category in wich the module is put under
	long GetFreeTag();										
	bool RegisterTag(CControl *pcon, long tag);
	bool UnRegisterTag(CControl *pcon);
	static long ClassGetFreeTag(Module *tthis);
	static bool ClassRegisterTag(Module *tthis, CControl *pcon, long tag);
	static bool ClassUnRegisterTag(Module *tthis, CControl *pcon);
	//void ForceUnRegisterTag(CControl *pcon, long tag);
	bool CallProcessEvents();
	bool CallStartOfBlock();
	bool CallMouseObserve();
	bool ClearIfOrphaned();
	static bool ClassCallProcessEvents(Module *tthis);
	static bool ClassCallStartOfBlock(Module *tthis);
	static bool ClassCallMouseObserve(Module *tthis);
	bool UnRegisterOldTag(CControl *pcon,long oldtag, long newtag);
	static bool ClassUnRegisterOldTag(Module *tthis, CControl *pcon,long oldtag, long newtag);
	virtual bool GetAlwaysONDefault() { return false; }	
	virtual bool GetIsMonoDefault() { return true; }	
	virtual bool IsAudioToDAW() { return false; }
	virtual bool IsPolyManager() { return false; }
	void SendAudioToDAW(float left, float right);
	void SendAudioToDAW(PatchPoint **pps_outputs);
	void SendAudioToDAW(PatchPoint **pps_outputs, int first_output);
	void SendAudioToDAW(float *outputs, int last_output);
	void SendAudioToDAW(float *outputs, int first_output, int last_output);

	void ReceiveAudioFromDAW(float *left, float *right);
	void ReceiveAudioFromDAW(PatchPoint **pps_inputs);
	void ReceiveAudioFromDAW(PatchPoint **pps_inputs, int first_input);
	void ReceiveAudioFromDAW(float *inputs, int last_input);
	void ReceiveAudioFromDAW(float *inputs, int first_input, int last_input);

	static void ClassSendAudioToDAW(Module *tthis, float left, float right);
	static void ClassSendAudioToDAW(Module *tthis, PatchPoint **pps_outputs);
	static void ClassSendAudioToDAW(Module *tthis, PatchPoint **pps_outputs, int first_output);
	static void ClassSendAudioToDAW(Module *tthis, float *outputs, int last_output);
	static void ClassSendAudioToDAW(Module *tthis, float *outputs, int first_output, int last_output);

	static void ClassReceiveAudioFromDAW(Module *tthis, float *left, float *right);
	static void ClassReceiveAudioFromDAW(Module *tthis, PatchPoint **pps_inputs);
	static void ClassReceiveAudioFromDAW(Module *tthis, PatchPoint **pps_inputs, int first_input);
	static void ClassReceiveAudioFromDAW(Module *tthis, float *inputs, int last_input);
	static void ClassReceiveAudioFromDAW(Module *tthis, float *inputs, int first_input, int last_input);

	int GetNumberOfAudioFromDAW();
	int GetNumberOfAudioToDAW();
	static int ClassGetNumberOfAudioFromDAW(Module *tthis);
	static int ClassGetNumberOfAudioToDAW(Module *tthis);
	virtual void ProcessEvents(const EventsHandle ev);
	virtual void StartOfBlock(int sample_frames);

	virtual void CableConnected(PatchPoint *pp);
	virtual void CableDisconnected(PatchPoint *pp);
	virtual void ValueChanged(CControl* pControl);
	void CountControlsAndPatchPoints();

	void ResetNbControlsAndPatchPoints();							// Must be called if number of controls in module changes for GetControlsValuesSize to work properly
	void SetPPTags(long &ctag);
	void ConstructTags2PP(PatchPoint **tags2pp, long &ctag, int nb_pp);
	int GetControlsValuesSize();
	void ZeroDAWModValues();
	void SaveControlsValues(void *pdata);
	void LoadControlsValues(void *pdata, int size);
	int GetControlsTagsSize();
	void SaveControlsTags(void *pdata);
	void LoadControlsTags(void *pdata, int size);
	void UnRegisterControlsTags();
	virtual int GetPresetSize();
	virtual void SavePreset(void *pdata, int size);
	virtual void LoadPreset(void *pdata, int size, int version);
	virtual void SetSampleRate(float sr);				// Will not be auto called when a module is first created. sample_rate should be first set by the contructor (which already done by default)
														// Will only be called when sample rate changes.
														// Your version of SetSampleRate() is responsible for calling SetBandLimit() OR accounting for bandlimit value within it (if you require it by your module). Solorack will not autocall SetBandLimit() for you
														// I do this because SetBandLimit() may include memory allocation for things like Minblep, etc. Which can trigger multiple/redundant allocations if both functions are called when a preset is loaded.
	virtual void SetDAWSampleRate(float daw_sr) { DAW_sample_rate = daw_sr; }			// SetSampleRate() will be imidiatly called after SetDAWSampleRate().
	virtual void SetDAWBlockSize(float blocksize);				// Called when ever DAW block size changes. This usually not neccesary to be implemented except for modules that need to know the DAW block size
	//virtual void SetBlockSize(int bs);
	virtual void SetKnobsSmoothDelay(float del);
	virtual int GetVersion() { return -1; }				// -1 means no version is specified. But modules better override this with a real version number
	virtual bool SetBandLimit(int bndlim);				// Will only be called by solorack when bandlimit is changed/set by the user.
	static Product *Activate(char *fullname, char *email, char *serial);		// Activate Licence
	//Deactivate() { is_active=false; }					// But will stay active if parent product is active
	static bool IsActive();								// Is Licence activated ?
	virtual Product *InstanceActivate(char *fullname, char *email, char *serial);		// Activate Licence
	virtual bool InstanceIsActive();					// Is Licence activated ?. Same, but called from instance instead of class
	virtual void AddDemoViews(char *msg);
	virtual void SetEnableDemo(bool state);
	void EnterProcessingCriticalSection();
	void LeaveProcessingCriticalSection();
	static void ClassEnterProcessingCriticalSection(Module *tthis);
	static void ClassLeaveProcessingCriticalSection(Module *tthis);
	virtual const char *GetInfoURL();
	virtual void PolyphonyChanged(int voices);
	virtual void SetForceMono(PatchPoint *pp, bool set_is_mono);
	virtual Module *GetVoiceModule(int voice);
	virtual void ConstructionComplete(int voices);
	virtual void DestructionStarted();
	virtual void Destructor();
	PatchPoint *AddPatchPoint(float x, float y, int pptype, CBitmap **ppbit, int bitm_index, CControlListener *listener);
	PatchPoint *AddMIDIPatchPoint(float x, float y, int pptype, CBitmap *bitmap, CControlListener *listener);
	ModuleKnob *AddModuleKnob(float x, float y, CBitmap *bitmap, int num_images, bool is_stepping, CControlListener *listener, long tag=NEW_TAG);
	CVerticalSwitch *AddVerticalSwitch(float x, float y, CBitmap *bitmap, int num_images, CControlListener *listener);
	CMovieBitmap *AddMovieBitmap(float x, float y, CBitmap *bitmap, int num_images, CControlListener *listener, bool centre=true);
	CKickButton *AddKickButton(float x, float y, CBitmap *bitmap, int num_images, CControlListener *listener);
	COnOffButton *AddOnOffButton(float x, float y, CBitmap *bitmap, int num_images, CControlListener *listener, long style, long tag=NEW_TAG);
	CSpecialDigit *AddSpecialDigit(float x, float y, CBitmap *bitmap, int num_images, int inumbers, long tag, CControlListener *listener);
	CSpecialDigitEx *AddSpecialDigitEx(float x, float y, CBitmap *bitmap, int num_images, int inumbers, long tag, CControlListener *listener, CBitmap* blank);
	ModuleKnobEx *AddModuleKnobEx(float x, float y, CBitmap *bitmap, int num_images, bool is_stepping, CControlListener *listener, CControl *vattached1, int vtag1, long tag=NEW_TAG);
	CTextLabel *AddTextLabel(float x, float y, CBitmap *bitmap, long style, float width, float height);
	CTextLabelEx *AddTextLabelEx(float x, float y, CBitmap *bitmap, long style, float width, float height);
	CHorizontalSlider *AddHorizontalSlider(float x, float y, float width, float height, CBitmap *bitmap, CBitmap *background, CControlListener *listener);

	static const char *GetProductName();
	//virtual void DeleteModule();
	//virtual void forget();
	//static Module *Constructor(CFrame *pParent, CControlListener *listener, const SynthComm *psynth_comm, const int vvoice);				// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
	//void SetSynthComm(const SynthComm *psynth_comm);			// Used for Dll modules

	CLASS_METHODS(Module, CViewContainer);

	//// Now, Setters and Getters. I'm not a fan of these things, but it's better for future ABI compatibility.
	//// "virtual" because it's the only way for Dll modules when solorack calls these functions.
	virtual float GetSampleRate();
	virtual float GetHalfSampleRate();
	virtual float Get60SampleRate();
	virtual float GetDAWSampleRate();
	virtual int GetDAWBlockSize();

	virtual int GetNBControls();
	virtual void SetNBControls(int n);
	virtual int GetNBPatchPoints();
	virtual void SetNBPatchPoints(int n);
	virtual int GetNBCables();
	virtual void SetNBCables(int n);
	virtual int GetBandLimit();

	virtual SoloRack *GetSynth();
	virtual void SetSynth(SoloRack *p);
	virtual const SynthComm GetSynthComm();
	virtual void SetSynthComm(const SynthComm *p);
	virtual SynEditor *GetSynEditor();
	virtual void SetSynEditor(SynEditor *p);
	
	virtual bool GetInMove();
	virtual void SetInMove(bool b);
	virtual int GetIndex();
	virtual void SetIndex(int i);

	virtual int GetProcIndex();
	virtual void SetProcIndex(int i);
	virtual int GetEvIndex();
	virtual void SetEvIndex(int i);
	virtual int GetSbIndex();
	virtual void SetSbIndex(int i);
	virtual int GetMouseObIndex();
	virtual void SetMouseObIndex(int i);
	virtual float GetClipLevel();
	virtual void SetClipLevel(float v);
	virtual int GetVoice();
	virtual void SetVoice(int vvoice);

	static CCoord SkinScale(CCoord v);
	static float FSkinScale(float v);
	static CRect SkinScale(CRect r);
	static CCoord SkinUnScale(CCoord v);
	static float GetClipLevelPreset(int v);

	static char *skindir, *defskindir, *dlldatadir, *dllskindir, *datadir, *plugindir;
	static float uiscale;
	static long hp, vp, vp_5, vp_3;
	
protected:
	float sample_rate;									// Solorack internal sampling rate, that accounts for "over sampling".
														// So sample_rate = DAW_sample_rate * oversampling factor
	float hsample_rate;									// half sample_rate
	float sample_rate60;								// 60*sample_rate. i.e sample rate per minute
	float DAW_sample_rate;								// DAW (real) sampling rate.
	int DAW_block_size;
	//static int overs,sovers;							// overs: Oversampling factor 2X, 4X, etc. sovers: overs without zero stuffing
	#ifdef MODULE_OVERSAMPLE
	OversampleSet ovr;									// Per modules Oversampling factor. Implementation not complete. I decided it's not worth the effort.
	#endif
	bool allow_oversample, always_on, is_mono, enable_is_mono_menu, enable_allow_oversample;
	//static char *name;								// Module title name. Redeclare this in your derived modules. Has to be "static private" because each module class has a different name.
	//static int name_len;				// Length of the name string excluding terminating NULL.
	int nbcontrols;						// Number of controls
	int nb_pp;							// Number of path points. Auto calculated after creation. Used when saving presets.
	int nb_cables;						// Number of cables currently connected to the module
	int nb_force_mono;					// Number of patch points set to force_mono=true
	int bandlimit;						// Bandlimiting setting.

	static CBitmap **mcbits;			// Array of common bitmaps that can be used by all modules 
	enum ModuleCBits
	{
		scrbit = 0,						// Bitmap(s) of screws		
		MIDIppbit,						// Bitmap of MIDI patch point

		// Shreds N Charades
		knob_big_white,
		//knob_medium_green,
		//knob_medium_red,
		knob_medium_white,
		knob_medium_green,
		knob_medium_red,
		knob_small_white,
		knob_small_green,
		knob_small_red,
		knob_small_black,
		knob_tiny_black,

		led_big_green,
		led_big_red,
		led_big_yellow,
		led_big_gyr,

		led_small_green,
		led_small_red,
		led_small_yellow,
		led_small_gyr,

		black_butbit_tiny,
		black_butbit_tiny_up,
		black_butbit_tiny_down,
		button_led,
		button_big,

		fader_back,
		fader_on,
		fader_off,

		black_switch,

		kModuleCBitsCount
	};
	static CBitmap **ppbit;				// Bitmap(s) of patch points
	static CColor digit_color;
	SoloRack *psynth;
	SynEditor *peditor;
	SynthComm synth_comm;				// Used with Dll Modules. Contains all callback function pointers to comunicate with SoloRack.
	float clip_level;					// Clipping level in voltage. // Added after SDK v0.1 Alpha
	int voice;							// Used for polyphony. To indicate which voice this module is working for. if voice>0, it's invisible from the screen.
	//COptionMenu *modtype_menu;		// Module type menu
	//int menu_index;						
	

private:
	bool in_move;						//** try static
	int index;							// index in the mods[] array
	int procindex;						// index in the procmods[] array
	int evindex;						// index in the evmods[] array
	int sbindex;						// index in the sbmods[] array
	int mouseobindex;					// index in the mouseobmods[] array
	int orphindex;						// Index in the orphanmods[] array
	CTextLabel *demolabel;
	char *infourl;						// This should have been static. But I'm sick of having to deal with it.
};



#endif
