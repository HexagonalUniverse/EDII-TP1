# Makefile at EDII-TP1/
# ======================


# Directories

DIR_EXE		:= Executables/
DIR_SCRIPTS	:= Scripts/
DIR_SOURCE	:= Source/


# Source files

SOURCE_C	:= $(wildcard $(DIR_SOURCE)*c)
HEADERS_C	:= $(wildcard $(DIR_SOURCE)*h)


# Target executables

SYSTEM_ATTR				:= w10-$(PROCESSOR_ARCHITECTURE)
# SYSTEM_ATTR			:= linux-x64

SEARCHING_EXE			:= $(DIR_EXE)searching_$(SYSTEM_ATTR).exe
BUILDEBST_EXE			:= $(DIR_EXE)build-ebst_$(SYSTEM_ATTR).exe

ALL_EXECUTABLES			:= $(SEARCHING_EXE) $(BUILDEBST_EXE)

# * Temporary solution...
DRIVER_SOURCES			:= $(DIR_SOURCE)search.c $(DIR_SOURCE)build-ebst.c
LIBRARY_SOURCE			:= $(filter-out $(DRIVER_SOURCES),$(SOURCE_C))


build_dependencies		:= $(ALL_EXECUTABLES)
COMPILING_DIRECTIVES	:= -Wall -O3


build: $(build_dependencies)
	@echo Target-executable avaiable. $(OS), $(DRIVER_SOURCES), $(LIBRARY_SOURCE)


$(SEARCHING_EXE): $(SOURCE_C) $(HEADERS_C)
	gcc $(LIBRARY_SOURCE) $(DIR_SOURCE)search.c -o $@ $(COMPILING_DIRECTIVES)


$(BUILDEBST_EXE): $(SOURCE_C) $(HEADERS_C)
	gcc $(LIBRARY_SOURCE) $(DIR_SOURCE)build-ebst.c -o $@ $(COMPILING_DIRECTIVES)


.PHONY: build

