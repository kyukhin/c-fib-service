ifndef RESTBED_PATH
RESTBED_PATH	= ./restbed/release
endif

ifndef APP
APP		= fib
endif

ifndef TEST_CACHE_SIZE
TEST_CACHE_SIZE	= 100
endif

ifndef TEST_PORT
TEST_PORT	= 1983
endif

ifndef TEST_MAX_BENCH_FIB
TEST_MAX_BENCH_FIB=2000
endif

SOURCE-DIR	= ./src
OBJS-DIR	= ./obj
TESTS-DIR	= ./test

SRCS 		= $(wildcard $(SOURCE-DIR)/*.cpp)
OBJS 		= $(SRCS:$(SOURCE-DIR)/%.cpp=$(OBJS-DIR)/%.o)

BOOST_LIBS	= -lboost_system -lboost_thread -lboost_program_options
GMP_LIBS	= -lgmp
RESTBED_LIBS	= -lrestbed

CXX_FLAGS	= -std=c++11 -Ofast
CXX		= g++
PYTHON		= python

DEPS		= $(SOURCE-DIR)/logger.h

LIBS		= $(BOOST_LIBS) $(GMP_LIBS) $(RESTBED_LIBS)
INCLUDES	= -I$(RESTBED_PATH)/include -I$(SOURCE-DIR)


$(APP): $(OBJS)
	$(CXX) $(CXX_FLAGS) $^ -o $@ -L$(RESTBED_PATH)/library $(LIBS)

run: $(APP)
	./$(APP) --port $(TEST_PORT) --cache-size=$(TEST_CACHE_SIZE) --verbose=0 &

check: run
	$(PYTHON) test/fib-service.py ; pkill $(APP)

bench: run
	$(TESTS-DIR)/benchmark.sh $(TEST_PORT) $(TEST_MAX_BENCH_FIB)  ; pkill $(APP)


clean: cleano
	-$(RM) -rf $(OBJS-DIR) $(APP)

cleano:
	-$(RM) -f $(OBJS)

$(OBJS-DIR)/%.o: $(SOURCE-DIR)/%.cpp $(DEPS) | $(OBJS-DIR)
	$(CXX) $(CXX_FLAGS) -o $@ -c $< $(INCLUDES)

$(OBJS-DIR):
	[ -d $@ ] || mkdir -p $@
