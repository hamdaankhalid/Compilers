/*
 * expr -> term rest
 * rest -> + term { print(+) } rest
 *      -> - term { print(-) } rest
 *      -> épsilon
 *
 * term -> digit { print( digit ) }
 * */
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
}
