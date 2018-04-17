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
#ifndef STOCKTICKERSB_SINGLETON_H
#define STOCKTICKERSB_SINGLETON_H

#include <mutex>

namespace stocktickersb {

using namespace std;

// used for publisher
template <typename T>
class Singleton {
 public:
  static T& get_instance() {
    static once_flag flag_;
    call_once(flag_, [&]() { instance_pointer = new T;});
    return *instance_pointer;
  }

 private:
  Singleton() {}
  Singleton(const Singleton& value) = delete; // {}
  ~Singleton() {
    Remove();
  }
  void Remove() {
    delete instance_pointer;
    instance_pointer = NULL;
  }
  static T* instance_pointer;
};
} // namespace StockTickerSB {
#endif