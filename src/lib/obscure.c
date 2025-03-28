/* Obscure stuff that is handy every now and again.
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include <unistd.h>
//#include <sys/syscall.h>
#include "portable.h"
#include "localmem.h"
#include "hash.h"
#include "obscure.h"
#include "linefile.h"
#include "sqlNum.h"

static int _dotForUserMod = 100; /* How often does dotForUser() output a dot. */

long incCounterFile(char *fileName)
/* Increment a 32 bit value on disk. */
{
long val = 0;
FILE *f = fopen(fileName, "r+b");
if (f != NULL)
    {
    mustReadOne(f, val);
    rewind(f);
    }
else
    {
    f = fopen(fileName, "wb");
    }
++val;
if (f != NULL)
    {
    fwrite(&val, sizeof(val), 1, f);
    if (fclose(f) != 0)
        errnoAbort("fclose failed");
    }
return val;
}

int digitsBaseTwo(unsigned long x)
/* Return base two # of digits. */
{
int digits = 0;
while (x)
    {
    digits += 1;
    x >>= 1;
    }
return digits;
}

int digitsBaseTen(int x)
/* Return number of digits base 10. */
{
int digCount = 1;
if (x < 0)
    {
    digCount = 2;
    x = -x;
    }
while (x >= 10)
    {
    digCount += 1;
    x /= 10;
    }
return digCount;
}

void writeGulp(char *file, char *buf, int size)
/* Write out a bunch of memory. */
{
FILE *f = mustOpen(file, "w");
mustWrite(f, buf, size);
carefulClose(&f);
}

void readInGulp(char *fileName, char **retBuf, size_t *retSize)
/* Read whole file in one big gulp. */
{
if (fileExists(fileName) && !isRegularFile(fileName))
    errAbort("can only read regular files with readInGulp: %s", fileName);
size_t size = (size_t)fileSize(fileName);
char *buf;
FILE *f = mustOpen(fileName, "rb");
*retBuf = buf = needLargeMem(size+1);
mustRead(f, buf, size);
buf[size] = 0;      /* Just in case it needs zero termination. */
fclose(f);
if (retSize != NULL)
    *retSize = size;
}

void readAllWords(char *fileName, char ***retWords, int *retWordCount, char **retBuf)
/* Read in whole file and break it into words. You need to freeMem both
 * *retWordCount and *retBuf when done. */
{
int wordCount;
char *buf = NULL;
char **words = NULL;
size_t bufSize;

readInGulp(fileName, &buf, &bufSize);
wordCount = chopByWhite(buf, NULL, 0);
if (wordCount != 0)
    {
    words = needMem(wordCount * sizeof(words[0]));
    chopByWhite(buf, words, wordCount);
    }
*retWords = words;
*retWordCount = wordCount;
*retBuf = buf;
}

int countWordsInFile(char *fileName)
/* Count number of words in file. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *line;
int wordCount = 0;
while (lineFileNext(lf, &line, NULL))
    wordCount += chopByWhite(line, NULL, 0);
lineFileClose(&lf);
return wordCount;
}

struct hash *hashWordsInFile(char *fileName, int hashSize)
/* Create a hash of space delimited words in file. */
{
struct hash *hash = newHash(hashSize);
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *line, *word;
while (lineFileNext(lf, &line, NULL))
    {
    while ((word = nextWord(&line)) != NULL)
        hashAdd(hash, word, NULL);
    }
lineFileClose(&lf);
return hash;
}

struct hash *hashNameIntFile(char *fileName)
/* Given a two column file (name, integer value) return a
 * hash keyed by name with integer values */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];
struct hash *hash = hashNew(16);
while (lineFileRow(lf, row))
    hashAddInt(hash, row[0], lineFileNeedNum(lf, row, 1));
lineFileClose(&lf);
return hash;
}

