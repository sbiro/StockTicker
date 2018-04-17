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
#ifndef STOCKTICKERSB_CONFIGURATION_H_
#define STOCKTICKERSB_CONFIGURATION_H_


#include <cassert>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

#include <boost/filesystem.hpp>

#include "constants.h"

namespace stocktickersb {

using namespace std;

enum class FinancialProduct {
  undefined, BasicStock, AdvancedStock
};

// StockTickerSB.config is very similar to App.config used by .NET applications. Needs to reside where the source files are or where .exe is
class StockTickerConfiguration {
 public:
  StockTickerConfiguration(void) : version_("1.0"), 
                                   enable_cout_disable_unit_test_(false),
                                   output_folder_full_path_(""),
                                   number_of_subscribers_(0),
                                   subscribers_log_file_base_name_(""),
                               
                                   trace_log_file_(""),
                                   financial_product_(FinancialProduct::undefined) {}
  StockTickerConfiguration(const StockTickerConfiguration&) = delete;
  bool ReadConfigFile(const string& configFileName);
  bool get_enable_cout_disable_unit_test(void)  const{
    return enable_cout_disable_unit_test_;
  }
  void set_enable_cout_disable_unit_test(bool value)  {
    enable_cout_disable_unit_test_ = value;
  }
  string get_output_folder_full_path(void) const {
    return output_folder_full_path_;
  }
  int get_number_of_subscribers(void) {
    return number_of_subscribers_;
  }

