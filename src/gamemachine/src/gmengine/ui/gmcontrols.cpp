﻿#include "stdafx.h"
#include "gmcontrols.h"
#include "gmcanvas.h"

void GMElementBlendColor::init(const GMVec4& defaultColor, const GMVec4& disabledColor, const GMVec4& hiddenColor)
{
	D(d);
	GM_FOREACH_ENUM_CLASS(state, GMControlState::Normal, GMControlState::EndOfControlState)
	{
		d->states[state] = defaultColor;
	};
	d->states[GMControlState::Disabled] = disabledColor;
	d->states[GMControlState::Hidden] = hiddenColor;
	d->current = hiddenColor;
}

void GMElementBlendColor::blend(GMControlState::State state, GMfloat elapsedTime, GMfloat rate)
{
	D(d);
	GMVec4 destColor = d->states[state];
	d->current = Lerp(d->current, destColor, 1/20.f);
}

void GMStyle::setTexture(GMuint texture, const GMRect& rc, const GMVec4& defaultTextureColor)
{
	D(d);
	d->texture = texture;
	d->rc = rc;
	d->textureColor.init(defaultTextureColor);
}

void GMStyle::setFont(GMuint font, const GMVec4& defaultColor)
{
	D(d);
	d->font = font;
	d->fontColor.init(defaultColor);
}

void GMStyle::setFontColor(GMControlState::State state, const GMVec4& color)
{
	D(d);
	d->fontColor.getStates()[state] = color;
}

void GMStyle::setTextureColor(GMControlState::State state, const GMVec4& color)
{
	D(d);
	d->textureColor.getStates()[state] = color;
}

void GMStyle::refresh()
{
	D(d);
	d->textureColor.setCurrent(d->textureColor.getStates()[GMControlState::Hidden]);
	d->fontColor.setCurrent(d->fontColor.getStates()[GMControlState::Hidden]);
}

GMControl::GMControl(GMCanvas* canvas)
{
	D(d);
	d->canvas = canvas;
}

bool GMControl::handleKeyboard(GMSystemKeyEvent* event)
{
	bool handled = false;
	switch (event->getType())
	{
	case GMSystemEventType::KeyDown:
		handled = onKeyDown(event);
		break;
	case GMSystemEventType::KeyUp:
		handled = onKeyUp(event);
		break;
	}
	return handled;
}

bool GMControl::handleMouse(GMSystemMouseEvent* event)
{
	bool handled = false;
	switch (event->getType())
	{
	case GMSystemEventType::MouseDown:
		handled = onMousePress(event);
		break;
	case GMSystemEventType::MouseUp:
		handled = onMouseRelease(event);
		break;
	case GMSystemEventType::MouseMove:
		handled = onMouseMove(event);
		break;
	case GMSystemEventType::MouseDblClick:
		handled = onMouseDblClick(event);
		break;
	case GMSystemEventType::MouseWheel:
		handled = onMouseWheel(gm_cast<GMSystemMouseWheelEvent*>(event));
		break;
	}
	return handled;
}

void GMControl::updateRect()
{
	D(d);
	d->boundingBox.x = d->x;
	d->boundingBox.y = d->y;
	d->boundingBox.width = d->width;
	d->boundingBox.height = d->height;
}

void GMControl::refresh()
{
	D(d);
	d->mouseOver = false;
	d->hasFocus = false;
}

GMControlStatic::GMControlStatic(GMCanvas* parent)
	: GMControl(parent)
{
	D_BASE(d, GMControl);
	d->type = GMControlType::Static;
}

void GMControlStatic::render(GMfloat elapsed)
{
	if (!getVisible())
		return;

	D(d);
	D_BASE(db, Base);

	GMControlState::State state = GMControlState::Normal;
	if (!getEnabled())
		state = GMControlState::Disabled;

	d->foreStyle.getFontColor().blend(state, elapsed);
	getParent()->drawText(getText(), d->foreStyle, db->boundingBox, false, false);
}

void GMControlStatic::refresh()
{
	D(d);
	Base::refresh();
	d->foreStyle.refresh();
}

void GMControlStatic::initStyles()
{
	D(d);
	d->foreStyle.setFont(0);
	d->foreStyle.setFontColor(GMControlState::Disabled, GMVec4(.87f, .87f, .87f, .87f));
}

void GMControlStatic::setText(const GMString& text)
{
	D(d);
	d->text = text;
}

GM_DEFINE_SIGNAL(GMControlButton::click);

GMControlButton::GMControlButton(GMCanvas* parent)
	: GMControlStatic(parent)
{
	D_BASE(d, GMControl);
	d->type = GMControlType::Button;
}

void GMControlButton::refresh()
{
	D(d);
	Base::refresh();
	d->fillStyle.refresh();
}

