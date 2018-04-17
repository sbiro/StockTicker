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
#ifndef STOCKTICKERSB_DB_STOCKTICKER_H_
#define STOCKTICKERSB_DB_STOCKTICKER_H_


#include <algorithm>
#include <cassert>
#include <iostream>
#include <mutex>
#include <set>
#include <string>

#include "configuration.h"
#include "constants.h"
#include "stockproductadvanced.h"
#include "tracelogger.h"

namespace stocktickersb {

using namespace std;

///////////////DB_StockTickerItem/////////////////////
class DBStockTickerItem {
 public:
 
 DBStockTickerItem() { }
  DBStockTickerItem(const DBStockTickerItem& stock_ticker_item);
  DBStockTickerItem operator= (const DBStockTickerItem& stock_ticker_item);
  DBStockTickerItem(DBStockTickerItem&& stock_ticker_item);
  DBStockTickerItem& operator= (DBStockTickerItem&& stock_ticker_item);

  DBStockTickerItem(const StockProductAdvanced& sta) {
	  stock_ticker_item_ = sta;
  }
  DBStockTickerItem(StockProductAdvanced&& sta) {
	  stock_ticker_item_ = sta;
	  sta.set_symbol(string(""));
  }

  bool operator == (const DBStockTickerItem& sti) const {
	  return (stock_ticker_item_.get_symbol() == sti.get_symbol());
  }

  bool operator < (const DBStockTickerItem& sti) const {
	  return (stock_ticker_item_.get_symbol() < sti.get_symbol());
  }

  ~DBStockTickerItem(void) {
    Clear();
  }
  void Clear(void);
  StockProductAdvanced getStock(void) const { 
    return stock_ticker_item_; 
  }
  
  bool set_stock(StockProductAdvanced&& val);
  bool set_stock(const StockProductAdvanced& val);
  void set_symbol(string &str) {
    stock_ticker_item_.set_symbol(str);
  }
  string get_symbol(void) const { 
	  return (stock_ticker_item_.get_symbol());

  }
 private:
  StockProductAdvanced	stock_ticker_item_;
};

///////////////////  DB_StockTickerItem implementation////////////////////////////////////////

 DBStockTickerItem::DBStockTickerItem(const DBStockTickerItem& stock_ticker_item) {
  stock_ticker_item_.set_stock(stock_ticker_item.getStock());

}

DBStockTickerItem DBStockTickerItem::operator= (const DBStockTickerItem& stock_ticker_item) {
  if (this == &stock_ticker_item)
    return *this;
  stock_ticker_item_.set_stock(stock_ticker_item.getStock());
 
  return *this;
}

DBStockTickerItem::DBStockTickerItem(DBStockTickerItem&& stock_ticker_item) {
  stock_ticker_item_.set_stock(move(stock_ticker_item.getStock()));
  string empty = string("");
  stock_ticker_item.set_symbol(empty);

}

DBStockTickerItem& DBStockTickerItem::operator= (DBStockTickerItem&& stock_ticker_item) {
  if (this == &stock_ticker_item)
    return *this;
  stock_ticker_item_.set_stock(move(stock_ticker_item.getStock()));
  return *this;
}

bool DBStockTickerItem::set_stock(const StockProductAdvanced& val) {
  if (val.get_symbol().empty())
    return false;
  stock_ticker_item_ = val;
  return true;
}

bool DBStockTickerItem::set_stock(StockProductAdvanced&& val) {
  if (val.get_symbol().empty())
    return false;
  stock_ticker_item_ = move(val);
  return true;
}

void DBStockTickerItem::Clear(void) {
  stock_ticker_item_.Clear();
 
}

////////////////////////DB_StockTickerContainer///////////////////////////////////////////

// container for database_ of stocks

class DBStockTickerContainer {
 public:
 DBStockTickerContainer(void) {
   Init();
 }
 
  ~DBStockTickerContainer(void) {
    Clear();
  }

  size_t size(void) const { 
    return database_.size(); 
  }
  void Clear(void) {
    database_.clear();
  }
  
  bool UpdateEntry(StockProductAdvanced *stock, bool *busy);
  
  void Init(void);

 private:
 
  set<DBStockTickerItem> database_;
  mutex	mutex_;
  StockTickerConfiguration &config_ = Singleton<StockTickerConfiguration>::get_instance();
  TraceLogger &trace_log = Singleton<TraceLogger>::get_instance();
};


///////////////   DB_StockTickerContainer /////////////////////////////////

void DBStockTickerContainer::Init(void) {
  Clear();
  
  string str;
  DBStockTickerItem DB_temp;
  for (int i = 0; i < kMaxDatabaseSymbols; i++) {
    str = 'a' + static_cast<char>(i);
    DB_temp.set_symbol(str);
   
	   database_.insert(move(DB_temp));
  }

}


bool DBStockTickerContainer::UpdateEntry(StockProductAdvanced* stock, bool* busy) {
  if (busy == nullptr)
    return false;

  if (stock == nullptr)
    return false;

  string symbol = stock->get_symbol();
  if (symbol.empty())
	  return false;

  if (config_.get_enable_cout_disable_unit_test() == true) 
    std::cout << "DBStockTickerContainer::UpdateEntry Symbol: " << symbol << endl;     
 
  if (mutex_.try_lock()) {

    auto itr = database_.find(DBStockTickerItem(*stock));
    if (itr != database_.end())		
    {
      DBStockTickerItem temp = *itr;
      temp.set_stock(*stock);
    }
   
    *busy = false;
    mutex_.unlock();
	if (itr == database_.end())
		return false;
  } // if (mutex_.try_lock())
  else {
    *busy = true;
    if (config_.get_enable_cout_disable_unit_test() == true) 
     cout << "DBStockTickerContainer::UpdateEntry mutex busy "  << endl; 
	return false;
  }
  return true;
}

}
#endif