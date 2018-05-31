#ifndef CHERRY_TASK_RUNNER_H_
#define CHERRY_TASK_RUNNER_H_

#include "cherry/callback.h"
#include "cherry/time.h"
#include "cherry/waitable_event.h"

#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace cherry {

struct PendingTask;

using TaskQueue = std::queue<PendingTask>;

// Class TaskRunner -----------------------------------------------------------
class TaskRunner {
public:
  using ThreadID = std::thread::id;
  enum ID {
    // Main thread
    EVENT,
    
    // Thread processes IO, i.e. file, IPC and network.
    IO,

    // Number of well-known threads.
    THREAD_COUNT
  };

  TaskRunner();

  void BindToCurrentThread();
  void Run();
  bool RunsTasksInCurrentThread();

  void Stop();

  static bool CurrentlyOn(ID id);
  static std::shared_ptr<TaskRunner> GetTaskRunner(ID id);
  static bool PostTask(ID id, Callback callback);
  static bool PostDelayedTask(ID id, Callback callback, TimeDelta delay);
  static void RunAll(Callback&& init_op);
  static void StopAll();

private:
  bool DoWork();
  bool DoDelayedWork();

  void ReloadTriageTasksIfEmpty();
  
  bool PostDelayedTask(Callback callback, TimeDelta delay);

  // The queue receiving all posted tasks.
  std::unique_ptr<TaskQueue> incomming_tasks_;
  // Tasks to be dealing with.
  std::unique_ptr<TaskQueue> triage_tasks_;
  // Delayed tasks.
  std::unique_ptr<std::priority_queue<PendingTask>> delayed_tasks_;

  std::mutex thread_id_lock_;
  ThreadID thread_id_;

  // The next sequence number of tasks.
  int next_sequence_num_ = 0;

  // Data lock
  std::mutex incomming_tasks_lock_;

  // The time to call DoDelayedWork.
  TimeTicks delayed_work_time_;
  // A recent snapshot of Time::Now(), used to check delayed_tasks_.
  TimeTicks recent_time_;

  // Used to sleep until there is more work to do.
  WaitableEvent event_;

  bool keep_running_;

};

} // namespace cherry

#endif  // CHERRY_TASK_RUNNER_H_