bool GMControlButton::onMousePress(GMSystemMouseEvent* event)
{
	return handleMousePressOrDblClick(event->getPoint());
}

bool GMControlButton::onMouseDblClick(GMSystemMouseEvent* event)
{
	return handleMousePressOrDblClick(event->getPoint());
}

bool GMControlButton::onMouseRelease(GMSystemMouseEvent* event)
{
	return handleMouseRelease(event->getPoint());
}

bool GMControlButton::containsPoint(const GMPoint& pt)
{
	D_BASE(d, GMControl);
	return GM_inRect(d->boundingBox, pt);
}

bool GMControlButton::canHaveFocus()
{
	return getEnabled() && getVisible();
}

bool GMControlButton::onKeyDown(GMSystemKeyEvent* event)
{
	D(d);
	if (event->getKey() == GMKey_Space)
	{
		d->pressed = true;
		return true;
	}
	return false;
}

bool GMControlButton::onKeyUp(GMSystemKeyEvent* event)
{
	D(d);
	if (event->getKey() == GMKey_Space)
	{
		if (d->pressed)
		{
			d->pressed = false;
			emit(click);
			return true;
		}
	}
	return false;
}

void GMControlButton::render(GMfloat elapsed)
{
	if (!getVisible())
		return;

	D(d);
	D_BASE(db, Base);
	GMint offsetX = 0, offsetY = 0;
	GMControlState::State state = GMControlState::Normal;
	if (!getEnabled())
	{
		state = GMControlState::Disabled;
	}
	else if (d->pressed)
	{
		state = GMControlState::Pressed;
		offsetX = 1;
		offsetY = 2;
	}
	else if (getMouseOver())
	{
		state = GMControlState::MouseOver;
		offsetX = -1;
		offsetY = -2;
	}
	else if (hasFocus())
	{
		state = GMControlState::Focus;
	}

	GMCanvas* canvas = getParent();
	GMfloat blendRate = (state == GMControlState::Pressed) ? 0.0f : 0.8f;

	db->foreStyle.getTextureColor().blend(state, elapsed, blendRate);
	db->foreStyle.getFontColor().blend(state, elapsed, blendRate);
	canvas->drawSprite(db->foreStyle, boundingRect(), .8f);
	canvas->drawText(getText(), db->foreStyle, boundingRect(), false, true);

	d->fillStyle.getTextureColor().blend(state, elapsed, blendRate);
	d->fillStyle.getFontColor().blend(state, elapsed, blendRate);
	canvas->drawSprite(d->fillStyle, boundingRect(), .8f);
	canvas->drawText(getText(), d->fillStyle, boundingRect(), false, true);
}

bool GMControlButton::handleMousePressOrDblClick(const GMPoint& pt)
{
	if (!canHaveFocus())
		return false;

	D(d);
	D_BASE(db, GMControl);
	if (containsPoint(pt))
	{
		d->pressed = true;
		// TODO SetCapture

		if (!db->hasFocus)
			getParent()->requestFocus(this);
		return true;
	}

	return false;
}

bool GMControlButton::handleMouseRelease(const GMPoint& pt)
{
	if (!canHaveFocus())
		return false;

	D(d);
	D_BASE(db, GMControl);
	if (d->pressed)
	{
		d->pressed = false;
		// TODO ReleaseCapture

		GMCanvas* canvas = getParent();
		if (!canvas->canKeyboardInput())
			canvas->clearFocus();

		if (containsPoint(pt))
		{
			emit(click);
		}

		return true;
	}
	return false;
}

void GMControlButton::initStyles()
{
	D(d);
	D_BASE(db, Base);

	GMCanvas* canvas = getParent();
	db->foreStyle.setTexture(0, canvas->getArea(GMCanvasControlArea::ButtonArea));
	db->foreStyle.setFont(0);
	db->foreStyle.setTextureColor(GMControlState::Normal, GMVec4(1.f, 1.f, 1.f, 0));
	db->foreStyle.setTextureColor(GMControlState::Pressed, GMVec4(0, 0, 0, .24f));
	db->foreStyle.setFontColor(GMControlState::MouseOver, GMVec4(0, 0, 0, 1.f));

	d->fillStyle.setTexture(0, canvas->getArea(GMCanvasControlArea::ButtonFillArea));
	d->fillStyle.setFont(0);
	d->fillStyle.setTextureColor(GMControlState::Normal, GMVec4(.59f, 1.f, 1.f, 1.f));
	d->fillStyle.setTextureColor(GMControlState::Pressed, GMVec4(.78f, 1.f, 1.f, 1.f));
	d->fillStyle.setFontColor(GMControlState::MouseOver, GMVec4(0, 0, 0, 1.f));
}