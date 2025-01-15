// fizzbuzz.cpp
// This solution assumes the following:
//		- needs to return a string array (vector initialised with n size has almost the same memory footprint as array of size n)
//		- n <= 10000

#include <iostream>
#include <vector>
#include <string>
#include <limits>

const std::string f = "fizz";
const std::string b = "buzz";
const std::string fb = "fizzbuzz";

void printOutput(const std::vector<std::string>& arr);

int main()
{
	int input = 0;
	
	while (true) {
		std::cout << "Enter a positive integer: ";

		// Validate input
		if (!(std::cin >> input)) {
			std::cerr << "Error: Invalid input. Please enter a positive integer." << std::endl;
			// Clear the error state and discard invalid input
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			continue;
		}

		// Check for positive integer
		if (input <= 0) {
			std::cerr << "Error: Input must be a positive integer." << std::endl;
			continue;
		}

		// Input is valid
		break;
	}

	std::vector<std::string> arr(input);

	for (size_t i = 1; i <= (size_t)input; i++) {
		if (i % 15 == 0) {
			arr[i - 1] = fb;
		}
		else if (i % 5 == 0) {
			arr[i - 1] = b;
		}
		else if (i % 3 == 0) {
			arr[i - 1] = f;
		}
		else {
			arr[i - 1] = std::to_string(i);
		}
	}

	// output is arr

	// verification
	printOutput(arr);

	return 0;

}

void printOutput(const std::vector<std::string>& arr) {
	std::cout << "[";
	for (size_t i = 0; i < arr.size(); i++) {
		std::cout << "\"" << arr[i] << "\"";
		if (i != arr.size() - 1) {
			std::cout << ",";
		}
	}
	std::cout << "]" << std::endl;
}