# Makefile at <external-search/>
# ==============================
# Last update: 28/5/24.


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
__TEST_EXE_FILENAMES	:= $(subst .c,.exe,$(__TEST_SOURCE_FILENAMES))
test_exe_files			:= $(foreach testsource,$(__TEST_EXE_FILENAMES),$(dir_tests)$(testsource))


# Building		make build <FLAGS>
# --------------------------------

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


EXE_COMPILE_DIRECTIVES		:= -Wall -Wextra -O2 -I"$(dir_hdr)" -L"$(dir_lib)" -l:"$(target_libname)"
OBJ_COMPILE_DIRECTIVES		:= -Wall -Wextra -O2 -I"$(dir_hdr)"

__transparent_directive		:= -D TRANSPARENT_COUNTER=true
OPTIONAL_COMPILE_DIRECTIVES	:= 


DEBUG_MODE 	?= 0

ifeq ($(DEBUG_MODE),1)
	OPTIONAL_COMPILE_DIRECTIVES += -D IMPL_LOGGING=true
endif


build:
build: $(target_executables)
	@echo tudo legal.


build-transparent: OPTIONAL_COMPILE_DIRECTIVES += $(__transparent_directive)
build-transparent: $(target_executables)
	@echo tudo transparente.


# Compiling the drivers.
$(dir_exe)%.exe: $(dir_drivers)%.c $(target_lib)
	@$(MAKEDIR)
	gcc -o $@ $< $(OPTIONAL_COMPILE_DIRECTIVES) $(EXE_COMPILE_DIRECTIVES)
	@echo ES-LIB: Compiled $<.

# Joining in the library.
$(target_lib): $(object_files)
	@$(MAKEDIR)
	@ar cr $@ $^
	@echo ES-LIB: Libraryd.

# Assembling each object file.
$(dir_obj)%.o: $(dir_src)%.c $(header_files)
	@$(MAKEDIR)
	gcc -c $< -o $@ $(OPTIONAL_COMPILE_DIRECTIVES) $(OBJ_COMPILE_DIRECTIVES)
	@echo ES-LIB: Assembled $@.



# Cleaning		make clean
# ------------------------

# In case of windows, "rmdir /s /q" deletes
# all subdirectories and files of it (and finally the original directory)
# without asking for confirmation.

ifeq ($(OS),Windows_NT)
    CMD_remove_file = rmdir /s /q
else
    CMD_remove_file = rm -r
endif

# The path up to where the makefile is on the system.
# It is for guaranteeing the deletion will occur correctly.
ROOT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

# TODO: Delete temporary stuff...
clean:
	$(CMD_remove_file) "$(ROOT_DIR)$(dir_obj)"


# (* Temporary)
# Running 		make run
# ----------------------

run:
	./bin/exe/pesquisa.exe 1 1000 2 60


# Testing		make test
# -----------------------

TEST_EXE_COMPILE_DIRECTIVES	:= -Wall -Wextra -O2 -I"$(dir_hdr)" -L"$(dir_lib)" -l:"$(target_libname)"

# * After resolving each test successfully, the following block will be get into.
test: test_1
	@echo ES: All tests passed.

__test_build: $(test_exe_files) $(target_lib)
	@echo ES: Tests built.

$(dir_tests)%.exe: $(dir_tests_src)%.c
	@$(MAKEDIR)
	gcc -o $@ $< $(OPTIONAL_COMPILE_DIRECTIVES) $(TEST_EXE_COMPILE_DIRECTIVES)


# Running the tests.
test_1: __test_build
	@echo TEST 1


.PHONY: build clean test
.PHONY: __test_build test_1
