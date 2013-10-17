#ifndef _H_PATTERNSPRITE_
#define _H_PATTERNSPRITE_

#include "GameBasic.h"

class PatternSprite : public CCSprite, public CCTargetedTouchDelegate
{
public:
	PatternSprite();
	~PatternSprite();

	virtual void onEnter();
	virtual void onExit();

public:
	static PatternSprite* createWithAttribute(int type, ePatternExtraAttr attr);

	virtual bool initWithAttribute(int type, ePatternExtraAttr attr);

	void destroyPattern(CCArray * frams);

	void explodePattern(CCArray* frams);

	void removeFreeze();

	void moveTo(float duration, CCPoint position);

	void swapTo(float duration, CCPoint position);

	void onMoveEnd();

	bool containsTouchLocation(CCTouch* pTouch);

	virtual bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent);
    virtual void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent);
    virtual void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent);

    //virtual void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent);

public:
	int m_nRowIndex, m_nColIndex;

	int m_ePatternType, g_nRemoveBatchIndex;

	PatternSprite* g_pSwapPattern;

	ePatternStatus g_ePatternStatus;
	ePatternExtraAttr m_eExtraAttr;

	bool m_bSwapEnable, g_bIsRecover;

	eSwapDirection m_eSwapDirection;

private:
	
	CCSprite* m_extraTypeSpr;

	bool m_bHandleTouch;
};

#endif