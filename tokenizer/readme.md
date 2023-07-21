# Lexical Analyisis / Lexer / Tokenizer

First stage of a compiler, responsible for dividing the characters into units/chunks that represent
something of meaning in the language, for example a keyword, a literal, or punctuation.

I was introduced to lexical analysis in the first few chapters of Dragon Book. Although this version of
the lexer actively chooses to skip over certain parts described in chapter 3 such as using regular expressions,
and reading code to be compiled in chunks. The algorithm written does adhere to the concept of walking through
a state machine's execution.

The algorithm maintains the current token when it starts iterating over the characters and based on the next character
searches for a valid state to transition to, based on the found state it moves ahead. If no valid state is found to
transition into the program shows the syntax error and it's location.

The next version after I finish chapter 3 will leverage regular expressions, and chunking to be more "production ready"
