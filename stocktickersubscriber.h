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
#ifndef STOCKTICKERSB_STOCKTICKERSUBSCRIBER_H_
#define STOCKTICKERSB_STOCKTICKERSUBSCRIBER_H_




#include <cassert>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <ostream>
#include <thread>

#include <boost/filesystem.hpp>

#include "configuration.h"
#include "constants.h"
#include "logger.h"
#include "singleton.h"
#include "stockproductadvanced.h"
#include "synchronizedqueue.h"
#include "tracelogger.h"
#include "utility.h"

namespace stocktickersb {
using namespace std;

////////////////////SUBSCRIBER/////////////////////////////////

class Subscriber {
 public:
  virtual ~Subscriber() {}
  virtual bool UpdateSubscriber(StockProductAdvanced* what, bool* busy) = 0;
};

class StockTickerSubscriber : public Subscriber {
 public:
   StockTickerSubscriber(void) { 
     Clear(); 
   };

   explicit StockTickerSubscriber(Logger* stock_logger)
   {
     stock_logger_ = stock_logger;
     Clear(); 
    }

  StockTickerSubscriber(const StockTickerSubscriber& ss)  {
    Clear();
    // not taking the ques 
     string str1 = ss.get_file_name();
    output_string_stream_.rdbuf()->str(str1);
    subscriber_id_ = ss.get_id();
    subscriber_name_ = ss.get_name();
    ss.get_symbol_list(&symbol_list_);
   
    stock_logger_ = ss.get_stock_logger();
    if (stock_logger_ != nullptr) {
     
      stock_logger_->SetFileName(output_string_stream_.str(),subscriber_id_);
    }
  }
  StockTickerSubscriber operator=(const StockTickerSubscriber& ss) {
    if (this == &ss)
      return *this;
    Clear();
    string str1 = ss.get_file_name();
    output_string_stream_.rdbuf()->str(str1);
    subscriber_id_ = ss.get_id();
    subscriber_name_ = ss.get_name();
    ss.get_symbol_list(&symbol_list_);
    stock_logger_ = ss.get_stock_logger();
    if (stock_logger_ != nullptr) {
    
      stock_logger_->SetFileName(output_string_stream_.str(), subscriber_id_);
    }
    return *this;
  }
  StockTickerSubscriber(StockTickerSubscriber&& ss) {
    Clear();
 
  string str1 = ss.get_file_name();
    output_string_stream_.rdbuf()->str(str1);
    subscriber_id_ = ss.get_id();
    subscriber_name_ = ss.get_name();
    ss.get_symbol_list(&symbol_list_);
    stock_logger_ = ss.get_stock_logger();
    if (stock_logger_ != nullptr) {
     
      stock_logger_->SetFileName(output_string_stream_.str(), subscriber_id_);
    }
    ss.Clear();
  }
  StockTickerSubscriber& operator=(StockTickerSubscriber&& ss) {
    if (this == &ss)
      return *this;
    Clear();
   
    string str1 = ss.get_file_name();
    output_string_stream_.rdbuf()->str(str1);
    subscriber_id_ = ss.get_id();
    subscriber_name_ = ss.get_name();
    ss.get_symbol_list(&symbol_list_);
    stock_logger_ = ss.get_stock_logger();
    if (stock_logger_ != nullptr) {
     
      stock_logger_->SetFileName(output_string_stream_.str(), subscriber_id_);
    }
    ss.Clear();
    return *this;
  }
  ~StockTickerSubscriber(void) override {
    Stop();
    Clear();
  }

