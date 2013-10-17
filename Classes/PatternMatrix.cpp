#include "PatternMatrix.h"

float gInitMatrixDelayTime = 0.7;
float gPatternsSwapTime = 0.17;
float gPatternsFallTime = 0.4;
float gPatternsClearTime = 0.5;

int MATRIX_CONTINUOUS = 3;
int MATRIX_ROW_MAX = 6;
int MATRIX_COL_MAX = 6;

PatternMatrix::PatternMatrix()
{

}

PatternMatrix::~PatternMatrix()
{
	delete []mPatternsPos;
	delete []mPatternsSpr;
	delete []matrixMark;

	mDestroyFrames->release();
	mExplodeFrames->release();
}

PatternMatrix* PatternMatrix::createWithGameMode(eGameMode mode)
{
	PatternMatrix *pRet = new PatternMatrix();
    if (pRet && pRet->initWithGameMode(mode))
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        CC_SAFE_DELETE(pRet);
        return NULL;
    }
}

bool PatternMatrix::initWithGameMode(eGameMode mode)
{
	if(!CCLayer::init())
	{
		return false;
	}

	gGameMode = mode;

	winSize = CCDirector::sharedDirector()->getVisibleSize();

	CCSprite* background = CCSprite::create("background.jpg");
	background->setPosition(ccp(winSize.width/2, winSize.height/2));
	this->addChild(background);

	mPatternBatchNode = CCSpriteBatchNode::create("baseResource.png", MATRIX_ROW_MAX * MATRIX_COL_MAX * 2);
    this->addChild(mPatternBatchNode, 1);

	mCheckMarkSpr = CCSprite::createWithSpriteFrameName("pattern_selected.png");
    mCheckMarkSpr->setScale(2.4);
	mCheckMarkSpr->setPosition(ccp(-100.0,-100.0));
    addChild(mCheckMarkSpr, 1);

    mPromptMarkSpr = CCSprite::createWithSpriteFrameName("pattern_selected.png");
	mPromptMarkSpr->setScale(2.4);
    mPromptMarkSpr->setPosition(ccp(-100.0, -100.0));
    addChild(mPromptMarkSpr, 1);

	mPromptMarkSpr->runAction(CCRepeatForever::create(CCSequence::create(CCFadeIn::create(1.0), CCFadeOut::create(1.0), NULL)));

	InitData();

	initProgressWithGameMode();

	initLabels();

	PatternBg = CCRenderTexture::create(768, 1280);
	PatternBg->setPosition( winSize.width / 2, winSize.height /2);
    this->addChild(PatternBg);

	initArrayFrames();

	mPatternsPos = createIntArray(MATRIX_ROW_MAX, MATRIX_COL_MAX, CCPointZero);
	mPatternsSpr = createIntArrays(MATRIX_ROW_MAX, MATRIX_COL_MAX, NULL);

	return true;
}

void PatternMatrix::onEnterTransitionDidFinish()
{
	initMatrix();
}

void PatternMatrix::InitData()
{
	mTimeTotal = 60;
	mPatternRequire = 100;
	mTimerTally = 0;

	mPromptTimerTally = 0;

	mDestroyBatchTally = 0;

	mPatternTypeMax = 0;
	mGameScore = 0;
	mPatternClearTally = 0;
}

void PatternMatrix::initProgressWithGameMode()
{
	mProgressBgSpr = CCSprite::create("ProgressBarBack.png");
	mProgressBgSpr->setAnchorPoint(ccp(0, 0.5));
    mProgressBgSpr->setPosition(ccp(35, mProgressBgSpr->getContentSize().height + 10));
    this->addChild(mProgressBgSpr);

    mProgressSpr = CCSprite::create("ProgressBarFront.png");

    switch(gGameMode)
	{
        case Timer:
        {
            mTimerTally = 0;
            mVisibleRect = CCRect(0, 0, 700, 52);
            break;
        }
        case Challenge:
        {
            mVisibleRect = CCRect(0,0,0,257);
            break;
        }
    }

	mProgressSpr->setPosition(ccp(35, mProgressBgSpr->getContentSize().height + 10));
	mProgressSpr->setAnchorPoint(ccp(0, 0.5));
	mProgressSpr->setTextureRect(mVisibleRect);

    this->addChild(mProgressSpr);
}

void PatternMatrix::initLabels()
{
	mScoreLabel = CCLabelTTF::create("Score 0", "Courier", 50);
	mScoreLabel->setPosition(ccp(winSize.width / 2, winSize.height - 130));

	this->addChild(mScoreLabel);
}

