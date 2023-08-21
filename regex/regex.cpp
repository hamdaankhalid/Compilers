#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

/*
 * Primitives:
 * - For now if something is not an operator we treat it as a literal
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

char groupingAndRelations[5] = {'*', '|', '+', '(', ')'};

bool isOperatorOrGroups(char a) {
  for (int i = 0; i < 5; i++) {
    if (groupingAndRelations[i] == a) {
      return true;
    }
  }
  return false;
}

bool isOperatorOrGroups(const std::string &x) {
  if (x.size() != 1) {
    return false;
  }
  return isOperatorOrGroups(x[0]);
}

class Nfa {
private:
  std::unordered_set<int> finalStates;
  std::unordered_map<int, std::unordered_set<int>> epsilonTransitions;
  std::unordered_map<int,
                     std::unordered_map<std::string, std::unordered_set<int>>>
      transitions;

  int stateCounter = 0;

public:
  const int startState =
      0; // Based on how states are create 0 is always the init state
  const int endAcceptanceState =
      1; // Based on how states are created 1 is always the accepting state

  static std::shared_ptr<Nfa> createEpsilonNfa() {
    std::shared_ptr<Nfa> base(new Nfa);
    base->addEpsilonTransition(base->createNewState(), base->createNewState());

    base->addFinalState(base->endAcceptanceState);

    return base;
  }

  Nfa() {}

  int createNewState() {
    this->stateCounter++;
    return this->stateCounter - 1;
  }

  bool isStateChainedAlready(int state) {
    std::unordered_map<int, std::unordered_set<int>>::iterator found =
        this->epsilonTransitions.find(state);
    if (found == this->epsilonTransitions.end()) {
      return false;
    }
    return !this->epsilonTransitions.at(state).empty();
  }

  void addTransition(int fromState, const std::string &symbol, int toState) {
    this->transitions[fromState][symbol].insert(toState);
  }

  void addEpsilonTransition(int fromState, int toState) {
    this->epsilonTransitions[fromState].insert(toState);
  }

  bool hasEpsilonTransitions(int state) {
    std::unordered_map<int, std::unordered_set<int>>::iterator found =
        this->epsilonTransitions.find(state);
    if (found == this->epsilonTransitions.end()) {
      return false;
    }

    return !this->epsilonTransitions[state].empty();
  }

  void transferEpsilonTransitions(int owner, int transferTo) {
    std::unordered_map<int, std::unordered_set<int>>::iterator found =
        this->epsilonTransitions.find(owner);
    if (found == this->epsilonTransitions.end()) {
      std::cout << "nothing transfered" << std::endl;
      return;
    }
    std::unordered_set<int> epsilons = this->epsilonTransitions[owner];
    this->epsilonTransitions[transferTo] = epsilons;
    this->epsilonTransitions[owner] = std::unordered_set<int>();
  }

  void removeEpsilonTransition(int fromState, int toState) {
    std::unordered_map<int, std::unordered_set<int>>::iterator found =
        this->epsilonTransitions.find(fromState);
    if (found == this->epsilonTransitions.end()) {
      return;
    }

    this->epsilonTransitions[fromState].erase(toState);
  }

  void addFinalState(int state) { finalStates.insert(state); }

  /*
   * Given an input string iterate over each character
   * for each character we are going to explore a next path
   * if there are any explorations that result in an accepted state
   * we return true, otherwise false.
   */
  // TODO: infinite loop stop
  bool runSimulation(const std::string &input) {
    int hardLimitSelfLoop = 100;
    std::unordered_map<int, int> selfLoopTransitions;

    int transitionEndsAt = input.size();

    // stack stores the current state and the current character index being
    // explored
    std::vector<std::pair<int, char>> stack;
    // add the intial state onto the stack
    std::pair<int, int> initial = std::make_pair(this->startState, 0);
    stack.push_back(initial);

    while (!stack.empty()) {
      std::pair<int, int> candidate = stack[0];
      stack.pop_back();

      // std::cout << "Candidate " << candidate.first << ", " <<
      // candidate.second << ", transitionEnd: " << transitionEndsAt <<
      // std::endl;
      if (candidate.second == transitionEndsAt &&
          candidate.first == this->endAcceptanceState) {
        return true;
      }

      // epsilon transitions
      std::unordered_map<int, std::unordered_set<int>>::iterator found =
          this->epsilonTransitions.find(candidate.first);
      if (found != this->epsilonTransitions.end()) {
        std::unordered_set<int> epsilonTransitions =
            this->epsilonTransitions[candidate.first];
        for (int nextState : epsilonTransitions) {
          std::pair<int, int> toExplore =
              std::make_pair(nextState, candidate.second);
          stack.push_back(toExplore);
        }
      }

      // when we are on last index of input string only epsilon transitions are
      // allowed
      if (candidate.second == transitionEndsAt) {
        continue;
      }

      // action based transitions
      std::unordered_map<
          int,
          std::unordered_map<std::string, std::unordered_set<int>>>::iterator
          foundTransitionAbleState = this->transitions.find(candidate.first);
      if (foundTransitionAbleState != this->transitions.end()) {
        std::unordered_map<std::string, std::unordered_set<int>>
            actionTransitions = this->transitions[candidate.first];

        std::string symbol = std::string(1, input.at(candidate.second));
        std::unordered_map<std::string, std::unordered_set<int>>::iterator
            foundSymbol = actionTransitions.find(symbol);

        if (foundSymbol != actionTransitions.end()) {
          std::unordered_set<int> nextStates = actionTransitions[symbol];
          int nextIdx = candidate.second + 1;
          for (int nextState : nextStates) {
            std::pair<int, int> exploration =
                std::make_pair(nextState, nextIdx);
            stack.push_back(exploration);
          }
        }
      }

      // push them onto the stack
    }

    return false;
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
  std::vector<std::string> splitted;
  std::string builder;
  for (char curr : regex) {
    if (isOperatorOrGroups(curr)) {
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

// Create an NFA and change its topology as we iterate through the regular
// expression return the NFA back to the invoker. This Nfa now has all the
// states and transitions needed to be able to match a pattern
std::shared_ptr<Nfa> buildNfa(std::vector<std::string> postFixed) {
  // all our primitive operators can be treated as binary so I will
  // solve them the same way we do mathematical binary operators at
  // this point we can only have primitive operators of Concatenation
  // and alternations infix to postfix took care of handling groupings
  const std::shared_ptr<Nfa> nfa = Nfa::createEpsilonNfa();

  const int lastStateStart = nfa->startState;
  int lastStateEnd = nfa->endAcceptanceState;

  // each base alphabet in the language is given a start state and an end state,
  // we will add these transitions into our nfa, and then connnect Epsilon
  // transitions to them
  std::unordered_map<std::string, std::pair<int, int>> symbolMapping;
  for (const std::string &candidate : postFixed) {
    if (!isOperatorOrGroups(candidate)) {
      int startState = nfa->createNewState();
      int endState = nfa->createNewState();

      std::pair<int, int> startEndStatePair =
          std::make_pair(startState, endState);

      symbolMapping.insert(std::make_pair(candidate, startEndStatePair));
      // these are the actual non epsilon transitions
      nfa->addTransition(startState, candidate, endState);
      // when we iterate over our actual regex we will be adding these real
      // transitions between the existing start and end state to create
      // topologies that reflect the expressions and sub-expressions
    }
  }

  std::vector<std::string> stack;

  for (const std::string &primitiveCandidate : postFixed) {
    // print stacks contents for debuggging
    std::cout << "Stack Contents:" << std::endl;
    for (const std::string &item : stack) {
      std::cout << item << ", ";
    }
    std::cout << "\n";

    if (primitiveCandidate == "|") {
      // pop the last two
      std::string y = stack.at(stack.size() - 1);
      stack.pop_back();

      std::string x = stack.at(stack.size() - 1);
      stack.pop_back();

      // make a fork connecting the alternator
      // x and y represent a state, this state is already in our symbol mapping
      std::pair<int, int> statesForMatchX = symbolMapping.at(x);
      std::pair<int, int> statesForMatchY = symbolMapping.at(y);

      // when doing a fork we basically break the connection between the current
      // start state and end state then we add our fork in between
      int preForkCommonState = nfa->createNewState();
      int postForkCommonState = nfa->createNewState();

      // break connection
      nfa->removeEpsilonTransition(lastStateStart, lastStateEnd);

      // create the alternator
      nfa->addEpsilonTransition(preForkCommonState, statesForMatchX.first);
      nfa->addEpsilonTransition(preForkCommonState, statesForMatchY.first);

      // if has epsilon transitions
      if (nfa->hasEpsilonTransitions(statesForMatchX.second)) {
        std::cout << "Attempt transfer from " << statesForMatchX.second
                  << " to " << postForkCommonState << std::endl;
        nfa->transferEpsilonTransitions(statesForMatchX.second,
                                        postForkCommonState);
      }

      if (nfa->hasEpsilonTransitions(statesForMatchY.second)) {
        std::cout << "Attempt transfer from " << statesForMatchY.second
                  << " to " << postForkCommonState << std::endl;
        nfa->transferEpsilonTransitions(statesForMatchY.second,
                                        postForkCommonState);
      }

      nfa->addEpsilonTransition(statesForMatchX.second, postForkCommonState);
      nfa->addEpsilonTransition(statesForMatchY.second, postForkCommonState);

      if (!nfa->isStateChainedAlready(postForkCommonState)) {
        nfa->addEpsilonTransition(postForkCommonState, lastStateEnd);
      }

      // add the alternator into the nfa
      nfa->addEpsilonTransition(lastStateStart, preForkCommonState);

      // find the x and y states start and ends for both and then create an
      // alternator then take their combined start and end and store in the
      // mapping
      std::string alternated = "(" + x + ")|(" + y + ")";

      // add the state and metadata back into mappings
      symbolMapping.insert(std::make_pair(
          alternated, std::make_pair(preForkCommonState, postForkCommonState)));

      lastStateEnd = preForkCommonState;

      stack.push_back(alternated);
    } else if (primitiveCandidate == "+") {
      // pop the last two
      std::string y = stack.at(stack.size() - 1);
      stack.pop_back();

      std::string x = stack.at(stack.size() - 1);
      stack.pop_back();

      // x and y represent a state, this state is already in our symbol mapping
      std::pair<int, int> statesForMatchX = symbolMapping.at(x);
      std::pair<int, int> statesForMatchY = symbolMapping.at(y);

      // we need to keep the start constant and be actually moving the last end
      // we are going to make it:
      // lastStateStart -> pseudoStartForConcatState -> x -> y
      // ->pseudoEndStateForConcatState -> lastStateEnd -> the rest of the
      // expression
      nfa->removeEpsilonTransition(lastStateStart, lastStateEnd);

      std::cout << "Concatenating X and Y: " << statesForMatchX.first << ", "
                << statesForMatchX.second << " ; " << statesForMatchY.first
                << " , " << statesForMatchY.second << std::endl;

      std::cout << "Current lastStateStart " << lastStateStart
                << " and current lastStateEnd " << lastStateEnd << std::endl;

      if (nfa->isStateChainedAlready(statesForMatchX.second)) {
        nfa->transferEpsilonTransitions(statesForMatchX.second,
                                        statesForMatchY.second);
      }
      // connect x and y
      nfa->addEpsilonTransition(statesForMatchX.second, statesForMatchY.first);

      // add them into the existing expr head (head because we are evaluating
      // reverse polish
      nfa->addEpsilonTransition(lastStateStart, statesForMatchX.first);

      // check if last state is already connected in a terminating chain
      // if a node has an epsilon transition out of it means it was already
      // connected
      if (!nfa->isStateChainedAlready(statesForMatchY.second) &&
          statesForMatchY.second != lastStateEnd) {
        nfa->addEpsilonTransition(statesForMatchY.second, lastStateEnd);
      }

      std::string concatenated = "(" + x + ")+(" + y + ")";
      symbolMapping.insert(
          std::make_pair(concatenated, std::make_pair(statesForMatchX.first,
                                                      statesForMatchY.second)));

      lastStateEnd = statesForMatchX.first;

      stack.push_back(concatenated);
    } else if (primitiveCandidate == "*") {
      // Kleene is a unary and is to be treated differently
      std::string x = stack.at(stack.size() - 1);
      stack.pop_back();

      std::pair<int, int> statesForMatchX = symbolMapping.at(x);

      // break connection
      nfa->removeEpsilonTransition(lastStateStart, lastStateEnd);

      // construct a kleene then add it into the nfa
      int preForkCommonState = nfa->createNewState();
      int pseudoEnd = nfa->createNewState();

      // transfer everything in second to pseudoEnd
      if (nfa->hasEpsilonTransitions(statesForMatchX.second)) {
        nfa->transferEpsilonTransitions(statesForMatchX.second, pseudoEnd);
      }

      nfa->addEpsilonTransition(statesForMatchX.second, pseudoEnd);

      // loop back on the kleene
      nfa->addEpsilonTransition(statesForMatchX.second, statesForMatchX.first);
      nfa->addEpsilonTransition(preForkCommonState, pseudoEnd);
      nfa->addEpsilonTransition(preForkCommonState, statesForMatchX.first);

      nfa->addEpsilonTransition(lastStateStart, preForkCommonState);
      if (!nfa->isStateChainedAlready(pseudoEnd)) {
        nfa->addEpsilonTransition(pseudoEnd, lastStateEnd);
      }

      std::string kleene = "(" + x + "*)";

      symbolMapping.insert(std::make_pair(
          kleene, std::make_pair(preForkCommonState, pseudoEnd)));

      stack.push_back(kleene);

      lastStateEnd = preForkCommonState;
    } else {
      stack.push_back(primitiveCandidate);
    }

    std::cout << "State of NFA" << std::endl;
    nfa->print();

    std::cout << "last state start first " << lastStateStart
              << ", last state end " << lastStateEnd << std::endl;
  }

  // if our answer is at the end do we need to add an epsilon for it?
  if (stack.size() != 0) {
    std::string x = stack.at(0);
    stack.pop_back();

    std::pair<int, int> xState = symbolMapping.at(x);

    if (xState.first != lastStateEnd) {
      std::cout << "WARNING: This should only execute if there was only one "
                   "item in the regular expression"
                << std::endl;

      assert(stack.size() == 0);

      nfa->removeEpsilonTransition(lastStateStart, lastStateEnd);
      nfa->addEpsilonTransition(lastStateStart, xState.first);

      // check if last state is already connected in a terminating chain
      // if a node has an epsilon transition out of it means it was already
      // connected
      if (!nfa->isStateChainedAlready(xState.second)) {
        nfa->addEpsilonTransition(xState.second, lastStateEnd);
      }
    }

    lastStateEnd = xState.first;

    std::cout << "Verify postfix by checking converted version:" << std::endl;

    std::cout << x << std::endl;
  }

  return nfa;
}

void test(const std::string &regex, std::vector<std::string> matches,
          std::vector<std::string> notMatches) {
  std::cout << "INFIX Regex: " << regex << std::endl;

  std::cout << "######## COMPILE TO NFA ########" << std::endl;

  std::vector<std::string> processed = preProcessRegex(regex);
  std::vector<std::string> postFixed = infixToPostFixTranslation(processed);

  std::shared_ptr<Nfa> nfa = buildNfa(postFixed);

  std::cout << "######## Completed Nfa: #########" << std::endl;

  nfa->print();

  std::cout << "######## Evaluate against NFA #########" << std::endl;

  for (const std::string &trues : matches) {
    assert(nfa->runSimulation(trues));
  }

  for (const std::string &falses : notMatches) {
    assert(!nfa->runSimulation(falses));
  }

  std::cout << "-------- Test Passes ---------" << std::endl;
}

int main() {
  std::cout << "############ REGULAR EXPRESSION PARSER & COMPILER #############"
            << std::endl;

  // single element
  std::string singleA = "a";
  std::vector<std::string> singleFalses = {"b", "c", ""};
  std::vector<std::string> singleTrues = {"a"};
  test(singleA, singleTrues, singleFalses);

  // concat as a binary operator
  std::string concatenationBinary = "a+b";
  std::vector<std::string> concatenationBinaryFalses = {"",   "a",   "b",  "c",
                                                        "ac", "acb", "cba"};
  std::vector<std::string> concatenationBinaryTrues = {"ab"};

  test(concatenationBinary, concatenationBinaryTrues,
       concatenationBinaryFalses);

  // triple concats
  std::string concatenationRegex = "(a+b)+c";
  std::vector<std::string> concatenationFalses = {"a",  "b",   "c",  "ab",
                                                  "ac", "acb", "cba"};
  std::vector<std::string> concatenationTrues = {"abc"};

  test(concatenationRegex, concatenationTrues, concatenationFalses);
}