  void Clear(void) {
  
    stock_updates_que_.Clear();
    subscriber_stopped_ = false;
    dbg_subscriber_output_count_=0; 
    dbg_subscriber_input_count_=0; 
    output_string_stream_.rdbuf()->str("");
   
    subscriber_id_ = -1;
    subscriber_name_.clear();
    symbol_list_.clear();
	subscriber_done_ = false;
  }
  bool CreateOutputFile(const int subscriber_id, const string &SubscriberName, string *fileName = nullptr);
  string get_file_name(void) const { return output_string_stream_.str(); }
  bool Start(void);
  bool Stop(void);
  bool UpdateSubscriber(StockProductAdvanced* what, bool *busy) override;
  bool WorkOnUpdates(void);
  bool OutputFieldNamesToFile(ofstream *f);
  bool OutputFieldValsToFile(StockProductAdvanced* stock);
  bool get_symbol_list(set<string>* tSymbolList) const {
    if (tSymbolList == nullptr)
      return false;
    *tSymbolList = symbol_list_;
    return true;
  }
  Logger* get_stock_logger(void) const {
    return (stock_logger_);
  }
 
  void set_id(int subscriber_id) {
    subscriber_id_ = subscriber_id;
  }
  int get_id(void) const {
    return subscriber_id_;
  }
  void set_name(const string& name) {
    subscriber_name_ = name;
 }
  string get_name(void) const { 
    return subscriber_name_; 
  }
  void AddSymbol(const string& Symbol) {
    symbol_list_.emplace(Symbol);
  }
  void RemoveSymbol(const string& Symbol) {
    symbol_list_.erase(Symbol);
  }
  void get_stock_queue(SynchronizedQueue<StockProductAdvanced>* StockUpdatesQue) {
    StockUpdatesQue = &stock_updates_que_;
  }
  long long dbg_subscriber_output_count_; // only for dubugging
  long long dbg_subscriber_input_count_; // onlyfor debugging
  void SetTargetAmount(long long val)
  {
	  assert(val > 0 && "target value is not valid");
	  target_amount_ = val;
  }
 
  void reset_counters(void) {
	  dbg_subscriber_output_count_ = 0;
	  dbg_subscriber_input_count_ = 0;
  }

  long long GetConsumedAmount() const {
	  return dbg_subscriber_output_count_;
  }
  bool WaitTargetWasReached(void);

