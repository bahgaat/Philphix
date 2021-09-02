/*
 * Include the provided hash table library.
 */
#include "hashtable.h"

#include <math.h>

/*
 * Include the header file.
 */
#include "philphix.h"

/*
 * Standard IO and file routines.
 */
#include <stdio.h>

/*
 * General utility routines (including malloc()).
 */
#include <stdlib.h>

/*
 * Character utility routines.
 */
#include <ctype.h>

/*
 * String utility routines.
 */
#include <string.h>

#include <errno.h>

/*
 * This hash table stores the dictionary.
 */
HashTable *dictionary;

/*
 * The MAIN routine.  You can safely print debugging information
 * to standard error (stderr) as shown and it will be ignored in
 * the grading process.
 */
int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Specify a dictionary\n");
    return 1;
  }
  /*
   * Allocate a hash table to store the dictionary.
   */
  fprintf(stderr, "Creating hashtable\n");
  dictionary = createHashTable(0x61C, &stringHash, &stringEquals);

  fprintf(stderr, "Loading dictionary %s\n", argv[1]);
  readDictionary(argv[1]);
  fprintf(stderr, "Processing stdin\n");
  processInput();
  freeTable(dictionary);
  /*
   * The MAIN function in C should always return 0 as a way of telling
   * whatever program invoked this that everything went OK.
   */
  return 0;
}

/*
 * This should hash a string to a bucket index.  void *s can be safely cast
 * to a char * (null terminated string)
 */
unsigned int stringHash(void *s) {
  //fprintf(stderr, "need to implement stringHash\n");
  unsigned int hash = 0;
  char *string = (char*) s;
  for (int i = 0; i < strlen(string); i++) {
    hash = 31 * hash + string[i];
  }
  unsigned int index = hash % 0x61C; //(Here has to be the size of hashtable)
  return index;
}

/*
 * This should return a nonzero value if the two strings are identical
 * (case sensitive comparison) and 0 otherwise.
 */
int stringEquals(void *s1, void *s2) {
  int isDifferent = strcmp((char *) s1, (char *) s2);
  if (isDifferent == 0) {
    return 1;
  } else {
    return 0;
  }
}

/*
 * This function should read in every word and replacement from the dictionary
 * and store it in the hash table.  You should first open the file specified,
 * then read the words one at a time and insert them into the dictionary.
 * Once the file is read in completely, return.  You will need to allocate
 * (using malloc()) space for each word.  As described in the spec, you
 * can initially assume that no word is longer than 60 characters.  However,
 * for the final bit of your grade, you cannot assumed that words have a bounded
 * length.  You CANNOT assume that the specified file exists.  If the file does
 * NOT exist, you should print some message to standard error and call exit(61)
 * to cleanly exit the program.
 */
 void resizeIfNeeded(int counter, char **originalKeyString, char **originalValueString, int findKey,
                    int *sizeOFString, char **string);
char* findTheReplacableWord(char* string);
void checkMalloc(char* string);

void readDictionary(char *dictName) {
  //Step1 : open the file.
  FILE *fp;
  fp = fopen(dictName, "r");
  if (fp == NULL) {
    fprintf(stderr, "Specify a dictionary\n");
    exit(61);
  }
  //Step2 : read the words one at a time and insert into dictionary.
  char ch;
  int sizeOFString = 61;
  char *originalKeyString = (char*) malloc(sizeOFString * sizeof(char)); /*+1 for '\0' character */
  char *originalValueString = (char*) malloc(sizeOFString * sizeof(char)); /*+1 for '\0' character */
  checkMalloc(originalKeyString);
  checkMalloc(originalValueString);
  char *string = originalKeyString;
  int findKey = 0;
  unsigned int counter = 0;
  while ((ch = fgetc(fp))!= EOF) {
    resizeIfNeeded(counter, &originalKeyString, &originalValueString, findKey, &sizeOFString, &string);
    if (isspace(ch) && findKey == 0) { // the word has ended
      *string = '\0'; //it has to be leading zero
      findKey = 1;
      string = originalValueString;
      counter = 0;
      sizeOFString = 61;
    } else if (ch == '\n') {
      *string = '\0';
      insertData(dictionary, (void *) originalKeyString, (void *) originalValueString);
      originalKeyString = (char*) malloc(61*sizeof(char));
      originalValueString = (char*) malloc(61*sizeof(char));
      checkMalloc(originalKeyString);
      checkMalloc(originalValueString);
      string = originalKeyString;
      sizeOFString = 61;
      findKey = 0;
      counter = 0;
    } else if (!isspace(ch)) {
      *string = ch;
      string ++;
      counter ++;
    }
  }
  *string = '\0';
  insertData(dictionary, (void *) originalKeyString, (void *) originalValueString);
  fclose(fp);
}

