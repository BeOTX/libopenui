/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include "libopenui_config.h"
#include "button.h"
#include "font.h"
#include "theme.h"

static void lvglEvent(lv_event_t* e)
{
  Button* btn = (Button*)lv_event_get_user_data(e);
  if(btn->deleted())
    return;

  btn->onEvent(e);
}

static void update_checked_flag(lv_obj_t* obj, WindowFlags flags)
{
  if (!(flags & BUTTON_CHECKED))
    lv_obj_clear_state(obj, LV_STATE_CHECKED);
  else
    lv_obj_add_state(obj, LV_STATE_CHECKED);  
}

Button::Button(Window* parent, const rect_t& rect,
       std::function<uint8_t(void)> pressHandler,
       WindowFlags windowFlag, LcdFlags textFlags,
       LvglCreate objConstruct) :
    FormField(parent, rect, windowFlag, textFlags, objConstruct),
    pressHandler(std::move(pressHandler))
{
  lv_obj_add_event_cb(lvobj, lvglEvent, LV_EVENT_CLICKED, this);

  if (windowFlag & BUTTON_CHECKED)
    lv_obj_add_state(lvobj, LV_STATE_CHECKED);
}

void Button::check(bool checked)
{
  if (checked != bool(windowFlags & BUTTON_CHECKED)) {
    windowFlags ^= BUTTON_CHECKED;
    update_checked_flag(lvobj, windowFlags);
    invalidate();
  }
}

bool Button::checked() const
{
  if (windowFlags & BUTTON_CHECKED_ON_FOCUS)
    return hasFocus();
  else
    return windowFlags & BUTTON_CHECKED;
}

void Button::onPress()
{
  bool check = (pressHandler && pressHandler());
  windowFlags =
      check ? windowFlags | BUTTON_CHECKED : windowFlags & ~BUTTON_CHECKED;
  if (lvobj == nullptr) return;
  update_checked_flag(lvobj, windowFlags);
}

#if defined(HARDWARE_KEYS)
void Button::onEvent(event_t event)
{
  auto indev_act = lv_indev_get_act();
  switch (event) {
    case EVT_KEY_FIRST(KEY_ENTER):
      lv_event_send(lvobj, LV_EVENT_PRESSED, indev_act);
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      lv_event_send(lvobj, LV_EVENT_RELEASED, indev_act);
      lv_event_send(lvobj, LV_EVENT_CLICKED, indev_act);
      break;

    default:
      FormField::onEvent(event);
      break;
  }
}
#endif

void Button::onEvent(lv_event_t* event)
{
  lv_event_code_t code = lv_event_get_code(event);

  TRACE_WINDOWS("%s received lvgl event code 0x%X", getWindowDebugString("Button").c_str(), code);

  if (enabled && code == LV_EVENT_CLICKED) {
    if (!(windowFlags & NO_FOCUS)) {
      setFocus(SET_FOCUS_DEFAULT);
    }
    onKeyPress();
    onPress();
  }
}

#if defined(HARDWARE_TOUCH)
// this needs to stay so that viewmain doesnt get the touch event
bool Button::onTouchEnd(coord_t x, coord_t y)  
{
  return true;
}
#endif

void Button::checkEvents()
{
  Window::checkEvents();
  if (checkHandler)
    checkHandler();
}

TextButton::TextButton(Window* parent, const rect_t& rect, std::string text,
                       std::function<uint8_t(void)> pressHandler,
                       WindowFlags windowFlags, LcdFlags textFlags) :
    Button(parent, rect, std::move(pressHandler), windowFlags, textFlags,
           lv_btn_create),
    text(std::move(text))
{
  setTextFlags(textFlags | COLOR_THEME_PRIMARY1);
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_CHECKABLE);

  if (windowFlags & BUTTON_CHECKED)
    lv_obj_add_state(lvobj, LV_STATE_CHECKED);

  label = lv_label_create(lvobj);
  lv_label_set_text(label, this->text.c_str());
  lv_obj_center(label);
}

void IconButton::paint(BitmapBuffer * dc)
{
  dc->drawBitmap(0, 0, theme->getIcon(icon, checked() ? STATE_PRESSED : STATE_DEFAULT));
}
