#include "cherry/waitable_event.h"

#include <chrono>

using namespace std::chrono;


namespace cherry {

// Class WaitableEvent --------------------------------------------------------

void WaitableEvent::Signal() {
  cv_.notify_one();
}

void WaitableEvent::Wait() {
  std::unique_lock<std::mutex> lock(wait_mutex_);
  cv_.wait(lock);
}

bool WaitableEvent::TimedWait(const TimeDelta& wait_delta) {
  microseconds ticks(wait_delta.Microseconds());
  std::unique_lock<std::mutex> lock(wait_mutex_);
  std::cv_status status = cv_.wait_for(lock, ticks);
  return status == std::cv_status::no_timeout;
}

bool WaitableEvent::TimedWaitUntil(const TimeTicks& end_time) {
  TimeDelta delta = end_time - TimeTicks::Now();
  assert(delta.Microseconds() > 0);
  return TimedWait(delta);
}

} // namespace cherry
