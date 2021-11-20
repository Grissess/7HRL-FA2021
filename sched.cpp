#include <algorithm>

#include "sched.h"

using namespace std;

template<class T, class K>
void SortedVec<T,K>::add(T elem) {
	if(content.empty()) {
		content.push_back(elem);
		return;
	}

	auto idx = lower_bound(content.begin(), content.end(), elem, comp);
	content.insert(idx, elem);
}

template<class T, class K>
bool SortedVec<T,K>::remove(T elem) {
	if(binary_search(content.begin(), content.end(), elem, comp)) {
		auto low = lower_bound(content.begin(), content.end(), elem, comp);
		auto high = upper_bound(content.begin(), content.end(), elem, comp);
		content.erase(low, high);
		return true;
	}
	return false;
}

void Sched::add(Event *e) {
	e->initTimebase(time);
	events.add(e);
}

bool Sched::step() {
	if(events.content.empty()) {
		return false;
	}

	auto idx = min_element(
			events.content.begin(),
			events.content.end(),
			KeyComparator<Event *, double(*)(Event *)>(Event::time_key)
	);

	Event *evp = *idx;
	time = evp->nextTime();
	bool remove = evp->perform(time);
	if(remove) {
		events.content.erase(idx);
	}
	return true;
}
