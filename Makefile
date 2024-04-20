# Makefile at ./EDII/TP1
# ======================


# Directories

DIR_EXE		:= Executables/
DIR_SCRIPTS	:= Scripts/
DIR_SOURCE	:= Source/


# Source files

SOURCE_C	:= $(wildcard $(DIR_SOURCE)*c)
HEADERS_C	:= $(wildcard $(DIR_SOURCE)*h)


# Target executable

TARGET_EXE	:= $(DIR_EXE)searching_w10-x64.exe


build_dependencies		:= $(TARGET_EXE)
COMPILING_DIRECTIVES	:= -Wall -O3


build: $(build_dependencies)
	@echo Target-executable avaiable. $(OS), $(PROCESSOR_ARCHITECTURE)


$(TARGET_EXE): $(SOURCE_C)
	gcc $^ -o $@ $(COMPILING_DIRECTIVES)


.PHONY: build

