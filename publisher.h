/*
Copyright (C) 2017 Shoshana Biro

This file is part of StockTicker, a free-software/open-source C++ library
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
#ifndef STOCKTICKERSB_PUBLISHER_H_
#define STOCKTICKERSB_PUBLISHER_H_

#include <array>
#include <set>
#include <string>

#include "configuration.h"
#include "constants.h"
#include "singleton.h"
#include "stocktickersubscriber.h"
#include "stockproductadvanced.h"

namespace stocktickersb {
////////////////PUBLISHER//////////////////////////
class Publisher {
 public:
  Publisher(void) { Clear();  }
  Publisher(const Publisher&) = delete;
  ~Publisher() { Clear(); }
  bool UpdateAllSubscribers(StockProductAdvanced* stock);
  void Clear(void) {    
	reset_counters();
    subscribers_.fill(nullptr);
  }
  bool AddSubscriber(StockTickerSubscriber* stock_subscriber);
  bool RemoveSubscriber(StockTickerSubscriber* stock_subscriber);
  long long dbg_published_count_;
  array<long long, kMaxSubscribers> dbg_subscibers_count_;

  void reset_counters(void) {
	  dbg_published_count_ = 0;
	  dbg_subscibers_count_.fill(0);
  }
  long long get_subscriber_quotes_amount(int index) {
	  assert((index >= 0) && (index < kMaxSubscribers) && "index out of range for subscribers quote");
	  return dbg_subscibers_count_[index];
  }
  long long get_published_amount(void) {
	  return dbg_published_count_;
  }

 private:
  array<StockTickerSubscriber *, kMaxSubscribers> subscribers_;  
  TraceLogger &trace_log = Singleton<TraceLogger>::get_instance();
  StockTickerConfiguration &config_ = Singleton<StockTickerConfiguration>::get_instance();
};


bool Publisher::UpdateAllSubscribers(StockProductAdvanced* stock) {
  
  if (stock == nullptr)
    return false;
  string Symbol = stock->get_symbol();
  if (Symbol.empty())
      return false;    
  if (config_.get_enable_cout_disable_unit_test() == true) 
    cout << "Publisher::updateallsubs Symbol" << Symbol << endl;     

  if (subscribers_.size() > 0) {
    set<string> SymList;
    for (auto stock_subscriber: subscribers_) {  
      if (stock_subscriber != nullptr) {
        stock_subscriber->get_symbol_list(&SymList);
        auto found = SymList.find(Symbol);
        if (found != SymList.end()) {
          bool busy = false;
          do {
            stock_subscriber->UpdateSubscriber(stock, &busy);
            if (busy == true)
              this_thread::yield();
            else {
              dbg_published_count_++;
              dbg_subscibers_count_[stock_subscriber->get_id()]++;
			  return true;
            }
          } while (busy == true);
          break;
        }
      }
    }    
  }
  return false;
}

bool Publisher::AddSubscriber(StockTickerSubscriber *stock_subscriber) {

  if (stock_subscriber == nullptr)
    return false;
  int id = stock_subscriber->get_id();
  if ((id < 0) || (id >= kMaxSubscribers))
    return false;
  subscribers_[id] = stock_subscriber;
  return true;
}

bool Publisher::RemoveSubscriber(StockTickerSubscriber *stock_subscriber) {
 
  if (stock_subscriber == nullptr)
    return false;
  int id = stock_subscriber->get_id();
  if ((id < 0) || (id >= kMaxSubscribers))
    return false;
  subscribers_[id] = nullptr;
  return true;
}

} // namespace StockTickerSB {

#endif