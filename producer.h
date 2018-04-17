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
#ifndef STOCKTICKERSB_PRODUCER_H_
#define STOCKTICKERSB_PRODUCER_H_


#include <iostream>
#include <limits>

#include "configuration.h"
#include "constants.h"
#include "singleton.h"
#include "stockproductadvanced.h"
#include "tracelogger.h"

namespace stocktickersb {

using namespace std;

class Producer {
 public:
  Producer(void);
  Producer(const Producer &) = delete;
  Producer(SynchronizedQueue<StockProductAdvanced> *sque, vector<StockProductAdvanced> *input_container) {
    engine_queue_ = sque; 
	input_container_ = input_container;
	reset_counters();
  }
  ~Producer(void);
  void Clear(void);
  bool StartProducer(StockSource val);
  bool StopProducer(void);
  bool WaitTargetWasReached(void);

  long long dbg_produced_count_; 

  void SetStockSource(StockSource val) {
	 
	  producer_done_ = false;
	  if (val != StockSource::undefined) {
		  producer_sleep_done_ = true;
		  producer_sleep_condition_.notify_one();
	  }
	  else
		  producer_sleep_done_ = false;

	  stock_source_ = val;
  }

  void SetTargetAmount(long long val) {
	  assert(val > 0 && "target value is not valid");
	  target_amount_ = val;
	  producer_done_ = false;
  }

  void reset_counters(void) {
	  dbg_produced_count_ = 0;
	  current_input_index_ = 0;
	  producer_done_ = false;
  }

  long long GetProducedAmount(void) const {
	  return dbg_produced_count_;
  }
 private:
  thread producer_thread_;
  SynchronizedQueue<StockProductAdvanced> *engine_queue_;
  bool producer_stopped_;
  void ThreadFuncProducer(void);
  long long target_amount_=numeric_limits<long long>::max();
  StockSource stock_source_;
  vector<StockProductAdvanced> *input_container_;
  int current_input_index_;
  mutex producer_mutex_;
  condition_variable producer_condition_;
  bool producer_done_;
  mutex producer_sleep_mutex_;
  condition_variable producer_sleep_condition_;
  bool producer_sleep_done_;
  StockTickerConfiguration &config_ = Singleton<StockTickerConfiguration>::get_instance();
  TraceLogger &trace_log = Singleton<TraceLogger>::get_instance();
};


void Producer::ThreadFuncProducer(void) {
  StockProductAdvanced stock;
  bool busy=false;
  trace_log.AddTrace("Producer::ThreadFuncProducer begin"); 
  while (!producer_stopped_) {
    if (stock_source_ == StockSource::random_input) {
      stock.GenerateRandomStock();   
    }
    
	if ((stock_source_ == StockSource::file_input) && (input_container_->size() > 0)) {

      if ((busy == false) && (current_input_index_ < static_cast<int>(input_container_->size()))) {
        stock = (*input_container_)[current_input_index_++];
        if (config_.get_enable_cout_disable_unit_test() == true) 
          cout << "Producer::thread Symbol: " << stock.get_symbol() << endl;     
    
      }
      else {
        stock.Clear();
        producer_done_ = true;
        producer_condition_.notify_one();
      }
    }
    if (stock.get_symbol().size() > 0) {
      bool ret = engine_queue_->PushBack(stock);
      if (ret == true)
        this->dbg_produced_count_++;
      busy = !ret;
	  stock.Clear();
    }
	if (stock_source_ == StockSource::undefined) {
		unique_lock<mutex> lck1(producer_sleep_mutex_);
		producer_sleep_condition_.wait(lck1, [this]() {return producer_sleep_done_; });
		lck1.unlock();
	}
	else {
		// this_thread::yield();
		this_thread::sleep_for(100ms);
	}
  } // while
  trace_log.AddTrace("Producer::ThreadFuncProducer end"); 
}


bool Producer::StartProducer(StockSource val) { 
  trace_log.AddTrace("Producer::StartProducer"); 

  stock_source_ = val;
  producer_stopped_ = false;
  producer_done_ = false;
  producer_sleep_done_ = false;
  reset_counters();
  target_amount_ = numeric_limits<long long>::max();
  producer_thread_ = thread(&Producer::ThreadFuncProducer, this);
  return true;
}

bool Producer::StopProducer(void) { 
  if (producer_stopped_ == false) {
    producer_stopped_ = true;
    if (producer_thread_.joinable()) {
      producer_thread_.join();
      producer_stopped_ = false;
    }
  }
  return true;
}

void Producer::Clear(void) {
  engine_queue_ = nullptr;
  producer_stopped_ = false;
  input_container_ = nullptr;
  producer_done_ = false;
  producer_sleep_done_ = false;
  reset_counters();
}

Producer::Producer(void) {
  Clear();
}

Producer::~Producer(void) {
  StopProducer();
  Clear();
}


bool Producer::WaitTargetWasReached(void) {
  trace_log.AddTrace("Producer::WaitTargetWasReached");
  unique_lock<mutex> lck1(producer_mutex_);
  producer_condition_.wait(lck1, [this]() {return producer_done_;});
  lck1.unlock();
  cout << "WaitTargetWasReached producer was done" << endl;
  return true;
}

} //namespace StockTickerSB {

#endif