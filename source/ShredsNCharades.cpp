#include "ShredsNCharades.h"

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
	OnReset(); last_vu = 0.f;
	SetSampleRate(sample_rate);
}

Clouds::~Clouds()
{
	delete processor;
	delete[] block_mem;
	delete[] block_ccm;
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
	Module::SetSampleRate(sr);

	if (quality == 4) processor->set_sample_rate(sample_rate);
	else processor->set_sample_rate(32000.f);
	inputSrc.setRates(sample_rate, 32000);
	outputSrc.setRates(32000, sample_rate);
	//UpdateProcessor();
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
	if (!inputBuffer.full())
	{
		inputFrame.samples[0] = ppinl->in * kingain->value;
		inputFrame.samples[1] = ppinr->num_cables ? ppinr->in * kingain->value : inputFrame.samples[0];
		inputBuffer.push(inputFrame);
	}

	//if (freezeTrigger.process(bfreez->value)) freeze ^= true;
	//if (blendTrigger.process(bmode->value)) 
	//	blendMode = (blendMode + 1) % 4;

	// Trigger
	if (pptrig->in >= 1.0) triggered = true;

	// Render frames
	if (outputBuffer.empty())
	{
		clouds::ShortFrame input[32] = {};
		dsp::Frame<2> inputFrames[32];
		if (quality != 4)
		{	// Convert input buffer
			int inLen = inputBuffer.size();
			int outLen = 32;
			inputSrc.process(inputBuffer.startData(), &inLen, inputFrames, &outLen);
			inputBuffer.startIncr(inLen);

			// We might not fill all of the input buffer if there is a deficiency, but this cannot be avoided due to imprecisions between the input and output SRC.
			for (int i = 0; i < outLen; i++) {
				input[i].l = CLIP(inputFrames[i].samples[0] * 32767.0f, -32768.0f, 32767.0f);
				input[i].r = CLIP(inputFrames[i].samples[1] * 32767.0f, -32768.0f, 32767.0f);
			}
		}
		else
		{	// No sample rate convertion
			int blen = mmin(32, inputBuffer.size());
			for (int i = 0; i < blen; i++)
			{
				input[i].l = CLIP(inputBuffer.startData()[i].samples[0] * 32767.0f, -32768.0f, 32767.0f);
				input[i].r = CLIP(inputBuffer.startData()[i].samples[1] * 32767.0f, -32768.0f, 32767.0f);
			}
			inputBuffer.startIncr(blen);
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
			int outLen = outputBuffer.capacity();
			outputSrc.process(outputFrames, &inLen, outputBuffer.endData(), &outLen);
			outputBuffer.endIncr(outLen);
		}
		else
		{	// No sample rate convertion
			for (int i = 0; i < 32; i++)
			{
				outputBuffer.endData()[i].samples[0] = output[i].l / 32768.0;
				outputBuffer.endData()[i].samples[1] = output[i].r / 32768.0;
			}
			outputBuffer.endIncr(32);
		}
		triggered = false;
		outputFrame = outputBuffer.shift();

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
	else outputFrame = outputBuffer.shift();

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
{
	Module::SetSampleRate(sr);

	inputSrc.setRates(sample_rate, 48000);
	outputSrc.setRates(48000, sample_rate);

	// This is just a quick trick to allow it to work without sample rate convertion
	// it's not perfect. kSampleRate is const. It is a whole mess changing it to dynamic.
	if (low_cpu == 1) sr_notefix = log2(48000.f / sample_rate);
	else sr_notefix = 0.f;
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
	if (!inputBuffer.full()) {
		dsp::Frame<1> f;
		f.samples[0] = ppin->in;
		inputBuffer.push(f);
	}

	// Render frames
	if (outputBuffer.empty())
	{	//float in[24] = {};
		float* in;
		// Convert input buffer
		if (low_cpu == 0)
		{
			in = pin;
			int inLen = inputBuffer.size();
			int outLen = 24;
			inputSrc.process(inputBuffer.startData(), &inLen, (dsp::Frame<1>*) in, &outLen);
			inputBuffer.startIncr(inLen);
		}
		else
		{	// No sample rate convertion
			int blen = mmin(24, inputBuffer.size());
			//for (int i = 0; i < blen; i++)
			//	in[i] = inputBuffer.startData()[i].samples[0];
			inputBuffer.startIncr(blen);
			// This only works here because inputBuffer has 1 float per elemnt
			in = (float*)inputBuffer.startData();

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

		// Convert output buffer
		if (low_cpu == 0)
		{
			dsp::Frame<2> outputFrames[24];
			for (int i = 0; i < 24; i++)
			{
				outputFrames[i].samples[0] = out[i];
				outputFrames[i].samples[1] = aux[i];
			}
			int inLen = 24;
			int outLen = outputBuffer.capacity();
			outputSrc.process(outputFrames, &inLen, outputBuffer.endData(), &outLen);
			outputBuffer.endIncr(outLen);
		}
		else
		{	// No sample rate convertion
			for (int i = 0; i < 24; i++)
			{
				outputBuffer.endData()[i].samples[0] = out[i];
				outputBuffer.endData()[i].samples[1] = aux[i];
			}
			outputBuffer.endIncr(24);
		}
	}

	// Set output
	// if (!outputBuffer.empty()) 
	{	dsp::Frame<2> outputFrame = outputBuffer.shift();
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
	settings.signature = 0;
	SetSampleRate(sample_rate);
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
	Module::SetSampleRate(sr);

	src.setRates(96000, sample_rate);
#ifdef BRAIDS_SYNC
	sync_src.setRates(sample_rate, 96000);
#endif
	lowcpu_pitch_fix = log2(96000.f / sample_rate);
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
	if (outputBuffer.empty())
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
				outputBuffer.push(f);
			}
		}
		else {
			// Sample rate convert
			dsp::Frame<1> in[24];
			for (int i = 0; i < 24; i++) {
				in[i].samples[0] = render_buffer[i] / 32768.0;
			}

			int inLen = 24;
			int outLen = outputBuffer.capacity();
			src.process(in, &inLen, outputBuffer.endData(), &outLen);
			outputBuffer.endIncr(outLen);
		}
	}

	// Output
	//if (!outputBuffer.empty()) 
	{
		dsp::Frame<1> f = outputBuffer.shift();
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