#ifndef SCHED_H
#define SCHED_H

#include <vector>
#include <functional>

using namespace std;

class Event {
	public:
		static double time_key(Event *e) { return e->nextTime(); }
		static int priority_key(Event *e) { return e->priority(); }

		virtual void initTimebase(double origin) = 0;
		virtual double nextTime() = 0;
		virtual int priority() { return 0; }
		virtual bool perform(double time) = 0;
};

class PeriodicEvent: public Event {
	protected:
		double next;

	public:
		virtual double period() = 0;
		virtual double delay() = 0;

		virtual void initTimebase(double origin) { next = origin + delay(); }
		virtual double nextTime() { return next; }
		virtual bool perform(double time) { next += period(); return false; }
};

template<class T, class K>
class KeyComparator {
	protected:
		K key;

	public:
		KeyComparator(K k): key(k) {}
		bool operator()(T a, T b) { return key(a) < key(b); }
};

template<class T, class K>
class SortedVec {
	protected:
		KeyComparator<T,K> comp;
	public:
		SortedVec(K k): comp(k) {}

		vector<T> content;

		void add(T elem);
		bool remove(T elem);
};

class Sched {
	public:
		SortedVec<Event *, int(*)(Event *)> events;
		double time;

		Sched(): time(0.0), events(Event::priority_key) {}

		void add(Event *e);
		bool step();
		void run() { while(step()); }
		void runUntil(double t) { while(time < t && step()); }
};

#endif
