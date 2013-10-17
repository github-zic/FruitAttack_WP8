#pragma once
#include "SimpleAudioEngine.h"

#include "cocos2d.h"

USING_NS_CC;

enum eSwapDirection
{
	Up = 0, 
	Down, 
	Left, 
	Right
};

enum ePatternExtraAttr
{
     Attr_Normal = 0,
     Attr_Bomb,
	 Attr_Freeze,
	 Attr_Stone
};

enum ePatternStatus
{
	Status_Normal = 0,
	Status_Move,
	Status_Destroy,
	Status_Explode
};

enum eGameMode
{
    Invalid = -1,
    Challenge = 0,
    Timer = 1,
    Count = 2
};


const char MSG_CHECK_PATTERN[] = "MSG_CHECK_PATTERN";
const char MSG_SWAP_PATTERN[] = "MSG_SWAP_PATTERN";