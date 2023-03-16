#include "displayapp/screens/Tennis.h"
#include "displayapp/DisplayApp.h"
#include "lvgl/src/lv_core/lv_disp.h"
#include "lvgl/src/lv_widgets/lv_label.h"
#include <cstdio>
#include <cstring>

using namespace Pinetime::Applications::Screens;
using Pinetime::Applications::TouchEvents;

static void btnCallback(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Tennis*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

Tennis::Tennis() : Screen() {
  lv_obj_t* labelColon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelColon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(labelColon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(labelColon, ":");
  lv_obj_align(labelColon, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  labelLeft = lv_label_create(lv_scr_act(), nullptr);
  labelLeft->user_data = this;
  labelLeft->click = true;
  lv_obj_set_style_local_text_font(labelLeft, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_align_origo(labelLeft, labelColon, LV_ALIGN_OUT_LEFT_MID, -32, 0);
  lv_obj_set_auto_realign(labelLeft, true);
  lv_obj_set_event_cb(labelLeft, btnCallback);
  lv_obj_set_ext_click_area(labelLeft, 10, 10, 10, 10);

  labelRight = lv_label_create(lv_scr_act(), nullptr);
  labelRight->user_data = this;
  labelRight->click = true;
  lv_obj_set_style_local_text_font(labelRight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_align_origo(labelRight, labelColon, LV_ALIGN_OUT_RIGHT_MID, 32, 0);
  lv_obj_set_auto_realign(labelRight, true);
  lv_obj_set_event_cb(labelRight, btnCallback);
  lv_obj_set_ext_click_area(labelRight, 10, 10, 10, 10);

  labelGames = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelGames, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_label_set_recolor(labelGames, true);

  UpdateLabels();
  state.sets.push({0, 0});
}

Tennis::~Tennis() {
  lv_obj_clean(lv_scr_act());
}

bool Tennis::OnTouchEvent(TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeRight:
      if (history.empty()) {
        return false;
      } else {
        future.push(state);
        state = history.pop();
        UpdateLabels();
        return true;
      }

    case TouchEvents::SwipeLeft:
      if (future.empty()) {
        return false;
      } else {
        history.push(state);
        state = future.pop();
        UpdateLabels();
        return true;
      }

    default:
      return false;
  }
}

void Tennis::OnButtonEvent(lv_obj_t* obj, lv_event_t evt) {
  if (evt == LV_EVENT_CLICKED && (obj == labelLeft || obj == labelRight)) {
    history.push(state);
    future.clear();
    if (obj == labelLeft) {
      state.RegisterPoint(0);
    } else if (obj == labelRight) {
      state.RegisterPoint(1);
    }
    UpdateLabels();
  }
}

void Tennis::State::GetScoreString(int player, char ret[3]) const {
  int pts = cur[player];
  int oppPts = cur[player ^ 1];
  if (IsTiebreak()) {
    ret[0] = '0' + (pts / 10);
    ret[1] = '0' + (pts % 10);
    ret[2] = '\0';
  } else {
    switch (pts) {
      case 0:
        strcpy(ret, "00");
        break;
      case 1:
        strcpy(ret, "15");
        break;
      case 2:
        strcpy(ret, "30");
        break;
      default:
        if (oppPts < 3 || pts == oppPts) {
          strcpy(ret, "40");
        } else if (pts > oppPts) {
          strcpy(ret, "AD");
        } else {
          strcpy(ret, "  ");
        }
    }
  }
}

void Tennis::UpdateLabels() {
  lv_obj_t* high = state.GetServer() ? labelRight : labelLeft;
  lv_obj_t* normal = state.GetServer() ? labelLeft : labelRight;

  lv_obj_set_style_local_text_color(normal, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_bg_opa(normal, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_text_color(high, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_bg_opa(high, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

  char scoreString[3];
  state.GetScoreString(0, scoreString);
  lv_label_set_text(labelLeft, scoreString);
  state.GetScoreString(1, scoreString);
  lv_label_set_text(labelRight, scoreString);

  char str[6 + 4 * SET_COUNT + 1];
  char* head = str + sprintf(str, "#DDDD00 %d-%d# ", state.sets.last()[0], state.sets.last()[1]);
  if (state.sets.size() > 1) {
    for (size_t i = state.sets.size() - 1; i > 0; i--) {
      int cnt = sprintf(head, "%d-%d ", state.sets.at(i - 1)[0], state.sets.at(i - 1)[1]);
      head += cnt;
    }
  }
  lv_label_set_text(labelGames, str);
}

void Tennis::State::RegisterPoint(int player) {
  cur[player] += 1;
  TotalGame();
}

void Tennis::State::TotalGame() {
  for (uint8_t i : {0, 1}) {
    bool won;
    if (IsTiebreak()) {
      won = cur[i] >= 7 && cur[i] > cur[i ^ 1] + 1;
    } else {
      won = cur[i] >= 4 && cur[i] > cur[i ^ 1] + 1;
    }
    if (won) {
      sets.last()[i] += 1;
      cur[0] = cur[1] = 0;
      totalGames += 1;
    }
  }

  auto games = sets.last();
  for (uint8_t i : {0, 1}) {
    if (games[i] == 7 || (games[i] == 6 && games[i ^ 1] <= 4)) {
      sets.push({0, 0});
    }
  }
}

bool Tennis::State::IsTiebreak() const {
  auto games = sets.last();
  return games[0] == 6 && games[1] == 6;
}

int Tennis::State::GetServer() const {
  int srv = totalGames % 2;
  if (IsTiebreak()) {
    // First ball served by the same player as previous games.
    // Then alternate after every two points.
    srv ^= 1;
    int totalPoints = cur[0] + cur[1];
    srv ^= ((totalPoints + 1) / 2) % 2;
  }
  return srv;
}
