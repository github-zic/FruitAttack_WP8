#include "ResultLayer.h"
#include "PatternMatrix.h"

const int SCORE_MAX = 9999999;

enum MENU_TAG
{
	Continue = 1,
    Restart = 2,
	Quit = 3
};

ResultLayer::ResultLayer()
{

}
ResultLayer::~ResultLayer()
{
	//for(int i = 0; i < 3; i++)
	//{
	//	mStarSprites[i]->release();
	//}

	delete mStarSprites;
}

bool ResultLayer::init()
{
	if(!CCLayer::init())
	{
		return false;
	}

	mStarSprites = new CCSprite*[3];

	mIgnoreTouch = false;

	CCMenuItemImage* itemRestart = CCMenuItemImage::create("btnResultRestart.png", "btnResultRestartDown.png", this, menu_selector(ResultLayer::menuCallBack));
	itemRestart->setTag(Restart);
	itemRestart->setScale(1.8);

	CCMenu* menu = CCMenu::create(itemRestart, NULL);
	
	menu->setPosition(ccp( CCDirector::sharedDirector()->getWinSize().width / 2, 500));

	this->addChild(menu);

	return true;
}

void ResultLayer::initResultData(int gameScore, int referenceScore, int recordScore, bool isSucceed)
{
	bool bRet = true;

    if(gameScore > SCORE_MAX)
        gameScore = SCORE_MAX;
    else if(gameScore < 0)
        gameScore = 0;

    mGameScore = gameScore;
	float mStarScores[] = {0, 0, 0};

    if(referenceScore > 0 && referenceScore <= SCORE_MAX)
	{
        mStarScores[0] = gameScore * 0.4;
        mStarScores[1] = gameScore * 0.6;
        mStarScores[2] = gameScore * 0.8;
    }
    else
        bRet = false;

    mIsSucceed = isSucceed;
    for (int starIndex = 0; starIndex < 3; starIndex++)
	{
        if (mGameScore > mStarScores[starIndex])
		{
            showStar(starIndex);
        }
    }

    if (bRet == false)
        CCLog("GameResult Error");
}

void ResultLayer::showStar(int starIndex)
{
	mStarSprites[starIndex] = CCSprite::create("star.png");
    mStarSprites[starIndex]->setScale(0.1);

    switch(starIndex)
	{
        case 0:
            mStarSprites[starIndex]->setPosition(ccp(200, 745));
            break;
        case 1:
            mStarSprites[starIndex]->setPosition(ccp(384, 795));
            break;
        case 2:
            mStarSprites[starIndex]->setPosition(ccp(568, 745));
            break;
    }
    this->addChild(mStarSprites[starIndex]);

    mStarSprites[starIndex]->runAction(CCScaleTo::create(0.7, 1.6, 1.6));
    mStarSprites[starIndex]->runAction(CCRotateBy::create(0.7, 720.0));
}

void ResultLayer::menuCallBack(CCObject* sender)
{
	if(!mIgnoreTouch)
	{
		mIgnoreTouch = true;

		CCNode* node = (CCNode*)sender;

		switch (node->getTag())
		{
		case Restart:
			{
				CCScene* nextScene = CCScene::create();

				PatternMatrix* layer = PatternMatrix::createWithGameMode(Timer);
                nextScene->addChild(layer);

				CCDirector::sharedDirector()->replaceScene(CCTransitionSlideInB::create(0.4, nextScene));
                break;
			}
			break;
		default:
			break;
		}
	}
}