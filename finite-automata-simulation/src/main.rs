use serde::{Deserialize, Serialize};
use std::env;
use std::fs;

// State and transition function where from each state an action only leads to one other state.
// That is, one action cannot lead us to multiple states from the current state.
// If we were in conflict with the above statement we would have a non deterministic finite
// automata. Automata means a mathematical model to describe a computational process, or machine
// Finite means .... finite states that the machine/process can be in

#[derive(Debug, Deserialize, Serialize, Clone)]
struct Action {
    epsilon_state: Option<bool>,
    action: String,
    next_state: String,
}

#[derive(Debug, Deserialize, Serialize, Clone)]
struct State {
    state: String,
    allowed_terminal: bool,
    actions: Vec<Action>,
}

fn dfa_simulation(dfa: &Vec<State>, input: &String) -> Result<bool, String> {
    let mut current_state = &dfa[0];

    println!("Init State -> {:?}", current_state);
    for character in input.chars() {
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

fn nfa_simulation(nfa: &Vec<State>, input: &String, current_state: &State) -> Result<bool, String> {
    if input.len() == 0 {
        return Ok(current_state.allowed_terminal);
    }

    let first_char = input.chars().nth(0).unwrap().to_string();

    // available states includes epsilon states from here or character
    let next_states: Vec<State> = find_available_states(&nfa, current_state, &first_char);

    println!("{:?}", next_states);

    let next_input: String = input.chars().skip(1).collect();

    println!("Next input {}", next_input);

    for state in next_states {
        if state.state == current_state.state {
            continue;
        }
        let nfa_res = nfa_simulation(nfa, &next_input, &state)?;
        if nfa_res {
            return Ok(true);
        }
    }

    return Ok(false);
}

fn find_available_states(nfa: &Vec<State>, curr_state: &State, action_char: &String) -> Vec<State> {
    let next_actions: Vec<String> = curr_state
        .actions
        .iter()
        .filter(|&a| {
            (a.epsilon_state.is_some() && a.epsilon_state.unwrap()) || &a.action == action_char
        })
        .map(|a| a.next_state.clone())
        .collect();

    let next_states = nfa
        .iter()
        .filter(|&s| next_actions.contains(&s.state))
        .map(|s| s.clone())
        .collect();

    println!(
        "Next states from state {:?} based on {} are {:?}",
        curr_state, action_char, next_states
    );

    next_states
}

fn main() {
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

    let finit_automata_definition: Vec<State> = serde_json::from_str(&file_contents).unwrap();

    if nfa_or_dfa == "--nfa" {
        println!("Running NFA simulation on input {}", input_string);
        let result = nfa_simulation(
            &finit_automata_definition,
            input_string,
            &finit_automata_definition[0],
        );
        println!("{:?}", result);
    } else if nfa_or_dfa == "--dfa" {
        println!("Running DFA simulation on input {}", input_string);
        let result = dfa_simulation(&finit_automata_definition, input_string);
        println!("{:?}", result);
    } else {
        println!("First arg has to be --nfa or --dfa");
        return;
    }
}