void PatternMatrix::initMatrix()
{
	//CCLOG("HERE IS INITMATRIX!");

	mPatternTypeMax = 7;

    m_nFreezeProbability = 3;
    m_nStoneProbability = 3;
    m_nBombProbability = 6;

	mUpdateLogic = true;

    m_nMatrixRow = 6, m_nMatrixCol = 6;

	int row,col;

	//渲染背景.
	{
		float halfSpace = 60;
		float space = 120;

		float baseX = 84;
		float baseY = 340;

		CCSprite* patternBg = CCSprite::create("PatternBg.png");
		PatternBg->begin();
	
		for ( row = 0; row < m_nMatrixRow; row++)
		{
			for ( col = 0; col < m_nMatrixCol; col++)
			{
				mPatternsPos[row][col] = ccp(baseX + col * space, baseY + row * space);
				
				patternBg->setPosition(mPatternsPos[row][col]);
				patternBg->visit();
			}
		}
		PatternBg->end();
	}

	//添加精灵.
	{
		for ( row = 0; row < m_nMatrixRow; row++)
		{
			for ( col = 0; col < m_nMatrixCol; col++)
			{
				this->addOnePattern(row,col);
			}
		}
	}

	//CCLOG("HERE IS INITMATRIX OVER!");

	this->schedule(schedule_selector(PatternMatrix::updateTimerForPrompt), 1 / 60);
	this->runAction(CCSequence::create(CCDelayTime::create(gPatternsFallTime + 0.1), CCCallFunc::create(this, callfunc_selector(PatternMatrix::detectionMatrix)), NULL));
}

void PatternMatrix::initArrayFrames()
{
	char str[30];
	CCSpriteFrame* frame;

	mDestroyFrames = CCArray::create();
    for (int i = 0; i < 18; i++)
    {
		sprintf(str, "pattern_destroy_%02d.png", i);

		frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(str);
		mDestroyFrames->addObject(frame);
    }

    mExplodeFrames = CCArray::create();
    for (int i = 0; i < 17; i++)
    {
		sprintf(str, "pattern_explode_%02d.png", i);

        frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(str);
        mExplodeFrames->addObject(frame);
    }

	mDestroyFrames->retain();
	mExplodeFrames->retain();
}

void PatternMatrix::addOnePattern(int row, int col)
{
	int temp = 0 | (int)(CCRANDOM_0_1() * 10000);
    float prob = temp % 100;

    ePatternExtraAttr attr = Attr_Normal;

    if (m_nFreezeProbability != 0  && prob < m_nFreezeProbability)
        attr = Attr_Freeze;
    else if (row!=0 && m_nStoneProbability!=0  && prob < m_nFreezeProbability + m_nStoneProbability )
        attr = Attr_Stone;
    else if (m_nBombProbability!=0  && prob < m_nFreezeProbability + m_nStoneProbability + m_nBombProbability )
        attr = Attr_Bomb;

    int patternType = 0 | (temp % mPatternTypeMax);

	mPatternsSpr[row][col] = PatternSprite::createWithAttribute(patternType, attr);
    mPatternsSpr[row][col]->setAnchorPoint(ccp(0.5, 0.5));
	mPatternsSpr[row][col]->m_nRowIndex = row;
    mPatternsSpr[row][col]->m_nColIndex = col;
    mPatternsSpr[row][col]->setPosition(ccp(mPatternsPos[row][col].x, mPatternsPos[row][col].y + 300 + col * 50));
    mPatternsSpr[row][col]->moveTo(gPatternsFallTime, mPatternsPos[row][col]);

    mPatternBatchNode->addChild(mPatternsSpr[row][col]);
}

void PatternMatrix::excludeDeadlock()
{
	if (isHasSolution() == false)
    {
        if (gGameMode == Timer)
        {
            for (int row=0; row < m_nMatrixRow; row++)
            {
                for (int col=0; col < m_nMatrixCol; col++)
				{
					mPatternBatchNode->removeChild(mPatternsSpr[row][col], true);
					addOnePattern(row, col);
				}
            }
			this->runAction(CCSequence::create(CCDelayTime::create(gPatternsFallTime + 0.1), CCCallFunc::create(this, callfunc_selector(PatternMatrix::detectionMatrix)), NULL));
        }
        else
        {
            onExit();
            showGameResult(false);
        }
    }
}

