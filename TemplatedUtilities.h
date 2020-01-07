#pragma once
#include <algorithm>
#include <utility>
#include <type_traits>
#include <vector>
#include <set>
#include <array>
#include <functional>

//////////////////////////////////////////////////////////////////
// Utility methods
//////////////////////////////////////////////////////////////////
template <typename InputIterator1, typename InputIterator2>
bool set_intersects(InputIterator1 first1, InputIterator1 last1,
	InputIterator2 first2, InputIterator2 last2)
{
	while (first1 != last1 && first2 != last2)
	{
		if (*first1 < *first2) ++first1;
		else if (*first2 < *first1) ++first2;
		else return true;
	}
	return false;
}

template <typename T>
constexpr void GrowToEncompass(T& min, T& max, T val)
{
	min = std::min(min, val);
	max = std::max(max, val);
}

template <typename T>
constexpr bool Within(T min, T val, T max)
{
	return min <= val && val <= max;
}

///////////////////////////////////////////////////////////////////
// Coordinate systems
///////////////////////////////////////////////////////////////////
template<typename T>
struct Coord
{
	constexpr Coord(T X = 0, T Y = 0) : x(X), y(Y) {}
	constexpr bool operator <  (const Coord& p) const { if (x < p.x) return true; else if (p.x < x) return false; else return y < p.y; }
	constexpr Coord operator + (const Coord& p) const { return Coord(x + p.x, y + p.y); }
	constexpr Coord& operator+=(const Coord& p) { x += p.x; y += p.y; return *this; }
	constexpr Coord operator - (const Coord& p) const { return Coord(x - p.x, y - p.y); }
	constexpr Coord& operator-=(const Coord& p) { x -= p.x; y -= p.y; }
	constexpr bool operator==(const Coord& p) const { return x == p.x && y == p.y; }
	constexpr bool operator!=(const Coord& p) const { return x != p.x || y != p.y; }
	T x, y;
};

template<typename T>
struct Coord3D
{
	constexpr Coord3D(T X = 0, T Y = 0, T Z = 0) : x(X), y(Y), z(Z) {}
	constexpr bool operator <  (const Coord3D& p) const { if (x < p.x) return true; if (p.x < x) return false; if (y < p.y) return true; if (p.y < y) return false; return z < p.z; }
	constexpr Coord3D operator + (const Coord3D& p) const { return Coord3D(x + p.x, y + p.y, z + p.z); }
	constexpr Coord3D& operator+=(const Coord3D& p) { x += p.x; y += p.y; z += p.z; return *this; }
	constexpr Coord3D operator - (const Coord3D& p) { return Coord3D(x - p.x, y - p.y, z - p.z); };
	constexpr Coord3D& operator-=(const Coord3D& p) { x -= p.x; y -= p.y; z -= p.z; return *this; };
	constexpr bool operator==(const Coord3D& p) const { return x == p.x && y == p.y && z == p.z; }
	constexpr bool operator!=(const Coord3D& p) const { return x != p.x || y != p.y || z != p.z; }
	constexpr Coord<T> Flatten() const { return Coord<T>(x, y); }
	T x, y, z;
};

template<typename T>
T ManhattanDistance(const Coord<T>& c1, const Coord<T>& c2)
{
	if constexpr (std::is_signed_v<T>)
		return std::abs(c1.x - c2.x) + std::abs(c1.y - c2.y);
	else
		return static_cast<T>(std::abs(static_cast<std::make_signed_t<T>>(c1.x) - static_cast<std::make_signed_t<T>>(c2.x)) +
			std::abs(static_cast<std::make_signed_t<T>>(c1.y) - static_cast<std::make_signed_t<T>>(c2.y)));
}

using Point = Coord<int>;
using Point3D = Coord3D<int>;

constexpr char coords[] = { 'N', 'E', 'S', 'W' };
constexpr Point directions[] = { Point(1, 0), Point(0, 1), Point(-1, 0), Point(0,-1) };
constexpr Point3D flatDirections[] = { Point3D(1, 0, 0), Point3D(0, 1, 0), Point3D(-1, 0, 0), Point3D(0, -1, 0) };

