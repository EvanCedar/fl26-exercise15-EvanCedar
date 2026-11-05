
// thread_safe_vector.hpp
// A thread-safe generic vector using C++11 synchronization primitives,
// modeled on the ThreadSafeQueue from lecture.
#ifndef THREAD_SAFE_VECTOR_HPP
#define THREAD_SAFE_VECTOR_HPP

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <utility>
#include <vector>

template <typename T>
class ThreadSafeVector
{
public:
  ThreadSafeVector() = default;

  // Copy: lock the source so it can't change while we copy it
  ThreadSafeVector(const ThreadSafeVector& other)
  {
    std::lock_guard<std::mutex> lock(other.mut_);
    data_ = other.data_;
  }

  // Assignment would need to lock two mutexes safely; not needed here
  ThreadSafeVector& operator=(const ThreadSafeVector&) = delete;

  // Add an item to the back and wake one waiting thread
  void push_back(T value)
  {
    {
      std::lock_guard<std::mutex> lock(mut_);
      data_.push_back(std::move(value));
    } // unlock before notifying
    cond_.notify_one();
  }

  // Block until the vector is non-empty, then remove the last item
  void wait_and_pop_back(T& value)
  {
    std::unique_lock<std::mutex> lock(mut_);
    cond_.wait(lock, [this] { return !data_.empty(); });
    value = std::move(data_.back());
    data_.pop_back();
  }

  // Non-blocking version: returns false immediately if empty
  bool try_pop_back(T& value)
  {
    std::lock_guard<std::mutex> lock(mut_);
    if (data_.empty()) {
      return false;
    }
    value = std::move(data_.back());
    data_.pop_back();
    return true;
  }

  bool empty() const
  {
    std::lock_guard<std::mutex> lock(mut_);
    return data_.empty();
  }

  std::size_t size() const
  {
    std::lock_guard<std::mutex> lock(mut_);
    return data_.size();
  }

private:
  mutable std::mutex mut_;        // mutable so const methods can lock it
  std::vector<T> data_;
  std::condition_variable cond_;
};

#endif // THREAD_SAFE_VECTOR_HPP