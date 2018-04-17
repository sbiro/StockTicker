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
#ifndef STOCKTICKERSB_STOCKTICKER_H_
#define STOCKTICKERSB_STOCKTICKER_H_

#include <array>
#include <list>
#include <regex>
#include <vector>

#include "configuration.h"
#include "constants.h"
#include "consumer.h"
#include "db_stockticker.h"
#include "logger.h"
#include "producer.h"
#include "publisher.h"
#include "singleton.h"
#include "stockproductadvanced.h"
#include "stocktickersubscriber.h"
#include "synchronizedqueue.h"
#include "tracelogger.h"

namespace stocktickersb {
using namespace std;

class StockTicker {
 public: 
  
 StockTicker(void) { 
   StockTicker(nullptr); 
 }

  explicit StockTicker(Logger* stock_logger) :all_logger_(stock_logger), stock_ticker_container_(), producer_(&engine_queue_, &input_container_), consumer_(&engine_queue_,&stock_ticker_container_,&publisher_) {
    Clear(); 
  }
  StockTicker(const StockTicker&) = delete;
  ~StockTicker(void);
  void Clear(void) {
    engine_queue_.Clear(); 
    input_container_.clear();  
  }
  vector<StockProductAdvanced>& GetInputContainer(void) { return input_container_;}
  bool SetInputFiles(array<StockTickerSubscriber*, kMaxSubscribers>* subscriber_list);
  bool Start(StockSource val);
  bool Stop(void);
  bool WaitTargetWasReached(array<StockTickerSubscriber*, kMaxSubscribers>* subscriber_list);
  bool AddSubscriber(StockTickerSubscriber* stock_subscriber) {
    return(publisher_.AddSubscriber(stock_subscriber));
  }
  bool RemoveSubscription(StockTickerSubscriber* stock_subscriber) {
    return(publisher_.RemoveSubscriber(stock_subscriber));
  }
  long long GetProducedAmount() {
	  return (producer_.GetProducedAmount());
  }
  long long GetConsumedAmount() {
	  return (consumer_.GetConsumedAmount());
  }
 private:
  SynchronizedQueue<StockProductAdvanced> engine_queue_;
  vector<StockProductAdvanced> input_container_;
  Producer producer_;
  Consumer consumer_;
  DBStockTickerContainer stock_ticker_container_;
  Publisher publisher_;
  StockTickerConfiguration &config_ = Singleton<StockTickerConfiguration>::get_instance();
  Logger* all_logger_;
  TraceLogger &trace_log = Singleton<TraceLogger>::get_instance();
};

///////////////////stockTicker implementation///////////////////////////////////////
// replacing random input to producer (of stock prices) to input from files (last state of output files)
bool StockTicker::SetInputFiles(array<StockTickerSubscriber*, kMaxSubscribers>* subscriber_list) {
  string tt;
  StockProductAdvanced stock;
   // input line is like: "a,  34.4,  67.23,common, preferred";
  regex pat("(\\w{1}),(\\d{1,5}[.]*\\d*),(\\d{1,5}[.]*\\d*),(\\w{1,10}),(\\w{1,10})");
  smatch matches;
  long long count = 0;
  int subscriber_id;

  trace_log.AddTrace("SetInputFiles ");

  if (subscriber_list == nullptr)
    return false;

  // stop producing and consuming random stock prices
  trace_log.AddTrace("StockTicker stopping producer from generating random quotes ");
  producer_.SetStockSource(StockSource::undefined);
  this_thread::sleep_for(100ms);
  long long res = producer_.GetProducedAmount();
  if (res == 0) {
	  trace_log.AddTrace("Error produced random amt 0");
	  return false;
  }
  consumer_.SetTargetAmount(res);
  long long res2 = consumer_.GetConsumedAmount();
  if (res2 < res)
	  consumer_.WaitTargetWasReached();

  array<StockTickerSubscriber*, kMaxSubscribers> sts = (*subscriber_list);
  long long res3 = 0;
  for (subscriber_id = 0, res2=0; subscriber_id < config_.get_number_of_subscribers(); subscriber_id++) {
	  res = sts[subscriber_id]->GetConsumedAmount();
	  sts[subscriber_id]->SetTargetAmount(res);
	  res3 += res;

	  all_logger_->SetTargetAmount(res,subscriber_id);
	  res2 += all_logger_->GetLoggedAmount(subscriber_id);
	 
  }
  for (;subscriber_id < kMaxSubscribers; subscriber_id++)
    all_logger_->SetTargetAmount(0,subscriber_id);
  if (res2 < res3)
	  all_logger_->WaitTargetWasReached();
  producer_.SetTargetAmount(res3);
  consumer_.SetTargetAmount(res3);

  input_container_.reserve(5000);
  
  for (subscriber_id = 0, count=0; subscriber_id < config_.get_number_of_subscribers(); subscriber_id++) {
    string line;
    string f;
    f = sts[subscriber_id]->get_file_name();
    ifstream ifile(f);
    if (!ifile.is_open()) {
      cout << "Input file: " << f << "Not found" << endl;
      continue;
    }
	  count = 0;
    getline(ifile, line); // header
    while (!ifile.eof()) {
      getline(ifile, line);
      // remove whitespace: erase-remove_if idiom because remove does not erase what it removes but "appends" it to after the "end" 
      line.erase(std::remove_if(line.begin(), line.end(),
        [](char c) { return (c == '\r' || c == '\t' || c == ' ' || c == '\n');}), line.end());
      // search for input line like: "a,  34.4,  67.23," with 3 sub-patterns
      if (regex_search(line, matches, pat) && (matches.size() == 6)) {
        // break line into symbol, buy Tick, sell tick
        string str2 = matches[1]; // symbol
        stock.set_symbol(str2);
        str2 = matches[2]; // buy tick
        double d = stod(str2);
        stock.set_buy_tick(d);        
        str2 = matches[3]; // sell tick
        d = stod(str2);
        stock.set_sell_tick(d);
        str2 = matches[4]; // company size
        if (str2 == "small")
          stock.set_company_size(CompanySize::small_cap);
        else if (str2 == "medium")
          stock.set_company_size(CompanySize::mid_cap);
        else if (str2 == "large")
          stock.set_company_size(CompanySize::large_cap);
        else
          stock.set_company_size(CompanySize::undefined);
        str2 = matches[5]; // stock type
        if (str2 == "preferred")
          stock.set_stock_type(StockType::preferred);
        else if (str2 == "common")
          stock.set_stock_type(StockType::common);
        else
          stock.set_stock_type(StockType::undefined);
        input_container_.push_back(stock);
		    count++;
      }
    }
    ifile.close();
	  trace_log.AddTrace("Subscriber " + to_string(subscriber_id) + " log amount: " + to_string(count));
    // truncate the output file after it was read in
    sts[subscriber_id]->CreateOutputFile(0, "", &f);
  }
  if (input_container_.size()==0) {

    trace_log.AddTrace(" Error: no subscriber input file data ");
    return false;
  }
  else if (input_container_.size() != res3) {
	  trace_log.AddTrace(" Error: input file data size mismatch");
	  return false;
  }
  else
    trace_log.AddTrace("Starting. input size: "+ to_string(input_container_.size()));
 
  // reset counters because we are about to restart the producer from file input

  publisher_.reset_counters();
  producer_.reset_counters();
  consumer_.reset_counters();

  for (subscriber_id = 0; subscriber_id < config_.get_number_of_subscribers(); subscriber_id++) {
	  sts[subscriber_id]->reset_counters();
	  all_logger_->reset_counters();
  }

  producer_.SetStockSource(StockSource::file_input);
	 
//  config_.set_enable_cout_disable_unit_test(true);// temp code for debugging on Linux. To see why files were not written.
   
  return true;
}

bool StockTicker::Start(StockSource val) {
  trace_log.AddTrace("StockTicker::Start");

  publisher_.reset_counters();
  consumer_.StartConsumer();
  producer_.StartProducer(val);
  
  return true;
}

bool StockTicker::Stop(void) { 
  
  //trace_log.AddTrace("StockTicker::Stop");
  cout << "StockTicker::Stop" << endl;
  engine_queue_.SetStop(true);
  producer_.StopProducer();
  consumer_.StopConsumer();

  return true;
}

bool StockTicker::WaitTargetWasReached(array<StockTickerSubscriber*, kMaxSubscribers>* subscriber_list) { // called by unit test so I dont implement it as condition variables..
  trace_log.AddTrace("StockTicker::WaitTargetWasReached");
  producer_.WaitTargetWasReached();
  consumer_.WaitTargetWasReached();
  array<StockTickerSubscriber*, kMaxSubscribers> sts = *subscriber_list;
  
  for (int id=0; id < config_.get_number_of_subscribers(); id++)
     sts[id]->WaitTargetWasReached();

  all_logger_->WaitTargetWasReached();  
  return true;
}

StockTicker::~StockTicker(void) {
  producer_.StopProducer();
  consumer_.StopConsumer();
  Clear();
}

} //namespace StockTickerSB 
#endif