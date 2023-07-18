use std::env;
use std::fs;

/*
* Tokenize a C like syntax program after reading the file name from cli
Tokens folow the type:
    interface Token {
      type: 'Keyword' | 'Identifier' | 'Literal' | 'Operator' | 'Punctuation';
      value: string;
      line: number;
      column: number;
    }

    where the types are as follows:

   'Keyword': Represents keywords in the language, such as if, for, while, etc.
   'Identifier': Represents user-defined names or identifiers, such as variable names or function names.
   'Literal': Represents literal values, such as numbers or strings.
   'Operator': Represents operators, such as +, -, *, /, etc.
   'Punctuation': Represents punctuation marks, such as ,, ;, (, ), etc.
* */

const OPERATORS: [char; 7] = ['+', '-', '*', '/', '=', '<', '>'];
const PUNCTUATIONS: [char; 7] = ['.', ',', '(', ')', '{', '}', ';'];

const TWO_CHAR_COMP_OPERATORS: [char; 3] = ['>', '<', '='];

const LITERAL_IDENTIFIER: char = '\'';
const WHITESPACE: char = ' ';
const NEWLINE: char = '\n';

#[derive(Clone, Debug)]
enum TokenType {
    Keyword,
    Identifier,
    Literal,
    Operator,
    Punctuation,
}

#[derive(Clone, Debug)]
struct Token {
    token_type: TokenType,
    value: String,
    line: u32,
    column: u32,
}

fn tokenize(code: String) -> Result<Vec<Token>, String> {
    let mut tokens: Vec<Token> = Vec::new();
    let mut current_token: Option<Token> = None;
    let mut line_num: u32 = 0;

    let mut two_char_operator: bool = false;

    for character_idx in 0..code.len() {
        let character = code.chars().nth(character_idx).unwrap();

        // println!("{} at idx {} {:?}", character, character_idx, current_token);

        if character == WHITESPACE && !currently_literal(&current_token) {
            // an identifier, keyword, Operator, or Punctuation would not contain a whitespace so
            // this marks the end of the previous token
            push_if_not_none(&mut tokens, &current_token);
            current_token = None;
            // skip over white space only if we are not actively building a literal
            continue;
        }

        if character == NEWLINE && !currently_literal(&current_token) {
            // an identifier, keyword, Operator, or Punctuation would not contain a newline so
            // this marks the end of the previous token
            push_if_not_none(&mut tokens, &current_token);
            current_token = None;
            // skip over new line character only if we are not actively building a literal
            line_num += 1;
            continue;
        }

        if PUNCTUATIONS.contains(&character) && !currently_literal(&current_token) {
            push_if_not_none(&mut tokens, &current_token);
            tokens.push(Token {
                token_type: TokenType::Punctuation,
                value: character.to_string(),
                line: line_num,
                column: character_idx as u32,
            });
            current_token = None;
        } else if OPERATORS.contains(&character) && !currently_literal(&current_token) {
            if two_char_operator {
                current_token.as_mut().unwrap().value.push(character);
                push_if_not_none(&mut tokens, &current_token);
                two_char_operator = false;
                current_token = None;
            } else if is_two_char_seq(&character, code.chars().nth(character_idx + 1)) {
                // check if it should start a 2 char operator sequence
                push_if_not_none(&mut tokens, &current_token);
                two_char_operator = true;
                current_token = Some(Token {
                    token_type: TokenType::Operator,
                    value: character.to_string(),
                    line: line_num,
                    column: character_idx as u32,
                });
                // if no then just do insert and move forward, else do something else
            } else {
                // if not 2 char operator
                push_if_not_none(&mut tokens, &current_token);
                tokens.push(Token {
                    token_type: TokenType::Operator,
                    value: character.to_string(),
                    line: line_num,
                    column: character_idx as u32,
                });

                current_token = None;
            }
        } else if character == LITERAL_IDENTIFIER && !currently_literal(&current_token) {
            push_if_not_none(&mut tokens, &current_token);
            current_token = Some(Token {
                token_type: TokenType::Literal,
                value: "".to_string(),
                line: line_num,
                column: character_idx as u32,
            });
        } else if character == LITERAL_IDENTIFIER && currently_literal(&current_token) {
            // the end of a literal
            push_if_not_none(&mut tokens, &current_token);
            current_token = None;
        }
        // Else if we have an ongoing literal in that case shove the character into the value
        else if currently_literal(&current_token) {
            current_token.as_mut().unwrap().value.push(character);
        }
        // Now we know we dont have a literal running anymore so use the logic for idfentifier
        // and the keyword, Initiall everything is constructed as a identifier, then we later do
        // a parse and change somel identifiers to keywords as we deem necessary
        else if character.is_alphabetic() || character.is_digit(10) {
            // if existing current token and of type identifier add this character there
            // if no exsitign token then this is the new one of identifier type
            // otherwise this is a syntax error
            if current_token.is_some()
                && matches!(
                    &current_token.as_ref().unwrap().token_type,
                    TokenType::Identifier
                )
            {
                current_token.as_mut().unwrap().value.push(character);
            } else if current_token.is_some() {
                return Err(format!(
                    "Character not a part of identifier Syntax is off buddy: line {}, col {}",
                    line_num, character_idx
                )
                .to_string());
            } else {
                // current token is none in this case construct a new one for the identifier
                current_token = Some(Token {
                    token_type: TokenType::Identifier,
                    value: character.to_string(),
                    line: line_num,
                    column: character_idx as u32,
                });
            }
        } else {
            return Err(format!(
                "{} Identifier after a space. Syntax is off buddy: line {}, col {}",
                character, line_num, character_idx
            ));
        }
    }

    if let Some(remaining) = &current_token {
        if currently_literal(&current_token) {
            return Err(format!(
                "Unfinished literal from line {} col {}",
                remaining.line, remaining.column
            )
            .to_string());
        }
        tokens.push(remaining.clone());
    }
    Ok(tokens)
}

fn currently_literal(token: &Option<Token>) -> bool {
    match token {
        None => false,
        Some(t) => {
            matches!(t.token_type, TokenType::Literal)
        }
    }
}

fn push_if_not_none(tokens: &mut Vec<Token>, curr: &Option<Token>) {
    if let Some(existing) = curr {
        tokens.push(existing.clone());
    }
}

fn is_two_char_seq(character: &char, next_char: Option<char>) -> bool {
    if next_char.is_none() {
        return false;
    }

    (*character == '<' && next_char.unwrap() == '=')
        || (*character == '>' && next_char.unwrap() == '=')
        || (*character == '=' && next_char.unwrap() == '=')
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        println!("Please provide a file name");
        return;
    }
    let file_name = &args[1];

    // Read the file contents into a String
    let file_contents = match fs::read_to_string(file_name) {
        Ok(contents) => contents,
        Err(err) => {
            println!("Error reading file: {}", err);
            return;
        }
    };

    let tokens = tokenize(file_contents);

    match tokens {
        Err(e) => println!("Tokenizer found error: {}", e),
        Ok(t) => t.iter().for_each(|a| println!("{:?}", a)),
    };
}
