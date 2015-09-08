#### BUILD modalities
# use 
#   $ make BUILD=release 
# to compile different binaries
BUILD = release
cxxflags.debug = \
 -Wall -Wextra -std=c++11
linkflags.debug = \
 -lm
cxxflags.trace = \
 -Wall -Wextra -std=c++11 -DTRACE_ON
linkflags.trace = \
 -lm
cxxflags.release = \
 -Wall -Wextra -std=c++11 -DNDEBUG -O3
linkflags.release = \
 -lm
cxxflags.gprof = \
 -Wall -Wextra -std=c++11 -DNDEBUG -O3 -g -pg
linkflags.gprof = \
 -lm -g -pg
cxxflags.stats = \
 -Wall -Wextra -std=c++11 -DNDEBUG -DSTATS_ON -O3
linkflags.stats = \
 -lm

# for g++ <= 4.6
cxxflags.debug0x = \
 -Wall -Wextra -std=c++0x
linkflags.debug0x = \
 -lm
cxxflags.trace0x = \
 -Wall -Wextra -std=c++0x -DTRACE_ON
linkflags.trace = \
 -lm
cxxflags.release0x = \
 -Wall -Wextra -std=c++0x -DNDEBUG -O3
linkflags.release0x = \
 -lm
cxxflags.gprof0x = \
 -Wall -Wextra -std=c++0x -DNDEBUG -O3 -g -pg 
linkflags.gprof0x = \
 -lm -g -pg
cxxflags.stats0x = \
 -Wall -Wextra -std=c++0x -DNDEBUG -DSTATS_ON -O3
linkflags.stats0x = \
 -lm
####

SOURCE_DIR = src
BUILD_DIR = build/$(BUILD)

BINARY = $(BUILD_DIR)/dwasp
GCC = g++
CXX = $(GCC)
CXXFLAGS = $(cxxflags.$(BUILD))
LINK = $(GCC)
LINKFLAGS = $(linkflags.$(BUILD))

SRCS = $(wildcard $(SOURCE_DIR)/*.cpp) $(wildcard $(SOURCE_DIR)/**/*.cpp)

OBJS = $(patsubst $(SOURCE_DIR)%.cpp,$(BUILD_DIR)%.o, $(SRCS))
DEPS = $(patsubst $(SOURCE_DIR)%.cpp,$(BUILD_DIR)%.d, $(SRCS))

## OS specific commands
ifeq ($(OS),Windows_NT)
	PATHSEP2=\\
    MKDIR = mkdir $(subst /,\,$(1)) > nul 2>&1 || (exit 0)
	RMF = del /Q $(subst /,\,$(1))
	RMDIR = rmdir /S /Q $(subst /,\,$(1))
else
	PATHSEP2=/
    MKDIR = mkdir -p $(1)
	RMF = rm -f $(1)
	RMDIR = rm -fr $(1)
endif

PATHSEP=$(strip $(PATHSEP2))
##

all: $(BINARY)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.d: $(SOURCE_DIR)/%.cpp
	$(call MKDIR,$(dir $@))
	$(CXX) $(CXXFLAGS) -MM -MT '$(@:.d=.o)' $< -MF $@
	
$(BINARY): $(OBJS) $(DEPS)
	$(LINK) $(LINKFLAGS) $(LIBS) $(OBJS) -o $(BINARY)

static: $(OBJS) $(DEPS)
	$(LINK) $(LINKFLAGS) $(LIBS) $(OBJS) -static -o $(BINARY)

run: $(BINARY)
	./$(BINARY)

########## Tests

TESTS_DIR = tests

TESTS_TESTER = $(TESTS_DIR)/pyregtest.py

TESTS_COMMAND_AllAnswerSets = $(BINARY) -n 0 --silent
TESTS_COMMAND_gringo = gringo | $(BINARY) -n 0 --silent
TESTS_COMMAND_SatModel = $(BINARY)
TESTS_COMMAND_WeakConstraints = $(BINARY) -n 0 --silent

TESTS_CHECKER_AllAnswerSets = $(TESTS_DIR)/allAnswerSets.checker.py
TESTS_CHECKER_SatModels = $(TESTS_DIR)/satModels.checker.py
TESTS_CHECKER_WeakConstraints = $(TESTS_DIR)/weakConstraints.checker.py

TESTS_REPORT_text = $(TESTS_DIR)/text.report.py

TESTS_DIR_wasp1_AllAnswerSets = $(TESTS_DIR)/wasp1/AllAnswerSets
TESTS_SRC_wasp1_AllAnswerSets = $(sort $(wildcard $(TESTS_DIR_wasp1_AllAnswerSets)/**/*.test.py))
TESTS_OUT_wasp1_AllAnswerSets = $(patsubst %.test.py,%.test.py.text, $(TESTS_SRC_wasp1_AllAnswerSets))

