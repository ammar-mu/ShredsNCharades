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

// For Elements
#include "elements/dsp/part.h"

// For Marbles
#include "marbles/random/random_generator.h"
#include "marbles/random/random_stream.h"
#include "marbles/random/t_generator.h"
#include "marbles/random/x_y_generator.h"
#include "marbles/note_filter.h"

// For Frames
#include "frames/keyframer.h"
#include "frames/poly_lfo.h"

// For Streams
#include "Streams/streams.hpp"

// For Plaits
#include "plaits/dsp/voice.h"

// For Shelves
#include "Shelves/shelves.hpp"

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
	dsp::DynDoubleRingBuffer<dsp::Frame<2>> *inputBuffer;
	dsp::DynDoubleRingBuffer<dsp::Frame<2>> *outputBuffer;

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
	~Rings();

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
	dsp::DynDoubleRingBuffer<dsp::Frame<1>> *inputBuffer;
	dsp::DynDoubleRingBuffer<dsp::Frame<2>> *outputBuffer;

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

	~Braids();

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
	dsp::DynDoubleRingBuffer<dsp::Frame<1>> *outputBuffer;
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


//---------------------------------------------
// Elements
class Elements : public Module
{
public:

	Elements(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	~Elements();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Elements* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
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
	//void UpdateProcessor();
	const char* GetInfoURL();
	void SetModel(int model);
	bool GetIsMonoDefault() { return false; }


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
	ModuleKnob* kcontour, * kbow, * kblow, * kstrike, * kcoarse, * kfine;
	ModuleKnob* kfm, * kflow, * kmallet, * kgeom, *kbright, *kbow_timbre, *kblow_timbre, *kstrike_timbre;
	ModuleKnob* kdamp, * kpos, * kspace;

	ModuleKnob *kflowcv, * kmalletcv, * kgeomcv, * kbrightcv, * kbow_timbrecv, * kblow_timbrecv, * kstrike_timbrecv;
	ModuleKnob *kdampcv, * kposcv, * kspacecv;

	// Knobs. If you want to use a change pool, you'll have to define them as ModuleKnobEx

	// Buttons
	//COnOffButton * blow_cpu;
	CKickButton* bplay, *bmode;

	// LEDs
	CMovieBitmap* lexciter, * lresonator, *lplay, *lmode; // , * llow_cpu;;

	// Patch points
	PatchPoint* pppitch, * ppfm, * ppgate, * ppstrength, * ppextblow, * ppextstrike, * ppoutl, * ppoutr;
	PatchPoint* ppflowcv, * ppmalletcv, * ppgeomcv, * ppbrightcv, * ppbow_timbrecv, * ppblow_timbrecv, * ppstrike_timbrecv;
	PatchPoint* ppdampcv, * ppposcv, * ppspacecv;

	// Switches
	//CVerticalSwitch *sw1;

	// Screws
	CMovieBitmap* screw1, * screw2, * screw3, * screw4;

	// knob change pool
	//ModuleKnobExPool<Elements_NUM_KNOBS> chpool;

	// VCVR Port
	dsp::SampleRateConverter<2> inputSrc;
	dsp::SampleRateConverter<2> outputSrc;
	// 780 buffer size supports arround 16x oversampling when sampling rate is 48000
	dsp::DynDoubleRingBuffer<dsp::Frame<2>> *inputBuffer;
	dsp::DynDoubleRingBuffer<dsp::Frame<2>> *outputBuffer;
	dsp::Frame<2> inputFrame = {};
	dsp::Frame<2> outputFrame = {};

	uint16_t reverb_buffer[32768] = {};
	elements::Part* epart;
	int mmode;
	//bool low_cpu;
	//float sr_notefix=0.f;

};



//---------------------------------------------
// Marbles

static const marbles::Scale preset_scales[6] = {
	// C major
	{
		1.0f,
		12,
		{
			{ 0.0000f, 255 },  // C
			{ 0.0833f, 16 },   // C#
			{ 0.1667f, 96 },   // D
			{ 0.2500f, 24 },   // D#
			{ 0.3333f, 128 },  // E
			{ 0.4167f, 64 },   // F
			{ 0.5000f, 8 },    // F#
			{ 0.5833f, 192 },  // G
			{ 0.6667f, 16 },   // G#
			{ 0.7500f, 96 },   // A
			{ 0.8333f, 24 },   // A#
			{ 0.9167f, 128 },  // B
		}
	},

	// C minor
	{
		1.0f,
		12,
		{
			{ 0.0000f, 255 },  // C
			{ 0.0833f, 16 },   // C#
			{ 0.1667f, 96 },   // D
			{ 0.2500f, 128 },  // Eb
			{ 0.3333f, 8 },    // E
			{ 0.4167f, 64 },   // F
			{ 0.5000f, 4 },    // F#
			{ 0.5833f, 192 },  // G
			{ 0.6667f, 16 },   // G#
			{ 0.7500f, 96 },   // A
			{ 0.8333f, 128 },  // Bb
			{ 0.9167f, 16 },   // B
		}
	},

	// Pentatonic
	{
		1.0f,
		12,
		{
			{ 0.0000f, 255 },  // C
			{ 0.0833f, 4 },    // C#
			{ 0.1667f, 96 },   // D
			{ 0.2500f, 4 },    // Eb
			{ 0.3333f, 4 },    // E
			{ 0.4167f, 140 },  // F
			{ 0.5000f, 4 },    // F#
			{ 0.5833f, 192 },  // G
			{ 0.6667f, 4 },    // G#
			{ 0.7500f, 96 },   // A
			{ 0.8333f, 4 },    // Bb
			{ 0.9167f, 4 },    // B
		}
	},

	// Pelog
	{
		1.0f,
		7,
		{
			{ 0.0000f, 255 },  // C
			{ 0.1275f, 128 },  // Db+
			{ 0.2625f, 32 },  // Eb-
			{ 0.4600f, 8 },    // F#-
			{ 0.5883f, 192 },  // G
			{ 0.7067f, 64 },  // Ab
			{ 0.8817f, 16 },    // Bb+
		}
	},

	// Raag Bhairav That
	{
		1.0f,
		12,
		{
			{ 0.0000f, 255 }, // ** Sa
			{ 0.0752f, 128 }, // ** Komal Re
			{ 0.1699f, 4 },   //    Re
			{ 0.2630f, 4 },   //    Komal Ga
			{ 0.3219f, 128 }, // ** Ga
			{ 0.4150f, 64 },  // ** Ma
			{ 0.4918f, 4 },   //    Tivre Ma
			{ 0.5850f, 192 }, // ** Pa
			{ 0.6601f, 64 },  // ** Komal Dha
			{ 0.7549f, 4 },   //    Dha
			{ 0.8479f, 4 },   //    Komal Ni
			{ 0.9069f, 64 },  // ** Ni
		}
	},

	// Raag Shri
	{
		1.0f,
		12,
		{
			{ 0.0000f, 255 }, // ** Sa
			{ 0.0752f, 4 },   //    Komal Re
			{ 0.1699f, 128 }, // ** Re
			{ 0.2630f, 64 },  // ** Komal Ga
			{ 0.3219f, 4 },   //    Ga
			{ 0.4150f, 128 }, // ** Ma
			{ 0.4918f, 4 },   //    Tivre Ma
			{ 0.5850f, 192 }, // ** Pa
			{ 0.6601f, 4 },   //    Komal Dha
			{ 0.7549f, 64 },  // ** Dha
			{ 0.8479f, 128 }, // ** Komal Ni
			{ 0.9069f, 4 },   //    Ni
		}
	},
};

class Marbles : public Module
{
public:

