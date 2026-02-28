========================================================================
       CONSOLE APPLICATION : Test
========================================================================


AppWizard has created this Test application for you.  

This file contains a summary of what you will find in each of the files that
make up your Test application.

Test.dsp
    This file (the project file) contains information at the project level and
    is used to build a single project or subproject. Other users can share the
    project (.dsp) file, but they should export the makefiles locally.

Test.cpp
    This is the main application source file.

Test.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named Test.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
Tests to be run

class CObservableObject:
	Test (1) 
		- Add an observer
		- Fire a change
		* Observer should receive the change

	Test (2) 
		- Remove the observer
		- Fire a change
		* Observer should not receive the change

class CValue:
	For different template parameters
	Test (1) 
		- Generate an initial value
		- Create the CValue object with the initial value
		- Display the value in the newly created object
		* Value should match initial value

	Test (2) 
		- Add a change listener
		- Generate a new value
		- Change to the new value
		* Listener should receive a change event with old value
			equal to the initial value

	Test (3)
		- Display the value in the object
		* Value should match new value
		
	Test (4)
		- Remove the change listener
		- Change the object with a second new value
		* No change should be received by listener

	Test (5)
		- Read the value
		* The stored value should equal the second new value

	Test (6)
		Synchronization - values
		Test for synchronization to NULL, synchronization to self

	Test (7)
		Synchronization - change events

	Test (8)
		De-Synchronization - values

	Test (8)
		De-Synchronization - change events

	Test (9) 
		De-Synchronization - after synchronized object is deleted
	

Association
	Test (1)
	.
	.
	.

AutoSyncValue
	Test for initial update