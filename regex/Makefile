CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET = regex 
SRCS = *.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

debug:
	$(CXX) $(CXXFLAGS) -g -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET) && rm -f $(ANALYZER_TARGET)

lint:
	clang-format -i *.cpp
