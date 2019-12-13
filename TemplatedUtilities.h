#pragma once
#include <algorithm>
#include <utility>
#include <type_traits>
#include <vector>
#include <set>

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

template<typename T>
struct Coord
{
	constexpr Coord(T X = 0, T Y = 0) : x(X), y(Y) {}
	constexpr bool operator <  (const Coord& p) const { if (x < p.x) return true; else if (p.x < x) return false; else return y < p.y; }
	constexpr Coord operator + (const Coord& p) const { return Coord(x + p.x, y + p.y); }
	constexpr Coord& operator+=(const Coord& p) { x += p.x; y += p.y; return *this; }
	constexpr bool operator==(const Coord& p) const { return x == p.x && y == p.y; }
	T x, y;
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

constexpr char coords[] = { 'N', 'E', 'S', 'W' };
constexpr Point directions[] = { Point(1, 0), Point(0, 1), Point(-1, 0), Point(0,-1) };

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
	T minX, minY, maxX, maxY;
};

using Bounds = Boundaries<int>;

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
	
	AStar() {}
	AStar(std::vector<ASI>&& data) : heap(std::move(data)), predicate()
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
	
	inline bool HasEvaluatedNode(const NodeInfo& ni) const { return repeatGuard.find(ni) != repeatGuard.cend(); }
	inline bool Empty() const { return heap.empty(); }
	inline void Reset() { heap.clear(); repeatGuard.clear(); }

private:
	std::vector<ASI> heap;
	std::set<NodeInfo> repeatGuard;
	Pred predicate;
};


