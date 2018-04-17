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
#ifndef STOCKTICKERSB_STOCKPRODUCTADVANCED_H_
#define STOCKTICKERSB_STOCKPRODUCTADVANCED_H_

#include "stockproductbasic.h"

#include <string>

namespace stocktickersb {
using namespace std;

/// enumerations classes and custom output to print them ///////////////////////////////////
enum class CompanySize {
  undefined, small_cap, mid_cap, large_cap
};

enum class StockType {
  undefined, common, preferred
};

struct StockProductAdvancedStruct {
  CompanySize company_size;
  StockType	stock_type;
};

class StockProductAdvanced {
 public:
   StockProductAdvanced(void) {
     Clear();
   }
   StockProductAdvanced(const StockProductAdvanced& stock) {
     set_symbol(stock.basic_stock_.get_symbol());
     set_buy_tick(stock.basic_stock_.get_buy_tick());
     set_sell_tick(stock.basic_stock_.get_sell_tick());
     set_company_size(stock.get_company_size());
     set_stock_type(stock.get_stock_type());
   }
   StockProductAdvanced operator=(const StockProductAdvanced& stock) {
     if (this == &stock)
       return *this;
     set_symbol(stock.basic_stock_.get_symbol());
     set_buy_tick(stock.basic_stock_.get_buy_tick());
     set_sell_tick(stock.basic_stock_.get_sell_tick());
     set_company_size(stock.get_company_size());
     set_stock_type(stock.get_stock_type());
     return *this;
   }
   StockProductAdvanced(StockProductAdvanced&& stock) {
     set_symbol(stock.basic_stock_.get_symbol());
     set_buy_tick(stock.basic_stock_.get_buy_tick());
     set_sell_tick(stock.basic_stock_.get_sell_tick());
     set_company_size(stock.get_company_size());
     set_stock_type(stock.get_stock_type());
     stock.Clear();
   }
   StockProductAdvanced& operator=(StockProductAdvanced&& stock) {
     if (this == &stock)
       return *this;
     set_symbol(stock.basic_stock_.get_symbol());
     set_buy_tick(stock.basic_stock_.get_buy_tick());
     set_sell_tick(stock.basic_stock_.get_sell_tick());
     set_company_size(stock.get_company_size());
     set_stock_type(stock.get_stock_type());
     stock.Clear();
   
     return *this;
   }
  void Clear(void);
  string get_symbol() const { return  basic_stock_.get_symbol(); }
  double get_buy_tick() const { return basic_stock_.get_buy_tick(); }
  double get_sell_tick() const { return basic_stock_.get_sell_tick(); }
  CompanySize get_company_size() const { return advanced_stock_struct_.company_size; }
  StockType get_stock_type() const { return advanced_stock_struct_.stock_type; }
  void Print(void);
  void set_stock(const StockProductAdvanced &stock);
  void set_symbol(const string& s) { basic_stock_.set_symbol(s); }
  void set_buy_tick(double val) { basic_stock_.set_buy_tick(val); }
  void set_sell_tick(double val) { basic_stock_.set_sell_tick(val); }
  void set_company_size(CompanySize cs) {
    advanced_stock_struct_.company_size = cs;
  }
  void set_stock_type(StockType st) {
    advanced_stock_struct_.stock_type = st;
  }
  bool GenerateRandomStock(void);
  void OutputFieldNamesToFile(ofstream* f);
  void OutputFieldValsToString(ostringstream& strs);
 private:
  StockProductBasic basic_stock_;
  StockProductAdvancedStruct advanced_stock_struct_;
};
} // namespace StockTickerSB 
#endif
