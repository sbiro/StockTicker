/*
Copyright (C) 2017 Shoshana Biro

This file is part of StockTicker, a free-software/open-source library
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
#ifndef STOCKTICKERSB_CONSUMER_H_
#define STOCKTICKERSB_CONSUMER_H_


#include <condition_variable>
#include <cassert>
#include <iostream>
#include <limits>
#include <mutex>
#include <thread>

#include "configuration.h"
#include "constants.h"
#include "db_stockticker.h"
#include "publisher.h"
#include "singleton.h"
#include "stockproductadvanced.h"
#include "synchronizedqueue.h"
#include "tracelogger.h"

namespace stocktickersb {

using namespace std;

class Consumer {
 public:
    Consumer(void);
    Consumer(const Consumer&) = delete;
    Consumer(SynchronizedQueue<StockProductAdvanced> *sque, DBStockTickerContainer *db, Publisher *pub) { 
      engine_queue_ = sque;
      stock_ticker_container_ = db;
      publisher_ = pub;
    }
    ~Consumer(void);
    void Clear(void);
    bool StartConsumer(void);
    bool StopConsumer(void);
    bool WaitTargetWasReached(void);
	void SetTargetAmount(long long val)
	{
		assert(val > 0 && "target value is not valid");
		target_amount_ = val;
		consumer_done_ = false;
	}

    long long dbg_consumed_count_; // debugging only

	void reset_counters() {
		dbg_consumed_count_ = 0;
	}
	long long GetConsumedAmount(void) {
		return dbg_consumed_count_;
	}
 private:
    thread consumer_thread_;
    SynchronizedQueue<StockProductAdvanced> *engine_queue_;
    DBStockTickerContainer* stock_ticker_container_;
    Publisher* publisher_;
    bool consumer_stopped_;
 
    mutex consumer_mutex_;
    condition_variable condition_consumer_;
    bool consumer_done_;
	long long target_amount_ = numeric_limits<long long>::max();
    StockTickerConfiguration &config_ = Singleton<StockTickerConfiguration>::get_instance();
    void ThreadFuncConsumer(void);
    TraceLogger &trace_log = Singleton<TraceLogger>::get_instance();
};

void Consumer::ThreadFuncConsumer(void) {
 
  bool busyDB = false;
  StockProductAdvanced stock; 
  //bool DoContinueRunning = true;
  bool ret;
  int err = 0;

  trace_log.AddTrace("Consumer::ThreadFuncConsumer begin"); 
  stock.Clear();


   while (!consumer_stopped_) {
	if (dbg_consumed_count_ < target_amount_)
	{
		ret = engine_queue_->PopFront(&stock); // engine_queue_ is shared with producer

		ret = stock_ticker_container_->UpdateEntry(&stock, &busyDB); // mutex inside there
		if (ret == true)
			dbg_consumed_count_++;

		if ((consumer_stopped_ == false) && (busyDB == false)) {
			ret = publisher_->UpdateAllSubscribers(&stock);
			if (ret == true) {
				
				stock.Clear();
			}
			else { //no subscriber wanted this quote
				err = 0;
			}

		}
		else {
			err = 1;
			cout << "consumer error 2:" << stock.get_symbol() << endl;
		}
		if ((dbg_consumed_count_ >= target_amount_) || (consumer_stopped_ == true)) {
			consumer_done_ = true;
			condition_consumer_.notify_one();
		}
	}
	this_thread::yield();


  }

  trace_log.AddTrace("Consumer::ThreadFuncConsumer end");
}

bool Consumer::StartConsumer(void) {
  trace_log.AddTrace("Consumer::StartConsumer");
  if (stock_ticker_container_ == nullptr)
    return false;
  if (engine_queue_ == nullptr)
    return false;
  consumer_stopped_ = false;
  dbg_consumed_count_ = 0;
  consumer_done_ = false;
  consumer_thread_ = thread(&Consumer::ThreadFuncConsumer, this);
  return true;
}

void Consumer::Clear(void) {
    engine_queue_ = nullptr;
    consumer_stopped_ = false;
    consumer_done_ = false;
    dbg_consumed_count_ = 0;
}

Consumer::Consumer(void) {
    Clear();
}


Consumer::~Consumer(void) {
    StopConsumer();
    Clear();
}

bool Consumer::StopConsumer(void) {
  if (consumer_stopped_ == false)
  {
    //cout << "4" << endl;
    consumer_stopped_ = true;
    if (consumer_thread_.joinable())
    {
      //cout << "5" << endl;
        consumer_thread_.join();
        //cout << "6" << endl;
        consumer_stopped_ = false;
    }
  }
  return true;
}

bool Consumer::WaitTargetWasReached(void) {
  trace_log.AddTrace("Consumer::WaitTargetWasReached");
	unique_lock<mutex> lck2(consumer_mutex_);

	condition_consumer_.wait(lck2, [this]() {return consumer_done_;});

	lck2.unlock();
	cout << "WaitTargetWasReached consumer was done" << endl;

  return true; 
}

} // namespace StockTickerSB
#endif