struct hash *hashTwoColumnFile(char *fileName)
/* Given a two column file (key, value) return a hash. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
struct hash *hash = hashNew(16);
char *row[3];
int fields = 0;
// Try tab-separated first; if no tabs, then try whitespace-separated.
while ((fields = lineFileChopTab(lf, row)) != 0)
    {
    if (fields == 1)
        {
        char *line = row[0];
        fields = chopLine(line, row);
        }
    lineFileExpectWords(lf, 2, fields);
    char *name = row[0];
    char *value = lmCloneString(hash->lm, row[1]);
    hashAdd(hash, name, value);
    }
lineFileClose(&lf);
return hash;
}

struct hash *hashTsvBy(char *in, int keyColIx, int *retColCount)
/* Return a hash of rows keyed by the given column */
{
struct lineFile *lf = lineFileOpen(in, TRUE);
struct hash *hash = hashNew(0);
char *line = NULL, **row = NULL;
int colCount = 0, colAlloc=0;	/* Columns as counted and as allocated */
while (lineFileNextReal(lf, &line))
    {
    if (colCount == 0)
        {
	*retColCount = colCount = chopByChar(line, '\t', NULL, 0);
	verbose(2, "Got %d columns in first real line\n", colCount);
	colAlloc = colCount + 1;  // +1 so we can detect unexpected input and complain
	lmAllocArray(hash->lm, row, colAlloc);
	}
    int count = chopByChar(line, '\t', row, colAlloc);
    if (count != colCount)
        {
	errAbort("Expecting %d words, got more than that line %d of %s",
	    colCount, lf->lineIx, lf->fileName);
	}
    hashAdd(hash, row[keyColIx], lmCloneRow(hash->lm, row, colCount) );
    }
lineFileClose(&lf);
return hash;
}

void writeTsvRow(FILE *f, int rowSize, char **row)
/* Write out row of strings to a line in tab-sep file */
{
if (rowSize > 0)
    {
    fprintf(f, "%s", row[0]);
    int i;
    for (i=1; i<rowSize; ++i)
        fprintf(f, "\t%s", row[i]);
    }
fprintf(f, "\n");
}

struct slPair *slPairTwoColumnFile(char *fileName)
/* Read in a two column file into an slPair list */
{
char *row[2];
struct slPair *list = NULL;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
while (lineFileRow(lf, row))
    slPairAdd(&list, row[0], cloneString(row[1]));
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct slName *readAllLines(char *fileName)
/* Read all lines of file into a list.  (Removes trailing carriage return.) */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
struct slName *list = NULL, *el;
char *line;

while (lineFileNext(lf, &line, NULL))
     {
     el = newSlName(line);
     slAddHead(&list, el);
     }
lineFileClose(&lf);
slReverse(&list);
return list;
}

void copyFile(char *source, char *dest)
/* Copy file from source to dest. */
{
int bufSize = 64*1024;
char *buf = needMem(bufSize);
int bytesRead;
int s, d;

s = open(source, O_RDONLY);
if (s < 0)
    errAbort("Couldn't open %s. %s\n", source, strerror(errno));
d = creat(dest, 0777);
if (d < 0)
    {
    close(s);
    errAbort("Couldn't open %s. %s\n", dest, strerror(errno));
    }
while ((bytesRead = read(s, buf, bufSize)) > 0)
    {
    if (write(d, buf, bytesRead) < 0)
        errAbort("Write error on %s. %s\n", dest, strerror(errno));
    }
close(s);
if (close(d) != 0)
    errnoAbort("close failed");
freeMem(buf);
}

void copyOpenFile(FILE *inFh, FILE *outFh)
/* copy an open stdio file */
{
int c;
while ((c = fgetc(inFh)) != EOF)
    fputc(c, outFh);
if (ferror(inFh))
    errnoAbort("file read failed");
if (ferror(outFh))
    errnoAbort("file write failed");
}

void cpFile(int s, int d)
/* Copy from source file to dest until reach end of file. */
{
int bufSize = 64*1024, readSize;
char *buf = needMem(bufSize);

for (;;)
    {
    readSize = read(s, buf, bufSize);
    if (readSize > 0)
        mustWriteFd(d, buf, readSize);
    if (readSize <= 0)
        break;
    }
freeMem(buf);
}

void *charToPt(char c)
/* Convert char to pointer. Use when really want to store
 * a char in a pointer field. */
{
char *pt = NULL;
return pt+c;
}

