#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
 * Epsilon NFA Explanation Copy Pasta:
 *
 *An ε-NFA (epsilon-Nondeterministic Finite Automaton) is a type of finite
automaton used in theoretical computer science and formal language theory to
describe and recognize regular languages. It is an extension of a traditional
NFA (Nondeterministic Finite Automaton) with the addition of epsilon transitions
(also known as ε-transitions or lambda transitions).

Let's break down the components and concepts of an ε-NFA in absolute detail:

States: An ε-NFA consists of a finite set of states, often denoted by Q. Each
state represents a specific condition or configuration of the automaton.

Alphabet: There is a finite input alphabet Σ, which consists of symbols or
characters that the automaton reads from an input string. These symbols are the
basic building blocks of the language.

Transitions: Unlike a traditional DFA (Deterministic Finite Automaton), an ε-NFA
can have multiple transitions for a given state and input symbol. These
transitions are non-deterministic, meaning that the automaton can move to
multiple states at once when processing a particular symbol. Additionally,
ε-NFAs can have epsilon transitions (ε-transitions), which are transitions that
occur without consuming any input symbol. An ε-transition allows the automaton
to move from one state to another without reading any input, effectively adding
an extra dimension to its computation.

Start State: There is a designated start state (often denoted as q0) where the
automaton begins its computation.

Accept States: The ε-NFA also has a set of accept states (or final states),
which are states that, when reached after processing the entire input string,
indicate that the automaton recognizes the input string as a valid member of the
language.

Epsilon Transitions: Epsilon transitions (ε-transitions) are transitions that
are not associated with any input symbol. They are represented using the ε
symbol. When the automaton encounters an ε-transition, it can move from the
current state to the target state without consuming any input symbol. Epsilon
transitions allow for non-determinism, as the automaton can make choices without
needing any input cues.

Language Recognition: To determine whether a given input string belongs to the
language recognized by the ε-NFA, the automaton explores all possible paths
through its states, considering both regular transitions (based on input
symbols) and ε-transitions. It accepts the input string if there exists at least
one path that leads to an accept state after processing the entire input.

Equivalence to Regular Languages: ε-NFAs are capable of recognizing the same
class of languages as DFAs and NFAs. This means that any language recognized by
an ε-NFA can also be recognized by a regular NFA or DFA. Similarly, any language
recognized by a regular NFA or DFA can be recognized by an ε-NFA.
 * */

class Nfa {
private:
  std::unordered_set<int> finalStates;
  std::unordered_map<int, std::unordered_set<int>> epsilonTransitions;
  std::unordered_map<int,
                     std::unordered_map<std::string, std::unordered_set<int>>>
      transitions;

public:
  int stateCounter = 0;

  static std::shared_ptr<Nfa> createEpsilonNfa() {
    std::shared_ptr<Nfa> base(new Nfa);
    base->addEpsilonTransition(0, 1);

    base->stateCounter += 2;

    base->addFinalState(1);

    return base;
  }

  Nfa() {}

  void addTransition(int fromState, const std::string &symbol, int toState) {
    this->transitions[fromState][symbol].insert(toState);
  }

  void addEpsilonTransition(int fromState, int toState) {
    this->epsilonTransitions[fromState].insert(toState);
  }

  void addFinalState(int state) { finalStates.insert(state); }

  bool runSimulation(const std::string &input) {
    // TODO: tbf
    return true;
  }

  void print() {
    std::cout << "NFA { \n";

    std::cout << "Final States: \n";

    for (int finalState : this->finalStates) {
      std::cout << "+ " << finalState << std::endl;
    }

    std::cout << "EpsilonTransitions \n";

    for (std::pair<int, std::unordered_set<int>> epsilonTransition :
         this->epsilonTransitions) {
      std::cout << "+ " << epsilonTransition.first << " -> [ ";
      for (const int &state : epsilonTransition.second) {
        std::cout << state << ", ";
      }
      std::cout << " ] \n";
    }

    std::cout << "Transitions \n";

    for (std::pair<int,
                   std::unordered_map<std::string, std::unordered_set<int>>>
             transition : this->transitions) {
      std::cout << "+ From " << transition.first << std::endl;
      std::cout << "\t";
      for (std::pair<std::string, std::unordered_set<int>> symbolToStates :
           transition.second) {
        std::cout << "Given Symbol " << symbolToStates.first << ": [ ";
        for (const int &toState : symbolToStates.second) {
          std::cout << toState << ", ";
        }
        std::cout << " ] \n";
      }
    }

    std::cout << "} \n";
  }
};

std::vector<std::string> preProcessRegex(const std::string &regex) {
  std::vector<char> groupingAndRelations =
      std::vector<char>{'|', '+', '(', ')'};
  std::vector<std::string> splitted;
  std::string builder;
  for (char curr : regex) {
    std::vector<char>::iterator it = std::find(
        groupingAndRelations.begin(), groupingAndRelations.end(), curr);
    if (it != groupingAndRelations.end()) {
      if (!builder.empty()) {
        splitted.push_back(builder);
      }
      builder.clear();
      // insert the delimitter
      splitted.push_back(std::string(1, curr));
    } else {
      builder += curr;
    }
  }

  if (!builder.empty()) {
    splitted.push_back(builder);
  }
  return splitted;
}

