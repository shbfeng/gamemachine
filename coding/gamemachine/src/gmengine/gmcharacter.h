﻿#ifndef __CHARACTER_H__
#define __CHARACTER_H__
#include "common.h"
#include "gmgameobject.h"
#include "foundation/utilities/utilities.h"
#include "gmphysics/gmphysicsstructs.h"

BEGIN_NS

typedef GMbyte MoveAction;
enum 
{
	MD_NONE = 0,
	MD_FORWARD = 1,
	MD_BACKWARD = 2,
	MD_LEFT = 4,
	MD_RIGHT = 8,
	MD_JUMP = 16,

	// 以上MoveAction数目
	MD_ACTION_COUNT = 5,
};

class MoveRate
{
public:
	MoveRate()
	{
		for (GMuint i = 0; i < MD_ACTION_COUNT; i++)
		{
			m_moveRate[i] = 1;
		}
	}

	void setMoveRate(MoveAction action, GMfloat rate)
	{
		m_moveRate[(GMint)log((GMfloat)action)] = rate;
	}

	GMfloat getMoveRate(MoveAction action)
	{
		return m_moveRate[(GMint)log((GMfloat)action)];
	}

private:
	GMfloat m_moveRate[MD_ACTION_COUNT];
};

GM_PRIVATE_OBJECT(GMCharacter)
{
	GMfloat radius;
	Frustum frustum;
	MoveAction moveDirection;
	MoveRate moveRate;

	PositionState state;
	CameraLookAt lookAt;
	GMCommandVector3 moveCmdArgFB;
	GMCommandVector3 moveCmdArgLR;
};

class GMCharacter : public GMGameObject
{
	DECLARE_PRIVATE(GMCharacter)

public:
	GMCharacter(GMfloat radius);

public:
	void setJumpSpeed(const linear_math::Vector3& jumpSpeed);
	void setMoveSpeed(GMfloat moveSpeed);
	void simulation();
	void action(MoveAction md, MoveRate rate);
	void lookUp(GMfloat degree);
	void lookRight(GMfloat degree);
	void setPitchLimitDegree(GMfloat deg);
	const PositionState& getPositionState();
	void updateCamera();
	CameraLookAt& getLookAt();
	Frustum& getFrustum();

public:
	virtual void onAppendingObjectToWorld() override;

private:
	void moveForwardOrBackward(bool forward);
	void moveLeftOrRight(bool left);
	void update();
	void sendMoveCommand();
	void clearMoveArgs();
};

END_NS
#endif