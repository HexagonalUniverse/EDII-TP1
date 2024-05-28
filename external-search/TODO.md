TODOs
=====

Things yet to do on the project. Last update on 27/5/24.



Smaller stuff
-------------

Simpler things, that, besides not being that critical, must have attention.

	. Choose colors for the printing and logging. 
	. Organize the printing of the application response.
	. Help directive "-h" display on main exe. call.
	. Search errors debug and error presentation.
	. Logging construction optionally specified in makefile building. Search whether it can be accomplished with parameters passed to makefile.
	. Refactor frame_t to Frame?
	. Make B / B* trees split-root, split-child safe.
	. (Analysis) Frame to search_registry?


Bigger stuff
------------

Critical things to attent.

	. Ensure system will works accross the target plataforms - it is: linux amd64.
	. Set properly the external red-black tree.
		Our integration attempt failed, for say; so it is still pendent.
	. Make an automated testing system.

### Data Analysis
	. For measuring with that main search executable: separated results report via logging system, for ease in obtaining the data.
	. Separated executable for execution various consecutive searches for further measures and profiling.
	. Improve / finish-up the registry data-generation scheme. 
	

### Revision
Things that urges being revised.

	. ISS's index table adapted externally.
		- Currently, it is entirely assembled on main-memory. Since the point is not to use that much space on main-memory, this should be considered. That will have, by extension, have to do as well with the definition of the page-size for various types of them on the application. Should it be defined after specified the "maximum space" for use in main-memory?


Perhaps'es
------------

	. B / B* trees removing system.