/*
 * This should process standard input (stdin) and perform replacements as
 * described by the replacement set then print either the original text or
 * the replacement to standard output (stdout) as specified in the spec (e.g.,
 * if a replacement set of `taest test\n` was used and the string "this is
 * a taest of  this-proGram" was given to stdin, the output to stdout should be
 * "this is a test of  this-proGram").  All words should be checked
 * against the replacement set as they are input, then with all but the first
 * letter converted to lowercase, and finally with all letters converted
 * to lowercase.  Only if all 3 cases are not in the replacement set should
 * it report the original word.
 *
 * Since we care about preserving whitespace and pass through all non alphabet
 * characters untouched, scanf() is probably insufficent (since it only considers
 * whitespace as breaking strings), meaning you will probably have
 * to get characters from stdin one at a time.
 *
 * Do note that even under the initial assumption that no word is longer than 60
 * characters, you may still encounter strings of non-alphabetic characters (e.g.,
 * numbers and punctuation) which are longer than 60 characters. Again, for the
 * final bit of your grade, you cannot assume words have a bounded length.
 */
void processInput() {
  char ch;
  int sizeOFString = 61;
  char *string = (char*) malloc(sizeOFString * sizeof(char));
  checkMalloc(string);
  int counter = 0;
  int findString = 0;
  char *helperString = string;
  while ((ch = getchar())!= EOF) {
    resizeIfNeeded(counter, &string, &string, 0, &sizeOFString, &helperString);
    if (isalnum(ch)) {
      *helperString = ch;
      findString = 0;
      counter ++;
      helperString ++;
    } else if (findString == 0) {
      *helperString = '\0';
      findString = 1;
      char *replacableWord = findTheReplacableWord(string);
      if (replacableWord != NULL) {
        printf("%s", replacableWord);
      } else {
        printf("%s", string);
      }
      putchar(ch);
      free(string);
      counter = 0;
      sizeOFString = 61;
      string = (char*) malloc(sizeOFString * sizeof(char));
      checkMalloc(string);
      helperString = string;
    } else {
      putchar(ch);
    }
  }
  free(string);
}

void resizeIfNeeded(int counter, char **originalKeyString, char **originalValueString, int findKey,
                   int *sizeOFString, char **string) {
  if (counter >= *sizeOFString - 1) {
    *sizeOFString = *sizeOFString * 2;
    if (findKey == 1) {
      *originalValueString = (char *) realloc(*originalValueString, *sizeOFString);
      *string = *originalValueString + counter;
    } else {
      *originalKeyString = (char *) realloc(*originalKeyString, *sizeOFString);
      *string = *originalKeyString + counter;
    }
  }
}

char* findTheReplacableWord(char* string) {
  char cpyString[strlen(string) + 1];
  strcpy(cpyString, string);
  char* replacableWord;// = (char*) findData(dictionary, string);
  for (int i = 0; i < 3; i++) {
    if (i == 1) {
      for (int j = 1; j < strlen(cpyString); j++) {
        cpyString[j] = tolower(cpyString[j]);
      }
    } else if (i == 2) {
        *cpyString = tolower(*cpyString);
    }
    replacableWord = (char*) findData(dictionary, cpyString);
    if (replacableWord != NULL) {
      return replacableWord;
    }
  }
  return replacableWord;
}

void checkMalloc(char* string) {
  if (NULL == string) {
    fprintf(stderr, "malloc failed \n");
    exit(1);
  }
}
