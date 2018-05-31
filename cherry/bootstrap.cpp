#include "cherry/bootstrap.h"

#include "cherry/event_macro.h"
#include "cherry/task_runner.h"


namespace cherry {

// Class Bootstrap ------------------------------------------------------------

void Bootstrap::Run() {
  EventBus::Initialize();
  cherry::TaskRunner::RunAll(BindObj(this, &Bootstrap::Initialize));
  EventBus::Unregister(this);
  EventBus::Uninitialize();
}

void Bootstrap::Initialize() {
  EventBus::Register(this);
  OnStart();
}

bool Bootstrap::OnEvent(const Event* event) {
  bool handled = true;
  BEGIN_EVENT_MAP(Bootstrap, event)
    EVENT_HANDLER(Cherry_Stop, Stop)
    EVENT_UNHANDLED(handled = false)
  END_EVENT_MAP()
  return handled;
}

void Bootstrap::Stop() {
  TaskRunner::StopAll();
}

} // namespace cherry
