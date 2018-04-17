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
#include <cassert>
#include <fstream>
#include <istream>
#include <iostream>
#include <list>
#include <regex>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

#include "constants.h"
#include "configuration.h"
#include "singleton.h"
#include "stocktickersubscriber.h"
#include "stockticker.h"
#include "tracelogger.h"
#include "logger.h"
using namespace stocktickersb;
using namespace std;

// create instance of singleton class
template<typename T>
T* Singleton<T>::instance_pointer = nullptr;

template <typename T>
static bool CompareInputToOutput(T& elem1, T& elem2) {
  return ((elem1.get_symbol() == elem2.get_symbol()) &&
    (elem1.get_buy_tick() == elem2.get_buy_tick()) &&
    (elem1.get_sell_tick() == elem2.get_sell_tick()));
}


/*
test()
Replace random inputs to pricing producer with reading from files.
the last output files become the input files.
run until all entries in the input files have been fully processed.

*/

static bool RunTest(array<StockTickerSubscriber*,kMaxSubscribers>* subscriber_list, StockTicker *stock_ticker, Logger* logger) {
  string file_name;
  double d;
  StockProductAdvanced stock;
  vector<StockProductAdvanced> test_input_container; // does it have to be a vector?
  string str2;
  string line;
  regex pat("(\\w{1}),(\\d{1,5}[.]*\\d*),(\\d{1,5}[.]*\\d*),(\\w{1,10}),(\\w{1,10})");
  smatch matches;
  TraceLogger &trace_log = Singleton<TraceLogger>::get_instance();
  StockTickerConfiguration &config = Singleton<StockTickerConfiguration>::get_instance();
  bool ret;
  size_t amt;
  int subscriber_id;

  trace_log.AddTrace("RunTest");

  if (subscriber_list == nullptr)
    return false;

  if (stock_ticker == nullptr)
	  return false;

  if (logger == nullptr)
	  return false;

  ret = stock_ticker->SetInputFiles(subscriber_list);
  if (ret == false) {
   
  	trace_log.AddTrace("SetInputFiles failed");
    return false;
  }
  this_thread::yield();

  stock_ticker->WaitTargetWasReached(subscriber_list);
  // read in output files of subscribers into test_input_container
  test_input_container.clear();
  test_input_container.reserve(5000);
  array<StockTickerSubscriber*, kMaxSubscribers> sts = (*subscriber_list);
  for (subscriber_id = 0; subscriber_id < config.get_number_of_subscribers(); subscriber_id++) {
	  file_name = sts[subscriber_id]->get_file_name();
    if (file_name.empty()) {
      trace_log.AddTrace("subscriber "+to_string(subscriber_id)+"no file specified");
      return false;
    }
    ifstream ifile(file_name);
    if (ifile.is_open())
      amt=0;
    else {
      trace_log.AddTrace(string("output file: ") + file_name + string("failed to open"));
      return false;
    }
    getline(ifile, line); // header
    if (line.empty()) {
      trace_log.AddTrace("output file: " + file_name + "is empty"); 
      return false;
    }
    while (!ifile.eof()) {
      getline(ifile, line);
      if (line.empty())
        break; // end of file
      // remove whitestest failed " " "pace // erase-remove_if idiom because remove does not erase what it removes but "appends" it to after the "end" 
      line.erase(std::remove_if(line.begin(), line.end(),
        [](char c) { return (c == '\r' || c == '\t' || c == ' ' || c == '\n');}), line.end());

      if ((regex_search(line, matches, pat) == false) || (matches.size() != 6))
        return false;
      // break line into symbol, buy Tick, sell tick
      str2 = matches[1]; // symbol
      // buy tick
      stock.set_symbol(str2);
      str2 = matches[2];
      d = stod(str2);
      stock.set_buy_tick(d);
      // sell tick
      str2 = matches[3];
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
      test_input_container.push_back(stock);
      ++amt;
    }
    trace_log.AddTrace("Subscriber " + to_string(subscriber_id) + " stocks found: " + to_string(amt));
    ifile.close();
  }
  
  // compare to original input files
  vector<StockProductAdvanced>& OriginalInputVec = stock_ticker->GetInputContainer();
  if (OriginalInputVec.size() != test_input_container.size()) {
    trace_log.AddTrace("input size: " + to_string(OriginalInputVec.size()) + "does not match output size: " + to_string(test_input_container.size()) );
	trace_log.AddTrace("Producer produced: " + to_string(stock_ticker->GetProducedAmount()));
	trace_log.AddTrace("Consumer consumed: " + to_string(stock_ticker->GetConsumedAmount()));
	for (int subscriber_id = 0; subscriber_id < config.get_number_of_subscribers(); subscriber_id++) {
		trace_log.AddTrace("Subscriber " + to_string(subscriber_id) + " quotes consumed: " + to_string(sts[subscriber_id]->GetConsumedAmount()));
		trace_log.AddTrace("Logger " + to_string(subscriber_id) + " quotes saved: " + to_string(logger->GetLoggedAmount(subscriber_id)));
	}
    return false;
  }

  if (equal(test_input_container.begin(), test_input_container.end(), OriginalInputVec.begin(), CompareInputToOutput<StockProductAdvanced>)) {
    
  	trace_log.AddTrace("test successful");
    return true;
  }
  else {
    
  	trace_log.AddTrace("input to output values comparison failed");
    return false;
  }
  return true;
}


