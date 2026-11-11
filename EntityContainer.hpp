#pragma once
#include "SizedArray.hpp"

constexpr int16_t INVALID_INDEX = -1;

// A class holding a
template<typename T, int8_t MaxSize>
class EntityContainer {
public:
  // Wrapper type to hold the item, aswell as a bool which determines if this slot is free and should be skipped, or if it is a real item
  struct ItemWrapper {
    bool isFree = true;
    T item = T{};
  };
  SizedArray<ItemWrapper, MaxSize> _items = {}; // Holds the entity object itself, aswell as a boolean determinig if the slot is free, and thus should be ignored
  SizedArray<uint8_t, MaxSize> _freeQueue = {}; // Holds a FIFO(first-in-first-out) queue of free slot indexes to be filled first
//public:
  // default constructor. Init _freeQueue so all free indicies are present in order
  EntityContainer() {
    for(int16_t i = MaxSize - 1;i >= 0;i--) {
      Serial.println(i);
      _freeQueue.PushBack(i);
    }
  }
  // Gets the size used of the underlying array
  uint8_t Size() const {
    return _items.Size();
  }

  // Adds an item to the container, and returns the index to it. Returns -1 if the item could not be added due to lack of space
  int16_t Add(const T& item) {
    // Check if the container can hold anymore things
    if(_items.Size() == MaxSize) {
      return INVALID_INDEX;
    }
    // Get the next free index
    uint8_t indx = _freeQueue.PopBack();
    ItemWrapper toAdd;
    toAdd.isFree = false;
    toAdd.item = item;
    // If the next free index is one above the current container size, push it back
    if(indx ==_items.Size()) {
      _items.PushBack(toAdd);
    }
    // otherwise, add it at given index
    _items[indx] = toAdd;
    return int16_t(indx);
  }
  // Returns a bool representing if the index is valid or not
  bool IndexValid(int16_t index) const {
    // Check it is not negative
    if(index < 0) {
      return false;
    }
    uint16_t unsigned_index = uint16_t(index);
    // If the index within the valid range, check if the slot is considered free.
    if(unsigned_index < _items.Size()) {
      return !_items[unsigned_index].isFree;
    }
    // Otherwise, its out of range and it is invalid
    else {
      return false;
    }
  }
  // Attempts to delete the given index. Returns true if it was sucessfully deleted, and false if the index is invalid and was not deleted
  bool Delete(int16_t index) {
    if(IndexValid(index)) {
      uint16_t unsigned_index = uint16_t(index);
       // If the index is valid, push the index into the queue of free indicies, then pop the array back one element if it is the front element, otherwise set the slot to be free
      _freeQueue.PushBack(unsigned_index);
      if(unsigned_index == _items.Size() - 1) {
        _items.PopBack();
      }
      else {
        _items[unsigned_index].item = {};
        _items[unsigned_index].isFree = true;
      }
      return true;
    }
    else {
      return false;
    }
  }


  // Returns a reference to the element at the given index. Must call IndexValid prior to check if the index is valid
  T& Get(uint8_t index) {
    return _items[index].item;
  }


};