	Marbles(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	~Marbles();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Marbles* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
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
	//void UpdateProcessor();
	const char* GetInfoURL();
	void UpdateLEDs();
	void StepBlock();


	static Product* pproduct;
	static char* name;					// Module title name that will be displayed in the menu
	static int name_len;				// Length of the name string excluding terminating NULL.
	static char* vendorname;			// Module vendor name
	static int vendorname_len;			// Length of the vendor name string excluding terminating NULL.
	static const int BLOCK_SIZE = 5;

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
	ModuleKnob* kdeja, * ktrate, * kxspread, * klength;
	ModuleKnob* ktbias, * kxbias, * ktjitter, * kxsteps;

	// Buttons
	COnOffButton * btdeja, *bxdeja;
	CKickButton* btmode, *bxmode, *btrange, *bxrange, *bextern;

	// LEDs
	CMovieBitmap * ltmode, *lxmode, *ltrange, *lxrange, *lextern;
	CMovieBitmap* lt1, * lt2, * lt3, *ly, * lx1, * lx2, * lx3;

	// Patch points
	PatchPoint* pptbias, *ppxbias, *pptclock, *pptrate, *pptjitter, *ppdeja, *ppxsteps;
	PatchPoint*ppxspread, *ppxclock, *ppt1, *ppt2, *ppt3, *ppy, *ppx1, *ppx2, *ppx3;

	// Switches
	//CVerticalSwitch *sw1;

	// Menus
	COptionMenu* more_menu, *mtmode, *mtrange, *mxmode, *mxrange, *mscale, *mint_x_clock, *my_divider;

	// Screws
	CMovieBitmap* screw1, * screw2, * screw3, * screw4;

	// knob change pool
	//ModuleKnobExPool<Marbles_NUM_KNOBS> chpool;

	// VCVR Port
	//dsp::SampleRateConverter<2> inputSrc;
	//dsp::SampleRateConverter<2> outputSrc;
	//// 780 buffer size supports arround 16x oversampling when sampling rate is 48000
	//dsp::DoubleRingBuffer<dsp::Frame<2>, 780> inputBuffer;
	//dsp::DoubleRingBuffer<dsp::Frame<2>, 780> outputBuffer;

	marbles::RandomGenerator random_generator;
	marbles::RandomStream random_stream;
	marbles::TGenerator t_generator;
	marbles::XYGenerator xy_generator;
	marbles::NoteFilter note_filter;

	// State
	//dsp::BooleanTrigger tDejaVuTrigger;
	//dsp::BooleanTrigger xDejaVuTrigger;
	//dsp::BooleanTrigger tModeTrigger;
	//dsp::BooleanTrigger xModeTrigger;
	//dsp::BooleanTrigger tRangeTrigger;
	//dsp::BooleanTrigger xRangeTrigger;
	//dsp::BooleanTrigger externalTrigger;
	bool t_deja_vu;
	bool x_deja_vu;
	int t_mode;
	int x_mode;
	int t_range;
	int x_range;
	bool external;
	int x_scale;
	int y_divider_index;
	int x_clock_source_internal;

	// Buffers
	stmlib::GateFlags t_clocks[BLOCK_SIZE] = {};
	stmlib::GateFlags last_t_clock = 0;
	stmlib::GateFlags xy_clocks[BLOCK_SIZE] = {};
	stmlib::GateFlags last_xy_clock = 0;
	float ramp_master[BLOCK_SIZE] = {};
	float ramp_external[BLOCK_SIZE] = {};
	float ramp_slave[2][BLOCK_SIZE] = {};
	bool gates[BLOCK_SIZE * 2] = {};
	float voltages[BLOCK_SIZE * 4] = {};
	int blockIndex = 0;


};


//---------------------------------------------
// Frames

#define FRAMES_STEP_TIME	0.001

class Frames : public Module
{