bool MainFunction(Logger* logger) {
  
  array<StockTickerSubscriber *,kMaxSubscribers> subscriber_list; // Should be changed into a fixed size array so no mutex is needed if adding or deleting subscribers live
  StockTickerConfiguration &config = Singleton<StockTickerConfiguration>::get_instance();
  TraceLogger &trace_log = Singleton<TraceLogger>::get_instance();
  StockTicker stock_ticker(logger);

  if (config.get_number_of_subscribers() > 0) {

    // initialize subscribers with name and 3 different stocks each
    for (int subscriber_id = 0; subscriber_id < config.get_number_of_subscribers(); subscriber_id++) {
     
      subscriber_list[subscriber_id] = new StockTickerSubscriber(logger);
      if (subscriber_list[subscriber_id] == nullptr)
        return false;
     
      subscriber_list[subscriber_id]->set_id(subscriber_id);
      string s = to_string(subscriber_id);
      string name(config.get_subscribers_log_file_base_name() + "_" + s);
      subscriber_list[subscriber_id]->set_name(name);
      subscriber_list[subscriber_id]->CreateOutputFile(subscriber_id, name);
      // subscribe to some stocks
      char temp = 'a' + static_cast<char>(subscriber_id * 3);
      string str = { temp };
      subscriber_list[subscriber_id]->AddSymbol(str);
    
      temp = 'b' + static_cast<char>(subscriber_id * 3);
      string str2 = { temp };
      subscriber_list[subscriber_id]->AddSymbol(str2);

      temp = 'c' + static_cast<char>(subscriber_id * 3);
      string str3 = { temp };
      subscriber_list[subscriber_id]->AddSymbol(str3);
    }
    // start the subscribers
 
    for (int subscriber_id = 0; subscriber_id < config.get_number_of_subscribers(); subscriber_id++) {
      stock_ticker.AddSubscriber(subscriber_list[subscriber_id]);
      subscriber_list[subscriber_id]->Start();
	  this_thread::sleep_for(200ms);
    }
  }
  this_thread::sleep_for(1s);
  // start generating random stocks, consuming them and publishing to subscribers

  stock_ticker.Start(StockSource::random_input);

  this_thread::sleep_for(1s);

  char ch;
  cout << "Enter q  <ENTER> to quit" << endl;
  if (config.get_number_of_subscribers() > 0)
  {
	  cout << "There are " << config.get_number_of_subscribers() << " subscribers.\n You can watch stock quotes for each subscriber at folder: " << config.get_output_folder_full_path() << endl;
  }
  if (config.get_enable_cout_disable_unit_test() == false) 
    cout << "Enter u  <ENTER> to test if subscribers last output becomes input and matches" << endl;

  bool quit = false;
  bool test = false;

  while (!quit && !test) {
    cin >> ch;
    if ((ch == 'q') || (ch == 'Q')) {
      quit = true;

    } else if ((ch == 'u') || (ch == 'U')) {
      test = true;
    } else {
      this_thread::yield();
    }
  }
  if (test) {
    bool ret = RunTest(&subscriber_list,&stock_ticker,logger);
	if (ret == false) {
		
		trace_log.AddTrace("error in RunTest");
	}
    quit = true;
  }
  if (quit) {
    // quitting the program
    stock_ticker.Stop();
	  for (int subscriber_id = 0; subscriber_id < config.get_number_of_subscribers(); subscriber_id++) {
		  if (subscriber_list[subscriber_id] != nullptr) {
			  subscriber_list[subscriber_id]->Stop();
			  this_thread::yield();
		  }
	  }
    //stock_ticker.Stop();
  } // quit
  return true;
}