void PatternMatrix::onCheckPattern(PatternSprite* pPattern)
{
	if (pPattern)
	{
        mPromptTimerTally = 0;
        mCheckMarkSpr->setPosition(ccp(-1000.0, -1000.0));

        if (mFirstCheckPattern == NULL)
		{
            mFirstCheckPattern = pPattern;
            mCheckMarkSpr->setPosition(mPatternsPos[mFirstCheckPattern->m_nRowIndex][mFirstCheckPattern->m_nColIndex]);
        }
        else
		{
            mSecondCheckPattern = pPattern;
            if (mSecondCheckPattern == mFirstCheckPattern)
            {
                mSecondCheckPattern = NULL;
                return;
            }

            bool isAdjacent = false;
            if (mFirstCheckPattern->m_nRowIndex == mSecondCheckPattern->m_nRowIndex)
            {
                if (mFirstCheckPattern->m_nColIndex>0 &&
                    mFirstCheckPattern->m_nColIndex-1 == mSecondCheckPattern->m_nColIndex)
                    isAdjacent = true;
                else if (mFirstCheckPattern->m_nColIndex + 1 < m_nMatrixCol &&
                    mFirstCheckPattern->m_nColIndex+1 == mSecondCheckPattern->m_nColIndex)
                    isAdjacent = true;
            }
            else if (mFirstCheckPattern->m_nColIndex == mSecondCheckPattern->m_nColIndex)
            {
                if(mFirstCheckPattern->m_nRowIndex>0 &&
                    mFirstCheckPattern->m_nRowIndex - 1 == mSecondCheckPattern->m_nRowIndex)
                    isAdjacent = true;
                else if(mFirstCheckPattern->m_nRowIndex + 1 < m_nMatrixRow &&
                    mFirstCheckPattern->m_nRowIndex+1 == mSecondCheckPattern->m_nRowIndex)
                    isAdjacent = true;
            }

            if (isAdjacent)
			{
                mCheckMarkSpr->setPosition(ccp(-1000.0, -1000.0));

                swapTwoPattern(mFirstCheckPattern, mSecondCheckPattern, false);

                mFirstCheckPattern = NULL;
                mSecondCheckPattern = NULL;
            }
            else
            {
                mCheckMarkSpr->setPosition(mPatternsPos[mSecondCheckPattern->m_nRowIndex][mSecondCheckPattern->m_nColIndex]);

                mFirstCheckPattern = mSecondCheckPattern;
                mSecondCheckPattern = NULL;
            }
        }
    }
}

void PatternMatrix::onSwapTwoPattern(PatternSprite* pPattern)
{
	if (pPattern)
    {
        PatternSprite* pFirstCheckPattern = pPattern;
        if (mFirstCheckPattern == pFirstCheckPattern)
		{
            mFirstCheckPattern = NULL;
            mCheckMarkSpr->setPosition(ccp(-1000.0, -1000.0));
        }

        if(pFirstCheckPattern->g_ePatternStatus != Status_Normal)
            return;

        mPromptTimerTally = 0;
        mPromptMarkSpr->setPosition(ccp(-1000.0, -1000.0));

        PatternSprite* pSecondCheckPattern = NULL;
        switch(pFirstCheckPattern->m_eSwapDirection)
		{
            case Left:
                if (pFirstCheckPattern->m_nColIndex > 0)
                    pSecondCheckPattern = mPatternsSpr[pFirstCheckPattern->m_nRowIndex][pFirstCheckPattern->m_nColIndex-1];
                break;
            case Right:
                if (pFirstCheckPattern->m_nColIndex+1 < m_nMatrixCol)
                    pSecondCheckPattern = mPatternsSpr[pFirstCheckPattern->m_nRowIndex][pFirstCheckPattern->m_nColIndex+1];
                break;
            case Up:
                if (pFirstCheckPattern->m_nRowIndex+1 < m_nMatrixRow)
                    pSecondCheckPattern = mPatternsSpr[pFirstCheckPattern->m_nRowIndex+1][pFirstCheckPattern->m_nColIndex];
                break;
            case Down:
                if (pFirstCheckPattern->m_nRowIndex > 0)
                    pSecondCheckPattern = mPatternsSpr[pFirstCheckPattern->m_nRowIndex-1][pFirstCheckPattern->m_nColIndex];
                break;
            default :
                mFirstCheckPattern = NULL;
                mSecondCheckPattern = NULL;
                break;
        }

        if (pSecondCheckPattern && pSecondCheckPattern->g_ePatternStatus == Status_Normal)
		{
            if (mFirstCheckPattern == pSecondCheckPattern)
			{
                mFirstCheckPattern = NULL;
                mCheckMarkSpr->setPosition(ccp(-1000.0, -1000.0));
            }
            swapTwoPattern(pFirstCheckPattern,pSecondCheckPattern,false);
        }
    }
}

void PatternMatrix::swapTwoPattern(PatternSprite* firstPattern, PatternSprite* secondPattern, bool isRecover)
{
	int fpRow,fpCol,spRow,spCol;

    fpRow = firstPattern->m_nRowIndex;
    fpCol = firstPattern->m_nColIndex;
    spRow = secondPattern->m_nRowIndex;
    spCol = secondPattern->m_nColIndex;

    firstPattern->g_pSwapPattern = secondPattern;
    secondPattern->g_pSwapPattern = firstPattern;

    firstPattern->g_bIsRecover = isRecover;
    secondPattern->g_bIsRecover = isRecover;

    firstPattern->swapTo(gPatternsSwapTime, mPatternsPos[spRow][spCol]);
    secondPattern->swapTo(gPatternsSwapTime, mPatternsPos[fpRow][fpCol]);

    firstPattern->m_nRowIndex = spRow;
    firstPattern->m_nColIndex = spCol;
    secondPattern->m_nRowIndex = fpRow;
    secondPattern->m_nColIndex = fpCol;

    mPatternsSpr[fpRow][fpCol] = secondPattern;
    mPatternsSpr[spRow][spCol] = firstPattern;
	
	runAction(CCSequence::create(CCDelayTime::create(gPatternsSwapTime), CCCallFuncND::create(this, callfuncND_selector(PatternMatrix::onSwapFinish), firstPattern), NULL));
}

