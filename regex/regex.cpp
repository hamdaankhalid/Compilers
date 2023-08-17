#include <memory>
#include<vector>
#include <iostream>
#include <unordered_map>

/*
 * Primitives:
 * - EMPTY 
 * - DIGIT 
 * - NUMBER 
 * - 'literal'
 * 
 * Concatenation:
 * - "-"
 *
 * Alternation:
 * - "|"
 *
 * Kleene Star:
 * - "*"
 *
 * Grouping & Paranthesis:
 * - "( )"
 * */

/*
 * This class is a node, it can be appended to, and it can be pointed to by others
 * */
class Nfa {
private:

	bool acceptingState;
	// multiple exits given
	std::unordered_map<std::string, std::unique_ptr<Nfa> > relations;

public:

	// Used during construction
	void addNext(std::string, std::unique_ptr<Nfa>) {
	}

	// Concatenate
	
	// Alternation
};

int main() { 
	std::cout << "la la lala" << std::endl; 
}
