#pragma once
#include <mutex>
#include <iostream>

using namespace std;
/// TODO: complete this implementation of a thread-safe (concurrent) sorted
/// linked list of integers
class clist {
private:
  /// a node consists of a value and a pointer to another node
  struct node
  {
    int value;
    node* next;
  };
  
  /// The head of the list is referenced by this pointer
  node* head;
  
  std::mutex mtx;   // define lock
  
  
public:
  /// insert *key* into the linked list if it doesn't already exist; return
  /// true if the key was added successfully.
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
  // insert
  bool insert(int key){
    if(!mtx.try_lock()){ // lock if not locked by other thread
      return false; // fail to lock
      //cout << "acquiring lock failed"<<endl;
    }
    bool ifinsert = myinsert(key);
    mtx.unlock();  // unlock
    //cout << "insert:" << key << endl;
    //printlist();
    return ifinsert;
  }
  /// remove *key* from the list if it was present; return true if the key
  /// was removed successfully.
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
  // remove
  bool remove(int key){
    if(!mtx.try_lock()){
       return false;
    }
    bool result = myremove(key);
    //cout << "remove: "<< key<< endl;;
    //printlist();
    mtx.unlock();
    return result;
  }
  
  /// return true if *key* is present in the list, false otherwise
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

  bool lookup(int key){
    if(!mtx.try_lock()) return false;
    bool result = mylookup(key);
    //cout << "look up :" <<key<<endl;
    mtx.unlock();
    return result;
  }

  void printlist(){
    if (!head) return;
    cout << "In List: ";
    node * current = head;
    cout<< current -> value;
    while(current->next){
      current = current -> next;
      cout << "->" << current->value;
    }
    cout << "\n";
  }
  
  /// constructor code goes here
  clist(int): head(NULL) { }
};
