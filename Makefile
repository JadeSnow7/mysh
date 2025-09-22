# Makefile for MyShell






CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc -Isrc/core -Isrc/platform
DEBUG_FLAGS = -std=c++17 -Wall -Wextra -g -DDEBUG -Isrc -Isrc/core -Isrc/platform

# Check for readline
READLINE_AVAILABLE := $(shell pkg-config --exists readline 2>/dev/null && echo 1 || echo 0)
ifeq ($(READLINE_AVAILABLE),1)
	CXXFLAGS += -DHAVE_READLINE $(shell pkg-config --cflags readline)
	LIBS += $(shell pkg-config --libs readline)
else
	# Try direct linking if pkg-config is not available
	ifneq ($(wildcard /usr/include/readline/readline.h),)
		CXXFLAGS += -DHAVE_READLINE
		LIBS += -lreadline
	endif
endif
SRCDIR = src
COREDIR = $(SRCDIR)/core
PLATFORMDIR = $(SRCDIR)/platform
BUILDDIR = build
SOURCES = $(SRCDIR)/main.cpp \
          $(COREDIR)/shell.cpp \
          $(COREDIR)/parser.cpp \
          $(COREDIR)/executor.cpp \
          $(COREDIR)/builtin.cpp \
          $(COREDIR)/history.cpp \
          $(COREDIR)/completion.cpp \
          $(COREDIR)/syntax_highlighter.cpp \
          $(COREDIR)/input_handler.cpp \
          $(PLATFORMDIR)/platform.cpp
OBJECTS = $(SOURCES:%.cpp=$(BUILDDIR)/%.o)
TARGET = mysh

.PHONY: all clean debug test install help

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LIBS)

$(BUILDDIR)/%.o: %.cpp | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR)/$(SRCDIR)
	mkdir -p $(BUILDDIR)/$(COREDIR)
	mkdir -p $(BUILDDIR)/$(PLATFORMDIR)

debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: $(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

test: $(TARGET)
	@echo "Running basic functionality test..."
	@if [ -f tests/integration/test_shell.sh ]; then \
		bash tests/integration/test_shell.sh; \
	else \
		echo "Test script not found"; \
	fi

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
$(BUILDDIR)/$(SRCDIR)/main.o: $(COREDIR)/shell.h
$(BUILDDIR)/$(COREDIR)/shell.o: $(COREDIR)/shell.h $(COREDIR)/parser.h $(COREDIR)/executor.h $(COREDIR)/builtin.h $(COREDIR)/history.h
$(BUILDDIR)/$(COREDIR)/parser.o: $(COREDIR)/parser.h
$(BUILDDIR)/$(COREDIR)/executor.o: $(COREDIR)/executor.h $(COREDIR)/parser.h $(COREDIR)/shell.h
$(BUILDDIR)/$(COREDIR)/builtin.o: $(COREDIR)/builtin.h $(COREDIR)/parser.h $(COREDIR)/shell.h $(COREDIR)/history.h
$(BUILDDIR)/$(COREDIR)/history.o: $(COREDIR)/history.h
$(BUILDDIR)/$(PLATFORMDIR)/platform.o: $(PLATFORMDIR)/platform.h