TESTS_DIR_asp_AllAnswerSetsTight = $(TESTS_DIR)/asp/AllAnswerSets/tight
TESTS_SRC_asp_AllAnswerSetsTight = $(sort $(wildcard $(TESTS_DIR_asp_AllAnswerSetsTight)/**/*.test.py))
TESTS_OUT_asp_AllAnswerSetsTight = $(patsubst %.test.py,%.test.py.text, $(TESTS_SRC_asp_AllAnswerSetsTight))

TESTS_DIR_asp_AllAnswerSetsNonTight = $(TESTS_DIR)/asp/AllAnswerSets/nontight
TESTS_SRC_asp_AllAnswerSetsNonTight = $(sort $(wildcard $(TESTS_DIR_asp_AllAnswerSetsNonTight)/**/*.test.py))
TESTS_OUT_asp_AllAnswerSetsNonTight = $(patsubst %.test.py,%.test.py.text, $(TESTS_SRC_asp_AllAnswerSetsNonTight))

TESTS_DIR_asp_AllAnswerSetsAggregates = $(TESTS_DIR)/asp/AllAnswerSets/aggregates
TESTS_SRC_asp_AllAnswerSetsAggregates = $(sort $(wildcard $(TESTS_DIR_asp_AllAnswerSetsAggregates)/**/*.test.py))
TESTS_OUT_asp_AllAnswerSetsAggregates = $(patsubst %.test.py,%.test.py.text, $(TESTS_SRC_asp_AllAnswerSetsAggregates))

TESTS_DIR_asp_AllAnswerSetsIntensive = $(TESTS_DIR)/asp/AllAnswerSetsIntensive
TESTS_SRC_asp_AllAnswerSetsIntensive = $(sort $(wildcard $(TESTS_DIR_asp_AllAnswerSetsIntensive)/**/*.test.py))
TESTS_OUT_asp_AllAnswerSetsIntensive = $(patsubst %.test.py,%.test.py.text, $(TESTS_SRC_asp_AllAnswerSetsIntensive))

TESTS_DIR_asp_WeakConstraints = $(TESTS_DIR)/asp/weakConstraints
TESTS_SRC_asp_WeakConstraints = $(sort $(wildcard $(TESTS_DIR_asp_WeakConstraints)/**/*.test.py))
TESTS_OUT_asp_WeakConstraints = $(patsubst %.test.py,%.test.py.text, $(TESTS_SRC_asp_WeakConstraints))

TESTS_DIR_asp_gringo = $(TESTS_DIR)/asp/gringo
TESTS_SRC_asp_gringo = $(sort $(wildcard $(TESTS_DIR_asp_gringo)/**/*.test.py))
TESTS_OUT_asp_gringo = $(patsubst %.test.py,%.test.py.text, $(TESTS_SRC_asp_gringo))

TESTS_DIR_sat_Models = $(TESTS_DIR)/sat/Models
TESTS_SRC_sat_Models = $(sort $(wildcard $(TESTS_DIR_sat_Models)/**/*.test.py))
TESTS_OUT_sat_Models = $(patsubst %.test.py,%.test.py.text, $(TESTS_SRC_sat_Models))

TESTS_DIR_sat_Intensive = $(TESTS_DIR)/sat/Intensive
TESTS_SRC_sat_Intensive = $(sort $(wildcard $(TESTS_DIR_sat_Intensive)/**/*.test.py))
TESTS_OUT_sat_Intensive = $(patsubst %.test.py,%.test.py.text, $(TESTS_SRC_sat_Intensive))

TESTS_DIR_sat_Intensive2 = $(TESTS_DIR)/sat/Intensive2
TESTS_SRC_sat_Intensive2 = $(sort $(wildcard $(TESTS_DIR_sat_Intensive2)/**/*.test.py))
TESTS_OUT_sat_Intensive2 = $(patsubst %.test.py,%.test.py.text, $(TESTS_SRC_sat_Intensive2))

TESTS_DIR_sat_Intensive3 = $(TESTS_DIR)/sat/Intensive3
TESTS_SRC_sat_Intensive3 = $(sort $(wildcard $(TESTS_DIR_sat_Intensive3)/**/*.test.py))
TESTS_OUT_sat_Intensive3 = $(patsubst %.test.py,%.test.py.text, $(TESTS_SRC_sat_Intensive3))

#tests: tests/wasp1 tests/sat tests/asp

test: tests/sat/Models tests/asp/gringo tests/asp/AllAnswerSets/tight tests/asp/AllAnswerSets/nontight tests/asp/AllAnswerSets/aggregates tests/asp/weakConstraints

tests/wasp1: tests/wasp1/AllAnswerSets

tests/wasp1/AllAnswerSets: $(TESTS_OUT_wasp1_AllAnswerSets)

