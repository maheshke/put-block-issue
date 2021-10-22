SOURCE_FILES=snapshot.cpp
OBJECT_FILES:=$(SOURCE_FILES:.cpp=.o)

PROJECT_NAME=put-block-issue

DEPENDENT_DLS=-laws-cpp-sdk-ebs -laws-cpp-sdk-core

EXECUTABLE=$(PROJECT_NAME)

CXXFLAGS := -W -Wall -Werror
CXXFLAGS += -std=c++11

all: $(EXECUTABLE)

debug: CXXFLAGS += -DDEBUG -g
debug: all

$(EXECUTABLE): $(OBJECT_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(DEPENDENT_DLS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(EXECUTABLE) $(OBJECT_FILES)
