#include "hashtable.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * This creates a new hash table of the specified size and with
 * the given hash function and comparison function.
 */
HashTable *createHashTable(int size, unsigned int (*hashFunction)(void *),  // hasFunction is apointer to a function that returns usigned int.
                           int (*equalFunction)(void *, void *)) {  // equalFunction is apointer to a function that returns int
  int i = 0;
  HashTable* newTable = (HashTable*) malloc(sizeof(HashTable));
  if (NULL == newTable) {
    fprintf(stderr, "malloc failed \n");
    exit(1);
  }
  newTable->size = size;
  newTable->data = malloc(sizeof(HashBucket) * size);
  if (NULL == newTable->data) {
    fprintf(stderr, "malloc failed \n");
    exit(1);
  }
  for (i = 0; i < size; i++) {
    newTable->data[i] = NULL;
  }
  newTable->hashFunction = hashFunction;
  newTable->equalFunction = equalFunction;
  return newTable;
}

/*
 * This inserts a key/data pair into a hash table.  To use this
 * to store strings, simply cast the char * to a void * (e.g., to store
 * the string referred to by the declaration char *string, you would
 * call insertData(someHashTable, (void *) string, (void *) string).
 */
void insertData(HashTable *table, void *key, void *data) {
  // 1. Find the right hash bucket location with table->hashFunction.
  unsigned int HashBucketLocation = table->hashFunction(key); // this will return to me an index from the array of hashBuckets
  // 2. Allocate a new hash bucket struct.
  HashBucket* newBucket = (HashBucket*) malloc(sizeof(HashBucket));
  if (NULL == newBucket) {
    fprintf(stderr, "malloc failed \n");
    exit(1);
  }
  newBucket->key = key;
  newBucket->data = data;
  newBucket->next = NULL;
  // 3. Append to the linked list or create it if it does not yet exist.
  HashBucket* buckets = table->data[HashBucketLocation];
  if (buckets == NULL) {
    table->data[HashBucketLocation] = newBucket;
  } else {
    while (buckets->next != NULL) {
      buckets = buckets -> next;
    }
    buckets->next = newBucket;
  }
}

/*
 * This returns the corresponding data for a given key.
 * It returns NULL if the key is not found.
 */
void *findData(HashTable *table, void *key) {
  // 1. Find the right hash bucket with table->hashFunction.
  unsigned int HashBucketLocation = table->hashFunction(key);
  if (table->data == NULL) {
    return NULL;
  }
  HashBucket* bucket = table->data[HashBucketLocation];
  // 2. Walk the linked list and check for equality with table->equalFunction.
  if (bucket == NULL) {
    return NULL;
  }
  while (bucket != NULL) {
    if (table->equalFunction(key, bucket->key) != 0) {
      return bucket->data;
    }
    bucket = bucket->next;
  }
  return NULL;
}

void freeTable(HashTable *table) {
  HashBucket** hashBuckets = table->data;
  for (int i = 0; i < table->size; i++) {
    HashBucket* bucket = hashBuckets[i];
    while (bucket != NULL) {
      free(bucket->key);
      free(bucket->data);
      HashBucket* oldBucket = bucket;
      bucket = bucket->next;
      free(oldBucket);
    }
  }
  free(hashBuckets);
  free(table);
}