$(TESTS_OUT_wasp1_AllAnswerSets):
	@$(TESTS_TESTER) "$(TESTS_COMMAND_gringo)" $(patsubst %.test.py.text,%.test.py , $@) $(TESTS_CHECKER_AllAnswerSets) $(TESTS_REPORT_text)

tests/sat: tests/sat/Models tests/sat/Intensive tests/sat/Intensive2 tests/sat/Intensive3

tests/sat/Models: $(TESTS_OUT_sat_Models)

$(TESTS_OUT_sat_Models):
	@$(TESTS_TESTER) "$(TESTS_COMMAND_SatModel)" $(patsubst %.test.py.text,%.test.py , $@) $(TESTS_CHECKER_SatModels) $(TESTS_REPORT_text)

tests/sat/Intensive: $(TESTS_OUT_sat_Intensive)

$(TESTS_OUT_sat_Intensive):
	@$(TESTS_TESTER) "$(TESTS_COMMAND_SatModel)" $(patsubst %.test.py.text,%.test.py , $@) $(TESTS_CHECKER_SatModels) $(TESTS_REPORT_text)

tests/sat/Intensive2: $(TESTS_OUT_sat_Intensive2)

$(TESTS_OUT_sat_Intensive2):
	@$(TESTS_TESTER) "$(TESTS_COMMAND_SatModel)" $(patsubst %.test.py.text,%.test.py , $@) $(TESTS_CHECKER_SatModels) $(TESTS_REPORT_text)
	
tests/sat/Intensive3: $(TESTS_OUT_sat_Intensive3)

$(TESTS_OUT_sat_Intensive3):
	@$(TESTS_TESTER) "$(TESTS_COMMAND_SatModel)" $(patsubst %.test.py.text,%.test.py , $@) $(TESTS_CHECKER_SatModels) $(TESTS_REPORT_text)

tests/asp: tests/asp/gringo tests/asp/AllAnswerSets/tight tests/asp/AllAnswerSets/nontight tests/asp/AllAnswerSets/aggregates tests/asp/weakConstraints

tests/asp/gringo: $(TESTS_OUT_asp_gringo)
tests/asp/AllAnswerSets/tight: $(TESTS_OUT_asp_AllAnswerSetsTight)
tests/asp/AllAnswerSets/nontight: $(TESTS_OUT_asp_AllAnswerSetsNonTight)
tests/asp/AllAnswerSets/aggregates: $(TESTS_OUT_asp_AllAnswerSetsAggregates)
tests/asp/weakConstraints: $(TESTS_OUT_asp_WeakConstraints)

tests/asp/AllAnswerSetsIntensive: $(TESTS_OUT_asp_AllAnswerSetsIntensive)

$(TESTS_OUT_asp_AllAnswerSetsTight):
	@$(TESTS_TESTER) "$(TESTS_COMMAND_AllAnswerSets)" $(patsubst %.test.py.text,%.test.py , $@) $(TESTS_CHECKER_AllAnswerSets) $(TESTS_REPORT_text)

$(TESTS_OUT_asp_AllAnswerSetsNonTight):
	@$(TESTS_TESTER) "$(TESTS_COMMAND_AllAnswerSets)" $(patsubst %.test.py.text,%.test.py , $@) $(TESTS_CHECKER_AllAnswerSets) $(TESTS_REPORT_text)

$(TESTS_OUT_asp_AllAnswerSetsAggregates):
	@$(TESTS_TESTER) "$(TESTS_COMMAND_AllAnswerSets)" $(patsubst %.test.py.text,%.test.py , $@) $(TESTS_CHECKER_AllAnswerSets) $(TESTS_REPORT_text)

$(TESTS_OUT_asp_AllAnswerSetsIntensive):
	@$(TESTS_TESTER) "$(TESTS_COMMAND_AllAnswerSets)" $(patsubst %.test.py.text,%.test.py , $@) $(TESTS_CHECKER_AllAnswerSets) $(TESTS_REPORT_text)

$(TESTS_OUT_asp_WeakConstraints):
	@$(TESTS_TESTER) "$(TESTS_COMMAND_WeakConstraints)" $(patsubst %.test.py.text,%.test.py , $@) $(TESTS_CHECKER_WeakConstraints) $(TESTS_REPORT_text)

$(TESTS_OUT_asp_gringo):
	@$(TESTS_TESTER) "$(TESTS_COMMAND_gringo)" $(patsubst %.test.py.text,%.test.py , $@) $(TESTS_CHECKER_AllAnswerSets) $(TESTS_REPORT_text)

########## Clean

clean-dep:
	$(call RMF,$(DEPS))
clean: clean-dep
	$(call RMF,$(OBJS))

distclean:
	$(call RMDIR,$(BUILD_DIR))

-include $(DEPS)
