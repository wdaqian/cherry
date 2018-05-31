#include "cherry/task_runner.h"

#include "cherry/callback.h"

#include <assert.h>


namespace cherry {

std::shared_ptr<TaskRunner> g_task_runners[TaskRunner::THREAD_COUNT];

void RunTaskRunner(TaskRunner::ID id) {
  g_task_runners[id]->Run();
}

TimeTicks ToTimeTicks(TimeDelta delay) {
  TimeTicks run_time;
  if (delay > TimeDelta())
    run_time = TimeTicks::Now() + delay;
  else
    assert(delay.Microseconds() == 0);
  return run_time;
}


// class PendingTask ----------------------------------------------------------
struct PendingTask {
  PendingTask(Callback task, int seq, TimeTicks ticks)
      : task(std::move(task)), sequence_num(seq), run_time(ticks) {}
  PendingTask(PendingTask&& other) = default;

  PendingTask& operator=(PendingTask&& other) = default;

  bool operator<(const PendingTask& other) const {
    // Biggist element with smalleat run_time.
    if (run_time < other.run_time)
      return false;
    if (run_time > other.run_time)
      return true;
    return (sequence_num - other.sequence_num) > 0;
  }

  Callback task;
  TimeTicks run_time;
  int sequence_num = 0;
};


// class TaskRunner -----------------------------------------------------------

// static
bool TaskRunner::CurrentlyOn(ID id) {
  return g_task_runners[id] &&
         g_task_runners[id]->RunsTasksInCurrentThread();
}

// static
std::shared_ptr<TaskRunner> TaskRunner::GetTaskRunner(ID id) {
  if (id < THREAD_COUNT && id >= EVENT)
    return g_task_runners[id];
  return nullptr;
}

bool TaskRunner::PostTask(ID id, Callback callback) {
  return PostDelayedTask(id, std::move(callback), TimeDelta());
}

bool TaskRunner::PostDelayedTask(ID id, Callback callback, TimeDelta delay) {
  assert(id >= EVENT && id < THREAD_COUNT && g_task_runners[id]);
  return g_task_runners[id]->PostDelayedTask(std::move(callback), delay);
}

// static
void TaskRunner::RunAll(Callback&& init_op) {
  for (int i = 0; i < THREAD_COUNT; ++i)
    g_task_runners[i].reset(new TaskRunner);
  PostTask(EVENT, std::move(init_op));

  std::thread io_thread(RunTaskRunner, IO);

  g_task_runners[EVENT]->Run();
  io_thread.join();
}

// static
void TaskRunner::StopAll() {
  for (int i = 0; i < THREAD_COUNT; ++i)
    g_task_runners[i]->Stop();
}


TaskRunner::TaskRunner()
    : incomming_tasks_(new TaskQueue),
      triage_tasks_(new TaskQueue),
      delayed_tasks_(new std::priority_queue<PendingTask>),
      keep_running_(true) {
}

void TaskRunner::BindToCurrentThread() {
  std::lock_guard<std::mutex> lock(thread_id_lock_);
  thread_id_ = std::this_thread::get_id();
}

void TaskRunner::Run() {
  BindToCurrentThread();
  
  while (keep_running_) {
    bool did_work = DoWork();
    if (!keep_running_)
      break;

    did_work |= DoDelayedWork();
    if (!keep_running_)
      break;
    if (did_work)
      continue;

    if (delayed_work_time_.is_null()) {
      event_.Wait();
    } else {
      event_.TimedWaitUntil(delayed_work_time_);
    }
  }
}

bool TaskRunner::RunsTasksInCurrentThread() {
  std::lock_guard<std::mutex> lock(thread_id_lock_);
  return thread_id_ == std::this_thread::get_id();
}

void TaskRunner::Stop() {
  keep_running_ = false;
  event_.Signal();
}

bool TaskRunner::DoWork() {
  ReloadTriageTasksIfEmpty();
  if (!triage_tasks_->empty()) {
    PendingTask pending_task = std::move(triage_tasks_->front());
    triage_tasks_->pop();
    if (pending_task.run_time.is_null()) {
      pending_task.task.Run();
    } else {
      int sequence_num = pending_task.sequence_num;
      TimeTicks delayed_time = pending_task.run_time;
      delayed_tasks_->push(std::move(pending_task));
      // Update time of next delayed task.
      if (delayed_tasks_->top().sequence_num == sequence_num)
        delayed_work_time_ = delayed_time;
    }
    return true;
  }
  return false;
}

bool TaskRunner::DoDelayedWork() {
  if (!delayed_tasks_->empty()) {
    TimeTicks next_run_time = delayed_tasks_->top().run_time;
    if (next_run_time > recent_time_) {
      recent_time_ = TimeTicks::Now();
      if (next_run_time > recent_time_) {
        delayed_work_time_ = next_run_time;
        return false;
      }
    }
    PendingTask delayed_task =
        std::move(const_cast<PendingTask&>(delayed_tasks_->top()));
    delayed_tasks_->pop();

    if (!delayed_tasks_->empty())
      delayed_work_time_ = delayed_tasks_->top().run_time;
    else
      delayed_work_time_ = TimeTicks();
    delayed_task.task.Run();
    return true;
  }
  return false;
}

void TaskRunner::ReloadTriageTasksIfEmpty() {
  if (triage_tasks_->empty()) {
    std::lock_guard<std::mutex> lock(incomming_tasks_lock_);
    incomming_tasks_->swap(*triage_tasks_);
  }
}

bool TaskRunner::PostDelayedTask(Callback callback, TimeDelta delay) {
  PendingTask task(std::move(callback),
                   next_sequence_num_++, ToTimeTicks(delay));
  {
    std::lock_guard<std::mutex> lock(incomming_tasks_lock_);
    if (keep_running_) {
      incomming_tasks_->push(std::move(task));
      event_.Signal();
    }
  }
  return true;
}

} // namespace cherry