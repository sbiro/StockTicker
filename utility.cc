/*
Copyright (C) 2017 Shoshana Biro

This file is part of StockTickerSB, a free-software/open-source library
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
#include "utility.h"
//#include <cassert>


namespace stocktickersb {

namespace utility { // global functions

using namespace std;

long long Factorial(long long n) {
  if (n <= 0)
    return 0;
  if (n == 1)
    return 1;
   else
     return (n * Factorial(n - 1));
} // factorial
} // namespace utility
} // namespace StockTickerSB {