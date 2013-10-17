#ifndef _H_PATTERNMATRIX_
#define _H_PATTERNMATRIX_

#include "GameBasic.h"
#include "PatternSprite.h"
#include "ResultLayer.h"

class PatternMatrix : public CCLayer
{
public:

	PatternMatrix();
	~PatternMatrix();

	static PatternMatrix* createWithGameMode(eGameMode mode);

	bool initWithGameMode(eGameMode mode);

	virtual void onEnterTransitionDidFinish();

public:

	void InitData();

	void initProgressWithGameMode();

	void initLabels();

	void initMatrix();

	void initArrayFrames();

public:

	void updateScore(float patternTally);

	void updateTimerForPrompt(float dt);

	void updateProgress();

	CCPoint** createIntArray(int row, int col, CCPoint defValue);
	int** createIntArray(int row, int col, int defValue);

	PatternSprite*** createIntArrays(int row, int col, PatternSprite* defValue);

	int clearSomePatterns(int** matrix);

	void addOnePattern(int row, int col);


	void onCheckPattern(PatternSprite* pPattern);
	void onSwapTwoPattern(PatternSprite* pPattern);
	void swapTwoPattern(PatternSprite* firstPattern, PatternSprite* secondPattern, bool isRecover);
	void onSwapFinish(CCNode* pNode, void* pPattern);


	void onClearFinish(CCNode* pNode, void* index);

	void removeNode(CCNode* child);

	void detectionMatrix();
	void excludeDeadlock();

	void showGameResult(bool result);
	void stopGameLogic();

	bool getResultByPoint(int row, int col, int** matrixMark);

	bool isHasSolution();


public:
	float mPatternRequire;

	CC_SYNTHESIZE(eGameMode, gGameMode, GGameMode);

private:
	CCSprite* mPromptPattern;
	CCSprite* mProgressBgSpr;
	CCSprite* mProgressSpr;
	CCSprite* mCheckMarkSpr;
	CCSprite* mPromptMarkSpr;

	PatternSprite* mFirstCheckPattern, *mSecondCheckPattern;

	CCRenderTexture* PatternBg;

	CCSpriteBatchNode* mPatternBatchNode;

	CCLabelTTF* mScoreLabel;

	CCSize winSize;

	CCRect mVisibleRect;

private:

	float mTimeTotal, mTimerTally, mPatternClearTally, mPromptTimerTally, mMultipleTimer;

	int m_nMatrixRow, m_nMatrixCol, mScoreMultiple, mGameScore, mDestroyBatchTally;

	int m_nFreezeProbability, m_nStoneProbability, m_nBombProbability;

	int mPatternTypeMax;

	CCPoint** mPatternsPos;
	PatternSprite*** mPatternsSpr;
	int** matrixMark;

	bool mUpdateLogic;

	CCArray* mDestroyFrames, *mExplodeFrames;
};

#endif