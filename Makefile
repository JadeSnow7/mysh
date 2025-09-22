# Makefile for MyShell

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
DEBUG_FLAGS = -std=c++17 -Wall -Wextra -g -DDEBUG
SRCDIR = src
BUILDDIR = build
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
TARGET = mysh

.PHONY: all clean debug test install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: $(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

test: $(TARGET)
	@echo "Running basic functionality test..."
	@bash test_shell.sh

install: $(TARGET)
	@echo "Installing to /usr/local/bin/ (requires sudo)"
	sudo cp $(TARGET) /usr/local/bin/

help:
	@echo "Available targets:"
	@echo "  all      - Build the shell (default)"
	@echo "  debug    - Build with debug information"
	@echo "  clean    - Remove build files"
	@echo "  test     - Run functionality tests"
	@echo "  install  - Install to system (requires sudo)"
	@echo "  help     - Show this help message"

# 依赖关系
$(BUILDDIR)/main.o: $(SRCDIR)/shell.h
$(BUILDDIR)/shell.o: $(SRCDIR)/shell.h $(SRCDIR)/parser.h $(SRCDIR)/executor.h $(SRCDIR)/builtin.h $(SRCDIR)/history.h
$(BUILDDIR)/parser.o: $(SRCDIR)/parser.h
$(BUILDDIR)/executor.o: $(SRCDIR)/executor.h $(SRCDIR)/parser.h $(SRCDIR)/shell.h
$(BUILDDIR)/builtin.o: $(SRCDIR)/builtin.h $(SRCDIR)/parser.h $(SRCDIR)/shell.h $(SRCDIR)/history.h
$(BUILDDIR)/history.o: $(SRCDIR)/history.h