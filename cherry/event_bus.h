#ifndef CHERRY_EVENT_BUS_H_
#define CHERRY_EVENT_BUS_H_

#include <list>
#include <tuple>


namespace cherry {

class EventObserver;

// Event dispatch helper method
template <typename ObjT, typename Method, typename Tuple, size_t... Ns>
inline void DispatchToMethodImpl(const ObjT& obj,
                                 Method method,
                                 Tuple&& args,
                                 std::index_sequence<Ns...>) {
  (obj->*method)(std::get<Ns>(std::forward<Tuple>(args))...);
}

template <typename ObjT, typename Method, typename Tuple>
inline void DispatchToMethod(const ObjT& obj,
                             Method method,
                             Tuple&& args) {
  constexpr size_t size = std::tuple_size<std::decay_t<Tuple>>::value;
  DispatchToMethodImpl(obj, method, std::forward<Tuple>(args),
                       std::make_index_sequence<size>());
}


// Class Event ----------------------------------------------------------------
class Event {
public:
  Event() : canceled_(false) {}
  virtual ~Event() = default;

  virtual int GetID() const = 0;
  bool IsCanceled() const { return canceled_; }

private:
  bool canceled_;

};


// Class EventT ---------------------------------------------------------------
// Event template
template <typename Meta,
          typename InTuple = typename Meta::InTuple>
class EventT;

template <typename Meta, typename... Ins>
class EventT<Meta, std::tuple<Ins...>> : public Event {
public:
  using ThisType = EventT<Meta, std::tuple<Ins...>>;
  using Param = std::tuple<Ins...>;
  enum { ID = Meta::ID };

  EventT(const Ins&... ins) : param_(ins...) {}

  int GetID() const override { return ID; }

  template <typename T, typename F>
  static void Dispatch(const Event* msg, T* obj, F func) {
#ifdef _DEBUG
    const ThisType* this_msg = dynamic_cast<const ThisType*>(msg);
#else
    const ThisType* this_msg = static_cast<const ThisType*>(msg);
#endif // _DEBUG

    DispatchToMethod(obj, func, this_msg->param_);
  }

private:
  Param param_;
};


// Class EventBus -------------------------------------------------------------
class EventBus {
public:
  static void Initialize();
  static void Uninitialize();
  static void FireEvent(const Event* event);
  static void Register(EventObserver* observer);
  static void Unregister(EventObserver* observer);

  void OnEvent(const Event* event);
  void AddObserver(EventObserver* observer);
  void RemoveObserver(EventObserver* observer);
  bool HasObserver(const EventObserver* observer) const;

private:
  EventBus() = default;

  void Compact();

  // Static instance
  static EventBus* s_instance;

  std::list<EventObserver*> observers_;

};


// Class EventObserver --------------------------------------------------------
class EventObserver {
public:
  virtual bool OnEvent(const Event* event) = 0;
};

} // namespace cherry

#endif  // CHERRY_EVENT_BUS_H_
