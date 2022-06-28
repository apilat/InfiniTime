#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include <array>

template <class T, size_t N> class RingBuffer {
public:
  RingBuffer() = default;

  void push(T val) {
    if (len < N) {
      buf[(start + len) % N] = val;
      len += 1;
    } else {
      buf[start] = val;
      start = (start + 1) % N;
    }
  }
  T pop() {
    len -= 1;
    return buf[(start + len) % N];
  }
  void clear() {
    len = 0;
  }

  T& at(size_t i) {
    return buf[(start + i) % N];
  }
  const T& at(size_t i) const {
    return buf[(start + i) % N];
  }
  T& last() {
    return at(len - 1);
  }
  const T& last() const {
    return at(len - 1);
  }
  size_t size() const {
    return len;
  }
  bool empty() const {
    return len == 0;
  }

private:
  std::array<T, N> buf;
  size_t start = 0, len = 0;
};

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Tennis : public Screen {
      public:
        explicit Tennis(DisplayApp* app);
        ~Tennis() override;

        bool OnTouchEvent(TouchEvents event) override;
        void OnButtonEvent(lv_obj_t* obj, lv_event_t evt);

      private:
        static const size_t SET_COUNT = 5;
        static const size_t HISTORY_LEN = 32;
        static const size_t FUTURE_LEN = 4;

        class State {
        public:
          State() : cur {0, 0}, sets(), totalGames {0} {};

          void RegisterPoint(int player);
          bool IsTiebreak() const;
          int GetServer() const;
          void GetScoreString(int player, char ret[3]) const;

          RingBuffer<std::array<uint8_t, 2>, SET_COUNT> sets;

        private:
          uint8_t cur[2];
          uint8_t totalGames;

          void TotalGame();
        };

        State state {};
        RingBuffer<State, HISTORY_LEN> history {};
        RingBuffer<State, FUTURE_LEN> future {};

        lv_obj_t *labelLeft, *labelRight, *labelGames;
        void UpdateLabels();
      };
    }
  }
}
