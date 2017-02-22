#include "clist.h"

#pragma once

/// TODO: complete this implementation of a thread-safe (concurrent) hash
///       table of integers, implemented as an array of linked lists.
class chash {
  /// The bucket list
  clist **buckets;
  int num;
public:
  
    /// insert *key* into the appropriate linked list if it doesn't already
    /// exist; return true if the key was added successfully.
    bool insert(int key) {
      bool res = buckets[key%num]->insert(key);
      return res;
    }
    /// remove *key* from the appropriate list if it was present; return true
    /// if the key was removed successfully.
    bool remove(int key) {
      bool res = buckets[key%num] -> remove(key);
      return res;
    }
    /// return true if *key* is present in the appropriate list, false
    /// otherwise
    bool lookup(int key) {
      bool res = buckets[key%num] -> lookup(key);
      return res;
    }
    /// constructor code goes here
    chash(unsigned _buckets) {
      num = _buckets;
      buckets = (clist **)malloc(sizeof(clist)*num);
      for(int i=0;i<num;i++){
	clist *row = new clist(num);
	buckets[i] = row;
      }
    }
};
