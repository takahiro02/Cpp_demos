#------------------------------------------------------------------------#
#       Makefile for unix systems               			 #
#       This makefile is copied from Massive_Graph_Analysis course       #
# /Users/furuyatakahiro/Desktop/2019_spring_semester/Massive_Graph_Analysis/project/Final_project_MGA/max_flow #
#------------------------------------------------------------------------#
CC=g++ 
#FLAGS=-g -Wall -D__USE_FIXED_PROTOTYPES__ -ansi
VER=-std=c++14
fltk_option = `fltk-config --ldflags --use-images`

# TARGET = main
# OBJS = main.o Graph.o GUI.o Simple_window.o Window.o

# all: $(TARGET)

# $(TARGET): $(OBJS)
# 	$(CC) -o $(TARGET) $(OBJS) $(VER) $(fltk_option)

# %.o: %.cpp
# 	$(CC) -c $< -o $@ $(VER)


# from https://stackoverflow.com/questions/52034997/
SOURCES := $(wildcard *.cpp)
EXCLUDE := memory_layout.cpp test_template_class.cpp
SOURCES := $(filter-out $(EXCLUDE), $(SOURCES))
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
DEPENDS := $(patsubst %.cpp,%.d,$(SOURCES))

# ADD MORE WARNINGS!
#WARNING := -Wall -Wextra

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: all clean
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html

# The first rule is the default, ie. "make",
# "make all" and "make parking" mean the same
all: main

# Linking the executable from the object files
main: $(OBJECTS)
	$(CC) $(WARNING) $(VER) $(fltk_option) $^ -o $@

-include $(DEPENDS)

# when I mistakenly write makefile as Makefile, this make instruction
# mysteriously didn't execute CC=g++ and VER=-std=c++14
%.o: %.cpp makefile
	$(CC) $(WARNING) $(VER) -MMD -MP -c $< -o $@


clean: clean_exe_obj

# delete executable and object files
clean_exe_obj:
	rm -f $(OBJECTS) $(DEPENDS) main
	#rm -f $(OBJS) main
