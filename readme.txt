Multithreaded C++ stock pricing engine demonstration.
Name of Exe: StockTickerSB
Version: 0.4
software patterns used:Producer/consumer, templated singleton, subscriber/publisher
C++ version: C++ 11/14. 
Compiler: VS 2015. 
Notes: Tested in Windows 10. However, no Windows specific code was included so it is intended to ran with any C++ compiler and on all OS versions.

Configuration class that holds the info for new configuration file StockTickerSB.config

<configuration>
  <appSettings>
    <add key="NumberOfSubscribers" value="2" />
    <add key="EnableConsoleOutputDisableUnitTest" value="0" />
    <add key="OutputFolderFullPath" value="C:\CplusPlus\StockTickerSB\log" />
    <add key="SubscribersLogFileBaseName" value="Shosh" />
    <add key="TraceLogFileName" value="StockTickerSBTrace" />
    <add key="StockProductType" value="Advanced" />
  </appSettings>
</configuration>

EnableCoutDisableUnitTest- true: to print out to console stock prices and not have ability to do unit-test. 
                          false: to not print out to console stock prices. Instead prompt user to type 'u' for doing unit test. 
OutputFolderFullPath-  folder to save all output files: subscribers logs, trace log

NumberOfSubscribers - less or equal to 8

SubscribersLogFileBaseName -   base name is used to make up "company name" for a subscriber. e.g: subscriber_3_shosh_3.txt.

TraceLogFileName - base name of trace strings and error strings log file. Given extension .is CSV. 
  
Normal operation:
Upon startup prices are being randomly generated to 8 files: subscriber_0_shosh_0.txt to subscriber_7_shosh_7.txt
  
How to do unit tests:
Wait a little to let the 8 files get filled in.
Type u at console. 
It would read in the last generated 8 files and use them as input instead of random stock generation.
It would generate again these 8 files and upon finishing processing them compare the original content that is now in memory.
if matched the test succeeded.

How to add boost:
additional include directories: \\tsclient\E\boost_1_65_0
To use FileSystem from Boost need to compile boost libraries:
Access the console from the same environment you want to compile (same VM so same compiler version matches)
Got to boost root folder and run:
>bootstrap
>.\b2
It could take hours...
Next: add to additional libraries to link: boost_root\stage\lib

preprocessor debug: WIN32;_DEBUG;_CONSOLE; Not shure it is needed
Boost links:
www.boost.org on sourceforge.
//  chapter 35:	https://theboostcpplibraries.com/boost.filesystem-files-and-directories
www.boost.org/doc/libs/1_53_0/libs/filesystem/doc/reference.html



internal structure:
taken from StockTicker.vcxproj
<ClInclude Include="configuration.h" />
    <ClInclude Include="constants.h" />
    <ClInclude Include="consumer.h" />
    <ClInclude Include="db_stockticker.h" />
    <ClInclude Include="logger.h" />
    <ClInclude Include="producer.h" />
    <ClInclude Include="publisher.h" />
    <ClInclude Include="singleton.h" />
    <ClInclude Include="stockproductadvanced.h" />
    <ClInclude Include="stockproductbasic.h" />
    <ClInclude Include="stockticker.h" />
    <ClInclude Include="stocktickersubscriber.h" />
    <ClInclude Include="synchronizedqueue.h" />
    <ClInclude Include="tracelogger.h" />
    <ClInclude Include="utility.h" />
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="mainstockticker.cc" />
    <ClCompile Include="stockproductadvanced.cc" />
    <ClCompile Include="stockproductbasic.cc" />
    <ClCompile Include="utility.cc" />
  </ItemGroup>


  g++ -W4 -g -o hello hello.cpp

-g 
Produce debugging information in the operating system's native format (stabs, COFF, XCOFF, or DWARF). GDB can work with this debugging information. 

-Wall 
All of the above `-W' options combined. This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros. 

-c 
Compile or assemble the source files, but do not link. The linking stage simply is not done. The ultimate output is in the form of an object file for each source file. 
By default, the object file name for a source file is made by replacing the suffix `.c', `.i', `.s', etc., with `.o'. 

-o file 
Place output in file file. This applies regardless to whatever sort of output is being produced, whether it be an executable file, an object file, an assembler file or preprocessed C code. 
Since only one output file can be specified, it does not make sense to use `-o' when compiling more than one input file, unless you are producing an executable file as output. 
If `-o' is not specified, the default is to put an executable file in `a.out', the object file for `source.suffix' in `source.o', its assembler file in `source.s', and all preprocessed C source on standard output.

stockticker : mainstockticker.o stockproductadvanced.o stockproductbasic.o utility.o  
        $(CXX) -o stockticker mainstockticker.o stockproductadvanced.o stockproductbasic.o utility.o
                  
mainstockticker.o: mainstockticker.cc Configuration.h Singleton.h StockTickerSubscriber.h StockTicker.h TraceLogger.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c mainstockticker.cc

stockproductadvanced.o: stockproductadvanced.cc StockProductAdvanced.h constants.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c stockproductadvanced.cc

stockproductbasic.o: stockproductbasic.cc StockProductBasic.h constants.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c stockproductbasic.cc 

utility.o: utility.cc utility.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c utility.cc

readme.md:
Multithreaded C++ stock pricing engine
Name of Exe: StockTickerSB Version: 0.4 Author: Shoshana Biro
Reviewer: Orly Caduri software patterns used:Producer/consumer, templated singleton, subscriber/publisher C++ version: C++ 11/14. Compiler: VS 2015. Notes: Tested in Windows 10. However, no Windows specific code was included so it is intended to ran with any C++ compiler and on all OS versions.
Configuration class that holds the info for new configuration file StockTickerSB.config
EnableCoutDisableUnitTest- true: to print out to console stock prices and not have ability to do unit-test. false: to not print out to console stock prices. Instead prompt user to type 'u' for doing unit test. OutputFolderFullPath- folder to save all output files: subscribers logs, trace log
NumberOfSubscribers - less or equal to 8
SubscribersLogFileBaseName - base name is used to make up "company name" for a subscriber. e.g: subscriber_3_shosh_3.txt.
TraceLogFileName - base name of trace strings and error strings log file. Given extension .is CSV.
Normal operation: Upon startup prices are being randomly generated to 8 files: subscriber_0_shosh_0.txt to subscriber_7_shosh_7.txt
How to do unit tests: Wait a little to let the 8 files get filled in. Type u at console. It would read in the last generated 8 files and use them as input instead of random stock generation. It would generate again these 8 files and upon finishing processing them compare the original content that is now in memory. if matched the test succeeded.