void PatternMatrix::onSwapFinish(CCNode* pNode, void* pPattern)
{
	PatternSprite* pfPattern = (PatternSprite*) pPattern;
    PatternSprite* psPattern = pfPattern->g_pSwapPattern;

    pfPattern->g_ePatternStatus = Status_Normal;
    psPattern->g_ePatternStatus = Status_Normal;

    if (pfPattern->g_bIsRecover)
	{
        onClearFinish(NULL, NULL);
	}
    else
	{
        int** matrixMark = createIntArray(m_nMatrixRow, m_nMatrixCol, 0);

		//如果交换之后 可以消除，那么执行消除动画.
		if (getResultByPoint(pfPattern->m_nRowIndex, pfPattern->m_nColIndex, matrixMark) 
			| getResultByPoint(psPattern->m_nRowIndex, psPattern->m_nColIndex, matrixMark))
		{
            clearSomePatterns(matrixMark);
		}
        else //没有匹配成功.
		{
            swapTwoPattern(pfPattern, psPattern, true);

            //gSharedEngine.playEffect(EFFECT_PATTERN_UN_SWAP);
        }

		delete []matrixMark;
    }
}

//检测是循环检测的.
//先检测整个-> 然后清除 -> 然后继续检测.
void PatternMatrix::detectionMatrix()
{
	if(!mUpdateLogic)
        return;

	int** matrixMark = createIntArray(m_nMatrixRow, m_nMatrixCol, 0);
    for (int col=0; col< m_nMatrixCol; col++)
    {
        for (int row=0; row < m_nMatrixRow; row++)
		{
            getResultByPoint(row, col, matrixMark);
		}
    }

    if (clearSomePatterns(matrixMark) == 0)
	{
        bool bRet = true;
        for (int col = 0; col < m_nMatrixCol && bRet; col++)
        {
            for (int row=0; row < m_nMatrixRow && bRet; row++)
            {
                if (mPatternsSpr[row][col] == NULL || mPatternsSpr[row][col]->g_ePatternStatus != Status_Normal)
                    bRet = false;
            }
        }

        if (bRet)
            excludeDeadlock();
    }

	delete [] matrixMark;
}

int PatternMatrix::clearSomePatterns(int** matrix)
{
	int tally = 0;
    mDestroyBatchTally++;

    for (int row = 0; row < m_nMatrixRow; row++)
    {
        for (int col = 0; col < m_nMatrixCol; col++)
        {
            if(mPatternsSpr[row][col] == NULL || mPatternsSpr[row][col]->g_ePatternStatus != Status_Normal)
                continue;

            switch(matrix[row][col])
			{
                case 1:
                    mPatternsSpr[row][col]->destroyPattern(mDestroyFrames);
                    mPatternsSpr[row][col]->g_nRemoveBatchIndex = mDestroyBatchTally;
                    tally++;
                    break;
                case 2:
                    mPatternsSpr[row][col]->removeFreeze();
                    break;
                case 3:
                    mPatternsSpr[row][col]->explodePattern(mExplodeFrames);
                    mPatternsSpr[row][col]->g_nRemoveBatchIndex = mDestroyBatchTally;
                    tally++;
                    break;
                default:
                    break;
            }
        }
    }

    if (tally != 0)
	{
        updateScore(tally);
        updateProgress();

        if(mMultipleTimer > 0.0)
            mScoreMultiple++;

        mMultipleTimer = 3.0;

		this->runAction(CCSequence::create(CCDelayTime::create(gPatternsClearTime), CCCallFuncND::create(this, callfuncND_selector(PatternMatrix::onClearFinish), (void*)&mDestroyBatchTally), NULL));
    }
	else
	{
		//return 0;
	}

    return tally;
}

