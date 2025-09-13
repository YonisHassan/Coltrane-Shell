CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lreadline
SRC_DIR = src
BUILD_DIR = build
TARGET = coltrane
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: CXXFLAGS += -g -DDEBUG
debug: clean $(TARGET)

help:
	@echo "Available targets:"
	@echo "  all      - Build the shell (default)"
	@echo "  clean    - Remove build files"
	@echo "  run      - Build and run the shell"
	@echo "  debug    - Build with debug symbols"
	@echo "  install  - Install to /usr/local/bin"
	@echo "  help     - Show this help"

.PHONY: all clean install uninstall run debug help