	enum MoreMenuIndexs
	{	kChannel1 = 0,
		kChannel2,
		kChannel3,
		kChannel4,
		kClearKeyframes,
		kSeparator,
		kKeyframer,
		kPolyLFO,

		kMoreMenuCount
	};

	enum MoreChannelMenuIndexs
	{
		KInterpolationCurve = -1,
		kStep,
		kLinear,
		kAccelerating,
		kDecelerating,
		kDepartureArrival,
		kBouncing,
		kRSeparator,
		kResponseCurve,
		kLinearR,
		kExponentialR,

		kMoreChannelMenuCount
	};
public:

	Frames(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	~Frames();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Frames* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
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
	void CableConnected(PatchPoint* pp);
	void CableDisconnected(PatchPoint* pp);
	//void UpdateProcessor();
	const char* GetInfoURL();
	void drawRect(CDrawContext* pContext, const CRect& updateRect);
	void UpdateFrameColors(bool force_redraw);
	bool GetAlwaysONDefault() { return true; }
	bool GetIsMonoDefault() { return false; }


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
	ModuleKnob* kgain[4];
	ModuleKnob* kframe, * kmod;

	// Buttons
	COnOffButton* boffset;		// Should be a switch, but for now this will work because we don't have AddHorizontalSwitch() in SDK. Only vertical
	CKickButton* badd, * bdel;

	// LEDs
	CMovieBitmap* ladd_del;
	CMovieBitmap* lg[4];

	// Patch points
	PatchPoint* ppall, * ppin[4], * ppframe;
	PatchPoint* ppmix, * ppout[4], * ppfrstep;

	// Switches
	CVerticalSwitch *sw1;

	// Menus
	COptionMenu* more_menu, * mch[4];

	// Screws
	CMovieBitmap* screw1, * screw2, * screw3, * screw4;

	// knob change pool
	//ModuleKnobExPool<Frames_NUM_KNOBS> chpool;

	// VCVR Port
	frames::Keyframer keyframer;
	frames::PolyLfo poly_lfo;
	bool poly_lfo_mode = false, allin = false, isppin[4] = {}, isppout[4] = {};
	uint16_t lastcontrols[4] = {};
	uint16_t controls[4] = {};
	int32_t timestamp;
	int16_t nearestIndex = -1, last_position = -1;
	float frmod, frx, fry, offsetv = 0.0;
	int32_t last_timestampMod = 0;
	uint8_t frcolors[3] = {};
	int32_t last_colorsi = (255 | 255 << 8 | 255 << 16);
	int step_count, step_time;

	dsp::SchmittTrigger addTrigger;
	dsp::SchmittTrigger delTrigger;
};



//---------------------------------------------
// Streams

namespace streams {

