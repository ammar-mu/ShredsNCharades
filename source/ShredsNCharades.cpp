#include "ShredsNCharades.h"

// Currently only rescale() and crossfade() is used from math.hpp. So I just copy thier code
//#include "math.hpp"
//using namespace rack::math;

/** Rescales `x` from the range `[xMin, xMax]` to `[yMin, yMax]`.
*/
inline float rescale(float x, float xMin, float xMax, float yMin, float yMax) {
	return yMin + (x - xMin) / (xMax - xMin) * (yMax - yMin);
}
/** Linearly interpolates between `a` and `b`, from `p = 0` to `p = 1`.
*/
inline float crossfade(float a, float b, float p) {
	return a + (b - a) * p;
}

// This is the one in VSTGUI. 
extern OSVERSIONINFOEX	gSystemVersion;

// MIDI Frequencies Table
extern float mfreqtab[128];

#define SNC_BIG_KNOB_IMAGES		139
#define SNC_MEDIUM_KNOB_IMAGES	69
#define SNC_SMALL_KNOB_IMAGES	59
#define SNC_TINY_KNOB_IMAGES	41
#define SNC_LED_GYR_IMAGES		75
#define SNC_LED_GREEN_IMAGES	25
#define SNC_LED_RED_IMAGES		25
#define SNC_LED_YELLOW_IMAGES	25
#define SNC_FADER_IMAGES		69

// ----------------------------------
// The following few functions and struct was taken from NanoVG. 
// Could have just included nanovg.h. But really don't need it at the moment
// License can be found in RackSDK/dep/include/nanovg.h
struct NVGcolor {
	union {
		float rgba[4];
		struct {
			float r, g, b, a;
		};
	};
};
typedef struct NVGcolor NVGcolor;

float nvg__hue(float h, float m1, float m2)
{
	if (h < 0) h += 1;
	if (h > 1) h -= 1;
	if (h < 1.0f / 6.0f)
		return m1 + (m2 - m1) * h * 6.0f;
	else if (h < 3.0f / 6.0f)
		return m2;
	else if (h < 4.0f / 6.0f)
		return m1 + (m2 - m1) * (2.0f / 3.0f - h) * 6.0f;
	return m1;
}

float nvg__modf(float a, float b) { return fmodf(a, b); }
float nvg__clampf(float a, float mn, float mx) { return a < mn ? mn : (a > mx ? mx : a); }

