/*
 * Actions for postfix translation
 *
 * expr -> expr + term { print + }
 *      |  expr - term { print - }
 *      |  term
 *
 * term -> term * factor { print * }
 *      |  term / factor { print / }
 *      |  factor
 *
 * factor -> ( expr )
 *      |  num { print num.value }
 *      |  id { print id.lexeme }
 * */

#[derive(Debug)]
struct Num {
    value: i32,
}

#[derive(Debug)]
struct Identifier {
    lexme: String,
}

#[derive(Debug)]
enum Operator {
    Plus,
    Minus,
    Divide,
    Multiply,
}

const CHAR_OPERATORS: [char; 4] = ['+', '-', '/', '*'];

#[derive(Debug)]
enum Tokens {
    Num(Num),
    Identifier(Identifier),
    Operator(Operator),
}

fn get_operator(operator: char) -> Result<Operator, String> {
    match operator {
        '+' => Ok(Operator::Plus),
        '-' => Ok(Operator::Minus),
        '/' => Ok(Operator::Divide),
        '*' => Ok(Operator::Multiply),
        _ => Err("Not an operator".to_string()),
    }
}

// 12+32   - 45 /3 0
fn tokenize(expr: String) -> Result<Vec<Tokens>, String> {
    let mut counter: i32 = 0;
    let mut tokens: Vec<Tokens> = Vec::new();
    let mut current_token: Option<Tokens> = None;

    for character in expr.chars() {
        // is character a number or is it something else
        if character.is_whitespace() {
            continue;
        } else if character.is_digit(10) {
            current_token = match current_token {
                Some(curr_tok) => match curr_tok {
                    Tokens::Num(mut num) => {
                        // already a num so just add the current digit to token
                        let mut num_str = num.value.to_string();

                        num_str.push(character);

                        num.value = num_str.parse().unwrap();

                        counter += 1;
                        Some(Tokens::Num(num))
                    }
                    Tokens::Identifier(id) => {
                        // save the existing token to vector then create a new one
                        tokens.push(Tokens::Identifier(id));
                        counter = 0;
                        Some(Tokens::Num(Num {
                            value: character.to_digit(10).unwrap() as i32,
                        }))
                    }
                    Tokens::Operator(op) => {
                        tokens.push(Tokens::Operator(op));
                        counter = 0;
                        Some(Tokens::Num(Num {
                            value: character.to_digit(10).unwrap() as i32,
                        }))
                    }
                },
                None => Some(Tokens::Num(Num {
                    value: character.to_digit(10).unwrap() as i32,
                })),
            }
        } else if CHAR_OPERATORS.contains(&character) {
            current_token = match current_token {
                Some(curr_tok) => match curr_tok {
                    Tokens::Num(num) => {
                        tokens.push(Tokens::Num(num));
                        let op = get_operator(character)?;
                        Some(Tokens::Operator(op))
                    }
                    Tokens::Operator(op) => {
                        tokens.push(Tokens::Operator(op));
                        let op = get_operator(character)?;
                        Some(Tokens::Operator(op))
                    }
                    Tokens::Identifier(id) => {
                        tokens.push(Tokens::Identifier(id));
                        let op = get_operator(character)?;
                        Some(Tokens::Operator(op))
                    }
                },
                None => {
                    let op = get_operator(character)?;
                    Some(Tokens::Operator(op))
                }
            }
        } else {
            return Err("Syntax error".to_string());
        }
    }

    tokens.push(current_token.unwrap());
    Ok(tokens)
}

fn translate(mut expr: String) {
    expr = match term(expr) {
        Ok(s) => s,
        Err(e) => {
            println!("syntax error: {}", e);
            return;
        }
    };

    match rest(expr) {
        Some(e) => {
            println!("{}", e);
        }
        None => {}
    }
}

fn term(expr: String) -> Result<String, String> {
    let dig = match expr.chars().nth(0) {
        Some(v) => v,
        None => return Err("term cannot be empty".to_string()),
    };

    let parsed_dif = match dig.to_digit(10) {
        Some(v) => v,
        None => return Err("term supposed to be a single digit int".to_string()),
    };

    print!("{}", parsed_dif);

    Ok(expr.clone().chars().skip(1).collect())
}

fn rest(mut expr: String) -> Option<String> {
    if expr.len() == 0 {
        println!("");
        return None;
    }

    let next_char = expr.chars().nth(0).unwrap();

    match next_char {
        '+' => {
            let subset = expr.clone().chars().skip(1).collect();
            expr = match term(subset) {
                Ok(v) => v,
                Err(e) => return Some(e),
            };
            print!("+");
            rest(expr);
            return None;
        }
        '-' => {
            let subset = expr.clone().chars().skip(1).collect();
            expr = match term(subset) {
                Ok(v) => v,
                Err(e) => return Some(e),
            };
            print!("-");
            rest(expr);
            return None;
        }
        _ => Some("expected + or - but found a digit".to_string()),
    }
}

fn main() {
    translate("9-5+2".to_string());
    translate("1+2-3".to_string());
    translate("1+2-3+5-2+3".to_string());
    translate("1".to_string());
    translate("".to_string());
    translate("11".to_string());
    translate("-1+4".to_string());

    let test = "324 3 + 34 4 - 45 /4".to_string();
    println!("Tokenize {}", test);
    if let Ok(result) = tokenize(test) {
        println!("{:?}", result);
    }
}
