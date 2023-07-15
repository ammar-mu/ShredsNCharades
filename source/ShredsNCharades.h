#ifndef __ShredsNCharades__
#define __ShredsNCharades__

// For VCVR
#define	M_PI		3.14159265358979323846
#define __attribute__(x)
#define TEST
#undef min(a,b)
#undef max(a,b)

// For Clouds
#include "dsp/common.hpp"
#include "dsp/resampler.hpp"
#include "dsp/digital.hpp"
#include "clouds/dsp/granular_processor.h"

// For Rings
#include "rings/dsp/part.h"
#include "rings/dsp/strummer.h"
#include "rings/dsp/string_synth_part.h"

// For Braids
#include "braids/macro_oscillator.h"
#include "braids/vco_jitter_source.h"
#include "braids/signature_waveshaper.h"

// For Tides
#include "stmlib/dsp/hysteresis_quantizer.h"
#include "stmlib/dsp/units.h"
#include "tides2/poly_slope_generator.h"
#include "tides2/ramp_extractor.h"
#include "tides2/io_buffer.h"

// For Warps
#include "warps/dsp/modulator.h"

// For Stages
#include "stages/segment_generator.h"
#include "stages/oscillator.h"
//#include "engine/Param.hpp"
#include "context.hpp"
#include <vector>
//#include "avrlib/avrlib.h"

using namespace rack;
#define __PI 3.14159265358979
#define __TWO_PI (2.0*__PI)
#define mmax(a,b)	(((a) > (b)) ? (a) : (b))
#define mmin(a,b)	(((a) < (b)) ? (a) : (b))

#include "Modules.h"

//---------------------------------------------
// Clouds
#define CLOUDS_LEDS_WAIT		1.0
class Clouds : public Module
{
public:

	Clouds(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	~Clouds();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Clouds* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
	static const char* GetProductName();
	const char* GetName();
	const int GetNameLen();
	const char* GetVendorName();
	const int GetVendorNameLen();
	inline void ProcessSample();
	int GetVersion() { return 1000; }														// Is Licence activated ?
	Product* InstanceActivate(char* fullname, char* email, char* serial);		// Virtual Version
	bool InstanceIsActive();													// Virtual Version
	void ValueChanged(CControl* pControl);
	void OnReset();
	void SetSampleRate(float sr);
	void LoadPreset(void* pdata, int size, int version);
	void SavePreset(void* pdata, int size);
	int GetPresetSize();
	void UpdateBlendKnob();
	void UpdateProcessor();
	const char* GetInfoURL();


	static Product* pproduct;
	static char* name;					// Module title name that will be displayed in the menu
	static int name_len;				// Length of the name string excluding terminating NULL.
	static char* vendorname;			// Module vendor name
	static int vendorname_len;			// Length of the vendor name string excluding terminating NULL.

protected:
	static CBitmap* panel;

private:
	// Inputs
	//float in1, in2, in3, in4;

	// Input connections (cables)
	//float *pin1,*pin2,*pin3,*pin4;

	// Outputs
	//float out;

	// Knobs
	ModuleKnob* kposition, * ksize, * kpitch, * kingain, * kdensity, * ktexture;
	ModuleKnob* kblend, * kspread, * kfeedback, * kreverb;

	// Knobs. If you want to use a change pool, you'll have to define them as ModuleKnobEx

	// Buttons
	COnOffButton* bfreez;
	CKickButton* bmode, * bquality, * balt;

	// LEDs
	CMovieBitmap* lmix, * lpan, * lfb, * lreverb;

	// Patch points
	PatchPoint* ppfreeze, * pptrig, * ppposition, * ppsize, * pppitch, * ppblend, * ppinl, * ppinr, * ppdensity, * pptexture;
	PatchPoint* ppoutl, * ppoutr;

	// Switches
	//CVerticalSwitch *sw1;

	// Screws
	CMovieBitmap* screw1, * screw2, * screw3, * screw4;

	// knob change pool
	//ModuleKnobExPool<Clouds_NUM_KNOBS> chpool;

	// Other
	bool invert;

	// VCVR Port
	dsp::SampleRateConverter<2> inputSrc;
	dsp::SampleRateConverter<2> outputSrc;
	// 780 buffer size supports arround 16x oversampling when sampling rate is 48000
	dsp::DoubleRingBuffer<dsp::Frame<2>, 780> inputBuffer;
	dsp::DoubleRingBuffer<dsp::Frame<2>, 780> outputBuffer;

	uint8_t* block_mem;
	uint8_t* block_ccm;
	clouds::GranularProcessor* processor;

	bool triggered = false;
	bool freeze = false;
	int blendMode = 0, leds_wait = 0;

	clouds::PlaybackMode playback;
	int quality = 0;
	float last_vu;

};



//---------------------------------------------
// Rings
#define Rings_LEDS_WAIT		1.0
class Rings : public Module
{
public:

	Rings(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Rings* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
	static const char* GetProductName();
	const char* GetName();
	const int GetNameLen();
	const char* GetVendorName();
	const int GetVendorNameLen();
	inline void ProcessSample();
	int GetVersion() { return 1000; }														// Is Licence activated ?
	Product* InstanceActivate(char* fullname, char* email, char* serial);		// Virtual Version
	bool InstanceIsActive();													// Virtual Version
	void ValueChanged(CControl* pControl);
	void SetSampleRate(float sr);
	void LoadPreset(void* pdata, int size, int version);
	void SavePreset(void* pdata, int size);
	int GetPresetSize();
	void UpdateResonatorModel();
	bool GetIsMonoDefault() { return false; }
	const char* GetInfoURL();


	static Product* pproduct;
	static char* name;					// Module title name that will be displayed in the menu
	static int name_len;				// Length of the name string excluding terminating NULL.
	static char* vendorname;			// Module vendor name
	static int vendorname_len;			// Length of the vendor name string excluding terminating NULL.

protected:
	static CBitmap* panel;

private:

	// Knobs
	ModuleKnob* kfreq, * kstruct, * kbright, * kdamping, * kposition;
	ModuleKnob* kbright_cv, * kfreq_cv, * kdamping_cv, * kstruct_cv, * kposition_cv;

	// Buttons
	COnOffButton* beaster, * blow_cpu;
	CKickButton* bpoly, * bresonator;

	// LEDs
	CMovieBitmap* lpoly, * lresonator, * llow_cpu;

	// Patch points
	PatchPoint* ppbright, * ppfreq, * ppdamping, * ppstruct, * ppposition, * ppstrum, * pppitch, * ppin;
	PatchPoint* ppodd, * ppeven;

	// Switches
	//CVerticalSwitch *sw1;

	// Screws
	CMovieBitmap* screw1, * screw2, * screw3, * screw4;

	// knob change pool
	//ModuleKnobExPool<Rings_NUM_KNOBS> chpool;

	// VCVR Port
	dsp::SampleRateConverter<1> inputSrc;
	dsp::SampleRateConverter<2> outputSrc;
	// 780 buffer size supports arround 16x oversampling when sampling rate is 48000
	dsp::DoubleRingBuffer<dsp::Frame<1>, 780> inputBuffer;
	dsp::DoubleRingBuffer<dsp::Frame<2>, 780> outputBuffer;

	uint16_t reverb_buffer[32768] = {};
	rings::Part part;
	rings::StringSynthPart string_synth;
	rings::Strummer strummer;
	bool lastStrum = false;

	//dsp::SchmittTrigger polyphonyTrigger;
	//dsp::SchmittTrigger modelTrigger;
	int polyphonyMode = 0, low_cpu = 0;
	rings::ResonatorModel resonatorModel = rings::RESONATOR_MODEL_MODAL;
	bool easterEgg = false;
	float sr_notefix = 0;
	float pin[24] = {};
};




//---------------------------------------------
// Braids

#undef BRAIDS_SYNC

class Braids : public Module
{
public:

	Braids(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	//~Braids();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Braids* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
	static const char* GetProductName();
	const char* GetName();
	const int GetNameLen();
	const char* GetVendorName();
	const int GetVendorNameLen();
	inline void ProcessSample();
	int GetVersion() { return 1000; }														// Is Licence activated ?
	Product* InstanceActivate(char* fullname, char* email, char* serial);		// Virtual Version
	bool InstanceIsActive();													// Virtual Version
	void ValueChanged(CControl* pControl);
	void SetSampleRate(float sr);
	bool GetIsMonoDefault() { return false; }
	//void LoadPreset(void* pdata, int size, int version);
	//void SavePreset(void* pdata, int size);
	//int GetPresetSize();
	const char* GetInfoURL();


	static Product* pproduct;
	static char* name;					// Module title name that will be displayed in the menu
	static int name_len;				// Length of the name string excluding terminating NULL.
	static char* vendorname;			// Module vendor name
	static int vendorname_len;			// Length of the vendor name string excluding terminating NULL.

protected:
	static CBitmap* panel;

private:

	struct ShapeInfo
	{
		char* code;
		char* label;
	};

	static const ShapeInfo shape_infos[];

	// Knobs
	ModuleKnob* kshape, * kfine, * kcoarse, * kfm, * ktimbre;
	ModuleKnob* kmodulation, * kcolor;

	// Buttons
	COnOffButton* bshape_mod, * blow_cpu;
	//CKickButton* bpoly, * bresonator;

	// LEDs
	CMovieBitmap* lshape_mod, * llow_cpu;

	// Patch points
	PatchPoint* pptrig, * pppitch, * ppfm, * pptimbre, * ppcolor, * ppout;

	CTextLabel* shape_text;

	// Switches
	//CVerticalSwitch *sw1;

	// Screws
	CMovieBitmap* screw1, * screw2, * screw3, * screw4;

	// knob change pool
	//ModuleKnobExPool<Braids_NUM_KNOBS> chpool;

	// VCVR Port
	braids::MacroOscillator osc;
	braids::SettingsData settings;
	braids::VcoJitterSource jitter_source;
	braids::SignatureWaveshaper ws;

	dsp::SampleRateConverter<1> src;
	dsp::DoubleRingBuffer<dsp::Frame<1>, 780> outputBuffer;
#ifdef BRAIDS_SYNC
	// 780 buffer size supports arround 16x oversampling when sampling rate is 48000
	dsp::DoubleRingBuffer<dsp::Frame<1>, 780> sync_inbuffer;
	dsp::SampleRateConverter<1> sync_src;
#endif
	uint8_t syncint[24] = {};
	bool lastTrig = false;
	bool lowCpu = false;
	int shape = braids::MACRO_OSC_SHAPE_CSAW, synci = 0;
	float lowcpu_pitch_fix;

};



//---------------------------------------------
// Tides

class Tides : public Module
{
public:

	Tides(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	//~Tides();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Tides* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
	static const char* GetProductName();
	const char* GetName();
	const int GetNameLen();
	const char* GetVendorName();
	const int GetVendorNameLen();
	inline void ProcessSample();
	int GetVersion() { return 1000; }														// Is Licence activated ?
	Product* InstanceActivate(char* fullname, char* email, char* serial);		// Virtual Version
	bool InstanceIsActive();													// Virtual Version
	void ValueChanged(CControl* pControl);
	void SetSampleRate(float sr);
	bool GetIsMonoDefault() { return false; }
	void LoadPreset(void* pdata, int size, int version);
	void SavePreset(void* pdata, int size);
	int GetPresetSize();
	void onReset();
	const char* GetInfoURL();
	void UpdateRangeMode();
	void UpdateOutPutMode();
	void UpdateRampMode();


	static Product* pproduct;
	static char* name;					// Module title name that will be displayed in the menu
	static int name_len;				// Length of the name string excluding terminating NULL.
	static char* vendorname;			// Module vendor name
	static int vendorname_len;			// Length of the vendor name string excluding terminating NULL.

protected:
	static CBitmap* panel;

private:

	// Knobs
	ModuleKnob* kfreq, * kshape, * kslope, * ksmooth, * kshift;
	ModuleKnob* kslopecv, * kfreqcv, * ksmoothcv, * kshapecv, * kshiftcv;

	// Buttons
	//COnOffButton* brange, * bmode;
	CKickButton* bfrange, * bmode, * bramp;

	// LEDs
	CMovieBitmap* lfrange, * lmode, * lramp, * lout1, * lout2, * lout3, * lout4;

	// Patch points
	PatchPoint* ppslope, * ppfreq, * ppvoct, * ppsmooth, * ppshape, * ppshift, * pptrig, * ppclock;
	PatchPoint* ppout1, * ppout2, * ppout3, * ppout4;

	// Switches
	//CVerticalSwitch *sw1;

	// Screws
	CMovieBitmap* screw1, * screw2, * screw3, * screw4;

	// VCVR Port
	static const float kRootScaled[3];
	//static const tides2::Ratio kRatios[20];

	tides2::PolySlopeGenerator poly_slope_generator;
	tides2::RampExtractor ramp_extractor;
	stmlib::HysteresisQuantizer ratio_index_quantizer;

	// State
	int range;
	tides2::Range range_mode;
	tides2::OutputMode output_mode;
	tides2::RampMode ramp_mode;
	//dsp::BooleanTrigger rangeTrigger;
	//dsp::BooleanTrigger modeTrigger;
	//dsp::BooleanTrigger rampTrigger;

	// Buffers
	tides2::PolySlopeGenerator::OutputSample out[tides2::kBlockSize] = {};
	stmlib::GateFlags trig_flags[tides2::kBlockSize] = {};
	stmlib::GateFlags clock_flags[tides2::kBlockSize] = {};
	stmlib::GateFlags previous_trig_flag = stmlib::GATE_FLAG_LOW;
	stmlib::GateFlags previous_clock_flag = stmlib::GATE_FLAG_LOW;

	bool must_reset_ramp_extractor = true;
	//tides2::OutputMode previous_output_mode = tides2::OUTPUT_MODE_GATES;
	uint8_t frame = 0;
};



//---------------------------------------------
// Branches

class Branches : public Module
{
public:

	Branches(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	//~Branches();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Branches* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
	static const char* GetProductName();
	const char* GetName();
	const int GetNameLen();
	const char* GetVendorName();
	const int GetVendorNameLen();
	inline void ProcessSample();
	int GetVersion() { return 1001; }														// Is Licence activated ?
	Product* InstanceActivate(char* fullname, char* email, char* serial);		// Virtual Version
	bool InstanceIsActive();													// Virtual Version
	void ValueChanged(CControl* pControl);
	//void SetSampleRate(float sr);
	//bool GetIsMonoDefault() { return false; }
	void LoadPreset(void* pdata, int size, int version);
	//void SavePreset(void* pdata, int size);
	//int GetPresetSize();
	void onReset();
	const char* GetInfoURL();
	void UpdateOutputs(bool gate, int i);

	static Product* pproduct;
	static char* name;					// Module title name that will be displayed in the menu
	static int name_len;				// Length of the name string excluding terminating NULL.
	static char* vendorname;			// Module vendor name
	static int vendorname_len;			// Length of the vendor name string excluding terminating NULL.

protected:
	static CBitmap* panel;

private:

	// Knobs
	ModuleKnob* kthresh[2];

	// Buttons
	COnOffButton* bmode[2];
	//CKickButton* bfrange, * bmode, * bramp;

	// LEDs
	CMovieBitmap* lout[2];

	// Patch points
	PatchPoint* ppin[2], * pp[2], * ppouta[2], * ppoutb[2];

	// Switches
	//CVerticalSwitch *sw1;

	// Screws
	CMovieBitmap* screw1, * screw2;

	// VCVR Port
	bool outcomes[2] = {};
	bool modes[2] = {};
	float last_gate[2] = {};
};



//---------------------------------------------
// Warps
class Warps : public Module
{
public:

	Warps(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	//~Warps();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Warps* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
	static const char* GetProductName();
	const char* GetName();
	const int GetNameLen();
	const char* GetVendorName();
	const int GetVendorNameLen();
	inline void ProcessSample();
	int GetVersion() { return 1000; }														// Is Licence activated ?
	Product* InstanceActivate(char* fullname, char* email, char* serial);		// Virtual Version
	bool InstanceIsActive();													// Virtual Version
	void ValueChanged(CControl* pControl);
	void SetSampleRate(float sr);
	//bool GetIsMonoDefault() { return false; }
	void LoadPreset(void* pdata, int size, int version);
	void SavePreset(void* pdata, int size);
	int GetPresetSize();
	const char* GetInfoURL();
	void Warps::drawRect(CDrawContext* pContext, const CRect& updateRect);

	static Product* pproduct;
	static char* name;					// Module title name that will be displayed in the menu
	static int name_len;				// Length of the name string excluding terminating NULL.
	static char* vendorname;			// Module vendor name
	static int vendorname_len;			// Length of the vendor name string excluding terminating NULL.

protected:
	static CBitmap* panel;

private:

	// Knobs
	ModuleKnob* kalgo, * ktimbre, * klevel1, * klevel2;

	// Buttons
	COnOffButton* beaster;
	CKickButton* bstate;

	// LEDs
	CMovieBitmap* lstate, * leaster;

	// Patch points
	PatchPoint* ppalgo, * pptimbre, * ppcar, * ppmod, * pplevel1, * pplevel2, * ppmodout, * ppaux;

	// Switches
	//CVerticalSwitch *sw1;

	// Screws
	CMovieBitmap* screw1, * screw2;

	float noteinc;

	// VCVR Port
	int frame = 0;
	warps::Modulator modulator;
	warps::ShortFrame inputFrames[60] = {};
	warps::ShortFrame outputFrames[60] = {};
	//dsp::SchmittTrigger stateTrigger;
};



//---------------------------------------------
// Stages

//struct Param 
//{	/** Unstable API. Use setValue() and getValue() instead. */
//	float value = 0.f;
//
//	float getValue() {
//		return value;
//	}
//
//	void setValue(float value) {
//		this->value = value;
//	}
//};

// Must match io_buffer.h
static const int NUM_CHANNELS = 6;
static const int BLOCK_SIZE = 8;
static const int GATE_INPUTS = NUM_CHANNELS;

struct LongPressButton
{
	float sample_rate;

	enum Events {
		NO_PRESS,
		SHORT_PRESS,
		LONG_PRESS
	};

	float pressedTime = 0.f;
	dsp::BooleanTrigger trigger;

	Events step(CControl* param)
	{
		Events result = NO_PRESS;

		bool pressed = param->value > 0.f;
		if (pressed && pressedTime >= 0.f) {
			pressedTime += 1.f / sample_rate;
			if (pressedTime >= 1.f) {
				pressedTime = -1.f;
				result = LONG_PRESS;
			}
		}

		// Check if released
		if (trigger.process(!pressed)) {
			if (pressedTime >= 0.f) {
				result = SHORT_PRESS;
			}
			pressedTime = 0.f;
		}

		return result;
	}
};


struct GroupInfo {
	int first_segment = 0;
	int segment_count = 0;
	int active_segment = -1;
	bool gated = false;
};

struct GroupBuilder {

	GroupInfo groups[NUM_CHANNELS];
	int groupCount = 0;

	bool buildGroups(std::vector<PatchPoint*>* gateInputs, size_t first, size_t count)
	{
		bool any_gates = false;

		GroupInfo nextGroups[NUM_CHANNELS];

		int currentGroup = 0;
		for (int i = 0; i < NUM_CHANNELS; i++)
		{
			bool gated = (*gateInputs)[first + i]->num_cables > 0;

			if (!any_gates) {
				if (!gated) {
					// No gates at all yet, segments are all single segment groups
					nextGroups[currentGroup].first_segment = i;
					nextGroups[currentGroup].segment_count = 1;
					nextGroups[currentGroup].active_segment = -1;
					nextGroups[currentGroup].gated = false;
					currentGroup++;
				}
				else {
					// first gate, current group is start of a segment group
					any_gates = true;
					nextGroups[currentGroup].first_segment = i;
					nextGroups[currentGroup].segment_count = 1;
					nextGroups[currentGroup].active_segment = -1;
					nextGroups[currentGroup].gated = true;
					currentGroup++;
				}
			}
			else {
				if (!gated) {
					// We've had a gate, this ungated segment is part of the previous group
					nextGroups[currentGroup - 1].segment_count++;
				}
				else {
					// This gated input indicates the start of the next group
					nextGroups[currentGroup].first_segment = i;
					nextGroups[currentGroup].segment_count = 1;
					nextGroups[currentGroup].active_segment = -1;
					nextGroups[currentGroup].gated = true;
					currentGroup++;
				}
			}
		}

		bool changed = false;

		if (currentGroup != groupCount) {
			changed = true;
			groupCount = currentGroup;
		}

		for (int i = 0; i < groupCount; i++) {
			if (nextGroups[i].segment_count != groups[i].segment_count ||
				nextGroups[i].gated != groups[i].gated ||
				nextGroups[i].first_segment != groups[i].first_segment) {
				changed = true;
			}

			groups[i].first_segment = nextGroups[i].first_segment;
			groups[i].segment_count = nextGroups[i].segment_count;
			groups[i].active_segment = nextGroups[i].active_segment;
			groups[i].gated = nextGroups[i].gated;
		}

		return changed;
	}
};

class Stages : public Module
{
public:

	Stages(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	//~Stages();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Stages* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
	static const char* GetProductName();
	const char* GetName();
	const int GetNameLen();
	const char* GetVendorName();
	const int GetVendorNameLen();
	inline void ProcessSample();
	int GetVersion() { return 1000; }														// Is Licence activated ?
	Product* InstanceActivate(char* fullname, char* email, char* serial);		// Virtual Version
	bool InstanceIsActive();													// Virtual Version
	void ValueChanged(CControl* pControl);
	void SetSampleRate(float sr);
	//bool GetIsMonoDefault() { return false; }
	void LoadPreset(void* pdata, int size, int version);
	void SavePreset(void* pdata, int size);
	int GetPresetSize();
	const char* GetInfoURL();
	void onReset();
	void stepBlock();
	void toggleMode(int i);
	void toggleLoop(int segment);
	void Stages::drawRect(CDrawContext* pContext, const CRect& updateRect);
	//CMouseEventResult onMouseUp(CPoint& where, const long& buttons);
	void UpdateButton(int i);
	void UpdateMode(int i);
	void UpdateLoop(int segment);
	void CableConnected(PatchPoint* pp);
	void CableDisconnected(PatchPoint* pp);
	void ResetLoopLeds(int segment);
	bool GetAlwaysONDefault() { return true; }

	static Product* pproduct;
	static char* name;					// Module title name that will be displayed in the menu
	static int name_len;				// Length of the name string excluding terminating NULL.
	static char* vendorname;			// Module vendor name
	static int vendorname_len;			// Length of the vendor name string excluding terminating NULL.

protected:
	static CBitmap* panel;

private:

	// Knobs
	ModuleKnob* kshape[NUM_CHANNELS], * klevel[NUM_CHANNELS];

	// Buttons
	CKickButtonEx* btype[NUM_CHANNELS];
	//COnOffButton* btype[NUM_CHANNELS];

	// LEDs
	CMovieBitmap* ltypeg[NUM_CHANNELS], * ltypey[NUM_CHANNELS], * ltyper[NUM_CHANNELS], * lenv[NUM_CHANNELS];

	// Faders
	CMovieBitmap* faderback[NUM_CHANNELS];

	// Patch points
	PatchPoint* pplevel[NUM_CHANNELS], * ppgate[NUM_CHANNELS], * ppout[NUM_CHANNELS];

	// Switches
	//CVerticalSwitch *sw1;

	// Screws
	CMovieBitmap* screw1, * screw2, * screw3, * screw4;

	// VCVR Port
	stages::segment::Configuration configurations[NUM_CHANNELS];
	bool configuration_changed[NUM_CHANNELS];
	stages::SegmentGenerator segment_generator[NUM_CHANNELS];
	float lightOscillatorPhase;

	// Buttons
	LongPressButton typeButtons[NUM_CHANNELS];

	// Buffers
	float envelopeBuffer[NUM_CHANNELS][BLOCK_SIZE] = {};
	stmlib::GateFlags last_gate_flags[NUM_CHANNELS] = {};
	stmlib::GateFlags gate_flags[NUM_CHANNELS][BLOCK_SIZE] = {};
	int blockIndex = 0;
	GroupBuilder groupBuilder;
	bool groups_changed;

	std::vector<PatchPoint*> all_inputs;
	float pressed_time;
	float sample_time, phase_add;
};

#endif
