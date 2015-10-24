#include "HelloWorldScene.h"

USING_NS_CC;

Scene* HelloWorld::createScene() {
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = HelloWorld::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init() {
	//////////////////////////////
	// 1. super init first
	if (!Layer::init()) {
		return false;
	}
	this->setKeypadEnabled(true); //allows us to close by using back (android)
	touched = false;	//handling for multiple screen touches
	isMoving = false;
	totalMoveDistance = 0;
	distancePast = 0;
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	visibleSize = Director::getInstance()->getVisibleSize();	//gets the screen size of device

	// adds our grass background
	bg = Sprite::create("bg.jpg");
	bgSize = bg->getContentSize();	//gets size of bg image
	bg->setPosition(Vec2(bgSize.width / 2 + origin.x, bgSize.height / 2 + origin.y));

	// add the background as a child to this layer
	this->addChild(bg, 0);


	drawNode = DrawNode::create();	//for drawing our red lines
	drawNode->setPosition(0, 0);

	this->addChild(drawNode, 1);	//add to layer

	SpriteFrameCache* cache = SpriteFrameCache::getInstance();	//loads in our spritesheets
	cache->addSpriteFramesWithFile("sphere.plist");

	ball = Sprite::createWithSpriteFrameName("sphere0.png");	//sets the starting image
	ball->setPosition(Vec2(bgSize.width / 2 + origin.x, bgSize.height / 2 + origin.y));
	this->addChild(ball, 2);	//adds ball to layer

	Vector<SpriteFrame*> animFrames(100);	//our sprite sheet has 100 frames

	char str[100] = { 0 };

	for (int i = 0; i < 100; i++) {
		sprintf(str, "sphere%d.png", i);
		SpriteFrame* frame = cache->getSpriteFrameByName(str);
		animFrames.pushBack(frame);
	}

	//Animation is added to the ball, for the moment we don't play the animation as we're not moving
	Animation *animation = Animation::createWithSpriteFrames(animFrames, 0.01f);
	animateAction = Speed::create(RepeatForever::create(Animate::create(animation)), 0.0f);
	ball->runAction(animateAction);


	auto listener = EventListenerTouchOneByOne::create();	//allows us to listen for touches
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	Rect rect(0, 0, bgSize.width, bgSize.height);	//sets the limits that camera will follow ball for
	auto followAction = Follow::create(ball, rect);
	this->runAction(followAction);	//camera always tries to focus on ball
	this->scheduleUpdate();

	return true;
}

bool HelloWorld::onTouchBegan(Touch *touch, Event *event) {
	if (!touched) {	//only perform logic is no other touch is currently active
		this->stopAnimation();
		touched = true;
		pointsOnLine.clear();	//emptys our red line points
		ball->stopAction(moveSequence);	//stops any running moves
		isMoving = false;
		totalMoveDistance = 0;
		distancePast = 0;
		/*CCLOG("ball x = %f, y = %f", ball->getPositionX(), ball->getPositionY());
		CCLOG("abs x = %f, y = %f", this->getPositionX(), this->getPositionY());
		CCLOG("onTouchBegan x = %f, y = %f", touch->getLocation().x, touch->getLocation().y);*/
		drawNode->clear();	//clears any previous lines
		firstPt = this->getAbsPos(touch->getLocation());	//records the first touch point
		lastPt = firstPt;	//allows us to draw our red line
		return true;
	}
	return false;
}

void HelloWorld::onTouchMoved(Touch *touch, Event *event) {
	//CCLOG("onTouchMoved x = %f, y = %f", touch->getLocation().x, touch->getLocation().y);
	this->draw(drawNode, touch->getLocation());	//draws the red line as touch drags
	pointsOnLine.push_back(this->getAbsPos(touch->getLocation()));	//record points so ball can later roll over
}

void HelloWorld::onTouchEnded(Touch *touch, Event *event) {
	this->move(firstPt);	//calls our move function
	touched = false; //allows a new touch to take place
}

void HelloWorld::move(const Vec2& pos) {
	Vector<FiniteTimeAction *> moves;	//stores all our moves
	std::vector<Vec2> points = this->getPointsOnLine(ball->getPosition(), pos);	//points on way to tap
	if(pointsOnLine.size() > 1 )	//only add for important drags
		points.insert(points.end(), pointsOnLine.begin(), pointsOnLine.end());

	if (points.size() > 1)
		moves.pushBack(this->createMoveAction(ball->getPosition(), points[0], 1)); //first move on red line
	for (int i = 1; i < points.size(); i++) {
		moves.pushBack(this->createMoveAction(points[i - 1], points[i], i)); //all subsequent red line moves
	}

	moves.pushBack(CallFuncN::create(CC_CALLBACK_0(HelloWorld::stopAnimation, this))); //call back function to stop anim
	moves.pushBack(CallFuncN::create(CC_CALLBACK_0(HelloWorld::clearLines, this)));	//call back to clean lines
	moves.pushBack(CallFuncN::create(CC_CALLBACK_0(HelloWorld::stopMoving, this)));
	moveSequence = Speed::create(Sequence::create(moves), 1.0f);
	ball->runAction(moveSequence);
	isMoving = true;
}

void HelloWorld::draw(cocos2d::DrawNode* node, const cocos2d::Vec2& pos) {
	Vec2 absPos = this->getAbsPos(pos);
	if (absPos.x > bgSize.width - BORDER_SIZE ) //insure lines stay within bounds
		absPos.x = bgSize.width - BORDER_SIZE;
	if (absPos.x < 0 + BORDER_SIZE)
		absPos.x = 0 + BORDER_SIZE;
	if (absPos.y > bgSize.height - BORDER_SIZE )
		absPos.y = bgSize.height - BORDER_SIZE ;
	if (absPos.y < 0 + BORDER_SIZE)
		absPos.y = 0 + BORDER_SIZE;

	node->drawSegment(lastPt, absPos, 2, Color4F::RED); //connect the dots..

	lastPt = absPos;
}
//gets the absolute position of a touch
Vec2 HelloWorld::getAbsPos(const Vec2& pos) {
	Vec2 diff;
	Vec2 middle;
	middle.set(visibleSize.width / 2, visibleSize.height / 2);
	Vec2::subtract(pos, middle, &diff);

	Vec2 addedVec;
	if (ball->getPositionX() > (bgSize.width - BORDER_SIZE - (visibleSize.width / 2))) //ensures the ball stays on the screen
		addedVec.x = (bgSize.width - BORDER_SIZE  - (visibleSize.width / 2) + diff.x);
	else if (ball->getPositionX() < (0 + BORDER_SIZE + (visibleSize.width / 2)))
		addedVec.x = (0 + BORDER_SIZE +  (visibleSize.width / 2) + diff.x);
	else
		addedVec.x = ball->getPositionX() + diff.x;
	if (ball->getPositionY() > (bgSize.height - BORDER_SIZE  - (visibleSize.height / 2)))
		addedVec.y = (bgSize.height - BORDER_SIZE  - (visibleSize.height / 2) + diff.y);
	else if (ball->getPositionY() < (0 + BORDER_SIZE + (visibleSize.height / 2)))
		addedVec.y = (0 + BORDER_SIZE + (visibleSize.height / 2) + diff.y);
	else
		addedVec.y = ball->getPositionY() + diff.y;
	return addedVec;
}
// get angle between two points in degrees
float HelloWorld::getAngle(const cocos2d::Vec2& p1, const cocos2d::Vec2& p2) {
	Vec2 diff;
	Vec2::subtract(p2, p1, &diff);
	//CCLOG("diff  x = %f, y = %f", diff.x, diff.y);
	float angleRadians = atan2f(diff.x, diff.y);
	float angleDegrees = CC_RADIANS_TO_DEGREES(angleRadians);
	return angleDegrees;
}
//creates a move action, handles rotate, move and animation
cocos2d::Sequence* HelloWorld::createMoveAction(const cocos2d::Vec2& p1,
		const cocos2d::Vec2& p2, int num) {

	Vec2 newPos = p2;
	//makes sure we stay inbounds
	if (newPos.x > bgSize.width - BORDER_SIZE  - ball->getContentSize().width / 2)
		newPos.x = bgSize.width - BORDER_SIZE  - ball->getContentSize().width / 2;
	if (newPos.x <  ball->getContentSize().width / 2 + BORDER_SIZE)
		newPos.x = ball->getContentSize().width / 2 + BORDER_SIZE;
	if (newPos.y > bgSize.height - BORDER_SIZE  - ball->getContentSize().width / 2)
		newPos.y = bgSize.height - BORDER_SIZE  - ball->getContentSize().width / 2;
	if (newPos.y < ball->getContentSize().width / 2 + BORDER_SIZE)
		newPos.y = ball->getContentSize().width / 2 + BORDER_SIZE;
	float distance = newPos.getDistance(p1); //distance used to adjust speed
	totalMoveDistance += distance;

	//CCLOG("moving to  x = %f, y = %f", newPos.x, newPos.y);

	float angle = this->getAngle(p1, p2);
	auto rotate = RotateTo::create(0, angle - 90); //rotate to face point
	float speed = distance/600;

	auto moveAction = MoveTo::create(speed, newPos);
	Sequence *sequence;
	sequence = Sequence::create(rotate, moveAction, CallFuncN::create(CC_CALLBACK_0(HelloWorld::advanceDistance, this, distance)), NULL);
	return sequence;
}

void HelloWorld::stopAnimation() {
	animateAction->setSpeed(0.0f);
}

void HelloWorld::setAnimationSpeed(const float speed) {
	animateAction->setSpeed(speed);
}

void HelloWorld::advanceDistance(const float distance){
	distancePast += distance;
}

void HelloWorld::clearLines() {
	drawNode->clear();
}

void HelloWorld::stopMoving() {
	isMoving = false;
}

void HelloWorld::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)	//used to escape on android
{
    if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
    {
         CCLOG("You pressed back button");
         Director::getInstance()->end();
         exit(0);
    }
}

