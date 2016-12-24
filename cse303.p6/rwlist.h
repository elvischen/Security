#pragma once
#include <pthread.h>
/// TODO: complete this implementation of a thread-safe (concurrent) sorted
/// linked list of integers, which should use readers/writer locking.
class rwlist {
  /// a node consists of a value and a pointer to another node
  struct node
  {
      int value;
      node* next;
  };
  
  /// The head of the list is referenced by this pointer
  node* head;

  pthread_rwlock_t rwlock;
  
public:
  bool myinsert(int key) {
    if(!head){  // if head NULL insert into head
      head = new node();
      head -> value = key;
    } else {
      node * current = head;
      node * new_node = new node();
      new_node -> value = key;
      if(current->value > key){  // inserted key is less than head, be head
	head = new_node;
	new_node -> next = current;
	return true;
      } else if (current->value == key) {
	return false;  // already exist
      } else { // key > head
	while(current -> next && current->next->value <= key){ 
	  if(current->next->value == key){
	    return false; //already exist
	  } else {  // key > next-val
	    current = current -> next;
	  }
	} // now key < next-val
	new_node -> next = current -> next;
	current -> next = new_node; // insert after current
      }
    }
    return true;
  }

  bool myremove(int key) {
    node * current = head;
    node * last;
    if(!current){  // if head is NULL
      return false;
    }
    if(current -> value == key){ // if head is the key
      head = current -> next;
      delete(current);
      return true;
    }
    last = current;
    current = current -> next;
    while(current){
      if(current -> value == key){
	last -> next = current -> next;
	delete(current);
	return true;
      }
      last = current;
      current = current -> next;
    }
    return false;
  }

  bool mylookup(int key) {
    node * current = head;
    if(!head){ // head null
      return false;
    }
    while(current){
      if(current -> value == key) return true;
      current = current -> next;
    }
    return false;
  }
    /// insert *key* into the linked list if it doesn't already exist; return
    /// true if the key was added successfully.
    bool insert(int key) {
      while(pthread_rwlock_trywrlock(&rwlock)){
	// lock
      }
      bool res = myinsert(key);
      pthread_rwlock_unlock(&rwlock);
      return res;
    }
    /// remove *key* from the list if it was present; return true if the key
    /// was removed successfully.
    bool remove(int key) {
      while(pthread_rwlock_trywrlock(&rwlock)){
      }
      bool res = myremove(key);
      pthread_rwlock_unlock(&rwlock);
      return res;
    }
    /// return true if *key* is present in the list, false otherwise
    bool lookup(int key) {
      while(pthread_rwlock_tryrdlock(&rwlock)){
      }
      bool res = mylookup(key);
      pthread_rwlock_unlock(&rwlock);
      return res;
    }
    /// constructor code goes here
    rwlist(int): head(NULL) {
      rwlock = PTHREAD_RWLOCK_INITIALIZER;
    }
};
