TODOs
=====

Things yet to do on the project. Last update on 10/06/24.



Smaller stuff
-------------

Simpler things, that, besides not being that critical, must have attention.

* [ ] Organize the printing of the application response.
* [ ] Help directive "-h" display on main exe. call.
* [ ]  Search errors debug and error presentation.
* [X] Logging construction optionally specified in makefile building.
* [ ] (Ponder) Refactor frame_t to Frame?
* [ ] Make B / B* trees split-root, split-child safe.
* [ ] (Analysis) Frame to search_registry?
* [ ] Choose colors for the printing and logging. 


Bigger stuff
------------

Critical things to attent.

* [ ] Ensure system will works accross the target plataforms - it is: linux amd64.
* [x] Set properly the external red-black tree.
* [ ] Integrate the "frame" into the remaining searching methods.
* [X] Make an automated testing system.

### Data Analysis
* [ ] For measuring with that main search executable: separated results report via logging system, for ease in obtaining the data.
* [ ] Separated executable for execution various consecutive searches for further measures and profiling.
	* [ ] ISS.
	* [ ] B tree.
	* [ ] B+ tree.
	* [ ] EBST and ERBT.
* [ ] Improve and finish-up the registry data-generation scheme. 
	

### Revision
Things that urges being revised.

	. ISS's index table adapted externally.
		- Currently, it is entirely assembled on main-memory. Since the point is not to use that much space on main-memory, this should be considered. That will have, by extension, have to do as well with the definition of the page-size for various types of them on the application. Should it be defined after specified the "maximum space" for use in main-memory?


Perhaps'es
------------

	. B / B* trees removing system.