char ptToChar(void *pt)
/* Convert pointer to char.  Use when really want to store a
 * pointer in a char. */
{
char *a = NULL, *b = pt;
return b - a;
}


void *intToPt(int i)
/* Convert integer to pointer. Use when really want to store an
 * int in a pointer field. */
{
char *pt = NULL;
return pt+i;
}

int ptToInt(void *pt)
/* Convert pointer to integer.  Use when really want to store a
 * pointer in an int. */
{
char *a = NULL, *b = pt;
return b - a;
}

void *sizetToPt(size_t i)
/* Convert size_t to pointer. Use when really want to store a
 * size_t in a pointer. */
{
char *pt = NULL;
return pt+i;
}

size_t ptToSizet(void *pt)
/* Convert pointer to size_t.  Use when really want to store a
 * pointer in a size_t. */
{
char *a = NULL, *b = pt;
return b - a;
}

boolean parseQuotedStringNoEscapes( char *in, char *out, char **retNext)
/* Read quoted string from in (which should begin with first quote).
 * Write unquoted string to out, which may be the same as in.
 * Return pointer to character past end of string in *retNext.
 * Return FALSE if can't find end.
 * Unlike parseQuotedString() do not treat backslash as an escape
 *	character, merely pass it on through.
 */
{
char c, *s = in;
int quoteChar = *s++;

for (;;)
   {
   c = *s++;
   if (c == 0)
       {
       warn("Unmatched %c", quoteChar);
       return FALSE;
       }
   else if (c == quoteChar)
       break;
   else
       *out++ = c;
   }
*out = 0;
if (retNext != NULL)
    *retNext = s;
return TRUE;
}

boolean parseQuotedString( char *in, char *out, char **retNext)
/* Read quoted string from in (which should begin with first quote).
 * Write unquoted string to out, which may be the same as in.
 * Return pointer to character past end of string in *retNext.
 * Return FALSE if can't find end. */
{
char c, *s = in;
int quoteChar = *s++;
boolean escaped = FALSE;

for (;;)
   {
   c = *s++;
   if (c == 0)
       {
       warn("Unmatched %c", quoteChar);
       return FALSE;
       }
   if (escaped)
       {
       if (c == '\\' || c == quoteChar)
          *out++ = c;
       else
          {
	  *out++ = '\\';
	  *out++ = c;
	  }
       escaped = FALSE;
       }
   else
       {
       if (c == '\\')
           escaped = TRUE;
       else if (c == quoteChar)
           break;
       else
           *out++ = c;
       }
   }
*out = 0;
if (retNext != NULL)
    *retNext = s;
return TRUE;
}

char *nextQuotedWord(char **pLine)
/* Generalization of nextWord.  Returns next quoted
 * string or if no quotes next word.  Updates *pLine
 * to point past word that is returned. Does not return
 * quotes. */
{
char *line, c;
line = skipLeadingSpaces(*pLine);
if (line == NULL || line[0] == 0)
    return NULL;
c = *line;
if (c == '"' || c == '\'')
    {
    if (!parseQuotedString(line, line, pLine))
        return NULL;
    return line;
    }
else
    {
    return nextWord(pLine);
    }
}

struct slName *slNameListOfUniqueWords(char *text,boolean respectQuotes)
/* Return list of unique words found by parsing string delimited by whitespace.
 * If respectQuotes then ["Lucy and Ricky" 'Fred and Ethyl'] will yield 2 slNames no quotes */
{
struct slName *list = NULL;
char *word = NULL;
while (text != NULL)
    {
    if (respectQuotes)
        word = nextQuotedWord(&text);
    else
        word = nextWord(&text);
    if (word)
        slNameStore(&list, word);
    else
        break;
    }
slReverse(&list);
return list;
}

void escCopy(char *in, char *out, char toEscape, char escape)
/* Copy in to out, escaping as needed.  Out better be big enough.
 * (Worst case is strlen(in)*2 + 1.) */
{
char c;
for (;;)
    {
    c = *in++;
    if (c == toEscape)
        *out++ = escape;
    *out++ = c;
    if (c == 0)
        break;
    }
}

