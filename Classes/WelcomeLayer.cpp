#include "WelcomeLayer.h"
#include "SimpleAudioEngine.h"
#include "PatternMatrix.h"

using namespace CocosDenshion;

CCScene* WelcomeLayer::scene()
{
	// 'scene' is an autorelease object
    CCScene *scene = CCScene::create();

	CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("baseResource.plist");
    
    // 'layer' is an autorelease object
    WelcomeLayer *layer = WelcomeLayer::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

bool WelcomeLayer::init()
{
	if(!CCLayer::init())
	{
		return false;
	}

	CCSize winSize = CCDirector::sharedDirector()->getVisibleSize();

	//SimpleAudioEngine::sharedEngine()->setBackgroundMusicVolume(1);
	//SimpleAudioEngine::sharedEngine()->setEffectsVolume(1);
	//SimpleAudioEngine::sharedEngine()->playBackgroundMusic("audio/musicByFoxSynergy.mp3");

	CCSprite* background = CCSprite::create("background.jpg");
	background->setPosition(ccp(winSize.width/2, winSize.height/2));
	this->addChild(background);

	CCSprite* logo = CCSprite::create("logo.png");
	logo->setPosition(ccp(winSize.width/2, winSize.height/2 + 150));
	this->addChild(logo);

	CCMenuItemImage* image = CCMenuItemImage::create("btn/btnStartGameNor.png", "btn/btnStartGameDown.png", this, menu_selector(WelcomeLayer::StartGame));
	image->setPosition(ccp(winSize.width/2, winSize.height/2 - 150));

	CCMenu* menu = CCMenu::create(image, NULL);
	menu->setPosition(CCPointZero);

	this->addChild(menu);

	return true;
}

void WelcomeLayer::StartGame(CCObject* sender)
{
	CCScene* scene = CCScene::create();

	PatternMatrix* layer = PatternMatrix::createWithGameMode(Timer);
	scene->addChild(layer);

	CCDirector::sharedDirector()->replaceScene(CCTransitionSlideInB::create(0.4f, scene));
}