void PatternMatrix::onClearFinish(CCNode* pnode, void* removeIndex)
{
	if(!removeIndex)
		return;

	int removeBatchIndex = *((int*)removeIndex);
    int row,col;

	//将要重新设置的区域清空值.
    for ( col=0; col<m_nMatrixCol && removeBatchIndex; col++)
    {
        for ( row=0; row<m_nMatrixRow; row++)
        {
			if(!mPatternsSpr[row][col])
			{

			}
			else
			{
				if(mPatternsSpr[row][col]->g_nRemoveBatchIndex == removeBatchIndex)
				{
					if (mPatternsSpr[row][col])
					{
						mPatternBatchNode->removeChild(mPatternsSpr[row][col], true);
						mPatternsSpr[row][col] = NULL;
					}
				}
			}
        }
    }

    for ( col=0; col < m_nMatrixCol; col++)
    {
        for ( row=0; row < m_nMatrixRow; row++)
        {
            if (row == 0 && mPatternsSpr[row][col] && mPatternsSpr[row][col]->m_eExtraAttr == Attr_Stone)
            {
				mPatternsSpr[row][col]->runAction(CCSequence::create(CCMoveBy::create(gPatternsFallTime, ccp(0.0, -400.0)), CCCallFuncN::create(this, callfuncN_selector(PatternMatrix::removeNode)), NULL));
                mPatternsSpr[row][col] = NULL;
            }

            if (mPatternsSpr[row][col] == NULL)
            {
                int notnull_r = -1;
                for (int n = row + 1; n < m_nMatrixRow; n++)
                {
                    if (mPatternsSpr[n][col] != NULL)
                    {
                        if (row == 0 && mPatternsSpr[n][col]->m_eExtraAttr == Attr_Stone)
                        {
							mPatternsSpr[n][col]->runAction(CCSequence::create(CCMoveBy::create(gPatternsFallTime, ccp(0.0, -400.0)), CCCallFuncN::create(this, callfuncN_selector(PatternMatrix::removeNode)), NULL));
                            mPatternsSpr[n][col] = NULL;
                        }
                        else
                        {
                            notnull_r = n;
                            break;
                        }
                    }
                }

                if (notnull_r != -1)
                {
                    if (mPatternsSpr[notnull_r][col]->g_ePatternStatus != Status_Normal)
                    {
                        row = m_nMatrixRow;
                        break;
                    }

                    if(mPatternsSpr[notnull_r][col] == mFirstCheckPattern)
					{
                        mCheckMarkSpr->setPosition(ccp(-100.0, -100.0));
                        mFirstCheckPattern = NULL;
                    }

                    mPatternsSpr[notnull_r][col]->moveTo((notnull_r - row) * 0.1, mPatternsPos[row][col]);
                    
					mPatternsSpr[row][col] = mPatternsSpr[notnull_r][col];
                    mPatternsSpr[row][col]->m_nRowIndex = row;
                    mPatternsSpr[row][col]->m_nColIndex = col;

                    mPatternsSpr[notnull_r][col] = NULL;
                }
            }
        }
    }

    for ( col=0; col < m_nMatrixCol; col++)
    {
        for ( row = m_nMatrixRow - 1; row >= 0; row--)
		{
			if (mPatternsSpr[row][col] == NULL)
			{
				addOnePattern(row,col);
			}
			else
				break;
        }
    }

	//消除连接的, 添加新的, 完成之后,检测是否又可以消除了.
	this->runAction(CCSequence::create(CCDelayTime::create(0.65), CCCallFunc::create(this, callfunc_selector(PatternMatrix::detectionMatrix)), NULL));
}

void PatternMatrix::removeNode(CCNode* child)
{
    mPatternBatchNode->removeChild(child, true);
}

CCPoint** PatternMatrix::createIntArray(int arow, int acol, CCPoint defValue)
{
    CCPoint** arr = new CCPoint*[arow];
    for (int row = 0; row < arow; row++ )
    {
        arr[row] = new CCPoint [acol];
        for (int col = 0; col < acol; col++)
		{
            arr[row][col] = defValue;
        }
    }
    return arr;
}

int** PatternMatrix::createIntArray(int arow, int acol, int defValue)
{
	int** arr = new int*[arow];
    for (int row = 0; row < arow; row++ )
    {
        arr[row] = new int [acol];
        for (int col = 0; col < acol; col++)
		{
            arr[row][col] = defValue;
        }
    }
    return arr;
}

PatternSprite*** PatternMatrix::createIntArrays(int arow, int acol, PatternSprite* defValue)
{
	PatternSprite*** arr = new PatternSprite**[arow];
    for (int row = 0; row < arow; row++ )
    {
        arr[row] = new PatternSprite* [acol];
        for (int col = 0; col < acol; col++)
		{
            arr[row][col] = defValue;
        }
    }

	return arr;
}

void PatternMatrix::updateTimerForPrompt(float dt)
{
	mPromptTimerTally += dt;

    if (mMultipleTimer > 0.0)
        mMultipleTimer -= dt;
    else
	{
        mMultipleTimer = 0.0;
        mScoreMultiple = 1;
    }

    if (gGameMode == Timer)
    {
        mTimerTally += dt;
        updateProgress();
    }

    if (mPromptTimerTally >= 10.0)
    {
        bool bRet = true;
        for (int col=0; col < m_nMatrixCol && bRet; col++)
        {
            for (int row=0; row < m_nMatrixRow && bRet; row++)
            {
				if(!mPatternsSpr[row][col] || mPatternsSpr[row][col]->g_ePatternStatus != Status_Normal)
                    bRet = false;
            }
        }

        if (bRet)
		{
            mPromptTimerTally = 0;
            isHasSolution();

            mPromptMarkSpr->setPosition(mPromptPattern->getPosition());
            mPromptPattern = NULL;
        }
    }
}

