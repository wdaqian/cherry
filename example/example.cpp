// Used for test

#include "cherry/bootstrap.h"
#include "cherry/event_macro.h"
#include "cherry/task_runner.h"

#include <iostream>
#include <memory>
#include <tuple>

using namespace cherry;
using namespace std;


EVENT_DEFINE2(TestEvent, int, int)
EVENT_DEFINE2(TestEvent2, float, int)


// Class EventTester ----------------------------------------------------------
class EventTester {
public:
  EventTester() { cout << "Constructor of EventTester.\n"; }
  ~EventTester() { cout << "Destructor of EventTester.\n"; }
  EventTester(const EventTester& other) {
    cout << "Copy constructor of EventTester.\n";
  }
  EventTester(EventTester&& other) {
    cout << "Move constructor of EventTester.\n";
  }
  EventTester& operator=(const EventTester& other) {
    cout << "Assignment of EventTester.\n";
  }
  EventTester& operator=(const EventTester&& other) {
    cout << "Move assignment of EventTester.\n";
  }

};
EVENT_DEFINE1(ObjectEvent, EventTester)


class EventListener : public EventObserver {
public:
  bool OnEvent(const Event* event) override {
    bool handled = true;
    BEGIN_EVENT_MAP(EventListener, event)
      EVENT_HANDLER(TestEvent, Test)
      EVENT_HANDLER(TestEvent2, Test2)
      EVENT_HANDLER(ObjectEvent, ObjectTest)
      EVENT_UNHANDLED(handled = false)
    END_EVENT_MAP()
    return handled;
  }

  void Test(int x, int y) {
    std::cout << TimeTicks::Now().Microseconds();
    std::cout << " Test, post to Test2" << std::endl;
    EventBus::FireEvent(new TestEvent2(3.2f, 1));

    ++count_;
  }

  void Test2(float x, int y) {
    std::cout << TimeTicks::Now().Microseconds();
    std::cout << " Test2, post to Test" << std::endl;
    EventBus::FireEvent(new TestEvent(3, 1));

    ++count_;
  }

  void ObjectTest(EventTester t) {
    cout << "On object test event.\n";
  }

private:
  int count_ = 0;
};


// Class TestBootstrap --------------------------------------------------------
class TestBootstrap : public Bootstrap {
public:
  TestBootstrap() {
    listener_ = new EventListener;
  }

  // Bootstrap implementations
  void OnStart() override {
    cout << "TestBootstrap start.\n";
    std::cout << TimeTicks::Now().Microseconds() << " Initialize." << std::endl;

    EventTest();
  }

  void EventTest() {
    EventBus::Register(listener_);
    EventBus::FireEvent(new ObjectEvent(tester_));
    EventBus::FireEvent(new TestEvent2(3.2f, 1));

    cout << "Quit after 5 seconds.\n";
    TaskRunner::PostDelayedTask(
        TaskRunner::EVENT,
        BindObj(this, &TestBootstrap::Stop),
        TimeDelta::FromSeconds(5));
  }

  void Stop() {
    EventBus::Unregister(listener_);
    delete listener_;
    EventBus::FireEvent(new Cherry_Stop);
  }

private:
  EventListener* listener_;
  EventTester tester_;

};

int main() {
  TestBootstrap bootstrap;
  bootstrap.Run();
  return 0;
}
