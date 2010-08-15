#include "../Include/_All.h"

#ifdef _WINDOWS
  #include <direct.h>
  #include <windows.h>
#else
  #include <unistd.h>
  #include <dirent.h>
  #include <stdlib.h>
  #define _chdir chdir
  #define _getcwd getcwd
#endif

#include <stdarg.h>

using namespace R5;

//============================================================================================================
// Global access
//============================================================================================================

FILE* g_file = 0;

//============================================================================================================
// File container that has a destructor that will properly close the file
//============================================================================================================

struct File
{
	Thread::Lockable lock;

	File (const char* filename) { g_file = fopen(filename, "w"); }
	~File() { if (g_file) { fflush(g_file); fclose(g_file); g_file = 0; } }

	operator FILE* () { return g_file; }
	operator bool () const { return g_file != 0; }

	void Lock()   const { lock.Lock(); }
	void Unlock() const { lock.Unlock(); }
};

//============================================================================================================
// Returns the OS type
//============================================================================================================

#ifdef _WINDOWS
uint System::GetOS() { return System::OS::Windows; }
#elif defined _MACOS
uint System::GetOS() { return System::OS::MacOS; }
#elif defined _LINUX
uint System::GetOS() { return System::OS::Linux; }
#else
uint System::GetOS() { return System::OS::Unknown; }
#endif

//============================================================================================================
// Executes the specified command via shell
//============================================================================================================

int System::Execute (const char* command) { return ::system(command); }

//============================================================================================================
// Changes the local working directory
//============================================================================================================

bool System::SetCurrentPath(const char* path)
{
	return (_chdir(path) == 0);
}

//============================================================================================================
// Flush the log file, saving out the contents
//============================================================================================================

void System::FlushLog()
{
	if (g_file != 0) fflush(g_file);
}

//============================================================================================================
// Dumps a string into the log file
//============================================================================================================

void System::Log(const char *format, ...)
{
	static File log ("log.txt");
	static ulong last = 0;

	// In rare cases the log may need to be used after the static variable has been released.
	// If this happens, open the log manually and simply append to it.
	if (g_file == 0) g_file = fopen("log.txt", "a");

	if (g_file != 0)
	{
		if (log) log.Lock();
		{
			va_list args;
			va_start(args, format);
			
#ifdef _WINDOWS
			uint iLength = _vscprintf(format, args) + 1;
			char* text = new char[iLength];
#else
			static char text[2048];
#endif
			vsprintf(text, format, args);
			va_end(args);

			Time::Update();
			ulong totalMS	= Time::GetMilliseconds();
			ulong remMS		= totalMS % 1000;
			ulong totalSEC	= totalMS / 1000;
			ulong remSEC	= totalSEC % 60;
			ulong totalMIN	= totalSEC / 60;
			ulong remMIN	= totalMIN % 60;
			ulong totalHRS	= totalMIN / 60;
			
			fprintf(g_file, "[%3u.%02u.%02u.%03u]: ", (uint)totalHRS, (uint)remMIN, (uint)remSEC, (uint)remMS);
			fputs(text, g_file);
			fputc('\n', g_file);

#ifdef _WINDOWS
			delete [] text;
#endif
			// Write to file no more frequently than every 100 milliseconds
			if (log && totalMS - last > 100)
			{
				last = totalMS;
				fflush(g_file);
			}
		}
		if (log) log.Unlock();
		else
		{
			fclose(g_file);
			g_file = 0;
		}
	}
}

//============================================================================================================
// Checks if the file exists
//============================================================================================================

bool System::FileExists(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp) { fclose(fp); return true; }
	return false;
}

//============================================================================================================
// Gets the file's header
//============================================================================================================

uint System::GetFileHeader(const char* filename)
{
	// The first 4 bytes are enough to tell what type of file it is
	FILE* fp = fopen(filename, "rb");
	if (!fp) return 0;
	uint fd;
	fread(&fd, sizeof(uint), 1, fp);
	fclose(fp);
	return fd;
}

//============================================================================================================
// Gets the current folder path (ends with '/')
//============================================================================================================

String System::GetCurrentPath()
{
	char temp[256] = {0};
	_getcwd(temp, 255);
	String path(temp);
	path.Replace("\\", "/", true);
	if (!path.EndsWith("/")) path << "/";
	return path;
}

//============================================================================================================
// "c:\temp\test.abc" becomes "test.abc"
//============================================================================================================