char *makeEscapedString(char *in, char toEscape)
/* Return string that is a copy of in, but with all
 * toEscape characters preceded by '\'
 * When done freeMem result. */
{
int newSize = strlen(in) + countChars(in, toEscape);
char *out = needMem(newSize+1);
escCopy(in, out, toEscape, '\\');
return out;
}

char *makeQuotedString(char *in, char quoteChar)
/* Create a string surrounded by quoteChar, with internal
 * quoteChars escaped.  freeMem result when done. */
{
int newSize = 2 + strlen(in) + countChars(in, quoteChar);
char *out = needMem(newSize+1);
out[0] = quoteChar;
escCopy(in, out+1, quoteChar, '\\');
out[newSize-1] = quoteChar;
return out;
}

struct hash *hashThisEqThatLine(char *line, int lineIx, boolean firstStartsWithLetter)
/* Return a symbol table from a line of form:
 *   1-this1=val1 2-this='quoted val2' var3="another val"
 * If firstStartsWithLetter is true, then the left side of the equals must start with
 * a letter. */
{
char *dupe = cloneString(line);
char *s = dupe, c;
char *var, *val;
struct hash *hash = newHash(8);

for (;;)
    {
    if ((var = skipLeadingSpaces(s)) == NULL)
        break;

    if ((c = *var) == 0)
        break;
    if (firstStartsWithLetter && !isalpha(c))
	errAbort("line %d of custom input: variable needs to start with letter '%s'", lineIx, var);
    val = strchr(var, '=');
    if (val == NULL)
        {
        errAbort("line %d of var %s in custom input: %s \n missing = in var/val pair", lineIx, var, line);
        }
    *val++ = 0;
    c = *val;
    if (c == '\'' || c == '"')
        {
	if (!parseQuotedString(val, val, &s))
	    errAbort("line %d of input: missing closing %c", lineIx, c);
	}
    else
	{
	s = skipToSpaces(val);
	if (s != NULL) *s++ = 0;
	}
    hashAdd(hash, var, cloneString(val));
    }
freez(&dupe);
return hash;
}

struct hash *hashVarLine(char *line, int lineIx)
/* Return a symbol table from a line of form:
 *   var1=val1 var2='quoted val2' var3="another val" */
{
return hashThisEqThatLine(line, lineIx, TRUE);
}

struct slName *stringToSlNames(char *string)
/* split string and convert to a list of slNames separated by
 * white space, but allowing multiple words in quotes.
 * Quotes if any are stripped.  */
{
struct slName *list = NULL, *name;
char *dupe = cloneString(string);
char c, *s = dupe, *e = NULL;

for (;;)
    {
    if ((s = skipLeadingSpaces(s)) == NULL)
        break;
    if ((c = *s) == 0)
        break;
    if (c == '\'' || c == '"')
        {
	if (!parseQuotedString(s, s, &e))
	    errAbort("missing closing %c in %s", c, string);
	}
    else
        {
	e = skipToSpaces(s);
	if (e != NULL) *e++ = 0;
	}
    name = slNameNew(s);
    slAddHead(&list, name);
    s = e;
    }
freeMem(dupe);
slReverse(&list);
return list;
}

struct slName *charSepToSlNames(char *string, char c)
/* Split string and convert character-separated list of items to slName list.
 * Note that the last occurence of c is optional.  (That
 * is for a comma-separated list a,b,c and a,b,c, are
 * equivalent. */
{
struct slName *list = NULL, *el;
char *s, *e;

s = string;
while (s != NULL && s[0] != 0)
    {
    e = strchr(s, c);
    if (e == NULL)
        {
	el = slNameNew(s);
	slAddHead(&list, el);
	break;
	}
    else
        {
	el = slNameNewN(s, e - s);
	slAddHead(&list, el);
	s = e+1;
	}
    }
slReverse(&list);
return list;
}

struct slName *commaSepToSlNames(char *commaSep)
/* Split string and convert comma-separated list of items to slName list.  */
{
return charSepToSlNames(commaSep, ',');
}


