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
#ifndef STOCKTICKERSB_CONSTANTS_H_
#define STOCKTICKERSB_CONSTANTS_H_

namespace stocktickersb {

const int kMaxSubscribers = 8; // 2 worked 8; // 27/3 each subscriber initially 3 different characters
const int kTraceId = kMaxSubscribers;
const int kMaxLogs = kMaxSubscribers + 1;
const int kMaxDatabaseSymbols = 26;  // a, b, c, d, in future allow for unlimited amount of symbols
                                // add more symbols by reading from a XML or JSON files or a REST service etc.

enum class StockSource {
	undefined, random_input, file_input
};

enum class StockDestination { // not used yet
	undefined, file_storage, memory_storage, display_only
};

} // namespace StockTickerSB 
#endif
