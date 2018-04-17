/*
Copyright (C) 2017 Shoshana Biro

This file is part of StockTickerSB, a free-software/open-source C++ library
for financial quantitative analysts and developers - https://github.com/sbiro/MNI-Pricing-Engine

StockTickerSB is free software: you can redistribute it and/or modify it
under the terms of the StockTickerSB license.  You should have received a
copy of the license along with this program; if not, please email
<sbiro@ix.netcom.com>. The license is also available online at
<https://github.com/sbiro/MNI-Pricing-Engine/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
#ifndef STOCKTICKERSB_SYNCHRONIZEDQUEUE_H_
#define STOCKTICKERSB_SYNCHRONIZEDQUEUE_H_

#include <condition_variable>
#include <cassert>
#include <deque>
#include <mutex>

namespace stocktickersb {
using namespace std;

/*
* Used in StockTicker, StockTickerSubscriber, ErrorLogger, TraceLogger
*/
template <typename T>
class SynchronizedQueue {
 public:
  SynchronizedQueue(void) { Clear(); }
  SynchronizedQueue(const SynchronizedQueue<T>& queue) {
    queue.get_queue(&que_);
  }
  SynchronizedQueue operator=(const SynchronizedQueue<T>& queue) {
    if (this == &queue)
      return *this;
    queue.get_queue(&que_);
    return *this;
  }
  ~SynchronizedQueue(void) { Clear();}
  bool PushBack(const T& rec);
  bool PopFront(T* rec);
  void Clear(void);
  int  size(void) { return static_cast<int>(que_.size()); }

  bool get_queue(deque<T>* queue) const {
    if (queue == nullptr)
      return false;
    *queue = que_;
    return true;
  }
  void SetStop(bool val) {
	  stop_ = val;
  }
 private:
  deque<T>	que_;
  mutex	mutex_;
  condition_variable condition_;
  bool stop_=false;
};

////////////////////////  SynchronizedQueue implementation ////////////////////////////////////////
template <typename T>
inline void SynchronizedQueue<T>::Clear(void) {
  que_.clear();

}

template <typename T>
bool SynchronizedQueue<T>::PushBack(const T& rec) {
 
  if (mutex_.try_lock()) {
    que_.push_back(rec);
    mutex_.unlock();
    if (que_.size() == 1)
      condition_.notify_one();
    return true;
  } else {
    return false;
  }  
}

template <typename T>
bool SynchronizedQueue<T>::PopFront(T* rec) {
  if (rec == nullptr)
    return false;
  unique_lock<mutex> lck(mutex_);
  condition_.wait(lck, [this]() { return (!que_.empty()) || (stop_==true); }); // to prevent spurious wakeups
  if (stop_ == false) {
	  *rec = que_.front();
	  que_.pop_front();
  }
  lck.unlock();
  return true;
}
} ///namespace StockTickerSB 
#endif