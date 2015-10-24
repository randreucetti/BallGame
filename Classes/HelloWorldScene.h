#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include <algorithm>
#include "Block.h"

class HelloWorld: public cocos2d::Layer {
	//all delete for pointers is handled by cocos2d
private:
	const static int BORDER_SIZE = 10;
	cocos2d::Sprite *ball;
	cocos2d::Sprite *bg;
	cocos2d::Size visibleSize;
	cocos2d::Size bgSize;
	cocos2d::Vec2 firstPt;
	cocos2d::DrawNode* drawNode;
	cocos2d::Vec2 lastPt;
	cocos2d::Speed *animateAction;
	cocos2d::Speed *moveSequence;
	std::vector<cocos2d::Vec2> pointsOnLine;
	float totalMoveDistance;
	float distancePast;
	bool touched;
	bool isMoving;
public:
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	// a selector callback
	void menuCloseCallback(cocos2d::Ref* pSender);
	//handlers for touch
	bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event);
	void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event);
	void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event);
	//moves our ball
	void move(const cocos2d::Vec2& pos);
	cocos2d::Sequence* createMoveAction(const cocos2d::Vec2& p1,const cocos2d::Vec2& p2, int num);
	//draws red line
	void draw(cocos2d::DrawNode* node, const cocos2d::Vec2& pos);
	//gets absolute position from a touch
	cocos2d::Vec2 getAbsPos(const cocos2d::Vec2& pos);
	void stopAnimation();
	float getAngle(const cocos2d::Vec2& p1, const cocos2d::Vec2& p2);

	void setAnimationSpeed(const float speed);

	void clearLines();

	void stopMoving();

	void advanceDistance(const float distance);

	void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode,	cocos2d::Event *event);

	void update(float dt);

	std::vector<cocos2d::Vec2> getPointsOnLine(const cocos2d::Vec2 &pos1,const cocos2d::Vec2 &pos2);

	// implement the "static create()" method manually
	CREATE_FUNC(HelloWorld)
	;
};

#endif // __HELLOWORLD_SCENE_H__
