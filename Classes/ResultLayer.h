#pragma once 

#include "GameBasic.h"

class ResultLayer : public CCLayer
{
public:
	ResultLayer();
	~ResultLayer();

	CREATE_FUNC(ResultLayer);

	virtual bool init();

	void showStar(int starIndex);

public:
	void menuCallBack(CCObject* sender);

	void initResultData(int gameScore, int referenceScore, int recordScore, bool ifSuccess);

private:
	bool mIgnoreTouch, mIsSucceed;

	int mGameScore;

	CCSprite** mStarSprites;
};