int main() {
	/*  
		http://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio
		the synchronized C++ streams are unbuffered, and each I / O operation on a C++ stream is immediately applied to the corresponding C stream's buffer. This makes it possible to freely mix C++ and C I/O. 
		In addition, synchronized C++ streams are guaranteed to be thread - safe(individual characters output from multiple threads may interleave, but no data races occur)
		If the synchronization is turned off, the C++ standard streams are allowed to buffer their I / O independently, which may be considerably faster in some cases.
		By default, all eight standard C++ streams are synchronized with their respective C streams.
		If this function is called after I / O has occurred on the standard stream, the behavior is implementation - defined: implementations range from no effect to destroying the read buffer.
	*/
	std::ios::sync_with_stdio(false); // see above

	try {
		unsigned num_cpus = thread::hardware_concurrency(); 

		StockTickerConfiguration &config = Singleton<StockTickerConfiguration>::get_instance();
		Logger all_logger;
		TraceLogger &trace_log = Singleton<TraceLogger>::get_instance();

		boost::filesystem::path currPath(boost::filesystem::current_path());
   
		currPath /= "stockticker.config";
		string config_file_name(currPath.string());
		
		ifstream ifile(config_file_name);
		if (!ifile.is_open()) {
			cout << "Error opening " << config_file_name << endl;
			return 0;
		}
		ifile.close();
    bool bret = config.ReadConfigFile(config_file_name);
    if (bret == false) {
      cout << "Error: stockticker.config syntax " << endl;
    }
    else {
      all_logger.Start();
      bret = trace_log.Start(config.get_trace_log_file(), &all_logger);
      if (bret == false) {
        cout << "Error: trace log failed to start" << endl;

      }
      else {

        trace_log.AddTrace(config.get_version());
        trace_log.AddTrace(config.get_output_folder_full_path());
        trace_log.AddTrace("Number of processors" + to_string(num_cpus));
        string str = to_string(config.get_number_of_subscribers());
        str += " number of subscribers";
        trace_log.AddTrace(str);
        str = to_string(config.get_enable_cout_disable_unit_test());
        str += " to enable cout and disable unit test";
        trace_log.AddTrace(str);

        if (config.get_financial_product() == FinancialProduct::AdvancedStock) {
          bool ret = MainFunction(&all_logger);
          if (ret == false) {

            trace_log.AddTrace("MainFunction returned error");
          }
        }       
       
        all_logger.Stop();
      } // if (bret == false)  trace log
    } // if (bret == false) { for readConfig
    cout << "About to exit stockticker 1" << endl;
  }
  catch (const boost::filesystem::filesystem_error& ex)
  {
	  std::cout << "************* filesystem_error *****************\n";
	  std::cout << ex.what() << '\n';
  }
  catch (...)
  {
	  cout << "Main() ... Exception" << endl;
  }
  cout << "About to exit stockticker 2" << endl;
  return 0;
} // main