template<typename T>
constexpr Coord<T> ReverseDirection(const Coord<T>& p)
{
	return Coord<T>(-p.x, -p.y);
}

template <typename T>
struct Boundaries
{
	constexpr Boundaries() : minX(0), minY(0), maxX(0), maxY(0) {}
	constexpr Boundaries(const Coord<T>& c) : minX(c.x), minY(c.y), maxX(c.x), maxY(c.y) {}
	constexpr const Boundaries& operator+=(const Boundaries& b) {
		minX = std::min(minX, b.minX);
		minY = std::min(minY, b.minY);
		maxX = std::max(maxX, b.maxX);
		maxY = std::max(maxY, b.maxY);
		return *this;
	}
	constexpr const Boundaries& operator+=(const Coord<T>& p) { GrowToEncompass(minX, maxX, p.x); GrowToEncompass(minY, maxY, p.y); return *this; }
	constexpr void Translate(const Coord<T>& p) { minX += p.x; maxX += p.x; minY += p.y; maxY += p.y; }
	constexpr bool Inside(const Coord<T>& p) const { return Within(minX, p.x, maxX) && Within(minY, p.y, maxY); }
	T minX, minY, maxX, maxY;
};

using Bounds = Boundaries<int>;

/////////////////////////////////////////////////////////////////
// Modulo mathematics
/////////////////////////////////////////////////////////////////
// From https://www.geeksforgeeks.org/multiply-large-integers-under-large-modulo/
template <typename T>
constexpr T ModuloMul(T a, T b, T mod)
{
	T result = 0;
	a %= mod;
	while (b)
	{
		if (b & 1)
			result = (result + a) % mod;
		a = (2 * a) % mod;
		b /= 2;
	}
	return result;
}

// From https://stackoverflow.com/questions/8496182/calculating-powa-b-mod-n
// Primality testing approach
// Modified to avoid multiplication overflow
template <typename T>
constexpr T ModuloExp(T base, T exp, T mod)
{
	T result = 1;
	while (exp)
	{
		if (exp & 1)
			result = ModuloMul(result, base, mod);
		base = ModuloMul(base, base, mod);
		exp /= 2;
	}
	return result % mod;
}

// From https://www.rookieslab.com/posts/how-to-find-multiplicative-inverse-of-a-number-modulo-m-in-python-cpp
template <typename T>
constexpr T ModuloInvMul(T A, T M) {
	// Assumes that M is a prime number
	// Returns multiplicative modulo inverse of A under M
	return ModuloExp(A, M - 2, M);
}

template<typename T>
constexpr std::array<T, 3> ExtendedEuclidGCD(T a, T b) {
	// Returns a vector `result` of size 3 where:
	// Referring to the equation ax + by = gcd(a, b)
	//     result[0] is gcd(a, b)
	//     result[1] is x
	//     result[2] is y 

	T s = 0, old_s = 1;
	T t = 1, old_t = 0;
	T r = b, old_r = a;

	while (r != 0) {
		T quotient = old_r / r;
		// We are overriding the value of r, before that we store it's current
		// value in temp variable, later we assign it to old_r
		T temp = r;
		r = old_r - quotient * r;
		old_r = temp;

		// We treat s and t in the same manner we treated r
		temp = s;
		s = old_s - quotient * s;
		old_s = temp;

		temp = t;
		t = old_t - quotient * t;
		old_t = temp;
	}
	std::array<T, 3> result{ old_r, old_s, old_t };
	return result;
}

template <typename T>
constexpr T ModuloInvMul_NonPrime(T A, T M) {
	// Assumes that A and M are co-prime
	// Returns multiplicative modulo inverse of A under M

	// Find gcd using Extended Euclid's Algorithm
	std::array<T, 3> v = ExtendedEuclidGCD(A, M);
	//i64 gcd = v[0];
	//i64 x = v[1];
	//i64 y = v[2]; // We don't really need this though

	// In case x is negative, we handle it by adding extra M
	// Because we know that multiplicative inverse of A in range M lies
	// in the range [0, M-1]
	return v[1] < 0 ? v[1] + M : v[1];
}

