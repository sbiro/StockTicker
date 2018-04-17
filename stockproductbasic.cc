
#include "stockproductbasic.h"

#include <iostream>
#include <random>
#include "constants.h"
  
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
namespace stocktickersb {
using namespace std;
////////////////////StockProductBasic class //////////////////////////////////////////
void StockProductBasic::set_stock(const StockProductBasic& stock) {
  basic_stock_struct_.symbol = stock.get_symbol();
  basic_stock_struct_.buy_tick = stock.get_buy_tick();
  basic_stock_struct_.sell_tick = stock.get_sell_tick();
}

bool StockProductBasic::GenerateRandomStock(void) {
  random_device rd;   // non-deterministic generator  
  mt19937 gen(rd());  // to seed mersenne twister. 
  uniform_int_distribution<> distr(0, kMaxDatabaseSymbols - 1);
  basic_stock_struct_.symbol = 'a' + static_cast<char>(distr(gen));
  uniform_real_distribution<> distr2(20.2, 100.8);
  basic_stock_struct_.buy_tick = 20.6 + distr2(gen);
  basic_stock_struct_.sell_tick = basic_stock_struct_.buy_tick + 5.4;
  return true;
}

void StockProductBasic::Print() {
  cout << "symbol: " << get_symbol() << endl;
  cout << "buy tick: " << get_buy_tick() << endl;
  cout << "sell tick: " << get_sell_tick() << endl;
}
} // namespace StockTickerSB {