# Makefile at <external-search/>
# ==============================
# Last update: 04/6/24. Test-system.


# Directories
# -----------

dir_drivers	:= cleber/
dir_src		:= src/
dir_hdr		:= $(dir_src)
dir_bin		:= bin/
dir_lib		:= $(dir_bin)
dir_obj		:= $(dir_bin)obj/
dir_exe		:= $(dir_bin)exe/
dir_tests	:= tests/
dir_tests_src	:= $(dir_tests)src/
dir_tests_data	:= $(dir_tests)template/
dir_tests_cache	:= $(dir_tests)cache/

# Source files
# ------------

source_files			:= $(wildcard $(dir_src)*.c) 
header_files			:= $(wildcard $(dir_hdr)*.h)

__SOURCE_FILENAMES		:= $(subst $(dir_src),,$(source_files))
__OBJECT_FILENAMES		:= $(subst .c,.o,$(__SOURCE_FILENAMES)) 
object_files			:= $(foreach object,$(__OBJECT_FILENAMES),$(dir_obj)$(object))

# Main drivers
# -----------

driver_srcs				:= $(wildcard $(dir_drivers)*.c)

__DRIVERSRC_FILENAMES	:= $(subst $(dir_drivers),,$(driver_srcs))
__DRIVEREXE_FILENAMES	:= $(subst .c,.exe,$(__DRIVERSRC_FILENAMES))
target_executables		:= $(foreach executables,$(__DRIVEREXE_FILENAMES),$(dir_exe)$(executables))

# Static library
# --------------

target_libname			:= external-search.lib
target_lib				:= $(dir_lib)$(target_libname)

# Test executables
# ----------------

test_src_files			:= $(wildcard $(dir_tests_src)*.c)
__TEST_SOURCE_FILENAMES	:= $(subst $(dir_tests_src),,$(test_src_files))
__TEST_FILENAMES		:= $(subst .c,,$(__TEST_SOURCE_FILENAMES))
__TEST_EXE_FILENAMES	:= $(subst .c,.exe,$(__TEST_SOURCE_FILENAMES))
test_exe_files			:= $(foreach testsource,$(__TEST_EXE_FILENAMES),$(dir_tests)$(testsource))


# Building		make build <FLAGS>
# --------------------------------

EXE_COMPILE_DIRECTIVES		:= -Wall -Wextra -O3 -I"$(dir_hdr)" -L"$(dir_lib)" -l:"$(target_libname)"
OBJ_COMPILE_DIRECTIVES		:= -Wall -Wextra -O3 -I"$(dir_hdr)"

__transparent_directive		:= -D TRANSPARENT_COUNTER=true
OPTIONAL_COMPILE_DIRECTIVES	:= 

# Directives
DEBUG			?= 0
TRANSPARENT		?= 0


ifeq ($(DEBUG),1)
	OPTIONAL_COMPILE_DIRECTIVES += -D IMPL_LOGGING=true
else ifeq ($(DEBUG),2)
	OPTIONAL_COMPILE_DIRECTIVES += -D IMPL_LOGGING=true -D STDERR_DEBUG_LOGGING=true
endif


# __build_msg...
ifeq ($(TRANSPARENT),1)
	OPTIONAL_COMPILE_DIRECTIVES += -D TRANSPARENT_COUNTER=true
	
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
build: $(target_executables)
	@echo tudo $(__build_msg).


# Compiling the drivers.
$(dir_exe)%.exe: $(dir_drivers)%.c $(target_lib)
	@$(MAKEDIR)
	@gcc -o $@ $< $(OPTIONAL_COMPILE_DIRECTIVES) $(EXE_COMPILE_DIRECTIVES)
	@echo ES-LIB: Compiled $<.

# Joining in the library.
$(target_lib): $(object_files)
	@$(MAKEDIR)
	@ar cr $@ $^
	@echo ES-LIB: Libraryd.

# Assembling each object file.
$(dir_obj)%.o: $(dir_src)%.c $(header_files)
	@$(MAKEDIR)
	@gcc -c $< -o $@ $(OPTIONAL_COMPILE_DIRECTIVES) $(OBJ_COMPILE_DIRECTIVES)
	@echo ES-LIB: Assembled $@.


# Re-building		make rebuild <FLAGS>
# --------------------------------------

rebuild: clean build
#@echo System rebuilt.


# Cleaning		make clean
# ------------------------

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

# By the way CMD_remove works, every cleaning part, 
# is separately resolved.
clean: __clean_obj __clean_test
	@echo All clean.


# The object files, byproduct of the library construction.
__clean_obj: $(dir_obj)
	@$(CMD_remove)

# The temporary data of the tests.
__clean_test: $(dir_tests_cache)
	@$(CMD_remove) 


# * Each dependency of its respective cleaning part need
# not be analyzed. Their purpose is to become sort of a
# "parameter" for CMD_remove, so they're hereby ignored...
$(dir_obj):
$(dir_tests_cache):



# Testing		make test
# -----------------------

TEST_EXE_COMPILE_DIRECTIVES	:= -Wall -Wextra -O2 -I"$(dir_hdr)" -L"$(dir_lib)" -l:"$(target_libname)"
test_data_files			:= $(wildcard $(dir_tests_data)*.6t)

# Boilerplate for guaranteeing the creation of the tests cache folder.
__test_cache_nothing := $(dir_tests_cache)__nothing


ifeq ($(OS),Windows_NT)
	PYTHON = python
else
	PYTHON = python3
endif


# In order, tries resolving the dependencies.
# So it will first build the tests executables, generate each response (and compare 
# them with the expected along the way), and then clean those.
# * After resolving each test successfully, the following block will be get into.
test: __build_test __run_test __clean_test
	@echo ES: All tests passed.


# Building the test executables.
__build_test: $(test_exe_files) __test_cache_build
	@echo ES: Tests built.

$(dir_tests)%.exe: $(dir_tests_src)%.c $(target_lib)
	@$(MAKEDIR)
	@gcc -o $@ $< $(OPTIONAL_COMPILE_DIRECTIVES) $(TEST_EXE_COMPILE_DIRECTIVES)

__run_test:
	@$(PYTHON) tests/o_6t.py $(test_data_files)
	

# Creating the tests cache path. 
__test_cache_build: $(__test_cache_nothing)
$(__test_cache_nothing):
	@$(MAKEDIR)


.PHONY: build rebuild clean
.PHONY: __clean_obj
.PHONY: test __run_test __build_test __clean_test
.PHONY: __test_cache_build
