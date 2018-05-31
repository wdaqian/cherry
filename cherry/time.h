#ifndef CHERRY_TIME_H_
#define CHERRY_TIME_H_

#include <assert.h>
#include <chrono>


namespace cherry {

// TimeDelta ------------------------------------------------------------------

class TimeDelta {
 public:
  // Converts units of time to TimeDeltas.
  static constexpr TimeDelta FromHours(int hours);
  static constexpr TimeDelta FromMinutes(int minutes);
  static constexpr TimeDelta FromSeconds(int64_t secs);
  static constexpr TimeDelta FromMilliseconds(int64_t ms);
  static constexpr TimeDelta FromMicroseconds(int64_t us);

  static constexpr TimeDelta Max();
  static constexpr TimeDelta Min();

  constexpr TimeDelta() : microsecond_(0) {}
  int64_t Microseconds() const { return microsecond_; }

  constexpr bool is_zero() const { return microsecond_ == 0; }

  constexpr bool is_max() const {
    return microsecond_ == std::numeric_limits<int64_t>::max();
  }
  constexpr bool is_min() const {
    return microsecond_ == std::numeric_limits<int64_t>::min();
  }

  constexpr TimeDelta& operator=(const TimeDelta& other) {
    microsecond_ = other.microsecond_;
    return *this;
  }
  
  // Comparison operators.
  constexpr bool operator==(const TimeDelta& other) const {
    return microsecond_ == other.microsecond_;
  }
  constexpr bool operator!=(const TimeDelta& other) const {
    return microsecond_ != other.microsecond_;
  }
  constexpr bool operator<(const TimeDelta& other) const {
    return microsecond_ < other.microsecond_;
  }
  constexpr bool operator<=(const TimeDelta& other) const {
    return microsecond_ <= other.microsecond_;
  }
  constexpr bool operator>(const TimeDelta& other) const {
    return microsecond_ > other.microsecond_;
  }
  constexpr bool operator>=(const TimeDelta& other) const {
    return microsecond_ >= other.microsecond_;
  }

 private:
  constexpr explicit TimeDelta(int64_t us) : microsecond_(us) {}

  static constexpr TimeDelta FromHelper(int64_t value, int64_t positive_value);

  // Delta in microseconds.
  int64_t microsecond_;
};


// TimeTicks-------------------------------------------------------------------

class TimeTicks {
 public:
  static const int64_t kMillisecondsPerSecond = 1000;
  static const int64_t kMicrosecondsPerMillisecond = 1000;
  static const int64_t kMicrosecondsPerSecond =
      kMicrosecondsPerMillisecond * kMillisecondsPerSecond;
  static const int64_t kMicrosecondsPerMinute = kMicrosecondsPerSecond * 60;
  static const int64_t kMicrosecondsPerHour = kMicrosecondsPerMinute * 60;

  constexpr TimeTicks() : microsecond_(0) {}
  constexpr explicit TimeTicks(int64_t us) : microsecond_(us) {}

  int64_t Microseconds() const { return microsecond_; }

  bool is_null() const {
    return microsecond_ == 0;
  }

  bool is_max() const { return microsecond_ == std::numeric_limits<int64_t>::max(); }
  bool is_min() const { return microsecond_ == std::numeric_limits<int64_t>::min(); }

  static TimeTicks Now() {
    return TimeTicks(std::chrono::time_point_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now()).time_since_epoch().count());
  }

  static TimeTicks Max() {
    return TimeTicks(std::numeric_limits<int64_t>::max());
  }

  static TimeTicks Min() {
    return TimeTicks(std::numeric_limits<int64_t>::min());
  }

  // Comparison operators
  bool operator==(const TimeTicks& other) const {
    return microsecond_ == other.microsecond_;
  }
  bool operator!=(const TimeTicks& other) const {
    return microsecond_ != other.microsecond_;
  }
  bool operator<(const TimeTicks& other) const {
    return microsecond_ < other.microsecond_;
  }
  bool operator<=(const TimeTicks& other) const {
    return microsecond_ <= other.microsecond_;
  }
  bool operator>(const TimeTicks& other) const {
    return microsecond_ > other.microsecond_;
  }
  bool operator>=(const TimeTicks& other) const {
    return microsecond_ >= other.microsecond_;
  }
  
  TimeTicks operator+(const TimeDelta& delta) const {
    if (delta == TimeDelta::Max())
      return TimeTicks::Max();

    int64_t delta_us = delta.Microseconds();
    int64_t new_tick = microsecond_ + delta_us;
    if (delta_us > 0 && microsecond_ > 0 && new_tick < 0)
      return TimeTicks::Max();
    if (delta_us < 0 && microsecond_ < 0 && new_tick > 0)
      return TimeTicks::Min();
    return TimeTicks(new_tick);
  }

  TimeDelta operator-(const TimeTicks& other) const {
    return TimeDelta::FromMicroseconds(microsecond_ - other.microsecond_);
  }

 protected:
  // Time value in a microsecond timebase.
  int64_t microsecond_;
};


// static
constexpr TimeDelta TimeDelta::FromHours(int hours) {
  return FromHelper(hours, TimeTicks::kMicrosecondsPerHour);
}

// static
constexpr TimeDelta TimeDelta::FromMinutes(int minutes) {
  return FromHelper(minutes, TimeTicks::kMicrosecondsPerMinute);
}

// static
constexpr TimeDelta TimeDelta::FromSeconds(int64_t secs) {
  return FromHelper(secs, TimeTicks::kMicrosecondsPerSecond);
}

// static
constexpr TimeDelta TimeDelta::FromMilliseconds(int64_t ms) {
  return FromHelper(ms, TimeTicks::kMicrosecondsPerMillisecond);
}

// static
constexpr TimeDelta TimeDelta::FromMicroseconds(int64_t us) {
  return TimeDelta(us);
}

// static
constexpr TimeDelta TimeDelta::Max() {
  return FromMicroseconds(std::numeric_limits<int64_t>::max());
}

// static
constexpr TimeDelta TimeDelta::Min() {
  return FromMicroseconds(std::numeric_limits<int64_t>::min());
}

// static
constexpr TimeDelta TimeDelta::FromHelper(int64_t value,
                                          int64_t positive_value) {
  assert(positive_value > 0);
  return value > std::numeric_limits<int64_t>::max() / positive_value
             ? Max()
             : value < std::numeric_limits<int64_t>::min() / positive_value
                   ? Min()
                   : FromMicroseconds(value * positive_value);
}

}  // namespace cherry

#endif  // CHERRY_TIME_H_