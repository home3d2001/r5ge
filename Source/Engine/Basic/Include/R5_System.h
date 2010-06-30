#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

template <typename T> struct Array;

namespace System
{
	struct OS
	{
		enum
		{
			Unknown			= 0x0,
			Windows			= 0x1,
			MacOS			= 0x2,
			Linux			= 0x4,
		};
	};

	uint	GetOS();											// Returns the current operating system
	bool	SetCurrentPath			(const char* path);			// _chdir()
	void	Log						(const char* format, ...);	// Dumps a string into the log file
	void	FlushLog();											// Flush the log file, saving out the contents
	bool	FileExists				(const char* filename);		// Checks if the file exists
	uint	GetFileHeader			(const char* filename);		// Gets the file's header (first 4 bytes)
	String	GetFilenameFromPath		(const String& path);		// "c:\temp\test.abc" becomes "test.abc"
	String	GetPathFromFilename		(const String& file);		// "c:\temp\test.abc" becomes "c:\temp\"
	String	GetExtensionFromFilename(const String& file);		// "c:\temp\test.abc" becomes "abc"

	// Reads the contents of the specified folder, populating file and folder lists
	bool ReadFolder (const String& dir, Array<String>& folders, Array<String>& files);
};