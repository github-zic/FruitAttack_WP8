#ifndef _H_WELCOME_
#define _H_WELCOME_

#include "cocos2d.h"

USING_NS_CC;

class WelcomeLayer : public CCLayer
{
public:
	static CCScene* scene();

	CREATE_FUNC(WelcomeLayer);

	virtual bool init();

public:
	void StartGame(CCObject* sender);
};

#endif