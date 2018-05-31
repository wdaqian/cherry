#ifndef CHERRY_BOOTSTRAP_H_
#define CHERRY_BOOTSTRAP_H_

#include "cherry/event_bus.h"


namespace cherry {

class Bootstrap : public EventObserver {
public:
  Bootstrap() = default;
  virtual ~Bootstrap() {}

  void Run();
  void Initialize();

  virtual void OnStart() = 0;

  // EventObserver implementation
  bool OnEvent(const Event* event);

private:
  void Stop();

};

} // namespace cherry

#endif  // CHERRY_BOOTSTRAP_H_