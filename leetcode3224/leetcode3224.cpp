// leetcode3224.cpp : Defines the entry point for the application.
//


// NOT WORKING (TIME LIMIT EXCEEDED)

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
    int countChanges(const std::pair<int, int>& p, int X) {
        int a = p.first, b = p.second;
        int diff = std::abs(a - b);

        // 0 changes if already X
        if (diff == X) {
            return 0;
        }

        // Check if 1 change is possible
        //
        //   c = b + X
        //   c = b - X
        //   c = a + X
        //   c = a - X
        //
        // and each c must be in [0, m_k].
        bool canOneChange = false;

        // c = b + X
        if ((b + X) >= 0 && (b + X) <= m_k) {
            canOneChange = true;
        }
        // c = b - X
        if ((b - X) >= 0 && (b - X) <= m_k) {
            canOneChange = true;
        }
        // c = a + X
        if ((a + X) >= 0 && (a + X) <= m_k) {
            canOneChange = true;
        }
        // c = a - X
        if ((a - X) >= 0 && (a - X) <= m_k) {
            canOneChange = true;
        }

        return canOneChange ? 1 : 2;
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

