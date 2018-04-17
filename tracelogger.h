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
#ifndef STOCKTICKERSB_TRACELOGER_H_
#define STOCKTICKERSB_TRACELOGER_H_


#include <iostream>
#include <fstream>
#include <string>

#include "constants.h"
#include "logger.h"

namespace stocktickersb {

using namespace std;

class TraceLogger {
 public:
  TraceLogger(int TraceID= kTraceId)  {
    Clear(); 
    trace_id_ = TraceID;
  }
  TraceLogger(TraceLogger&) = delete;
  ~TraceLogger(void) {Clear();}
  bool AddTrace(const string& str) {
    if (str.empty())
      return false;
    if (trace_log_ == nullptr)
      return false;
    cout << str << endl;
    return trace_log_->PushBack(str,trace_id_);

  }
  bool Start(const string& file_name, Logger* all_logger)  {
    if (file_name.empty())
      return false;
    if (all_logger == nullptr)
      return false;
    cout << " trace file: " << file_name << endl;
    ofstream f(file_name);
    if (!f.is_open()) {
      cout << "Error: Trace file: " << file_name << " could not open" << endl;
      return false;
    }
    f << "try\n";
    f.close();
    trace_log_ = all_logger;
    bool ret = trace_log_->SetFileName(file_name, trace_id_);
    if (ret == false)
      return false;
	  trace_log_->Start();
    return true;
  }
 // bool Stop(void) { 
   
	//  if (trace_log_)
	//	  return trace_log_->Stop();
//	  else
	//	  return false;
 // }
  void Clear(void) {
  }

 private:
 
  Logger* trace_log_;
  int trace_id_;
};

} // namespace StockTickerSB 
#endif
