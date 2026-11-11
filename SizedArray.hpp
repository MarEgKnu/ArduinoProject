#pragma once
// Represents an array with a List-like API, in which only the first _sizeUsed array slots are utilized, up to MaxSize
template<typename T, uint8_t MaxSize>
class SizedArray {
public:
	uint8_t _sizeUsed = 0; // How much size of the array is actually utilized?
	T _data[MaxSize] = {}; // The underlying array

//public:
	// Add item to the front of the array. Muust check if there is available size left before calling
	void PushBack(const T& item) {
			_data[_sizeUsed] = item;
			_sizeUsed++;
			return true;
	}
	// Remove item from front of array, and return it. Must check that size() is not zero before calling
	T PopBack() {
		_sizeUsed--;
		T out = _data[_sizeUsed];
		_data[_sizeUsed] = T{};
		return out;
		
	}
	// Gets the currently used size
	uint8_t Size() const {
		return _sizeUsed;
	}
	// Gets if it is currently empty
	bool IsEmpty() const {
		return size() == 0;
	}
	// Resizes the array to the specified new size
	void Resize(uint8_t newSize) {
		// Clear excess elements if the container is resized downwards
		if(newSize < _sizeUsed) {
			for(uint8_t i = newSize;i < _sizeUsed;i++) {
				_data[i] = T{};
			}
		}
		_sizeUsed = newSize;

	}
	// Indexes the array
	T& operator[](uint8_t index) {
		return _data[index];
	}
	// Indexes the array, but const
	const T& operator[](uint8_t index) const {
		return _data[index];
	}
	
};