String System::GetFilenameFromPath (const String& in, bool extension)
{
	for (uint i = in.GetLength(); i > 0; )
	{
		char character = in[--i];

		if (character == '/' || character == '\\')
		{
			String out;
			in.GetString(out, i + 1);

			if (!extension)
			{
				for (uint b = in.GetLength(); b > i; )
				{
					char character = in[--b];

					if (character == '.')
					{
						in.GetString(out, i + 1, b);
						break;
					}
				}
			}
			return out;
		}
	}
	return in;
}

//============================================================================================================
// "c:\temp\test.abc" becomes "c:\temp\"
//============================================================================================================

String System::GetPathFromFilename (const String& in)
{
	for (uint i = in.GetLength(); i > 0; )
	{
		char character = in[--i];

		if (character == '/' || character == '\\')
		{
			String out;
			in.GetString(out, 0, i + 1);
			out.Replace("\\", "/", true);
			return out;
		}
	}
	return "";
}

//============================================================================================================
// "c:\temp\test.abc" becomes "abc"
//============================================================================================================

String System::GetExtensionFromFilename (const String& in)
{
	for (uint i = in.GetLength(); i > 0; )
	{
		char character = in[--i];
		if (character == '.')
		{
			String out;
			in.GetString(out, i + 1);
			return out;
		}
	}
	return "";
}

//============================================================================================================
// Reads the contents of the specified folder, populating file and folder lists
//============================================================================================================

bool System::ReadFolder (const String& dir, Array<String>& folders, Array<String>& files)
{
	String file;

	String path (dir);
	path.Replace("\\", "/", true);
	if (path.IsValid() && !path.EndsWith("/")) path << "/";
	else if (path == "/") path.Clear();

#ifdef _WINDOWS
	WIN32_FIND_DATA info;

	String match (path);
	match << "*.*";

	void* fileHandle = FindFirstFile(match.GetBuffer(), &info);
	if (fileHandle == INVALID_HANDLE_VALUE) return false;

	do 
	{
		file = info.cFileName;

		if (!file.BeginsWith("."))
		{
			if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				String& out = folders.Expand();
				out << path;
				out << file;
				out << "/";
			}
			else
			{
				String& out = files.Expand();
				out << path;
				out << file;
			}
		}
	}
	while (FindNextFile(fileHandle, &info));

#else
	DIR* dp = opendir(dir.IsValid() ? dir.GetBuffer() : "./");
	if (dp == 0) return false;
	struct dirent* dirp = 0;

	while ((dirp = readdir(dp)) != 0)
	{
		String file (dirp->d_name);

		if (!file.BeginsWith("."))
		{
			if (dirp->d_type == DT_DIR)
			{
				String& out = folders.Expand();
				out << path;
				out << file;
				out << "/";
			}
			else
			{
				String& out = files.Expand();
				out << path;
				out << file;
			}
		}
	}
    closedir(dp);
#endif
	folders.Sort();
	files.Sort();
    return true;
}

//============================================================================================================
// Fills out a list of all files with the partial path matching 'path'. Returns 'true' if one was found.
//============================================================================================================

bool System::GetFiles (const String& path, Array<String>& files, bool recursive)
{
	if (FileExists(path))
	{
		files.Expand() = path;
	}
	else
	{
		String dir  (GetPathFromFilename(path));
		String name (GetFilenameFromPath(path, false));

		String match (dir);
		match << name;

		Array<String> fd, fl;

		if (ReadFolder(dir, fd, fl))
		{
			FOREACH(i, fl)
			{
				const String& s = fl[i];

				if (s.BeginsWith(match))
				{
					String& fout = files.Expand();
					fout << s;
				}
			}

			if (recursive)
			{
				FOREACH(i, fd)
				{
					GetFiles(fd[i], files, true);
				}
			}
		}
	}
	return files.IsValid();
}

//============================================================================================================
// Returns the best matching filename that exists. Allows specifying a different extension than
// that of the existing file. "c:/temp/test.abc" will match "c:/temp/test.txt" if it exists instead.
//============================================================================================================

String System::GetBestMatch (const String& filename)
{
	if (FileExists(filename)) return filename;

	String dir  (GetPathFromFilename(filename));
	String name (GetFilenameFromPath(filename, false));

	if (name.IsValid())
	{
		String match (dir);
		match << name;

		Array<String> fd, fl;

		if (ReadFolder(dir, fd, fl))
		{
			FOREACH(i, fl)
			{
				const String& s = fl[i];
				if (s.BeginsWith(match)) return s;
			}
		}
	}
	return "";
}