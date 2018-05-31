#ifndef CHERRY_CALLBACK_H_
#define CHERRY_CALLBACK_H_

#include <functional>
#include <memory>


namespace cherry {

// Callable target
class Callback {
public:
  Callback(const Callback&) = delete;
  Callback& operator=(const Callback&) = delete;

  Callback(Callback&&) = default;
  Callback& operator=(Callback&&) = default;

  // This type of parameter can also accept r-value reference.
  explicit Callback(std::function<void()>&& function)
      : function_(function) {}

  void Run() {
    function_();
  }

private:
  std::function<void()> function_;
};


// Args are passed by value.
template<typename T, typename R, typename... Args>
Callback BindObj(
    T* obj, R(T::*mfunc)(Args...), Args... args) {
  std::function<void()> func = [obj, mfunc, args...]() mutable {
    if (obj)
      ((*obj).*mfunc)(std::move(args)...);
  };
  return Callback(std::move(func));
}

// Args are passed by value.
template<typename T, typename R, typename... Args>
Callback BindObj(
    std::shared_ptr<T> ptr, R(T::*mfunc)(Args...), Args... args) {
  std::weak_ptr<T> wptr(ptr);
  std::function<void()> func = [wptr, mfunc, args...]() mutable {
    std::shared_ptr<T> ptr = wptr.lock();
    if (ptr)
      ((*ptr).*mfunc)(std::move(args)...);
  };
  return Callback(std::move(func));
}


// Args are passed by value.
template<typename R, typename... Args>
Callback Bind(R(*functor)(Args...), Args... args) {
  return Callback([functor, args...]() mutable {
    functor(std::move(args)...);
  });
}

} // namespace cherry

#endif  // CHERRY_CALLBACK_H_