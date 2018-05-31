#include "cherry/event_bus.h"

#include "cherry/task_runner.h"


namespace cherry {


// Class EventBus -------------------------------------------------------------
EventBus* EventBus::s_instance = nullptr;

// static
void EventBus::Initialize() {
  if (s_instance == nullptr)
    s_instance = new EventBus;
}

// static
void EventBus::EventBus::Uninitialize() {
  if (s_instance) {
    delete s_instance;
    s_instance = nullptr;
  }
}

// static
void EventBus::FireEvent(const Event* event) {
  TaskRunner::PostTask(TaskRunner::EVENT,
                       BindObj(s_instance, &EventBus::OnEvent, event));
}

// static
void EventBus::Register(EventObserver* observer) {
  assert(s_instance);
  TaskRunner::PostTask(TaskRunner::EVENT,
                       BindObj(s_instance, &EventBus::AddObserver,
                               observer));
}

// static
void EventBus::Unregister(EventObserver* observer) {
  assert(s_instance);
  assert(TaskRunner::CurrentlyOn(TaskRunner::EVENT));
  s_instance->RemoveObserver(observer);
}

void EventBus::OnEvent(const Event* event) {
  Compact();
  for (auto observer : observers_) {
    if (observer && observer->OnEvent(event))
      break;
  }
  delete event;
}

void EventBus::AddObserver(EventObserver* observer) {
  assert(observer);
  assert(!HasObserver(observer));
  observers_.push_back(observer);
}

void EventBus::RemoveObserver(EventObserver* observer) {
  auto itr = std::find(observers_.begin(), observers_.end(), observer);
  if (itr != observers_.end())
    *itr = nullptr;
}

bool EventBus::HasObserver(const EventObserver* observer) const {
  std::list<EventObserver*>::const_iterator itr =
      std::find(observers_.begin(), observers_.end(), observer);
  return itr != observers_.end();
}

void EventBus::Compact() {
  observers_.remove(nullptr);
}

} // namespace cherry