void HelloWorld::update(float dt){
	if(isMoving){
		float amtComplete = distancePast/totalMoveDistance;
		CCLOG("dist %f", distancePast);
		if(distancePast < 300 && amtComplete < .5){
			this->setAnimationSpeed(distancePast/300 * 3.5);
			moveSequence->setSpeed(distancePast/300 + .1);
		}
		else if((totalMoveDistance - distancePast)  < 300){
			this->setAnimationSpeed(((totalMoveDistance - distancePast)/300) * 3.5);
			moveSequence->setSpeed((totalMoveDistance - distancePast)/300 + .1);
		}

	}
}

std::vector<Vec2> HelloWorld::getPointsOnLine(const cocos2d::Vec2 &pos1,const cocos2d::Vec2 &pos2){
	Vec2 newPos = pos2;
	if (newPos.x > bgSize.width - BORDER_SIZE  - ball->getContentSize().width / 2)
		newPos.x = bgSize.width - BORDER_SIZE  - ball->getContentSize().width / 2;
	if (newPos.x <  ball->getContentSize().width / 2 + BORDER_SIZE)
		newPos.x = ball->getContentSize().width / 2 + BORDER_SIZE;
	if (newPos.y > bgSize.height - BORDER_SIZE  - ball->getContentSize().width / 2)
		newPos.y = bgSize.height - BORDER_SIZE  - ball->getContentSize().width / 2;
	if (newPos.y < ball->getContentSize().width / 2 + BORDER_SIZE)
		newPos.y = ball->getContentSize().width / 2 + BORDER_SIZE;
	std::vector<Vec2> points;
	for(int i = 0; i <  static_cast<int>(pos2.getDistance(pos1)); i++){
		float x = pos1.x + ((i/pos2.getDistance(pos1)) * (newPos.x - pos1.x));
		float y = pos1.y + ((i/pos2.getDistance(pos1)) * (newPos.y - pos1.y));

		Vec2 pt(x, y);
		points.push_back(pt);
	}
	return points;
}

void HelloWorld::menuCloseCallback(Ref* pSender) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
	return;
#endif

	Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif
}
