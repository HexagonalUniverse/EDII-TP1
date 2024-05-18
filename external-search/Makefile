# Makefile at <external-search/>
# ==============================

# Directories
# ==============================

dir_drivers	:= cleber/
dir_src		:= src/
dir_hdr		:= $(dir_src)
dir_bin		:= bin/
dir_lib		:= $(dir_bin)
dir_obj		:= $(dir_bin)obj/
dir_exe		:= $(dir_bin)exe/

# Files
# ===============================

source_files	:= $(wildcard $(dir_src)*.c) 
header_files	:= $(wildcard $(dir_hdr)*.h)

__SOURCE_FILENAMES	:= $(subst $(dir_src),,$(source_files))
__OBJECT_FILENAMES	:= $(subst .c,.o,$(__SOURCE_FILENAMES)) 
object_files		:= $(foreach object,$(__OBJECT_FILENAMES),$(dir_obj)$(object))

driver_srcs			:= $(wildcard $(dir_drivers)*.c)

__DRIVERSRC_FILENAMES	:= $(subst $(dir_drivers),,$(driver_srcs))
__DRIVEREXE_FILENAMES	:= $(subst .c,.exe,$(__DRIVERSRC_FILENAMES))
target_executables		:= $(foreach executables,$(__DRIVEREXE_FILENAMES),$(dir_exe)$(executables))

target_libname	:= external-search.lib
target_lib		:= $(dir_lib)$(target_libname)

# Building		<make build>
# ================================

build: $(target_executables)
	@echo tudo legal.


EXE_COMPILE_DIRECTIVES	:= -Wall -Wextra -O2 -I"$(dir_hdr)" -L"$(dir_lib)" -l:"$(target_libname)"
OBJ_COMPILE_DIRECTIVES	:= -Wall -Wextra -O2 -I"$(dir_hdr)"


# Compiling the drivers.
$(dir_exe)%.exe: $(dir_drivers)%.c $(target_lib)
	gcc -o $@ $< $(EXE_COMPILE_DIRECTIVES)
	@echo STDLIB.H: Compiled $<.

# Joining the library.
$(target_lib): $(object_files)
	ar cr $@ $^
	@echo STDLIB.H: Libraryd.

# Assembling each object file.
$(dir_obj)%.o: $(dir_src)%.c 
	gcc -c $< -o $@ $(OBJ_COMPILE_DIRECTIVES)
	@echo STDLIB.H: Assembled $@.



# Cleaning		<make clean>
# ================================

# TODO: Complete...



.PHONY: build
.PHONY: clean

# <>
