/*	SoloRack SDK v0.11 Beta
	Copyright 2017-2022 Ammar Muqaddas
*/

#include <time.h>

#ifndef __Modules__
#include "Modules.h"
#endif

extern OSVERSIONINFOEX	gSystemVersion;			// This is the one in VSTGUI. 

// The folloiwng are just X,Y offsets that we use for pannels and positioning of controls. You can remove them if you don't need
#define FIRST_KY	55
#define FIRST_KY_DOWN	FIRST_KY+18
#define FIRST_SKY_DANGLING		FIRST_KY-11
#define FIRST_PPY	56
#define FIRST_PPY_DOWN	FIRST_PPY+17
#define FIRST_RIGHT_KX	62
#define FIRST_LEFT_KX	32
#define FIRST_LEFT_SKX	31
#define FIRST_LEFT_PPX	16
#define FIRST_RIGHT_PPX	72
#define FIRST_RIGHT_PPX2	63
#define XOFFSET 48
#define YOFFSET	43
#define YOFFSET_PP_SQWEEZE	32
#define YOFFSET_SK_SQWEEZE	32
#define PP_XOFFSET 31

// Some more constants
#define SCREW_X	9
#define SCREW_Y	9
#define NUM_PP_BITMAPS	1
#define NUM_SCREW_BITMAPS	10
#define MAIN_KNOB_IMAGES 49
#define MAIN_KNOB_BIG_IMAGES 99
#define SMALL_KNOB_IMAGES	31
#define TINY_KNOB_IMAGES	31
#define LED_BLUE_IMAGES	10
#define LED_RED_IMAGES	10
#define DIGITS_RED_IMAGES	10
#define DIGITS_RED_ALPHA_IMAGES	26
#define DIGITS_PLUS_MINUS_IMAGES	3
#define MIN_PW		0.01
#define RNG_PW		0.98
#define MAX_PW		(MIN_PW+RNG_PW)
#define HALF_PW		((MIN_PW+MAX_PW)/2)
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
#define DENORMAL			0.0000000000001
#undef NONE_CPP_DESTRUCTOR

// Shreds N Charades
#define SNC_MEDIUM_KNOB_IMAGES	69
#define SNC_SMALL_KNOB_IMAGES	59
#define SNC_TINY_KNOB_IMAGES	41
#define SNC_LED_GYR_IMAGES		75
#define SNC_LED_GREEN_IMAGES	25
#define SNC_LED_RED_IMAGES		25
#define SNC_LED_YELLOW_IMAGES	25


// Supports automatic 14bit/7bit CC changes regardless of the type of controller used.
#define CCHANGE(midi,msb,last_msb,lsb,conmsb,conlsb,val,drag_lsb,no_hold_lsb)		\
	/* MSB */								\
	case (char)conmsb:						\
		msb=midi.data2;						\
		/* Drag LSB If hi res 14bit MIDI is enabled. Maily to handle erratic jumping or delayed LSB */		\
		if (drag_lsb)									\
		{	if (msb>last_msb) lsb=(char)0;			\
			else if (msb<last_msb) lsb=(char)127;	\
			last_msb = msb;					\
		}									\
		temp=MSBLSB(msb,lsb);				\
		val=temp/16383.0;					\
		break;								\
	/* LSB */								\
	case (char)conlsb:						\
		lsb=midi.data2;						\
		if (no_hold_lsb) { temp=MSBLSB(msb,lsb); val=temp/16383.0;	}				\
		break;	


