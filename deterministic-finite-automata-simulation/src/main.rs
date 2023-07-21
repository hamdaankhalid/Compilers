
// State and transition function where from each state an action only leads to one other state.
// That is, one action cannot lead us to multiple states from the current state.
// If we were in conflict with the above statement we would have a non deterministic finite
// automata. Automata means a mathematical model to describe a computational process, or machine
// Finite means .... finite states that the machine/process can be in


// Given an input string run it through DFA simulation based on provided DFA definition file
struct Action {
    action: String,
    next_state: String
}

struct State {
    state: String,
    allowed_terminal: bool,
    actions: Vec<Action>, 
}

fn simulation(dfa: Vec<State>, input: String) -> Result<bool> {
}

fn main() {
 // read dfa from json file
 // read input from cli
 // run through DFA
}
