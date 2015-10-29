#pragma once

namespace STLUtil {
	template <class T>
	void Swap(T &a, T &b)
	{
		T t = a;
		a = b;
		b = t;
	}
}