/* portable.h - wrappers around things that vary from server
 * to server and operating system to operating system.
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#ifndef PORTABLE_H
#define PORTABLE_H

#include <sys/types.h>
#include <sys/stat.h>

struct slName *listDir(char *dir, char *pattern);
/* Return an alphabetized list of all files that match
 * the wildcard pattern in directory. */

struct slName *listDirRegEx(char *dir, char *regEx, int flags);
/* Return an alphabetized list of all files that match
 * the regular expression pattern in directory.
 * See REGCOMP(3) for flags (e.g. REG_ICASE)  */

struct slName *pathsInDirAndSubdirs(char *dir, char *wildcard);
/* Return list of all non-directory files in dir and it's
 * subdirs.  Returns path to files including dir and subdir. */

struct fileInfo
/* Info about a file. */
    {
    struct fileInfo  *next;	/* Next in list. */
    off_t size;		/* Size in bytes. */
    bool isDir;		/* True if file is a directory. */
    int statErrno;	/* Result of stat (e.g. bad symlink). */
    time_t lastAccess;  /* Last access time. */
    time_t creationTime; /* Creation time. */
    char name[1];	/* Allocated at run time. */
    };

struct fileInfo *newFileInfo(char *name, off_t size, bool isDir, int statErrno,
	time_t lastAccess, time_t creationTime);
/* Return a new fileInfo. */

struct fileInfo *listDirXExt(char *dir, char *pattern, boolean fullPath, boolean ignoreStatFailures);
/* Return list of files matching wildcard pattern with
 * extra info. If full path is true then the path will be
 * included in the name of each file.  You can free the
 * resulting list with slFreeList. */

struct fileInfo *listDirX(char *dir, char *pattern, boolean fullPath);
/* Return list of files matching wildcard pattern with
 * extra info. If full path is true then the path will be
 * included in the name of each file.  You can free the
 * resulting list with slFreeList. */

char *getCurrentDir();
/* Return current directory.  Abort if it fails. */

void setCurrentDir(char *newDir);
/* Set current directory.  Abort if it fails. */

boolean maybeSetCurrentDir(char *newDir);
/* Change directory, return FALSE (and set errno) if fail. */

boolean makeDir(char *dirName);
/* Make dir.  Returns TRUE on success.  Returns FALSE
 * if failed because directory exists.  Prints error
 * message and aborts on other error. */

void makeDirsOnPath(char *pathName);
/* Create directory specified by pathName.  If pathName contains
 * slashes, create directory at each level of path if it doesn't
 * already exist.  Abort with error message if there's a problem.
 * (It's not considered a problem for the directory to already
 * exist. ) */

char *simplifyPathToDir(char *path);
/* Return path with ~ (for home) and .. taken out.   freeMem result when done. */

long clock1000();
/* 1000 hz clock */

void sleep1000(int milli);
/* Sleep for given number of milliseconds. */

long clock1();
/* A 1 hz clock. */

char *getTempDir(void);
/* get temporary directory to use for programs.  This first checks TMPDIR environment
 * variable, then /data/tmp, /scratch/tmp, /var/tmp, /tmp.  Return is static and
 * only set of first call */

char *rTempName(char *dir, char *base, char *suffix);
/* Make a temp name that's almost certainly unique. */

/* This structure helps us generate temp names and use
 * them.  Since different servers locate where the cgi
 * runs from differently, and where the generated html
 * file runs from - this is necessary for portable code. */
struct tempName
	{
	char forCgi[4096];
	char forHtml[4096];
	};

void makeTempName(struct tempName *tn, char *base, char *suffix);
/* Make a good name for a temp file. */

void mustRename(char *oldName, char *newName);
/* Rename file or die trying. */

void mustRemove(char *path);
/* Remove file or die trying */

char *semiUniqName(char *base);
/* Figure out a name likely to be unique.
 * Name will have no periods.  Returns a static
 * buffer, so best to clone result unless using
 * immediately. */

char *cgiDir();
/* Return directory to look for cgi in. */

//char *trashDir();
/* Return directory for relative path to trash from cgi binaries */

void mkdirTrashDirectory(char *prefix);
/*	create the specified trash directory if it doesn't exist */

double machineSpeed();
/* Return relative speed of machine.  UCSC CSE dept. 1999 web server is 1.0 */

char *mysqlHost();
/* Return host computer on network for mySQL database. */

char *getHost();
/* Get name of this machine. */

void uglyfBreak();
/* Invoke the debugger. */

char *getUser();
/* Get user name */

void envUpdate(char *name, char *value);
/* Update an environment string */

int mustFork();
/* Fork or abort. */

int rawKeyIn();
/* Read in an unbuffered, unechoed character from keyboard. */

time_t fileModTime(char *pathName);
/* Return file last modification time.  The units of
 * these may vary from OS to OS, but you can depend on
 * later files having a larger time. */


boolean isPipe(int fd);
/* determine in an open file is a pipe  */

boolean maybeTouchFile(char *fileName);
/* If file exists, set its access and mod times to now.  If it doesn't exist, create it.
 * Return FALSE if we have a problem doing so. */

void touchFileFromFile(const char *oldFile, const char *newFile);
/* Set access and mod time of newFile from oldFile. */

boolean isDirectory(char *pathName);
/* Return TRUE if pathName is a directory. */

void mustBeReadableAndRegularFile(char *fileName);
/* errAbort if fileName is a regular file and readable. */

boolean isRegularFile(char *fileName);
/* Return TRUE if fileName is a regular file. */

char *mustReadSymlinkExt(char *path, struct stat *sb);
/* Read symlink or abort. FreeMem the returned value. */

char *mustReadSymlink(char *path);
/* Read symlink or abort. Checks that path is a symlink.
FreeMem the returned value. */

void makeSymLink(char *oldName, char *newName);
/* Return a symbolic link from newName to oldName or die trying */

long long freeSpaceOnFileSystem(char *path);
/* Given a path to a file or directory on a file system,  return free space
 * in bytes. */

void setMemLimit(unsigned long maxMem);
/* Set the maximum amount of memory that the application can use. */

#endif /* PORTABLE_H */