NVGcolor nvgHSLA(float h, float s, float l, unsigned char a)
{
	float m1, m2;
	NVGcolor col;
	h = nvg__modf(h, 1.0f);
	if (h < 0.0f) h += 1.0f;
	s = nvg__clampf(s, 0.0f, 1.0f);
	l = nvg__clampf(l, 0.0f, 1.0f);
	m2 = l <= 0.5f ? (l * (1 + s)) : (l + s - l * s);
	m1 = 2 * l - m2;
	col.r = nvg__clampf(nvg__hue(h + 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);
	col.g = nvg__clampf(nvg__hue(h, m1, m2), 0.0f, 1.0f);
	col.b = nvg__clampf(nvg__hue(h - 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);
	col.a = a / 255.0f;
	return col;
}
// NanoVG end
// ----------------------------------

// Adapted from ChatGPT version. Saturation is ignored. I don't need it in my case.
float Rgb2Hue(uint8_t r, uint8_t g, uint8_t b)
{
	float min, max, delta, hue;

	min = r < g ? r : g;
	min = min < b ? min : b;

	max = r > g ? r : g;
	max = max > b ? max : b;

	delta = max - min;
	if (delta < 0.00001f)
	{
		return 0; // undefined, can return NAN or any value that indicates undefined hue
	}
	if (r >= max)                     // > is bogus, just keeps compiler happy
		hue = (g - b) / delta;        // between yellow & magenta
	else if (g >= max)
		hue = 2.0f + (b - r) / delta;  // between cyan & yellow
	else
		hue = 4.0f + (r - g) / delta;  // between magenta & cyan

	hue *= 60.0f;                              // degrees

	if (hue < 0.0f)
		hue += 360.0f;

	hue /= 360.0f;  // Normalize to 0-1

	return hue;
}

int GetParentDir(char* str)
{	// Makes given path, the parent and returns the length
	int i;

	i = strlen(str) - 1; if (str[i] == '\\') i--;
	while (i >= 0)
	{
		if (str[i] == '\\') { str[i + 1] = '\0'; return i + 1; }
		i--;
	}
	return -1;
}

// Probably will add this later to Module class
CKickButtonEx* ModAddKickButtonEx(Module* mod, float x, float y, CBitmap* bitmap, int num_images, CControlListener* listener)
{	// x,y are the cordinates for the centre
	long tag;
	CKickButtonEx* btemp;
	CRect r(CPoint(0, 0), CPoint(1, 1));

	x = Module::FSkinScale(x); y = Module::FSkinScale(y);
	r.moveTo(FRound(x - (float)(bitmap->getWidth() / 2)), FRound(y - (float)(bitmap->getHeight() / (2 * num_images))));
	r.setSize(CPoint(bitmap->getWidth(), bitmap->getHeight() / num_images));
	btemp = new CKickButtonEx(r, listener, tag = mod->GetFreeTag(), bitmap); mod->addView(btemp); mod->RegisterTag(btemp, tag);
	return btemp;
}

using namespace Gdiplus;

//---------------------------------------------
// Clouds
CBitmap* Clouds::panel = NULL;
char* Clouds::name = "Baadalon";
int Clouds::name_len = 0;
char* Clouds::vendorname = "ShredsNCharades";
int Clouds::vendorname_len = 0;
Product* Clouds::pproduct = NULL;

Clouds::Clouds(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{
	PatchPoint* temp[6];

	// Create The Knobs
	kposition = AddModuleKnob(39, 101, mcbits[knob_medium_red], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kposition->setValue(0.5); kposition->svalue = 0.5; //ADDPOOLKNOB(chpool,kin1);

	ksize = AddModuleKnob(101, 101, mcbits[knob_medium_green], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	ksize->setValue(0.5); ksize->svalue = 0.5; //ADDPOOLKNOB(chpool,kin2);

	kpitch = AddModuleKnob(164, 101, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kpitch->setValue(0.5); kpitch->svalue = 0.5; //ADDPOOLKNOB(chpool,kin3);

	kingain = AddModuleKnob(26, 160, mcbits[knob_small_red], SNC_SMALL_KNOB_IMAGES, false, listener);
	kingain->setValue(0.5); kingain->svalue = 0.5; //ADDPOOLKNOB(chpool,kout);

	kdensity = AddModuleKnob(76, 160, mcbits[knob_small_red], SNC_SMALL_KNOB_IMAGES, false, listener);
	kdensity->setValue(0.5); kdensity->svalue = 0.5;

	ktexture = AddModuleKnob(126, 160, mcbits[knob_small_green], SNC_SMALL_KNOB_IMAGES, false, listener);
	ktexture->setValue(0.5); ktexture->svalue = 0.5;

	kblend = AddModuleKnob(178, 160, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kblend->setValue(0.5); kblend->svalue = 0.5;

	kspread = AddModuleKnob(178, 160, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener); kspread->setVisible(false);
	kspread->setValue(0.0); kspread->svalue = 0.0;

	kfeedback = AddModuleKnob(178, 160, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener); kfeedback->setVisible(false);
	kfeedback->setValue(0.0); kfeedback->svalue = 0.0;

	kreverb = AddModuleKnob(178, 160, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener); kreverb->setVisible(false);
	kreverb->setValue(0.0); kreverb->svalue = 0.0;

	// Create Patch Points
	ppfreeze = AddPatchPoint(17, 218, ppTypeInput, ppbit, 0, listener);
	pptrig = AddPatchPoint(51, 218, ppTypeInput, ppbit, 0, listener);
	ppposition = AddPatchPoint(84, 218, ppTypeInput, ppbit, 0, listener);
	ppsize = AddPatchPoint(117, 218, ppTypeInput, ppbit, 0, listener);
	pppitch = AddPatchPoint(150, 218, ppTypeInput, ppbit, 0, listener);
	ppblend = AddPatchPoint(183, 218, ppTypeInput, ppbit, 0, listener);

	ppinl = AddPatchPoint(17, 251, ppTypeInput, ppbit, 0, listener);
	ppinr = AddPatchPoint(51, 251, ppTypeInput, ppbit, 0, listener);
	ppdensity = AddPatchPoint(84, 251, ppTypeInput, ppbit, 0, listener);
	pptexture = AddPatchPoint(117, 251, ppTypeInput, ppbit, 0, listener);
	ppoutl = AddPatchPoint(150, 251, ppTypeOutput, ppbit, 0, listener);
	ppoutr = AddPatchPoint(183, 251, ppTypeOutput, ppbit, 0, listener);

	// Buttons
	bfreez = AddOnOffButton(20, 43, mcbits[button_led], 2, listener, COnOffButton::kPostListenerUpdate);
	bmode = AddKickButton(166, 45, mcbits[black_butbit_tiny], 2, listener);
	bquality = AddKickButton(187.6, 45, mcbits[black_butbit_tiny], 2, listener);
	balt = AddKickButton(187.6, 72, mcbits[black_butbit_tiny], 2, listener);


	lmix = AddMovieBitmap(65.6, 44.7, mcbits[led_big_green], SNC_LED_GREEN_IMAGES, listener);
	lpan = AddMovieBitmap(89.6, 44.7, mcbits[led_big_green], SNC_LED_GREEN_IMAGES, listener);
	lfb = AddMovieBitmap(114, 44.7, mcbits[led_big_yellow], SNC_LED_YELLOW_IMAGES, listener);
	lreverb = AddMovieBitmap(138, 44.7, mcbits[led_big_red], SNC_LED_RED_IMAGES, listener);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	PutRightScrews(screw3, screw4, listener);
	InitPatchPoints(0.0);

	const int memLen = 118784;
	const int ccmLen = 65536 - 128;
	block_mem = new uint8_t[memLen]();
	block_ccm = new uint8_t[ccmLen]();
	processor = new clouds::GranularProcessor();
	memset(processor, 0, sizeof(*processor));

	processor->Init(block_mem, memLen, block_ccm, ccmLen);
	OnReset(); last_vu = 0.f; inputBuffer = NULL; outputBuffer = NULL;
	SetSampleRate(sample_rate);
}

Clouds::~Clouds()
{
	delete processor;
	delete[] block_mem;
	delete[] block_ccm;
	if (inputBuffer != NULL) delete inputBuffer;
	if (outputBuffer != NULL) delete outputBuffer;
}

void Clouds::OnReset()
{
	freeze = false;
	blendMode = 0;
	playback = clouds::PLAYBACK_MODE_GRANULAR;
	quality = 0;
}


// SoloRack calls this. It can't directly access rhe constructor since this is a Dll and pointers to constructor can not be easily achieved.
Clouds* Clouds::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Clouds(pParent, listener, psynth_comm, vvoice);
}

void Clouds::Initialize()
{
	char* stemp;

	panel = new CBitmap(dllskindir, NULL, "Clouds.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Clouds::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Clouds::GetName()
{
	return name;
}

const int Clouds::GetNameLen()
{
	return name_len;
}

const char* Clouds::GetVendorName()
{
	return vendorname;
}

const int Clouds::GetVendorNameLen()
{
	return vendorname_len;
}

const int Clouds::GetType()
{
	return kModifierEffect;
}

Product* Clouds::Activate(char* fullname, char* email, char* serial)
{	// Replace this with your own implementation
	// This should return a pointer to the activated product. Or NULL if activation fails
	// In our Clouds, NULL is retuned because the module is already active all the time, so no need to activate.
	return NULL;

	// Here is a simplistic example.
	//if (pproduct!=NULL) delete pproduct;
	//pproduct = new Product(0,1,NULL,false,"ShredsNCharades Baadalon");		// Product names has to include vendor name to ensure uniquenes
	//return pproduct->Activate(fullname,email,serial);
}

bool Clouds::IsActive()
{	// This test module is Active. Replace this with your own check. 
	// It's better not to store this active/inactive status in an obvious place in memory, like a data member of the module or like that.
	// It's even better if the status is not stored at all, but rather a sophisticated test is done
	return true;

	// simple example
	//if (pproduct!=NULL) return pproduct->IsActive(); else return false;
}

Product* Clouds::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Clouds::InstanceIsActive()
{
	return this->IsActive();
}

const char* Clouds::GetProductName()
{	// Change this to your own product name. 
	return "ShredsNCharades Baadalon";
}

void Clouds::UpdateBlendKnob()
{
	kblend->setVisible(blendMode == 0);
	kspread->setVisible(blendMode == 1);
	kfeedback->setVisible(blendMode == 2);
	kreverb->setVisible(blendMode == 3);

	lmix->value = (blendMode == 0);
	lpan->value = (blendMode == 1);
	lfb->value = (blendMode == 2);
	lreverb->value = (blendMode == 3);
	leds_wait = CLOUDS_LEDS_WAIT * sample_rate / 32.f;
}

inline void Clouds::UpdateProcessor()
{
	// Set up processor
	processor->set_playback_mode(playback);
	if (quality == 4)
		processor->set_quality(0);
	else
		processor->set_quality(quality);
	processor->Prepare();
}

void Clouds::ValueChanged(CControl* pControl)
{
	if (pControl == bmode && bmode->value >= 0.5)
	{
		blendMode = (blendMode + 1) % 4;
		UpdateBlendKnob();
	}
	else if (pControl == bfreez)
		freeze = bfreez->value >= 0.5;
	else if (pControl == bquality && bquality->value == 1.0)
	{
		quality = (quality + 1) % 5;
		// Mode 4 doesn't sample rate convertion
		if (quality == 4) processor->set_sample_rate(sample_rate);
		else processor->set_sample_rate(32000.f);
		//UpdateProcessor();

		lmix->value = (quality == 0 || quality == 4);
		lpan->value = (quality == 1 || quality == 4);
		lfb->value = (quality == 2 || quality == 4);
		lreverb->value = (quality == 3 || quality == 4);
		leds_wait = CLOUDS_LEDS_WAIT * sample_rate / 32.f;
	}
	else if (pControl == balt && balt->value == 1.0)
	{
		lmix->value = 0.f; lpan->value = 0.f;
		lfb->value = 0.f; lreverb->value = 0.f;

		switch (playback)
		{
		case clouds::PLAYBACK_MODE_GRANULAR:
			playback = clouds::PLAYBACK_MODE_STRETCH; lpan->value = 1.f; break;
		case clouds::PLAYBACK_MODE_STRETCH:
			playback = clouds::PLAYBACK_MODE_LOOPING_DELAY; lfb->value = 1.f; break;
		case clouds::PLAYBACK_MODE_LOOPING_DELAY:
			playback = clouds::PLAYBACK_MODE_SPECTRAL; lreverb->value = 1.f;  break;
		case clouds::PLAYBACK_MODE_SPECTRAL:
			playback = clouds::PLAYBACK_MODE_GRANULAR; lmix->value = 1.f; break;
		}
		leds_wait = CLOUDS_LEDS_WAIT * sample_rate / 32.f;

		//UpdateProcessor();
		// Doesn't work!!
		//playback = (playback + 1) % clouds::PLAYBACK_MODE_LAST;
	}


	// If a knob change pool is defined
	/*else
	{	bool result;
		IS_KNOB_IN_POOL(chpool,pControl,result);
		if (result)
		{	EnterProcessingCriticalSection();
			POOL_KNOB_VALUECHANGED(chpool,pControl);
			LeaveProcessingCriticalSection();
		}
	}*/

}

void Clouds::SetSampleRate(float sr)
{
	int i,j;

	Module::SetSampleRate(sr);

	if (quality == 4) processor->set_sample_rate(sample_rate);
	else processor->set_sample_rate(32000.f);
	inputSrc.setRates(sample_rate, 32000);
	outputSrc.setRates(32000, sample_rate);
	//UpdateProcessor();
	
	i = (float)32 * sample_rate / 32000.f + 4;		// + 4 for safty
	if (i < 32) i = 32;
	if (inputBuffer != NULL) delete inputBuffer;
	inputBuffer = new dsp::DynDoubleRingBuffer<dsp::Frame<2>>(i);
	if (outputBuffer != NULL) delete outputBuffer;
	outputBuffer = new dsp::DynDoubleRingBuffer<dsp::Frame<2>>(i);

	// Push intial empty data to input buffer
	dsp::Frame<2> f; f.samples[0] = 0.f; f.samples[1] = 0.f;
	for (j = 0; j < i-1; j++) inputBuffer->push(f);		// i-1 (not i) because we can allow first sample in
}

int Clouds::GetPresetSize()
{
	return GetControlsValuesSize() + sizeof(playback) + sizeof(quality) + sizeof(blendMode);
}

void Clouds::SavePreset(void* pdata, int size)
{
	char* pcdata = (char*)pdata;
	SaveControlsValues(pcdata);
	pcdata += GetControlsValuesSize();

	// Save none control data
	*(clouds::PlaybackMode*)pcdata = playback; pcdata += sizeof(playback);
	*(int*)pcdata = quality; pcdata += sizeof(quality);
	*(int*)pcdata = blendMode; pcdata += sizeof(blendMode);
}

void Clouds::LoadPreset(void* pdata, int size, int version)
{
	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();

	LoadControlsValues(pcdata, csize);
	pcdata += csize;

	// Load non control data
	playback = *(clouds::PlaybackMode*)pcdata; pcdata += sizeof(playback);
	quality = *(int*)pcdata; pcdata += sizeof(quality);
	blendMode = *(int*)pcdata; pcdata += sizeof(blendMode);

	UpdateBlendKnob();

	if (quality == 4) processor->set_sample_rate(sample_rate);
	else processor->set_sample_rate(32000.f);
	//UpdateProcessor();
}

const char* Clouds::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/clouds/";
}

inline void Clouds::ProcessSample()
{
	dsp::Frame<2> inputFrame = {};
	dsp::Frame<2> outputFrame = {};

	// Get input
	if (!inputBuffer->full())
	{
		inputFrame.samples[0] = ppinl->in * kingain->value;
		inputFrame.samples[1] = ppinr->num_cables ? ppinr->in * kingain->value : inputFrame.samples[0];
		inputBuffer->push(inputFrame);
	}

	//if (freezeTrigger.process(bfreez->value)) freeze ^= true;
	//if (blendTrigger.process(bmode->value)) 
	//	blendMode = (blendMode + 1) % 4;

	// Trigger
	if (pptrig->in >= 1.0) triggered = true;

	// Render frames
	if (outputBuffer->empty())
	{
		clouds::ShortFrame input[32] = {};
		dsp::Frame<2> inputFrames[32];
		if (quality != 4)
		{	// Convert input buffer
			int inLen = inputBuffer->size();
			int outLen = 32;
			inputSrc.process(inputBuffer->startData(), &inLen, inputFrames, &outLen);
			inputBuffer->startIncr(inLen);

			// We might not fill all of the input buffer if there is a deficiency, but this cannot be avoided due to imprecisions between the input and output SRC.
			for (int i = 0; i < outLen; i++) {
				input[i].l = CLIP(inputFrames[i].samples[0] * 32767.0f, -32768.0f, 32767.0f);
				input[i].r = CLIP(inputFrames[i].samples[1] * 32767.0f, -32768.0f, 32767.0f);
			}
		}
		else
		{	// No sample rate convertion
			int blen = mmin(32, inputBuffer->size());
			for (int i = 0; i < blen; i++)
			{
				input[i].l = CLIP(inputBuffer->startData()[i].samples[0] * 32767.0f, -32768.0f, 32767.0f);
				input[i].r = CLIP(inputBuffer->startData()[i].samples[1] * 32767.0f, -32768.0f, 32767.0f);
			}
			inputBuffer->startIncr(blen);
		}

		UpdateProcessor();

		clouds::Parameters* p = processor->mutable_parameters();
		p->trigger = triggered;
		p->gate = triggered;
		p->freeze = freeze || (ppfreeze->in >= 1.0);
		p->position = CLIP(kposition->value + ppposition->in, 0.0f, 1.0f);
		p->size = CLIP(ksize->value + ppsize->in, 0.0f, 1.0f);
		p->pitch = CLIP((SCALE(kpitch->value, -2.f, 2.f) + pppitch->in * MIDI_TUNE_FACTOR) * 12.0f, -48.0f, 48.0f);
		p->density = CLIP(kdensity->value + ppdensity->in, 0.0f, 1.0f);
		p->texture = CLIP(ktexture->value + pptexture->in, 0.0f, 1.0f);
		p->dry_wet = kblend->value;
		p->stereo_spread = kspread->value;
		p->feedback = kfeedback->value;

		p->reverb = kreverb->value;
		float blend = ppblend->in;

		switch (blendMode)
		{
		case 0:
			p->dry_wet += blend;
			p->dry_wet = CLIP(p->dry_wet, 0.0f, 1.0f);
			break;
		case 1:
			p->stereo_spread += blend;
			p->stereo_spread = CLIP(p->stereo_spread, 0.0f, 1.0f);
			break;
		case 2:
			p->feedback += blend;
			p->feedback = CLIP(p->feedback, 0.0f, 1.0f);
			break;
		case 3:
			p->reverb += blend;
			p->reverb = CLIP(p->reverb, 0.0f, 1.0f);
			break;
		}

		clouds::ShortFrame output[32];
		processor->Process(input, output, 32);

		if (quality != 4)
		{	// Convert output buffer
			dsp::Frame<2> outputFrames[32];
			for (int i = 0; i < 32; i++) {
				outputFrames[i].samples[0] = output[i].l / 32768.0;
				outputFrames[i].samples[1] = output[i].r / 32768.0;
			}

			int inLen = 32;
			int outLen = outputBuffer->capacity();
			outputSrc.process(outputFrames, &inLen, outputBuffer->endData(), &outLen);
			outputBuffer->endIncr(outLen);
		}
		else
		{	// No sample rate convertion
			for (int i = 0; i < 32; i++)
			{
				outputBuffer->endData()[i].samples[0] = output[i].l / 32768.0;
				outputBuffer->endData()[i].samples[1] = output[i].r / 32768.0;
			}
			outputBuffer->endIncr(32);
		}
		triggered = false;
		outputFrame = outputBuffer->shift();

		// Lights
		if (leds_wait <= 0)
		{
			dsp::Frame<2> lightFrame = freeze ? outputFrame : inputFrame;
			float tf, tf2;
			float vu = fmaxf(fabsf(lightFrame.samples[0]), fabsf(lightFrame.samples[1]));

			// Low pass
			vu = last_vu + 0.05 * (vu - last_vu);
			last_vu = vu;

			// Rough and quick LEDs
			tf2 = 4.0 * vu; if (tf2 > 1.f) lmix->value = 1.f; else lmix->value = tf2;
			tf = tf2 - 0.25; lpan->value = CLIP(tf, 0.f, 1.f);
			tf = tf2 - 0.5; lfb->value = CLIP(tf, 0.f, 1.f);
			tf = tf2 - 0.75; lreverb->value = CLIP(tf, 0.f, 1.f);
		}
		else leds_wait--;

	}
	else outputFrame = outputBuffer->shift();

	// Set output
	ppoutl->out = outputFrame.samples[0];
	ppoutr->out = outputFrame.samples[1];
}



//---------------------------------------------
// Rings
CBitmap* Rings::panel = NULL;
char* Rings::name = "Riga";
int Rings::name_len = 0;
char* Rings::vendorname = "ShredsNCharades";
int Rings::vendorname_len = 0;
Product* Rings::pproduct = NULL;

Rings::Rings(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{
	PatchPoint* temp[6];

	// Create The Knobs
	kfreq = AddModuleKnob(41, 84, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kfreq->setValue(0.5);

	kstruct = AddModuleKnob(116, 84, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kstruct->setValue(0.5);

	kbright = AddModuleKnob(23, 143, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kbright->setValue(0.5);

	kdamping = AddModuleKnob(77, 143, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kdamping->setValue(0.5);

	kposition = AddModuleKnob(131, 143, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kposition->setValue(0.5);

	kbright_cv = AddModuleKnob(19, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kbright_cv->setValue(0.5);

	kfreq_cv = AddModuleKnob(48, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kfreq_cv->setValue(0.5);

	kdamping_cv = AddModuleKnob(78, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kdamping_cv->setValue(0.5);

	kstruct_cv = AddModuleKnob(107, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kstruct_cv->setValue(0.5);

	kposition_cv = AddModuleKnob(137, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kposition_cv->setValue(0.5);

	// Create Patch Points
	ppbright = AddPatchPoint(19, 216, ppTypeInput, ppbit, 0, listener);
	ppfreq = AddPatchPoint(48, 216, ppTypeInput, ppbit, 0, listener);
	ppdamping = AddPatchPoint(77, 216, ppTypeInput, ppbit, 0, listener);
	ppstruct = AddPatchPoint(107, 216, ppTypeInput, ppbit, 0, listener);
	ppposition = AddPatchPoint(137, 216, ppTypeInput, ppbit, 0, listener);

	ppstrum = AddPatchPoint(19, 251, ppTypeInput, ppbit, 0, listener);
	pppitch = AddPatchPoint(48, 251, ppTypeInput, ppbit, 0, listener);
	ppin = AddPatchPoint(77, 251, ppTypeInput, ppbit, 0, listener);
	ppodd = AddPatchPoint(107, 251, ppTypeOutput, ppbit, 0, listener);
	ppeven = AddPatchPoint(137, 251, ppTypeOutput, ppbit, 0, listener);

	// Buttons
	bpoly = AddKickButton(14, 37, mcbits[black_butbit_tiny], 2, listener);
	bresonator = AddKickButton(142, 37, mcbits[black_butbit_tiny], 2, listener);
	beaster = AddOnOffButton(78, 37, mcbits[black_butbit_tiny], 2, listener, COnOffButton::kPostListenerUpdate);
	blow_cpu = AddOnOffButton(88, 281, mcbits[black_butbit_tiny], 2, listener, COnOffButton::kPostListenerUpdate);

	lpoly = AddMovieBitmap(28, 36, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	lresonator = AddMovieBitmap(128, 36, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	llow_cpu = AddMovieBitmap(102, 281, mcbits[led_small_red], SNC_LED_RED_IMAGES, listener);
	//lfb = AddMovieBitmap(114, 44, mcbits[led_red], LED_RED_IMAGES, listener);
	//lreverb = AddMovieBitmap(138, 44, mcbits[led_red], LED_RED_IMAGES, listener);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	PutRightScrews(screw3, screw4, listener);
	InitPatchPoints(0.0);

	inputBuffer = NULL; outputBuffer = NULL;
	SetSampleRate(sample_rate);
	strummer.Init(0.01, 44100.f / 24);
	part.Init(reverb_buffer);
	string_synth.Init(reverb_buffer);

	// Polyphony
	int polyphony = 1 << polyphonyMode;
	part.set_polyphony(polyphony);

	UpdateResonatorModel();

	// LEDs
	lpoly->value = ((float)polyphonyMode + 1.f) / 3.f;
	lresonator->value = ((float)resonatorModel + 1.f) / 6.f;
}

Rings::~Rings()
{	if (inputBuffer != NULL) delete inputBuffer;
	if (outputBuffer != NULL) delete outputBuffer;
}

// SoloRack calls this. It can't directly access rhe constructor since this is a Dll and pointers to constructor can not be easily achieved.
Rings* Rings::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Rings(pParent, listener, psynth_comm, vvoice);
}

void Rings::Initialize()
{
	panel = new CBitmap(dllskindir, NULL, "Rings.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Rings::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Rings::GetName()
{
	return name;
}

const int Rings::GetNameLen()
{
	return name_len;
}

const char* Rings::GetVendorName()
{
	return vendorname;
}

const int Rings::GetVendorNameLen()
{
	return vendorname_len;
}

const int Rings::GetType()
{
	return kOscillatorSource;
}

Product* Rings::Activate(char* fullname, char* email, char* serial)
{	// Replace this with your own implementation
	// This should return a pointer to the activated product. Or NULL if activation fails
	// In our Rings, NULL is retuned because the module is already active all the time, so no need to activate.
	return NULL;

	// Here is a simplistic example.
	//if (pproduct!=NULL) delete pproduct;
	//pproduct = new Product(0,1,NULL,false,"ShredsNCharades Baadalon");		// Product names has to include vendor name to ensure uniquenes
	//return pproduct->Activate(fullname,email,serial);
}

bool Rings::IsActive()
{	// This test module is Active. Replace this with your own check. 
	// It's better not to store this active/inactive status in an obvious place in memory, like a data member of the module or like that.
	// It's even better if the status is not stored at all, but rather a sophisticated test is done
	return true;

	// simple example
	//if (pproduct!=NULL) return pproduct->IsActive(); else return false;
}

Product* Rings::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Rings::InstanceIsActive()
{
	return this->IsActive();
}

const char* Rings::GetProductName()
{	// Change this to your own product name. 
	return "ShredsNCharades Riga";
}

//inline void Rings::UpdateProcessor()
//{
//	// Set up processor
//	processor->set_playback_mode(playback);
//	if (quality == 4)
//		processor->set_quality(0);
//	else
//		processor->set_quality(quality);
//	processor->Prepare();
//}

void Rings::ValueChanged(CControl* pControl)
{
	if (pControl == bpoly && bpoly->value >= 0.5)
	{
		EnterProcessingCriticalSection();
		polyphonyMode = (polyphonyMode + 1) % 3;

		// Polyphony
		int polyphony = 1 << polyphonyMode;
		if (part.polyphony() != polyphony)
			part.set_polyphony(polyphony);

		LeaveProcessingCriticalSection();

		// LED
		lpoly->value = ((float)polyphonyMode + 1.f) / 3.f;
	}
	else if (pControl == bresonator && bresonator->value >= 0.5)
	{
		EnterProcessingCriticalSection();
		resonatorModel = (rings::ResonatorModel)((resonatorModel + 1) % 6);
		UpdateResonatorModel();
		LeaveProcessingCriticalSection();

		// LED
		lresonator->value = ((float)resonatorModel + 1.f) / 6.f;
	}
	else if (pControl == beaster)
	{
		EnterProcessingCriticalSection();
		easterEgg = beaster->value >= 0.5;
		UpdateResonatorModel();
		LeaveProcessingCriticalSection();
	}
	else if (pControl == blow_cpu)
	{
		EnterProcessingCriticalSection();
		if (blow_cpu->value >= 0.5f) low_cpu = 1; else low_cpu = 0;
		llow_cpu->value = low_cpu;
		SetSampleRate(sample_rate);
		LeaveProcessingCriticalSection();
	}
}

void Rings::SetSampleRate(float sr)
{	int i,j;

	Module::SetSampleRate(sr);

	inputSrc.setRates(sample_rate, 48000);
	outputSrc.setRates(48000, sample_rate);

	// This is just a quick trick to allow it to work without sample rate convertion
	// it's not perfect. kSampleRate is const. It is a whole mess changing it to dynamic.
	if (low_cpu == 1) sr_notefix = log2(48000.f / sample_rate);
	else sr_notefix = 0.f;

	i = (float)24 * sample_rate / 48000 + 4;		// + 4 for safty
	if (i < 24) i = 24;
	if (inputBuffer != NULL) delete inputBuffer;
	inputBuffer = new dsp::DynDoubleRingBuffer<dsp::Frame<1>>(i);
	if (outputBuffer != NULL) delete outputBuffer;
	outputBuffer = new dsp::DynDoubleRingBuffer<dsp::Frame<2>>(i);

	// Push intial empty data to input buffer
	dsp::Frame<1> f; f.samples[0] = 0.f;
	for (j = 0; j < i - 1; j++) inputBuffer->push(f);		// i-1 (not i) because we can allow first sample in
}

int Rings::GetPresetSize()
{
	return GetControlsValuesSize() + sizeof(polyphonyMode) + sizeof(resonatorModel);
}

void Rings::SavePreset(void* pdata, int size)
{
	char* pcdata = (char*)pdata;
	SaveControlsValues(pcdata);
	pcdata += GetControlsValuesSize();

	// Save none control data
	*(int*)pcdata = polyphonyMode; pcdata += sizeof(polyphonyMode);
	*(rings::ResonatorModel*)pcdata = resonatorModel; pcdata += sizeof(resonatorModel);
}

void Rings::LoadPreset(void* pdata, int size, int version)
{
	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();

	LoadControlsValues(pcdata, csize);
	pcdata += csize;

	// Load non control data
	polyphonyMode = *(int*)pcdata; pcdata += sizeof(polyphonyMode);
	resonatorModel = *(rings::ResonatorModel*)pcdata; pcdata += sizeof(resonatorModel);

	// Polyphony
	int polyphony = 1 << polyphonyMode;
	part.set_polyphony(polyphony);

	UpdateResonatorModel();

	// LEDs
	lpoly->value = ((float)polyphonyMode + 1.f) / 3.f;
	lresonator->value = ((float)resonatorModel + 1.f) / 6.f;
	//llow_cpu->value = quality; // No need
}

void Rings::UpdateResonatorModel()
{
	if (easterEgg)
		string_synth.set_fx((rings::FxType)resonatorModel);
	else
		part.set_model(resonatorModel);
}

const char* Rings::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/rings/";
}

inline void Rings::ProcessSample()
{
	bool strum;

	// Get input
	if (!inputBuffer->full()) {
		dsp::Frame<1> f;
		f.samples[0] = ppin->in;
		inputBuffer->push(f);
	}

	// Render frames
	if (outputBuffer->empty())
	{	//float in[24] = {};
		float* in;
		// Convert input buffer
		if (low_cpu == 0)
		{
			in = pin;
			int inLen = inputBuffer->size();
			int outLen = 24;
			inputSrc.process(inputBuffer->startData(), &inLen, (dsp::Frame<1>*) in, &outLen);
			inputBuffer->startIncr(inLen);
		}
		else
		{	// No sample rate convertion
			int blen = mmin(24, inputBuffer->size());
			//for (int i = 0; i < blen; i++)
			//	in[i] = inputBuffer->startData()[i].samples[0];
			
			// This only works here because inputBuffer has 1 float per elemnt
			in = (float*)inputBuffer->startData();
			inputBuffer->startIncr(blen);

		}

		// Patch
		rings::Patch patch;
		float structure = kstruct->value + 3.3 * dsp::quadraticBipolar(SCALE(kstruct_cv->value, -1.f, 1.f)) * ppstruct->in;
		patch.structure = CLIP(structure, 0.0f, 0.9995f);
		patch.brightness = CLIP(kbright->value + 3.3 * dsp::quadraticBipolar(SCALE(kbright_cv->value, -1.f, 1.f)) * ppbright->in, 0.0f, 1.0f);
		patch.damping = CLIP(kdamping->value + 3.3 * dsp::quadraticBipolar(SCALE(kdamping_cv->value, -1.f, 1.f)) * ppdamping->in, 0.0f, 0.9995f);
		patch.position = CLIP(kposition->value + 3.3 * dsp::quadraticBipolar(SCALE(kposition_cv->value, -1.f, 1.f)) * ppposition->in, 0.0f, 0.9995f);

		// Performance
		rings::PerformanceState performance_state;
		float transpose = SCALE(kfreq->value, 0.f, 60.f);
		if (pppitch->num_cables)
		{
			performance_state.note = 12.0 * (MIDI_TUNE_FACTOR * pppitch->in + sr_notefix);
			// Quantize transpose if pitch input is connected
			transpose = roundf(transpose);
		}
		else performance_state.note = 1.f;

		performance_state.tonic = 12.0 + CLIP(transpose, 0.0f, 60.0f);
		performance_state.fm = CLIP(48.0 * 3.3 * dsp::quarticBipolar(SCALE(kfreq_cv->value, -1.f, 1.f)) * ppfreq->in, -48.0f, 48.0f);

		performance_state.internal_exciter = !ppin->num_cables;
		performance_state.internal_strum = !ppstrum->num_cables;
		performance_state.internal_note = !pppitch->num_cables;

		strum = ppstrum->in >= HIGH_CV;
		performance_state.strum = strum && !lastStrum;
		lastStrum = strum;

		performance_state.chord = CLIP((int)roundf(structure * (rings::kNumChords - 1)), 0, rings::kNumChords - 1);

		// Process audio
		float out[24];
		float aux[24];
		if (easterEgg)
		{
			strummer.Process(NULL, 24, &performance_state);
			string_synth.Process(performance_state, patch, in, out, aux, 24);
		}
		else
		{
			strummer.Process(in, 24, &performance_state);
			part.Process(performance_state, patch, in, out, aux, 24);
		}

		if (low_cpu == 0)
		{	// Convert output buffer
			dsp::Frame<2> outputFrames[24];
			for (int i = 0; i < 24; i++)
			{
				outputFrames[i].samples[0] = out[i];
				outputFrames[i].samples[1] = aux[i];
			}
			int inLen = 24;
			int outLen = outputBuffer->capacity();
			outputSrc.process(outputFrames, &inLen, outputBuffer->endData(), &outLen);
			outputBuffer->endIncr(outLen);
		}
		else
		{	// No sample rate convertion
			for (int i = 0; i < 24; i++)
			{
				outputBuffer->endData()[i].samples[0] = out[i];
				outputBuffer->endData()[i].samples[1] = aux[i];
			}
			outputBuffer->endIncr(24);
		}
	}

	// Set output
	// if (!outputBuffer->empty()) 
	{	dsp::Frame<2> outputFrame = outputBuffer->shift();
	// "Note that you need to insert a jack into each output to split the signals: when only one jack is inserted, both signals are mixed together."
	if (ppodd->num_cables && ppeven->num_cables)
	{
		//ppodd->out = CLIP(outputFrame.samples[0], -1.0, 1.0);
		//ppeven->out = CLIP(outputFrame.samples[1], -1.0, 1.0);

		ppodd->out = outputFrame.samples[0];
		ppeven->out = outputFrame.samples[1];
	}
	else
	{
		//float v = CLIP(outputFrame.samples[0] + outputFrame.samples[1], -1.0, 1.0);
		float v = outputFrame.samples[0] + outputFrame.samples[1];
		ppodd->out = ppeven->out = v;
	}
	}
}



//---------------------------------------------
// Braids
CBitmap* Braids::panel = NULL;
char* Braids::name = "Chotiyon";
int Braids::name_len = 0;
char* Braids::vendorname = "ShredsNCharades";
int Braids::vendorname_len = 0;
Product* Braids::pproduct = NULL;
const Braids::ShapeInfo Braids::shape_infos[] = {
	{"CSAW", "Quirky sawtooth"},
	{"/\\-_", "Triangle to saw"},
	{"//-_", "Sawtooth wave with dephasing"},
	{"FOLD", "Wavefolded sine/triangle"},
	{"uuuu", "Buzz"},
	{"SUB-", "Square sub"},
	{"SUB/", "Saw sub"},
	{"SYN-", "Square sync"},
	{"SYN/", "Saw sync"},
	{"//x3", "Triple saw"},
	{"-_x3", "Triple square"},
	{"/\\x3", "Triple triangle"},
	{"SIx3", "Triple sine"},
	{"RING", "Triple ring mod"},
	{"////", "Saw swarm"},
	{"//uu", "Saw comb"},
	{"TOY*", "Circuit-bent toy"},
	{"ZLPF", "Low-pass filtered waveform"},
	{"ZPKF", "Peak filtered waveform"},
	{"ZBPF", "Band-pass filtered waveform"},
	{"ZHPF", "High-pass filtered waveform"},
	{"VOSM", "VOSIM formant"},
	{"VOWL", "Speech synthesis"},
	{"VFOF", "FOF speech synthesis"},
	{"HARM", "12 sine harmonics"},
	{"FM  ", "2-operator phase-modulation"},
	{"FBFM", "2-operator phase-modulation with feedback"},
	{"WTFM", "2-operator phase-modulation with chaotic feedback"},
	{"PLUK", "Plucked string"},
	{"BOWD", "Bowed string"},
	{"BLOW", "Blown reed"},
	{"FLUT", "Flute"},
	{"BELL", "Bell"},
	{"DRUM", "Drum"},
	{"KICK", "Kick drum circuit simulation"},
	{"CYMB", "Cymbal"},
	{"SNAR", "Snare"},
	{"WTBL", "Wavetable"},
	{"WMAP", "2D wavetable"},
	{"WLIN", "1D wavetable"},
	{"WTx4", "4-voice paraphonic 1D wavetable"},
	{"NOIS", "Filtered noise"},
	{"TWNQ", "Twin peaks noise"},
	{"CLKN", "Clocked noise"},
	{"CLOU", "Granular cloud"},
	{"PRTC", "Particle noise"},
	{"QPSK", "Digital modulation"},
};

Braids::Braids(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{
	PatchPoint* temp[6];

	// Create The Knobs
	kshape = AddModuleKnob(168, 64, mcbits[knob_small_black], SNC_SMALL_KNOB_IMAGES, false, listener);

	kfine = AddModuleKnob(34, 131, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kfine->setValue(0.5);

	kcoarse = AddModuleKnob(102, 131, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kcoarse->setValue(0.5);

	kfm = AddModuleKnob(170, 131, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kfm->setValue(0.5);

	ktimbre = AddModuleKnob(34, 192, mcbits[knob_medium_green], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	ktimbre->setValue(0.5);

	kmodulation = AddModuleKnob(102, 192, mcbits[knob_medium_green], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kmodulation->setValue(0.5);

	kcolor = AddModuleKnob(170, 192, mcbits[knob_medium_red], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kcolor->setValue(0.5);

	//kfreq_cv = AddModuleKnob(48, 183, mcbits[tknobit_black], TINY_KNOB_IMAGES, false, listener);
	//kfreq_cv->setValue(0.5);

	//kdamping_cv = AddModuleKnob(78, 183, mcbits[tknobit_black], TINY_KNOB_IMAGES, false, listener);
	//kdamping_cv->setValue(0.5);

	//kstruct_cv = AddModuleKnob(107, 183, mcbits[tknobit_black], TINY_KNOB_IMAGES, false, listener);
	//kstruct_cv->setValue(0.5);

	//kposition_cv = AddModuleKnob(137, 183, mcbits[tknobit_black], TINY_KNOB_IMAGES, false, listener);
	//kposition_cv->setValue(0.5);

	//// Create Patch Points
	pptrig = AddPatchPoint(19, 249, ppTypeInput, ppbit, 0, listener);
	pppitch = AddPatchPoint(50, 249, ppTypeInput, ppbit, 0, listener);
	ppfm = AddPatchPoint(82, 249, ppTypeInput, ppbit, 0, listener);
	pptimbre = AddPatchPoint(114, 249, ppTypeInput, ppbit, 0, listener);
	ppcolor = AddPatchPoint(146, 249, ppTypeInput, ppbit, 0, listener);
	ppout = AddPatchPoint(185, 249, ppTypeOutput, ppbit, 0, listener);

	// Set Segment/digital font to shape screen
	CColor cc = CColor();
	//shape_text = AddTextLabel(14, 41.6, NULL, 0, 126, 39);
	//shape_text = AddTextLabel(15, 47, NULL, 0, 125, 50); shape_text->setVertAlign(kTopTextWithTopCoord); // Segment14.ttf
	shape_text = AddTextLabel(14, 46, NULL, 0, 125, 62); shape_text->setVertAlign(kTopTextWithTopCoord); // DSEG14Classic-Italic.ttf

	shape_text->setTransparency(true); shape_text->setText("");
	cc.red = 175; cc.green = 210; cc.blue = 44; cc.alpha = 255;
	shape_text->setFontColor(cc);
	CFontDesc* pf;
	char* st = (char*)malloc(MAX_PATH * 2 * sizeof(*st));
	if (st != NULL)
	{
		strcpy(st, dllskindir); GetParentDir(st);
		//strcat(st, "Segment14.ttf");
		//pf = new CFontDesc(st, SkinScale(40));
		strcat(st, "DSEG14Classic-Italic.ttf");
		pf = new CFontDesc(st, SkinScale(32), kItalicFace);
		if (pf->getPlatformFontFromFile() == NULL)
		{
			pf->forget();
			// Try the font as a name instead of path (ie. uses installed fonts).
			pf = new CFontDesc("DSEG14Classic-Italic.ttf", SkinScale(32), kItalicFace);
			pf->getPlatformFont();
		}
		shape_text->setFont(pf); pf->forget(); free(st);
	}
	ValueChanged(kshape);

	//// Buttons
	bshape_mod = AddOnOffButton(194, 85, mcbits[black_butbit_tiny], 2, listener, COnOffButton::kPostListenerUpdate);
	blow_cpu = AddOnOffButton(135, 281, mcbits[black_butbit_tiny], 2, listener, COnOffButton::kPostListenerUpdate);

	lshape_mod = AddMovieBitmap(194, 71, mcbits[led_small_red], SNC_LED_RED_IMAGES, listener);
	llow_cpu = AddMovieBitmap(149, 281, mcbits[led_small_red], SNC_LED_RED_IMAGES, listener);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	PutRightScrews(screw3, screw4, listener);
	InitPatchPoints(0.0);

	std::memset(&osc, 0, sizeof(osc));
	osc.Init();
	std::memset(&jitter_source, 0, sizeof(jitter_source));
	jitter_source.Init();
	std::memset(&ws, 0, sizeof(ws));
	ws.Init(0x0000);
	std::memset(&settings, 0, sizeof(settings));

	// List of supported settings
	settings.meta_modulation = 0;
	settings.vco_drift = 0;
	settings.signature = 0; outputBuffer = NULL;
	SetSampleRate(sample_rate);
}

Braids::~Braids()
{	if (outputBuffer != NULL) delete outputBuffer;
}

// SoloRack calls this. It can't directly access rhe constructor since this is a Dll and pointers to constructor can not be easily achieved.
Braids* Braids::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Braids(pParent, listener, psynth_comm, vvoice);
}

void Braids::Initialize()
{
	panel = new CBitmap(dllskindir, NULL, "Braids.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Braids::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Braids::GetName()
{
	return name;
}

const int Braids::GetNameLen()
{
	return name_len;
}

const char* Braids::GetVendorName()
{
	return vendorname;
}

const int Braids::GetVendorNameLen()
{
	return vendorname_len;
}

const int Braids::GetType()
{
	return kOscillatorSource;
}

Product* Braids::Activate(char* fullname, char* email, char* serial)
{	// Replace this with your own implementation
	// This should return a pointer to the activated product. Or NULL if activation fails
	// In our Braids, NULL is retuned because the module is already active all the time, so no need to activate.
	return NULL;

	// Here is a simplistic example.
	//if (pproduct!=NULL) delete pproduct;
	//pproduct = new Product(0,1,NULL,false,"ShredsNCharades Baadalon");		// Product names has to include vendor name to ensure uniquenes
	//return pproduct->Activate(fullname,email,serial);
}

bool Braids::IsActive()
{	// This test module is Active. Replace this with your own check. 
	// It's better not to store this active/inactive status in an obvious place in memory, like a data member of the module or like that.
	// It's even better if the status is not stored at all, but rather a sophisticated test is done
	return true;

	// simple example
	//if (pproduct!=NULL) return pproduct->IsActive(); else return false;
}

Product* Braids::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Braids::InstanceIsActive()
{
	return this->IsActive();
}

const char* Braids::GetProductName()
{	// Change this to your own product name. 
	return "ShredsNCharades Chotiyon";
}

void Braids::ValueChanged(CControl* pControl)
{
	if (pControl == kshape)
	{
		//EnterProcessingCriticalSection();
		// Set shape
		shape = std::round(kshape->value * braids::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META);
		uint8_t sshape = CLIP(shape, 0, braids::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META);
		//LeaveProcessingCriticalSection();

		shape_text->setText(shape_infos[sshape].code);
		//this->invalid();
	}
	else if (pControl == bshape_mod)
	{
		settings.meta_modulation = bshape_mod->value >= 0.5f;
		lshape_mod->value = settings.meta_modulation;
	}
	else if (pControl == blow_cpu)
	{
		lowCpu = blow_cpu->value >= 0.5f;
		llow_cpu->value = lowCpu;
	}

}

void Braids::SetSampleRate(float sr)
{
	int i;

	Module::SetSampleRate(sr);

	src.setRates(96000, sample_rate);
#ifdef BRAIDS_SYNC
	sync_src.setRates(sample_rate, 96000);
#endif
	lowcpu_pitch_fix = log2(96000.f / sample_rate);

	i = (float)24 * sample_rate / 96000.f + 4;		// + 4 for safty
	if (i < 24) i = 24;
	if (outputBuffer != NULL) delete outputBuffer;
	outputBuffer = new dsp::DynDoubleRingBuffer<dsp::Frame<1>>(i);
}

//int Braids::GetPresetSize()
//{
//	return GetControlsValuesSize() + sizeof(polyphonyMode) + sizeof(resonatorModel);
//}

//void Braids::SavePreset(void* pdata, int size)
//{
//	char* pcdata = (char*)pdata;
//	SaveControlsValues(pcdata);
//	pcdata += GetControlsValuesSize();
//
//	// Save none control data
//	*(int*)pcdata = polyphonyMode; pcdata += sizeof(polyphonyMode);
//	*(rings::ResonatorModel*)pcdata = resonatorModel; pcdata += sizeof(resonatorModel);
//}

//void Braids::LoadPreset(void* pdata, int size, int version)
//{
//	char* pcdata = (char*)pdata;
//	int csize = GetControlsValuesSize();
//
//	LoadControlsValues(pcdata, csize);
//	pcdata += csize;
//
//	// Load non control data
//	polyphonyMode = *(int*) pcdata; pcdata += sizeof(polyphonyMode);
//	resonatorModel = *(rings::ResonatorModel*) pcdata; pcdata += sizeof(resonatorModel);
//}

const char* Braids::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/braids/";
}

inline void Braids::ProcessSample()
{
	// Trigger
	bool trig = pptrig->in >= HIGH_CV;
	if (!lastTrig && trig) osc.Strike();
	lastTrig = trig;

#ifdef BRAIDS_SYNC
	// Get sync input
	if (!sync_inbuffer.full()) {
		dsp::Frame<1> f;
		f.samples[0] = pptrig->in;
		sync_inbuffer.push(f);
	}
#endif

	// Render frames
	if (outputBuffer->empty())
	{
		// Sync doesn't work well for some reason!! Don't have time for it now.
#ifdef BRAIDS_SYNC
// Convert sync input buffer
		float sync_in[24] = {};
		if (!lowCpu)
		{
			int inLen = sync_inbuffer.size();
			int outLen = 24;
			sync_src.process(sync_inbuffer.startData(), &inLen, (dsp::Frame<1>*) sync_in, &outLen);
			sync_inbuffer.startIncr(inLen);
		}
		else
		{	// No sample rate convertion
			int blen = mmin(24, sync_inbuffer.size());
			for (int i = 0; i < blen; i++)
				sync_in[i] = sync_inbuffer.startData()[i].samples[0];
			sync_inbuffer.startIncr(blen);
		}

		int i;
		if (sync_in[0] >= HIGH_CV && sync_in[23] < HIGH_CV)
		{
			syncint[0] = 1; i = 2; syncint[1] = 0;
		}
		else { syncint[0] = 0; i = 1; }
		for (; i < 24; i++)
		{
			if (sync_in[i] >= HIGH_CV && sync_in[i - 1] < HIGH_CV)
			{
				syncint[i] = 1; i++; syncint[i] = 0;
			}
			else syncint[i] = 0;
		}
#endif	


		float fm = SCALE(kfm->value, -1.f, 1.f) * ppfm->in;

		// Set modulating shape
		if (settings.meta_modulation)
		{
			int tshape;
			tshape = shape + std::round(fm * braids::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META);
			settings.shape = CLIP(tshape, 0, braids::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META);
		}
		else settings.shape = shape;

		// Setup oscillator from settings
		osc.set_shape((braids::MacroOscillatorShape)settings.shape);

		// Set timbre/modulation
		float timbre = ktimbre->value + SCALE(kmodulation->value, -1.f, 1.f) * pptimbre->in;
		float modulation = kcolor->value + ppcolor->in;
		int16_t param1 = SCALE(CLIP(timbre, 0.0f, 1.0f), 0, INT16_MAX);
		int16_t param2 = SCALE(CLIP(modulation, 0.0f, 1.0f), 0, INT16_MAX);
		osc.set_parameters(param1, param2);

		// Set pitch
		float pitchV = pppitch->in * MIDI_TUNE_FACTOR + SCALE(kcoarse->value, -5.f, 3.f) + SCALE(kfine->value, -1.f / 12.f, 1.f / 12.f);
		if (!settings.meta_modulation)
			pitchV += fm;
		if (lowCpu)
			pitchV += lowcpu_pitch_fix;
		int32_t pitch = (pitchV * 12.0 + 60) * 128;
		//pitch += jitter_source.Render(settings.vco_drift);
		pitch = CLIP(pitch, 0, 16383);
		osc.set_pitch(pitch);

		// TODO: add a sync input buffer (must be sample rate converted)
		int16_t render_buffer[24];
		osc.Render(syncint, render_buffer, 24);

		// Signature waveshaping, decimation (not yet supported), and bit reduction (not yet supported)
		//uint16_t signature = settings.signature * settings.signature * 4095;
		const uint16_t signature = 0;
		for (size_t i = 0; i < 24; i++)
		{
			const int16_t bit_mask = 0xffff;
			int16_t sample = render_buffer[i] & bit_mask;
			int16_t warped = ws.Transform(sample);
			render_buffer[i] = stmlib::Mix(sample, warped, signature);
		}

		if (lowCpu)
		{
			for (int i = 0; i < 24; i++)
			{
				dsp::Frame<1> f;
				f.samples[0] = render_buffer[i] / 32768.0;
				outputBuffer->push(f);
			}
		}
		else {
			// Sample rate convert
			dsp::Frame<1> in[24];
			for (int i = 0; i < 24; i++) {
				in[i].samples[0] = render_buffer[i] / 32768.0;
			}

			int inLen = 24;
			int outLen = outputBuffer->capacity();
			src.process(in, &inLen, outputBuffer->endData(), &outLen);
			outputBuffer->endIncr(outLen);
		}
	}

	// Output
	//if (!outputBuffer->empty()) 
	{
		dsp::Frame<1> f = outputBuffer->shift();
		ppout->out = f.samples[0];
	}
}



//---------------------------------------------
// Tides
CBitmap* Tides::panel = NULL;
char* Tides::name = "Jvaar";
int Tides::name_len = 0;
char* Tides::vendorname = "ShredsNCharades";
int Tides::vendorname_len = 0;
Product* Tides::pproduct = NULL;
const float Tides::kRootScaled[3] =
{
	0.125f,
	2.0f,
	130.81f
};

static const tides2::Ratio kRatios[20] =
{
	{ 0.0625f, 16 },
	{ 0.125f, 8 },
	{ 0.1666666f, 6 },
	{ 0.25f, 4 },
	{ 0.3333333f, 3 },
	{ 0.5f, 2 },
	{ 0.6666666f, 3 },
	{ 0.75f, 4 },
	{ 0.8f, 5 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1.25f, 4 },
	{ 1.3333333f, 3 },
	{ 1.5f, 2 },
	{ 2.0f, 1 },
	{ 3.0f, 1 },
	{ 4.0f, 1 },
	{ 6.0f, 1 },
	{ 8.0f, 1 },
	{ 16.0f, 1 },
};

Tides::Tides(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{
	PatchPoint* temp[6];

	// Create The Knobs
	kfreq = AddModuleKnob(40, 85, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kfreq->setValue(0.5);

	kshape = AddModuleKnob(126.3, 85, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kshape->setValue(0.5);

	kslope = AddModuleKnob(29, 142, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kslope->setValue(0.5);

	ksmooth = AddModuleKnob(83, 131, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	ksmooth->setValue(0.5);

	kshift = AddModuleKnob(136, 142, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kshift->setValue(0.5);

	kslopecv = AddModuleKnob(24, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kslopecv->setValue(0.5);

	kfreqcv = AddModuleKnob(53.6, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kfreqcv->setValue(0.5);

	ksmoothcv = AddModuleKnob(83, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	ksmoothcv->setValue(0.5);

	kshapecv = AddModuleKnob(113, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kshapecv->setValue(0.5);

	kshiftcv = AddModuleKnob(142.6, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kshiftcv->setValue(0.5);

	// Create Patch Points
	ppslope = AddPatchPoint(19, 218, ppTypeInput, ppbit, 0, listener);
	ppfreq = AddPatchPoint(45, 218, ppTypeInput, ppbit, 0, listener);
	ppvoct = AddPatchPoint(71, 218, ppTypeInput, ppbit, 0, listener);
	ppsmooth = AddPatchPoint(97, 218, ppTypeInput, ppbit, 0, listener);
	ppshape = AddPatchPoint(123, 218, ppTypeInput, ppbit, 0, listener);
	ppshift = AddPatchPoint(148, 218, ppTypeInput, ppbit, 0, listener);

	pptrig = AddPatchPoint(19, 252, ppTypeInput, ppbit, 0, listener);
	ppclock = AddPatchPoint(45, 252, ppTypeInput, ppbit, 0, listener);
	ppout1 = AddPatchPoint(71, 252, ppTypeOutput, ppbit, 0, listener);
	ppout2 = AddPatchPoint(97, 252, ppTypeOutput, ppbit, 0, listener);
	ppout3 = AddPatchPoint(123, 252, ppTypeOutput, ppbit, 0, listener);
	ppout4 = AddPatchPoint(148, 252, ppTypeOutput, ppbit, 0, listener);

	// Buttons
	bfrange = AddKickButton(18, 36.6, mcbits[black_butbit_tiny], 2, listener);
	bmode = AddKickButton(150, 36.6, mcbits[black_butbit_tiny], 2, listener);
	bramp = AddKickButton(84, 89, mcbits[black_butbit_tiny], 2, listener);

	lfrange = AddMovieBitmap(34, 36.6, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	lmode = AddMovieBitmap(134, 36.6, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	lramp = AddMovieBitmap(84, 75, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);

	lout1 = AddMovieBitmap(63, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	lout2 = AddMovieBitmap(89, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	lout3 = AddMovieBitmap(115, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	lout4 = AddMovieBitmap(141, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	PutRightScrews(screw3, screw4, listener);
	InitPatchPoints(0.0);

	poly_slope_generator.Init();
	ratio_index_quantizer.Init();
	onReset();
	SetSampleRate(sample_rate);
}

void Tides::onReset()
{
	range = 1;
	output_mode = tides2::OUTPUT_MODE_GATES;
	ramp_mode = tides2::RAMP_MODE_LOOPING;

	UpdateRangeMode(); UpdateOutPutMode(); UpdateRampMode();
}

// SoloRack calls this. It can't directly access rhe constructor since this is a Dll and pointers to constructor can not be easily achieved.
Tides* Tides::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Tides(pParent, listener, psynth_comm, vvoice);
}

void Tides::Initialize()
{
	panel = new CBitmap(dllskindir, NULL, "Tides.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Tides::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Tides::GetName()
{
	return name;
}

const int Tides::GetNameLen()
{
	return name_len;
}

const char* Tides::GetVendorName()
{
	return vendorname;
}

const int Tides::GetVendorNameLen()
{
	return vendorname_len;
}

const int Tides::GetType()
{
	return kModulator;
}

Product* Tides::Activate(char* fullname, char* email, char* serial)
{
	return NULL;
}

bool Tides::IsActive()
{
	return true;
}

Product* Tides::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Tides::InstanceIsActive()
{
	return this->IsActive();
}

const char* Tides::GetProductName()
{
	return "ShredsNCharades Jvaar";
}

void Tides::UpdateRangeMode()
{
	range_mode = (range < 2) ? tides2::RANGE_CONTROL : tides2::RANGE_AUDIO;
	lfrange->value = (float)(range + 1) / 3.f;
	lfrange->invalid();
}
void Tides::UpdateOutPutMode()
{
	poly_slope_generator.Reset();
	lmode->value = (float)(output_mode + 1) / 4.f;
	lmode->invalid();
}
void Tides::UpdateRampMode()
{
	lramp->value = (float)(ramp_mode + 1) / 3.f;
	lramp->invalid();
}

void Tides::ValueChanged(CControl* pControl)
{
	// Switches
	if (pControl == bfrange && bfrange->value >= 0.5f)
	{
		EnterProcessingCriticalSection();
		range = (range + 1) % 3;
		UpdateRangeMode();
		LeaveProcessingCriticalSection();
	}
	else if (pControl == bmode && bmode->value >= 0.5f)
	{
		EnterProcessingCriticalSection();
		output_mode = (tides2::OutputMode)((output_mode + 1) % 4);
		UpdateOutPutMode();
		LeaveProcessingCriticalSection();
	}
	else if (pControl == bramp && bramp->value >= 0.5f)
	{
		EnterProcessingCriticalSection();
		ramp_mode = (tides2::RampMode)((ramp_mode + 1) % 3);
		UpdateRampMode();
		LeaveProcessingCriticalSection();
	}
}

void Tides::SetSampleRate(float sr)
{
	Module::SetSampleRate(sr);
	ramp_extractor.Init(sr, 40.f / sr);
}

int Tides::GetPresetSize()
{
	return GetControlsValuesSize() + sizeof(range) + sizeof(output_mode) + sizeof(ramp_mode);
}

void Tides::SavePreset(void* pdata, int size)
{
	char* pcdata = (char*)pdata;
	SaveControlsValues(pcdata);
	pcdata += GetControlsValuesSize();

	// Save none-control data
	*(int*)pcdata = range; pcdata += sizeof(range);
	*(tides2::OutputMode*)pcdata = output_mode; pcdata += sizeof(output_mode);
	*(tides2::RampMode*)pcdata = ramp_mode; pcdata += sizeof(ramp_mode);
}

void Tides::LoadPreset(void* pdata, int size, int version)
{
	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();

	LoadControlsValues(pcdata, csize);
	pcdata += csize;

	// Load none-control data
	range = *(int*)pcdata; pcdata += sizeof(range);
	output_mode = *(tides2::OutputMode*)pcdata; pcdata += sizeof(output_mode);
	ramp_mode = *(tides2::RampMode*)pcdata; pcdata += sizeof(ramp_mode);

	UpdateRangeMode(); UpdateOutPutMode(); UpdateRampMode();
}

const char* Tides::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/tides/";
}


inline void Tides::ProcessSample()
{
	float ft;

	// Input gates
	trig_flags[frame] = stmlib::ExtractGateFlags(previous_trig_flag, pptrig->in >= HIGH_CV);
	previous_trig_flag = trig_flags[frame];

	clock_flags[frame] = stmlib::ExtractGateFlags(previous_clock_flag, ppclock->in >= HIGH_CV);
	previous_clock_flag = clock_flags[frame];

	// Process block
	if (++frame >= tides2::kBlockSize)
	{
		frame = 0;

		ft = SCALE(kfreq->value, -48.f, 48.f) + 12.f * MIDI_TUNE_FACTOR * ppvoct->in;
		float note = CLIP(ft, -96.f, 96.f);
		ft = SCALE(kfreqcv->value, -1.f, 1.f) * ppfreq->in * 5.f * 12.f;
		float fm = CLIP(ft, -96.f, 96.f);
		float transposition = note + fm;

		float ramp[tides2::kBlockSize];
		float frequency;

		if (ppclock->num_cables)
		{
			if (must_reset_ramp_extractor) ramp_extractor.Reset();

			tides2::Ratio r = ratio_index_quantizer.Lookup(kRatios, 0.5f + transposition * 0.0105f, 20);
			frequency = ramp_extractor.Process(
				range_mode == tides2::RANGE_AUDIO,
				range_mode == tides2::RANGE_AUDIO && ramp_mode == tides2::RAMP_MODE_AR,
				r,
				clock_flags,
				ramp,
				tides2::kBlockSize);
			must_reset_ramp_extractor = false;
		}
		else
		{
			frequency = kRootScaled[range] / sample_rate * stmlib::SemitonesToRatio(transposition);
			must_reset_ramp_extractor = true;
		}

		// Get parameters
		ft = kslope->value + dsp::cubic(SCALE(kslopecv->value, -1.f, 1.f)) * ppslope->in;
		float slope = CLIP(ft, 0.f, 1.f);
		ft = kshape->value + dsp::cubic(SCALE(kshapecv->value, -1.f, 1.f)) * ppshape->in;
		float shape = CLIP(ft, 0.f, 1.f);
		ft = ksmooth->value + dsp::cubic(SCALE(ksmoothcv->value, -1.f, 1.f)) * ppsmooth->in;
		float smoothness = CLIP(ft, 0.f, 1.f);
		ft = kshift->value + dsp::cubic(SCALE(kshiftcv->value, -1.f, 1.f)) * ppshift->in;
		float shift = CLIP(ft, 0.f, 1.f);

		// Render generator
		poly_slope_generator.Render(
			ramp_mode,
			output_mode,
			range_mode,
			frequency,
			slope,
			shape,
			smoothness,
			shift,
			trig_flags,
			!(pptrig->num_cables) && ppclock->num_cables ? ramp : NULL,
			out,
			tides2::kBlockSize);
	}

	// Outputs
	ft = out[frame].channel[0]; ppout1->out = ft;
	lout1->value = ft;

	ft = out[frame].channel[1]; ppout2->out = ft;
	lout2->value = ft;

	ft = out[frame].channel[2]; ppout3->out = ft;
	lout3->value = ft;

	ft = out[frame].channel[3]; ppout4->out = ft;
	lout4->value = ft;
}




//---------------------------------------------
// Branches
CBitmap* Branches::panel = NULL;
char* Branches::name = "Shaakhaon";
int Branches::name_len = 0;
char* Branches::vendorname = "ShredsNCharades";
int Branches::vendorname_len = 0;
Product* Branches::pproduct = NULL;


Branches::Branches(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{

	// Create The Knobs
	kthresh[0] = AddModuleKnob(36, 69, mcbits[knob_small_red], SNC_SMALL_KNOB_IMAGES, false, listener);
	kthresh[0]->setValue(0.5);

	kthresh[1] = AddModuleKnob(36, 189, mcbits[knob_small_green], SNC_SMALL_KNOB_IMAGES, false, listener);
	kthresh[1]->setValue(0.5);

	// Create Patch Points
	ppin[0] = AddPatchPoint(17, 102, ppTypeInput, ppbit, 0, listener);
	pp[0] = AddPatchPoint(53, 102, ppTypeInput, ppbit, 0, listener);
	ppouta[0] = AddPatchPoint(17, 131, ppTypeOutput, ppbit, 0, listener);
	ppoutb[0] = AddPatchPoint(53, 131, ppTypeOutput, ppbit, 0, listener);

	ppin[1] = AddPatchPoint(17, 222, ppTypeInput, ppbit, 0, listener);
	pp[1] = AddPatchPoint(53, 222, ppTypeInput, ppbit, 0, listener);
	ppouta[1] = AddPatchPoint(17, 251, ppTypeOutput, ppbit, 0, listener);
	ppoutb[1] = AddPatchPoint(53, 251, ppTypeOutput, ppbit, 0, listener);

	// Buttons
	bmode[0] = AddOnOffButton(60, 50, mcbits[black_butbit_tiny], 2, listener, COnOffButton::kPostListenerUpdate);
	bmode[1] = AddOnOffButton(60, 170, mcbits[black_butbit_tiny], 2, listener, COnOffButton::kPostListenerUpdate);

	lout[0] = AddMovieBitmap(35, 131, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	lout[1] = AddMovieBitmap(35, 251, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	InitPatchPoints(0.0);

	ValueChanged(bmode[0]); ValueChanged(bmode[1]);
}

void Branches::onReset()
{
}

// SoloRack calls this. It can't directly access rhe constructor since this is a Dll and pointers to constructor can not be easily achieved.
Branches* Branches::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Branches(pParent, listener, psynth_comm, vvoice);
}

void Branches::Initialize()
{
	panel = new CBitmap(dllskindir, NULL, "Branches.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Branches::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Branches::GetName()
{
	return name;
}

const int Branches::GetNameLen()
{
	return name_len;
}

const char* Branches::GetVendorName()
{
	return vendorname;
}

const int Branches::GetVendorNameLen()
{
	return vendorname_len;
}

const int Branches::GetType()
{
	return kClockGate;
}

Product* Branches::Activate(char* fullname, char* email, char* serial)
{
	return NULL;
}

bool Branches::IsActive()
{
	return true;
}

Product* Branches::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Branches::InstanceIsActive()
{
	return this->IsActive();
}

const char* Branches::GetProductName()
{
	return "ShredsNCharades Shaakhaon";
}


void Branches::ValueChanged(CControl* pControl)
{
	if (pControl == bmode[0])
	{
		modes[0] = bmode[0]->value >= 0.5;
		UpdateOutputs(ppin[0]->in >= HIGH_CV, 0);
	}
	else if (pControl == bmode[1])
	{
		modes[1] = bmode[1]->value >= 0.5;
		UpdateOutputs(ppin[1]->in >= HIGH_CV, 1);
	}
}

//void Branches::SetSampleRate(float sr)
//{
//	Module::SetSampleRate(sr);
//	ramp_extractor.Init(sr, 40.f / sr);
//}

//int Branches::GetPresetSize()
//{
//	return GetControlsValuesSize();
//}

//void Branches::SavePreset(void* pdata, int size)
//{
//
//}

void Branches::LoadPreset(void* pdata, int size, int version)
{
	if (version < 1001) return;		// Fixes bug in version 1000 which doesn't save presets

	Module::LoadPreset(pdata, size, version);
}

const char* Branches::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/branches/";
}

inline void Branches::UpdateOutputs(bool gate, int i)
{	// Output gate logic
	bool gateA = !outcomes[i] && (modes[i] ? true : gate);
	bool gateB = outcomes[i] && (modes[i] ? true : gate);

	if (gateA)
	{
		ppouta[i]->out = 1.0; ppoutb[i]->out = 0.0; lout[i]->value = 1.0;
	}
	else if (gateB)
	{
		ppouta[i]->out = 0.0; ppoutb[i]->out = 1.0; lout[i]->value = 0.3333;
	}
	else
	{
		ppouta[i]->out = 0.0; ppoutb[i]->out = 0.0; lout[i]->value = 0.0;
	}
}

inline void Branches::ProcessSample()
{
	int i;

	for (i = 0; i < 2; i++)
	{
		bool gate = ppin[i]->in >= HIGH_CV;
		if (gate >= HIGH_CV && last_gate[i] < HIGH_CV)
		{
			// Trigger
			float threshold = kthresh[i]->value + 0.5 * pp[i]->in;
			bool toss = (PMRAND() < threshold);
			if (modes[i])
			{	// toggle modes
				if (toss) outcomes[i] ^= true;
			}
			else
			{	// direct modes
				outcomes[i] = toss;
			}

			UpdateOutputs(gate, i);
		}
		else if (gate < HIGH_CV && last_gate[i] >= HIGH_CV) UpdateOutputs(gate, i);

		last_gate[i] = gate;
	}
}



//---------------------------------------------
// Warps
CBitmap* Warps::panel = NULL;
char* Warps::name = "Vaarps";
int Warps::name_len = 0;
char* Warps::vendorname = "ShredsNCharades";
int Warps::vendorname_len = 0;
Product* Warps::pproduct = NULL;


Warps::Warps(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{

	// Create The Knobs
	kalgo = AddModuleKnob(59, 81, mcbits[knob_big_white], SNC_BIG_KNOB_IMAGES, false, listener);

	ktimbre = AddModuleKnob(90, 152, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	ktimbre->setValue(0.5f);

	klevel1 = AddModuleKnob(21, 174, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	klevel1->setValue(1.f);

	klevel2 = AddModuleKnob(50, 174, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	klevel2->setValue(1.f);

	// Create Patch Points
	pplevel1 = AddPatchPoint(18, 218, ppTypeInput, ppbit, 0, listener);
	pplevel2 = AddPatchPoint(45, 218, ppTypeInput, ppbit, 0, listener);
	ppalgo = AddPatchPoint(73, 218, ppTypeInput, ppbit, 0, listener);
	pptimbre = AddPatchPoint(100, 218, ppTypeInput, ppbit, 0, listener);
	ppcar = AddPatchPoint(18, 252, ppTypeInput, ppbit, 0, listener);
	ppmod = AddPatchPoint(45, 252, ppTypeInput, ppbit, 0, listener);
	ppmodout = AddPatchPoint(73, 252, ppTypeOutput, ppbit, 0, listener);
	ppaux = AddPatchPoint(100, 252, ppTypeOutput, ppbit, 0, listener);

	// Buttons
	bstate = AddKickButton(20, 146, mcbits[black_butbit_tiny], 2, listener);
	beaster = AddOnOffButton(108, 114, mcbits[black_butbit_tiny], 2, listener, COnOffButton::kPostListenerUpdate);

	// LEDs
	lstate = AddMovieBitmap(20, 130, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	leaster = AddMovieBitmap(108, 129, mcbits[led_small_red], SNC_LED_RED_IMAGES, listener);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	InitPatchPoints(0.0);

	//modulator.mutable_parameters()->carrier_shape = 0;
	memset(&modulator, 0, sizeof(modulator));
	modulator.Init(96000.f);
	SetSampleRate(sample_rate);
}

// SoloRack calls this. It can't directly access the constructor since this is a Dll and pointers to constructor can not be easily achieved.
Warps* Warps::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Warps(pParent, listener, psynth_comm, vvoice);
}

void Warps::Initialize()
{
	panel = new CBitmap(dllskindir, NULL, "Warps.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Warps::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Warps::GetName()
{
	return name;
}

const int Warps::GetNameLen()
{
	return name_len;
}

const char* Warps::GetVendorName()
{
	return vendorname;
}

const int Warps::GetVendorNameLen()
{
	return vendorname_len;
}

const int Warps::GetType()
{
	return kModifierEffect;
}

Product* Warps::Activate(char* fullname, char* email, char* serial)
{
	return NULL;
}

bool Warps::IsActive()
{
	return true;
}

Product* Warps::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Warps::InstanceIsActive()
{
	return this->IsActive();
}

const char* Warps::GetProductName()
{
	return "ShredsNCharades Warps";
}


void Warps::ValueChanged(CControl* pControl)
{
	warps::Parameters* p;

	if (pControl == bstate && bstate->value >= 0.5)
	{
		p = modulator.mutable_parameters();
		p->carrier_shape = (p->carrier_shape + 1) % 4;
		lstate->value = (float)(p->carrier_shape) / 3.f;
		lstate->invalid();
	}
	else if (pControl == beaster)
	{
		if (beaster->value >= 0.5)
		{
			leaster->value = 1.f;
			modulator.set_easter_egg(true);
		}
		else
		{
			leaster->value = 0.f;
			modulator.set_easter_egg(false);
		}
		leaster->invalid();
	}
	else if (pControl == kalgo)
	{
		p = modulator.mutable_parameters();
		p->modulation_algorithm = kalgo->value;		// Needed to change color when no cables connected (ie. processing is off)
		p->frequency_shift_pot = kalgo->value;
	}
}


inline void DrawGlow(CDrawContext* pContext, float const x, float const y, float const lsize, float const hue, float const light, float const alpha, float step = -1)
{
	Gdiplus::Graphics* gdi_graphics;
	NVGcolor algorithmColor;
	Color cc;
	float d;
	float i, j;
	float step_1 = 1.f / step;

	if (step < 0) step = Module::SkinScale(6);
	step_1 = 1.f / step;

	Gdiplus::SolidBrush gdi_bsh(Color::Transparent);
	gdi_graphics = pContext->getGraphics();

	//Gdiplus::Pen gdi_pen(Color::Black, 5);
	//gdi_graphics->DrawLine(&gdi_pen, x+2, y+2, x+50, y+50);

	d = lsize;
	while (d >= 5.f)
	{
		i = (lsize - d) * step_1;
		j = 5 + alpha * i;
		algorithmColor = nvgHSLA(hue, 1.0, 0.5 + light * i, CLIP(j, 0, 255));
		cc = Color(algorithmColor.a * 255.f, algorithmColor.r * 255.f, algorithmColor.g * 255.f, algorithmColor.b * 255.f);
		gdi_bsh.SetColor(cc);
		gdi_graphics->FillEllipse(&gdi_bsh, x - 0.5f * d, y - 0.5f * d, d, d);
		d -= step;
	}
}

void Warps::drawRect(CDrawContext* pContext, const CRect& updateRect)
{
	float x, y, ft;
	CDrawMode drmode = pContext->getDrawMode();

	Module::drawRect(pContext, updateRect);
	pContext->setDrawMode(kCopyMode);
	warps::Parameters* p = modulator.mutable_parameters();
	ft = p->modulation_algorithm + 0.5; if (ft > 1.0) ft -= 1.0;

	// x,y is centre position of kalgo knob
	// Conditonals for fixing minor problem in scalling. 
	if (uiscale < 1.1f)
	{
		x = 59.1; y = 75.8;
	}
	else if (uiscale < 1.6f)
	{
		x = 59; y = 74.8;
	}
	else
	{
		x = 59.1; y = 75;
	}
	DrawGlow(pContext, FSkinScale(x) + pContext->offset.x + size.left, FSkinScale(y) + pContext->offset.y + size.top, FSkinScale(85), ft, 0.04, 10);

	pContext->setDrawMode(drmode);
}

void Warps::SetSampleRate(float sr)
{
	Module::SetSampleRate(sr);
	noteinc = log2f(96000.0f / sample_rate) * 12.0f;
}

int Warps::GetPresetSize()
{
	return GetControlsValuesSize() + sizeof(warps::Parameters::carrier_shape);
}

void Warps::SavePreset(void* pdata, int size)
{
	char* pcdata = (char*)pdata;
	SaveControlsValues(pcdata);
	pcdata += GetControlsValuesSize();

	// Save none-control data
	*(int32_t*)pcdata = modulator.mutable_parameters()->carrier_shape; pcdata += sizeof(warps::Parameters::carrier_shape);
}

void Warps::LoadPreset(void* pdata, int size, int version)
{
	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();

	LoadControlsValues(pcdata, csize);
	pcdata += csize;

	// Load none-control data
	modulator.mutable_parameters()->carrier_shape = *(int32_t*)pcdata; pcdata += sizeof(warps::Parameters::carrier_shape);
}

const char* Warps::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/warps/";
}

inline void Warps::ProcessSample()
{
	int i;
	float ft;
	warps::Parameters* p;

	// Buffer loop
	if (++frame >= 60)
	{
		frame = 0;

		p = modulator.mutable_parameters();
		p->channel_drive[0] = CLIP(klevel1->value + pplevel1->in, 0.0f, 1.0f);
		p->channel_drive[1] = CLIP(klevel2->value + pplevel2->in, 0.0f, 1.0f);
		ft = p->modulation_algorithm;
		p->modulation_algorithm = CLIP(kalgo->value + ppalgo->in, 0.0f, 1.0f);

		// Update glow if cv changes algo
		if (ft != p->modulation_algorithm) kalgo->setDirty(true);

		p->modulation_parameter = CLIP(ktimbre->value + pptimbre->in, 0.0f, 1.0f);

		//p->frequency_shift_pot = kalgo->value;
		p->frequency_shift_cv = CLIP(ppalgo->in, -1.0f, 1.0f);
		p->phase_shift = p->modulation_algorithm;
		if (pplevel1->num_cables)
			p->note = 60.f * klevel1->value + 12.f * 5.f * pplevel1->in + 12.f;
		else p->note = 60.f * klevel1->value + 12.f * 2.f + 12.f;
		p->note += noteinc;

		modulator.Process(inputFrames, outputFrames, 60);
	}

	inputFrames[frame].l = CLIP((int)(ppcar->in * 5.f / 16.f * 0x8000), -0x8000, 0x7fff);
	inputFrames[frame].r = CLIP((int)(ppmod->in * 5.f / 16.f * 0x8000), -0x8000, 0x7fff);
	ppmodout->out = (float)outputFrames[frame].l / 0x8000;
	ppaux->out = (float)outputFrames[frame].r / 0x8000;

}



//---------------------------------------------
// Stages
CBitmap* Stages::panel = NULL;
char* Stages::name = "Charanon";
int Stages::name_len = 0;
char* Stages::vendorname = "ShredsNCharades";
int Stages::vendorname_len = 0;
Product* Stages::pproduct = NULL;


Stages::Stages(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{
	int i;

	// Create The Knobs
	//kalgo = AddModuleKnob(59, 81, mcbits[knob_big_white], SNC_BIG_KNOB_IMAGES, false, listener);

	//ktimbre = AddModuleKnob(90, 152, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	//ktimbre->setValue(0.5f);

	for (i = 0; i < NUM_CHANNELS; i++)
	{
		kshape[i] = AddModuleKnob(20 + 25.8f * i, 42, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
		kshape[i]->value = 0.5f;

		faderback[i] = AddMovieBitmap(19 + 25.9f * i, 128, mcbits[fader_back], 1, listener);
		klevel[i] = AddModuleKnob(19.1 + 25.9f * i, 128, mcbits[fader_off], SNC_FADER_IMAGES, false, listener);
		//fader[i]->setInverseBitmap(true);

		btype[i] = ModAddKickButtonEx(this, 19 + 25.9f * i, 79, mcbits[black_butbit_tiny], 2, listener);
		//btype[i] = AddOnOffButton(19 + 25.9f * i, 79, mcbits[black_butbit_tiny], 2, listener, COnOffButton::kPostListenerUpdate);
		ltypeg[i] = AddMovieBitmap(19 + 25.9f * i, 63, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
		ltypeg[i]->setValue(1.f);
		ltypey[i] = AddMovieBitmap(19 + 25.9f * i, 62, mcbits[led_small_yellow], SNC_LED_YELLOW_IMAGES, listener);
		ltypey[i]->setValue(1.f); ltypey[i]->setVisible(false);
		ltyper[i] = AddMovieBitmap(19 + 25.9f * i, 63, mcbits[led_small_red], SNC_LED_RED_IMAGES, listener);
		ltyper[i]->setValue(1.f); ltyper[i]->setVisible(false);

		pplevel[i] = AddPatchPoint(19 + 25.9f * i, 184, ppTypeInput, ppbit, 0, listener);
		ppgate[i] = AddPatchPoint(19 + 25.9f * i, 216, ppTypeInput, ppbit, 0, listener);
		ppout[i] = AddPatchPoint(19 + 25.9f * i, 252, ppTypeOutput, ppbit, 0, listener);

		lenv[i] = AddMovieBitmap(12 + 25.9f * i, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	}

	//klevel2 = AddModuleKnob(50, 174, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	//klevel2->setValue(1.f);

	//// Create Patch Points
	//pplevel1 = AddPatchPoint(18, 218, ppTypeInput, ppbit, 0, listener);
	//pplevel2 = AddPatchPoint(45, 218, ppTypeInput, ppbit, 0, listener);
	//ppalgo = AddPatchPoint(73, 218, ppTypeInput, ppbit, 0, listener);
	//pptimbre = AddPatchPoint(100, 218, ppTypeInput, ppbit, 0, listener);
	//ppcar = AddPatchPoint(18, 252, ppTypeInput, ppbit, 0, listener);
	//ppmod = AddPatchPoint(45, 252, ppTypeInput, ppbit, 0, listener);
	//ppmodout = AddPatchPoint(73, 252, ppTypeOutput, ppbit, 0, listener);
	//ppaux = AddPatchPoint(100, 252, ppTypeOutput, ppbit, 0, listener);

	//// Buttons
	//bstate = AddKickButton(20, 146, mcbits[black_butbit_tiny], 2, listener);
	//beaster = AddOnOffButton(108, 114, mcbits[black_butbit_tiny], 2, listener, COnOffButton::kPostListenerUpdate);

	//// LEDs
	//lstate = AddMovieBitmap(20, 130, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	//leaster = AddMovieBitmap(108, 129, mcbits[led_small_red], SNC_LED_RED_IMAGES, listener);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	PutRightScrews(screw3, screw4, listener);
	InitPatchPoints(0.0);

	// Trick to simulatee VCV input (gates) vector
	for (i = 0; i < NUM_CHANNELS; i++)
		all_inputs.push_back(pplevel[i]);
	for (i = 0; i < GATE_INPUTS; i++)
		all_inputs.push_back(ppgate[i]);

	onReset();
}

// SoloRack calls this. It can't directly access the constructor since this is a Dll and pointers to constructor can not be easily achieved.
Stages* Stages::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Stages(pParent, listener, psynth_comm, vvoice);
}

void Stages::Initialize()
{
	panel = new CBitmap(dllskindir, NULL, "Stages.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Stages::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Stages::GetName()
{
	return name;
}

const int Stages::GetNameLen()
{
	return name_len;
}

const char* Stages::GetVendorName()
{
	return vendorname;
}

const int Stages::GetVendorNameLen()
{
	return vendorname_len;
}

const int Stages::GetType()
{
	return kModulator;
}

Product* Stages::Activate(char* fullname, char* email, char* serial)
{
	return NULL;
}

bool Stages::IsActive()
{
	return true;
}

Product* Stages::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Stages::InstanceIsActive()
{
	return this->IsActive();
}

const char* Stages::GetProductName()
{
	return "ShredsNCharades Charanon";
}

//CMouseEventResult Stages::onMouseUp(CPoint& where, const long& buttons)
//{	// A trick to release the OnOffbutton. The reason I don't use the CKickButton is that I need the COnOffbutton::kPostListenerUpdate
//	// For the ValueChanged to be called imidiatly on the onMouseDown()
//
//	int i;
//	for (i = 0; i < NUM_CHANNELS; i++)
//		if (btype[i]->value >= 0.5f)
//		{	btype[i]->value = 0.0f;
//			UpdateButton(i); break;
//		}
//
//	return Module::onMouseUp(where, buttons);
//}

void Stages::UpdateButton(int i)
{
	if (btype[i]->value >= 0.5f)
		pressed_time = 0.f;
	else if (pressed_time >= 1.f)
	{	// Long press
		EnterProcessingCriticalSection();
		toggleLoop(i);
		LeaveProcessingCriticalSection();
		ltypeg[i]->invalid(); ltypey[i]->invalid();
		ltyper[i]->invalid();
	}
	else
	{	// Short press
		EnterProcessingCriticalSection();
		toggleMode(i);
		LeaveProcessingCriticalSection();
	}
}

void Stages::ValueChanged(CControl* pControl)
{
	int i;

	// Buttons
	for (i = 0; i < NUM_CHANNELS; i++)
	{
		if (pControl == btype[i])
		{
			UpdateButton(i); break;
		}
	}
}

//void Stages::drawRect(CDrawContext* pContext, const CRect& updateRect)
//{
//	//float x, y, ft;
//	//CDrawMode drmode = pContext->getDrawMode();
//
//	//Module::drawRect(pContext, updateRect);
//	//pContext->setDrawMode(kCopyMode);
//	//Stages::Parameters* p = modulator.mutable_parameters();
//	//ft = p->modulation_algorithm + 0.5; if (ft > 1.0) ft -= 1.0;
//
//	//// x,y is centre position of kalgo knob
//	//// Conditonals for fixing minor problem in scalling. 
//	//if (uiscale < 1.1f)
//	//{
//	//	x = 59.1; y = 75.8;
//	//}
//	//else if (uiscale < 1.6f)
//	//{
//	//	x = 59; y = 74.8;
//	//}
//	//else
//	//{
//	//	x = 59.1; y = 75;
//	//}
//	//DrawGlow(pContext, FSkinScale(x) + pContext->offset.x + size.left, FSkinScale(y) + pContext->offset.y + size.top, 85, ft, 0.04, 10);
//
//	//pContext->setDrawMode(drmode);
//}

void Stages::SetSampleRate(float sr)
{
	Module::SetSampleRate(sr);
	sample_time = 1.f / sr; phase_add = 2.f * sample_time;
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		segment_generator[i].SetSampleRate(sr);
		typeButtons[i].sample_rate = sr;
	}
}

int Stages::GetPresetSize()
{
	return GetControlsValuesSize() + NUM_CHANNELS * (sizeof(stages::segment::Type) + sizeof(bool));
}

void Stages::SavePreset(void* pdata, int size)
{
	int i;
	char* pcdata = (char*)pdata;
	SaveControlsValues(pcdata);
	pcdata += GetControlsValuesSize();

	// Save none-control data
	for (i = 0; i < NUM_CHANNELS; i++)
	{
		*(stages::segment::Type*)pcdata = configurations[i].type; pcdata += sizeof(stages::segment::Type);
		*(bool*)pcdata = configurations[i].loop; pcdata += sizeof(bool);
	}
}

void Stages::LoadPreset(void* pdata, int size, int version)
{
	int i;
	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();

	LoadControlsValues(pcdata, csize);
	pcdata += csize;

	// Load none-control data
	for (i = 0; i < NUM_CHANNELS; i++)
	{
		configurations[i].type = *(stages::segment::Type*)pcdata; pcdata += sizeof(stages::segment::Type);
		UpdateMode(i);
		configurations[i].loop = *(bool*)pcdata; pcdata += sizeof(bool);
		ResetLoopLeds(i); UpdateLoop(i);
	}

	// See if the group associations have changed since the last group
	groups_changed = groupBuilder.buildGroups(&all_inputs, GATE_INPUTS, NUM_CHANNELS);
}

const char* Stages::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/stages/";
}

void Stages::onReset()
{
	pressed_time = 0.f;
	for (size_t i = 0; i < NUM_CHANNELS; ++i) {
		segment_generator[i].Init();

		configurations[i].type = stages::segment::TYPE_RAMP;
		configurations[i].loop = false; ResetLoopLeds(i);
		configuration_changed[i] = true;
	}

	lightOscillatorPhase = 0.f;
	SetSampleRate(sample_rate);

	// See if the group associations have changed since the last group
	groups_changed = groupBuilder.buildGroups(&all_inputs, GATE_INPUTS, NUM_CHANNELS);
}

void Stages::drawRect(CDrawContext* pContext, const CRect& updateRect)
{	// Notes: 1. updateRect origin is at drawing context origin
	// 2. Using CDrawContext methods we only need to add x,y of parent
	// 3. Using GDI+, we also need to add pContext->offset

	float x, y, ft;
	int i, j, k;
	CRect r;
	CDrawMode drmode = pContext->getDrawMode();

	Module::drawRect(pContext, updateRect);
	pContext->setDrawMode(kCopyMode);

	k = -1;
	for (i = 0; i < groupBuilder.groupCount; i++)
	{
		GroupInfo& group = groupBuilder.groups[i];
		j = group.active_segment;
		if (j > k /*&& ppout[j]->out > 0.000001f*/)
		{
			k = j;
			r = klevel[j]->getViewSize(); r.offset(size.left, size.top);
			//CColor cc = CColor(); cc.blue = 150; cc.green = 150; cc.red = 255; cc.alpha = 160;
			//pContext->setFillColor(cc); //pContext->drawRect(updateRect);
			if (r.rectOverlap(updateRect))
			{
				r.offset(pContext->offset.x, pContext->offset.y);
				DrawGlow(pContext, (r.x2 + r.x) * 0.5f, (r.y + r.y2) * 0.5f - 0.4 * r.height() * (klevel[j]->value - 0.5f), FSkinScale(25), 0.35, 0.12, 20, FSkinScale(4));
			}
		}
	}
	pContext->setDrawMode(drmode);
}

void Stages::stepBlock()
{
	// Get parameters
	float primaries[NUM_CHANNELS];
	float secondaries[NUM_CHANNELS];
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		primaries[i] = CLIP(klevel[i]->value + pplevel[i]->in, 0.f, 1.f);
		secondaries[i] = kshape[i]->value;
	}

	// This is done else where only when there is actual possibility of a change (ammar)
	//bool groups_changed = groupBuilder.buildGroups(&all_inputs, GATE_INPUTS, NUM_CHANNELS);

	// Process block
	stages::SegmentGenerator::Output out[BLOCK_SIZE] = {};
	for (int i = 0; i < groupBuilder.groupCount; i++) {
		GroupInfo& group = groupBuilder.groups[i];

		// Check if the config needs applying to the segment generator for this group
		bool apply_config = groups_changed;
		int numberOfLoopsInGroup = 0;
		for (int j = 0; j < group.segment_count; j++) {
			int segment = group.first_segment + j;
			numberOfLoopsInGroup += configurations[segment].loop ? 1 : 0;
			apply_config |= configuration_changed[segment];
			configuration_changed[segment] = false;
		}

		if (numberOfLoopsInGroup > 2) {
			// Too many segments are looping, turn them all off
			apply_config = true;
			for (int j = 0; j < group.segment_count; j++)
			{
				configurations[group.first_segment + j].loop = false;
				ResetLoopLeds(group.first_segment + j);
			}
		}

		if (apply_config) {
			segment_generator[i].Configure(group.gated, &configurations[group.first_segment], group.segment_count);
		}

		// Set the segment parameters on the generator we're about to process
		for (int j = 0; j < group.segment_count; j++) {
			segment_generator[i].set_segment_parameters(j, primaries[group.first_segment + j], secondaries[group.first_segment + j]);
		}

		segment_generator[i].Process(gate_flags[group.first_segment], out, BLOCK_SIZE);

		for (int j = 0; j < BLOCK_SIZE; j++)
		{
			for (int k = 0; k < group.segment_count; k++)
			{
				int segment = group.first_segment + k;
				if (k == out[j].segment)
				{	// Set the phase output for the active segment
					if (k > 0) envelopeBuffer[segment][j] = 1.f - out[j].phase;
					if (group.active_segment != segment)
					{
						group.active_segment = segment;
						klevel[segment]->setDirty(true);
					}
				}
				else
				{	// Non active segments have 0.f output
					if (k > 0) envelopeBuffer[segment][j] = 0.f;
					if (group.active_segment == segment)
					{
						group.active_segment = -1;
						klevel[segment]->setDirty(true);
					}
				}
			}
			// First group segment gets the actual output
			envelopeBuffer[group.first_segment][j] = out[j].value;
		}
	}
	groups_changed = false;
}

void Stages::UpdateMode(int i)
{
	switch (configurations[i].type)
	{
	case stages::segment::Type::TYPE_RAMP:
		ltypeg[i]->setVisible(true);
		ltypey[i]->setVisible(false);
		ltyper[i]->setVisible(false);
		break;

	case stages::segment::Type::TYPE_STEP:
		ltypeg[i]->setVisible(false);
		ltypey[i]->setVisible(true);
		ltyper[i]->setVisible(false);
		break;

	case stages::segment::Type::TYPE_HOLD:
		ltypeg[i]->setVisible(false);
		ltypey[i]->setVisible(false);
		ltyper[i]->setVisible(true);
		break;
	}
}

void Stages::toggleMode(int i)
{
	configurations[i].type = (stages::segment::Type)((configurations[i].type + 1) % 3);
	configuration_changed[i] = true;
	UpdateMode(i);
}

void Stages::UpdateLoop(int segment)
{
	// ensure that we don't have too many looping segments in the group
	if (configurations[segment].loop) {
		int segment_count = 0;
		for (int i = 0; i < groupBuilder.groupCount; i++) {
			segment_count += groupBuilder.groups[i].segment_count;

			if (segment_count > segment) {
				GroupInfo& group = groupBuilder.groups[i];

				// See how many loop items we have
				int numberOfLoopsInGroup = 0;

				for (int j = 0; j < group.segment_count; j++) {
					numberOfLoopsInGroup += configurations[group.first_segment + j].loop ? 1 : 0;
				}

				// If we've got too many loop items, clear down to the one looping segment
				if (numberOfLoopsInGroup > 2) {
					for (int j = 0; j < group.segment_count; j++) {
						configurations[group.first_segment + j].loop = (group.first_segment + j) == segment;
						ResetLoopLeds(group.first_segment + j);
					}
				}

				break;
			}
		}
	}
}

inline void Stages::ResetLoopLeds(int segment)
{
	if (!(configurations[segment].loop))
	{
		ltypeg[segment]->value = 1.f; ltypey[segment]->value = 1.f;
		ltyper[segment]->value = 1.f;
	}
}

void Stages::toggleLoop(int segment)
{
	configuration_changed[segment] = true;
	configurations[segment].loop = !configurations[segment].loop;
	ResetLoopLeds(segment);
	UpdateLoop(segment);
}

void Stages::CableConnected(PatchPoint* pp)
{
	// See if the group associations have changed since the last group
	groups_changed = groupBuilder.buildGroups(&all_inputs, GATE_INPUTS, NUM_CHANNELS);
}

void Stages::CableDisconnected(PatchPoint* pp)
{
	// See if the group associations have changed since the last group
	groups_changed = groupBuilder.buildGroups(&all_inputs, GATE_INPUTS, NUM_CHANNELS);
}

inline void Stages::ProcessSample()
{
	float ft;

	// Oscillate flashing the type lights
	lightOscillatorPhase += phase_add;

	if (lightOscillatorPhase > 1.0f)
	{
		phase_add = -2.f * sample_time;
		lightOscillatorPhase += phase_add;
	}
	else if (lightOscillatorPhase < 0.0f)
	{
		phase_add = 2.f * sample_time;
		lightOscillatorPhase += phase_add;
	}

	//// Buttons (Done in ValueChanged())
	//for (int i = 0; i < NUM_CHANNELS; i++) 
	//{	switch (typeButtons[i].step(btype[i])) 
	//	{	default:
	//		case LongPressButton::NO_PRESS: break;
	//		case LongPressButton::SHORT_PRESS: toggleMode(i); break;
	//		case LongPressButton::LONG_PRESS: toggleLoop(i); break;
	//	}
	//}
	pressed_time += sample_time;

	// Input
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		bool gate = (ppgate[i]->in >= HIGH_CV);
		last_gate_flags[i] = stmlib::ExtractGateFlags(last_gate_flags[i], gate);
		gate_flags[i][blockIndex] = last_gate_flags[i];
	}

	// Process block
	if (++blockIndex >= BLOCK_SIZE)
	{
		blockIndex = 0;
		stepBlock();
	}

	// Output
	for (int i = 0; i < groupBuilder.groupCount; i++)
	{
		GroupInfo& group = groupBuilder.groups[i];

		int numberOfLoopsInGroup = 0;
		for (int j = 0; j < group.segment_count; j++)
		{
			int segment = group.first_segment + j;

			float envelope = envelopeBuffer[segment][blockIndex];
			ppout[segment]->out = envelope /** 8.f*/;
			lenv[segment]->value = envelope;
			if (lenv[segment]->isDirty()) klevel[segment]->setDirty(true);

			if (configurations[segment].loop)
			{
				float flashlevel = 1.f;
				numberOfLoopsInGroup++;
				if (numberOfLoopsInGroup == 1)
				{
					flashlevel = lightOscillatorPhase;
				}
				else if (numberOfLoopsInGroup > 1)
				{	//float advancedPhase = lightOscillatorPhase + 0.25f;
					//if (advancedPhase > 1.0f)
					//	advancedPhase -= 1.0f;

					flashlevel = 1.f - lightOscillatorPhase;
				}

				switch (configurations[segment].type)
				{
				case stages::segment::Type::TYPE_RAMP:
					ltypeg[segment]->value = flashlevel;
					break;

				case stages::segment::Type::TYPE_STEP:
					ltypey[segment]->value = flashlevel;
					break;

				case stages::segment::Type::TYPE_HOLD:
					ltyper[segment]->value = flashlevel;
					break;
				}
			}
		}
	}
}



//---------------------------------------------
// Elements
CBitmap* Elements::panel = NULL;
char* Elements::name = "Tatvon";
int Elements::name_len = 0;
char* Elements::vendorname = "ShredsNCharades";
int Elements::vendorname_len = 0;
Product* Elements::pproduct = NULL;

Elements::Elements(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{

	// Create The Knobs
	kcontour = AddModuleKnob(33.6, 52, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kcontour->setValue(1.0); 

	kbow = AddModuleKnob(87.4, 52, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kbow->setValue(0.0); 

	kblow = AddModuleKnob(142, 52, mcbits[knob_small_red], SNC_SMALL_KNOB_IMAGES, false, listener);
	kblow->setValue(0.0);

	kstrike = AddModuleKnob(196, 52, mcbits[knob_small_green], SNC_SMALL_KNOB_IMAGES, false, listener);
	kstrike->setValue(0.5);

	kcoarse = AddModuleKnob(248.8, 52, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kcoarse->setValue(0.5); 

	kfine = AddModuleKnob(303.6, 52, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kfine->setValue(0.5); 

	kfm = AddModuleKnob(357, 52, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kfm->setValue(0.5);

	kflow = AddModuleKnob(105, 118, mcbits[knob_medium_red], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kflow->setValue(0.5); kflow->svalue = 0.5;

	kmallet = AddModuleKnob(180, 118, mcbits[knob_medium_green], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kmallet->setValue(0.5); kmallet->svalue = 0.5;

	kgeom = AddModuleKnob(266.5, 118, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kgeom->setValue(0.5); kgeom->svalue = 0.5;

	kbright = AddModuleKnob(341.6, 118, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kbright->setValue(0.5); kbright->svalue = 0.5;

	kbow_timbre = AddModuleKnob(87.4, 176, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kbow_timbre->setValue(0.5); kbow_timbre->svalue = 0.5;

	kblow_timbre = AddModuleKnob(142, 176, mcbits[knob_small_red], SNC_SMALL_KNOB_IMAGES, false, listener);
	kblow_timbre->setValue(0.5); kblow_timbre->svalue = 0.5;


	kstrike_timbre = AddModuleKnob(196, 176, mcbits[knob_small_green], SNC_SMALL_KNOB_IMAGES, false, listener);
	kstrike_timbre->setValue(0.5); kstrike_timbre->svalue = 0.5;

	kdamp = AddModuleKnob(248.8, 176, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kdamp->setValue(0.5); kdamp->svalue = 0.5;

	kpos = AddModuleKnob(303.6, 176, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kpos->setValue(0.5); kpos->svalue = 0.5;

	kspace = AddModuleKnob(357, 176, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kspace->setValue(0.0); 

	kbow_timbrecv = AddModuleKnob(83, 221, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kbow_timbrecv->setValue(0.5);

	kflowcv = AddModuleKnob(113, 221, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kflowcv->setValue(0.5);

	kblow_timbrecv = AddModuleKnob(142.5, 221, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kblow_timbrecv->setValue(0.5);

	kmalletcv = AddModuleKnob(172, 221, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kmalletcv->setValue(0.5);

	kstrike_timbrecv = AddModuleKnob(201.7, 221, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kstrike_timbrecv->setValue(0.5);

	kdampcv = AddModuleKnob(245.2, 221, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kdampcv->setValue(0.5);

	kgeomcv = AddModuleKnob(275.2, 221, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kgeomcv->setValue(0.5);

	kposcv = AddModuleKnob(304.2, 221, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kposcv->setValue(0.5);

	kbrightcv = AddModuleKnob(334, 221, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kbrightcv->setValue(0.5);

	kspacecv = AddModuleKnob(363.3, 221, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kspacecv->setValue(0.5);

	// Create Patch Points
	pppitch = AddPatchPoint(19.7, 144, ppTypeInput, ppbit, 0, listener);
	ppfm = AddPatchPoint(47.6, 144, ppTypeInput, ppbit, 0, listener);
	ppgate = AddPatchPoint(19.7, 179.6, ppTypeInput, ppbit, 0, listener);
	ppstrength = AddPatchPoint(47.6, 179.6, ppTypeInput, ppbit, 0, listener);

	ppextblow = AddPatchPoint(19.7, 215.2, ppTypeInput, ppbit, 0, listener);
	ppextstrike = AddPatchPoint(47.6, 215.2, ppTypeInput, ppbit, 0, listener);

	ppoutl = AddPatchPoint(19.7, 250.8, ppTypeOutput, ppbit, 0, listener);
	ppoutr = AddPatchPoint(47.6, 250.8, ppTypeOutput, ppbit, 0, listener);

	ppbow_timbrecv = AddPatchPoint(82.5, 250.8, ppTypeInput, ppbit, 0, listener);
	ppflowcv = AddPatchPoint(112, 250.8, ppTypeInput, ppbit, 0, listener);
	ppblow_timbrecv = AddPatchPoint(141.5, 250.8, ppTypeInput, ppbit, 0, listener);
	ppmalletcv = AddPatchPoint(171, 250.8, ppTypeInput, ppbit, 0, listener);
	ppstrike_timbrecv = AddPatchPoint(200.5, 250.8, ppTypeInput, ppbit, 0, listener);
	ppdampcv = AddPatchPoint(244.5, 250.8, ppTypeInput, ppbit, 0, listener);
	ppgeomcv = AddPatchPoint(274, 250.8, ppTypeInput, ppbit, 0, listener);
	ppposcv = AddPatchPoint(303, 250.8, ppTypeInput, ppbit, 0, listener);
	ppbrightcv = AddPatchPoint(332.5, 250.8, ppTypeInput, ppbit, 0, listener);
	ppspacecv = AddPatchPoint(362, 250.8, ppTypeInput, ppbit, 0, listener);

	bplay = AddKickButton(34.5, 99, mcbits[button_led], 2, listener);
	bmode = AddKickButton(326, 280, mcbits[black_butbit_tiny], 2, listener);
	//blow_cpu = AddOnOffButton(309, 281, mcbits[black_butbit_tiny], 2, listener, COnOffButton::kPostListenerUpdate);

	// Leds
	lexciter = AddMovieBitmap(142, 133, mcbits[led_big_green], SNC_LED_GREEN_IMAGES, listener);
	lresonator = AddMovieBitmap(304.5, 133, mcbits[led_big_red], SNC_LED_RED_IMAGES, listener);
	lplay = AddMovieBitmap(34.5, 99, mcbits[button_led], 2, listener); 
	lplay->value = 1.f; lplay->setVisible(false);
	lmode = AddMovieBitmap(340, 280, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	//llow_cpu = AddMovieBitmap(323, 281, mcbits[led_small_red], SNC_LED_RED_IMAGES, listener);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	PutRightScrews(screw3, screw4, listener);
	InitPatchPoints(0.0);

	//low_cpu = false; 
	outputBuffer = NULL; inputBuffer = NULL;
	epart = NULL; OnReset(); SetSampleRate(sample_rate);
}

Elements::~Elements()
{
	if (epart != NULL) delete epart;
	if (outputBuffer != NULL) delete outputBuffer;
	if (inputBuffer != NULL) delete inputBuffer;
}

void Elements::OnReset()
{	
	if (epart != NULL) { delete epart; epart = NULL; }
	
	epart = new elements::Part();
	// In the Mutable Instruments code, Part doesn't initialize itself, so zero it here.
	std::memset(epart, 0, sizeof(*epart));
	//if (low_cpu)
	//	epart->Init(reverb_buffer, sample_rate);
	//else
		epart->Init(reverb_buffer, 32000.0f);
	// Just some random numbers
	uint32_t seed[3] = { 1, 2, 3 };
	epart->Seed(seed, 3);
	SetModel(0);
}

void Elements::SetModel(int model)
{
	mmode = model;
	if (mmode == 4)
	{	epart->set_easter_egg(true);
	}
	else 
	{	epart->set_easter_egg(false);
		epart->set_resonator_model((elements::ResonatorModel)mmode);
	}

	// LED
	lmode->value = ((float)mmode + 1.f) / 5.f;
}

// SoloRack calls this. It can't directly access rhe constructor since this is a Dll and pointers to constructor can not be easily achieved.
Elements* Elements::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Elements(pParent, listener, psynth_comm, vvoice);
}

void Elements::Initialize()
{
	char* stemp;

	panel = new CBitmap(dllskindir, NULL, "Elements.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Elements::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Elements::GetName()
{
	return name;
}

const int Elements::GetNameLen()
{
	return name_len;
}

const char* Elements::GetVendorName()
{
	return vendorname;
}

const int Elements::GetVendorNameLen()
{
	return vendorname_len;
}

const int Elements::GetType()
{
	return kOscillatorSource;
}

Product* Elements::Activate(char* fullname, char* email, char* serial)
{	// Replace this with your own implementation
	// This should return a pointer to the activated product. Or NULL if activation fails
	// In our Elements, NULL is retuned because the module is already active all the time, so no need to activate.
	return NULL;

	// Here is a simplistic example.
	//if (pproduct!=NULL) delete pproduct;
	//pproduct = new Product(0,1,NULL,false,"ShredsNCharades Baadalon");		// Product names has to include vendor name to ensure uniquenes
	//return pproduct->Activate(fullname,email,serial);
}

bool Elements::IsActive()
{	// This test module is Active. Replace this with your own check. 
	// It's better not to store this active/inactive status in an obvious place in memory, like a data member of the module or like that.
	// It's even better if the status is not stored at all, but rather a sophisticated test is done
	return true;

	// simple example
	//if (pproduct!=NULL) return pproduct->IsActive(); else return false;
}

Product* Elements::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Elements::InstanceIsActive()
{
	return this->IsActive();
}

const char* Elements::GetProductName()
{	// Change this to your own product name. 
	return "ShredsNCharades Tatvon";
}

//void Elements::UpdateBlendKnob()
//{
//	kblend->setVisible(blendMode == 0);
//	kspread->setVisible(blendMode == 1);
//	kfeedback->setVisible(blendMode == 2);
//	kreverb->setVisible(blendMode == 3);
//
//	lmix->value = (blendMode == 0);
//	lpan->value = (blendMode == 1);
//	lfb->value = (blendMode == 2);
//	lreverb->value = (blendMode == 3);
//	leds_wait = CLOUDS_LEDS_WAIT * sample_rate / 32.f;
//}

//inline void Elements::UpdateProcessor()
//{
//	// Set up processor
//	processor->set_playback_mode(playback);
//	if (quality == 4)
//		processor->set_quality(0);
//	else
//		processor->set_quality(quality);
//	processor->Prepare();
//}

void Elements::ValueChanged(CControl* pControl)
{
	if (pControl == bmode && bmode->value >= 0.5)
	{	EnterProcessingCriticalSection();
		SetModel((mmode + 1) % 5);
		LeaveProcessingCriticalSection();
	}

}

void Elements::SetSampleRate(float sr)
{
	int i,j;

	float prev_sr = sample_rate;
	
	Module::SetSampleRate(sr);
	inputSrc.setRates(sr, 32000); inputSrc.setChannels(2);
	outputSrc.setRates(32000, sr); outputSrc.setChannels(2);

	i = (float)16 * sample_rate / 32000.f + 4;		// + 4 for safty
	if (i < 16) i = 16;
	if (inputBuffer != NULL) delete inputBuffer;
	inputBuffer = new dsp::DynDoubleRingBuffer<dsp::Frame<2>>(i);
	if (outputBuffer != NULL) delete outputBuffer;
	outputBuffer = new dsp::DynDoubleRingBuffer<dsp::Frame<2>>(i);

	// Push intial empty data to input buffer
	dsp::Frame<2> f; f.samples[0] = 0.f; f.samples[1] = 0.f;
	for (j = 0; j < i - 1; j++) inputBuffer->push(f);		// i-1 (not i) because we can allow first sample in
}

int Elements::GetPresetSize()
{
	return GetControlsValuesSize() + sizeof(mmode);
}

void Elements::SavePreset(void* pdata, int size)
{
	char* pcdata = (char*)pdata;
	SaveControlsValues(pcdata);
	pcdata += GetControlsValuesSize();

	// Save none control data
	*(int*)pcdata = mmode; pcdata += sizeof(mmode);
}

void Elements::LoadPreset(void* pdata, int size, int version)
{
	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();

	LoadControlsValues(pcdata, csize);
	pcdata += csize;

	// Load non control data
	mmode = *(int*)pcdata; pcdata += sizeof(mmode);
	SetModel(mmode);
}

const char* Elements::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/elements/";
}

inline void Elements::ProcessSample()
{	
	bool fl;
	float ft;

	// Get input
	if (!inputBuffer->full()) 
	{	inputFrame.samples[0] = ppextblow->in;
		inputFrame.samples[1] = ppextstrike->in;
		inputBuffer->push(inputFrame);
	}

	// Generate output if output buffer is empty
	if (outputBuffer->empty()) 
	{	// blow[channel][bufferIndex]
		float blow[16]; // = {};
		float strike[16]; // = {};

		{	// Convert input buffer
			int inLen = inputBuffer->size();
			int outLen = 16;
			dsp::Frame<2> inputFrames[16];
			inputSrc.process(inputBuffer->startData(), &inLen, inputFrames, &outLen);
			inputBuffer->startIncr(inLen);

			for (int i = 0; i < outLen; i++) 
			{	blow[i] = inputFrames[i].samples[0];
				strike[i] = inputFrames[i].samples[1];
			}
		}

		// Process channels
		// main[channel][bufferIndex]
		float main[16];
		float aux[16];
		//float gateLight = 0.f;

		// Set patch from parameters
		elements::Patch* p = epart->mutable_patch();
		p->exciter_envelope_shape = kcontour->value;
		p->exciter_bow_level = kbow->value;
		p->exciter_blow_level = kblow->value;
		p->exciter_strike_level = kstrike->value;

		#define BIND(_p, _m, _i) CLIP(_p->value + 3.3f * dsp::quadraticBipolar(SCALE(_m->value,-1.f,1.f)) * _i->in, 0.f, 0.9995f)

		p->exciter_bow_timbre = BIND(kbow_timbre, kbow_timbrecv, ppbow_timbrecv);
		p->exciter_blow_meta = BIND(kflow, kflowcv, ppflowcv);
		p->exciter_blow_timbre = BIND(kblow_timbre, kblow_timbrecv, ppblow_timbrecv);
		p->exciter_strike_meta = BIND(kmallet, kmalletcv, ppmalletcv);
		p->exciter_strike_timbre = BIND(kstrike, kstrike_timbre, ppstrike_timbrecv);
		p->resonator_geometry = BIND(kgeom, kgeomcv, ppgeomcv);
		p->resonator_brightness = BIND(kbright, kbrightcv, ppbrightcv);
		p->resonator_damping = BIND(kdamp, kdampcv, ppdampcv);
		p->resonator_position = BIND(kpos, kposcv, ppposcv);
		p->space = CLIP(2.f*kspace->value + SCALE(kspacecv->value,-2.f,2.f) * ppspacecv->in, 0.f, 2.f);

		// Get performance inputs
		elements::PerformanceState performance;
		performance.note = 12.f * (MIDI_TUNE_FACTOR * pppitch->in /*+ sr_notefix*/) + std::round(SCALE(kcoarse->value,-30.f,30.f)) + SCALE(kfine->value,-2.f,2.f) + 69.f;
		performance.modulation = 3.3f * dsp::quarticBipolar(SCALE(kfm->value,-1.f,1.f)) * 49.5f * ppfm->in;
		
		// Play light logic.
		fl = ppgate->in >= HIGH_CV;
		performance.gate = bplay->value >= 0.5f || fl;
		lplay->setVisible(fl); if (!fl) bplay->setDirty(true);
		
		ft = 1.f - ppstrength->in;
		performance.strength = CLIP(ft, 0.f, 1.f);

		// Generate audio
		epart->Process(performance, blow, strike, main, aux, 16);

		{	// Convert output buffer
			dsp::Frame<2> outputFrames[16];
			for (int i = 0; i < 16; i++) 
			{	outputFrames[i].samples[0] = main[i];
				outputFrames[i].samples[1] = aux[i];
			}

			int inLen = 16;
			int outLen = outputBuffer->capacity();
			outputSrc.process(outputFrames, &inLen, outputBuffer->endData(), &outLen);
			outputBuffer->endIncr(outLen);
		}

		// Set lights
		//gateLight = performance.gate ? 0.75f : 0.f;
		lexciter->value = epart->exciter_level();
		lresonator->value = epart->resonator_level();
	}

	// Set output
	outputFrame = outputBuffer->shift();
	ppoutl->out = outputFrame.samples[0];
	ppoutr->out = outputFrame.samples[1];
}


//---------------------------------------------
// Marbles
CBitmap* Marbles::panel = NULL;
char* Marbles::name = "Patthar";
int Marbles::name_len = 0;
char* Marbles::vendorname = "ShredsNCharades";
int Marbles::vendorname_len = 0;
Product* Marbles::pproduct = NULL;

Marbles::Marbles(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{
	CRect r;

	// Create The Knobs
	kdeja = AddModuleKnob(101, 55, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kdeja->setValue(0.5f);

	ktrate = AddModuleKnob(51.5, 88.3, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	ktrate->setValue(0.5f);

	kxspread = AddModuleKnob(152.5, 88.3, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kxspread->setValue(0.5f);

	klength = AddModuleKnob(101, 119.3, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	klength->setValue(0.0f);

	ktbias = AddModuleKnob(18.5, 138, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	ktbias->setValue(0.5f);

	kxbias = AddModuleKnob(183.2, 138, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kxbias->setValue(0.5f);

	ktjitter = AddModuleKnob(69, 173, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	ktjitter->setValue(0.0f);

	kxsteps = AddModuleKnob(133, 173, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kxsteps->setValue(0.5f);

	// Buttons
	btdeja = AddOnOffButton(33, 40, mcbits[button_led], 2, listener, COnOffButton::kPostListenerUpdate);
	bxdeja = AddOnOffButton(168.3, 40, mcbits[button_led], 2, listener, COnOffButton::kPostListenerUpdate);
	btmode = AddKickButton(14, 88, mcbits[black_butbit_tiny], 2, listener);
	bxmode = AddKickButton(189, 88, mcbits[black_butbit_tiny], 2, listener);
	btrange = AddKickButton(58, 135, mcbits[black_butbit_tiny], 2, listener);
	bxrange = AddKickButton(145.5, 135, mcbits[black_butbit_tiny], 2, listener);
	bextern = AddKickButton(102, 153, mcbits[black_butbit_tiny], 2, listener);

	// LEDs
	ltmode = AddMovieBitmap(14, 69, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	lxmode = AddMovieBitmap(190, 69, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	ltrange = AddMovieBitmap(58.1, 121.5, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	lxrange = AddMovieBitmap(145.3, 121.5, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	lextern = AddMovieBitmap(102, 171, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);

	lt1 = AddMovieBitmap(12, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	lt2 = AddMovieBitmap(40, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	lt3 = AddMovieBitmap(67, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	ly = AddMovieBitmap(95.5, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	lx1 = AddMovieBitmap(122, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	lx2 = AddMovieBitmap(149, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	lx3 = AddMovieBitmap(177, 238, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);

	// Create Patch Points
	pptbias = AddPatchPoint(18.5, 182, ppTypeInput, ppbit, 0, listener);
	ppxbias = AddPatchPoint(183, 182, ppTypeInput, ppbit, 0, listener);

	pptclock = AddPatchPoint(18.5, 218, ppTypeInput, ppbit, 0, listener);
	pptrate = AddPatchPoint(46.5, 218, ppTypeInput, ppbit, 0, listener);
	pptjitter = AddPatchPoint(73, 218, ppTypeInput, ppbit, 0, listener);
	ppdeja = AddPatchPoint(101, 218, ppTypeInput, ppbit, 0, listener);
	ppxsteps = AddPatchPoint(129.6, 218, ppTypeInput, ppbit, 0, listener);
	ppxspread = AddPatchPoint(155.5, 218, ppTypeInput, ppbit, 0, listener);
	ppxclock = AddPatchPoint(183, 218, ppTypeInput, ppbit, 0, listener);

	ppt1 = AddPatchPoint(18.5, 251, ppTypeOutput, ppbit, 0, listener);
	ppt2 = AddPatchPoint(46.5, 251, ppTypeOutput, ppbit, 0, listener);
	ppt3 = AddPatchPoint(73, 251, ppTypeOutput, ppbit, 0, listener);
	ppy = AddPatchPoint(101, 251, ppTypeOutput, ppbit, 0, listener);
	ppx1 = AddPatchPoint(129.6, 251, ppTypeOutput, ppbit, 0, listener);
	ppx2 = AddPatchPoint(155.5, 251, ppTypeOutput, ppbit, 0, listener);
	ppx3 = AddPatchPoint(183, 251, ppTypeOutput, ppbit, 0, listener);

	// Create "More" menu
	r = CRect(CPoint(SkinScale(152),SkinScale(273)), CPoint(mcbits[black_butbit_tiny_up]->getWidth() - 1, mcbits[black_butbit_tiny_up]->getHeight() - 1));
	more_menu = new COptionMenu(r, listener, NOSAVE_TAG, mcbits[black_butbit_tiny_up], mcbits[black_butbit_tiny_down], kNoTextStyle | kMultipleCheckStyle);
	
	mtmode = new COptionMenu(CRect(0, 0, 0, 0), listener, NOSAVE_TAG, 0, 0, kNoTextStyle | kMultipleCheckStyle);
	addView(mtmode); mtmode->remember();
	mtmode->addEntry("Complementary Bernoulli"); mtmode->addEntry("Clusters");
	mtmode->addEntry("Drums"); mtmode->addEntry("Independent Bernoulli");
	mtmode->addEntry("Divider"); mtmode->addEntry("Three states"); mtmode->addEntry("Markov");
	more_menu->addEntry(mtmode, "T mode");

	mtrange = new COptionMenu(CRect(0, 0, 0, 0), listener, NOSAVE_TAG, 0, 0, kNoTextStyle | kMultipleCheckStyle);
	addView(mtrange); mtrange->remember();
	mtrange->addEntry("1/4x"); mtrange->addEntry("1x"); mtrange->addEntry("4x");
	more_menu->addEntry(mtrange, "T range");

	mxmode = new COptionMenu(CRect(0, 0, 0, 0), listener, NOSAVE_TAG, 0, 0, kNoTextStyle | kMultipleCheckStyle);
	addView(mxmode); mxmode->remember();
	mxmode->addEntry("Identical"); mxmode->addEntry("Bump");
	mxmode->addEntry("Tilt"); 
	more_menu->addEntry(mxmode, "X mode");

	mxrange = new COptionMenu(CRect(0, 0, 0, 0), listener, NOSAVE_TAG, 0, 0, kNoTextStyle | kMultipleCheckStyle);
	addView(mxrange); mxrange->remember();
	mxrange->addEntry("Narrow"); mxrange->addEntry("Positive"); mxrange->addEntry("Full");
	more_menu->addEntry(mxrange, "X range");

	// Add scale menu
	mscale = new COptionMenu(CRect(0, 0, 0, 0), listener, NOSAVE_TAG, 0, 0, kNoTextStyle | kMultipleCheckStyle);
	addView(mscale); mscale->remember();
	mscale->addEntry("Major"); mscale->addEntry("Minor");
	mscale->addEntry("Pentatonic"); mscale->addEntry("Pelog");
	mscale->addEntry("Raag Bhairav That"); mscale->addEntry("Raag Shri");
	more_menu->addEntry(mscale, "Scales");

	// Add Internal X clock source menu
	mint_x_clock = new COptionMenu(CRect(0, 0, 0, 0), listener, NOSAVE_TAG, 0, 0, kNoTextStyle | kMultipleCheckStyle);
	addView(mint_x_clock); mint_x_clock->remember();
	mint_x_clock->addEntry("T1->X1 T2->X2 T3->X3"); mint_x_clock->addEntry("T1->X1,X2,X3");
	mint_x_clock->addEntry("T2->X1,X2,X3"); mint_x_clock->addEntry("T3->X1,X2,X3");
	more_menu->addEntry(mint_x_clock, "Internal X clock source");

	// Add Y divider ratio menu
	my_divider = new COptionMenu(CRect(0, 0, 0, 0), listener, NOSAVE_TAG, 0, 0, kNoTextStyle | kMultipleCheckStyle);
	addView(my_divider); my_divider->remember();
	my_divider->addEntry("1/64"); my_divider->addEntry("1/48");
	my_divider->addEntry("1/32"); my_divider->addEntry("1/24");
	my_divider->addEntry("1/16"); my_divider->addEntry("1/12");
	my_divider->addEntry("1/8"); my_divider->addEntry("1/6");
	my_divider->addEntry("1/4"); my_divider->addEntry("1/3");
	my_divider->addEntry("1/2"); my_divider->addEntry("1");
	more_menu->addEntry(my_divider, "Y divider ratio");

	addView(more_menu);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	PutRightScrews(screw3, screw4, listener);
	InitPatchPoints(0.0);

	random_generator.Init(1);
	random_stream.Init(&random_generator);
	note_filter.Init();
	SetSampleRate(sample_rate);
	OnReset();
}

Marbles::~Marbles()
{
}

void Marbles::OnReset()
{
	t_deja_vu = false;
	x_deja_vu = false;
	t_mode = 0; mtmode->checkEntryAlone(t_mode);
	x_mode = 0; mxmode->checkEntryAlone(x_mode);
	t_range = 1; mtrange->checkEntryAlone(t_range);
	x_range = 1; mxrange->checkEntryAlone(x_range);
	external = false;
	x_scale = 0; mscale->checkEntryAlone(x_scale);
	y_divider_index = 8; my_divider->checkEntryAlone(y_divider_index);
	x_clock_source_internal = 0; mint_x_clock->checkEntryAlone(x_clock_source_internal);

	UpdateLEDs();
}


void Marbles::UpdateLEDs()
{	
	ltmode->value = ((float)t_mode + 1.f) / 7.f;
	lxmode->value = ((float)x_mode + 1.f) / 3.f;
	ltrange->value = ((float)t_range + 1.f) / 3.f;
	lxrange->value = ((float)x_range + 1.f) / 3.f;
	lextern->value = external;
	invalid();
}

// SoloRack calls this. It can't directly access rhe constructor since this is a Dll and pointers to constructor can not be easily achieved.
Marbles* Marbles::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Marbles(pParent, listener, psynth_comm, vvoice);
}

void Marbles::Initialize()
{
	char* stemp;

	panel = new CBitmap(dllskindir, NULL, "Marbles.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Marbles::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Marbles::GetName()
{
	return name;
}

const int Marbles::GetNameLen()
{
	return name_len;
}

const char* Marbles::GetVendorName()
{
	return vendorname;
}

const int Marbles::GetVendorNameLen()
{
	return vendorname_len;
}

const int Marbles::GetType()
{
	return kCVSourceProcessor;
}

Product* Marbles::Activate(char* fullname, char* email, char* serial)
{	// Replace this with your own implementation
	// This should return a pointer to the activated product. Or NULL if activation fails
	// In our Marbles, NULL is retuned because the module is already active all the time, so no need to activate.
	return NULL;

	// Here is a simplistic example.
	//if (pproduct!=NULL) delete pproduct;
	//pproduct = new Product(0,1,NULL,false,"ShredsNCharades Baadalon");		// Product names has to include vendor name to ensure uniquenes
	//return pproduct->Activate(fullname,email,serial);
}

bool Marbles::IsActive()
{	// This test module is Active. Replace this with your own check. 
	// It's better not to store this active/inactive status in an obvious place in memory, like a data member of the module or like that.
	// It's even better if the status is not stored at all, but rather a sophisticated test is done
	return true;

	// simple example
	//if (pproduct!=NULL) return pproduct->IsActive(); else return false;
}

Product* Marbles::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Marbles::InstanceIsActive()
{
	return this->IsActive();
}

const char* Marbles::GetProductName()
{	// Change this to your own product name. 
	return "ShredsNCharades Tatvon";
}

void Marbles::ValueChanged(CControl* pControl)
{
	if (pControl == btdeja)
		t_deja_vu = (btdeja->value >= 0.5);
	else if (pControl == bxdeja)
		x_deja_vu = (bxdeja->value >= 0.5);
	else if (pControl == btmode && btmode->value >= 0.5)
	{	t_mode = (t_mode + 1) % 7;
		mtmode->checkEntryAlone(t_mode);
		ltmode->value = ((float)t_mode + 1.f) / 7.f;
		ltmode->invalid();
	}
	else if (pControl == bxmode && bxmode->value >= 0.5)
	{	x_mode = (x_mode + 1) % 3;
		mxmode->checkEntryAlone(x_mode);
		lxmode->value = ((float)x_mode + 1.f) / 3.f;
		lxmode->invalid();
	}
	else if (pControl == btrange && btrange->value >= 0.5)
	{	t_range = (t_range + 1) % 3;
		mtrange->checkEntryAlone(t_range);
		ltrange->value = ((float)t_range + 1.f) / 3.f;
		ltrange->invalid();
	}
	else if (pControl == bxrange && bxrange->value >= 0.5)
	{	x_range = (x_range + 1) % 3;
		mxrange->checkEntryAlone(x_range);
		lxrange->value = ((float)x_range + 1.f) / 3.f;
		lxrange->invalid();
	}
	else if (pControl == bextern && bextern->value >= 0.5)
	{	external = !external;
		lextern->value = external;
		lextern->invalid();
	}
	else if (pControl == mscale)
	{	x_scale = mscale->value;
		mscale->checkEntryAlone(x_scale);
	}
	else if (pControl == mint_x_clock)
	{	x_clock_source_internal = mint_x_clock->value;
		mint_x_clock->checkEntryAlone(x_clock_source_internal);
	}
	else if (pControl == my_divider)
	{
		y_divider_index = my_divider->value;
		my_divider->checkEntryAlone(y_divider_index);
	}
	else if (pControl == mtmode)
	{	t_mode = mtmode->value;
		mtmode->checkEntryAlone(t_mode);
		ltmode->value = ((float)t_mode + 1.f) / 7.f;
		ltmode->invalid();
	}
	else if (pControl == mtrange)
	{	t_range = mtrange->value;
		mtrange->checkEntryAlone(t_range);
		ltrange->value = ((float)t_range + 1.f) / 3.f;
		ltrange->invalid();
	}
	else if (pControl == mxmode)
	{	x_mode = mxmode->value;
		mxmode->checkEntryAlone(x_mode);
		lxmode->value = ((float)x_mode + 1.f) / 3.f;
		lxmode->invalid();
	}
	else if (pControl == mxrange)
	{	x_range = mxrange->value;
		mxrange->checkEntryAlone(x_range);
		lxrange->value = ((float)x_range + 1.f) / 3.f;
		lxrange->invalid();
	}
}

void Marbles::SetSampleRate(float sr)
{	Module::SetSampleRate(sr);

	t_generator.Init(&random_stream, sr);
	xy_generator.Init(&random_stream, sr);

	// Set scales
	for (int i = 0; i < 6; i++) 
	{	xy_generator.LoadScale(i, preset_scales[i]);
	}
}

int Marbles::GetPresetSize()
{
	return GetControlsValuesSize() 	\
		+ sizeof(t_deja_vu)		\
		+ sizeof(x_deja_vu)		\
		+ sizeof(t_mode)		\
		+ sizeof(x_mode)		\
		+ sizeof(t_range)		\
		+ sizeof(x_range)		\
		+ sizeof(external)		\
		+ sizeof(x_scale)		\
		+ sizeof(y_divider_index)	\
		+ sizeof(x_clock_source_internal);
}

void Marbles::SavePreset(void* pdata, int size)
{
	char* pcdata = (char*)pdata;
	SaveControlsValues(pcdata);
	pcdata += GetControlsValuesSize();

	// Save none control data
	*(bool*)pcdata = t_deja_vu; pcdata += sizeof(t_deja_vu);
	*(bool*)pcdata = x_deja_vu; pcdata += sizeof(x_deja_vu);
	*(int*)pcdata = t_mode; pcdata += sizeof(t_mode);
	*(int*)pcdata = x_mode; pcdata += sizeof(x_mode);
	*(int*)pcdata = t_range; pcdata += sizeof(t_range);
	*(int*)pcdata = x_range; pcdata += sizeof(x_range);
	*(bool*)pcdata = external; pcdata += sizeof(external);
	*(int*)pcdata = x_scale; pcdata += sizeof(x_scale);
	*(int*)pcdata = y_divider_index; pcdata += sizeof(y_divider_index);
	*(int*)pcdata = x_clock_source_internal; pcdata += sizeof(x_clock_source_internal);
}

void Marbles::LoadPreset(void* pdata, int size, int version)
{
	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();

	LoadControlsValues(pcdata, csize);
	pcdata += csize;

	// Load non control data
	t_deja_vu = *(bool*)pcdata; pcdata += sizeof(t_deja_vu);
	x_deja_vu = *(bool*)pcdata; pcdata += sizeof(x_deja_vu);
	t_mode = *(int*)pcdata; pcdata += sizeof(t_mode);
	x_mode = *(int*)pcdata; pcdata += sizeof(x_mode);
	t_range = *(int*)pcdata; pcdata += sizeof(t_range);
	x_range = *(int*)pcdata; pcdata += sizeof(x_range);
	external = *(bool*)pcdata; pcdata += sizeof(external);
	x_scale = *(int*)pcdata; pcdata += sizeof(x_scale);
	y_divider_index = *(int*)pcdata; pcdata += sizeof(y_divider_index);
	x_clock_source_internal = *(int*)pcdata; pcdata += sizeof(x_clock_source_internal);

	// Update MORE menus
	mtmode->checkEntryAlone(t_mode);
	mtrange->checkEntryAlone(t_range);
	mxmode->checkEntryAlone(x_mode);
	mxrange->checkEntryAlone(x_range);
	mscale->checkEntryAlone(x_scale);
	mint_x_clock->checkEntryAlone(x_clock_source_internal);
	my_divider->checkEntryAlone(y_divider_index);

	UpdateLEDs();
}

const char* Marbles::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/marbles/";
}

void Marbles::StepBlock() 
{
	// Ramps
	marbles::Ramps ramps;
	ramps.master = ramp_master;
	ramps.external = ramp_external;
	ramps.slave[0] = ramp_slave[0];
	ramps.slave[1] = ramp_slave[1];

	float deja_vu = CLIP(kdeja->value + ppdeja->in, 0.f, 1.f);
	static const int loop_length[] = {
		1, 1, 1, 2, 2,
		2, 2, 2, 3, 3,
		3, 3, 4, 4, 4,
		4, 4, 5, 5, 6,
		6, 6, 7, 7, 8,
		8, 8, 10, 10, 12,
		12, 12, 14, 14, 16,
		16
	};
	float deja_vu_length_index = klength->value * (LENGTHOF(loop_length) - 1);
	int deja_vu_length = loop_length[(int)roundf(deja_vu_length_index)];

	// Set up TGenerator

	bool t_external_clock = pptclock->num_cables>0;

	t_generator.set_model((marbles::TGeneratorModel)t_mode);
	t_generator.set_range((marbles::TGeneratorRange)t_range);
	float t_rate = 60.f * (SCALE(ktrate->value,-1.f,1.f) + pptrate->in);
	t_generator.set_rate(t_rate);
	float t_bias = CLIP(ktbias->value + pptbias->in, 0.f, 1.f);
	t_generator.set_bias(t_bias);
	float t_jitter = CLIP(ktjitter->value + pptjitter->in, 0.f, 1.f);
	t_generator.set_jitter(t_jitter);
	t_generator.set_deja_vu(t_deja_vu ? deja_vu : 0.f);
	t_generator.set_length(deja_vu_length);
	// TODO
	t_generator.set_pulse_width_mean(0.f);
	t_generator.set_pulse_width_std(0.f);

	t_generator.Process(t_external_clock, t_clocks, ramps, gates, BLOCK_SIZE);

	// Set up XYGenerator

	marbles::ClockSource x_clock_source = (marbles::ClockSource)x_clock_source_internal;
	if (ppxclock->num_cables>0)
		x_clock_source = marbles::CLOCK_SOURCE_EXTERNAL;

	marbles::GroupSettings x;
	x.control_mode = (marbles::ControlMode)x_mode;
	x.voltage_range = (marbles::VoltageRange)x_range;
	// TODO Fix the scaling
	float note_cv = (kxspread->value + ppxspread->in);
	float u = note_filter.Process(0.25f * (note_cv + 2.f));
	x.register_mode = external;
	x.register_value = u;

	float x_spread = CLIP(note_cv, 0.f, 1.f);
	x.spread = x_spread;
	float x_bias = CLIP(kxbias->value + ppxbias->in, 0.f, 1.f);
	x.bias = x_bias;
	float x_steps = CLIP(kxsteps->value + ppxsteps->in, 0.f, 1.f);
	x.steps = x_steps;
	x.deja_vu = x_deja_vu ? deja_vu : 0.f;
	x.length = deja_vu_length;
	x.ratio.p = 1;
	x.ratio.q = 1;
	x.scale_index = x_scale;

	marbles::GroupSettings y;
	y.control_mode = marbles::CONTROL_MODE_IDENTICAL;
	// TODO
	y.voltage_range = (marbles::VoltageRange)x_range;
	y.register_mode = false;
	y.register_value = 0.0f;
	// TODO
	y.spread = x_spread;
	y.bias = x_bias;
	y.steps = x_steps;
	y.deja_vu = 0.0f;
	y.length = 1;
	static const marbles::Ratio y_divider_ratios[] = {
		{ 1, 64 },
		{ 1, 48 },
		{ 1, 32 },
		{ 1, 24 },
		{ 1, 16 },
		{ 1, 12 },
		{ 1, 8 },
		{ 1, 6 },
		{ 1, 4 },
		{ 1, 3 },
		{ 1, 2 },
		{ 1, 1 },
	};
	y.ratio = y_divider_ratios[y_divider_index];
	y.scale_index = x_scale;

	xy_generator.Process(x_clock_source, x, y, xy_clocks, ramps, voltages, BLOCK_SIZE);
}

inline void Marbles::ProcessSample()
{
	// Clocks
	bool t_gate = (pptclock->in >= 1.7f/5.f);
	last_t_clock = stmlib::ExtractGateFlags(last_t_clock, t_gate);
	t_clocks[blockIndex] = last_t_clock;

	bool x_gate = (ppxclock->in >= 1.7f/5.f);
	last_xy_clock = stmlib::ExtractGateFlags(last_xy_clock, x_gate);
	xy_clocks[blockIndex] = last_xy_clock;

	// Process block
	if (++blockIndex >= BLOCK_SIZE) 
	{	blockIndex = 0; StepBlock();
	}

	// Outputs and output LEDs 
	lt1->value = ppt1->out = (gates[blockIndex * 2] ? 1.f : 0.f);
	lt2->value = ppt2->out = ((ramp_master[blockIndex] < 0.5f) ? 1.f : 0.f);
	lt3->value = ppt3->out = (gates[blockIndex * 2 + 1] ? 1.f : 0.f);
	
	lx1->value = ppx1->out = voltages[blockIndex * 4]/5.f;
	lx2->value = ppx2->out = voltages[blockIndex * 4 + 1]/5.f;
	lx3->value = ppx3->out = voltages[blockIndex * 4 + 2]/5.f;
	ly->value = ppy->out = voltages[blockIndex * 4 + 3]/5.f;
}



//---------------------------------------------
// Frames
CBitmap* Frames::panel = NULL;
char* Frames::name = "Frem";
int Frames::name_len = 0;
char* Frames::vendorname = "ShredsNCharades";
int Frames::vendorname_len = 0;
Product* Frames::pproduct = NULL;

Frames::Frames(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{
	CRect r;
	int i;

	// Create The Knobs
	kgain[0] = AddModuleKnob(28, 56.5, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kgain[1] = AddModuleKnob(84, 56.5, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kgain[2] = AddModuleKnob(141, 56.5, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kgain[3] = AddModuleKnob(197.3, 56.5, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kframe = AddModuleKnob(113, 130, mcbits[knob_big_white], SNC_BIG_KNOB_IMAGES, false, listener);
	kmod = AddModuleKnob(192, 130, mcbits[knob_small_green], SNC_SMALL_KNOB_IMAGES, false, listener);
	//kdeja->setValue(0.5f);

	// LEDs
	lg[0] = AddMovieBitmap(28, 77, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	lg[1] = AddMovieBitmap(84, 77, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	lg[2] = AddMovieBitmap(141, 77, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	lg[3] = AddMovieBitmap(197.3, 77, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	ladd_del = AddMovieBitmap(55, 124, mcbits[led_big_green], SNC_LED_GREEN_IMAGES, listener);

	// Buttons
	badd = AddKickButton(27, 105, mcbits[button_big], 2, listener);
	bdel = AddKickButton(27, 142.6, mcbits[button_big], 2, listener);

	// Switches (on/off button for now)
	boffset = AddOnOffButton(24, 188, mcbits[black_switch], 2, listener, COnOffButton::kPostListenerUpdate);

	// Create Patch Points
	ppall = AddPatchPoint(22.6, 218, ppTypeInput, ppbit, 0, listener);
	ppin[0] = AddPatchPoint(59, 218, ppTypeInput, ppbit, 0, listener);
	ppin[1] = AddPatchPoint(95, 218, ppTypeInput, ppbit, 0, listener);
	ppin[2] = AddPatchPoint(131, 218, ppTypeInput, ppbit, 0, listener);
	ppin[3] = AddPatchPoint(167.5, 218, ppTypeInput, ppbit, 0, listener);
	ppframe = AddPatchPoint(204, 218, ppTypeInput, ppbit, 0, listener);

	ppmix = AddPatchPoint(22.6, 251, ppTypeOutput, ppbit, 0, listener);
	ppout[0] = AddPatchPoint(59, 251, ppTypeOutput, ppbit, 0, listener);
	ppout[1] = AddPatchPoint(95, 251, ppTypeOutput, ppbit, 0, listener);
	ppout[2] = AddPatchPoint(131, 251, ppTypeOutput, ppbit, 0, listener);
	ppout[3] = AddPatchPoint(167.5, 251, ppTypeOutput, ppbit, 0, listener);
	ppfrstep = AddPatchPoint(204, 251, ppTypeOutput, ppbit, 0, listener);

	// Create "More" menu
	r = CRect(CPoint(SkinScale(173), SkinScale(273)), CPoint(mcbits[black_butbit_tiny_up]->getWidth() - 1, mcbits[black_butbit_tiny_up]->getHeight() - 1));
	more_menu = new COptionMenu(r, listener, NOSAVE_TAG, mcbits[black_butbit_tiny_up], mcbits[black_butbit_tiny_down], kNoTextStyle | kMultipleCheckStyle);

	char buf[15];
	for (i = 0; i < 4; i++)
	{	mch[i] = new COptionMenu(CRect(0, 0, 0, 0), listener, NOSAVE_TAG, 0, 0, kNoTextStyle | kMultipleCheckStyle);
		addView(mch[i]); mch[i]->remember();
		mch[i]->addEntry("-- Interpolation curve --"); mch[i]->getEntry(mch[i]->getNbEntries()-1)->setEnabled(false);
		mch[i]->addEntry("Step"); mch[i]->addEntry("Linear");
		mch[i]->addEntry("Accelerating"); mch[i]->addEntry("Decelerating");
		mch[i]->addEntry("Departure/arrival"); mch[i]->addEntry("Bouncing");
		
		mch[i]->addSeparator();
		mch[i]->addEntry("-- Response curve --"); mch[i]->getEntry(mch[i]->getNbEntries()-1)->setEnabled(false);
		mch[i]->addEntry("Linear"); mch[i]->addEntry("Exponential");

		sprintf(buf, "Channel %d", i+1);
		more_menu->addEntry(mch[i], buf);
	}
	more_menu->addEntry("Clear keyframes"); more_menu->addSeparator();
	more_menu->addEntry("Keyframer"); more_menu->addEntry("Poly LFO");
	addView(more_menu);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	PutRightScrews(screw3, screw4, listener);
	InitPatchPoints(0.0);

	r = kframe->getViewSize();
	frx = 0.5*(r.getTopRight().x + r.getBottomLeft().x);
	fry = 0.5*(r.getTopRight().y + r.getBottomLeft().y) - FSkinScale(6.f);

	// Initialize
	memset(&keyframer, 0, sizeof(keyframer));
	keyframer.Init();
	memset(&poly_lfo, 0, sizeof(poly_lfo));
	poly_lfo.Init();
	OnReset(); UpdateFrameColors(true);

	// Set gain knobs
	ValueChanged(kframe); ValueChanged(boffset);
	for (int i = 0; i < 4; i++) ValueChanged(kgain[i]);
	kmod->value = 0.5; ValueChanged(kmod);
}

Frames::~Frames()
{
}

void Frames::drawRect(CDrawContext* pContext, const CRect& updateRect)
{
	float ft;
	int32_t colorsi;
	CDrawMode drmode;

	Module::drawRect(pContext, updateRect);
	
	colorsi = frcolors[0] | (frcolors[1] << 8) | (frcolors[2] << 16);
	if (colorsi != (255 | 255 << 8 | 255 << 16))		// White indicates no color/glow
	{	drmode = pContext->getDrawMode();
		pContext->setDrawMode(kCopyMode);
		ft = Rgb2Hue(frcolors[0], frcolors[1], frcolors[2]);
		DrawGlow(pContext, frx + pContext->offset.x + size.left, fry + pContext->offset.y + size.top, FSkinScale(85), ft, 0.04, 10);
		pContext->setDrawMode(drmode);
	}
	// Signal that new color hsa been drawn
	last_colorsi = colorsi;
}


void Frames::OnReset()
{	
	last_timestampMod = -10000;
	poly_lfo_mode = false;
	keyframer.Clear();
	for (int i = 0; i < 4; i++) 
	{	keyframer.mutable_settings(i)->easing_curve = frames::EASING_CURVE_LINEAR;
		keyframer.mutable_settings(i)->response = 0;
		mch[i]->checkEntryAlone(kLinear+1); mch[i]->checkEntry(kLinearR+1, true);
	}
	more_menu->checkEntryAlone(kKeyframer);

	step_count = 0;
	step_time = FRAMES_STEP_TIME * sample_rate;
}

// SoloRack calls this. It can't directly access rhe constructor since this is a Dll and pointers to constructor can not be easily achieved.
Frames* Frames::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Frames(pParent, listener, psynth_comm, vvoice);
}

void Frames::Initialize()
{
	char* stemp;

	panel = new CBitmap(dllskindir, NULL, "Frames.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Frames::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Frames::GetName()
{
	return name;
}

const int Frames::GetNameLen()
{
	return name_len;
}

const char* Frames::GetVendorName()
{
	return vendorname;
}

const int Frames::GetVendorNameLen()
{
	return vendorname_len;
}

const int Frames::GetType()
{
	return kMixerPanner;
}

Product* Frames::Activate(char* fullname, char* email, char* serial)
{	return NULL;
}

bool Frames::IsActive()
{	return true;
}

Product* Frames::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Frames::InstanceIsActive()
{
	return this->IsActive();
}

const char* Frames::GetProductName()
{	// Change this to your own product name. 
	return "ShredsNCharades Frem";
}

void Frames::CableConnected(PatchPoint* pp)
{
	int i;

	allin = (ppall->num_cables > 0);

	for (i = 0; i < 4; i++)
	{	if (pp == ppin[i])
		{	isppin[i] = (ppin[i]->num_cables > 0); return;
		}
		if (pp == ppout[i])
		{	isppout[i] = (ppout[i]->num_cables > 0); return;
		}
	}
}

void Frames::CableDisconnected(PatchPoint* pp)
{
	CableConnected(pp);
}

void Frames::ValueChanged(CControl* pControl)
{
	int i,j,n;

	// Set gain knobs
	for (i = 0; i < 4; i++)
	{
		if (pControl == kgain[i])
		{
			EnterProcessingCriticalSection();
			controls[i] = kgain[i]->value * 65535.0;

			if (poly_lfo_mode)
			{
				switch (i)
				{
				case 0: poly_lfo.set_shape(controls[0]); break;
				case 1: poly_lfo.set_shape_spread(controls[1]); break;
				case 2: poly_lfo.set_spread(controls[2]); break;
				case 3: poly_lfo.set_coupling(controls[3]); break;
				}
			}
			else
			{	// Update recently moved control
				if (keyframer.num_keyframes() == 0) {
					keyframer.set_immediate(i, controls[i]);
				}
				if (nearestIndex >= 0)
				{
					frames::Keyframe* nearestKeyframe = keyframer.mutable_keyframe(nearestIndex);
					nearestKeyframe->values[i] = controls[i];
				}
			}

			// Update last controls
			lastcontrols[i] = controls[i];

			LeaveProcessingCriticalSection();

			return;
		}
	}

	if (pControl == kframe)
	{
		timestamp = kframe->value * 65535.0;
		if (!poly_lfo_mode)
		{
			nearestIndex = keyframer.FindNearestKeyframe(timestamp, 2048);
		}
		else nearestIndex = -1;
		//last_timestampMod = -10000;
	}
	else if (pControl == kmod)
		frmod = SCALE(kmod->value, -1.f, 1.f);
	else if (pControl == badd)
	{	if (badd->value >= 0.5f && !poly_lfo_mode && nearestIndex < 0)
		{
			EnterProcessingCriticalSection();
			keyframer.AddKeyframe(timestamp, controls);
			keyframer.Evaluate(last_timestampMod);
			nearestIndex = keyframer.FindNearestKeyframe(timestamp, 2048);
			UpdateFrameColors(true);
			LeaveProcessingCriticalSection();
		}
	}
	else if (pControl == bdel)
	{	if (bdel->value >= 0.5f && !poly_lfo_mode && nearestIndex >= 0)
		{
			int32_t nearestTimestamp;
			EnterProcessingCriticalSection();
			nearestTimestamp = keyframer.keyframe(nearestIndex).timestamp;
			keyframer.RemoveKeyframe(nearestTimestamp);
			keyframer.Evaluate(last_timestampMod);
			nearestIndex = keyframer.FindNearestKeyframe(timestamp, 2048);
			//keyframer.num_keyframes()
			UpdateFrameColors(true);
			LeaveProcessingCriticalSection();
		}
	}
	else if (pControl == boffset)
	{
		offsetv = (boffset->value == 1.f) ? 10.0 : 0.0;
	}
	else if (pControl == more_menu)
	{
		if (more_menu->value == kKeyframer)
		{
			EnterProcessingCriticalSection();
			poly_lfo_mode = false;
			keyframer.Evaluate(last_timestampMod);
			nearestIndex = keyframer.FindNearestKeyframe(timestamp, 2048);
			UpdateFrameColors(true);
			LeaveProcessingCriticalSection();
			more_menu->checkEntryAlone(kKeyframer);
		}
		else if (more_menu->value == kPolyLFO)
		{
			EnterProcessingCriticalSection();
			poly_lfo_mode = true; nearestIndex = -1;
			UpdateFrameColors(true);
			LeaveProcessingCriticalSection();
			more_menu->checkEntryAlone(kPolyLFO);
		}
		else if (more_menu->value == kClearKeyframes)
		{	EnterProcessingCriticalSection();
			keyframer.Clear();
			keyframer.Evaluate(last_timestampMod);
			nearestIndex = keyframer.FindNearestKeyframe(timestamp, 2048);
			UpdateFrameColors(true);
			LeaveProcessingCriticalSection();
			more_menu->getEntry(kClearKeyframes)->setChecked(false);
		}
		
	}
	else
	{	j = pControl->value; j--;
		for (i = 0; i < 4; i++)
			if (pControl == mch[i])
			{	if (j <= kBouncing)
				{	keyframer.mutable_settings(i)->easing_curve = (frames::EasingCurve)j;
					for (n = kStep; n <= kBouncing; n++)
						mch[i]->checkEntry(n+1, false);
					mch[i]->checkEntry(j+1, true);
				}
				else if (j == kLinearR)
				{	keyframer.mutable_settings(i)->response = 0;
					mch[i]->checkEntry(kLinearR + 1, true);
					mch[i]->checkEntry(kExponentialR + 1, false);
				}
				else if (j == kExponentialR)
				{	keyframer.mutable_settings(i)->response = 255;
					mch[i]->checkEntry(kExponentialR + 1, true);
					mch[i]->checkEntry(kLinearR + 1, false);
				}
				break;
			}
	}
}

void Frames::SetSampleRate(float sr)
{	Module::SetSampleRate(sr);

	step_time = FRAMES_STEP_TIME * sample_rate;
}

int Frames::GetPresetSize()
{
	return GetControlsValuesSize() 	\
		+ sizeof(poly_lfo_mode)	\
		+ sizeof(uint16_t)	/*Number of keyframes*/ \
		+ keyframer.num_keyframes() * (sizeof(frames::Keyframe::timestamp) + 4 * sizeof(frames::Keyframe::values[0]))	\
		+ 4*(sizeof(int) + sizeof(uint8_t));	// for Easingcurve + response
}

void Frames::SavePreset(void* pdata, int size)
{
	int k;
	uint16_t i,num;

	char* pcdata = (char*)pdata;
	SaveControlsValues(pcdata);
	pcdata += GetControlsValuesSize();

	// Save none control data
	*(bool*)pcdata = poly_lfo_mode; pcdata += sizeof(poly_lfo_mode);

	// Number of keyframes
	num = keyframer.num_keyframes();
	*(uint16_t*)pcdata = num; pcdata += sizeof(num);

	// Save each keyframe
	for (i = 0; i < num; i++) 
	{	frames::Keyframe* keyframe = keyframer.mutable_keyframe(i);

		*(uint16_t*)pcdata = keyframe->timestamp; pcdata += sizeof(keyframe->timestamp);
		for (k = 0; k < 4; k++) 
		{	*(uint16_t*)pcdata = keyframe->values[k]; pcdata += sizeof(keyframe->values[0]);
		}
	}

	// Save channels curves 
	for (k = 0; k < 4; k++)
	{	*(int*)pcdata = (int)(keyframer.mutable_settings(k)->easing_curve); pcdata += sizeof(int);
		*(uint8_t*)pcdata = keyframer.mutable_settings(k)->response; pcdata += sizeof(uint8_t);
	}
}

void Frames::LoadPreset(void* pdata, int size, int version)
{
	uint16_t i, num, timestamp, values[4];
	uint8_t i8;
	int k;

	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();

	LoadControlsValues(pcdata, csize);
	pcdata += csize;

	// Load non control data
	poly_lfo_mode = *(bool*)pcdata; pcdata += sizeof(poly_lfo_mode);
	if (poly_lfo_mode)	more_menu->value = kPolyLFO;
	else more_menu->value = kKeyframer;
	ValueChanged(more_menu);

	// Number of keyframes
	num = *(uint16_t*)pcdata; pcdata += sizeof(num);

	for (i = 0; i < num; i++)
	{	timestamp = *(uint16_t*)pcdata; pcdata += sizeof(timestamp);
		for (k = 0; k < 4; k++)
		{	values[k] = *(uint16_t*)pcdata; pcdata += sizeof(values[0]);
		}
		keyframer.AddKeyframe(timestamp, values);
		keyframer.Evaluate(last_timestampMod);
		nearestIndex = keyframer.FindNearestKeyframe(timestamp, 2048);
	}

	// Load channels curves 
	for (i = 0; i < 4; i++)
	{	k = *(int*)pcdata;
		keyframer.mutable_settings(i)->easing_curve = (frames::EasingCurve) k; pcdata += sizeof(int);
		mch[i]->value = k+1; ValueChanged(mch[i]);

		i8 = *(uint8_t*)pcdata;
		keyframer.mutable_settings(i)->response = i8; pcdata += sizeof(i8);
		if (i8 == 0) mch[i]->value = kLinearR + 1;
		else mch[i]->value = kExponentialR + 1;
		ValueChanged(mch[i]);
	}

	ValueChanged(kframe);
	UpdateFrameColors(true);
}

const char* Frames::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/frames/";
}

void Frames::UpdateFrameColors(bool force_redraw)
{
	int32_t colorsi;

	if (poly_lfo_mode)
	{
		frcolors[0] = poly_lfo.color()[0];
		frcolors[1] = poly_lfo.color()[1];
		frcolors[2] = poly_lfo.color()[2];
	}
	else
	{
		frcolors[0] = keyframer.color()[0];
		frcolors[1] = keyframer.color()[1];
		frcolors[2] = keyframer.color()[2];
	}
	if (force_redraw) this->invalid();
	else
	{	colorsi = frcolors[0] | (frcolors[1] << 8) | (frcolors[2] << 16);
		if (colorsi != last_colorsi) this->setDirty();
	}
}

inline void Frames::ProcessSample()
{
	float ft;
	int32_t timestampMod = timestamp + frmod * ppframe->in * 0.5 * 65535.0;
	timestampMod = CLIP(timestampMod, 0, 65535);

	// Render, handle buttons
	if (poly_lfo_mode) 
	{	poly_lfo.Render(timestampMod);
	}
	else 
	{	keyframer.Evaluate(timestampMod);
		
		// FR step
		if (keyframer.position()!=-1 && keyframer.position() != last_position)
		{	step_count = step_time; ppfrstep->out = 1.0;
		}
		last_position = keyframer.position();
	}

	// Get gains
	float gains[4];
	for (int i = 0; i < 4; i++) 
	{	if (poly_lfo_mode) 
		{	//gains[i] = poly_lfo.level(i) / 255.0;
			gains[i] = poly_lfo.level16(i) / 65535.0;
		}
		else 
		{	float lin = keyframer.level(i) / 65535.0;
			gains[i] = lin;
		}
		// Simulate SSM2164
		if (keyframer.mutable_settings(i)->response > 0) 
		{	const float expBase = 200.0;
			float expGain = rescale(powf(expBase, gains[i]), 1.0f, expBase, 0.0f, 1.0f);
			gains[i] = crossfade(gains[i], expGain, keyframer.mutable_settings(i)->response / 255.0f);
		}
	}

	// Get inputs
	float all;
	if (allin) all = ppall->in * 5.0;
	else all = offsetv;

	float ins[4];
	for (int i = 0; i < 4; i++) 
	{	if (isppin[i])
			ins[i] = ppin[i]->in*5.0 * gains[i];
		else
			ins[i] = all * gains[i];
	}

	// Set outputs
	float mix = 0.0;
	for (int i = 0; i < 4; i++)
	{	if (isppout[i]) ppout[i]->out = ins[i] / 5.0;
		else mix += ins[i];
	}

	//ppmix->out = CLIP(mix/2, -10.f, 10.f);
	ppmix->out = CLIP(mix/10.f, -2.f, 2.f);

	// FR Step
	//if (step_count == 0) ppfrstep->out = 0.0; else step_count--;
	if (step_count > 0) step_count--; else ppfrstep->out = 0.0;

	// Set lights
	for (int i = 0; i < 4; i++) 
	{	lg[i]->value = gains[i];
	}

	if (poly_lfo_mode) 
		ladd_del->value = (poly_lfo.level(0) > 128 ? 1.0 : 0.0);
	else
	{	//ft = ladd_del->value;
		ladd_del->value = (nearestIndex >= 0 ? 1.0 : 0.0);
		//if (ft<ladd_del->value)
	}

	// If frame knob colors changed, update GUI
	if (timestampMod != last_timestampMod)
	{	last_timestampMod = timestampMod;
		UpdateFrameColors(false);
	}
}


//---------------------------------------------
// Streams
CBitmap* Streams::panel = NULL;
char* Streams::name = "Pravah";
int Streams::name_len = 0;
char* Streams::vendorname = "ShredsNCharades";
int Streams::vendorname_len = 0;
Product* Streams::pproduct = NULL;

Streams::Streams(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{
	int i,j;
	CRect r;

	// Create The Knobs
	kshape1 = AddModuleKnob(28, 52, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kmod1 = AddModuleKnob(28, 105, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	klevel1 = AddModuleKnob(28, 157, mcbits[knob_small_red], SNC_SMALL_KNOB_IMAGES, false, listener);
	kshape2 = AddModuleKnob(115.5, 52, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kmod2 = AddModuleKnob(115.5, 105, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	klevel2 = AddModuleKnob(115.5, 157, mcbits[knob_small_green], SNC_SMALL_KNOB_IMAGES, false, listener);
	kresp1 = AddModuleKnob(72.6, 140, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kresp2 = AddModuleKnob(72.6, 173, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);

	kshape1->value = 0.5; kmod1->value = 0.5; klevel1->value = 0.5; kresp1->value = 0.5;
	kshape2->value = 0.5; kmod2->value = 0.5; klevel2->value = 0.5; kresp2->value = 0.5;

	// LEDs
	for (i = 0; i < 4; i++)
	{	lch1[i] = AddMovieBitmap(60, 49.9 + i * 12.9, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
		lch2[i] = AddMovieBitmap(85.1, 49.9 + i * 12.9, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	}

	// Buttons
	bfunc1 = ModAddKickButtonEx(this, 59.7, 35, mcbits[black_butbit_tiny], 2, listener);
	bfunc2 = ModAddKickButtonEx(this, 84.8, 35, mcbits[black_butbit_tiny], 2, listener);
	bmeter = ModAddKickButtonEx(this, 72.25, 106, mcbits[black_butbit_tiny], 2, listener);

	// Switches (on/off button for now)
	//boffset = AddOnOffButton(24, 188, mcbits[black_switch], 2, listener, COnOffButton::kPostListenerUpdate);

	// Create Patch Points
	ppexcite1 = AddPatchPoint(22, 216, ppTypeInput, ppbit, 0, listener);
	ppin1 = AddPatchPoint(55.1, 216, ppTypeInput, ppbit, 0, listener);
	pplevel1 = AddPatchPoint(22, 250, ppTypeInput, ppbit, 0, listener);
	ppout1 = AddPatchPoint(55.1, 250, ppTypeOutput, ppbit, 0, listener);

	ppin2 = AddPatchPoint(88.2, 216, ppTypeInput, ppbit, 0, listener);
	ppexcite2 = AddPatchPoint(121.3, 216, ppTypeInput, ppbit, 0, listener);
	pplevel2 = AddPatchPoint(121.3, 250, ppTypeInput, ppbit, 0, listener);
	ppout2 = AddPatchPoint(88.2, 250, ppTypeOutput, ppbit, 0, listener);

	// Create "More" menu
	r = CRect(CPoint(SkinScale(90),SkinScale(273)), CPoint(mcbits[black_butbit_tiny_up]->getWidth() - 1, mcbits[black_butbit_tiny_up]->getHeight() - 1));
	more_menu = new COptionMenu(r, listener, NOSAVE_TAG, mcbits[black_butbit_tiny_up], mcbits[black_butbit_tiny_down], kNoTextStyle | kMultipleCheckStyle);
	more_menu->addEntry("Link channels");

	char buf[20];
	for (i = 0; i < 2; i++)
	{	mch[i] = new COptionMenu(CRect(0, 0, 0, 0), listener, NOSAVE_TAG, 0, 0, kNoTextStyle | kMultipleCheckStyle);
		addView(mch[i]); mch[i]->remember();
		for (j = 0; j < streams::kNumChannelModes; j++)
		{	mch[i]->addEntry(streams::kChannelModeTable[j].label.c_str());
		}
		sprintf(buf, "Channel %d mode", i+1);
		more_menu->addEntry(mch[i], buf);
	}
	mmeter = new COptionMenu(CRect(0, 0, 0, 0), listener, NOSAVE_TAG, 0, 0, kNoTextStyle | kMultipleCheckStyle);
	addView(mmeter); mmeter->remember();
	for (i=0; i<streams::kNumMonitorModes; i++) 
	{	mmeter->addEntry(streams::kMonitorModeTable[i].label.c_str());
	}
	more_menu->addEntry(mmeter,"Meter");
	addView(more_menu);

	ValueChanged(NULL);  // This is going to update for all knobs and buttons.
	CableDisconnected(NULL);
	OnReset();

	// Apply default setting on menus
	UpdateMenus();

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	InitPatchPoints(0.0);
}

Streams::~Streams()
{
}

void Streams::OnReset()
{	engine.Reset();
	engine.SetSampleRate(sample_rate);
}

void Streams::SetLinked(bool linked) 
{	streams::UiSettings settings = engine.ui_settings();
	settings.linked = linked;
	engine.ApplySettings(settings);
}

int Streams::GetChannelMode(int channel) 
{	streams::UiSettings settings = engine.ui_settings();
	// Search channel mode index in table
	for (int i = 0; i < streams::kNumChannelModes; i++) 
	{	if (settings.function[channel] == streams::kChannelModeTable[i].function
			&& settings.alternate[channel] == streams::kChannelModeTable[i].alternate)
			return i;
	}
	return -1;
}

void Streams::SetChannelMode(int channel, int mode_id) 
{	streams::UiSettings settings = engine.ui_settings();
	settings.function[channel] = streams::kChannelModeTable[mode_id].function;
	settings.alternate[channel] = streams::kChannelModeTable[mode_id].alternate;
	engine.ApplySettings(settings);
}

void Streams::SetMonitorMode(int mode_id) 
{	streams::UiSettings settings = engine.ui_settings();
	settings.monitor_mode = streams::kMonitorModeTable[mode_id].mode;
	engine.ApplySettings(settings);
}

// SoloRack calls this. It can't directly access rhe constructor since this is a Dll and pointers to constructor can not be easily achieved.
Streams* Streams::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Streams(pParent, listener, psynth_comm, vvoice);
}

void Streams::Initialize()
{
	char* stemp;

	panel = new CBitmap(dllskindir, NULL, "Streams.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Streams::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Streams::GetName()
{
	return name;
}

const int Streams::GetNameLen()
{
	return name_len;
}

const char* Streams::GetVendorName()
{
	return vendorname;
}

const int Streams::GetVendorNameLen()
{
	return vendorname_len;
}

const int Streams::GetType()
{
	return kAmplifier;
}

Product* Streams::Activate(char* fullname, char* email, char* serial)
{
	return NULL;
}

bool Streams::IsActive()
{
	return true;
}

Product* Streams::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Streams::InstanceIsActive()
{
	return this->IsActive();
}

const char* Streams::GetProductName()
{	// Change this to your own product name. 
	return "ShredsNCharades Pravah";
}

void Streams::CableConnected(PatchPoint* pp)
{	frame.ch1.signal_in_connected = ppin1->num_cables>0;
	frame.ch1.level_cv_connected  = pplevel1->num_cables>0;
	frame.ch2.signal_in_connected = ppin2->num_cables>0;
	frame.ch2.level_cv_connected  = pplevel2->num_cables>0;
}

void Streams::CableDisconnected(PatchPoint* pp)
{
	CableConnected(NULL);
}

void Streams::UpdateMenus()
{	streams::UiSettings settings = engine.ui_settings();
	more_menu->getEntry(0)->setChecked(settings.linked);
	mch[0]->checkEntryAlone(GetChannelMode(0));
	mch[1]->checkEntryAlone(GetChannelMode(1));
	mmeter->checkEntryAlone(settings.monitor_mode);
}

CMouseEventResult Streams::onMouseDown(CPoint &where, const long& buttons)
{	//CMouseEventResult result;
	CPoint where2 (where);

	// If mouse down on the menu, then update the menu UI to reflect the current settings.
	where2.offset(-size.left,-size.top);
	if (more_menu->hitTest(where2, buttons))
		UpdateMenus();

	return Module::onMouseDown(where,buttons);
}

void Streams::ValueChanged(CControl* pControl)
{	int i,j;

	if (pControl == more_menu)
	{	if (more_menu->value == 0) 
			SetLinked(more_menu->getEntry(0)->isChecked());
	}
	else if (pControl == mmeter)
	{	i = mmeter->value; mmeter->checkEntryAlone(i);
		SetMonitorMode(i);
	}
	else 
	{	for (i=0; i<2; i++)
			if (pControl == mch[i])
			{	j = mch[i]->value; mch[i]->checkEntryAlone(j);
				SetChannelMode(i, j); return;
			}

		// If we reach here then it's not a menu control
		EnterProcessingCriticalSection();
		// Knobs
		frame.ch1.shape_knob = kshape1->value;
		frame.ch1.mod_knob = kmod1->value;
		frame.ch1.level_mod_knob = klevel1->value;
		frame.ch1.response_knob = kresp1->value;
		frame.ch2.shape_knob = kshape2->value;
		frame.ch2.mod_knob = kmod2->value;
		frame.ch2.level_mod_knob = klevel2->value;
		frame.ch2.response_knob = kresp2->value;

		// Buttons
		frame.ch1.function_button = bfunc1->value >= 0.5;
		frame.ch2.function_button = bfunc2->value >= 0.5;
		frame.metering_button = bmeter->value >= 0.5;

		bfunc1->value = 0; bfunc2->value = 0; bmeter->value = 0;

		// This will give an effect when one of the three buttons are UNPRESSED
		//if (!(frame.ch1.function_button))
		//	i = i;
		//UpdateMenus();

		LeaveProcessingCriticalSection();
	}
}

void Streams::SetSampleRate(float sr)
{	Module::SetSampleRate(sr);

	engine.SetSampleRate(sr);
}

int Streams::GetPresetSize()
{	return GetControlsValuesSize() + sizeof(streams::UiSettings) - 2*sizeof(uint8_t);  // -2*.... because padding[] is not saved
}

void Streams::SavePreset(void* pdata, int size)
{	streams::UiSettings settings = engine.ui_settings();

	char* pcdata = (char*)pdata;
	SaveControlsValues(pcdata);
	pcdata += GetControlsValuesSize();

	// Save none control data
	*(uint8_t*)pcdata = settings.function[0]; pcdata += sizeof(settings.function[0]);
	*(uint8_t*)pcdata = settings.function[1]; pcdata += sizeof(settings.function[1]);
	*(uint8_t*)pcdata = settings.alternate[0]; pcdata += sizeof(settings.alternate[0]);
	*(uint8_t*)pcdata = settings.alternate[1]; pcdata += sizeof(settings.alternate[1]);
	*(uint8_t*)pcdata = settings.monitor_mode; pcdata += sizeof(settings.monitor_mode);
	*(uint8_t*)pcdata = settings.linked; pcdata += sizeof(settings.linked);
}

void Streams::LoadPreset(void* pdata, int size, int version)
{	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();
	streams::UiSettings settings = {};

	LoadControlsValues(pcdata, csize);
	pcdata += csize;

	// Load none control data
	settings.function[0] = *(uint8_t*)pcdata; pcdata += sizeof(settings.function[0]);
	settings.function[1] = *(uint8_t*)pcdata; pcdata += sizeof(settings.function[1]);
	settings.alternate[0] = *(uint8_t*)pcdata; pcdata += sizeof(settings.alternate[0]);
	settings.alternate[1] = *(uint8_t*)pcdata; pcdata += sizeof(settings.alternate[1]);
	settings.monitor_mode = *(uint8_t*)pcdata; pcdata += sizeof(settings.monitor_mode);
	settings.linked = *(uint8_t*)pcdata; pcdata += sizeof(settings.linked);

	engine.ApplySettings(settings);
}

const char* Streams::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/streams/";
}

inline void Streams::ProcessSample()
{
	int i;

	//streams::StreamsEngine::Frame frame = {};
	// memset(&frame, 0, sizeof(frame));

	//frame.ch1.signal_in_connected = ppin1->num_cables>0;
	//frame.ch1.level_cv_connected  = pplevel1->num_cables>0;
	//frame.ch2.signal_in_connected = ppin2->num_cables>0;
	//frame.ch2.level_cv_connected  = pplevel2->num_cables>0;

	//frame.ch1.shape_knob          = kshape1->value;
	//frame.ch1.mod_knob            = kmod1->value;
	//frame.ch1.level_mod_knob      = klevel1->value;
	//frame.ch1.response_knob       = kresp1->value;
	//frame.ch2.shape_knob          = kshape2->value;
	//frame.ch2.mod_knob            = kmod2->value;
	//frame.ch2.level_mod_knob      = klevel2->value;
	//frame.ch2.response_knob       = kresp2->value;

	//// Buttons
	//frame.ch1.function_button     = bfunc1->value>=0.5;
	//frame.ch2.function_button     = bfunc2->value>=0.5;
	//frame.metering_button         = bmeter->value>=0.5;

	frame.ch1.excite_in = 5.f*ppexcite1->in;
	frame.ch1.signal_in = 5.f*ppin1->in;
	frame.ch1.level_cv  = 5.f*pplevel1->in;
	frame.ch2.excite_in = 5.f*ppexcite2->in;
	frame.ch2.signal_in = 5.f*ppin2->in;
	frame.ch2.level_cv  = 5.f*pplevel2->in;

	engine.Process(frame);

	ppout1->out = frame.ch1.signal_out / 5.f;
	ppout2->out = frame.ch2.signal_out / 5.f;

	if (frame.lights_updated) 
	{	
		for (i = 0; i < 4; i++)
		{	// Channel 1
			if (frame.ch1.led_green[i] > led_off && frame.ch1.led_red[i] <= led_off)
			{	// Green case
				lch1[i]->value = 0.3333f * frame.ch1.led_green[i];
			} else if (frame.ch1.led_red[i] > led_off && frame.ch1.led_green[i] <= led_off)
			{	// Red case
				lch1[i]->value = 1.0 - 0.15f * frame.ch1.led_red[i];
			} else if (frame.ch1.led_red[i] > led_off && frame.ch1.led_green[i] > led_off)
			{	// Yellow case
				lch1[i]->value = 0.3333f + 0.3333f * frame.ch1.led_green[i];
			} else lch1[i]->value = 0.f;
			if (lch1[i]->value > 1.f) lch1[i]->value = 1.f;

			// Channel 2
			if (frame.ch2.led_green[i] > led_off && frame.ch2.led_red[i] <= led_off)
			{	// Green case
				lch2[i]->value = 0.3333f * frame.ch2.led_green[i];
			} else if (frame.ch2.led_red[i] > led_off && frame.ch2.led_green[i] <= led_off)
			{	// Red case
				lch2[i]->value = 1.0 - 0.15f * frame.ch2.led_red[i];
			} else if (frame.ch2.led_red[i] > led_off && frame.ch2.led_green[i] > led_off)
			{	// Yellow case
				lch2[i]->value = 0.3333f + 0.3333f * frame.ch2.led_green[i];
			} else lch2[i]->value = 0.f;
			if (lch2[i]->value > 1.f) lch2[i]->value = 1.f;
		}

	}
}


//---------------------------------------------
// Shelves
CBitmap* Shelves::panel = NULL;
char* Shelves::name = "Almariyan";
int Shelves::name_len = 0;
char* Shelves::vendorname = "ShredsNCharades";
int Shelves::vendorname_len = 0;
Product* Shelves::pproduct = NULL;

const float Shelves::freqMin = std::log2(shelves::kFreqKnobMin);
const float Shelves::freqMax = std::log2(shelves::kFreqKnobMax);
const float Shelves::freqInit = (freqMin + freqMax) / 2;
const float Shelves::gainMin = -shelves::kGainKnobRange;
const float Shelves::gainMax = shelves::kGainKnobRange;
const float Shelves::qMin = std::log2(shelves::kQKnobMin);
const float Shelves::qMax = std::log2(shelves::kQKnobMax);
const float Shelves::qInit = (qMin + qMax) / 2;

Shelves::Shelves(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{
	int i;
	CRect r;

	// Create The Knobs
	khsfreq = AddModuleKnob(90.5, 49, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	khsgain = AddModuleKnob(145.5, 49, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	kp1freq = AddModuleKnob(90.5, 102, mcbits[knob_small_red], SNC_SMALL_KNOB_IMAGES, false, listener);
	kp1gain = AddModuleKnob(145.5, 102, mcbits[knob_small_red], SNC_SMALL_KNOB_IMAGES, false, listener);
	kp2freq = AddModuleKnob(90.5, 156, mcbits[knob_small_green], SNC_SMALL_KNOB_IMAGES, false, listener);
	kp2gain = AddModuleKnob(145.5, 156, mcbits[knob_small_green], SNC_SMALL_KNOB_IMAGES, false, listener);
	klsfreq = AddModuleKnob(90.5, 211, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	klsgain = AddModuleKnob(145.5, 211, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);

	kp1q = AddModuleKnob(43.6, 111.4, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kp2q = AddModuleKnob(43.6, 145.9, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kin = AddModuleKnob(68, 250.5, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);

	khsfreq->value = 0.5; khsgain->value = 0.5; kp1freq->value = 0.5; kp1gain->value = 0.5; 
	kp2freq->value = 0.5; kp2gain->value = 0.5; klsfreq->value = 0.5; klsgain->value = 0.5; 
	kp1q->value = 0.5; kp2q->value = 0.5; kin->value = 0.5;

	//// LEDs
	//for (i = 0; i < 4; i++)
	//{	lch1[i] = AddMovieBitmap(60, 49.9 + i * 12.9, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	//	lch2[i] = AddMovieBitmap(85.1, 49.9 + i * 12.9, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
	//}
	lclip = AddMovieBitmap(118.5, 248, mcbits[led_small_red], SNC_LED_RED_IMAGES, listener);

	// Buttons
	//bfunc1 = ModAddKickButtonEx(this, 59.7, 35, mcbits[black_butbit_tiny], 2, listener);

	// Switches (on/off button for now)
	//boffset = AddOnOffButton(24, 188, mcbits[black_switch], 2, listener, COnOffButton::kPostListenerUpdate);

	// Create Patch Points
	pphsfreq = AddPatchPoint(11.5, 38, ppTypeInput, ppbit, 0, listener);
	pphsgain = AddPatchPoint(42.5, 38, ppTypeInput, ppbit, 0, listener);
	ppp1freq = AddPatchPoint(11.5, 76, ppTypeInput, ppbit, 0, listener);
	ppp1gain = AddPatchPoint(42.5, 76, ppTypeInput, ppbit, 0, listener);
	ppp1q = AddPatchPoint(11.5, 108, ppTypeInput, ppbit, 0, listener);
	ppp2q = AddPatchPoint(11.5, 143, ppTypeInput, ppbit, 0, listener);
	ppp2freq = AddPatchPoint(11.5, 175.5, ppTypeInput, ppbit, 0, listener);
	ppp2gain = AddPatchPoint(42.5, 175.5, ppTypeInput, ppbit, 0, listener);
	pplsfreq = AddPatchPoint(11.5, 212.5, ppTypeInput, ppbit, 0, listener);
	pplsgain = AddPatchPoint(42.5, 212.5, ppTypeInput, ppbit, 0, listener);
	ppfreq = AddPatchPoint(11.5, 247, ppTypeInput, ppbit, 0, listener);
	ppgain = AddPatchPoint(42.5, 247, ppTypeInput, ppbit, 0, listener);
	ppin = AddPatchPoint(90, 247, ppTypeInput, ppbit, 0, listener);

	ppout = AddPatchPoint(144.5, 247, ppTypeOutput, ppbit, 0, listener);
	ppp1hp = AddPatchPoint(187, 38, ppTypeOutput, ppbit, 0, listener);
	ppp1bp = AddPatchPoint(187, 76, ppTypeOutput, ppbit, 0, listener);
	ppp1lp = AddPatchPoint(187, 108, ppTypeOutput, ppbit, 0, listener);
	ppp2hp = AddPatchPoint(187, 143, ppTypeOutput, ppbit, 0, listener);
	ppp2bp = AddPatchPoint(187, 175.5, ppTypeOutput, ppbit, 0, listener);
	ppp2lp = AddPatchPoint(187, 212.5, ppTypeOutput, ppbit, 0, listener);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	PutRightScrews(screw3, screw4, listener);
	InitPatchPoints(0.0);

	OnReset(); CableConnected(NULL); ValueChanged(NULL);
}

Shelves::~Shelves()
{
}

void Shelves::OnReset()
{	//preGain = false;
	frame.pre_gain = 0.f;
	SetSampleRate(sample_rate);
}


// SoloRack calls this. It can't directly access rhe constructor since this is a Dll and pointers to constructor can not be easily achieved.
Shelves* Shelves::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Shelves(pParent, listener, psynth_comm, vvoice);
}

void Shelves::Initialize()
{
	char* stemp;

	panel = new CBitmap(dllskindir, NULL, "Shelves.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Shelves::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Shelves::GetName()
{
	return name;
}

const int Shelves::GetNameLen()
{
	return name_len;
}

const char* Shelves::GetVendorName()
{
	return vendorname;
}

const int Shelves::GetVendorNameLen()
{
	return vendorname_len;
}

const int Shelves::GetType()
{
	return kFilter;
}

Product* Shelves::Activate(char* fullname, char* email, char* serial)
{
	return NULL;
}

bool Shelves::IsActive()
{
	return true;
}

Product* Shelves::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Shelves::InstanceIsActive()
{
	return this->IsActive();
}

const char* Shelves::GetProductName()
{	// Change this to your own product name. 
	return "ShredsNCharades Almariyan";
}

void Shelves::CableConnected(PatchPoint* pp)
{
	frame.hs_freq_cv_connected = pphsfreq->num_cables>0;
	frame.hs_gain_cv_connected = pphsgain->num_cables>0;
	frame.p1_freq_cv_connected = ppp1freq->num_cables>0;
	frame.p1_gain_cv_connected = ppp1gain->num_cables>0;
	frame.p1_q_cv_connected = ppp1q->num_cables>0;
	frame.p2_freq_cv_connected = ppp2freq->num_cables>0;
	frame.p2_gain_cv_connected = ppp2gain->num_cables>0;
	frame.p2_q_cv_connected = ppp2q->num_cables>0;
	frame.ls_freq_cv_connected = pplsfreq->num_cables>0;
	frame.ls_gain_cv_connected = pplsgain->num_cables>0;
	frame.global_freq_cv_connected = ppfreq->num_cables>0;
	frame.global_gain_cv_connected = ppgain->num_cables>0;

	frame.p1_hp_out_connected = ppp1hp->num_cables>0;
	frame.p1_bp_out_connected = ppp1bp->num_cables>0;
	frame.p1_lp_out_connected = ppp1lp->num_cables>0;
	frame.p2_hp_out_connected = ppp2hp->num_cables>0;
	frame.p2_bp_out_connected = ppp2bp->num_cables>0;
	frame.p2_lp_out_connected = ppp2lp->num_cables>0;
}

void Shelves::CableDisconnected(PatchPoint* pp)
{
	CableConnected(NULL);
}

//CMouseEventResult Shelves::onMouseDown(CPoint &where, const long& buttons)
//{	//CMouseEventResult result;
//	CPoint where2 (where);
//
//	// If mouse down on the menu, then update the menu UI to reflect the current settings.
//	where2.offset(-size.left,-size.top);
//	if (more_menu->hitTest(where2, buttons))
//		UpdateMenus();
//
//	return Module::onMouseDown(where,buttons);
//}

void Shelves::ValueChanged(CControl* pControl)
{	
	frame.pre_gain = kin->value;
	frame.hs_freq_knob = khsfreq->value;
	frame.p1_freq_knob = kp1freq->value;
	frame.p2_freq_knob = kp2freq->value;
	frame.ls_freq_knob = klsfreq->value;

	frame.hs_gain_knob = SCALE(khsgain->value, -1.f, 1.f);
	frame.p1_gain_knob = SCALE(kp1gain->value, -1.f, 1.f);
	frame.p2_gain_knob = SCALE(kp2gain->value, -1.f, 1.f);
	frame.ls_gain_knob = SCALE(klsgain->value, -1.f, 1.f);

	frame.p1_q_knob = kp1q->value;
	frame.p2_q_knob = kp2q->value;
}

void Shelves::SetSampleRate(float sr)
{	Module::SetSampleRate(sr);

	engine.setSampleRate(sr);
}

//int Shelves::GetPresetSize()
//{	return GetControlsValuesSize() + sizeof(Shelves::UiSettings) - 2*sizeof(uint8_t);  // -2*.... because padding[] is not saved
//}

//void Shelves::SavePreset(void* pdata, int size)
//{	Shelves::UiSettings settings = engine.ui_settings();
//
//	char* pcdata = (char*)pdata;
//	SaveControlsValues(pcdata);
//	pcdata += GetControlsValuesSize();
//
//	// Save none control data
//	//*(uint8_t*)pcdata = settings.function[0]; pcdata += sizeof(settings.function[0]);
//}

//void Shelves::LoadPreset(void* pdata, int size, int version)
//{	char* pcdata = (char*)pdata;
//	int csize = GetControlsValuesSize();
//	Shelves::UiSettings settings = {};
//
//	LoadControlsValues(pcdata, csize);
//	pcdata += csize;
//
//	// Load none control data
//}

const char* Shelves::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/shelves/";
}

inline void Shelves::ProcessSample()
{
	frame.clip = 0.f;
	frame.main_in = ppin->in*5.f; 
	frame.hs_freq_cv = pphsfreq->in*5.f;
	frame.hs_gain_cv = pphsgain->in*5.f;
	frame.p1_freq_cv = ppp1freq->in*5.f;
	frame.p1_gain_cv =  ppp1gain->in*5.f;
	frame.p1_q_cv =  ppp1q->in*5.f;
	frame.p2_freq_cv = ppp2freq->in*5.f;
	frame.p2_gain_cv = ppp2gain->in*5.f;
	frame.p2_q_cv = ppp2q->in*5.f;
	frame.ls_freq_cv = pplsfreq->in*5.f;
	frame.ls_gain_cv = pplsgain->in*5.f;
	frame.global_freq_cv = ppfreq->in*5.f;
	frame.global_gain_cv = ppgain->in*5.f;

	engine.process(frame);

	// Compiler will optimize all these /5.f divisions to multiplication by 0.2 It's just more readable this way.
	ppp1hp->out = frame.p1_hp_out/5.f;
	ppp1bp->out = frame.p1_bp_out/5.f;
	ppp1lp->out = frame.p1_lp_out/5.f;
	ppp2hp->out = frame.p2_hp_out/5.f;
	ppp2bp->out = frame.p2_bp_out/5.f;
	ppp2lp->out = frame.p2_lp_out/5.f;
	ppout->out = frame.main_out/5.f;

	//frame.clip = 0.f;
	//frame.main_in = ppin->in; 
	//frame.hs_freq_cv = pphsfreq->in;
	//frame.hs_gain_cv = pphsgain->in;
	//frame.p1_freq_cv = ppp1freq->in;
	//frame.p1_gain_cv =  ppp1gain->in;
	//frame.p1_q_cv =  ppp1q->in;
	//frame.p2_freq_cv = ppp2freq->in;
	//frame.p2_gain_cv = ppp2gain->in;
	//frame.p2_q_cv = ppp2q->in;
	//frame.ls_freq_cv = pplsfreq->in;
	//frame.ls_gain_cv = pplsgain->in;
	//frame.global_freq_cv = ppfreq->in;
	//frame.global_gain_cv = ppgain->in;

	//engine.process(frame);

	//// Compiler will optimize all these /5.f divisions to multiplication by 0.2 It's just more readable this way.
	//ppp1hp->out = frame.p1_hp_out;
	//ppp1bp->out = frame.p1_bp_out;
	//ppp1lp->out = frame.p1_lp_out;
	//ppp2hp->out = frame.p2_hp_out;
	//ppp2bp->out = frame.p2_bp_out;
	//ppp2lp->out = frame.p2_lp_out;
	//ppout->out = frame.main_out;

	lclip->value = frame.clip;
}




//---------------------------------------------
// Plaits
CBitmap* Plaits::panel = NULL;
char* Plaits::name = "Chotee";
int Plaits::name_len = 0;
char* Plaits::vendorname = "ShredsNCharades";
int Plaits::vendorname_len = 0;
Product* Plaits::pproduct = NULL;
const std::string Plaits::modelLabels[16] = 
{	"Pair of classic waveforms",
	"Waveshaping oscillator",
	"Two operator FM",
	"Granular formant oscillator",
	"Harmonic oscillator",
	"Wavetable oscillator",
	"Chords",
	"Vowel and speech synthesis",
	"Granular cloud",
	"Filtered noise",
	"Particle noise",
	"Inharmonic string modeling",
	"Modal resonator",
	"Analog bass drum",
	"Analog snare drum",
	"Analog hi-hat",
};

Plaits::Plaits(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
	: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()), pParent, panel, psynth_comm, vvoice)
{
	int i;
	CRect r;

	// Create The Knobs
	kfreq = AddModuleKnob(25, 75, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	kharm = AddModuleKnob(106.5, 75, mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES, false, listener);
	ktimb = AddModuleKnob(22, 133, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	klpg_color = AddModuleKnob(22, 133, mcbits[knob_small_green], SNC_SMALL_KNOB_IMAGES, false, listener); 
	kmorph = AddModuleKnob(109, 133, mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES, false, listener);
	klpg_decay = AddModuleKnob(109, 133, mcbits[knob_small_green], SNC_SMALL_KNOB_IMAGES, false, listener);
	ktimbcv = AddModuleKnob(22.4, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kfreqcv = AddModuleKnob(66, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);
	kmorphcv = AddModuleKnob(110, 186, mcbits[knob_tiny_black], SNC_TINY_KNOB_IMAGES, false, listener);

	klpg_color->setVisible(false); klpg_decay->setVisible(false);
	kfreq->value = 0.5; kharm->value = 0.5; ktimb->value = 0.5; kmorph->value = 0.5;
	klpg_color->value = 0.5; klpg_decay->value = 0.5;
	ktimbcv->value = 0.5; kfreqcv->value = 0.5; kmorphcv->value = 0.5; 

	//khsfreq->value = 0.5; khsgain->value = 0.5; kp1freq->value = 0.5; kp1gain->value = 0.5; 
	//kp2freq->value = 0.5; kp2gain->value = 0.5; klsfreq->value = 0.5; klsgain->value = 0.5; 
	//kp1q->value = 0.5; kp2q->value = 0.5; kin->value = 0.5;

	// LEDs
	for (i = 0; i < 8; i++)
	{	lmodel[i] = AddMovieBitmap(65.63, 56.6 + 12.25 * i, mcbits[led_small_gyr], SNC_LED_GYR_IMAGES, listener);
		//lmodel[i + 8]->setVisible(false);
		//lmodel[i] = AddMovieBitmap(65.63, 56.6 + 12.25 * i, mcbits[led_small_green], SNC_LED_GREEN_IMAGES, listener);
	}

	// Buttons
	bmodel1 = ModAddKickButtonEx(this, 55.5, 38, mcbits[black_butbit_tiny], 2, listener);
	bmodel2 = ModAddKickButtonEx(this, 75.5, 38, mcbits[black_butbit_tiny], 2, listener);

	// Switches (on/off button for now)
	//boffset = AddOnOffButton(24, 188, mcbits[black_switch], 2, listener, COnOffButton::kPostListenerUpdate);

	// Create Patch Points
	ppeng = AddPatchPoint(12.4, 218, ppTypeInput, ppbit, 0, listener);
	pptimb = AddPatchPoint(38, 218, ppTypeInput, ppbit, 0, listener);
	ppfreq = AddPatchPoint(65, 218, ppTypeInput, ppbit, 0, listener);
	ppmorph = AddPatchPoint(91, 218, ppTypeInput, ppbit, 0, listener);
	ppharm = AddPatchPoint(117, 218, ppTypeInput, ppbit, 0, listener);

	pptrig = AddPatchPoint(12.4, 251.5, ppTypeInput, ppbit, 0, listener);
	pplevel = AddPatchPoint(38, 251.5, ppTypeInput, ppbit, 0, listener);
	ppnote = AddPatchPoint(65, 251.5, ppTypeInput, ppbit, 0, listener);
	ppout = AddPatchPoint(91, 251.5, ppTypeOutput, ppbit, 0, listener);
	ppaux = AddPatchPoint(117, 251.5, ppTypeOutput, ppbit, 0, listener);

	// Create "More" menu
	r = CRect(CPoint(SkinScale(84), SkinScale(273)), CPoint(mcbits[black_butbit_tiny_up]->getWidth() - 1, mcbits[black_butbit_tiny_up]->getHeight() - 1));
	more_menu = new COptionMenu(r, listener, NOSAVE_TAG, mcbits[black_butbit_tiny_up], mcbits[black_butbit_tiny_down], kNoTextStyle | kMultipleCheckStyle);

	more_menu->addEntry("Disable resampling");
	more_menu->addEntry("Edit LPG response/decay");
	more_menu->addSeparator();
	more_menu->addEntry("-- Pitched models --");
	more_menu->getEntry(more_menu->getNbEntries()-1)->setEnabled(false);
	for (i = 0; i < 8; i++)
	{	more_menu->addEntry(modelLabels[i].c_str());
	}
	more_menu->addSeparator();
	more_menu->addEntry("-- Noise/percussive models --");
	more_menu->getEntry(more_menu->getNbEntries()-1)->setEnabled(false);
	for (i = 8; i < 16; i++)
	{	more_menu->addEntry(modelLabels[i].c_str());
	}
	addView(more_menu);

	// Put some screws
	PutLeftScrews(screw1, screw2, listener);
	//PutRightScrews(screw3, screw4, listener);
	InitPatchPoints(0.0);

	plvoice = NULL; outputBuffer = NULL;
	CableConnected(NULL); SetSampleRate(sample_rate);

	//stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
	//plvoice = new plaits::Voice();
	//plvoice->Init(&allocator, 48000.f);

	OnReset(); more_menu->value = kPairOfClassicWaveforms; ValueChanged(more_menu);
	//UpdateLights();
	//CableConnected(NULL); ValueChanged(NULL);
}

Plaits::~Plaits()
{	if (plvoice != NULL) delete plvoice; 
	plvoice = NULL;
	if (outputBuffer != NULL) delete outputBuffer;
	outputBuffer = NULL;
}

void Plaits::OnReset()
{	patch.engine = 0;
	patch.lpg_colour = 0.5f;
	patch.decay = 0.5f;
}


// SoloRack calls this. It can't directly access rhe constructor since this is a Dll and pointers to constructor can not be easily achieved.
Plaits* Plaits::Constructor(CFrame* pParent, CControlListener* listener, const SynthComm* psynth_comm, const int vvoice)
{
	return new Plaits(pParent, listener, psynth_comm, vvoice);
}

void Plaits::Initialize()
{
	char* stemp;

	panel = new CBitmap(dllskindir, NULL, "Plaits.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Plaits::End()
{
	panel->forget();
	if (pproduct != NULL) delete pproduct;
}

const char* Plaits::GetName()
{
	return name;
}

const int Plaits::GetNameLen()
{
	return name_len;
}

const char* Plaits::GetVendorName()
{
	return vendorname;
}

const int Plaits::GetVendorNameLen()
{
	return vendorname_len;
}

const int Plaits::GetType()
{
	return kOscillatorSource;
}

Product* Plaits::Activate(char* fullname, char* email, char* serial)
{
	return NULL;
}

bool Plaits::IsActive()
{
	return true;
}

Product* Plaits::InstanceActivate(char* fullname, char* email, char* serial)
{
	return this->Activate(fullname, email, serial);
}

bool Plaits::InstanceIsActive()
{
	return this->IsActive();
}

const char* Plaits::GetProductName()
{	// Change this to your own product name. 
	return "ShredsNCharades Chotee";
}

void Plaits::CableConnected(PatchPoint* pp)
{
	modulations.frequency_patched = ppfreq->num_cables>0;
	modulations.timbre_patched = pptimb->num_cables>0;
	modulations.morph_patched = ppmorph->num_cables>0;
	modulations.trigger_patched = pptrig->num_cables>0;
	modulations.level_patched = pplevel->num_cables>0;
}

void Plaits::CableDisconnected(PatchPoint* pp)
{
	CableConnected(NULL);
}

//CMouseEventResult Plaits::onMouseDown(CPoint &where, const long& buttons)
//{	//CMouseEventResult result;
//	CPoint where2 (where);
//
//	// If mouse down on the menu, then update the menu UI to reflect the current settings.
//	where2.offset(-size.left,-size.top);
//	if (more_menu->hitTest(where2, buttons))
//		UpdateMenus();
//
//	return Module::onMouseDown(where,buttons);
//}

void Plaits::UpdateLights()
{	int i;
	bool fl;

	for (i = 0; i < 8; i++) lmodel[i]->value = 0.f;
	
	if (patch.engine < 8)
		lmodel[patch.engine]->value = 0.3333f;
	else
		lmodel[patch.engine-8]->value = 0.8;

	for (i = 0; i < 8; i++) lmodel[i]->invalid();
}

void Plaits::SetLpgMode(bool mode) 
{
	kmorph->setVisible(!mode);
	ktimb->setVisible(!mode);
	klpg_decay->setVisible(mode);
	klpg_color->setVisible(mode);
	lpgMode = mode;
}

void Plaits::ValueChanged(CControl* pControl)
{
	int i,j;

	if (pControl == bmodel1 && bmodel1->value >= 0.5)
	{	if (patch.engine >= 8) patch.engine -= 8;
		else patch.engine = (patch.engine + 1) % 8;
		more_menu->value = kPairOfClassicWaveforms + patch.engine;
		ValueChanged(more_menu);
	}
	else if (pControl == bmodel2 && bmodel2->value >= 0.5)
	{	if (patch.engine < 8) patch.engine += 8;
		else patch.engine = (patch.engine + 1) % 8 + 8;
		more_menu->value = kGranularColor + patch.engine - 8;
		ValueChanged(more_menu);
	}
	else if (pControl == more_menu)
	{
		j = more_menu->value;
		if (j == kLowCPU)
		{	lowCpu = !lowCpu;
			more_menu->getEntry(kLowCPU)->setChecked(lowCpu);
			SetEngineSampleRate();
		}
		else if (j == kEditLPG)
		{	SetLpgMode(!lpgMode);
			more_menu->getEntry(kEditLPG)->setChecked(lpgMode);
		} 
		else if (j == kSeparator1 || j == kSeparator1Title || j == kSeparator2 || j==kSeparator2Title);		// highly unlikley, but just in case
		else
		{	for (i=kPairOfClassicWaveforms; i<=kAnalogHiHat; i++)
				more_menu->getEntry(i)->setChecked(false);
			more_menu->getEntry(j)->setChecked(true);
			if (j>=kPairOfClassicWaveforms && j < kSeparator2)
				patch.engine = j - kPairOfClassicWaveforms;
			else if (j >= kGranularColor && j<=kAnalogHiHat)
				patch.engine = j - kGranularColor + 8;
			UpdateLights();
		}
	}

}

void Plaits::SetEngineSampleRate()
{
	float sr;

	if (lowCpu)	sr = sample_rate;
	else sr = 48000.f;

	EnterProcessingCriticalSection();
	if (plvoice != NULL) delete plvoice; plvoice = NULL;
	stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
	plvoice = new plaits::Voice();
	plvoice->Init(&allocator,sr);
	LeaveProcessingCriticalSection();
}

void Plaits::SetSampleRate(float sr)
{
	int i;
	Module::SetSampleRate(sr);

	SetEngineSampleRate();
	//phase_rate = 2.f * PLAITS_BLOCKSIZE / sr;
	pitchadd = std::log2(48000.f / sr);
	outputSrc.setRates(48000, sample_rate);
	outputSrc.setChannels(2);
	
	if (outputBuffer != NULL) delete outputBuffer;
	i = (float)PLAITS_BLOCKSIZE * sample_rate / 48000.f + 4;		// + 4 for safty
	if (i < PLAITS_BLOCKSIZE) i = PLAITS_BLOCKSIZE;
	outputBuffer = new dsp::DynDoubleRingBuffer<dsp::Frame<2>>(i);
}

int Plaits::GetPresetSize()
{
	return GetControlsValuesSize() + sizeof(lowCpu) + sizeof(lpgMode) + sizeof(patch.engine);
}

void Plaits::SavePreset(void* pdata, int size)
{	char* pcdata = (char*)pdata;
	SaveControlsValues(pcdata);
	pcdata += GetControlsValuesSize();

	// Save none control data
	*(bool*) pcdata = lowCpu; pcdata += sizeof(lowCpu);
	*(bool*) pcdata = lpgMode; pcdata += sizeof(lpgMode);
	*(int*) pcdata = patch.engine; pcdata += sizeof(patch.engine);
}

void Plaits::LoadPreset(void* pdata, int size, int version)
{	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();

	LoadControlsValues(pcdata, csize);
	pcdata += csize;

	// Load none control data
	lowCpu = *(bool*) pcdata; pcdata += sizeof(lowCpu);
	lpgMode = *(bool*) pcdata; pcdata += sizeof(lpgMode);
	patch.engine = *(int*) pcdata; pcdata += sizeof(patch.engine);

	// Simulate clicking on the menu
	lowCpu = !lowCpu; more_menu->value = kLowCPU; ValueChanged(more_menu);
	lpgMode = !lpgMode; more_menu->value = kEditLPG; ValueChanged(more_menu);

	if (patch.engine < 8)
	{	more_menu->value = kPairOfClassicWaveforms + patch.engine;
		ValueChanged(more_menu);
	}
	else
	{	more_menu->value = kGranularColor + patch.engine - 8;
		ValueChanged(more_menu);
	}
}

const char* Plaits::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/plaits/";
}

inline void Plaits::ProcessSample()
{
	if (outputBuffer->empty()) 
	{	
		float pitch = SCALE(kfreq->value,-4.f,4.f);
		// Calculate pitch for lowCpu mode if needed
		//if (lowCpu) pitch += pitchadd;
		// Update patch
		patch.note = 60.f + pitch * 12.f;
		patch.harmonics = kharm->value;
		patch.timbre = ktimb->value;
		patch.morph = kmorph->value;
		patch.lpg_colour = klpg_color->value;
		patch.decay = klpg_decay->value;
		patch.frequency_modulation_amount = SCALE(kfreqcv->value,-1.f,1.f);
		patch.timbre_modulation_amount = SCALE(ktimbcv->value,-1.f,1.f);
		patch.morph_modulation_amount = SCALE(kmorphcv->value,-1.f,1.f);

		// Render output buffer for each voice
		// Construct modulations
		modulations.engine = ppeng->in;
		modulations.note = ppnote->in * MIDI_TUNE_FACTOR * 12.f;
		modulations.frequency = ppfreq->in * 5.f * 6.f;
		modulations.harmonics = ppharm->in;
		modulations.timbre = pptimb->in * 5.f / 8.f;
		modulations.morph = ppmorph->in * 5.f / 8.f;
		// Triggers at around 0.7 V
		modulations.trigger = pptrig->in * 5.f / 3.f;
		modulations.level = pplevel->in * 5.f / 8.f;

		// Render frames
		plvoice->Render(patch, modulations, output, PLAITS_BLOCKSIZE);

		// Convert output to frames
		for (int i = 0; i < PLAITS_BLOCKSIZE; i++) 
		{	outputFrames[i].samples[0] = output[i].out / 32768.f;
			outputFrames[i].samples[1] = output[i].aux / 32768.f;
		}

		// Convert output
		if (lowCpu) 
		{	int len = min((int) outputBuffer->capacity(), PLAITS_BLOCKSIZE);
			std::memcpy(outputBuffer->endData(), outputFrames, len * sizeof(outputFrames[0]));
			outputBuffer->endIncr(len);
		}
		else 
		{	int inLen = PLAITS_BLOCKSIZE;
			int outLen = outputBuffer->capacity();
			outputSrc.process(outputFrames, &inLen, outputBuffer->endData(), &outLen);
			outputBuffer->endIncr(outLen);
		}
	}

	// Set output
	if (!outputBuffer->empty()) 
	{	dsp::Frame<2> outputFrame = outputBuffer->shift();
		// Inverting op-amp on outputs
		ppout->out = -outputFrame.samples[0];
		ppaux->out = -outputFrame.samples[1];
	}
}