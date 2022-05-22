#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace ThreadPool {

template <class T>
class SyncQueue {
 public:
  SyncQueue(size_t maxsize = 200) : _maxsize(maxsize), _needstop(false) {}
  ~SyncQueue() {}

  void Put(const T& x) { Add(x); }

  void Put(T&& x) { Add(std::forward<T>(x)); }

  void Take(std::list<T>& list) {
    std::unique_lock<std::mutex> locker(_mutex);
    _notempty.wait(locker, [this] { return _needstop || NotEmpty(); });

    if (_needstop) return;

    list = std::move(_queue);
    _notfull.notify_one();
  }

  void Take(T& t) {
    std::unique_lock<std::mutex> locker(_mutex);
    _notempty.wait(locker, [this] { return _needstop || NotEmpty(); });

    if (_needstop) return;

    t = _queue.front();
    _queue.pop_front();
    _notfull.notify_one();
  }

  void Stop() {
    {
      std::lock_guard<std::mutex> locker(_mutex);
      _needstop = true;
    }
    _notfull.notify_all();
    _notempty.notify_all();
  }

  bool Empty() {
    std::lock_guard<std::mutex> locker(_mutex);
    return _queue.empty();
  }

  bool Full() {
    std::lock_guard<std::mutex> locker(_mutex);
    return _queue.size() == _maxsize;
  }

  int Size() {
    std::lock_guard<std::mutex> locker(_mutex);
    return _queue.size();
  }

  int Count() { return _queue.size(); }

 private:
  bool NotFull() const { return _queue.size() <= _maxsize; }
  bool NotEmpty() const { return !_queue.empty(); }

  template <typename F>
  void Add(F&& x) {
    std::unique_lock<std::mutex> locker(_mutex);
    _notfull.wait(locker, [this] { return _needstop || NotFull(); });
    if (_needstop) return;
    _queue.push_back(std::forward<F>(x));
    _notempty.notify_one();
  }

 private:
  std::list<T> _queue;
  std::mutex _mutex;
  std::condition_variable _notempty;
  std::condition_variable _notfull;
  size_t _maxsize;
  bool _needstop;
};

class ThreadPool {
 public:
  using Task = std::function<void()>;
  ThreadPool(int threadnumber = 10) : _threadnumber(threadnumber) {
    _isrunning = true;

    for (int i = 0; i < _threadnumber; ++i) {
      _threads.push_back(std::make_shared<std::thread>(&ThreadPool::Run, this));
    }
  }

  void Run() {
    while (_isrunning) {
      std::list<Task> tasks;
      _tasks.Take(tasks);

      for (auto& task : tasks) {
        if (!_isrunning) return;
        task();
      }
    }
  }

  ~ThreadPool() { Stop(); }

  void Add(Task&& task) { _tasks.Put(std::forward<Task>(task)); }

  void Add(const Task& task) { _tasks.Put(task); }

  void Stop() {
    std::call_once(_flag, [this] { StopThreads(); });
  }

  void StopThreads() {
    _tasks.Stop();
    _isrunning = false;

    for (auto& thread : _threads) {
      if (thread) thread->join();
    }
    _threads.clear();
  }

 private:
  std::vector<std::shared_ptr<std::thread>> _threads;
  SyncQueue<Task> _tasks;

  std::mutex _thread_mutex;
  std::condition_variable _cond;

  int _threadnumber;
  std::atomic_bool _isrunning;
  std::once_flag _flag;
};

}  // namespace ThreadPool

#endif  // __THREAD_POOL_H__