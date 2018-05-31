#ifndef CHERRY_WAITABLE_EVENT_H_
#define CHERRY_WAITABLE_EVENT_H_

#include "cherry/time.h"

#include <condition_variable>


namespace cherry {

class WaitableEvent {
public:
  WaitableEvent() = default;

  void Signal();
  void Wait();
  bool TimedWait(const TimeDelta& wait_delta);
  bool TimedWaitUntil(const TimeTicks& end_time);

private:
  std::condition_variable cv_;
  std::mutex wait_mutex_;

};

} // namespace cherry

#endif  // CHERRY_WAITABLE_EVENT_H_