std::vector<std::string>
infixToPostFixTranslation(std::vector<std::string> infix) {
  std::vector<std::string> pushers =
      std::vector<std::string>{"+", "|", "(", "*"};
  std::string popOn = ")";

  std::vector<std::string> outputQueue;
  std::vector<std::string> stack;

  for (const std::string &candidate : infix) {
    std::vector<std::string>::iterator it =
        std::find(pushers.begin(), pushers.end(), candidate);
    if (it != pushers.end()) {
      stack.push_back(candidate);
    } else if (candidate == popOn) {
      // while the stack is not empty and the current top most element is an
      // operator of higher than the left one keep popping the top of
      while (!stack.empty()) {
        std::string top = stack[stack.size() - 1];
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
  while (!stack.empty()) {
    if (stack[stack.size() - 1] == "(") {
      std::__throw_invalid_argument("Invalid regex");
    }
    outputQueue.push_back(stack[stack.size() - 1]);
    stack.pop_back();
  }

  std::cout << "POSTFIX: ";
  for (const std::string &i : outputQueue) {
    std::cout << i;
  }

  std::cout << "\n";
  return outputQueue;
}


// Create an NFA and change its topology as we iterate through the regular expression
// return the NFA back to the invoker. This Nfa now has all the states and transitions
// needed to be able to match a pattern
std::shared_ptr<Nfa> buildNfa(std::vector<std::string> postFixed) {
  // all our primitive operators can be treated as binary so I will
  // solve them the same way we do mathematical binary operators
  // at this point we can only have primitive operators of Concatenation and
  // Alternations infix to postfix took care of handling groupings

  std::shared_ptr<Nfa> nfa = Nfa::createEpsilonNfa();
  int lastState = 0;

  std::unordered_map<std::string, std::string> symbolMapping;

  std::vector<std::string> stack;
  for (const std::string &primitiveCandidate : postFixed) {
    if (primitiveCandidate == "|") {
      // pop the last two
      std::string x = stack.at(stack.size() - 1);
      stack.pop_back();

      std::string y = stack.at(stack.size() - 1);
      stack.pop_back();

      // make a fork connecting the alternator
      int stateForX = nfa->stateCounter;
      int stateforY = nfa->stateCounter + 1;

      int stateForOnXMatch = nfa->stateCounter + 2;
      int stateForOnYMatch = nfa->stateCounter + 3;

      int commonStatePostMerge = nfa->stateCounter + 4;

      nfa->addEpsilonTransition(lastState, stateForX);
      // TODO: FIX ALL CALLS TO ADD TRANSITION, how to get a hold of the symbol
      nfa->addTransition(stateForX, x, stateForOnXMatch);

      nfa->addEpsilonTransition(lastState, stateforY);
      nfa->addTransition(stateforY, y, stateForOnYMatch);

      nfa->addEpsilonTransition(stateForOnXMatch, commonStatePostMerge);
      nfa->addEpsilonTransition(stateForOnYMatch, commonStatePostMerge);

      nfa->addEpsilonTransition(commonStatePostMerge,
                                1); // 1 is the final acceptance stage
      nfa->stateCounter += 5;

      lastState = commonStatePostMerge;

      std::string alternated = "(" + x + ")|(" + y + ")";

      stack.push_back(alternated);

    } else if (primitiveCandidate == "+") {
      // pop the last two
      std::string x = stack.at(stack.size() - 1);
      stack.pop_back();

      std::string y = stack.at(stack.size() - 1);
      stack.pop_back();

      // merge x and y and connect last state to x, then x to y, then make y
      // last state
      int stateForX = nfa->stateCounter;
      int stateForY = nfa->stateCounter + 1;

      // TODO
      nfa->addTransition(lastState, x, stateForX);
      nfa->addTransition(stateForX, y, stateForY);
      nfa->addEpsilonTransition(stateForY, 1); // link to acceptance state
      nfa->stateCounter += 2;
      lastState = stateForY;

      std::string concatenated = "(" + x + ")+(" + y + ")";

      stack.push_back(concatenated);
    } else if (primitiveCandidate == "*") {
      // Kleene is a unary and is to be treated differently
      std::string x = stack.at(stack.size() - 1);
      stack.pop_back();

      int topOfForkState = nfa->stateCounter;
      int topOfForkNextState = nfa->stateCounter + 1;
      int onXMatchFork = nfa->stateCounter + 2;
      int loopOnXMatchFork = topOfForkNextState;

      int pesudoAcceptanceState = nfa->stateCounter + 3;

      nfa->addEpsilonTransition(lastState, pesudoAcceptanceState);

      nfa->addEpsilonTransition(lastState, topOfForkState);

      // TODO
      nfa->addTransition(topOfForkNextState, x, onXMatchFork);
      nfa->addEpsilonTransition(onXMatchFork, loopOnXMatchFork);
      nfa->addEpsilonTransition(onXMatchFork, pesudoAcceptanceState);

      nfa->addEpsilonTransition(pesudoAcceptanceState,
                                1); // 1 is the base most acceptance state

      nfa->stateCounter += 4;

      std::string kleene = "(" + x + "*)";
      stack.push_back(kleene);
    } else {
      stack.push_back(primitiveCandidate);
    }
  }

  std::cout << stack.at(0) << std::endl;

  return nfa;
}

int main() {
  std::cout << "############ REGULAR EXPRESSION PARSER & COMPILER #############"
            << std::endl;

  std::string expr = "a+b*|a+(c|d)";
  std::cout << "INFIX Regex: " << expr << std::endl;

  std::cout << "######## COMPILE TO NFA ########" << std::endl;

  std::vector<std::string> processed = preProcessRegex(expr);
  std::vector<std::string> postFixed = infixToPostFixTranslation(processed);

  std::shared_ptr<Nfa> nfa = buildNfa(postFixed);

  std::cout << "######## Completed Nfa: #########" << std::endl;

  nfa->print();

  std::cout << "######## Evaluate against NFA #########" << std::endl;

  std::cout << "la la lala" << std::endl;
}
