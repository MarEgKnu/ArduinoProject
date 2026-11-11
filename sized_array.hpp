
template<typename T, uint32_t max_size>
class sized_array {
	uint32_t size_used = 0;
	T data[max_size] = {};
	
	bool push_back(const T& item) {
		if(size < max_size) {	
			data[size] = item;
			size++;
			return true;
		}
		else {
			return false;
		}
	}
	bool pop_back() {
		if(size_used == 0) {
			return false;
		}
		else {
			size_used--;
			data[size_used] = T{};
			return true;
		}
	}
	bool remove_at(uint32_t index) {
		
	}
	
}