#ifndef ENTROPY_H
#define ENTROPY_H

#include <random>

using namespace std;

extern knuth_b entropy;

template<class I>
class Roll {
	public:
		virtual Roll *root() { return this; }
		virtual I get() = 0;
		virtual I operator()() { return get(); }
		virtual double expected() = 0;
};

template<class I>
class Uniform: public Roll<I> {
	protected:
		I base, range;
		uniform_int_distribution<I> dist;

		void makeDistr() { dist = uniform_int_distribution<I>(0, range); }

	public:
		Uniform(I b, I r): base(b), range(r) { makeDistr(); }

		I getBase() { return base; }
		I getRange() { return range; }
		I get() { return base + dist(entropy); }
		void setBase(I b) { base = b; }
		void setRange(I r) { range = r; makeDistr(); }
		void set(I b, I r) { setBase(b); setRange(r); }
		double expected() { return (base + range) / 2.0; }
};

template<class I>
class Linear: public Roll<I> {
	public:
		I bias, mul;
		Roll<I> *inner;

		Linear(Roll<I> *i, I b = 0, I m = 1): inner(i), bias(b), mul(m) {}
		I get() { return bias + mul * inner->get(); }
		Roll<I> *root() { return inner->root(); }
		double expected() { return bias + mul * inner->expected(); }
};

#endif