	struct StreamsChannelMode {
		ProcessorFunction function;
		bool alternate;
		std::string label;
	};

	static constexpr int kNumChannelModes = 10;

	static const StreamsChannelMode kChannelModeTable[kNumChannelModes] = {
		{PROCESSOR_FUNCTION_ENVELOPE,          false, "Envelope"},
		{PROCESSOR_FUNCTION_VACTROL,           false, "Vactrol"},
		{PROCESSOR_FUNCTION_FOLLOWER,          false, "Follower"},
		{PROCESSOR_FUNCTION_COMPRESSOR,        false, "Compressor"},
		{PROCESSOR_FUNCTION_ENVELOPE,          true,  "AR envelope"},
		{PROCESSOR_FUNCTION_VACTROL,           true,  "Plucked vactrol"},
		{PROCESSOR_FUNCTION_FOLLOWER,          true,  "Cutoff controller"},
		{PROCESSOR_FUNCTION_COMPRESSOR,        true,  "Slow compressor"},
		{PROCESSOR_FUNCTION_FILTER_CONTROLLER, true,  "Direct VCF controller"},
		{PROCESSOR_FUNCTION_LORENZ_GENERATOR,  false, "Lorenz generator"},
	};

	struct StreamsMonitorMode {
		MonitorMode mode;
		std::string label;
	};

	static constexpr int kNumMonitorModes = 4;

	static const StreamsMonitorMode kMonitorModeTable[kNumMonitorModes] = {
		{MONITOR_MODE_EXCITE_IN, "Excite"},
		{MONITOR_MODE_VCA_CV,    "Level"},
		{MONITOR_MODE_AUDIO_IN,  "In"},
		{MONITOR_MODE_OUTPUT,    "Out"},
	};

}

class Streams : public Module
{

public:

	Streams(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	~Streams();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Streams* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
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
	void SetLinked(bool linked);
	int GetChannelMode(int channel);
	void SetChannelMode(int channel, int mode_id);
	void SetMonitorMode(int mode_id);
	void LoadPreset(void* pdata, int size, int version);
	void SavePreset(void* pdata, int size);
	int GetPresetSize();
	void CableConnected(PatchPoint* pp);
	void CableDisconnected(PatchPoint* pp);
	//void UpdateProcessor();
	const char* GetInfoURL();
	void UpdateMenus();
	CMouseEventResult onMouseDown(CPoint &where, const long& buttons);
	//void drawRect(CDrawContext* pContext, const CRect& updateRect);
	//bool GetAlwaysONDefault() { return true; }
	bool GetIsMonoDefault() { return false; }


	static Product* pproduct;
	static char* name;					// Module title name that will be displayed in the menu
	static int name_len;				// Length of the name string excluding terminating NULL.
	static char* vendorname;			// Module vendor name
	static int vendorname_len;			// Length of the vendor name string excluding terminating NULL.

protected:
	static CBitmap* panel;

private:
	// Knobs
	ModuleKnob* kshape1, *kmod1, *klevel1, *kresp1, *kshape2, *kmod2, *klevel2, *kresp2;
	ModuleKnob* kframe, * kmod;

	// Buttons
	//COnOffButton* boffset;		
	CKickButtonEx* bfunc1, * bfunc2, *bmeter;

	// LEDs
	CMovieBitmap* lch1[4], *lch2[4];

	// Patch points
	PatchPoint* ppexcite1, * ppin1, * pplevel1, * ppexcite2, * ppin2, * pplevel2;
	PatchPoint* ppout1, *ppout2;

	// Switches
	//CVerticalSwitch *sw1;

	// Menus
	COptionMenu* more_menu, *mch[2], *mmeter;

	// Screws
	CMovieBitmap* screw1, * screw2;

	// knob change pool
	//ModuleKnobExPool<Streams_NUM_KNOBS> chpool;

	// VCVR Port
	streams::StreamsEngine engine;
	//int prevNumChannels;
	//float brightnesses[NUM_LIGHTS];
	streams::StreamsEngine::Frame frame = {};
	const float led_off = 0.1;
	int mode_id[2];
};


// -------------------------------------------
class Shelves : public Module
{

public:

	Shelves(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	~Shelves();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Shelves* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
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
	//void LoadPreset(void* pdata, int size, int version);
	//void SavePreset(void* pdata, int size);
	//int GetPresetSize();
	void CableConnected(PatchPoint* pp);
	void CableDisconnected(PatchPoint* pp);
	//void UpdateProcessor();
	const char* GetInfoURL();
	//CMouseEventResult onMouseDown(CPoint &where, const long& buttons);
	//void drawRect(CDrawContext* pContext, const CRect& updateRect);
	//bool GetAlwaysONDefault() { return true; }
	bool GetIsMonoDefault() { return false; }


	static Product* pproduct;
	static char* name;					// Module title name that will be displayed in the menu
	static int name_len;				// Length of the name string excluding terminating NULL.
	static char* vendorname;			// Module vendor name
	static int vendorname_len;			// Length of the vendor name string excluding terminating NULL.

	static const float freqMin;
	static const float freqMax;
	static const float freqInit;
	static const float gainMin;
	static const float gainMax;
	static const float qMin;
	static const float qMax;
	static const float qInit;

protected:
	static CBitmap* panel;

private:
	// Knobs
	ModuleKnob* khsfreq, *kp1freq, *kp2freq, *klsfreq, *khsgain, *kp1gain, *kp2gain, *klsgain;
	ModuleKnob* kp1q, * kp2q, *kin;

	// Buttons
	//COnOffButton* boffset;		
	//CKickButtonEx* bfunc1, * bfunc2, *bmeter;

	// LEDs
	CMovieBitmap* lclip;

	// Patch points
	PatchPoint* pphsfreq, *ppp1freq, *ppp2freq, *pplsfreq, *pphsgain, *ppp1gain, *ppp2gain, *pplsgain;
	PatchPoint* ppp1q, * ppp2q, *ppfreq, *ppgain, *ppin;
	PatchPoint* ppp1hp, * ppp1bp, * ppp1lp, * ppp2hp, * ppp2bp, * ppp2lp, * ppout;

	// Switches
	//CVerticalSwitch *sw1;

	// Menus
	//COptionMenu* more_menu, *mch[2], *mmeter;

	// Screws
	CMovieBitmap* screw1, * screw2, * screw3, * screw4;

	// knob change pool
	//ModuleKnobExPool<Shelves_NUM_KNOBS> chpool;

	// VCVR Port
	shelves::ShelvesEngine engine;
	//bool preGain;
	shelves::ShelvesEngine::Frame frame = {};
};



// -------------------------------------------
#define PLAITS_BLOCKSIZE	12

class Plaits : public Module
{
	enum MoreMenuIndexs
	{	kLowCPU = 0,
		kEditLPG,
		kSeparator1,
		kSeparator1Title,
		kPairOfClassicWaveforms,
		kSeparator2 = 12,
		kSeparator2Title,
		kGranularColor,
		kAnalogHiHat = 21
	};

public:

