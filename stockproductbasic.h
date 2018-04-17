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
#ifndef STOCKTICKERSB_STOCKPRODUCTBASIC_H_
#define STOCKTICKERSB_STOCKPRODUCTBASIC_H_

#include <fstream>
#include <iomanip>
#include <sstream>
#include <ostream>
#include <string>

namespace stocktickersb {
using namespace std;

struct StockProductBasicStruct {
 string symbol;
  double buy_tick;
  double sell_tick;
};

class StockProductBasic {
 public:
  StockProductBasic(void) {
    Clear();
  }
  StockProductBasic(const StockProductBasic& stock) : basic_stock_struct_(stock.basic_stock_struct_) {}
  StockProductBasic operator=(const StockProductBasic& stock) {
    if (this == &stock)
      return *this;
    basic_stock_struct_.symbol = stock.basic_stock_struct_.symbol;
    basic_stock_struct_.buy_tick = stock.basic_stock_struct_.buy_tick;
    basic_stock_struct_.sell_tick = stock.basic_stock_struct_.sell_tick;
    return *this;
  }
  StockProductBasic(StockProductBasic&& stock) : basic_stock_struct_(stock.basic_stock_struct_) {
    stock.Clear();
  }
  StockProductBasic& operator=(StockProductBasic&& stock) {
    if (this == &stock)
      return *this;
    basic_stock_struct_.symbol = stock.basic_stock_struct_.symbol;
    basic_stock_struct_.buy_tick = stock.basic_stock_struct_.buy_tick;
    basic_stock_struct_.sell_tick = stock.basic_stock_struct_.sell_tick;
    stock.Clear();
    return *this;
  }
  string get_symbol() const { return basic_stock_struct_.symbol; }
  double get_buy_tick() const { return basic_stock_struct_.buy_tick; }
  double get_sell_tick() const { return basic_stock_struct_.sell_tick; }
  void set_stock(const StockProductBasic& stock);
  void set_symbol(const string& s) { basic_stock_struct_.symbol = s; }
  void set_buy_tick(double val) { basic_stock_struct_.buy_tick = val; }
  void set_sell_tick(double val) { basic_stock_struct_.sell_tick = val; }
  bool GenerateRandomStock(void);
  void Print(void);
  void Clear(void) {
    basic_stock_struct_.symbol.clear();
    basic_stock_struct_.buy_tick = 0;
    basic_stock_struct_.sell_tick = 0;
  }
  void OutputFieldNamesToFile( ofstream *f) {
    *f << "Symbol,   " << "buy tick,   " << "sell tick,   ";
  }
  void OutputFieldValsToString(ostringstream	&strs) {
    strs << get_symbol() << ",     " << setprecision(3) << get_buy_tick() << ",    " << setprecision(3) << get_sell_tick() << ",";
  }

 private:
  StockProductBasicStruct basic_stock_struct_;
};
} // namespace StockTickerSB {
#endif