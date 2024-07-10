# Makefile at <external-search/>
# ==============================
# Last update: 12/06/24. 
# (Changes) Included data-executables into building;
# old test system is pretty much now obsolete.
.PHONY: build rebuild full-clean clean test


# Directories
# -----------

dir_drivers		:= cleber/
dir_src			:= src/
dir_hdr			:= $(dir_src)/include/
dir_bin			:= bin/
dir_lib			:= $(dir_bin)
dir_obj			:= $(dir_bin)obj/
dir_exe			:= $(dir_bin)exe/
dir_cache		:= temp/cache/
dir_data		:= data/
dir_tests		:= $(dir_data)tests/
dir_tests_data	:= $(dir_tests)template/


# Files
# -----

source_files			:= $(wildcard $(dir_src)*.c) 
header_files			:= $(wildcard $(dir_hdr)*.h)
driver_sources			:= $(wildcard $(dir_drivers)*.c)

object_files			:= $(foreach object,$(subst .c,.o,$(subst $(dir_src),,$(source_files))),$(dir_obj)$(object))
target_executables		:= $(foreach executables,$(subst .c,.exe,$(subst $(dir_drivers),,$(driver_sources))),$(dir_exe)$(executables))

test_script				:= $(dir_tests)o_6t.py


# Static library
# --------------

target_libname			:= external-search.lib
target_lib				:= $(dir_lib)$(target_libname)


# Building		make build <FLAGS>
# ================================

# Toggling Directives
DEBUG			?= 0
TRANSPARENT		?= 0

# Compiling options*
OPTIONAL_COMPILE_DIRECTIVES	?= 
__OPTIONAL_COMPILE_DIRECTIVES	:=

EXE_COMPILE_DIRECTIVES		:= -Wall -Wextra -O3 -I"$(dir_hdr)" -L"$(dir_lib)" -l:"$(target_libname)"
OBJ_COMPILE_DIRECTIVES		:= -Wall -Wextra -O3 -I"$(dir_hdr)"

__transparent_directive		:= -D TRANSPARENT_COUNTER=true



ifeq ($(DEBUG),1)
	__OPTIONAL_COMPILE_DIRECTIVES += -D IMPL_LOGGING=true
else ifeq ($(DEBUG),2)
	__OPTIONAL_COMPILE_DIRECTIVES += -D IMPL_LOGGING=true -D STDERR_DEBUG_LOGGING=true
endif


# (Aesthetic) __build_msg...
ifeq ($(TRANSPARENT),1)
	__OPTIONAL_COMPILE_DIRECTIVES += $(__transparent_directive)
	
	ifdef __build_msg
		__build_msg += e transparente
	else
		__build_msg = transparente
	endif
endif
ifneq ($(DEBUG),0)
	ifdef __build_msg
		__build_msg += e debugado$(DEBUG)
	else
		__build_msg = debugado$(DEBUG)
	endif
endif
__build_msg ?= normal

__OPTIONAL_COMPILE_DIRECTIVES += $(OPTIONAL_COMPILE_DIRECTIVES) 


# Building the path...
ifeq ($(OS),Windows_NT)
define MAKEDIR =
	if not exist "$(@D)" mkdir "$(@D)"
endef
else
define MAKEDIR
	mkdir -p "$(@D)"
endef
endif

#${__build_msg:%=%.o}
# build:
build: $(target_executables) __build_data_gen
	@echo tudo $(__build_msg).


# Compiling the drivers.
$(dir_exe)%.exe: $(dir_drivers)%.c $(target_lib)
	@$(MAKEDIR)
	@gcc -o $@ $< $(__OPTIONAL_COMPILE_DIRECTIVES) $(EXE_COMPILE_DIRECTIVES)
	@echo ES-LIB: Assembling $<.

# Joining in the library.
$(target_lib): $(object_files)
	@$(MAKEDIR)
	@ar cr $@ $^
	@echo ES-LIB: Libraryd.

# Assembling each object file.
$(dir_obj)%.o: $(dir_src)%.c $(header_files)
	@$(MAKEDIR)
	@gcc -c $(__OPTIONAL_COMPILE_DIRECTIVES) $< -o $@ $(OBJ_COMPILE_DIRECTIVES)
	@echo ES-LIB: Compiled $@.


# Data-generation stuff...
__build_data_gen: ./bin/exe/data-gen.exe ./bin/exe/data-read.exe

./bin/exe/data-gen.exe: ./data/src/data-gen.c
	gcc $(EXE_COMPILE_DIRECTIVES) $< -o $@

./bin/exe/data-read.exe: ./data/src/data-read.c
	gcc $(EXE_COMPILE_DIRECTIVES) $< -o $@


# Re-building		make rebuild <FLAGS>
# --------------------------------------

rebuild: clean build
#@echo System rebuilt.


# Cleaning		make clean
#				make full-clean
# =============================

# In case of windows, "rmdir /s /q" deletes
# all subdirectories and files of it (and finally the original directory)
# without asking for confirmation. The equivallent in unix systems
# will be -r and -f, respectively.

# Deletes automatically the path specified by the 
# firstmost dependency.
ifeq ($(OS),Windows_NT)
    CMD_remove = if exist "$<" rmdir /s /q "$<"
else
    CMD_remove = rm -rf "$<"
endif

# The path up to where the makefile is on the system.
# It is for guaranteeing the deletion will occur correctly.
ROOT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))


full-clean: __clean_bin __clean_cache 
	@echo Full cleaned.

# By the way CMD_remove works, every cleaning part, 
# is separately resolved.
clean: __clean_obj
	@echo All clean.


# Byproduct of the library construction object's file.
__clean_obj: $(dir_obj)
	@$(CMD_remove)

__clean_bin: $(dir_bin)
	@$(CMD_remove)

__clean_cache: $(dir_cache)
	@$(CMD_remove)


# * Each dependency of its respective cleaning part need
# not be analyzed. Their purpose is to become sort of a
# "parameter" for CMD_remove, so they're hereby ignored...
$(dir_obj):
$(dir_cache):
$(dir_bin):


# Testing		make test
# ========================

test_data_files			:= $(wildcard $(dir_tests_data)*.6t)




ifeq ($(OS),Windows_NT)
	PYTHON = python
else
	PYTHON = python3
endif


# In order, tries resolving the dependencies.
# So it will first build the tests executables, generate each response (and compare 
# them with the expected along the way), and then clean those.
# * After resolving each test successfully, the following block will be get into.
test: __build_test __run_test
	@echo ES: All tests passed.

# Building the test executables.
__build_test: $(target_executables) __test_cache_build

$(dir_tests)%.exe: $(dir_tests_src)%.c $(target_lib)
	@$(MAKEDIR)
	@gcc -o $@ $< $(OPTIONAL_COMPILE_DIRECTIVES) $(TEST_EXE_COMPILE_DIRECTIVES)

__run_test:
	@echo running tests
	@$(PYTHON) $(test_script) $(test_data_files)
	@echo tests run


	
# Boilerplate for guaranteeing the creation of the tests cache folder.
__test_cache_nothing := $(dir_cache)__nothing__

# Creating the tests cache path. 
__test_cache_build: $(__test_cache_nothing)
$(__test_cache_nothing):
	@$(MAKEDIR)


.PHONY: __clean_obj __clean_bin __clean_cache
.PHONY: __run_test __build_test
.PHONY: __test_cache_build
