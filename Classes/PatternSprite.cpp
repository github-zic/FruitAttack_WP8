#include "PatternSprite.h"
#include "PatternMatrix.h"

PatternSprite::PatternSprite()
{
	m_bSwapEnable = true;

	g_ePatternStatus = Status_Normal;
}

PatternSprite::~PatternSprite()
{

}

PatternSprite* PatternSprite::createWithAttribute(int type, ePatternExtraAttr attr)
{
	PatternSprite* sprite = new PatternSprite();

	if(sprite->initWithAttribute(type, attr))
	{
		sprite->autorelease();
	}
	else
	{
		CC_SAFE_RELEASE_NULL(sprite);
	}

	return sprite;
}

bool PatternSprite::initWithAttribute(int type, ePatternExtraAttr attr)
{
	if(!CCSprite::init())
	{
		return false;
	}

	m_eExtraAttr = attr;

	if(attr == Attr_Stone)
		m_ePatternType = -1;
	else
		m_ePatternType = type;

    if (m_ePatternType == -1 || attr == Attr_Freeze)
        m_bSwapEnable = false;
    else
        m_bSwapEnable = true;

    switch (m_eExtraAttr)
    {
        case Attr_Bomb:
        {
            m_extraTypeSpr = CCSprite::createWithSpriteFrameName("pattern_mark_explode.png");
            break;
        }
        case Attr_Freeze:
        {
            m_extraTypeSpr = CCSprite::createWithSpriteFrameName("pattern_mark_freeze.png");
            break;
        }
        default:
            break;
    }

    if (m_eExtraAttr == Attr_Stone)
        initWithSpriteFrameName("pattern_mark_petrifaction.png");
    else  
	{
        char str[20];
		sprintf(str, "cocos%02d.png", type);

        initWithSpriteFrameName(str);
    }

	setScale(2.4);

    if (m_extraTypeSpr)
	{
		CCSize size = getContentSize();

        m_extraTypeSpr->setPosition(ccp(size.width/2, size.height/2));
		addChild(m_extraTypeSpr);
    }

	return true;
}

void PatternSprite::destroyPattern(CCArray * frams)
{
	g_ePatternStatus = Status_Destroy;

	CCSprite* effectSprite = CCSprite::createWithSpriteFrameName("pattern_destroy_00.png");
    effectSprite->setPosition(ccp(22.5, 22.5));
    this->addChild(effectSprite);

    CCAnimation* animation = CCAnimation::createWithSpriteFrames(frams, 0.025);
    effectSprite->runAction(CCAnimate::create(animation));

    this->runAction(CCFadeOut::create(0.5));
    //gSharedEngine.playEffect(EFFECT_PATTERN_CLEAR);
}

void PatternSprite::explodePattern(CCArray* frams)
{
	ePatternStatus g_ePatternStatus = Status_Explode;
        
	CCSprite* effectSprite = CCSprite::createWithSpriteFrameName("pattern_explode_00.png");
    effectSprite->setPosition(ccp(22.5, 22.5));
    this->addChild(effectSprite);

    CCAnimation* animation = CCAnimation::createWithSpriteFrames(frams, 0.025);
    effectSprite->runAction(CCAnimate::create(animation));

    this->runAction(CCFadeOut::create(0.5));
    //gSharedEngine.playEffect(EFFECT_PATTERN_BOMB);
}

void PatternSprite::removeFreeze()
{
	if (m_eExtraAttr == Attr_Freeze)
    {
        m_eExtraAttr = Attr_Normal;

        m_bSwapEnable = true;

        removeChild(m_extraTypeSpr, true);
        m_extraTypeSpr = NULL;
    }
}

void PatternSprite::moveTo(float duration, CCPoint position)
{
	this->stopAllActions();

	//todo.
	if (g_ePatternStatus == Status_Normal)
    {
        g_ePatternStatus = Status_Move;

		CCAction* action = CCSequence::create(CCMoveTo::create(duration, position), CCCallFunc::create(this, callfunc_selector(PatternSprite::onMoveEnd)), NULL);
        this->runAction(action);
    }
}

void PatternSprite::swapTo(float duration, CCPoint position)
{
	this->stopAllActions();

	//todo.
	if (g_ePatternStatus == Status_Normal)
    {
        g_ePatternStatus = Status_Move;

        this->runAction(CCMoveTo::create(duration, ccp(position.x, position.y)));
    }
}

void PatternSprite::onMoveEnd()
{
	g_ePatternStatus = Status_Normal;
}

bool PatternSprite::containsTouchLocation(CCTouch* touch)
{
	CCPoint getPoint = touch->getLocation();

	return this->boundingBox().containsPoint(getPoint);
}

bool PatternSprite::ccTouchBegan(CCTouch* touch, CCEvent* event)
{
    if (m_bSwapEnable && g_ePatternStatus == Status_Normal && containsTouchLocation(touch))
	{
        if (m_eExtraAttr==Attr_Normal || m_eExtraAttr==Attr_Bomb)
        {
            m_bHandleTouch = true;
            //gNotification.postNotification(MSG_CHECK_PATTERN,this);

			PatternMatrix* matrix = (PatternMatrix*) (this->getParent()->getParent());
			matrix->onCheckPattern(this);

            return true;
        }
    }
    return false;
}

void PatternSprite::ccTouchMoved(CCTouch* touch, CCEvent* event)
{
    if (m_bHandleTouch && g_ePatternStatus == Status_Normal)
	{
		CCPoint getPoint = touch->getLocation();

        float lx = getPoint.x - getPositionX();
        float ly = getPoint.y - getPositionY();

        if (lx > 45)
        {
            m_bHandleTouch = false;

            if (ly > lx)
                m_eSwapDirection = Up;
            else if (ly + lx < 0)
                m_eSwapDirection = Down;
            else
                m_eSwapDirection = Right;

            //gNotification.postNotification(MSG_SWAP_PATTERN,this);
        }
        else if (lx < -45)
        {
            m_bHandleTouch = false;

            if (ly < lx)
                m_eSwapDirection = Down;
            else if(ly + lx > 0)
                m_eSwapDirection = Up;
            else
                m_eSwapDirection = Left;

            //gNotification.postNotification(MSG_SWAP_PATTERN,this);
        }
		else if (ly > 45)
        {
            m_bHandleTouch = false;
            m_eSwapDirection = Up;

            //gNotification.postNotification(MSG_SWAP_PATTERN,this);
        }
		else if (ly < -45)
        {
            m_bHandleTouch = false;
            m_eSwapDirection = Down;

            //gNotification.postNotification(MSG_SWAP_PATTERN,this);
        }
    }
}

void PatternSprite::ccTouchEnded(CCTouch* touch, CCEvent* event)
{
    
} 

void PatternSprite::onEnter() 
{
	CCDirector* pDirector = CCDirector::sharedDirector();
    pDirector->getTouchDispatcher()->addTargetedDelegate(this, 1, true);

	CCSprite::onEnter();
}
void PatternSprite::onExit() 
{
    CCDirector* pDirector = CCDirector::sharedDirector();
    pDirector->getTouchDispatcher()->removeDelegate(this);

    CCSprite::onExit();
}