void sprintLongWithCommas(char *s, long long l)
/* Print out a long number with commas a thousands, millions, etc. */
{
long long trillions, billions, millions, thousands;
if (l >= 1000000000000LL)
    {
    trillions = l/1000000000000LL;
    l -= trillions * 1000000000000LL;
    billions = l/1000000000;
    l -= billions * 1000000000;
    millions = l/1000000;
    l -= millions * 1000000;
    thousands = l/1000;
    l -= thousands * 1000;
    sprintf(s, "%lld,%03lld,%03lld,%03lld,%03lld", trillions, billions, millions, thousands, l);
    }
else if (l >= 1000000000)
    {
    billions = l/1000000000;
    l -= billions * 1000000000;
    millions = l/1000000;
    l -= millions * 1000000;
    thousands = l/1000;
    l -= thousands * 1000;
    sprintf(s, "%lld,%03lld,%03lld,%03lld", billions, millions, thousands, l);
    }
else if (l >= 1000000)
    {
    millions = l/1000000;
    l -= millions * (long long)1000000;
    thousands = l/1000;
    l -= thousands * 1000;
    sprintf(s, "%lld,%03lld,%03lld", millions, thousands, l);
    }
else if (l >= 1000)
    {
    thousands = l/1000;
    l -= thousands * 1000;
    sprintf(s, "%lld,%03lld", thousands, l);
    }
else
    sprintf(s, "%lld", l);
}

void printLongWithCommas(FILE *f, long long l)
/* Print out a long number with commas at thousands, millions, etc. */
{
char ascii[32];
sprintLongWithCommas(ascii, l);
fprintf(f, "%s", ascii);
}

void sprintWithGreekByte(char *s, int slength, long long size)
/* Numbers formatted with PB, TB, GB, MB, KB, B */
{
char *greek[] = {"B", "KB", "MB", "GB", "TB", "PB"};
int maxGreek = (sizeof(greek)/sizeof(char*))-1;
int i = 0;
long long d = 1;
while (((size/d) >= 1024) && (i != maxGreek))
    {
    ++i;
    d *= 1024;
    }
double result = ((double)size)/d;
safef(s, slength, "%3.*f %s", result < 10 ? 1 : 0, ((double)size)/d, greek[i]);
}

void printWithGreekByte(FILE *f, long long l)
/* Print with formatting in gigabyte, terabyte, etc. */
{
char buf[32];
sprintWithGreekByte(buf, sizeof(buf), l);
fprintf(f, "%s", buf);
}

void sprintWithMetricBaseUnit(char *s, int slength, long long size)
/* Numbers formatted with Pb, Tb, Gb, Mb, kb, bp */
{
char *unit[] = {"bp", "kB", "Mb", "Gb", "Tb", "Pb"};
int maxUnit = (sizeof(unit)/sizeof(char*))-1;
int i = 0;
long long d = 1;
while (((size/d) >= 1000) && (i != maxUnit))
    {
    ++i;
    d *= 1000;
    }
double result = ((double)size)/d;
safef(s, slength, "%3.*f %s", result < 10 ? 1 : 0, ((double)size)/d, unit[i]);
}

void printWithMetricBaseUnit(FILE *f, long long l)
/* Print with formatting in megabase, kilobase, etc. */
{
char buf[32];
sprintWithMetricBaseUnit(buf, sizeof(buf), l);
fprintf(f, "%s", buf);
}

void shuffleArrayOfChars(char *array, int arraySize)
/* Shuffle array of characters of given size given number of times. */
{
char c;
int i, randIx;

/* Randomly permute an array using the method from Cormen, et al */
for (i=0; i<arraySize; ++i)
    {
    randIx = i + (rand() % (arraySize - i));
    c = array[i];
    array[i] = array[randIx];
    array[randIx] = c;
    }
}

void shuffleArrayOfInts(int *array, int arraySize)
/* Shuffle array of ints of given size given number of times. */
{
int c;
int i, randIx;

/* Randomly permute an array using the method from Cormen, et al */
for (i=0; i<arraySize; ++i)
    {
    randIx = i + (rand() % (arraySize - i));
    c = array[i];
    array[i] = array[randIx];
    array[randIx] = c;
    }
}

