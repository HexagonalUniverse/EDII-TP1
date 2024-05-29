external-search
===============

Here's where the code part for the project (assignment) resides in.


Organization
------------

In \<*bin/*\> the general binaries are placed. 
In \<*bin/obj/*> the temporary objects files assembled are placed, and in \<*bin/exe/*\> are the target executables. 
The bone source code are located at \<*src/*\>, and the ones that generates the target executables are at \<*drivers/*\>. 
Temporary data and logging are stored at \<*temp/*\>, and at \<*data/*\> there is the factory for the registry files, as well as the data analysis code for the program.
At \<*tests/*\> the automated tests source and data are located.


Set-up and building
-------------------

How to set-up the project.

### Requirements
    
    . GNU GCC compiler and Make. 
    . Either windows or linux as the OS.
    . x86 or amd64 for the architecture.
    

### Building

To generate the target executables run 
> make build


To activate the comparisons counter, build them setting *TRANSPARENT=1*.
To activate the debug mode, build them setting *DEBUG=1*. To activate it and set it to *stderr*, set *DEBUG=2*. In case of activating both:
> make build TRANSPARENT=1 DEBUG=1


### * Automated testing

(Not yet implemented) 

For asserting all automated tests, run
> make test

Applies the same building directives if one wants to have the debugging in them, etc.

The tests will be first built, depending upon the static core library, and then tested for their inputs.


### Cleaning

For cleaning the byproduct (and the parts doesn't add for the final use of the programs), run
> make clean

In it, the object files and the test executables will be erased.



