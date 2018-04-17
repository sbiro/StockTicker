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
#ifndef STOCKTICKERSB_STOCKLOGGER_H_
#define STOCKTICKERSB_STOCKLOGGER_H_


#include <cassert>
#include <fstream>
#include <limits>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "configuration.h"
#include "constants.h"
#include "singleton.h"
#include "synchronizedqueue.h"

namespace stocktickersb {
using namespace std;

struct StringFile {
  string info;  // trace string or subscriber string
  int index; // file index
};

class Logger {
 public:
  Logger(void) {
    Clear();
    file_list_.resize(kMaxLogs);
	  dbg_received_count_vec_.resize(kMaxLogs);
	  dbg_logged_count_vec_.resize(kMaxLogs);

	  target_amount_list_.resize(kMaxSubscribers);
	  target_amount_list_.assign(kMaxSubscribers, numeric_limits<long long>::max());
  }
  Logger(const Logger&) = delete;

  ~Logger(void) { 
   
    Stop();
    Clear(); 
  }

  bool PushBack(const string& rec, int index) {
    if (!rec.empty() && (index >= 0) && (index < kMaxLogs)) {
      StringFile string_file;
      string_file.info = rec;
      string_file.index = index;
      bool ret;
      do {
        ret = log_queue_.PushBack(string_file);    
        this_thread::yield();
      } while (ret == false);
      dbg_received_count_vec_[string_file.index]++;
      return true;
    } else {
      return false;
    }
  }
// add an index as a parameter
 bool SetFileName(const string& f,int index) {     
  if (!f.empty() && (index >= 0) && (index < kMaxLogs)) {
    // rest if file is valid
    ofstream file(f, ios::app);
    if (file.is_open() == false)
    {
      cout << "Error: Logger SetFileName() failed to open: " << f << endl;
      return false;
    }
    file.close();

    if (file_list_.size() < static_cast<size_t>(index + 1))
      return false;
    file_list_[index] = f;
    return true;
  } else {
    return false;
  }
};

  bool Start(void);
  bool Stop();
  void Clear(void) { 
    file_list_.clear(); 
    log_queue_.Clear();
	  logger_stopped_ = true;
    log_thread_is_running_ = false;
	  dbg_received_count_vec_.clear();
	  dbg_logged_count_vec_.clear();
	  logger_done_ = false;

  }
 
  vector<int> dbg_logged_count_vec_;
  vector<int> dbg_received_count_vec_;

  void SetTargetAmount(long long val,int index)
  {
	  assert( (index >=0) && (index < kMaxSubscribers) && "index value is not valid");
	  target_amount_list_[index] = val;
  }

  void reset_counters(void) {
	  fill(dbg_logged_count_vec_.begin(), dbg_logged_count_vec_.end(), 0);
	  fill(dbg_received_count_vec_.begin(), dbg_received_count_vec_.end(), 0);
  }

  long long GetLoggedAmount(int index) const {
	  assert((0 <= index) && (index < static_cast<int>(dbg_logged_count_vec_.size())) && "Logger GetProducedAmount programming error");
	  return dbg_logged_count_vec_[index];
  }

  bool WaitTargetWasReached(void) {
    cout << "logger WaitTargetWasReached" << logger_done_ << endl;  
    bool target_reached;
    int i;
    if (logger_done_ == false) {
      for (i = 0,target_reached=true; i < kMaxSubscribers; ++i) {
        if (dbg_logged_count_vec_[i] < target_amount_list_[i]) {
          cout << "i:" << i << " " << dbg_logged_count_vec_[i] << " " << target_amount_list_[i] << endl;
          target_reached = false;
          break;
        }
      }    
    } 
    if (target_reached == false) {
	    unique_lock<mutex> lck2(logger_mutex_);

	    condition_logger_.wait(lck2, [this]() {return logger_done_; });

      lck2.unlock();
    }
    else
      cout << " no need to lock logger" << endl;
	  cout << "WaitTargetWasReached logger was done" << endl;

	  return true;
  }

 private:
  SynchronizedQueue<StringFile> log_queue_; // holds strings and the index into the file to save to
  vector<string> file_list_; // list of file names   
  thread log_thread_;
  void ThreadFuncLogger(void);
  bool log_thread_is_running_;
  bool logger_stopped_;
  vector<long long> target_amount_list_;
  mutex logger_mutex_;
  condition_variable condition_logger_;
  bool logger_done_;
  StockTickerConfiguration &config_ = Singleton<StockTickerConfiguration>::get_instance();
};



void Logger::ThreadFuncLogger(void) { 
  
  if (file_list_.empty())
		return;

  StringFile string_file;
  bool ret;
  int err;
  bool target_reached = false;

  cout << "Logger thread running" << endl;
  while (!logger_stopped_ ) {	
    if (!target_reached) {
      ret = log_queue_.PopFront(&string_file);      
    
      if ((ret == true) && (logger_stopped_ == false) && (string_file.info.size() > 0) && (string_file.index >= 0) && (string_file.index <= kTraceId)) {
        ofstream m_file(file_list_[string_file.index], ios::app); // append to output file
        if (m_file.is_open()) {
          
          m_file << string_file.info;
          m_file << endl;
          m_file.flush();
          m_file.close();
          dbg_logged_count_vec_[string_file.index]++;
        }
        else {
          cout << " Error opening log file: " << file_list_[string_file.index] << endl;
        }

      }
      else {
        err = 1;
        cout << " Error Logger 1: " << endl;
      }
      if (!logger_stopped_) {
        // check if logging for all subscribers finished
        for (int i = 0,target_reached=true; i < kMaxSubscribers; ++i) {
          if (dbg_logged_count_vec_[i] < target_amount_list_[i]) {
            target_reached = false;
            break;
          }

        }
        if (target_reached == true) {
          cout << "logger reached target" << endl;
          logger_done_ = true;
          condition_logger_.notify_one();
          cout << "About to exit stockticker 1" << endl;
        }
      }
    }
    this_thread::yield();
  } // while WaitTargetWasReached
  cout << "Logger::ThreadFuncLogger() ended" << endl;  
}


bool Logger::Start(void) {
  if (logger_stopped_ == false)
	  return true;
  logger_stopped_= false;
 
  fill(dbg_logged_count_vec_.begin(), dbg_logged_count_vec_.end(), 0);
  fill(dbg_received_count_vec_.begin(), dbg_received_count_vec_.end(),0);
  if (log_thread_is_running_ == false) {
    log_thread_ = thread(&Logger::ThreadFuncLogger, this);
    log_thread_is_running_ = true;
  }
  return true;
}


bool Logger::Stop(void) {
  if (logger_stopped_ == false) {
    cout << "Stopping logger" << endl;
    this->log_queue_.SetStop(true);
    cout << "Stopping logger 2" << endl;
    logger_stopped_ = true;
    if (log_thread_.joinable()) {
      cout << "Stopping logger 3" << endl;
      log_thread_.join();
      cout << "Stopping logger 4 " << endl;
      log_thread_is_running_ = false;
    }
  }
  return true;
}

} // namespace StockTickerSB {
#endif
