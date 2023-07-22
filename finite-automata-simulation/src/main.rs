use serde::{Deserialize, Serialize};
use std::env;
use std::fs;

// State and transition function where from each state an action only leads to one other state.
// That is, one action cannot lead us to multiple states from the current state.
// If we were in conflict with the above statement we would have a non deterministic finite
// automata. Automata means a mathematical model to describe a computational process, or machine
// Finite means .... finite states that the machine/process can be in

// Given an input string run it through DFA simulation based on provided DFA definition file
#[derive(Debug, Deserialize, Serialize)]
struct Action {
    action: String,
    next_state: String,
}

#[derive(Debug, Deserialize, Serialize)]
struct State {
    state: String,
    allowed_terminal: bool,
    actions: Vec<Action>,
}

fn dfa_simulation(dfa: Vec<State>, input: &String) -> Result<bool, String> {
    let mut current_state = &dfa[0];

    println!("Init State -> {:?}", current_state);
    for character in input.chars() {
        // is there a state we can move to via the car as an action
        println!(
            "Searching from transitions {:?} for {}",
            current_state.actions, character
        );
        let valid_transition: Vec<&Action> = current_state
            .actions
            .iter()
            .filter(|&action| action.action == character.to_string())
            .collect();

        if valid_transition.len() > 1 {
            return Err(format!(
                "Action {} cannot have multiple transition options in a DFA",
                character
            )
            .to_string());
        }

        if valid_transition.len() == 0 {
            return Ok(false);
        }

        let next_state = &valid_transition[0].next_state;
        // find the next_state from our dfa
        current_state = dfa
            .iter()
            .filter(|&s| &s.state == next_state)
            .last()
            .unwrap();

        println!("Moved to State -> {:?}", current_state);
    }

    Ok(current_state.allowed_terminal)
}

fn nfa_simulation(dfa: Vec<State>, input: &String) -> Result<bool, String> {
    // find the initial state
    // see if either paths from initial state eventually leads to an accepting state
}

fn main() {
    // read dfa from json file
    // read input from cli
    // run through DFA
    let args: Vec<String> = env::args().collect();
    if args.len() < 4 {
        println!(
            "Please provide a flag --nfa or --dfa, file name, and input string to run DFA against"
        );
        return;
    }
    let nfa_or_dfa = &args[1];
    let file_name = &args[2];
    let input_string = &args[3];

    // Read the file contents into a String
    let file_contents = match fs::read_to_string(file_name) {
        Ok(contents) => contents,
        Err(err) => {
            println!("Error reading file: {}", err);
            return;
        }
    };

    let dfa: Vec<State> = serde_json::from_str(&file_contents).unwrap();

    if nfa_or_dfa == "--nfa" {
        let result = nfa_simulation(dfa, input_string);
        println!("{:?}", result);
    } else if nfa_or_dfa == "--dfa" {
        let result = dfa_simulation(dfa, input_string);
        println!("{:?}", result);
    } else {
        println!("First arg has to be --nfa or --dfa");
        return;
    }
}
