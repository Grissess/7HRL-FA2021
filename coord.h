#ifndef COORD_H
#define COORD_H

#include <algorithm>
#include <vector>

using namespace std;

template<class N>
struct V2 {
	N x;
	N y;

	V2(): x(0), y(0) {}
	V2(N a): x(a), y(a) {}
	V2(N a, N b): x(a), y(b) {}

#define defer(op)\
	V2<N> operator op(const V2<N>& rhs) { return V2<N>(x op rhs.x, y op rhs.y); } \
	V2<N> &operator op##=(const V2<N>& rhs) { x op##= rhs.x; y op##= rhs.y; return *this; }

	defer(+)
	defer(-)
	defer(*)
	defer(/)
	defer(%)

	V2<N> min(const V2<N>& rhs) { return V2<N>(min(x, rhs.x), min(y, rhs.y)); }
	V2<N> max(const V2<N>& rhs) { return V2<N>(max(x, rhs.x), max(y, rhs.y)); }
#undef
};

template<class N>
class R2 {
	public:
		using V = V2<N>;
	protected:
		V orig, sz;

	public:
		R2(V o, V s): orig(o), sz(s) {}

		static R2<N> origSize(V o, V s) { return R2(o, s); }
		static R2<N> origOpp(V org, V opp) { return R2(org, opp - org); }

		V origin() { return orig; }
		V size() { return sz; }
		V opposite() { return orig + sz; }
		void normalize() {
			if(sx.x < 0) {
				sz.x = -sz.x;
				orig.x -= sz.x;
			}
			if(sz.y < 0) {
				sz.y = -sz.y;
				orig.y -= sz.y;
			}
		}
};

template<class T>
class Grid {
	public:
		using index = int;
		using V = V2<index>;
		using R = R2<index>;

	protected:
		vector<T> data;
		R rect;

	public:
		Grid(R r): rect(r) { clear(); }
		Grid(V size): rect(R(V(), size)) { clear(); }

		void clear() {
			V s = rect.size();
			data.assign(s.x * s.y, T());
		}

		T *operator[](V idx) {
			size_t i = idx.y*rect.size().x + idx.x;
			if(i >= data.size()) return NULL;
			return &data[i];
		}

		T *operator[](index x, index y) { return operator[](V(x, y)); }
};

#endif
