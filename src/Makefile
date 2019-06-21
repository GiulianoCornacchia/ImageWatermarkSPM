ifndef FF_ROOT 
FF_ROOT		= ./lib
endif

CXX		= g++ -std=c++11 -D cimg_display=0 #-DNO_DEFAULT_MAPPING 
INCLUDES	= -I $(FF_ROOT) 
CXXFLAGS  	= 

LDFLAGS 	= -pthread
OPTFLAGS	= -O3 -finline-functions

TARGETS		= IW_sequential \
			  IW_parallel \
			  IW_fastflow
		 

.PHONY: all clean cleanall
.SUFFIXES: .cpp 


%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS)

all		: $(TARGETS)
	printf "\n-SUCCESS! parameters: source dest watermark avg par_deg[not in seq]\n";
clean		: 
	rm -f $(TARGETS)
cleanall	: clean
	\rm -f *.o *~
