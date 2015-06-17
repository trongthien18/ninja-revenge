#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "cocos2d.h"
#include "extensions/cocos-ext.h"
#include "cocostudio/CocoStudio.h"
#include "Global.h"

#if ENABLE_PHYSICS_BOX2D_DETECT
#include "../../Box2DTestBed/GLES-Render.h"
#include "Box2D/Box2D.h"
#elif ENABLE_PHYSICS_CHIPMUNK_DETECT
#include "chipmunk.h"
#endif

USING_NS_CC;
using namespace cocostudio;

#define	PLAYER_JUMP_SPEED		450
#define PLAYER_SLASH_DELAY		0.7f
#define PLAYER_TIME_HOLDING		0.3f
#define PLAYER_TIME_USE_SKILL1	10.0f

class Player : public Armature
{
private:
	bool		isJumping;
	float		timeDelayAttack;
	bool		isHolding;
	float		timeHolding;

	float		flashPositionY;
	bool		canFlash;
	bool		isOnGround;
	float		groundPosition;

	bool		canUseSkill1;
	float		timeUseSkill1;
	float		timeToAddNewShadow;

	//properties of player
	float		jumpSpeed;
	float		acttackSpeed;
	int			maxHitPoint;
	int			hitPoint = 500;
	
	float		timeCoolDownSkill1;
	float		timeOfSkill1;

	float		timeCoolDownSkill2;
	float		timeOfSkill2;

public:
	//static create method
	static Player* create();

	//constructor & destroy
	Player();
	~Player();

	bool initPlayer();
	void loadPlayerData();
	
	//override
	virtual void update(float dt);

	//get & set method || SYNTHESIZE
	CC_SYNTHESIZE(ESTATE, state, State);

	//behavior
	void run();
	void jump();
	void setHold(bool val);
	void attack();
	void setDie();
	void flashUp();
	void flashDown();
	void setHit(int damage);

	//skill 1
	void addAShadow();
	void setSkill1();
	bool getIsUseSkill() { return canUseSkill1; }
	void dirtPlay();

	//event
	void animationEvent(Armature *armature, MovementEventType movementType, const std::string& movementID);
	bool onContactBegin(PhysicsContact& contact);
	void onContactSeperate(PhysicsContact& contact);
	void onContactPostSolve(PhysicsContact& contact, const PhysicsContactPostSolve& solve);

	void destroyCallback(Node* node);
};

#endif