// Greatest Common Divider
int gcd(int a, int b) 
{	if (b == 0) return a; 
	return gcd(b, a % b);  
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

// MIDI Frequencies Table
float mfreqtab[128];

// for Park-Miller random generator
unsigned int zgen;

//Product Module::vproduct("M",0,NULL,true);

int IntPow(int base, int exp)
{	int i, out;
	
	out=1;
	for (i=1; i<=exp; i++) out*=base;
	return out;
}

int IntLog(int v, int base)
{	int i, out;

	out=0;
	while (v/=base) out++;
	return out;
}

// Stack definitions and implementation
template <typename TP>
void InitStack(cstack<TP> &stackv,int size)
{	stackv.top = -1;
	stackv.pbottom = (TP *) malloc(size*sizeof(*(stackv.pbottom)));
}

template <typename TP>
void FreeStack(cstack<TP> &stackv)
{	
	if (stackv.pbottom!=NULL) 
	{	free(stackv.pbottom); stackv.pbottom=NULL;
	}
}

#define PUSH(stackv,value) stackv.pbottom[++stackv.top]=value
#define POP(stackv) stackv.pbottom[stackv.top--]
#define FLUSHPOP(stackv) stackv.top--
#define NOTEMPTY(stackv) stackv.top!=-1
#define QISFULL(evin,evout,qsize)	(evin-evout==-1 || evin-evout==ev_qsize-1)


inline void ForceInside(long &x, long &y,const CRect &r,const CRect &rpar)
{	// rpar is rect of the container (parent)
	// Forces x,y of module to be inside container
	
	if (x<rpar.left) x=rpar.left;
	else if (x>rpar.right-r.width()) 
		x=rpar.right-r.width();

	if (y<rpar.top) y=rpar.top;
	else if (y>rpar.bottom-r.height()) 
		y=rpar.bottom-r.height();
}

CRect RectSkinFix(CRect r, float factor, float xfix=0.0, float yfix=0.0)
{	// Meant for old code. Scales a Rect to fit variable skin sizes
	//r.moveTo(FRound(factor*(float)r.x),FRound(factor*(float)r.y));
	r.moveTo(FRound(factor*((float)r.x+xfix+0.5*r.width())-0.5*r.width()),FRound(factor*((float)r.y+yfix+0.5*r.height())-0.5*r.height()));
	//r.setWidth(FRound(factor*(float)r.width()));
	//r.setHeight(FRound(factor*(float)r.height()));
	return r;
}

//---------------------------------------------
// Product Class

Product::Product(void *parameter, int opt, Product *pparent, bool isactive, char* prname) 
{	// Implement your own contructor here. The parameters above are just suggestions, you can remove them and do it your own way
	// parameter: a ppinter to something that is related/defines the module(s) that is being activated
	// pparent: a parent product for this product. If the parent is activated, then all children should become activated too.
	// isactive: indicates if the product should be active by default (like in freeware)
	// prname: Name of the product
	name=NULL;
	SetName(prname);
}

Product *Product::Activate(char *fullname, char *email, char *serial)
{	// This should try to activate the product using the given information.
	// If activation fails. It should try to activate the parent product (which will do the same recursivly)
	// If that fails, it should return NULL.
	// If activation is successful, then a pointer to the activated product should be returned.
	// A pointer to the product should NOT be returnred in ANY other situation, because an un-licensed caller could modify it.
	// Instead, if you want to make a copy of product. Implement something like CopyProduct()
	return NULL;
}

Product::~Product()
{	if (name!=NULL) free(name);
}

void Product::SetName(char* prname)
{	// Copy name
	if (prname!=NULL)
	{	if (prname!=name)
		{	name_len=strlen(prname);
			if (name!=NULL) free(name);
			name=(char *) malloc(sizeof(*name)*(name_len+1));
			strcpy(name,prname);
		}
	} 
	else 
	{	if (name!=NULL) free(name);
		name=NULL; name_len=0; 
	}
}


//PatchPointWrapper::PatchPointWrapper (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint& offset)
//: CMovieBitmap(size,listener,tag,background)
//{
//}
//
//PatchPointWrapper::~PatchPointWrapper()
//{
//}
//---------------------------------------------
// Patch Point Class
PatchPoint::PatchPoint(const CRect& size, CControlListener* listener, long tag, CBitmap* background, int pptype)
: CMovieBitmap(size,listener,tag,background)
{	type=pptype; active_type = ppTypeUnused; protocol=ppAudioCV;
	cable_in=NULL; pcable=NULL; coff_x = 0; coff_y = 0; pnext=this;
	num_cables=0; force_mono=false;
	peditor = (SynEditor *) listener;
}

PatchPoint::PatchPoint(const CRect& size, CControlListener* listener, long tag, CBitmap* background, int pptype, char x, char y)
: CMovieBitmap(size,listener,tag,background)
{	type=pptype; active_type = ppTypeUnused; protocol=ppAudioCV;
	cable_in=NULL; pcable=NULL; coff_x = x; coff_y = y; pnext=this;
	num_cables=0; force_mono=false;
	peditor = (SynEditor *) listener;
}

inline void PatchPoint::SetCenterOffset(char x, char y)
{	coff_x = x; coff_y = y;
}

inline void PatchPoint::SetType(int pptype)
{	type=pptype;
}

inline void PatchPoint::SetProtocol(int ppprotocol)
{	protocol=ppprotocol;
}

//inline void PatchPoint::SetPPTag(long tag)
//{	// This functoion is used exclusivly by SoloRack. It should not be called by modules
//	pptag = tag;
//}
//
//inline long PatchPoint::GetPPTag()
//{	return pptag;
//}

CMouseEventResult PatchPoint::onMouseDown (CPoint &where, const long& buttons)
{	// All this mess is for Dll modules. This also assumes a module is the direct parent of the patchpoint
	return ((Module *)getParentView())->synth_comm.PPOnMouseDownPtr(this,where,buttons);
}

CMouseEventResult PatchPoint::onMouseUp (CPoint &where, const long& buttons)
{	// All this mess is for Dll modules. This also assumes a module is the direct parent of the patchpoint
	return ((Module *)getParentView())->synth_comm.PPOnMouseUpPtr(this,where,buttons);
}

CMouseEventResult PatchPoint::onMouseMoved (CPoint &where, const long &buttons)
{	
	return kMouseEventNotHandled;
}

CMessageResult PatchPoint::notify(CBaseObject* sender, const char* message)
{	// Currently, only "Cable Draged In" message is sent here, so we will not check and assume it is the same message.
	// If you want to notify about any other message, you have to check then proccess and NOT call PPnotify if the message was not "Cable Draged In"
	return ((Module *)getParentView())->synth_comm.PPNotify(this,sender,message);
}



//---------------------------------------------
// Module Knob Class - for smooth tweaking
ModuleKnob::ModuleKnob(const CRect& size, CControlListener* listener, long tag, CBitmap* background, Module *parent, const CPoint &offset)
: CAnimKnob(size,listener,tag,background,offset)
, is_stepping(false)
{	svalue=qvalue=value;
	SetSmoothDelay(SMOOTH_DELAY,parent);
	setZoomFactor(5.0);
}

ModuleKnob::ModuleKnob (const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, Module *parent, const CPoint &offset)
: CAnimKnob(size,listener,tag,subPixmaps,heightOfOneImage,background,offset)
, is_stepping(false)
{	svalue=qvalue=value; 
	SetSmoothDelay(SMOOTH_DELAY,parent);
	setZoomFactor(5.0);
}

ModuleKnob::ModuleKnob (const ModuleKnob& v)
: CAnimKnob(v)
, is_stepping(false)
{	svalue=qvalue=value;
	SetSmoothDelay(SMOOTH_DELAY,NULL);
}

void ModuleKnob::UpdateQValue()
{	// Update Quantized value
	// This has to be called manually like in ValueChanged. Because VSTGUI does not call setValue but changes values directly.
	// I could have done it automatically, but I opted out for performance issues. I will leave to the developer

	float temp = subPixmaps - 1;
	if (bInverseBitmap)
		qvalue = ((float)((long) ((1-value) * temp)))/temp;
	else
		qvalue = ((float)((long) (value * temp)))/temp;

}

void ModuleKnob::setValue(float val)
{	CAnimKnob::setValue(val);
	if (is_stepping) UpdateQValue();
}

CMouseEventResult ModuleKnob::onMouseDown (CPoint& where, const long& buttons)
{	return ((Module *)getParentView())->synth_comm.ModuleKnobOnMouseDownPtr(this,where,buttons);
}

CMouseEventResult ModuleKnob::onMouseMoved(CPoint& where, const long& buttons)
{	CMouseEventResult result = CAnimKnob::onMouseMoved(where,buttons);
	
	//if ((buttons & kLButton) && is_stepping)
	//	UpdateQValue();
	return result;
}


void ModuleKnob::SetSmoothDelay(int del, Module *parent)
{	// getParentView() does not seam to work when the parent is still in it's constructor.
	// So you have to pass it as parent in this situation.
	
	if (del>=0) delay=del;

	if (parent==NULL) parent=(Module *) getParentView();
	//if (parent!=NULL)								//**
		smooth_samples=parent->sample_rate/1000.0*delay;
		//if (smooth_samples>30000) smooth_samples=30000;
	//else
	//	smooth_samples=Module/1000.0*delay;
}

//---------------------------------------------
// Extended Module Knob Class - Has a extra feature. Currently: a pointer to Attached control, and tag.
// License: Some functions in this class like onMouseMoved() and onMouseDown() are derived/modified from VSTGUI originals
// Therefore these functions are licensed under original VSTGUI license
ModuleKnobEx::ModuleKnobEx(const CRect& size, CControlListener* listener, long tag, CBitmap* background, Module *parent, const CPoint &offset)
	: ModuleKnob(size, listener, tag, background, parent, offset)
	, invalidate(true)
	, auto_zoom(false)
{
}

ModuleKnobEx::ModuleKnobEx(const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, Module *parent, const CPoint &offset)
	: ModuleKnob(size, listener, tag, subPixmaps, heightOfOneImage, background, parent, offset)
	, invalidate(true)
	, auto_zoom(false)
{
}

ModuleKnobEx::ModuleKnobEx(const ModuleKnobEx& v)
	: ModuleKnob(v)
{
	attached1 = v.attached1;
	invalidate = v.invalidate;
	auto_zoom = v.auto_zoom;
}

CMouseEventResult ModuleKnobEx::onMouseMoved(CPoint& where, const long& buttons)
{	// Same as in CKnob except that it can block invalidation
	// And can use auto_zoom which simulates pressing shift all the time while mooving the knob

	if (buttons & kLButton)
	{
		float middle = (vmax - vmin) * 0.5f;

		if (where != lastPoint)
		{
			lastPoint = where;
			if (modeLinear)
			{
				CCoord diff = (firstPoint.v - where.v) + (where.h - firstPoint.h);
				if (buttons != oldButton)
				{
					range = 400.f;			// Was 200
					if (auto_zoom)
					{	range *= zoomFactor;
						if (buttons & kShift) range *= 4.0;
					}
					else if (buttons & kShift) range *= zoomFactor;

					float coef2 = (vmax - vmin) / range;
					fEntryState += diff * (coef - coef2);
					coef = coef2;
					value = fEntryState + diff * coef;
					oldButton = buttons;
				}
				else value = fEntryState + diff * coef;
				bounceValue();
			}
			else
			{
				where.offset(-size.left, -size.top);
				value = valueFromPoint(where);
				if (startValue - value > middle)
					value = vmax;
				else if (value - startValue > middle)
					value = vmin;
				else
					startValue = value;
			}
			if (value != oldValue && listener)
				listener->valueChanged(this);
			if (invalidate && isDirty())
				invalid();
		}
	}
	return kMouseEventHandled;
}

CMouseEventResult ModuleKnobEx::onMouseDown(CPoint& where, const long& buttons)
{
	if (buttons & kRButton)
	{	// This will handle right click (copy/paste) menu
		return ModuleKnob::onMouseDown(where,buttons);
	}

	beginEdit();
	if (checkDefaultValue(buttons))
	{
		endEdit();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	firstPoint = where;
	lastPoint(-1, -1);
	startValue = oldValue;

	modeLinear = false;
	fEntryState = value;
	range = 400.f;					// Was 200
	coef = (vmax - vmin) / range;
	oldButton = buttons;

	long mode = kCircularMode;
	//long newMode = getFrame ()->getKnobMode ();	//Ammar to force kLinearMode
	long newMode = kLinearMode;						//Ammar to force kLinearMode
	if (kLinearMode == newMode)
	{
		if (!(buttons & kAlt))
			mode = newMode;
	}
	else if (buttons & kAlt)
		mode = kLinearMode;

	if (mode == kLinearMode && (buttons & kLButton))
	{
		if (auto_zoom)
		{	range *= zoomFactor;
			if (buttons & kShift) range *= 4.0;
		}
		else if (buttons & kShift) range *= zoomFactor;

		lastPoint = where;
		modeLinear = true;
		coef = (vmax - vmin) / range;
	}
	else
	{
		CPoint where2(where);
		where2.offset(-size.left, -size.top);
		startValue = valueFromPoint(where2);
	}

	return onMouseMoved(where, buttons);
}

//void ModuleKnobEx::setZoomFactor(float val)
//{
//	CPoint where;
//	long buttons;
//
//	zoomFactor = val; 
//	//where = lastPoint; lastPoint = CPoint(0, 0); buttons = oldButton; 
//	if (auto_zoom) oldButton = 0;
//	//onMouseMoved(where, buttons);
//}

// ---------------------------------------------------------------------
// Extended CSpecialDigit Class - Has a extra feature. Currently: a pointer to Attached control
CSpecialDigitEx::CSpecialDigitEx(const CRect& size, CControlListener* listener, long tag, long dwPos, long iNumbers, long* xpos, long* ypos, long width, long height, CBitmap* background, CBitmap* pblank)
: CSpecialDigit(size,listener,tag,dwPos,iNumbers,xpos,ypos,width,height,background)
{	attached1=NULL; tag1=-1;
	blank=pblank;
}

CSpecialDigitEx::CSpecialDigitEx(const CSpecialDigitEx& digit)
: CSpecialDigit(digit)
{	attached1 = digit.attached1;
	tag1=digit.tag1;
	//invalidate = digit.invalidate;
}

// License: This draw() function is derived/modified from the VSTGUI original
// Therefore the license for it is the original VSTGUI license
void CSpecialDigitEx::draw (CDrawContext *pContext)
{
	CPoint  where;
	CRect   rectDest;
	long    i, j;
	long    dwValue;
	long     one_digit[16];
	bool	leading_zero=true;
  
	if ((long)value >= getMax ()) 
		dwValue = (long)getMax ();
	else if ((long)value < getMin ()) 
		dwValue = (long)getMin ();
	else
		dwValue = (long)value;
	
	for (i = 0, j = ((long)getMax () + 1) / 10; i < iNumbers; i++, j /= 10)
	{
		one_digit[i] = dwValue / j;
		dwValue -= (one_digit[i] * j);
	}
	
	where.h = 0;
	for (i = 0; i < iNumbers; i++)
	{	
		j = one_digit[i];
		if (j > 9)
			j = 9;
		leading_zero = leading_zero && (j==0) && i<iNumbers-1;
		
		rectDest.left   = (CCoord)xpos[i];
		rectDest.top    = (CCoord)ypos[i];
		
		rectDest.right  = rectDest.left + width;
		rectDest.bottom = rectDest.top  + height;		
		
		// where = src from bitmap
		if (blank!=NULL && leading_zero)
		{	where.v = (CCoord)0;
			if (bTransparencyEnabled)
					blank->drawTransparent (pContext, rectDest, where);
				else
					blank->draw (pContext, rectDest, where);
		} else
		{	where.v = (CCoord)j * height;
			if (pBackground)
			{
				if (bTransparencyEnabled)
					pBackground->drawTransparent (pContext, rectDest, where);
				else
					pBackground->draw (pContext, rectDest, where);
			}
		}
	}
		
	setDirty (false);
}

//---------------------------------------------
// Extended CKickButtonEx Class - Has a extra features. Currently: a pointer to Attached control and new onMouseDown and onMouseUp to allow for valuechanged() to be called imidiatly
// License: Some functions in this class like onMouseUp() and onMouseDown() are derived/modified from VSTGUI originals
// Therefore these functions are licensed under the original VSTGUI license
CKickButtonEx::CKickButtonEx(const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint& offset)
: CKickButton(size,listener,tag,background,offset)
{	attached1=NULL; tag1=-1;
	immediate_valuechanged=true;
}

CKickButtonEx::CKickButtonEx(const CRect& size, CControlListener* listener, long tag, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset)
: CKickButton(size,listener,tag,heightOfOneImage,background,offset)
{	attached1=NULL; tag1=-1;
	immediate_valuechanged=true;
}

CKickButtonEx::CKickButtonEx(const CKickButtonEx& kickButton)
: CKickButton(kickButton)
{	attached1 = kickButton.attached1;
	tag1 = kickButton.tag1;
	immediate_valuechanged = kickButton.immediate_valuechanged;
}

CMouseEventResult CKickButtonEx::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
	beginEdit ();

	// Change value imidiatly instead of waiting for mousemove (Changed by Ammar)
	if (where.h >= size.left && where.v >= size.top  &&
			where.h <= size.right && where.v <= size.bottom)
			value = !fEntryState;
		else
			value = fEntryState;

	// This was in onMouseUp. This is the main defrence between CKickButton and CKickButtonEx. (Changed by Ammar)
	if (value && listener)
		listener->valueChanged (this);

	return onMouseMoved (where, buttons);
}

CMouseEventResult CKickButtonEx::onMouseUp (CPoint& where, const long& buttons)
{
	//if (value && listener)
	//	listener->valueChanged (this);
	value = 0.0f;  // set button to UNSELECTED state
	if (listener)
		listener->valueChanged (this);
	if (isDirty ())
		invalid ();
	endEdit ();
	return kMouseEventHandled;
}


//----------------------------------------------------------------------
// Extended CMovieBitmap. Mainly to block setDirty (false); when value is volatile.
CMovieBitmapEx::CMovieBitmapEx (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CPoint &offset)
: CMovieBitmap (size,listener,tag,background,offset)
{	
	#ifdef USE_NEW_OLD_VERTICAL
	old_vertical=-1000;
	#endif
	subPixmaps_1 = subPixmaps-1; update_count=update_ticks=5;
}

CMovieBitmapEx::CMovieBitmapEx (const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset)
: CMovieBitmap (size,listener,tag,subPixmaps,heightOfOneImage,background,offset)
, dirty_false_after_draw (true)
{	
	#ifdef USE_NEW_OLD_VERTICAL
	old_vertical=-1000;
	#endif
	subPixmaps_1 = subPixmaps-1; update_count=update_ticks=5;
}

CMovieBitmapEx::CMovieBitmapEx (const CMovieBitmapEx& v)
: CMovieBitmap (v)
{
	dirty_false_after_draw = v.dirty_false_after_draw;
	#ifdef USE_NEW_OLD_VERTICAL
	old_vertical=-1000; 
	#endif
	subPixmaps_1 = subPixmaps-1; update_count=update_ticks=5;
}

// License: This draw() function is derived/modified from the VSTGUI original
// Therefore the license for it is the original VSTGUI license
void CMovieBitmapEx::draw (CDrawContext *pContext)
{	// better for values that change slowly and smoothly in a none stepy fashion
	
	float temp;

	if (value > 1.0f)
		value = 1.0f;

	oldValue=value;

	if (pBackground)
	{	CPoint where (offset.h, offset.v);

 		if (oldValue > 0.0f)
		{	
			#ifdef USE_NEW_OLD_VERTICAL
				old_vertical = (int)(oldValue * subPixmaps_1 + 0.5);
				where.v += heightOfOneImage * old_vertical;
			#else
				where.v += heightOfOneImage * (int)(oldValue * subPixmaps_1 + 0.5);
			#endif
			
		}

		//if (bTransparencyEnabled)
		//	pBackground->drawTransparent (pContext, size, where);  // No need for that since both call alphablend
		//else
			pBackground->draw (pContext, size, where);
	}
	// If value changes rapidly in audio thread, then no use of setDirty (false)
	//if (dirty_false_after_draw) setDirty (false);
}

//void CMovieBitmapEx::setDirty (const bool val)
//{
//	CView::setDirty (val);
//	if (val)
//	{
//		if (value != -1.f)
//			oldValue = -1.f;
//		else
//			oldValue = 0.f;
//	}
//	else
//		oldValue = value;
//}
//


bool CMovieBitmapEx::isDirty () const
{	// Has tricks to lower CPU usage by limiting updating of LEDs and thus preventing updating of GDI+ CPU heavy cables
	return ((Module*)getParentView())->synth_comm.CMovieBitmapExIsDirtyPtr(this);
}



// -----------------------------
// The Critical section in the this class only protects against text[] resize/realloc
// Make sure you call module::EnterCriticalSection() and module::Leave.... where ever appropriate.
// From example if you AppendText() in audio thread and call FreeText() in GUI thread.
// You have to manually protect against those situation.
// I have performance reasons for this.
CTextLabelEx::CTextLabelEx(const CRect& size, const char* txt, CBitmap* background, const long style)
: CTextLabel(size,txt,background,style)
, line_offsets(NULL)
, line_offsets_size(0)	
, last_line(-1)
, cur_line(-1)
, max_draw_lines(-1)
, use_critsec(false)
, max_text_size(CTEXTLABEL_MAXTEXT_SIZE)
, max_chars_per_line(1000000)
{
}

CTextLabelEx::CTextLabelEx(const CTextLabelEx& v)
: CTextLabel(v)
, line_offsets(NULL)
, line_offsets_size(0)
, last_line(-1)
, cur_line(-1)
, max_draw_lines(-1)
, use_critsec(false)
, max_text_size(CTEXTLABEL_MAXTEXT_SIZE)
, max_chars_per_line(1000000)
{	
	max_text_size = v.max_text_size;
	max_chars_per_line = v.max_chars_per_line;
	if (v.line_offsets!=NULL) FindLineOffsets();
	if (v.use_critsec) InitCritSec();
}

CTextLabelEx::~CTextLabelEx()
{	freeText();
	DelCritSec();
}

void CTextLabelEx::InitCritSec()
{	InitializeCriticalSection(&critsec);
	use_critsec=true;
}

void CTextLabelEx::DelCritSec()
{	if (use_critsec) DeleteCriticalSection(&critsec);
	use_critsec = false;
}

void CTextLabelEx::EnterCritSec()
{	if (use_critsec) EnterCriticalSection(&critsec);
}

void CTextLabelEx::LeaveCritSec()
{	if (use_critsec) LeaveCriticalSection(&critsec);
}

void CTextLabelEx::setText(const char* txt)
{	if (use_critsec) EnterCriticalSection(&critsec);
	CTextLabel::setText(txt);
	if (use_critsec) LeaveCriticalSection(&critsec);
}

void CTextLabelEx::freeText()
{	
	if (use_critsec) EnterCriticalSection(&critsec);
	CTextLabel::freeText();
		
	if (line_offsets!=NULL) free(line_offsets); line_offsets=NULL;
	line_offsets_size=0; last_line=-1; cur_line=-1; max_draw_lines=-1;
	if (use_critsec) LeaveCriticalSection(&critsec);
}

// txt has to be allocated using malloc, reallloc, calloc, etc...
// This is NOT the best programming practice. But I use it for performance in some cases.
void CTextLabelEx::SetTextPointer(const char* txt, int arsize)
{	
	if (use_critsec) EnterCriticalSection(&critsec);
	freeText();
	text = (char *) txt; text[arsize-1]='\0';
	text_offset=0; text_arsize = arsize; text_len = strlen(text);
	if (use_critsec) LeaveCriticalSection(&critsec);
	setDirty(true);
}

// By Ammar
bool CTextLabelEx::AppendText(const char* txt, bool ignore_limit)				
{	int i, len1;

	if (txt==NULL) return false;
	//if (use_critsec) EnterCriticalSection(&critsec);
	if (text==NULL) setText(txt);
	else
	{	i = text_len; len1 = text_len+strlen(txt)+1; 
		if (len1>text_arsize)
		{	if (len1<=max_text_size)
			{	if (use_critsec) EnterCriticalSection(&critsec);
				text_arsize = len1*2;			// Double the required size to avoid reallocation too many times
				if (text_arsize>=max_text_size) text_arsize=max_text_size; 
				text = (char *) realloc(text,text_arsize*sizeof(*text));
				text[text_arsize-1]='\0';
				if (use_critsec) LeaveCriticalSection(&critsec);
			}
			else if (ignore_limit)
			{	if (use_critsec) EnterCriticalSection(&critsec);
				text_arsize = len1;
				text = (char *) realloc(text,text_arsize*sizeof(*text));
				text[text_arsize-1]='\0';
				if (use_critsec) LeaveCriticalSection(&critsec);
			}
			else { /*if (use_critsec) LeaveCriticalSection(&critsec);*/ return false; }
		}
		// I know it's safer to have the following inside the critical section but since AppendText()
		// could be called at audio rate. I decided not to do it this way for performance.
		// I handle this manually using the Module critical section if freeText() or setText() is called from GUI thread.
		strcat(text+i,txt); text_len=len1-1;		

		if (line_offsets!=NULL) FindLineOffsets(i);
		setDirty(true);
	}
	//if (use_critsec) LeaveCriticalSection(&critsec);
	return true;
}

void CTextLabelEx::FindLineOffsets()
{	// Subsequent calls to AppendText()
	// will automatically maintain the line offsets correctly.
	int i;

	if (use_critsec) EnterCriticalSection(&critsec);
	if (text==NULL) return;
	if (line_offsets!=NULL) free(line_offsets);
	line_offsets_size=20; 
	line_offsets = (int *) malloc(line_offsets_size*sizeof(*line_offsets));
	line_offsets[0]=0; last_line=0; cur_line=0;
	FindLineOffsets(0);
	if (use_critsec) LeaveCriticalSection(&critsec);
}

void CTextLabelEx::FindLineOffsets(int start)
{	int i,linelen, lastspace;

	lastspace=-1; linelen=0; i=start; 
	// Discover current running line length where at without the newly appended charaters
	while (i>=0 && text[i]!='\n') { i--; linelen++; }
	i=start;
	while (text[i]!='\0')
	{	if (text[i]==' ') lastspace=i;
		if (text[i]=='\n') 
		{	
			line_found:
			// adjust size if not enough
			if (last_line+1>=line_offsets_size)
			{	if (use_critsec) EnterCriticalSection(&critsec);
				line_offsets_size = (last_line+1)*2;
				line_offsets = (int *) realloc(line_offsets,line_offsets_size*sizeof(*line_offsets));
				if (use_critsec) LeaveCriticalSection(&critsec);
			}

			i++; if (text[i]=='\r') i++;	// skip carriage  return
			//if (text[i]=='\0') break;
			line_offsets[++last_line] = i; linelen=0;
		}
		else if (linelen>max_chars_per_line && lastspace>line_offsets[last_line])
		{	// Put a new line to word wrap
			text[lastspace] = '\n'; i=lastspace; lastspace=-1; goto line_found;
		}
		else { i++; linelen++; }
	}
}

void CTextLabelEx::SetCurrentLine(int line)
{	//if (use_critsec) EnterCriticalSection(&critsec);
	if (line>=0 && line<=last_line)
	{	cur_line = line;
		SetTextOffset(line_offsets[line]);
	}
	//if (use_critsec) LeaveCriticalSection(&critsec);
}

void CTextLabelEx::draw(CDrawContext *pContext)
{	int i,j;
	char ch; 

	// Ammar: Trick to stop drawing string out of control bounds (ClipRect is not enough in this case) by temporarly inserting null character.
	// At the end of a line. Not the cleanest nor thread safest way, but works fast
	// The proper other alternative is to calcualte the actual size the text takes on screen and limit drawing to that. Probably not CPU lite.
	if (use_critsec) EnterCriticalSection(&critsec);
	if (line_offsets!=NULL && max_draw_lines>0) 
	{	i = cur_line+max_draw_lines;
		if (i<last_line) { j=line_offsets[i]-1; ch=text[j]; text[j]='\0'; } else ch='\0';
		
		if (text_offset<=0)	
			drawText(pContext, text);
		else drawText(pContext, &text[text_offset]);

		if (ch!='\0') text[j]=ch;
	}
	else 
	{	// Just draw the whole string
		if (text_offset<=0)	
			drawText(pContext, text);
		else drawText(pContext, &text[text_offset]);
	}
	if (use_critsec) LeaveCriticalSection(&critsec);
	setDirty (false);
}


//---------------------------------------------
// Base Module Class
CBitmap **Module::mcbits = NULL;			// Bitmap(s) of the main patchpoints
CBitmap **Module::ppbit = NULL;				// Bitmap(s) of the main patchpoints
char *Module::skindir = NULL;
char *Module::defskindir = NULL;
char *Module::dlldatadir = NULL;
char *Module::dllskindir = NULL;
char *Module::datadir = NULL;
char *Module::plugindir = NULL;
float Module::uiscale = 1.0;
long Module::vp = BASE_MHEIGHT;
long Module::vp_3 = BASE_MHEIGHT/3;
long Module::vp_5 = BASE_MHEIGHT/5;
long Module::hp = BASE_HP;
CColor Module::digit_color = CColor();

int Module::GetSDKVersion() 
{	return SDK_VERSION; 
}

Module::Module(const CRect &size, CFrame *pParent, CBitmap *pBackground, const SynthComm *psynth_comm, const int vvoice)
: ModuleWrapper(size,pParent,pBackground) 
{	synth_comm=*psynth_comm;
	in_move = false; index=-1; evindex=-1; sbindex=-1; mouseobindex=-1; procindex=-1; orphindex=-1;
	voice=vvoice; is_mono=false; enable_is_mono_menu=true;
	nbcontrols=-1; nb_pp=-1; nb_force_mono=0; allow_oversample=true; enable_allow_oversample=true;
	nb_cables=0; always_on=GetAlwaysONDefault();

	//peditor = (SynEditor *) pParent->getEditor();
	peditor = synth_comm.GetEditor();
	psynth = synth_comm.GetSynth(peditor);
	DAW_block_size = synth_comm.GetDAWBlockSize(peditor);
	DAW_sample_rate = synth_comm.GetDAWSampleRate(peditor);

	#ifdef MODULE_OVERSAMPLE
	// Oversampling settings for per mdoule oversampling. Not complete yet.
	ovr.overs=1; ovr.sovers=1; ovr.overs_index=0; ovr.overs_filter=peditor->moversfilter_menu_index[kIIRChamberlin];
	ovr.ovin=NULL; ovr.cof=NULL; ovr.cofs=0; ovr.iovin=0;
	ovr.bp=0; ovr.bp_2=0; ovr.lp=0; ovr.lp_2=0;
	sample_rate = DAW_sample_rate*psynth->sovers*ovr.overs;
	#else
	//sample_rate = DAW_sample_rate*psynth->sovers;
	sample_rate = DAW_sample_rate*psynth_comm->GetOversamplingFactor(peditor);
	#endif
	
	//sample_rate = DAW_sample_rate*psynth->sovers*ovr.overs;
	hsample_rate = sample_rate/2.0;
	sample_rate60 = 60.0*sample_rate;

	// Default to NO Bandlimiting
	Module::SetBandLimit(kNoBandlimiting);
	demolabel=NULL; infourl=NULL;
	clip_level = DEFAULT_MAX_LEVEL;
}

Module::~Module()
{	
	#ifdef MODULE_OVERSAMPLE
	free(ovr.ovin); free(ovr.cof);
	#endif
	// removeAll() in CViewContiner will remove and delete all added views from memory
	free(infourl);
}

// Will be called only once by DllInitialize(). Don't call this in a derived module Initialize()
void Module::Initialize()
{	int i;
	char temp[40];
	char temp2[2];
	char *skin_config, *defskin_config;

	SetSeed(time(NULL));

	// Create cbits array
	mcbits = (CBitmap **) malloc(kModuleCBitsCount*sizeof(**mcbits));

	// Main Knob bitmaps
	mcbits[knobit] = new CBitmap(dllskindir,NULL,"main_knob_s.png");
	mcbits[sknobit_white] = new CBitmap(dllskindir,NULL,"sknobit_white.png");
	mcbits[tknobit_black] = new CBitmap(dllskindir, NULL, "tknobit_black.png");

	// Switch bitmaps
	mcbits[vert_swbit] = new CBitmap(skindir,defskindir,"vert_switch_toggle.png");
	mcbits[tr_vert_swbit] = new CBitmap(skindir,defskindir,"vert_triple_switch_toggle.png");

	// Prepare screws bitmap
	mcbits[scrbit] = new CBitmap(skindir,defskindir,"screw.png");

	// MIDI patch point
	mcbits[MIDIppbit] = new CBitmap(skindir,defskindir,"MIDIpp.png");
	//MIDIplugbit = new CBitmap(skindir,defskindir,"MIDIplug.png");
	//MIDIplugconbit = new CBitmap(skindir,defskindir,"MIDIplugcon.png");

	// Prepare patchpoints bitmaps
	ppbit = (CBitmap **) malloc(sizeof(*ppbit)*NUM_PP_BITMAPS);
	for (i=1; i<=NUM_PP_BITMAPS; i++)
	{	strcpy(temp,"patchpoint");
		_itoa(i,temp2,10); strcat(temp,temp2);
		strcat(temp,".png");
		ppbit[i-1] = new CBitmap(dllskindir,defskindir,temp);
	}

	// leds bitmaps
	mcbits[led_blue] = new CBitmap(dllskindir,defskindir,"led_blue.png");
	mcbits[led_red] = new CBitmap(dllskindir,defskindir,"led_red.png");

	
	// small knobs
	mcbits[sknobit_black5] = new CBitmap(dllskindir,NULL,"sknobit_black_5s.png");

	// Buttons
	mcbits[white_buttonbit] = new CBitmap(dllskindir,defskindir,"white_button_small.png");
	mcbits[black_butbit_tiny] = new CBitmap(dllskindir,defskindir,"black_button_tiny.png");

	// Shreds N Charades
	mcbits[knob_medium_white] = new CBitmap(dllskindir, NULL, "knob_medium_white.png");
	mcbits[knob_medium_green] = new CBitmap(dllskindir, NULL, "knob_medium_green.png");
	mcbits[knob_medium_red] = new CBitmap(dllskindir, NULL, "knob_medium_red.png");
	mcbits[knob_small_white] = new CBitmap(dllskindir, NULL, "knob_small_white.png");
	mcbits[knob_small_green] = new CBitmap(dllskindir, NULL, "knob_small_green.png");
	mcbits[knob_small_red] = new CBitmap(dllskindir, NULL, "knob_small_red.png");
	mcbits[knob_small_black] = new CBitmap(dllskindir, NULL, "knob_small_black.png");
	mcbits[knob_tiny_black] = new CBitmap(dllskindir, NULL, "knob_tiny_black.png");


	mcbits[led_big_green] = new CBitmap(dllskindir, NULL, "led_big_green.png");
	mcbits[led_big_red] = new CBitmap(dllskindir, NULL, "led_big_red.png");
	mcbits[led_big_yellow] = new CBitmap(dllskindir, NULL, "led_big_yellow.png");
	mcbits[led_big_gyr] = new CBitmap(dllskindir, NULL, "led_big_gyr.png");

	mcbits[led_small_green] = new CBitmap(dllskindir, NULL, "led_small_green.png");
	mcbits[led_small_red] = new CBitmap(dllskindir, NULL, "led_small_red.png");
	mcbits[led_small_yellow] = new CBitmap(dllskindir, NULL, "led_small_yellow.png");
	mcbits[led_small_gyr] = new CBitmap(dllskindir, NULL, "led_small_gyr.png");

	mcbits[button_led] = new CBitmap(dllskindir, NULL, "button_led.png");
	mcbits[button_big] = new CBitmap(dllskindir, NULL, "button_big.png");

	// Make skin config file
	skin_config = (char *) malloc((strlen(skindir)+21)*sizeof(*skin_config));
	defskin_config = (char *) malloc((strlen(defskindir)+21)*sizeof(*defskin_config));
	strcpy(skin_config,skindir); strcat(skin_config,"skin_config.ini");
	strcpy(defskin_config,defskindir); strcat(defskin_config,"skin_config.ini");

	// Skin attributes
	i = GetPrivateProfileIntA("Other","digit_red",MAX_INT,skin_config);
	if (i==MAX_INT) digit_color.red = GetPrivateProfileIntA("Other","digit_red",245,defskin_config);
	else digit_color.red=i;

	i = GetPrivateProfileIntA("Other","digit_blue",MAX_INT,skin_config);
	if (i==MAX_INT) digit_color.blue = GetPrivateProfileIntA("Other","digit_blue",0,defskin_config);
	else digit_color.blue=i;

	i = GetPrivateProfileIntA("Other","digit_green",MAX_INT,skin_config);
	if (i==MAX_INT) digit_color.green = GetPrivateProfileIntA("Other","digit_green",132,defskin_config);
	else digit_color.green=i;

	i = GetPrivateProfileIntA("Other","digit_alpha",MAX_INT,skin_config);
	if (i==MAX_INT) digit_color.alpha = GetPrivateProfileIntA("Other","digit_alpha",255,defskin_config);
	else digit_color.alpha=i;

	free(skin_config); free(defskin_config);

	// make frequency (Hz) table
	double k = 1.059463094359;	// 12th root of 2
	double a = 6.875;	// a
	a *= k;	// b
	a *= k;	// bb
	a *= k;	// c, frequency of midi note 0. (note: It's C-1, C at octave -1)

	for (i = 0; i < 128; i++)	// 128 midi notes
	{
		mfreqtab[i] = (float)a;
		a *= k;
	}

	// Get OS version, Required by VST gui. This would have been done if CFrame() was called
	memset(&gSystemVersion, 0, sizeof (gSystemVersion));
	gSystemVersion.dwOSVersionInfoSize = sizeof (gSystemVersion);
	GetVersionEx((OSVERSIONINFO *)&gSystemVersion);

}

void Module::End()
{	// Cleaning
	int i;

	for (i=0; i<NUM_PP_BITMAPS; i++)
		ppbit[i]->forget();
	free(ppbit);

	// Free common bitmaps
	for (i=0; i<kModuleCBitsCount; i++)
		mcbits[i]->forget();

	free(mcbits);

}

CMouseEventResult Module::onMouseDown(CPoint &where, const long& buttons)
{	// all this mess is for Dll modules
	return synth_comm.ModuleOnMouseDownPtr(this,where,buttons);
}

CMouseEventResult Module::onMouseUp(CPoint &where, const long& buttons)
{	// all this mess is for Dll modules
	return synth_comm.ModuleOnMouseUpPtr(this,where,buttons);
}

CMouseEventResult Module::onMouseMoved(CPoint &where, const long& buttons)
{	// all this mess is for Dll modules
	return synth_comm.ModuleOnMouseMovedPtr(this,where,buttons);
}

CMouseEventResult Module::OnMouseMovedObserve(CPoint &where, const long& buttons)
{	// This is NOT from VSTGUI.
	// Do nothing by default. This should be overiden by modules that call CallMouseObserve()
	// SoloRack will keep calling this for any mouse movements no matter where they are located, even
	// if they are far off this modules boundaries. This is usefull for example for a theremin module or such things.
	return kMouseEventNotHandled;
}


void Module::PutLeftScrews(CMovieBitmap *&top_screw,CMovieBitmap *&bottom_screw, CControlListener *listener)
{	float j;
	CRect r;
	
	// Top left screw
	j=((float) GenRand(0,NUM_SCREW_BITMAPS-1))/(NUM_SCREW_BITMAPS-1.0);
	r.moveTo(this->getViewSize().getTopLeft().x+SkinScale(SCREW_X)-mcbits[scrbit]->getWidth()/2,this->getViewSize().getTopLeft().y+SkinScale(SCREW_Y)-mcbits[scrbit]->getHeight()/NUM_SCREW_BITMAPS/2);
	r.setSize(CPoint(mcbits[scrbit]->getWidth(),mcbits[scrbit]->getHeight()/NUM_SCREW_BITMAPS));
	top_screw = new CMovieBitmap(r,listener,NO_TAG,mcbits[scrbit]); addView(top_screw);
	top_screw->setValue(j);
	
	// Bottom left screw
	j=((float) GenRand(0,NUM_SCREW_BITMAPS-1))/(NUM_SCREW_BITMAPS-1.0);
	r.moveTo(this->getViewSize().getBottomLeft().x+SkinScale(SCREW_X)-mcbits[scrbit]->getWidth()/2,this->getViewSize().getBottomLeft().y-SkinScale(SCREW_Y)-mcbits[scrbit]->getHeight()/NUM_SCREW_BITMAPS/2);
	r.setSize(CPoint(mcbits[scrbit]->getWidth(),mcbits[scrbit]->getHeight()/NUM_SCREW_BITMAPS));
	bottom_screw = new CMovieBitmap(r,listener,NO_TAG,mcbits[scrbit]); addView(bottom_screw);
	bottom_screw->setValue(j);
}

void Module::PutRightScrews(CMovieBitmap *&top_screw,CMovieBitmap *&bottom_screw, CControlListener *listener)
{	float j;
	CRect r;
	
	// Top right Screw
	j=((float) GenRand(0,NUM_SCREW_BITMAPS-1))/(NUM_SCREW_BITMAPS-1.0);
	r.moveTo(this->getViewSize().getTopRight().x-SkinScale(SCREW_X)-mcbits[scrbit]->getWidth()/2,this->getViewSize().getTopRight().y+SkinScale(SCREW_Y)-mcbits[scrbit]->getHeight()/NUM_SCREW_BITMAPS/2);
	r.setSize(CPoint(mcbits[scrbit]->getWidth(),mcbits[scrbit]->getHeight()/NUM_SCREW_BITMAPS));
	top_screw = new CMovieBitmap(r,listener,NO_TAG,mcbits[scrbit]); addView(top_screw);
	top_screw->setValue(j);
	
	// Bottom right Screw
	j=((float) GenRand(0,NUM_SCREW_BITMAPS-1))/(NUM_SCREW_BITMAPS-1.0);
	r.moveTo(this->getViewSize().getBottomRight().x-SkinScale(SCREW_X)-mcbits[scrbit]->getWidth()/2,this->getViewSize().getBottomRight().y-SkinScale(SCREW_Y)-mcbits[scrbit]->getHeight()/NUM_SCREW_BITMAPS/2);
	r.setSize(CPoint(mcbits[scrbit]->getWidth(),mcbits[scrbit]->getHeight()/NUM_SCREW_BITMAPS));
	bottom_screw = new CMovieBitmap(r,listener,NO_TAG,mcbits[scrbit]); addView(bottom_screw);
	bottom_screw->setValue(j);
}

long Module::GetFreeTag()
{	// Returns an unused tag for DAW automation. Should be called by the module, typicaly in the construtor.
	return synth_comm.ModuleGetFreeTagPtr(this);
}

bool Module::RegisterTag(CControl *pcon, long tag)
{	// Associates the given tag with the given control for DAW automation. Should be called by the module, typicaly in the construtor.
	return synth_comm.ModuleRegisterTagPtr(this,pcon,tag);
}

bool Module::UnRegisterTag(CControl *pcon)
{	// Unassociates the given tag with the given control for DAW automation. Should be called by the module
	return synth_comm.ModuleUnRegisterTagPtr(this,pcon);
}

bool Module::CallProcessEvents()
{	// Tells SoloRack to call ProcessEvents for this module, ProcessEvents() is called when ever a DAW event is sent to SoloRack. Should be called by the module, typicaly in the construtor.
	return synth_comm.ModuleCallProcessEventsPtr(this);
}

bool Module::CallStartOfBlock()
{	// Tells SoloRack to call StartOfBlock for this module every time processReplacing() is called in SoloRack. 
	return synth_comm.ModuleCallStartOfBlockPtr(this);
}

bool Module::CallMouseObserve()
{	// Tells SoloRack to call OnMouseMovedObserve() for this module every time the mouse changes position/moves no matter where the mouse is
	return synth_comm.ModuleCallMouseObservePtr(this);
}

bool Module::ClearIfOrphaned()
{	// Should only be called in the destructor or just before it.
	// In very odd cases, you may not want to imidiatly destruct your module after it's been deleted.
	// For example, with modules that connect to the network or have worker thread actively working.
	// This can be done calling remmember(), then calling forget() when work is finished and you are ready to destruct.
	// However, during this "hanging there period", SoloRack will put the module into an "orphaned" list that will be forced to destruct at synth exit.
	// This function tells SoloRack that this module has gracefully been detructed. So if it's in the orphaned list. SoloRack will remove it and not try to destruct it on synth exit
	// You HAVE TO call this function if you automatically destruct your orphaned modules later. Otherwise SoloRack WILL CRASH on synth exit
	// Because it will try to decontruct your modules which doesn't exist any more in memory.
	return synth_comm.ClearIfOrphaned(this);
}



bool Module::UnRegisterOldTag(CControl *pcon,long oldtag, long newtag)
{	// This function safely unregisters an old tag if a new tag for the same control has been registered (before calling this function).
	// This is mainly used when loading presets.
	
	return synth_comm.ModuleUnRegisterOldTagPtr(this,pcon,oldtag,newtag);
}



void Module::ProcessEvents(const EventsHandle ev)
{
}

inline void Module::StartOfBlock(int sample_frames)	
{	// Called by SoloRack on the start of each block, for all modules that called CallProcessEvents()
}


void Module::InitPatchPoints(float init)
{	long i,n;
	CView *temp;
	n=getNbViews();
	for (i=0; i<n; i++)
	{	temp=getView(i);				//** Use pFirstView and pNext instead. Should be faster.
		if (temp->isTypeOf("PatchPoint"))
		{	((PatchPoint *) temp)->in = init;
			((PatchPoint *) temp)->out = init;
		}
	}
}

inline void Module::ProcessSample()
{
}

void Module::CableConnected(PatchPoint *pp)
{
}

void Module::CableDisconnected(PatchPoint *pp)
{
}

void Module::ValueChanged(CControl* pControl)
{
}

void Module::CountControlsAndPatchPoints()
{	// Calculates the number of controls and patch points and stores them num_controls and num_pp
	nbcontrols=0; nb_pp=0;
	CCView *ptemp=pFirstView;

	while (ptemp)
	{	if (ptemp->pView->isTypeOf("PatchPoint"))
		{	nbcontrols++; nb_pp++;
		}
		else if (ptemp->pView->isTypeOf("CControl"))
				if (((CControl *)(ptemp->pView))->getTag()!=NOSAVE_TAG) nbcontrols++;
		ptemp = ptemp->pNext;
	}
}


int Module::GetControlsTagsSize()
{	// This fuction assumes that all controls tags are of type long

	// If we already calculated it before
	if (nbcontrols>=0) return nbcontrols*sizeof(long);				// must be type of CControl::tag

	// Otherwise
	CountControlsAndPatchPoints();
	return nbcontrols*sizeof(long);									// must be type of CControl::tag
}

void Module::SaveControlsTags(void *pdata)
{	
	long *pfdata = (long *) pdata;							// This type must be the same type as tag/getTag()
	CCView *ptemp=pFirstView;

	while (ptemp)
	{	if (ptemp->pView->isTypeOf("CControl"))
		{	if (((CControl *)(ptemp->pView))->getTag()!=NOSAVE_TAG)
			{	*pfdata = ((CControl *)(ptemp->pView))->getTag(); pfdata++;
			}
		}
		ptemp = ptemp->pNext;
	}
}

void Module::LoadControlsTags(void *pdata, int size)
{	// You could use size to check, just in case the saved preset uses less data
	long *pfdata = (long *) pdata, ttag;							// This type must be the same type as tag/getTag()
	CCView *ptemp=pFirstView;
	CControl *ptemp2;

	while (ptemp && size>=sizeof(*pfdata))		//** && pfdata<pdata+(size/sizeof(*pfdata))
	{	if (ptemp->pView->isTypeOf("PatchPoint"))	
		{	((PatchPoint *)(ptemp->pView))->setTag(*pfdata);
			size-=sizeof(*pfdata); pfdata++;
		}
		else if (ptemp->pView->isTypeOf("CControl"))	
		{	if (((CControl *)(ptemp->pView))->getTag()!=NOSAVE_TAG)
			{	if ((*pfdata)<kNumParams)
				{
					// I'm not doing force untagging any more because a hacker might exploit this to unregister good tags
					//ptemp2 = (CControl *)(ptemp->pView);
					//if (ptemp2->getTag()<kNumParams) peditor->controls[ptemp2->getTag()]=NULL;
					//ptemp2->setTag(NO_TAG);					// No need because were going to put a new tag down

					//// Make sure the target tag is also unregsitered from it's control, if there is one.
					//ptemp2 = peditor->controls[*pfdata];
					//if (ptemp2!=NULL)
					//{	// Again manual unregistration
					//	// UnRegisterTag(peditor->controls[*pfdata]);
					//	ptemp2->setTag(NO_TAG); 
					//	peditor->controls[*pfdata]=NULL;
					//}

					// Try to Register the preset requested tag. If succsefull, unregisted the tag that might alread have been
					// Registered in module constructor. if not keep the contructor tag registered.
					ttag=((CControl *)(ptemp->pView))->getTag();
					if (RegisterTag(((CControl *)(ptemp->pView)),*pfdata))
					{	// This is not needed any more since UnRegisterControlsTags() is laways called
						// Before calling this function
						UnRegisterOldTag((CControl *)(ptemp->pView),ttag,*pfdata);
					}
				}
				//else ((CControl *)(ptemp->pView))->setTag(*pfdata);		// Tag is discarded because it can create conflicts
				size-=sizeof(*pfdata); pfdata++;
			}
		}
		ptemp = ptemp->pNext;
	}
}

void Module::UnRegisterControlsTags()
{	CCView *pv=pFirstView;

	while (pv)
	{	if (pv->pView->isTypeOf("CControl"))
			UnRegisterTag((CControl *)(pv->pView));
		pv = pv->pNext;
	}
}

void Module::ResetNbControlsAndPatchPoints()
{	nbcontrols=-1; nb_pp=-1;
}

void Module::SetPPTags(long &ctag)
{	// The primary porpus of this it to set the pptag. which shouldn't be changed by developers. These are exclusively used by SoloRack to save cable connections
	// A side effect is that it will set the tag (VSTGUI) too to the same value. This one will be saved, although it is currently not used. Might be usefull in future
	// if something goes wrong.
	CCView *ptemp=pFirstView;

	while (ptemp)
	{	if (ptemp->pView->isTypeOf("PatchPoint"))
		{	((PatchPoint *)(ptemp->pView))->pptag=ctag; 
			((PatchPoint *)(ptemp->pView))->setTag(ctag);
			ctag++;
		}
		ptemp = ptemp->pNext;
	}
}

void Module::ConstructTags2PP(PatchPoint **tags2pp, long &ctag, int nb_pp)
{	// nb_pp here is the number of patch points in the saved module, not necceesarly the working module.
	// This is because the working one might be in a newer version.
	CCView *ptemp=pFirstView;
	int i=1;

	if (this->nb_force_mono>0 && voice!=0)
	{	// Some patchpoints are force_mono=true
		CCView *ptempz = GetVoiceModule(0)->pFirstView;
		while (ptemp && i<=nb_pp)
		{	if (ptemp->pView->isTypeOf("PatchPoint"))
			{	// If it's a forcemono patch point. Then take it from voice zero
				if (((PatchPoint *)ptemp->pView)->force_mono)
					tags2pp[ctag] = (PatchPoint *) ptempz->pView;
				else tags2pp[ctag] = (PatchPoint *) ptemp->pView;
				ctag++; i++;
			}
			ptemp = ptemp->pNext; ptempz = ptempz->pNext;
		}
	}
	else
	{	// Most usual case
		while (ptemp && i<=nb_pp)
		{	if (ptemp->pView->isTypeOf("PatchPoint"))
			{	tags2pp[ctag] = (PatchPoint *) ptemp->pView;
				ctag++; i++;
			}
			ptemp = ptemp->pNext;
		}
	}
}


int Module::GetControlsValuesSize()
{	// This fuction assumes that all controls tags of type float

	// If we already calculated it before
	if (nbcontrols>=0) return nbcontrols*sizeof(float);					// must be type of CControl::value

	// Otherwise
	CountControlsAndPatchPoints();
	return nbcontrols*sizeof(float);									// must be type of CControl::value
}

void Module::SaveControlsValues(void *pdata)
{	float *pfdata = (float *) pdata;							// This type must be the same type as value/getValue()
	CCView *ptemp=pFirstView;

	while (ptemp)
	{	if (ptemp->pView->isTypeOf("CControl"))
		{	if (((CControl *)(ptemp->pView))->getTag()!=NOSAVE_TAG)
			{	*pfdata = ((CControl *)(ptemp->pView))->getValue(); pfdata++;
			}
		}
		ptemp = ptemp->pNext;
	}
}

void Module::LoadControlsValues(void *pdata, int size)
{	// Size is used to check, just in case the saved preset uses less data
	float *pfdata = (float *) pdata;							// This type must be the same type as value/getValue()
	CCView *ptemp=pFirstView;

	while (ptemp && size>=sizeof(*pfdata))				//** && pfdata<pdata+(size/sizeof(*pfdata))
	{	if (ptemp->pView->isTypeOf("CControl"))	
		{	if (((CControl *)(ptemp->pView))->getTag()!=NOSAVE_TAG)
			{	//if (((CControl *)(ptemp->pView))->getBackground()!=mcbits[scrbit])
					((CControl *)(ptemp->pView))->setValue(*pfdata);
				// Set svalue, this some times but not always important, like for the case if you want to preserve the saved phase
				if (ptemp->pView->isTypeOf("ModuleKnob"))
					if (((ModuleKnob *)(ptemp->pView))->is_stepping)
						((ModuleKnob *)(ptemp->pView))->svalue=((ModuleKnob *)(ptemp->pView))->qvalue;				// qvalue is already set by setValue above
					else
						((ModuleKnob *)(ptemp->pView))->svalue=((ModuleKnob *)(ptemp->pView))->getValue();
				else
					// Set MIDI patch point value to zero, this will remove plug image. Untill it is connected latter by the preset
					// This is better if preset loading was interupted because in that case it will show a plug without a cable!!
					if (ptemp->pView->isTypeOf("PatchPoint"))
					{	PatchPoint *pp = (PatchPoint *)(ptemp->pView);
						if (pp->protocol==ppMIDI)
							if (pp->num_cables==0) pp->setValue(0); //pp->invalid();
							else pp->setValue(1);
					}
					
				size-=sizeof(*pfdata); pfdata++;
					
				ValueChanged((CControl *)(ptemp->pView));
			}
		}
		ptemp = ptemp->pNext;
	}
}

int Module::GetPresetSize()
{	return GetControlsValuesSize();
}

void Module::SavePreset(void *pdata, int size)
{	SaveControlsValues(pdata);
}

void Module::LoadPreset(void *pdata, int size, int version)
{	LoadControlsValues(pdata,size);
}

const char * Module::GetName()
{	return "";
}

const int Module::GetNameLen()
{	return -1;
}

const char * Module::GetVendorName()
{	return "";
}

const int Module::GetVendorNameLen()
{	return -1;
}

const int Module::GetType()
{	return -1;
}

void Module::SetKnobsSmoothDelay(int del)
{	//del=-1 will use the same delay already assciated with each knob. This function will be called by solorack when sample rate changes too because variable smooth_samples has to be recalculated
	CCView *ptemp=pFirstView;

	while (ptemp)				//** && pfdata<pdata+(size/sizeof(*pfdata))
	{	if (ptemp->pView->isTypeOf("ModuleKnob"))	
			((ModuleKnob *)(ptemp->pView))->SetSmoothDelay(del,this);
		ptemp = ptemp->pNext;
	}
}

void Module::SetSampleRate(float sr)
{	// Will not be auto called when a module is first created. sample_rate should be set by contructors. Sure you can call this function from inside your contructor
	// Will only be called by SoloRack when sample rate changes.
	// Your version of SetSampleRate() is responsible for calling SetBandLimit() OR accounting for bandlimit value within it (if you require it by your module). Solorack will not autocall SetBandLimit() for you
	// I do this because SetBandLimit() may include memory allocation for things like Minblep, etc. Which can trigger multiple/redundant allocations if both functions are called when a preset is loaded.
	
	sample_rate = sr;
	hsample_rate = sr/2.0;
	sample_rate60 = 60.0*sr;
	
	// Recalculate smoothed delay for all knobs
	SetKnobsSmoothDelay(-1);
}

void Module::SetDAWBlockSize(float blocksize)
{	DAW_block_size=blocksize;
}

//void Module::SetBlockSize(int bs)
//{	block_size=bs;
//}

bool Module::SetBandLimit(int bndlim)
{	// Called by solorack
	bandlimit=bndlim;
	return true;
}

Product *Module::Activate(char *fullname, char *email, char *serial)
{	// No activation done. Base module class is always active.
	return NULL;
}

bool Module::IsActive()
{	// By default, the module is active (licensed)
	return true;
}

Product *Module::InstanceActivate(char *fullname, char *email, char *serial)
{	return this->Activate(fullname,email,serial);
}

bool Module::InstanceIsActive()
{	return this->IsActive();
}

void Module::AddDemoViews(char *msg)
{	// Must set the tag to all added controls to NOSAVE_TAG. Otherwise, values of these controls will be saved
	// And will cause a crash when the preset is opened
	// NOTE: It's recommended that you have your own DIFFERENT version of this function
	
	CRect r;
	CColor cc,cc2;

	r = this->getViewSize();
	r = CRect(CPoint(5,27),CPoint(r.width()-10,r.height()-54));
	demolabel = new CTextLabel(r,msg,NULL,kShadowText);
	demolabel->getFont()->setStyle(kBoldFace);
	//line_height=1.6*demolabel->getFont()->getSize();
	//r.setHeight(line_height); demolabel->setViewSize(r);
	cc = CColor(); cc.blue=50; cc.green=50; cc.red=255; cc.alpha=160;
	demolabel->setBackColor(cc);
	cc.blue=0; cc.green=0; cc.red=0; cc.alpha=160;
	demolabel->setShadowColor(cc);
	//cc2.blue=0; cc2.green=0; cc2.red=0; cc2.alpha=255;
	//demolabel->setFontColor(cc2);
	demolabel->setTag(NOSAVE_TAG); demolabel->setVisible(false);
	addView(demolabel);
}

void Module::SetEnableDemo(bool state)
{	// NOTE: It's recommended that you have your own DIFFERENT version of this function.
	
	CRect r;
	CColor cc,cc2;
	
	if (demolabel==NULL)			// Just in case
	{	AddDemoViews("Demo");
	}
	else if (state)					// Make sure it the right size and color
	{	if (this!=demolabel->getParentView())			// Make sure it's still attached.
		{	if (demolabel->isAttached())
				((CViewContainer *) (demolabel->getParentView()))->removeView(demolabel);
			addView(demolabel);
		}
		r = this->getViewSize();						// Correct size
		r = CRect(CPoint(5,27),CPoint(r.width()-10,r.height()-54));
		demolabel->setViewSize(r);
		cc = CColor(); cc.blue=50; cc.green=50; cc.red=255; cc.alpha=160;
		demolabel->setBackColor(cc);
	}

	demolabel->setVisible(state);
	invalid();

	// Old version
	//if (demolabel!=NULL)
	//{	demolabel->setVisible(state);
	//	invalid();
	//}
}

// The folliwng Add... function are not general purpose. They are meant to make adding controls just a bit quicker.
// Coordinates here are in float!! this is to allow finer adjustment for larger than 1 skin scale
// Skin scale 1 is the minimum size (small) of the skin. Each 1.0 (x or y) coresponds to 1 pixel in that size.
PatchPoint *Module::AddPatchPoint(float x, float y, int pptype, CBitmap **ppbit, int bitm_index, CControlListener *listener)
{	// x,y are the cordinates for the centre
	// if bitm_index==RAND_BITMAP, then a random bitmap is chosen
	PatchPoint *pptemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	if (bitm_index==RAND_BITMAP) bitm_index = GenRand(0,NUM_PP_BITMAPS-1);
	x=FSkinScale(x); y=FSkinScale(y);
	r.moveTo(FRound(x-(float) (ppbit[bitm_index]->getWidth()/2)), FRound(y-(float) (ppbit[bitm_index]->getHeight()/2)));
	r.setSize(CPoint(ppbit[bitm_index]->getWidth(),ppbit[bitm_index]->getHeight()));
	pptemp = new PatchPoint(r,listener,NO_TAG,ppbit[bitm_index],pptype); addView(pptemp);
	return pptemp;
}

PatchPoint *Module::AddMIDIPatchPoint(float x, float y, int pptype, CBitmap *bitmap, CControlListener *listener)
{	// x,y are the cordinates for the centre
	PatchPoint *pptemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	// Create MIDI patch port
	x=FSkinScale(x); y=FSkinScale(y);
	r.moveTo(FRound(x-(float) (bitmap->getWidth()/2)), FRound(y-(float) (bitmap->getHeight()/4)));
	r.setSize(CPoint(bitmap->getWidth(),bitmap->getHeight()/2));
	pptemp = new PatchPoint(r,listener,NO_TAG,bitmap,pptype); addView(pptemp);
	pptemp->SetProtocol(ppMIDI);
	return pptemp;
}

ModuleKnob *Module::AddModuleKnob(float x, float y, CBitmap *bitmap, int num_images, bool is_stepping, CControlListener *listener, long tag)
{	// x,y are the cordinates for the centre
	ModuleKnob *ktemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	x=FSkinScale(x); y=FSkinScale(y);
	r.moveTo(FRound(x-(float) (bitmap->getWidth()/2)), FRound(y-(float) (bitmap->getHeight()/(2*num_images))));
	r.setSize(CPoint(bitmap->getWidth(), bitmap->getHeight()/num_images));
	if (tag==NEW_TAG) tag = GetFreeTag();
	ktemp = new ModuleKnob(r,listener,tag,bitmap,this); addView(ktemp); RegisterTag(ktemp,tag);
	ktemp->is_stepping=is_stepping;
	return ktemp;
}

CVerticalSwitch *Module::AddVerticalSwitch(float x, float y, CBitmap *bitmap, int num_images, CControlListener *listener)
{	// x,y are the cordinates for the centre
	long tag;
	CVerticalSwitch *swtemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	x=FSkinScale(x); y=FSkinScale(y);
	r.moveTo(FRound(x-(float) (bitmap->getWidth()/2)), FRound(y-(float) (bitmap->getHeight()/(2*num_images))));
	r.setSize(CPoint(bitmap->getWidth(),bitmap->getHeight()/num_images));
	swtemp = new CVerticalSwitch(r,listener,tag=GetFreeTag(),bitmap); addView(swtemp); RegisterTag(swtemp,tag);
	return swtemp;
}

CMovieBitmap *Module::AddMovieBitmap(float x, float y, CBitmap *bitmap, int num_images, CControlListener *listener, bool centre)
{	CMovieBitmap *mbtemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	r.setSize(CPoint(bitmap->getWidth(),bitmap->getHeight()/num_images));
	x=FRound(FSkinScale(x)); y=FRound(FSkinScale(y));
	if (centre) 
		r.moveCentreTo(x,y);
	else
		r.moveTo(x,y);
	mbtemp = new CMovieBitmap(r,listener,NO_TAG,bitmap); addView(mbtemp);
	return mbtemp;
}

CKickButton *Module::AddKickButton(float x, float y, CBitmap *bitmap, int num_images, CControlListener *listener)
{	// x,y are the cordinates for the centre
	long tag;
	CKickButton *btemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	x=FSkinScale(x); y=FSkinScale(y);
	r.moveTo(FRound(x-(float) (bitmap->getWidth()/2)), FRound(y-(float) (bitmap->getHeight()/(2*num_images))));
	r.setSize(CPoint(bitmap->getWidth(),bitmap->getHeight()/num_images));
	btemp = new CKickButton(r,listener,tag=GetFreeTag(),bitmap); addView(btemp); RegisterTag(btemp,tag);
	return btemp;
}

COnOffButton *Module::AddOnOffButton(float x, float y, CBitmap *bitmap, int num_images, CControlListener *listener, long style, long tag)
{	// x,y are the cordinates for the centre
	COnOffButton *btemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	x=FSkinScale(x); y=FSkinScale(y);
	r.moveTo(FRound(x-(float) (bitmap->getWidth()/2)), FRound(y-(float) (bitmap->getHeight()/(2*num_images))));
	r.setSize(CPoint(bitmap->getWidth(),bitmap->getHeight()/num_images));
	if (tag==NEW_TAG) tag = GetFreeTag();
	btemp = new COnOffButton(r,listener,tag,bitmap,style); addView(btemp); RegisterTag(btemp,tag);
	return btemp;
}

CSpecialDigit *Module::AddSpecialDigit(float x, float y, CBitmap *bitmap, int num_images, int inumbers, long tag, CControlListener *listener)
{	CSpecialDigit *dtemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	x=FRound(FSkinScale(x)); y=FRound(FSkinScale(y));
	r.moveTo(x, y);
	r.setSize(CPoint(inumbers*bitmap->getWidth(), bitmap->getHeight()/num_images ));
	dtemp = new CSpecialDigit(r,listener,tag,0,inumbers,NULL,NULL,bitmap->getWidth(),bitmap->getHeight()/num_images,bitmap); addView(dtemp); //RegisterTag(dch,tag);
	return dtemp;
}

CSpecialDigitEx *Module::AddSpecialDigitEx(float x, float y, CBitmap *bitmap, int num_images, int inumbers, long tag, CControlListener *listener, CBitmap* blank)
{	CSpecialDigitEx *dtemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	x=FRound(FSkinScale(x)); y=FRound(FSkinScale(y));
	r.moveTo(x, y);
	r.setSize(CPoint(inumbers*bitmap->getWidth(),bitmap->getHeight()/num_images));
	dtemp = new CSpecialDigitEx(r,listener,tag,0,inumbers,NULL,NULL,bitmap->getWidth(),bitmap->getHeight()/num_images,bitmap,blank); addView(dtemp); //RegisterTag(dch,tag);
	return dtemp;
}

ModuleKnobEx *Module::AddModuleKnobEx(float x, float y, CBitmap *bitmap, int num_images, bool is_stepping, CControlListener *listener, CControl *vattached1, int vtag1, long tag)
{	// x,y are the cordinates for the centre
	ModuleKnobEx *ktemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	x=FSkinScale(x); y=FSkinScale(y);
	r.moveTo(FRound(x-(float) (bitmap->getWidth()/2)), FRound(y-(float) (bitmap->getHeight()/(2*num_images))));
	r.setSize(CPoint(bitmap->getWidth(), bitmap->getHeight()/num_images));
	if (tag==NEW_TAG) tag = GetFreeTag();
	ktemp = new ModuleKnobEx(r,listener,tag,bitmap,this); addView(ktemp); RegisterTag(ktemp,tag);
	ktemp->is_stepping=is_stepping;
	ktemp->attached1=vattached1; ktemp->tag1=vtag1;
	return ktemp;
}

CTextLabel *Module::AddTextLabel(float x, float y, CBitmap *bitmap, long style, float width, float height)
{	long tag;
	CTextLabel *btemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	x=FRound(FSkinScale(x)); y=FRound(FSkinScale(y));
	r.moveTo(x, y);
	if (width>=0.0 || height>=0.0)
	{	r.setSize(CPoint(FRound(FSkinScale(width)),FRound(FSkinScale(height))));
	}
	else
	{	// Bitmap must be not NULL in this case
		r.setSize(CPoint(bitmap->getWidth(),bitmap->getHeight()));
	}
	btemp = new CTextLabel(r,NULL,bitmap,style); addView(btemp);
	btemp->setTag(NO_TAG);
	return btemp;
}

CTextLabelEx *Module::AddTextLabelEx(float x, float y, CBitmap *bitmap, long style, float width, float height)
{	long tag;
	CTextLabelEx *btemp;
	CRect r(CPoint(0,0),CPoint(1,1));

	x=FRound(FSkinScale(x)); y=FRound(FSkinScale(y));
	r.moveTo(x, y);
	if (width>=0.0 || height>=0.0)
	{	r.setSize(CPoint(FRound(FSkinScale(width)),FRound(FSkinScale(height))));
	}
	else
	{	// Bitmap must be not NULL in this case
		r.setSize(CPoint(bitmap->getWidth(),bitmap->getHeight()));
	}
	btemp = new CTextLabelEx(r,NULL,bitmap,style); addView(btemp);
	btemp->setTag(NO_TAG);
	return btemp;
}

CHorizontalSlider *Module::AddHorizontalSlider(float x, float y, float width, float height, CBitmap *bitmap, CBitmap *background, CControlListener *listener)
{	// if a width or height is <0 then it will be taken from background
	long tag;
	CHorizontalSlider *hslider;
	CRect r(CPoint(0,0),CPoint(1,1));

	x=FRound(FSkinScale(x)); y=FRound(FSkinScale(y)); r.moveTo(x, y);
	width = FRound(FSkinScale(width)); height = FRound(FSkinScale(height)); 
	if (width>=0.0 && height>=0.0) r.setSize(CPoint(width,height));
	else if (background!=NULL) r.setSize(CPoint(background->getWidth(),background->getHeight()));
	else r.setSize(CPoint(0,0));
	hslider = new CHorizontalSlider(r, listener, tag=GetFreeTag(), 0, r.getWidth(), bitmap, background,CPoint(0,0),kLeft);
	addView(hslider); RegisterTag(hslider,tag);
	return hslider;
}


inline void Module::SendAudioToDAW(float left, float right)
{	synth_comm.ModuleSendAudioToDAW1Ptr(this,left,right);
}

inline void Module::SendAudioToDAW(PatchPoint **pps_outputs)
{	// INs of patchpoints will be sent to DAW outs
	// NULL indicates the end of the array. Advantage is that there is no num_inputs to pass.
	synth_comm.ModuleSendAudioToDAW2Ptr(this,pps_outputs);
}

inline void Module::SendAudioToDAW(PatchPoint **pps_outputs, int first_output)
{	// INs of patchpoints will be sent to DAW outs
	// NULL indicated the end of the array. Advantage is that there is no last_output to pass.
	// Sending will start from pps_outputs[first_output]
	synth_comm.ModuleSendAudioToDAW3Ptr(this,pps_outputs,first_output);
}

inline void Module::SendAudioToDAW(float *outputs, int last_output)
{	// Send audio to DAW from an array of float. Sending will stop at outputs[last_output]
	synth_comm.ModuleSendAudioToDAW4Ptr(this,outputs,last_output);
}

inline void Module::SendAudioToDAW(float *outputs, int first_output, int last_output)
{	// Send audio to DAW from an array of float
	// Sending will start from outputs[first_output] and will stop at outputs[last_output]
	synth_comm.ModuleSendAudioToDAW5Ptr(this,outputs,first_output,last_output);
}

inline void Module::ReceiveAudioFromDAW(float *left, float *right)
{	synth_comm.ModuleReceiveAudioFromDAW1Ptr(this,left,right);
}

inline void Module::ReceiveAudioFromDAW(PatchPoint **pps_inputs)
{	// Outs of patch points will be filled with audio from DAW
	// NULL indicates the end of the array. Advantage is that there is no last_output to pass.
	synth_comm.ModuleReceiveAudioFromDAW2Ptr(this,pps_inputs);
}

inline void Module::ReceiveAudioFromDAW(PatchPoint **pps_inputs, int first_input)
{	// Outs of patch points will be filled with audio from DAW
	// NULL indicates the end of the array. Advantage is that there is no num_inputs to pass.
	// Recieving will start from pps_inputs[first_output]
	synth_comm.ModuleReceiveAudioFromDAW3Ptr(this,pps_inputs,first_input);
}

inline void Module::ReceiveAudioFromDAW(float *inputs, int last_input)
{	// Recieve audio from DAW to an array of float. Will stop at inputs[last_input]
	synth_comm.ModuleReceiveAudioFromDAW4Ptr(this,inputs,last_input);
}

inline void Module::ReceiveAudioFromDAW(float *inputs, int first_input, int last_input)
{	// Recieve audio from DAW to an array of float. Receiving will start at inputs[first_output] and will stop at inputs[last_input] 
	synth_comm.ModuleReceiveAudioFromDAW5Ptr(this,inputs,first_input,last_input);
}

inline int Module::GetNumberOfAudioFromDAW()
{	return synth_comm.ModuleGetNumberOfAudioFromDAWPtr(this);
}

inline int Module::GetNumberOfAudioToDAW()
{	return synth_comm.ModuleGetNumberOfAudioToDAWPtr(this);
}

inline void Module::EnterProcessingCriticalSection()
{	// Once called, it will block audio processing until LeaveProcessingCriticalSection() is called.
	// This means that ProcessSample() will not be called for any module while this module has not left the critical section.
	synth_comm.ModuleEnterProcessingCriticalSectionPtr(this);
}

inline void Module::LeaveProcessingCriticalSection()
{	synth_comm.ModuleLeaveProcessingCriticalSectionPtr(this);
}

const char *Module::GetInfoURL()
{	// Should return a string (infourl) containing the complete URL where the user can find more information about this module.
	// SoloRack will browse that URL whwn the user right clicks the module and clicks "Info...".
	return infourl;
}



// This is just a place holder. Although it's static, This function should be implemented in derived classes.
const char *Module::GetProductName()
{	return NULL;
}

//// Set module to synth communication object.
//void Module::SetSynthComm(const SynthComm *psynth_comm)
//{	synth_comm=*psynth_comm;
//}

//void DeleteModule()
//{	delete this;
//}

void Module::PolyphonyChanged(int voices)
{	// Called By SoloRack when the user changes the number of voices in the menu
	// By default, does nothing
}

void Module::SetForceMono(PatchPoint *pp, bool set_is_mono)
{	// The module must be a parent of pp.
	
	if (!pp->force_mono && set_is_mono) nb_force_mono++;
	else if (pp->force_mono && !set_is_mono) nb_force_mono--;
	pp->force_mono = set_is_mono;
}

Module *Module::GetVoiceModule(int voice)
{	// This usefull if you want different voices of the same module to talk to each other.
	return synth_comm.GetVoiceModule(peditor,this,voice);
}

void Module::ConstructionComplete(int voices)
{	// This function is called by SoloRack after all constructors of all voices of a module is called
	// And after all solorack specific information like index, procindex, etc... has been setup
	// This is where GetVoiceModule() will start working.

}

void Module::DestructionStarted()
{	// This function is called by SoloRack for all voices just before destructors are called of all voice.
	// This will give a chance for odd modules that cannot imidiatly destruct, for example in case the module
	// Is running a high CPU worker thread that needs to be stopped, or is connecting to the network.

	// The idea here is to give all modules a quick heads up, then atempt to destruct them one by one. This allows 
	// the odd modules to end there work WHILE solorack destructs other modules
}

// NONE_CPP_DESTRUCTOR is reserved for future use to resolve ABI compatibility issues with compilers other
// than VC++
#ifdef NONE_CPP_DESTRUCTOR
bool Module::forget()
{	nbReference--; 
	if (nbReference == 0) { this->Destructor(); delete this; return true; } else return false;
}
#else
bool Module::forget()
{	return CBaseObject::forget();
}
#endif

void Module::Destructor()
{	// Reserved for future use. Please do not implement this. It's not ready.
}

// Not a fan of these Setters and getters. But I think it's better for future generic coding and better ABI compatibility in the future.
float Module::GetSampleRate() {	return sample_rate; }
float Module::GetHalfSampleRate() { return hsample_rate; }
float Module::Get60SampleRate() { return sample_rate60; }
float Module::GetDAWSampleRate() { return DAW_sample_rate; }
int Module::GetDAWBlockSize() { return DAW_block_size; }

int Module::GetNBControls() { return nbcontrols; }
void Module::SetNBControls(int n) { nbcontrols=n; }
int Module::GetNBPatchPoints() { return nb_pp; }
void Module::SetNBPatchPoints(int n) { nb_pp=n; }
int Module::GetNBCables() { return nb_cables; }
void Module::SetNBCables(int n) { nb_cables=n; }
int Module::GetBandLimit() { return bandlimit; }

SoloRack *Module::GetSynth() { return psynth; }
void Module::SetSynth(SoloRack *p) { psynth=p; }
const SynthComm Module::GetSynthComm() { return synth_comm; }
void Module::SetSynthComm(const SynthComm *p) { synth_comm=*p; }
SynEditor *Module::GetSynEditor() { return peditor; }
void Module::SetSynEditor(SynEditor *p) { peditor=p; }

bool Module::GetInMove() { return in_move; }
void Module::SetInMove(bool b) { in_move=b; }
int Module::GetIndex() { return index; }
void Module::SetIndex(int i) { index=i; }

int Module::GetProcIndex() { return procindex; }
void Module::SetProcIndex(int i) { procindex=i; }
int Module::GetEvIndex() { return evindex; }
void Module::SetEvIndex(int i) { evindex=i; }
int Module::GetSbIndex() { return sbindex; }
void Module::SetSbIndex(int i) { sbindex=i; }
int Module::GetMouseObIndex() { return mouseobindex; }
void Module::SetMouseObIndex(int i) { mouseobindex=i; }
float Module::GetClipLevel() { return clip_level; }
void Module::SetClipLevel(float v) { clip_level=v; }
int Module::GetVoice() { return voice; }
void Module::SetVoice(int vvoice) { voice = vvoice; }

CCoord Module::SkinScale(CCoord v)
{	return FRound(Module::uiscale*(float)v);
}

float Module::FSkinScale(float v)
{	return Module::uiscale*(float)v;
}

CRect Module::SkinScale(CRect r)
{	r.x = FRound(uiscale*(float)r.x); r.y = FRound(uiscale*(float)r.y);
	r.x2 = FRound(uiscale*(float)r.x2); r.y2 = FRound(uiscale*(float)r.y2);
	return r;
}

CCoord Module::SkinUnScale(CCoord v)
{	// Does the inverse of SkinScale(). i.e returns v to the base (smallest) skin scale.
	return FRound((float)v/Module::uiscale);
}

float Module::GetClipLevelPreset(int v)
{	switch (v)
	{	case km12db: return 0.25;
		case km6db: return 0.5;
		case km3db: return 0.70794578;
		case k0db: return 1.0;
		case k3db: return 1.41253755;
		case k6db: return 2.0;
		case k12db: default: return 4.0;
	}	
}


//class GranularProcessorEx : clouds::GranularProcessor
//{
//	inline float sample_rate() const {
//		return 32000.0f / \
//			(low_fidelity_ ? kDownsamplingFactor : 1);
//	}
//}

//---------------------------------------------
// Clouds
CBitmap *Clouds::panel = NULL;
char *Clouds::name = "Baadalon";
int Clouds::name_len=0;
char *Clouds::vendorname = "ShredsNCharades";
int Clouds::vendorname_len=0;
Product *Clouds::pproduct = NULL;

Clouds::Clouds(CFrame *pParent, CControlListener *listener,const SynthComm *psynth_comm, const int vvoice)
: Module(CRect(0, 0, panel->getWidth(), panel->getHeight()),pParent,panel,psynth_comm,vvoice)
{	PatchPoint *temp[6];

	// Create The Knobs
	kposition = AddModuleKnob(39,101,mcbits[knob_medium_red], SNC_MEDIUM_KNOB_IMAGES,false,listener);
	kposition->setValue(0.5); kposition->svalue=0.5; //ADDPOOLKNOB(chpool,kin1);

	ksize = AddModuleKnob(101,101,mcbits[knob_medium_green],SNC_MEDIUM_KNOB_IMAGES,false,listener);
	ksize->setValue(0.5); ksize->svalue=0.5; //ADDPOOLKNOB(chpool,kin2);
	 
	kpitch = AddModuleKnob(164,101,mcbits[knob_medium_white], SNC_MEDIUM_KNOB_IMAGES,false,listener);
	kpitch->setValue(0.5); kpitch->svalue=0.5; //ADDPOOLKNOB(chpool,kin3);

	kingain = AddModuleKnob(26,160,mcbits[knob_small_red],SNC_SMALL_KNOB_IMAGES,false,listener);
	kingain->setValue(0.5); kingain->svalue=0.5; //ADDPOOLKNOB(chpool,kout);

	kdensity = AddModuleKnob(76, 160,mcbits[knob_small_red], SNC_SMALL_KNOB_IMAGES,false,listener);
	kdensity->setValue(0.5); kdensity->svalue = 0.5;

	ktexture = AddModuleKnob(126,160,mcbits[knob_small_green], SNC_SMALL_KNOB_IMAGES,false,listener);
	ktexture->setValue(0.5); ktexture->svalue = 0.5;

	kblend = AddModuleKnob(178,160,mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES,false,listener);
	kblend->setValue(0.5); kblend->svalue = 0.5;

	kspread = AddModuleKnob(178,160,mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES,false,listener); kspread->setVisible(false);
	kspread->setValue(0.0); kspread->svalue = 0.0;
	
	kfeedback = AddModuleKnob(178,160,mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES,false,listener); kfeedback->setVisible(false);
	kfeedback->setValue(0.0); kfeedback->svalue = 0.0;
	
	kreverb = AddModuleKnob(178,160,mcbits[knob_small_white], SNC_SMALL_KNOB_IMAGES,false,listener); kreverb->setVisible(false);
	kreverb->setValue(0.0); kreverb->svalue = 0.0;

	// Create Patch Points
	ppfreeze = AddPatchPoint(17,218,ppTypeInput,ppbit,0,listener);
	pptrig = AddPatchPoint(51,218,ppTypeInput,ppbit,0,listener);
	ppposition = AddPatchPoint(84,218,ppTypeInput,ppbit,0,listener);
	ppsize = AddPatchPoint(117,218,ppTypeInput,ppbit,0,listener);
	pppitch = AddPatchPoint(150,218,ppTypeInput,ppbit,0,listener); 
	ppblend = AddPatchPoint(183,218,ppTypeInput,ppbit,0,listener);

	ppinl = AddPatchPoint(17,251,ppTypeInput,ppbit,0,listener);
	ppinr = AddPatchPoint(51,251,ppTypeInput,ppbit,0,listener);
	ppdensity = AddPatchPoint(84,251,ppTypeInput,ppbit,0,listener);
	pptexture = AddPatchPoint(117,251,ppTypeInput,ppbit,0,listener);
	ppoutl = AddPatchPoint(150,251,ppTypeOutput,ppbit,0,listener); 
	ppoutr = AddPatchPoint(183,251,ppTypeOutput,ppbit,0,listener);

	// Buttons
	bfreez = AddOnOffButton(20,43,mcbits[button_led],2,listener,COnOffButton::kPostListenerUpdate);
	bmode = AddKickButton(166,45,mcbits[black_butbit_tiny],2,listener);
	bquality = AddKickButton(187.6,45,mcbits[black_butbit_tiny],2,listener);
	balt = AddKickButton(187.6, 72, mcbits[black_butbit_tiny], 2, listener);
	

	lmix = AddMovieBitmap(65.6,44.7,mcbits[led_big_green], SNC_LED_GREEN_IMAGES,listener);
	lpan = AddMovieBitmap(89.6,44.7,mcbits[led_big_green], SNC_LED_GREEN_IMAGES,listener);
	lfb = AddMovieBitmap(114,44.7,mcbits[led_big_yellow], SNC_LED_YELLOW_IMAGES,listener);
	lreverb = AddMovieBitmap(138,44.7,mcbits[led_big_red], SNC_LED_RED_IMAGES,listener);

	// Put some screws
	PutLeftScrews(screw1,screw2,listener);
	PutRightScrews(screw3,screw4,listener);
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
{	delete processor;
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
Clouds *Clouds::Constructor(CFrame *pParent, CControlListener *listener,const SynthComm *psynth_comm, const int vvoice)
{	return new Clouds(pParent,listener,psynth_comm,vvoice);
}

void Clouds::Initialize()
{	char *stemp;
	
	panel = new CBitmap(dllskindir,NULL,"Clouds.png");
	name_len = strlen(name); vendorname_len = strlen(vendorname);
}

void Clouds::End()
{	panel->forget();
	if (pproduct!=NULL) delete pproduct;
}

const char * Clouds::GetName()
{	return name;
}

const int Clouds::GetNameLen()
{	return name_len;
}

const char * Clouds::GetVendorName()
{	return vendorname;
}

const int Clouds::GetVendorNameLen()
{	return vendorname_len;
}

const int Clouds::GetType()
{	return kModifierEffect;
}

Product *Clouds::Activate(char *fullname, char *email, char *serial)
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

Product *Clouds::InstanceActivate(char *fullname, char *email, char *serial)
{	return this->Activate(fullname,email,serial);
}

bool Clouds::InstanceIsActive()
{	return this->IsActive();
}

const char *Clouds::GetProductName()
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
	if (pControl == bmode && bmode->value>=0.5)
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
		{	case clouds::PLAYBACK_MODE_GRANULAR:
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
{	Module::SetSampleRate(sr);

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
	*(clouds::PlaybackMode*) pcdata = playback; pcdata += sizeof(playback);
	*(int*) pcdata = quality; pcdata += sizeof(quality);
	*(int*) pcdata = blendMode; pcdata += sizeof(blendMode);
}

void Clouds::LoadPreset(void* pdata, int size, int version)
{
	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();

	LoadControlsValues(pcdata, csize);
	pcdata += csize;	

	// Load non control data
	playback = *(clouds::PlaybackMode*) pcdata; pcdata += sizeof(playback);
	quality = *(int*) pcdata; pcdata += sizeof(quality);
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
	{	clouds::ShortFrame input[32] = {};
		dsp::Frame<2> inputFrames[32];
		if (quality!=4)
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
			int blen = mmin(32,inputBuffer.size());
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
		p->pitch = CLIP((SCALE(kpitch->value,-2.f,2.f) + pppitch->in*MIDI_TUNE_FACTOR) * 12.0f, -48.0f, 48.0f);
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
			{	outputBuffer.endData()[i].samples[0] = output[i].l / 32768.0;
				outputBuffer.endData()[i].samples[1] = output[i].r / 32768.0;
			}
			outputBuffer.endIncr(32);
		}
		triggered = false;
		outputFrame = outputBuffer.shift();

		// Lights
		if (leds_wait <= 0)
		{	dsp::Frame<2> lightFrame = freeze ? outputFrame : inputFrame;
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
	polyphonyMode = *(int*) pcdata; pcdata += sizeof(polyphonyMode);
	resonatorModel = *(rings::ResonatorModel*) pcdata; pcdata += sizeof(resonatorModel);

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
{	if (easterEgg)
		string_synth.set_fx((rings::FxType)resonatorModel);
	else
		part.set_model(resonatorModel);
}

const char* Rings::GetInfoURL()
{
	return "https://mutable-instruments.net/modules/rings/";
}

inline void Rings::ProcessSample()
{	bool strum;

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
		if (low_cpu==0)
		{	in = pin;
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
			in = (float *) inputBuffer.startData();
			
		}

		// Patch
		rings::Patch patch;
		float structure = kstruct->value + 3.3 * dsp::quadraticBipolar(SCALE(kstruct_cv->value,-1.f,1.f)) * ppstruct->in;
		patch.structure = CLIP(structure, 0.0f, 0.9995f);
		patch.brightness = CLIP(kbright->value + 3.3 * dsp::quadraticBipolar(SCALE(kbright_cv->value, -1.f, 1.f)) * ppbright->in, 0.0f, 1.0f);
		patch.damping = CLIP(kdamping->value + 3.3 * dsp::quadraticBipolar(SCALE(kdamping_cv->value, -1.f, 1.f)) * ppdamping->in, 0.0f, 0.9995f);
		patch.position = CLIP(kposition->value + 3.3 * dsp::quadraticBipolar(SCALE(kposition_cv->value, -1.f, 1.f)) * ppposition->in, 0.0f, 0.9995f);

		// Performance
		rings::PerformanceState performance_state;
		float transpose = SCALE(kfreq->value, 0.f, 60.f);
		if (pppitch->num_cables)
		{	performance_state.note = 12.0 * (MIDI_TUNE_FACTOR * pppitch->in + sr_notefix);
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
		{	strummer.Process(NULL, 24, &performance_state);
			string_synth.Process(performance_state, patch, in, out, aux, 24);
		}
		else 
		{	strummer.Process(in, 24, &performance_state);
			part.Process(performance_state, patch, in, out, aux, 24);
		}

		// Convert output buffer
		if (low_cpu==0)
		{	dsp::Frame<2> outputFrames[24];
			for (int i = 0; i < 24; i++) 
			{	outputFrames[i].samples[0] = out[i];
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
			{	outputBuffer.endData()[i].samples[0] = out[i];
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
	char *st = (char*) malloc(MAX_PATH * 2 * sizeof(*st));
	if (st != NULL)
	{	strcpy(st, dllskindir); GetParentDir(st);
		//strcat(st, "Segment14.ttf");
		//pf = new CFontDesc(st, SkinScale(40));
		strcat(st, "DSEG14Classic-Italic.ttf");
		pf = new CFontDesc(st, SkinScale(32), kItalicFace);
		if (pf->getPlatformFontFromFile() == NULL)
		{	pf->forget();
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
		{	const int16_t bit_mask = 0xffff;
			int16_t sample = render_buffer[i] & bit_mask;
			int16_t warped = ws.Transform(sample);
			render_buffer[i] = stmlib::Mix(sample, warped, signature);
		}

		if (lowCpu) 
		{	for (int i = 0; i < 24; i++) 
			{	dsp::Frame<1> f;
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
{	range = 1;
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
{	return NULL;
}

bool Tides::IsActive()
{	return true;
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
{	return "ShredsNCharades Jvaar";
}

void Tides::UpdateRangeMode()
{
	range_mode = (range < 2) ? tides2::RANGE_CONTROL : tides2::RANGE_AUDIO;
	lfrange->value = (float)(range+1) / 3.f;
	lfrange->invalid();
}
void Tides::UpdateOutPutMode()
{
	poly_slope_generator.Reset();
	lmode->value = (float)(output_mode+1) / 4.f;
	lmode->invalid();
}
void Tides::UpdateRampMode()
{
	lramp->value = (float)(ramp_mode+1) / 3.f;
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
	*(tides2::OutputMode*) pcdata = output_mode; pcdata += sizeof(output_mode);
	*(tides2::RampMode*) pcdata = ramp_mode; pcdata += sizeof(ramp_mode);
}

void Tides::LoadPreset(void* pdata, int size, int version)
{
	char* pcdata = (char*)pdata;
	int csize = GetControlsValuesSize();

	LoadControlsValues(pcdata, csize);
	pcdata += csize;

	// Load none-control data
	range = *(int*) pcdata; pcdata += sizeof(range);
	output_mode = *(tides2::OutputMode*) pcdata; pcdata += sizeof(output_mode);
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
	{	frame = 0;

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
		{	frequency = kRootScaled[range] / sample_rate * stmlib::SemitonesToRatio(transposition);
			must_reset_ramp_extractor = true;
		}

		// Get parameters
		ft = kslope->value + dsp::cubic(SCALE(kslopecv->value,-1.f,1.f)) * ppslope->in;
		float slope = CLIP(ft,0.f, 1.f);
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
		UpdateOutputs(ppin[0]->in >= HIGH_CV,0);
	}
	else if (pControl == bmode[1])
	{
		modes[1] = bmode[1]->value >= 0.5;
		UpdateOutputs(ppin[1]->in >= HIGH_CV,1);
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
	return "https://mutable-instruments.net/modules/Branches/";
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