  string get_subscribers_log_file_base_name(void) const {
    return subscribers_log_file_base_name_;
  }
  string get_trace_log_file(void) const {
    return trace_log_file_;
  }
  string get_version(void) const {
    return version_;
  }
  FinancialProduct get_financial_product(void) const {
    return financial_product_;
  }
 private:
   bool	enable_cout_disable_unit_test_; // true: to print out to console stock prices and not have ability to do unit-test.  
                                     // false: to not print out to console stock prices. Instead prompt user to type 'u' for doing unit test.
   string	output_folder_full_path_; // folder to save all output files: subscribers logs, error log, 
   int		number_of_subscribers_;	// less or equal to MAX_SUBSSCIBERS
   string	subscribers_log_file_base_name_; //  base name is used to make up "company name" for a subscriber. e.g: subscriber_3_shosh_3.txt.
   string  trace_log_file_; // base name of trace log file. Given extension .is CSV. 
                           //Goal of trace file is not to inlcude tracing of live updates because that would slow down and affect performance but more towards "bigger" steps taken so that when "assert" occurs the context of the error can be found out.
   FinancialProduct financial_product_; // financial product type for pricing engine
   string	config_file_name_; // in future if config file may get updated from the program
   string  version_; // software version of StockTickerSB  
  const int kNumConfigurables = 6;
};

bool StockTickerConfiguration::ReadConfigFile(const string& config_file_name) {
  string str2;
  string line;
  regex pat0(R"(\s*<add key="NumberOfSubscribers" value="(\d{1}))");
  regex pat1(R"(\s*<add key="EnableConsoleOutputDisableUnitTest" value="(\d{1}))");
  regex pat2(R"(\s*<add key="OutputFolderFullPath" value="(.{3,255}))");
  regex pat3(R"(\s*<add key="SubscribersLogFileBaseName" value="(\w{3,10}))");
  regex pat4(R"(\s*<add key="TraceLogFileName" value="(\w{3,20}))");
  regex pat5(R"(\s*<add key="StockProductType" value="(\w{2,20}))");
  
  smatch matches;
  int NumMatchedLines = 0;
cout << "opening " << config_file_name << endl;
  ifstream ifile(config_file_name);
  if (!ifile.is_open())
	  return false;
  config_file_name_ = config_file_name;
  getline(ifile, line); //<configuration>
  size_t siz= line.size();
  if (line[siz-1] == '\r')
    line.resize(siz-1);
   
  if (line != "<configuration>")
    return false;
 
  getline(ifile, line); //<appSettings>
  cout << line << endl;
  siz= line.size();
  if (line[siz-1] == '\r')
    line.resize(siz-1); 
  if (line != "  <appSettings>")
    return false;

  while (!ifile.eof()) {
      getline(ifile, line);
      siz= line.size();
      if (line[siz-1] == '\r')
        line.resize(siz-1);      
      cout << line << endl;
      if (line.empty())
          break; // end of file
    
  // remove whitespace 
      // erase-remove_if idiom because remove does not erase what it removes but "appends" it to after the "end" 
      //line.erase(std::remove_if(line.begin(), line.end(),
      //	[](char c) { return (c == '\r' || c == '\t' || c == ' ' || c == '\n');}), line.end());
      if (line == "  </appSettings>")
          break;
      
      if (regex_search(line, matches, pat0) && (matches.size() == 2) && matches[1].matched) { 
          str2 = matches[1];
          number_of_subscribers_ = stoi(str2);
          if ((number_of_subscribers_ >= 0) && (number_of_subscribers_ <= kMaxSubscribers))
              NumMatchedLines++;
          cout << "Matched #: " << NumMatchedLines << line << str2 << endl; 
          
      }
      else if (regex_search(line, matches, pat1) && (matches.size() == 2) && matches[1].matched) {
          str2 = matches[1];
          enable_cout_disable_unit_test_ = stoi(str2) == 1;
          NumMatchedLines++;
          cout << "Matched #: " << NumMatchedLines << line << str2 << endl;
      }
      else if (regex_search(line, matches, pat2) && (matches.size() == 2) && matches[1].matched) {
          str2 = matches[1];
          
          int pos = str2.find_last_of('"');
          str2.erase(str2.begin() + pos,str2.end());
          boost::filesystem::path p(str2);
          if (boost::filesystem::is_directory(p)) {
              output_folder_full_path_ = str2;
              NumMatchedLines++;
          }
          else {
             cout << "Error: " << str2 << " is not a folder" << endl;
          }
          cout << "Matched #: " << NumMatchedLines << " " << line << " " << str2 << endl;
      }
      else if (regex_search(line, matches, pat3) && (matches.size() == 2) && matches[1].matched) {
          str2 = matches[1];

          subscribers_log_file_base_name_ = str2;
          NumMatchedLines++;
          cout << "Matched #: " << NumMatchedLines << line << str2 << endl;
      }
      else if (regex_search(line, matches, pat4) && (matches.size() == 2) && matches[1].matched) {
          str2 = matches[1];
          boost::filesystem::path p(output_folder_full_path_);
          p /= str2;
          p += ".csv";
         // trace_log_file_ = boost::filesystem::canonical(p).string();
		  trace_log_file_ = p.generic_string();
          cout << trace_log_file_ << endl;
          //trace_log_file_ = output_folder_full_path_ + "\\" + str2 + ".csv";
          ofstream m_file(trace_log_file_, ios::trunc);
          NumMatchedLines++;
          cout << "Matched #: " << NumMatchedLines << " " << line << " " << str2 << endl;
      }
      else if (regex_search(line, matches, pat5) && (matches.size() == 2) && matches[1].matched) {
        str2 = matches[1];
        financial_product_ = FinancialProduct::undefined;
        if (!str2.compare("Basic"))
          financial_product_ = FinancialProduct::BasicStock;
        else if (!str2.compare("Advanced"))
          financial_product_ = FinancialProduct::AdvancedStock;
        else
          if (financial_product_ == FinancialProduct::undefined)
            return false;

        NumMatchedLines++;
        cout << "Matched #: " << NumMatchedLines << " " << line << " " << str2 << endl;
      }
      else;
  }
  if (NumMatchedLines != kNumConfigurables) {
    return false;
  }
  return true;
}
} // namespace StokTickerSB
#endif
