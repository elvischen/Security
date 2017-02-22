#pragma once

/// TODO: complete this implementation of a thread-safe (concurrent) hash
///       table of integers, implemented as an array of linked lists.  In
///       this implementation, each list should have a "sentinel" node that
///       contains the lock, so we can't just reuse the clist implementation
class shash {
  struct node{
    int value;
    node * next;
  };

  struct sentinel{
    node * head;
    std::mutex mtx;
  };

  int num;
  sentinel * bucket;
    
public:
  bool myinsert(int key,node * head) {
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

  bool myremove(int key, node * head) {
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

  bool mylookup(int key, node * head) {
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
    /// insert *key* into the appropriate linked list if it doesn't already
    /// exist; return true if the key was added successfully.
    bool insert(int key) {
      bucket[key%num].mtx.lock();
      bool res = myinsert(key,bucket[key%num].head);
      bucket[key%num].mtx.unlock();
      return res;
    }
    /// remove *key* from the appropriate list if it was present; return true
    /// if the key was removed successfully.
    bool remove(int key) {
      bucket[key%num].mtx.lock();
      bool res = myremove(key,bucket[key%num].head);
      bucket[key%num].mtx.unlock();
      return res;
    }
    /// return true if *key* is present in the appropriate list, false
    /// otherwise
    bool lookup(int key) {
      bucket[key%num].mtx.lock();
      bool res = mylookup(key,bucket[key%num].head);
      bucket[key%num].mtx.unlock();
      return res;
    }
    /// constructor code goes here
    shash(unsigned _buckets) {
      num = _buckets;
      bucket = new sentinel[num];
    }
};
