﻿#include "stdafx.h"
#include "gameobject.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gmengine/elements/gameworld.h"
#include "gmdatacore/glyph/glyphmanager.h"
#include "gmengine/controllers/gamemachine.h"
#include "gmgl/gmglglyphmanager.h" //TODO 不应该有GMGL

GameObject::GameObject(AUTORELEASE Object* obj)
{
	setObject(obj);
}

void GameObject::setObject(AUTORELEASE Object* obj)
{
	D(d);
	d.object.reset(obj);
}

Object* GameObject::getObject()
{
	D(d);
	if (!d.object)
		d.object.reset(new Object());
	return d.object;
}

void GameObject::setWorld(GameWorld* world)
{
	D(d);
	ASSERT(!d.world);
	d.world = world;
}

GameWorld* GameObject::getWorld()
{
	D(d);
	return d.world;
}

void GameObject::getReadyForRender(DrawingList& list)
{
	D(d);
	linear_math::Matrix4x4 M = linear_math::Matrix4x4::identity();
	DrawingItem item;
	memcpy(item.trans, &M[0], sizeof(M));
	item.gameObject = this;
	list.push_back(item);
}

void GameObject::onAppendingObjectToWorld()
{

}

//GlyphObject
#define X(i) (i) / resolutionWidth
#define Y(i) (i) / resolutionHeight
#define UV_X(i) ((i) / (GMfloat)GMGLGlyphManager::CANVAS_WIDTH)
#define UV_Y(i) ((i) / (GMfloat)GMGLGlyphManager::CANVAS_HEIGHT)

GlyphObject::GlyphObject()
	: GameObject(nullptr)
{
	D(d);
}

void GlyphObject::setText(const GMWChar* text)
{
	D(d);
	d.text = text;
}

// 窗口中央坐标为(0,0)，左下角坐标为(-1, -1)
void GlyphObject::setGeometry(GMfloat left, GMfloat bottom, GMfloat width, GMfloat height)
{
	D(d);
	d.left = left;
	d.bottom = bottom;
	d.width = width;
	d.height = height;
}

void GlyphObject::constructObject()
{
	D(d);
	D_BASE(GameObject, db);

	GlyphManager* glyphManager = db.world->getGameMachine()->getGlyphManager();
	IWindow* window = db.world->getGameMachine()->getWindow();
	GMRect rect = window->getWindowRect();
	GMfloat resolutionWidth = rect.width, resolutionHeight = rect.height;

	Object* obj = new Object();
	ChildObject* child = new ChildObject();
	obj->append(child);
	child->setArrangementMode(ChildObject::Triangle_Strip);
	child->setType(ChildObject::Glyph);

	Component* component = new Component(child);
	Shader& shader = component->getShader();
	shader.texture.textures[TEXTURE_INDEX_AMBIENT].frames[0] = glyphManager->glyphTexture();
	shader.texture.textures[TEXTURE_INDEX_AMBIENT].frameCount = 1;
	shader.noDepthTest = true;
	shader.cull = GMS_NONE;
	shader.blend = true;
	shader.blendFactors[0] = GMS_ONE;
	shader.blendFactors[1] = GMS_ONE;

	const GMWChar* p = d.text.c_str();
	const GMfloat Z = 0;
	GMfloat x = d.left, y = d.bottom;

	while (*p)
	{
		component->beginFace();
		const GlyphInfo& glyph = glyphManager->getChar(*p);

		if (glyph.width > 0 && glyph.height > 0)
		{
			// 如果width和height为0，视为空格，只占用空间而已
			// 否则：按照条带顺序，创建顶点
			// 0 2
			// 1 3
			// 让所有字体origin开始的x轴平齐

			component->vertex(x + X(glyph.bearingX), y + Y(glyph.bearingY), Z);
			component->vertex(x + X(glyph.bearingX), y + Y(glyph.bearingY - glyph.height), Z);
			component->vertex(x + X(glyph.bearingX + glyph.width), y + Y(glyph.bearingY), Z);
			component->vertex(x + X(glyph.bearingX + glyph.width), y + Y(glyph.bearingY - glyph.height), Z);

			component->uv(UV_X(glyph.x), UV_Y(glyph.y));
			component->uv(UV_X(glyph.x), UV_Y(glyph.y + glyph.height));
			component->uv(UV_X(glyph.x + glyph.width), UV_Y(glyph.y));
			component->uv(UV_X(glyph.x + glyph.width), UV_Y(glyph.y + glyph.height));
		}
		x += X(glyph.advance);

		component->endFace();
		p++;
	}

	child->appendComponent(component);
	setObject(obj);
}


void GlyphObject::onAppendingObjectToWorld()
{
	D(d);
	d.lastRenderText = d.text;
	constructObject();
}

void GlyphObject::getReadyForRender(DrawingList& list)
{
	D(d);
	if (d.lastRenderText != d.text)
	{
		updateObject();
		d.lastRenderText = d.text;
	}
	GameObject::getReadyForRender(list);
}

void GlyphObject::updateObject()
{
	D_BASE(GameObject, d);
	constructObject();
	d.world->initObject(this);
}

//EntityObject
EntityObject::EntityObject(AUTORELEASE Object* obj)
	: GameObject(obj)
{
	calc();
}

Plane* EntityObject::getPlanes()
{
	D(d);
	return d.planes;
}

void EntityObject::getBounds(REF linear_math::Vector3& mins, REF linear_math::Vector3& maxs)
{
	D(d);
	mins = d.mins;
	maxs = d.maxs;
}

void EntityObject::calc()
{
	D(d);
	d.mins[0] = d.mins[1] = d.mins[2] = 999999.f;
	d.maxs[0] = d.maxs[1] = d.maxs[2] = -d.mins[0];

	Object* obj = getObject();
	for (auto iter = obj->getChildObjects().begin(); iter != obj->getChildObjects().end(); iter++)
	{
		ChildObject* child = *iter;
		Object::DataType* vertices = child->vertices().data();
		GMint sz = child->vertices().size();
		for (GMint i = 0; i < sz; i += 4)
		{
			for (GMint j = 0; j < 3; j++)
			{
				if (vertices[i + j] < d.mins[j])
					d.mins[j] = vertices[i + j];
				if (vertices[i + j] > d.maxs[j])
					d.maxs[j] = vertices[i + j];
			}
		}
	}

	makePlanes();
}

void EntityObject::makePlanes()
{
	D(d);
	// 前
	d.planes[0] = Plane(linear_math::Vector3(0, 0, 1), -d.maxs[2]);
	// 后
	d.planes[1] = Plane(linear_math::Vector3(0, 0, -1), d.mins[2]);
	// 左
	d.planes[2] = Plane(linear_math::Vector3(-1, 0, 0), d.mins[0]);
	// 右
	d.planes[3] = Plane(linear_math::Vector3(1, 0, 0), -d.maxs[0]);
	// 上
	d.planes[4] = Plane(linear_math::Vector3(0, 1, 0), -d.maxs[0]);
	// 下
	d.planes[5] = Plane(linear_math::Vector3(0, -1, 0), d.mins[0]);
}