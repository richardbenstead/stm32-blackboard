#ifndef BASE_ANIMATION_H__
#define BASE_ANIMATION_H__

#include "ILI9341Wrapper.h"
#include "MathUtil.h"
#include "FrameParams.h"
#include <string>

class BaseAnimation {
public:
	BaseAnimation() {
	}
	;

	virtual void init(ILI9341Wrapper &tft);
	virtual uint_fast16_t bgColor(void);
	virtual void reset(ILI9341Wrapper &tft);
	virtual std::string title();

	virtual bool willForceTransition(void);
	virtual bool forceTransitionNow(void);

	virtual void perFrame(ILI9341Wrapper &tft, FrameParams frameParams);
};

void BaseAnimation::init(ILI9341Wrapper &tft) {
	// Extend me
}

uint_fast16_t BaseAnimation::bgColor(void) {
	// Extend me
	return 0xf81f;	// Everyone loves magenta
}

void BaseAnimation::reset(ILI9341Wrapper &tft) {
	// Extend me
}

std::string BaseAnimation::title() {
	return "BaseAnimation";
}

bool BaseAnimation::willForceTransition(void) {
	return false;// Default: SuperTFT will transition animations automatically
}

bool BaseAnimation::forceTransitionNow(void) {
	// Extend me
	return false;// Default: SuperTFT will transition animations automatically
}

void BaseAnimation::perFrame(ILI9341Wrapper &tft, FrameParams frameParams) {
	// Extend me
}

#endif