 private:
  void ThreadFuncSubscriber(void);
  ostringstream	output_string_stream_; // output stream to write out processed updates.
  SynchronizedQueue<StockProductAdvanced> stock_updates_que_; // database updates are added here to back by consumer and processed from the front by subscriber
  bool	subscriber_stopped_;
  thread thread_; // subsriber sits in a loop in its own thread, doing work as well as processing updates.
  long long target_amount_ = numeric_limits<long long>::max();
  Logger* stock_logger_;
  int	subscriber_id_;
  string subscriber_name_;
  set<string> symbol_list_;
  mutex subscriber_mutex_;
  condition_variable condition_subscriber_;
  bool subscriber_done_;
  StockTickerConfiguration &config_ = Singleton<StockTickerConfiguration>::get_instance();
  TraceLogger &trace_log = Singleton<TraceLogger>::get_instance();
};

/////////////////  StockTickerSubscriber implementation///////////////////////////////////////

bool StockTickerSubscriber::WaitTargetWasReached(void) {
	trace_log.AddTrace("StockTickerSubscriber::WaitTargetWasReached");
	unique_lock<mutex> lck2(subscriber_mutex_);

	condition_subscriber_.wait(lck2, [this]() {return subscriber_done_; });

	lck2.unlock();
	cout << "WaitTargetWasReached StockTickerSubscriber was done" << endl;

	return true;
}
bool StockTickerSubscriber::CreateOutputFile(const int subscriber_id, const string &SubscriberName, string *fileName /*=nullptr*/) {
  if ((fileName != nullptr) && !fileName->empty()) {
    output_string_stream_.rdbuf()->str(*fileName);
  } else {    
    if ((subscriber_id < 0) || (subscriber_id >= config_.get_number_of_subscribers()))
      return false;
    if (SubscriberName.empty())
      return false;

    boost::filesystem::path p(config_.get_output_folder_full_path());
    p /= "subscriber_";
	//string tmpstr = boost::filesystem::canonical(p).string();
	string tmpstr = p.generic_string();
    output_string_stream_ << tmpstr << subscriber_id << "_" << SubscriberName << ".txt";
  }
  ofstream m_file(output_string_stream_.str(), ios::trunc);
  if (m_file.is_open() == false)
    return false;
  OutputFieldNamesToFile(&m_file);
  m_file << endl;
  m_file.close();
  stock_logger_->SetFileName(output_string_stream_.str(), subscriber_id_);
  return true;
}


bool StockTickerSubscriber::WorkOnUpdates(void) 
{ 
  StockProductAdvanced w;

  w.Clear();
  bool ret = stock_updates_que_.PopFront(&w); // if que empty it waits
  if ((ret == true) && (!w.get_symbol().empty()))
  {
    mutex local_mutex;
    lock_guard<std::mutex> lock(local_mutex);
    if (config_.get_enable_cout_disable_unit_test() == false)
      OutputFieldValsToFile(&w);
    else {
      cout << "subscriber_" << subscriber_id_ << endl;
      w.Print();
    }
    dbg_subscriber_output_count_++;
   
  }
  else {
    if (config_.get_enable_cout_disable_unit_test() == true)
      cout << "Subs:workonupdates ret " << ret << endl;
    if (!w.get_symbol().empty())
      return false;
  }

  return true;
}

/*
Subscriber threads works on updates to stocks with subscription
Also it works on other tasks occasionally if there are no updates for a while
*/
void StockTickerSubscriber::ThreadFuncSubscriber() {
  
  long long n = 0;
  trace_log.AddTrace("StockTickerSubscriber::ThreadFuncSubscriber begin");
  while (!subscriber_stopped_) {
	  if (dbg_subscriber_output_count_ < target_amount_) {
		  WorkOnUpdates();
		  if ((dbg_subscriber_output_count_ >= target_amount_) || (subscriber_stopped_ == true)) {
			  subscriber_done_ = true;
			  condition_subscriber_.notify_one();
		  }
	  }
      this_thread::yield();

  }

  trace_log.AddTrace("StockTickerSubscriber::ThreadFuncSubscriber end");
}

bool StockTickerSubscriber::UpdateSubscriber(StockProductAdvanced *what, bool *busy) {
  if (busy == nullptr)
    return false;
  if (what == nullptr)
    return false;
  *busy = false;
  bool ret = stock_updates_que_.PushBack(*what);
  if (ret == true)
    dbg_subscriber_input_count_++;
  else
    *busy = true;
 
  return true;
}

bool StockTickerSubscriber::Start(void) {
  trace_log.AddTrace("StockTickerSubscriber::Start");
  if (subscriber_stopped_ == false) {
     
    dbg_subscriber_output_count_ = 0;
    dbg_subscriber_input_count_ = 0;
    stock_updates_que_.Clear();
    thread_ = thread(&StockTickerSubscriber::ThreadFuncSubscriber, this); 
  }
    return true;
}

bool StockTickerSubscriber::Stop(void) {
  trace_log.AddTrace("StockTickerSubscriber::Stop");
    thread::id tid = thread_.get_id();
    if (subscriber_stopped_ == false) {
	  stock_updates_que_.SetStop(true);
      subscriber_stopped_ = true;
      if (thread_.joinable()) {
        thread_.join();
        subscriber_stopped_ = false;
      }
    }
    return true;
}

bool StockTickerSubscriber::OutputFieldNamesToFile(ofstream *f) {
  if (f == nullptr)
    return false;
  if (f->is_open() == false)
    return false;
  StockProductAdvanced stock;
  stock.OutputFieldNamesToFile(f);
  return true;
}

bool StockTickerSubscriber::OutputFieldValsToFile(StockProductAdvanced* stock) {
  if (stock == nullptr)
    return false;
  if (stock->get_symbol().empty())
    return false;
  ostringstream	strs;
  stock->OutputFieldValsToString(strs);
  return stock_logger_->PushBack(strs.str(),subscriber_id_);
  
}

}
#endif

