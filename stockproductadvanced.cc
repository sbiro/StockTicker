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
#include "stockproductadvanced.h"
#include <assert.h>
#include <iostream>
#include <sstream>
#include <ostream>
#include <random>

#include "constants.h"

namespace stocktickersb {
///////////////////////////  StockProductAdvanced class /////////////////////////////////
ostream& operator <<(ostream& os, const CompanySize& company_size) {
  if (company_size == CompanySize::small_cap)
    os << "small";
  else if (company_size == CompanySize::mid_cap)
    os << "medium";
  else if (company_size == CompanySize::large_cap)
    os << "large";
  else
    os << "undefined";
  return os;
}

ostream& operator <<(ostream& os, const StockType& stock_type) {
  if (stock_type == StockType::common)
    os << "common";
  else if (stock_type == StockType::preferred)
    os << "preferred";
  else
    os << "undefined";
  return os;
}

ostringstream& operator <<(ostringstream& os, const CompanySize& company_size) {
  if (company_size == CompanySize::small_cap)
    os << "small";
  else if (company_size == CompanySize::mid_cap)
    os << "medium";
  else if (company_size == CompanySize::large_cap)
    os << "large";
  else
    os << "undefined";
  return os;
}

ostringstream& operator <<(ostringstream& os, const StockType& stock_type) {
  if (stock_type == StockType::common)
    os << "common";
  else if (stock_type == StockType::preferred)
    os << "preferred";
  else
    os << "undefined";
  return os;
}

void StockProductAdvanced::Clear(void) {

  basic_stock_.Clear();
  advanced_stock_struct_.company_size = CompanySize::undefined;
  advanced_stock_struct_.stock_type = StockType::undefined;
}

void StockProductAdvanced::set_stock(const StockProductAdvanced &stock) {

  basic_stock_.set_stock(stock.basic_stock_);
  advanced_stock_struct_.company_size = stock.get_company_size();
  advanced_stock_struct_.stock_type = stock.get_stock_type();
}

bool StockProductAdvanced::GenerateRandomStock(void) {
  basic_stock_.GenerateRandomStock();
  // random company size 
  int temp = static_cast<int>(basic_stock_.get_sell_tick());
  temp = temp % 3;
  switch (temp) {
  case 0:
    advanced_stock_struct_.company_size = CompanySize::small_cap;
    break;
  case 1:
    advanced_stock_struct_.company_size = CompanySize::mid_cap;
    break;
  case 2:
    advanced_stock_struct_.company_size = CompanySize::large_cap;
    break;
  }
  // random stock type 
  temp = static_cast<int>(basic_stock_.get_sell_tick());
  temp = temp % 2;
  switch (temp) {
    case 0:
      advanced_stock_struct_.stock_type = StockType::common;
      break;
    case 1:
      advanced_stock_struct_.stock_type = StockType::preferred;
      break;
    default:
      //assert(false);
      return false;
      break;
  }
  return true;
}

void StockProductAdvanced::Print(void) {
  basic_stock_.Print();
  cout << "Company size: " << StockProductAdvanced::get_company_size() << endl;
  cout << "Stock type:  " << StockProductAdvanced::get_stock_type() << endl;
}

void StockProductAdvanced::OutputFieldNamesToFile(ofstream* f) {
  basic_stock_.OutputFieldNamesToFile(f);
  *f << "Company Size,     " << "Stock Type,";
}

void StockProductAdvanced::OutputFieldValsToString(ostringstream& strs) {
  basic_stock_.OutputFieldValsToString(strs);
  strs << get_company_size() << ",       " << get_stock_type() << ",";
}
} // namespace StockTickerSB {




