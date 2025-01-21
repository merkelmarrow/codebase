// leetcode3224.cpp : Defines the entry point for the application.
//


// NOT WORKING

//You are given an integer array nums of size n where n is even, and an integer k.
//
//You can perform some changes on the array, where in one change you can replace any element in the array with any integer in the range from 0 to k.
//
//You need to perform some changes(possibly none) such that the final array satisfies the following condition :
//
//There exists an integer X such that abs(a[i] - a[n - i - 1]) = X for all(0 <= i < n).
//Return the minimum number of changes required to satisfy the above condition.
//
//Constraints :
//
//	2 <= n == nums.length <= 10^5
//	n is even.
//	0 <= nums[i] <= k <= 10^5

#include <vector>
#include <utility>
#include <cmath>

class Solution {
public:
	int countChanges(const std::pair<int, int>& pair, int X) {
		int diff = std::abs(pair.first - pair.second);
		if (diff == X) { return 0; }
		bool first_bigger = pair.first >= pair.second ? true : false;
		if (first_bigger) {
			if (pair.first - X + diff < 0 && pair.second + X - diff > m_k) {
				return 2;
			}
		}
		else {
			if (pair.first + X - diff > m_k && pair.second - X + diff < 0) {
				return 2;
			}
		}
		return 1;
	}

	int minChanges(std::vector<int>& nums, int k) {
		m_k = k;
		// Populate pairs vector
		pairs.clear();
		int size = nums.size();
		for (int i = 0; i < size / 2; i++) {
			int a = nums[i], b = nums[size - i - 1];
			pairs.push_back(std::make_pair(a, b));
		}

		// Try all values of X
		int min_changes = 1000000;
		for (int X = 0; X <= k; X++) {
			int changes = 0;
			for (const auto& it : pairs) {
				changes += countChanges(it, X);
			}
			if (changes < min_changes) min_changes = changes;
		}

		return min_changes;
	}

private:
	std::vector<std::pair<int, int>> pairs;
	int m_k = 0;
};

