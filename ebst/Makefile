

target_exe	:= EBST.exe


build: $(target_exe)
	@echo Compiled.


sources := src/ebst.c src/stack.c

$(target_exe): $(sources)
	gcc $^ -o $@ -Wall