//提交得分.
void PatternMatrix::updateScore(float patternTally)
{
	if(!mUpdateLogic)
        return;

    mPatternClearTally += patternTally;

    mGameScore += patternTally * 100 * mScoreMultiple;       

	char str[30];
	sprintf(str, "Score %d", mGameScore);

    mScoreLabel->setString(str);
}

void PatternMatrix::updateProgress()
{
	if(!mUpdateLogic)
            return;

	switch(gGameMode)
	{
		case Challenge:
		{
			float penergyPercent = mPatternClearTally / mPatternRequire;

			if (penergyPercent > 1.0)
				penergyPercent = 1.0;
			else if(penergyPercent <0.0)
				penergyPercent = 0.0;

			float vh = 326 * penergyPercent;
			
			mVisibleRect = CCRect(0, 326-vh, 18, vh);
			mProgressSpr->setTextureRect(mVisibleRect);

			if (penergyPercent == 1.0)
				showGameResult(true);

			break;
		}
		case Timer:
		{
			float penergyPercent = (mTimeTotal - mTimerTally) / mTimeTotal;

			if (penergyPercent > 1.0)
				penergyPercent = 1.0;
			else if(penergyPercent <0.0)
				penergyPercent = 0.0;

			float vw = 700 * penergyPercent;

			mVisibleRect = CCRect(0, 0, vw, 52);
			mProgressSpr->setTextureRect(mVisibleRect);

			if (penergyPercent == 0.0)
			{
				if(mPatternClearTally >= mPatternRequire)
					showGameResult(true);
				else
					showGameResult(false);
			}
			break;
		}
	}
}

void PatternMatrix::stopGameLogic()
{
	this->unscheduleAllSelectors();

	mUpdateLogic = false;
}

void PatternMatrix::showGameResult(bool isPass)
{
	stopGameLogic();

	ResultLayer* resultLayer = ResultLayer::create();
    //resultLayer->initResultData(mGameScore, mPatternRequire * 100 * 3, gScoreData->bestScore, isPass);
	resultLayer->initResultData(mGameScore, mPatternRequire * 100 * 3, 1000, isPass);

    this->onExit();

    //gScoreData->setLastScore(mGameScore);

    /*if(isPass)
        gSharedEngine.playEffect(EFFECT_GAME_WIN);
    else
        gSharedEngine.playEffect(EFFECT_GAME_FAIL);*/

	CCDirector::sharedDirector()->getRunningScene()->addChild(resultLayer,99);
}