void shuffleArrayOfPointers(void *pointerArray, int arraySize)
/* Shuffle array of pointers of given size given number of times. */
{
void **array = pointerArray, *pt;
int i, randIx;

/* Randomly permute an array using the method from Cormen, et al */
for (i=0; i<arraySize; ++i)
    {
    randIx = i + (rand() % (arraySize - i));
    pt = array[i];
    array[i] = array[randIx];
    array[randIx] = pt;
    }
}

void shuffleList(void *pList)
/* Randomize order of slList.  Usage:
 *     randomizeList(&list)
 * where list is a pointer to a structure that
 * begins with a next field. */
{
struct slList **pL = (struct slList **)pList;
struct slList *list = *pL;
int count;
count = slCount(list);
if (count > 1)
    {
    struct slList *el;
    struct slList **array;
    int i;
    array = needLargeMem(count * sizeof(*array));
    for (el = list, i=0; el != NULL; el = el->next, i++)
        array[i] = el;
    for (i=0; i<4; ++i)
        shuffleArrayOfPointers(array, count);
    list = NULL;
    for (i=0; i<count; ++i)
        {
        array[i]->next = list;
        list = array[i];
        }
    freeMem(array);
    slReverse(&list);
    *pL = list;
    }
}

void *slListRandomReduce(void *list, double reduceRatio)
/* Reduce list to approximately reduceRatio times original size. Destroys original list. */
{
if (reduceRatio >= 1.0)
    return list;
int threshold = RAND_MAX * reduceRatio;
struct slList *newList = NULL, *next, *el;
for (el = list; el != NULL; el = next)
    {
    next = el->next;
    if (rand() <= threshold)
        {
	slAddHead(&newList, el);
	}
    }
return newList;
}

void *slListRandomSample(void *list, int maxCount)
/* Return a sublist of list with at most maxCount. Destroy list in process */
{
if (list == NULL)
    return list;
int initialCount = slCount(list);
if (initialCount <= maxCount)
    return list;
double reduceRatio = (double)maxCount/initialCount;
if (reduceRatio < 0.9)
    {
    double conservativeReduceRatio = reduceRatio * 1.05;
    list = slListRandomReduce(list, conservativeReduceRatio);
    }
int midCount = slCount(list);
if (midCount <= maxCount)
    return list;
shuffleList(list);
struct slList *lastEl = slElementFromIx(list, maxCount-1);
lastEl->next = NULL;
return list;
}


char *stripCommas(char *position)
/* make a new string with commas stripped out */
{
char *newPos = cloneString(position);
char *nPtr = newPos;

if (position == NULL)
    return NULL;
while((*nPtr = *position++))
    if (*nPtr != ',')
	nPtr++;

return newPos;
}

void dotForUserInit(int dotMod)
/* Set how often dotForUser() outputs a dot. */
{
assert(dotMod > 0);
_dotForUserMod = dotMod;
}

void dotForUser()
/* Write out a dot every _dotForUserMod times this is called. */
{
static int dot = -10;
/* Check to see if dot has been initialized. */
if(dot == - 10)
    dot = _dotForUserMod;

if (--dot <= 0)
    {
    putc('.', stderr);
    fflush(stderr);
    dot = _dotForUserMod;
    }
}

void dotForUserEnd()
/* Write out new line at end of dots for user */
{
putc('\n', stderr);
}

void spaceToUnderbar(char *s)
/* Convert white space to underbar. */
{
char c;
while ((c = *s) != 0)
    {
    if (isspace(c))
        *s = '_';
    ++s;
    }
}

