#include "HUDLayer.h"

USING_NS_CC;

HUDLayer* HUDLayer::create()
{
	HUDLayer* hud = new HUDLayer();
	if (hud && hud->init())
	{
		hud->autorelease();

		return hud;
	}
	CC_SAFE_DELETE(hud);

	return nullptr;
}

// on "init" you need to initialize your instance
bool HUDLayer::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	visibleSize = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();

	/////////////////////////////
	// 2. add a menu item with "X" image, which is clicked to quit the program
	//    you may modify it.
	
	// add a "close" icon to exit the progress. it's an autorelease object

	initSwordItem();
	initOmiSlashItem();
	initBladeStormItem();

	// create menu, it's an autorelease object
	cocos2d::Vector<MenuItem*> items;
	items.pushBack(swordItem);
	items.pushBack(omislashItem);
	items.pushBack(bladeStormItem);
	auto menu = Menu::createWithArray(items);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu, 1);

	//event
	//  Create a "one by one" touch event listener
	// (processes one touch at a time)
	auto listener1 = EventListenerTouchOneByOne::create();

	// trigger when you push down
	listener1->onTouchBegan = [=](Touch* touch, Event* event){
		isTouchDown = true;
		initTouchPos = Vec2(touch->getLocation().x, touch->getLocation().y);
		currentTouchPos = Vec2(touch->getLocation().x, touch->getLocation().y);
		if (canJump(currentTouchPos))
			player->jump();
			
		return true; // if you are consuming it
	};

	// trigger when moving touch
	listener1->onTouchMoved = [=](Touch* touch, Event* event){
		currentTouchPos = Vec2(touch->getLocation().x, touch->getLocation().y);
	};

	// trigger when you let up
	listener1->onTouchEnded = [=](Touch* touch, Event* event){
		if (player->getState() == ESTATE::JUMP || player->getState() == ESTATE::JUMP2)
			player->setHold(false);
		isTouchDown = false;
	};

	// Add listener
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener1, this);

	this->scheduleUpdate();
	return true;
}

void HUDLayer::update(float dt)
{
	if (isTouchDown == true)
	{
		if (initTouchPos.y - currentTouchPos.y > visibleSize.width * 0.05f)
		{
			CCLOG("SWIPED DOWN");
			player->flashDown();
			isTouchDown = false;
		}
		else if (initTouchPos.y - currentTouchPos.y < -visibleSize.width * 0.05f)
		{
			CCLOG("SWIPED UP");
			player->flashUp();
			isTouchDown = false;
		}
	}

	if (player->canAttack())
		this->swordItem->setEnabled(true);

	if (player->canOmiSlash())
		this->omislashItem->setEnabled(true);

	if (player->canBladeStorm())
		this->bladeStormItem->setEnabled(true);
}

void HUDLayer::slash(Ref* sender)
{
	if (player->canAttack()){
		swordItem->setEnabled(false);
		player->attack();
	}
}

void HUDLayer::omislash(Ref* sender)
{
	if (player->canOmiSlash()){
		omislashItem->setEnabled(false);
		player->attack();
	}
}

void HUDLayer::bladeStorm(Ref* sender)
{
	if (player->canBladeStorm()){
		bladeStormItem->setEnabled(false);
		player->attack();
	}
}

bool HUDLayer::canJump(Vec2 touchPoint)
{
	if (!swordItem->isEnabled()){
		if (intersect(swordRect, touchPoint))
		{
			return false;
		}
	}

	if (!bladeStormItem->isEnabled()){
		if (intersect(bladeRect, touchPoint))
		{
			return false;
		}
	}

	if (!omislashItem->isEnabled()){
		if (intersect(omiRect, touchPoint))
		{
			return false;
		}
	}
}

void HUDLayer::initSwordItem()
{
	visibleSize = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();
	swordItem = MenuItemImage::create(
		"UI\\b_knife.png",
		"UI\\b_knife_delay.png",
		CC_CALLBACK_1(HUDLayer::slash, this));

	swordItem->setPosition(visibleSize.width + origin.x - swordItem->getContentSize().width * 0.5f,
		0 + origin.y + swordItem->getContentSize().height * 0.5f);
	auto image = Sprite::create("UI\\b_knife_delay.png");
	swordItem->setDisabledImage(image);

	swordRect = new CCRect(swordItem->getPosition().x - swordItem->getContentSize().width/2
		, swordItem->getPosition().y - swordItem->getContentSize().height / 2
		, swordItem->getDisabledImage()->getContentSize().width
		, swordItem->getDisabledImage()->getContentSize().height);
}

void HUDLayer::initBladeStormItem()
{
	bladeStormItem = MenuItemImage::create(
		BLADE_STORM_PATH,
		SKILL_DISABLED_PATH,
		CC_CALLBACK_1(HUDLayer::bladeStorm, this));
	bladeStormItem->setPosition(swordItem->getPosition().x - swordItem->getContentSize().width / 2,
		swordItem->getPosition().y + omislashItem->getContentSize().height);
	auto bladeStormItemDisabledImage = Sprite::create(SKILL_DISABLED_PATH);
	bladeStormItem->setDisabledImage(bladeStormItemDisabledImage);
	bladeRect = new CCRect(bladeStormItem->getPosition().x - bladeStormItem->getContentSize().width/2
		, bladeStormItem->getPosition().y - bladeStormItem->getContentSize().height / 2
		, bladeStormItem->getDisabledImage()->getContentSize().width
		, bladeStormItem->getDisabledImage()->getContentSize().height);
}

void HUDLayer::initOmiSlashItem()
{
	omislashItem = MenuItemImage::create(
		OMISLASH_PATH,
		SKILL_DISABLED_PATH,
		CC_CALLBACK_1(HUDLayer::omislash, this));
	omislashItem->setPosition(swordItem->getPosition().x - swordItem->getContentSize().width / 2 - omislashItem->getContentSize().width / 2 - 3,
		swordItem->getPosition().y);
	auto omislashDisabledImage = Sprite::create(SKILL_DISABLED_PATH);
	omislashItem->setDisabledImage(omislashDisabledImage);

	omiRect = new CCRect(omislashItem->getDisabledImage()->getPosition().x - omislashItem->getContentSize().width/2
		, omislashItem->getPosition().y - omislashItem->getContentSize().height / 2
		, omislashItem->getDisabledImage()->getContentSize().width
		, omislashItem->getDisabledImage()->getContentSize().height);
}

bool HUDLayer::intersect(CCRect* rect, Vec2 point){
	bool a = rect->getMaxX() < point.x;
	float tmp = rect->getMinX();
	bool b = rect->getMinX() > point.x;
	bool c = rect->getMaxY() < point.y;
	bool d = rect->getMinY() > point.y;
	return !(a || b || c || d);
}