// 这里是统计横竖方向的连续情况，即可消除的元素.
// 根据炸弹，冰冻的，普通的，分别标记为不同的值.
// 返回值表示是否有可消除的元素.
bool PatternMatrix::getResultByPoint(int row, int col, int** matrixMark)
{
	if(mPatternsSpr[row][col] == NULL)
		return false;

	int targetType = mPatternsSpr[row][col]->m_ePatternType;

	if(targetType == -1 || mPatternsSpr[row][col]->g_ePatternStatus != Status_Normal)
		return false;

	bool bRet = false;
	int count = 1;
	int start = col;
	int end = col;

	int i = col-1;
	while (i >= 0) //左侧统计连续个数.
	{
		if (mPatternsSpr[row][i] && mPatternsSpr[row][i]->g_ePatternStatus== Status_Normal
			&& mPatternsSpr[row][i]->m_ePatternType == targetType)
		{
			count++;
			start = i;
		}
		else
			break;
		i--;
	}

	i = col+1;
	while (i < m_nMatrixCol)  //右侧统计连续个数.
	{
		if (mPatternsSpr[row][i] && mPatternsSpr[row][i]->g_ePatternStatus==Status_Normal &&
			mPatternsSpr[row][i]->m_ePatternType == targetType)
		{
			count++;
			end = i;
		}
		else
			break;
		i++;
	}

	if (count >= MATRIX_CONTINUOUS)
	{
		for (i = start; i <= end; i++)
		{
			switch(mPatternsSpr[row][i]->m_eExtraAttr)
			{
			case Attr_Bomb:
				{
					matrixMark[row][i] = 3;
					if(i > 0)
					{
						matrixMark[row][i-1] = 3;
						
						if(row > 0)
						{
							matrixMark[row-1][i-1] = 3;
							matrixMark[row-1][i] = 3;
						}

						if(row+1 < m_nMatrixRow)
						{
							matrixMark[row+1][i-1] = 3;
							matrixMark[row+1][i] = 3;
						}
					}

					if(i+1 < m_nMatrixCol)
					{
						matrixMark[row][i+1] = 3;

						if(row > 0)
							matrixMark[row-1][i+1] = 3;
						if(row+1 < m_nMatrixRow)
							matrixMark[row+1][i+1] = 3;
					}
					break;
				}
			case Attr_Freeze:
				if(matrixMark[row][i] != 3)
					matrixMark[row][i] = 2;
				break;
			default:
				if(matrixMark[row][i] == 0)
					matrixMark[row][i] = 1;
				break;
			}
		}
		bRet = true;
	}

	// Vertical
	count = 1;
	start = row;
	i = row-1;

	while (i >= 0)
	{
		if (mPatternsSpr[i][col] && mPatternsSpr[i][col]->g_ePatternStatus == Attr_Normal &&
			mPatternsSpr[i][col]->m_ePatternType == targetType)
		{
			count++;
			start = i;
		}
		else
			break;
		i--;
	}

	end = row;
	i = row+1;
	while (i < m_nMatrixRow)
	{
		if (mPatternsSpr[i][col] && mPatternsSpr[i][col]->g_ePatternStatus== Attr_Normal &&
			mPatternsSpr[i][col]->m_ePatternType == targetType)
		{
			count++;
			end = i;
		}
		else
			break;
		i++;
	}

	if (count >= MATRIX_CONTINUOUS)
	{
		for (i = start; i <= end; i++)
		{
			switch(mPatternsSpr[i][col]->m_eExtraAttr)
			{
			case Attr_Bomb:
				{
					matrixMark[i][col] = 3;
					if(col > 0)
					{
						matrixMark[i][col-1] = 3;
						if(i > 0)
						{
							matrixMark[i-1][col-1] = 3;
							matrixMark[i-1][col] = 3;
						}
						if(i+1 < m_nMatrixRow)
						{
							matrixMark[i+1][col-1] = 3;
							matrixMark[i+1][col] = 3;
						}
					}

					if(col+1 < m_nMatrixCol)
					{
						matrixMark[i][col+1] = 3;
						if(i > 0)
							matrixMark[i-1][col+1] = 3;
						if(i+1 < m_nMatrixRow)
							matrixMark[i+1][col+1] = 3;
					}
					break;
				}
			case Attr_Freeze:
				if(matrixMark[i][col] != 3)
					matrixMark[i][col] = 2;
				break;
			default:
				if(matrixMark[i][col] == 0)
					matrixMark[i][col] = 1;
				break;
			}
		}

		bRet = true;
	}

	return bRet;
}