	Plaits(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);

	~Plaits();

	static void Initialize();
	static void End();
	static const int GetType();
	static Product* Activate(char* fullname, char* email, char* serial);				// Activate Licence
	static bool IsActive();
	static Plaits* Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice = 0);										// Used to construct modules that are imported for Dll files. Each sub class should define it's own Constructor.
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
	void CableConnected(PatchPoint* pp);
	void CableDisconnected(PatchPoint* pp);
	//void UpdateProcessor();
	const char* GetInfoURL();
	void UpdateLights();
	//CMouseEventResult onMouseDown(CPoint &where, const long& buttons);
	//void drawRect(CDrawContext* pContext, const CRect& updateRect);
	//bool GetAlwaysONDefault() { return true; }
	void SetLpgMode(bool lpgMode);
	void SetEngineSampleRate();
	bool GetIsMonoDefault() { return false; }


	static Product* pproduct;
	static char* name;					// Module title name that will be displayed in the menu
	static int name_len;				// Length of the name string excluding terminating NULL.
	static char* vendorname;			// Module vendor name
	static int vendorname_len;			// Length of the vendor name string excluding terminating NULL.

	static const std::string modelLabels[16];

protected:
	static CBitmap* panel;

private:
	// Knobs
	ModuleKnob* kfreq, *kharm, *ktimb, *klpg_color, *kmorph, *klpg_decay, *ktimbcv, *kfreqcv, *kmorphcv;

	// Buttons
	//COnOffButton* boffset;		
	CKickButtonEx* bmodel1, * bmodel2;

	// LEDs
	CMovieBitmap *lmodel[8];

	// Patch points
	PatchPoint* ppeng, *pptimb, *ppfreq, *ppmorph, *ppharm, *pptrig, *pplevel, *ppnote;
	PatchPoint* ppout, * ppaux;

	// Switches
	//CVerticalSwitch *sw1;

	// Menus
	COptionMenu* more_menu; // , * mch[2], * mmeter;

	// Screws
	CMovieBitmap* screw1, * screw2;

	// knob change pool
	//ModuleKnobExPool<Plaits_NUM_KNOBS> chpool;

	// VCVR Port
	plaits::Voice *plvoice;
	plaits::Patch patch = {};
	char shared_buffer[16384] = {};
	//float triPhase = 0.f, phase_rate = 0.f;
	dsp::SampleRateConverter<2> outputSrc;
	// 780 buffer size supports arround 16x oversampling when sampling rate is 48000
	// Formula for calculation: required_buffer_size = block_size * sample_rate / hardware_module_sample_rate
	dsp::DynDoubleRingBuffer<dsp::Frame<2>> *outputBuffer;
	bool lowCpu = false;
	bool lpgMode = false;
	dsp::BooleanTrigger model1Trigger;
	dsp::BooleanTrigger model2Trigger;
	plaits::Voice::Frame output[PLAITS_BLOCKSIZE] = {};
	dsp::Frame<2> outputFrames[PLAITS_BLOCKSIZE] = {};

	float pitchadd;
	plaits::Modulations modulations;
};
#endif
