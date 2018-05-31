#ifndef CHERRY_EVENT_MACRO_H_
#define CHERRY_EVENT_MACRO_H_

#include "cherry/event_bus.h"

#include <tuple>


#define EVENT_TUPLE(...) CheckedTuple<__VA_ARGS__>::Tuple
#define EVENT_ID() ((EVENT_START << 16) + __LINE__)
#define EVENT_START Cherry_EventStart

#define EVENT_DEFINE(event_name, ...) \
  EVENT_DECL(event_name, EVENT_TUPLE(__VA_ARGS__))

#define EVENT_DEFINE0(name) EVENT_DEFINE(name)
#define EVENT_DEFINE1(name, a) EVENT_DEFINE(name, a)
#define EVENT_DEFINE2(name, a, b) EVENT_DEFINE(name, a, b)
#define EVENT_DEFINE3(name, a, b, c) EVENT_DEFINE(name, a, b, c)
#define EVENT_DEFINE4(name, a, b, c, d) EVENT_DEFINE(name, a, b, c, d)
#define EVENT_DEFINE5(name, a, b, c, d, e) EVENT_DEFINE(name, a, b, c, d, e)

#define EVENT_DECL(event_name, in_tuple) \
  struct event_name##_Meta { \
    using InTuple = in_tuple; \
    enum { ID = EVENT_ID() }; \
  }; \
  using event_name = cherry::EventT<event_name##_Meta>;


#define BEGIN_EVENT_MAP(class_name, e) \
  { \
    typedef class_name _EventHandlerClass; \
    const cherry::Event* event__ = e; \
    switch (event__->GetID()) {

#define EVENT_HANDLER(event_class, mfunc) \
    case event_class::ID: { \
      event_class::Dispatch(event__, this, &_EventHandlerClass::mfunc); \
      break; \
    }

#define EVENT_UNHANDLED(code) \
    default: { \
      code; \
    }

#define END_EVENT_MAP() \
    }\
  }


template <typename... Ts>
struct CheckedTuple {
  typedef std::tuple<Ts...> Tuple;
};


enum EventStart {
  Cherry_EventStart = 0,
};


// Event define.
EVENT_DEFINE0(Cherry_Stop)

#endif  // CHERRY_EVENT_MACRO_H_
