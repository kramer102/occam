/* Copyright 2000, Portland State University Systems Science Program.  All Rights Reserved
 */

 #include "ocCore.h"
 #include "ocModelCache.h"
 #include <assert.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <memory.h>
 #include <string.h>

static int hashcode(const char *name, int hashsize)
{
	unsigned int key = 0;
	for (const char *cp = name; *cp; cp++) key = (key << 1) + *cp;
	return key % hashsize;
}

ocModelCache::ocModelCache()
{
	hash = new ocModel*[MODELCACHE_HASHSIZE];
	memset(hash, 0, MODELCACHE_HASHSIZE*sizeof(ocModel*));
}
	
//-- destroy Model cache.  This also deletes all the Models held in the cache.
ocModelCache::~ocModelCache()
{
	ocModel *r1, *r2;
	int i;
	for (i = 0; i < MODELCACHE_HASHSIZE; i++) {
		r1 = hash[i];
		while (r1) {
			r2 = r1->getHashNext();
			delete r1;
			r1 = r2;
		}
	}
	delete hash;
}
	
long ocModelCache::size()
{
  long size = MODELCACHE_HASHSIZE * sizeof(ocModel*);
  ocModel *r1;
  int i;
  for (i = 0; i < MODELCACHE_HASHSIZE; i++) {
    r1 = hash[i];
    while (r1) {
      size += r1->size();
      r1 = r1->getHashNext();
    }
  }
  return size;
}

//-- addModel - put a new Model in the cache. If a matching Model already
//-- exists, an error is returned.
bool ocModelCache::addModel(class ocModel *model)
{
	if (findModel(model->getPrintName()) != NULL) return false;	//error; exists
	int hashindex = hashcode(model->getPrintName(), MODELCACHE_HASHSIZE);
	model->setHashNext(hash[hashindex]);
	hash[hashindex] = model;
	return true;
}

//-- deleteModel - deletes a model from the cache.
//-- returns true if successful, false if not found.
bool ocModelCache::deleteModel(class ocModel *model)
{
	if (model == NULL) return false;
	int hashindex = hashcode(model->getPrintName(), MODELCACHE_HASHSIZE);
	ocModel *rp = hash[hashindex];
	ocModel *prev = NULL;
	while (rp && (rp != model)) {
		prev = rp;
		rp = rp->getHashNext();
	}
	if (rp != NULL) {
		if (rp == hash[hashindex]) {
			hash[hashindex] = rp->getHashNext();
		} else {
			prev->setHashNext(rp->getHashNext());	
		}
//		printf("deleting: %s\n", model->getPrintName());
		delete rp;
		return true;
	} else {
		return false;
	}
}

//-- findModel - find a Model in the cache.  Null is returned if the given
//-- Model doesn't exist.
class ocModel *ocModelCache::findModel(const char *name)
{
	int hashindex = hashcode(name, MODELCACHE_HASHSIZE);
	ocModel *rp = hash[hashindex];
	while (rp && strcmp(name, rp->getPrintName()) != 0) rp = rp->getHashNext();
	return rp;	// either NULL, or the matching one
}

//-- dump - print out all Models in the cache
void ocModelCache::dump()
{
	printf("\nDump ModelCache:\n");
	for (int i = 0; i < MODELCACHE_HASHSIZE; i++) {
		if (hash[i]) {
			printf ("hash chain [%d]:\n", i);
			for (ocModel *model = hash[i]; model; model = model->getHashNext()) {
				model->dump();
			}
		}
	}
}


