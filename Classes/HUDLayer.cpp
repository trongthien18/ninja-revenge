#include "HUDLayer.h"
#include "ui\CocosGUI.h"
#include "Dialog.h"
#include "Background.h"

USING_NS_CC;
using namespace ui;

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

	initHpBar();

	initSwordItem();
	initOmiSlashItem();
	initBladeStormItem();
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

	auto pauseBtn = Button::create("UI\\b_pause.png");
	pauseBtn->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type){
		switch (type)
		{
		case ui::Widget::TouchEventType::BEGAN:
			break;
		case ui::Widget::TouchEventType::ENDED:
			auto dialog = PauseDialog::createPauseDialog();
			dialog->setPosition(visibleSize.width * 0.5, visibleSize.height *0.4);
			dialog->setScale(0.6);
			this->addChild(dialog);

			Global::isPause = true;
			Background::inst()->setSpeed(0);
			break;
		}
	});
	
	pauseBtn->setPosition(Vec2(visibleSize.width - pauseBtn->getSize().width * 0.5, visibleSize.height - pauseBtn->getSize().height *0.5));
	this->addChild(pauseBtn);

	auto coin = Sprite::create("UI\\coin2.png");
	coin->setPosition(coin->getContentSize().width * 0.5 + visibleSize.width * 0.038, visibleSize.height * 0.715);
	this->addChild(coin);

	lbGold = Label::createWithBMFont("an24.fnt", "0");
	lbGold->setColor(Color3B(Color4F::YELLOW));
	lbGold->setPosition(coin->getPositionX() + coin->getContentSize().width, coin->getPositionY());
	this->addChild(lbGold);

	lbDistance = Label::createWithBMFont("an32.fnt", "0");
	lbDistance->setPosition(coin->getPositionX() + visibleSize.width * 0.25, coin->getPositionY() + coin->getContentSize().height * 1.3);
	this->addChild(lbDistance);

	sprHit = Sprite::create("UI\\t_hit.png");
	sprHit->setPositionX(sprHit->getContentSize().width * 0.5);

	sprHits = Sprite::create("UI\\t_hits.png");
	sprHits->setPositionX(sprHits->getContentSize().width * 0.5);
	sprHits->setVisible(false);

	lbCombo = Label::createWithBMFont("number72.fnt", "1 Hit");
	lbCombo->setPositionX(-sprHits->getContentSize().width * 0.25);
	lbCombo->setColor(Color3B(Color4F::RED));

	auto comboNode = Node::create();
	comboNode->addChild(lbCombo);
	comboNode->addChild(sprHit);
	comboNode->addChild(sprHits);
	comboNode->setRotation(-30);
	comboNode->setPosition(visibleSize.width * 0.8, visibleSize.height * 0.5);
	this->addChild(comboNode);

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
	else
		this->omislashItem->setEnabled(false);

	if (player->canBladeStorm())
		this->bladeStormItem->setEnabled(true);
	else
		this->bladeStormItem->setEnabled(false);
	updateHpBar();
	
	char stringGold[12] = { 0 };
	sprintf(stringGold, "x %d", player->getGold());
	lbGold->setString(stringGold);

	char stringDistance[12] = { 0 };
	sprintf(stringDistance, "%d", (int)player->getDistance());
	lbDistance->setString(stringDistance);

	if (player->getCombo() > 0)
	{
		if (player->getCombo() > 1)
		{
			sprHit->setVisible(false);
			sprHits->setVisible(true);
		}
		else
		{
			sprHit->setVisible(true);
			sprHits->setVisible(false);
		}
		char stringCombo[12] = { 0 };
		sprintf(stringCombo, "%d", (int)player->getCombo());
		lbCombo->setString(stringCombo);
	}
	else
	{
		sprHit->setVisible(false);
		sprHits->setVisible(false);
		lbCombo->setString("");
	}
	Background::inst()->update(dt);
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
		player->useOmislash();
	}
}

void HUDLayer::bladeStorm(Ref* sender)
{
	if (player->canBladeStorm()){
		bladeStormItem->setEnabled(false);
		player->useBladeStorm();
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

	/*if (!bladeStormItem->isEnabled()){
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
	}*/
	return true;
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
		SKILL1_PATH,
		SKILL_DISABLED_PATH,
		CC_CALLBACK_1(HUDLayer::omislash, this));
	omislashItem->setPosition(swordItem->getPosition().x - swordItem->getContentSize().width / 2 - omislashItem->getContentSize().width / 2 - 3,
		swordItem->getPosition().y);
	auto omislashDisabledImage = Sprite::create(SKILL_DISABLED_PATH);
	omislashItem->setDisabledImage(omislashDisabledImage);

	omiRect = new CCRect(omislashItem->getDisabledImage()->getPosition().x - omislashItem->getContentSize().width / 2
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

void HUDLayer::initHpBar()
{


	hpBarBorder = CCSprite::create(HP_BAR_BORDER_PATH);
	hpBarBorder->setZOrder(10);
	hpBarBorder->setPosition(hpBarBorder->getContentSize().width / 2 - hpBarBorder->getContentSize().width / 50,
		hpBarBorder->getContentSize().height / 2 - hpBarBorder->getContentSize().height / 20);


	hpBar = CCProgressTimer::create(CCSprite::create(HP_BAR_PATH));
	// Set this progress bar object as kCCProgressTimerTypeBar (%)
	hpBar->setType(ProgressTimerType::BAR);
	// Set anchor point in 0,0 and add it as a child to our border sprite
	hpBar->setAnchorPoint(ccp(0, 0));
	hpBar->setMidpoint(ccp(0, 0));
	hpBar->setPosition(Vec2(hpBarBorder->getContentSize().width / 3, hpBarBorder->getContentSize().height / 2));
	hpBar->setBarChangeRate(ccp(1, 0)); // right to left
	hpBar->setPercentage(100);
	hpBarBorder->addChild(hpBar); // Add it inside the border sprite



	headIcon = CCSprite::create(HEAD_PATH);
	headIcon->setAnchorPoint(ccp(0, 0));
	headIcon->setPosition(headIcon->getContentSize().width / 5, headIcon->getContentSize().height / 5);
	headIcon->setZOrder(20);
	hpBarBorder->addChild(headIcon);

	auto iconBackground = CCSprite::create("UI\\power.png");
	iconBackground->setPosition(hpBarBorder->getContentSize().width / 4,
		visibleSize.height - 3 * hpBarBorder->getContentSize().height / 4);
	iconBackground->addChild(hpBarBorder);

	this->addChild(iconBackground, 1);

	hpDecRate = 0.0f;
	tmpHP = PLAYER_MAX_HP;
}

void HUDLayer::updateHpBar()
{
	float realHP = player->getHP();

	//calculate hp decrease rate
	float hpDiff =  tmpHP - realHP;
	if (hpDiff / 60.0f > hpDecRate){
		 
		hpDecRate = DELTA_TIME * hpDiff / HP_DECREASE_DURATION;
	}

	// update hp progess bar
	if (hpDecRate > 0){
		tmpHP = tmpHP - hpDecRate < realHP ? realHP : tmpHP - hpDecRate;

		if (tmpHP <= realHP)
			hpDecRate = 0;
		float percent = 100.0f * tmpHP / player->getMaxHP();
		this->hpBar->setPercentage(percent);
	}
}