long long currentVmPeak()
/* return value of peak Vm memory usage (if /proc/ business exists) */
{
long long vmPeak = 0;

pid_t pid = getpid();
char temp[256];
safef(temp, sizeof(temp), "/proc/%d/status", (int) pid);
struct lineFile *lf = lineFileMayOpen(temp, TRUE);
if (lf)
    {
    char *line;
    while (lineFileNextReal(lf, &line))
	{	// typical line: 'VmPeak:     62646196 kB'
		// seems to always be kB
	if (stringIn("VmPeak", line))
	    {
	    char *words[3];
	    chopByWhite(line, words, 3);
	    vmPeak = sqlLongLong(words[1]);	// assume always 2nd word
	    break;
	    }
	}
    lineFileClose(&lf);
    }

return vmPeak;
}

void printVmPeak()
/* print to stderr peak Vm memory usage (if /proc/ business exists) */
{
pid_t pid = getpid();
char temp[256];
safef(temp, sizeof(temp), "/proc/%d/status", (int) pid);
struct lineFile *lf = lineFileMayOpen(temp, TRUE);
if (lf)
    {
    char *line;
    while (lineFileNextReal(lf, &line))
	{
	if (stringIn("VmPeak", line))
	    {
	    fprintf(stderr, "# pid=%d: %s\n", pid, line);
	    break;
	    }
	}
    lineFileClose(&lf);
    }
else
    fprintf(stderr, "# printVmPeak: %s - not available\n", temp);
fflush(stderr);
}

boolean nameInCommaList(char *name, char *commaList)
/* Return TRUE if name is in comma separated list. */
{
if (commaList == NULL)
    return FALSE;
int nameLen = strlen(name);
for (;;)
    {
    char c = *commaList;
    if (c == 0)
        return FALSE;
    if (memcmp(name, commaList, nameLen) == 0)
        {
	c = commaList[nameLen];
	if (c == 0 || c == ',')
	    return TRUE;
	}
    commaList = strchr(commaList, ',');
    if (commaList == NULL)
        return FALSE;
    commaList += 1;
    }
}

boolean endsWithWordComma(char *string, char *word)
/* Return TRUE if string ends with word possibly followed by a comma, and the beginning
 * of word within string is the beginning of string or follows a comma. */
{
int stringLen = strlen(string);
int wordLen = strlen(word);
int commaSize = (stringLen > wordLen && string[stringLen-1] == ',') ? 1 : 0;
if (stringLen < wordLen + commaSize)
    return FALSE;
int wordOffset = stringLen - commaSize - wordLen;
if (sameStringN(string + wordOffset, word, wordLen) &&
    (wordOffset == 0 || string[wordOffset-1] == ','))
    return TRUE;
return FALSE;
}

void ensureNamesCaseUnique(struct slName *fieldList)
/* Ensure that there would be no name conflicts in fieldList if all fields were lower-cased. */
{
struct slName *field;
struct hash *hash = hashNew(0);
for (field = fieldList; field != NULL; field = field->next)
    {
    char *s = field->name;
    int len = strlen(s);
    assert(len<512);  // avoid stack overflow
    char lower[len+1];
    strcpy(lower, s);
    strLower(lower);
    char *conflict = hashFindVal(hash, lower);
    if (conflict)
	 {
	 if (sameString(conflict,s))
	     errAbort("Duplicate symbol %s", s);
	 else
	     errAbort("Conflict between symbols with different cases: %s vs %s",
		conflict, s);
	 }
    hashAdd(hash, lower, s);
    }
hashFree(&hash);
}

boolean readAndIgnore(char *fileName)
/* Read a byte from fileName, so its access time is updated. */
{
boolean ret = FALSE;
char buf[256];
FILE *f = fopen(fileName, "r");
if ( f && (fread(buf, 1, 1, f) == 1 ) )
    ret = TRUE;
if (f)
    fclose(f);
return ret;
}

//int get_thread_id() {
///* return some int specific to a thread, copied from https://stackoverflow.com/questions/21091000/how-to-get-thread-id-of-a-pthread-in-linux-c-program */
//#if defined(__linux__)
//    return syscall(SYS_gettid);
//#elif defined(__FreeBSD__)
//    long tid;
//    thr_self(&tid);
//    return (int)tid;
//#elif defined(__NetBSD__)
//    return _lwp_self();
//#elif defined(__OpenBSD__)
//    return getthrid();
//#else
//    return getpid();
//#endif
//}
