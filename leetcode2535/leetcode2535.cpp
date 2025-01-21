// leetcode2535.cpp

// SOLUTION ACCEPTED
// Runtime beats 100%
// Memory beats 79%

//You are given a positive integer array nums.
//
//The element sum is the sum of all the elements in nums.
//The digit sum is the sum of all the digits(not necessarily distinct) that appear in nums.
//Return the absolute difference between the element sum and digit sum of nums.
//
//Note that the absolute difference between two integers x and y is defined as | x - y | .
//
//Constraints:
//
//1 <= nums.length <= 2000
//1 <= nums[i] <= 2000

#include <vector>

class Solution {
public:
    unsigned int addDigits(int number) {
        unsigned int sum = 0;
        while (number > 0) {
            sum += number % 10; // extract last digit
            number = number / 10; // remove last digit
        }
        return sum;
    }

    int differenceOfSum(std::vector<int>& nums) {
        // Find element sum:
        int element_sum = 0;

        for (const auto& it : nums) {
            element_sum += it;
        }

        // Find sum of digits
        int sum_digits = 0;
        for (const auto& it : nums) {
            sum_digits += addDigits(it);
        }

        int answer = sum_digits - element_sum;
        return (answer < 0) ? answer * -1 : answer; // return absolute value
    }
};