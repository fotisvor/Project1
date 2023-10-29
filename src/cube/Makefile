SRCDIR := ../common
HEADERDIR := ../../include
OBJDIR := ../../obj

CC := g++
CFLAGS := -std=c++11 -Ofast -g3 -Wall -Wextra -I$(HEADERDIR)

BINARY := cube

SOURCES := $(shell find $(SRCDIR) -name '*.cpp')
SOURCES += cubemain.cpp

OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

$(BINARY): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(BINARY)

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -rf output $(BINARY) $(OBJDIR)/*