///////////////////////////////////////////////////////////////////////////////
// Heap utilities
///////////////////////////////////////////////////////////////////////////////
template <typename T>
void PushHeap(T& container, const typename T::value_type& val)
{
	container.push_back(val);
	std::push_heap(container.begin(), container.end());
}

template <typename T, typename Predicate>
void PushHeap(T& container, const typename T::value_type& val, const Predicate& pred)
{
	container.push_back(val);
	std::push_heap(container.begin(), container.end(), pred);
}

template <typename T>
typename T::value_type PopHeap(T& container)
{
	typename T::value_type result = container.front();
	std::pop_heap(container.begin(), container.end());
	container.pop_back();
	return result;
}

template <typename T, typename Predicate>
typename T::value_type PopHeap(T& container, const Predicate& pred)
{
	typename T::value_type result = container.front();
	std::pop_heap(container.begin(), container.end(), pred);
	container.pop_back();
	return result;
}

template <typename T, typename Predicate, typename... args>
void EmplaceHeap(T& container, const Predicate& pred, args&&... a)
{
	container.emplace_back(a...);
	std::push_heap(container.begin(), container.end(), pred);
}

///////////////////////////////////////////////////////////////////////////////
// A* base implementation
///////////////////////////////////////////////////////////////////////////////

// This AStarInfo structure is meant to be used with a std::heap
// NodeInfo is the data about the node in the A* search, must have operator <
// Cost_t are values used to compute cost and heuristics, must have operators <, == and +
template <typename NodeInfo, typename Cost_t>
struct AStarInfo
{
	AStarInfo(const NodeInfo& i, Cost_t c, Cost_t eval) : info(i), cost_plus_Eval(c + eval), cost(c) {}

	bool operator<(const AStarInfo& right) const
	{ // Heaps are highest first, so we invert the comparison
		return right.cost_plus_Eval < cost_plus_Eval ||
			(right.cost_plus_Eval == cost_plus_Eval && right.info < info);
	}

	NodeInfo info;
	Cost_t cost_plus_Eval;
	Cost_t cost;
};

template <typename NodeInfo, typename Cost_t, typename Pred = std::less<AStarInfo<NodeInfo, Cost_t>>>
class AStar
{
public:
	using ASI = AStarInfo<NodeInfo, Cost_t>;
	using Estimator = std::function<Cost_t(NodeInfo)>;

	AStar(const Estimator& e) : estimator(e) {}
	AStar(std::vector<ASI>&& data, const Estimator& e) : heap(std::move(data)), estimator(e), predicate()
	{
		std::make_heap(heap.begin(), heap.end(), predicate);
		for (const ASI& asi : heap)
			repeatGuard.insert(asi);
	}

	inline const ASI& PeakFront() const { return heap.front(); }
	inline ASI PopFront() { return PopHeap(heap, predicate); }
	inline void Push(const ASI& asi) { PushHeap(heap, asi, predicate); repeatGuard.insert(asi.info); }
	template<typename... Args>
	inline void Emplace(Args&& ... args) { heap.emplace_back(args...); repeatGuard.insert(heap.back().info); std::push_heap(heap.begin(), heap.end(), predicate); }
	inline void EstimatedEmplace(const NodeInfo& ni, Cost_t cost) { heap.emplace_back(ni, cost, cost + estimator(ni)); repeatGuard.insert(heap.back().info); std::push_heap(heap.begin(), heap.end(), predicate); }

	inline bool HasEvaluatedNode(const NodeInfo& ni) const { return repeatGuard.find(ni) != repeatGuard.cend(); }
	inline bool Empty() const { return heap.empty(); }
	inline void Reset() { heap.clear(); repeatGuard.clear(); }

private:
	std::vector<ASI> heap;
	std::set<NodeInfo> repeatGuard;
	Estimator estimator;
	Pred predicate;
};