bool PatternMatrix::isHasSolution()
{
	int targetType = 0;

    for (int row = 0; row < m_nMatrixRow; row++)
    {
        for (int col = 0; col < m_nMatrixCol-1; col++)
        {
            if (mPatternsSpr[row][col]->m_eExtraAttr != Attr_Stone)
            {
                targetType = mPatternsSpr[row][col]->m_ePatternType;

                if (targetType == mPatternsSpr[row][col+1]->m_ePatternType)
                {
                    //  *
                    //**
                    //  *
                    if ( row > 0 && col + 2 < m_nMatrixCol && mPatternsSpr[row-1][col+2]->m_ePatternType == targetType)
					{
                        if(mPatternsSpr[row][col+2]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row][col+2];
                            return true;
                        }
                        if(mPatternsSpr[row-1][col+2]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row-1][col+2];
                            return true;
                        }
                    }

                    if ( row+1<m_nMatrixRow && col+2<m_nMatrixCol && mPatternsSpr[row+1][col+2]->m_ePatternType == targetType )
					{
                        if(mPatternsSpr[row][col+2]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row][col+2];
                            return true;
                        }
                        if(mPatternsSpr[row+1][col+2]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row+1][col+2];
                            return true;
                        }
                    }

                    //*
                    // **
                    //*
                    if (row>0 && col>0 && mPatternsSpr[row-1][col-1]->m_ePatternType == targetType )
                    {
                        if(mPatternsSpr[row][col-1]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row][col-1];
                            return true;
                        }
                        if(mPatternsSpr[row-1][col-1]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row-1][col-1];
                            return true;
                        }
                    }
                    if (row+1<m_nMatrixRow && col>0 && mPatternsSpr[row+1][col-1]->m_ePatternType == targetType )
                    {
                        if(mPatternsSpr[row][col-1]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row][col-1];
                            return true;
                        }
                        if(mPatternsSpr[row+1][col-1]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row+1][col-1];
                            return true;
                        }
                    }

                    //*-**-*
                    if (col - 2 >= 0 && mPatternsSpr[row][col-2]->m_ePatternType == targetType )
                    {
                        if(mPatternsSpr[row][col-2]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row][col-2];
                            return true;
                        }
                        if(mPatternsSpr[row][col-1]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row][col-1];
                            return true;
                        }
                    }
                    if (col + 3 < m_nMatrixCol && mPatternsSpr[row][col+3]->m_ePatternType == targetType )
                    {
                        if(mPatternsSpr[row][col+3]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row][col+3];
                            return true;
                        }
                        if(mPatternsSpr[row][col+2]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row][col+2];
                            return true;
                        }
                    }
                }
            }
        }

        // x x |  x
        //  x  | x x
        for (int col = 0; col < m_nMatrixCol - 2; col++)
        {
            if (mPatternsSpr[row][col]->m_eExtraAttr != Attr_Stone)
			{
                targetType = mPatternsSpr[row][col]->m_ePatternType;
                if (targetType == mPatternsSpr[row][col+2]->m_ePatternType)
                {
                    if ( row>0 && targetType == mPatternsSpr[row-1][col+1]->m_ePatternType )
                    {
                        if(mPatternsSpr[row-1][col+1]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row-1][col+1];
                            return true;
                        }
                        if(mPatternsSpr[row][col+1]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row][col+1];
                            return true;
                        }
                    }

                    if ( row+1<m_nMatrixRow && targetType == mPatternsSpr[row+1][col+1]->m_ePatternType )
                    {
                        if(mPatternsSpr[row+1][col+1]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row+1][col+1];
                            return true;
                        }
                        if(mPatternsSpr[row][col+1]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row][col+1];
                            return true;
                        }
                    }
                }
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------------
    for (int col = 0; col < m_nMatrixCol; col++)
    {
        for (int row = 0; row < m_nMatrixRow - 1; row++)
        {
            targetType = mPatternsSpr[row][col]->m_ePatternType;
            if (mPatternsSpr[row][col]->m_eExtraAttr != Attr_Stone && targetType == mPatternsSpr[row+1][col]->m_ePatternType)
			{
                // ? ?	
                //  x
                //  x
                if (col>0 && row+2<m_nMatrixRow && mPatternsSpr[row+2][col-1]->m_ePatternType == targetType )
                {
                    if(mPatternsSpr[row+2][col-1]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row+2][col-1];
                        return true;
                    }
                    if(mPatternsSpr[row+2][col]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row+2][col];
                        return true;
                    }
                }
                if (col+1<m_nMatrixCol && row+2<m_nMatrixRow && mPatternsSpr[row+2][col+1]->m_ePatternType == targetType )
                {
                    if(mPatternsSpr[row+2][col+1]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row+2][col+1];
                        return true;
                    }
                    if(mPatternsSpr[row+2][col]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row+2][col];
                        return true;
                    }
                }

                //  x
                //  x
                // ? ?
                if (col>0 && row>0 && mPatternsSpr[row-1][col-1]->m_ePatternType == targetType )
                {
                    if(mPatternsSpr[row-1][col-1]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row-1][col-1];
                        return true;
                    }
                    if(mPatternsSpr[row-1][col]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row-1][col];
                        return true;
                    }
                }
                if (col+1<m_nMatrixCol && row>0 && mPatternsSpr[row-1][col+1]->m_ePatternType == targetType )
                {
                    if(mPatternsSpr[row-1][col+1]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row-1][col+1];
                        return true;
                    }
                    if(mPatternsSpr[row-1][col]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row-1][col];
                        return true;
                    }
                }

                //* ** *
                if (row - 2 >= 0 && mPatternsSpr[row-2][col]->m_ePatternType == targetType )
                {
                    if(mPatternsSpr[row-2][col]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row-2][col];
                        return true;
                    }
                    if(mPatternsSpr[row-1][col]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row-1][col];
                        return true;
                    }
                }
                if (row + 3 < m_nMatrixRow && mPatternsSpr[row+3][col]->m_ePatternType == targetType )
                {
                    if(mPatternsSpr[row+3][col]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row+3][col];
                        return true;
                    }
                    if(mPatternsSpr[row+2][col]->m_bSwapEnable)
                    {
                        mPromptPattern = mPatternsSpr[row+2][col];
                        return true;
                    }
                }
            }
        }

        //  x | x
        // x  |  x
        //  x | x
        for (int row = 0; row < m_nMatrixRow - 2; row++)
        {
            if (mPatternsSpr[row][col]->m_eExtraAttr != Attr_Stone)
            {
                targetType = mPatternsSpr[row][col]->m_ePatternType;
                if (targetType == mPatternsSpr[row+2][col]->m_ePatternType)
                {
                    if (col>0 && targetType == mPatternsSpr[row+1][col-1]->m_ePatternType )
                    {
                        if(mPatternsSpr[row+1][col-1]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row+1][col-1];
                            return true;
                        }

                        if(mPatternsSpr[row+1][col]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row+1][col];
                            return true;
                        }
                    }
                    if (col+1<m_nMatrixCol && targetType == mPatternsSpr[row+1][col+1]->m_ePatternType )
                    {
                        if(mPatternsSpr[row+1][col+1]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row+1][col+1];
                            return true;
                        }

                        if(mPatternsSpr[row+1][col]->m_bSwapEnable)
                        {
                            mPromptPattern = mPatternsSpr[row+1][col];
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}