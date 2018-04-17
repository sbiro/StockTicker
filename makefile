
stockticker : mainstockticker.o stockproductadvanced.o stockproductbasic.o utility.o  
	$(CXX) -pthread -o  stockticker mainstockticker.o stockproductadvanced.o stockproductbasic.o utility.o -L /home/shoshana/boost_1_65_1/stage/lib -lboost_system -lboost_filesystem
                  
mainstockticker.o: mainstockticker.cc configuration.h singleton.h stocktickersubscriber.h stockticker.h tracelogger.h producer.h consumer.h stockproductadvanced.h constants.h stockproductbasic.h logger.h publisher.h db_stockticker.h utility.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I /home/shoshana/boost_1_65_1 -c mainstockticker.cc

stockproductadvanced.o: stockproductadvanced.cc stockproductadvanced.h constants.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c stockproductadvanced.cc

stockproductbasic.o: stockproductbasic.cc stockproductbasic.h constants.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c stockproductbasic.cc 

utility.o: utility.cc utility.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c utility.cc
