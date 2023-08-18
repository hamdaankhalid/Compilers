#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>
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
 * - "+"
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

	int name;

	bool acceptingState;

	// multiple exits given a Concatenation means only one exit from this state to next
	// an alternation means multiple exits
	std::unordered_map<std::string, std::shared_ptr<Nfa> > relations;

public:
	static int stateCounter;

	static std::shared_ptr<Nfa> createEpsilonNfa() {
		std::shared_ptr<Nfa> root(new Nfa(stateCounter, true));
		std::shared_ptr<Nfa> baseCaseAcceptingState(new Nfa(-1, true));
		root->addRelation("EPSILON", baseCaseAcceptingState);
		stateCounter++;
		return root;
	}
	
	static std::shared_ptr<Nfa> createStateForSubexpressionA(std::shared_ptr<Nfa> a) {
		std::shared_ptr<Nfa> newState(new Nfa(stateCounter, true));
		std::shared_ptr<Nfa> baseCaseAcceptingState(new Nfa(-1, true));
		
		newState->addRelation("subexpr A", a);
		a->addRelation("concat with parent's accepting state", baseCaseAcceptingState);

		stateCounter++;

		return newState;
	}

	Nfa(int name, bool isAcceptance) : name(name), acceptingState(isAcceptance) {}	

	void addRelation(const std::string& action, std::shared_ptr<Nfa> next) {
		this->relations.insert(std::make_pair(action, next));
	}
	
	void printDfs(int depth = 0) const {
		// print indentation
		std::string space;
		if (depth != 0) {
			for (int i = 0; i < depth; i++) {
				space+= ' ';
			}
		}
		std::cout << space;
		// print self
		std::cout << "{ " << this->name << ", " << this->acceptingState << " }\n";
		
		// print children indented
		for (const auto& it : this->relations) {
			it.second->printDfs(depth+1);	
		}
	}
};

std::vector<std::string> preProcessRegex(const std::string& regex) {
	std::vector<char> groupingAndRelations = std::vector<char>{'|', '+', '(', ')'};
	std::vector<std::string> splitted;
	std::string builder;
	for(char curr : regex) {
		std::vector<char>::iterator it = std::find(groupingAndRelations.begin(), groupingAndRelations.end(), curr);
		if (it != groupingAndRelations.end()) {
			if (!builder.empty()) {
				splitted.push_back(builder);
			}
			builder.clear();
			// insert the delimitter
			splitted.push_back(std::string(1, curr));
		} else {
			builder+=curr;
		}
	}

	if (!builder.empty()) {
		splitted.push_back(builder);
	}
	return splitted;
}

std::vector<std::string> infixToPostFixTranslation(std::vector<std::string> infix) {
	std::vector<std::string> pushers = std::vector<std::string>{"+", "|", "(", "*"};
	std::string popOn = ")";

	std::vector<std::string> outputQueue;
	std::vector<std::string> stack;

	for(const std::string& candidate : infix) {
		std::vector<std::string>::iterator it = std::find(pushers.begin(), pushers.end(), candidate);
		if(it != pushers.end()) {
			stack.push_back(candidate);
		} else if (candidate == popOn) {
			// while the stack is not empty and the current top most element is an operator
			// of higher than the left one keep popping the top of
			while(!stack.empty()) { 
				std::string top = stack[stack.size()-1];
				if (top == "(") {
					stack.pop_back();
					break;
				} else {
					outputQueue.push_back(top);
					stack.pop_back();
				}
			}
		} else {
			outputQueue.push_back(candidate);
		}
	}

	// Anything remaining on stack?
	while(!stack.empty()) {
		if (stack[stack.size() - 1] == "(") {
			std::__throw_invalid_argument("Invalid regex");
		}
		outputQueue.push_back(stack[stack.size()-1]);
		stack.pop_back();
	}
	
	std::cout << "POSTFIX: ";
	for (const std::string& i : outputQueue) {
		std::cout << i;
	}

	std::cout << "\n";
	return outputQueue;
}

std::shared_ptr<Nfa> buildNfa(std::vector<std::string> postFixed) {	
	// all our primitive operators can be treated as binary so I will
	// solve them the same way we do mathematical binary operators

	// at this point we can only have primitive operators of Concatenation and Alternations
	// infix to postfix took care of handling groupings

	// root state

	std::shared_ptr<Nfa> lastState = Nfa::createEpsilonNfa();
	std::unordered_map<std::string, std::unique_ptr<Nfa> > stringToNode;

	std::vector<std::string> stack;	
	for (const std::string& primitiveCandidate : postFixed) {
		if (primitiveCandidate == "|") {
			// pop the last two
			std::string x = stack.at(stack.size() - 1);
			stack.pop_back();

			std::string y = stack.at(stack.size() - 1);
			stack.pop_back();
			
			// create 2 separate nodes for the primitives, and concatenate them?
			std::string	alternated = "(" + x + ")|(" + y + ")";
			
			stack.push_back(alternated);
		} else if (primitiveCandidate == "+") {
			// pop the last two
			std::string x = stack.at(stack.size() - 1);
			stack.pop_back();
	
			std::string y = stack.at(stack.size() - 1);
			stack.pop_back();
				
			std::string	concatenated = "(" + x + ")+(" + y + ")";
			stack.push_back(concatenated);
		} else if (primitiveCandidate == "*") {
			// Kleene is a unary and is to be treated differently
			std::string x = stack.at(stack.size() - 1);
			stack.pop_back();

			std::string kleene = "(" + x + "*)";
			stack.push_back(kleene);
		} else {
			stack.push_back(primitiveCandidate);
		}
	}
	
	std::cout << stack.at(0) << std::endl;

	return lastState;
}


int Nfa::stateCounter = 0;
int main() { 
	std::cout << "############ REGULAR EXPRESSION PARSER & COMPILER #############" << std::endl;

	std::string expr = "a+b*|a+(c|d)";
	std::cout << "INFIX Regex: " << expr << std::endl;
	
	std::cout << "######## COMPILE TO NFA ########" << std::endl;

	std::vector<std::string> processed = preProcessRegex(expr);
	std::vector<std::string> postFixed = infixToPostFixTranslation(processed);
	
	std::shared_ptr<Nfa> nfa = buildNfa(postFixed);
	
	std::cout << "Completed Nfa: " << std::endl;

	nfa->printDfs();

	std::cout << "######## Evaluate against NFA #########" << std::endl;

	std::cout << "la la lala" << std::endl; 
}
