# Writing a Parser for JSON grammar

Transforming human understood language into a representation for the computer is an intimidating challenge. Compiler that take such languages and perform these transformations are black magic written by Wizards not software engineers. Languages range from Programming Languages such as Python, Java, C etc... that are written by humans, and through a chain of steps brought down to instructions for your CPU to execute. Another set of "language" that has a new flavor every month is data definition languages ->  YAML, TOML, JSON, etc...

Both the data and Programming languages have the following common traits:
- Clear Syntax Definitions
- Custom Data Representations that the human readable languages transform into.(Think variables, classes, functions, JSON Arrays, JSON objects etc...)

So in my quest to uncover the abstractions of modern day Programming I decided to explore writing a Parser that would take a popular Language and be able to transform it into Node Representations that respect the language syntax rules. I chose JSON since this is a data type that has come to dominate the industry (and also because I wanted to learn parsing but not spend a month, I want to continue on my HTTP lib after this detour).

Parsing of all\*(that i know of) languages can be broken into 2 steps that are present in all cases: Tokenizing, and Parsing. The 2 phases are responsible for taking a program from a state of char iterables into Nodes that can be traveresed for further processing. In Data Languages such as JSON, parsing can often be the end goal, where as in Programming languages, we will create a Parse Tree, and then pass the tree to another set of chained operations that will then eventually be responsible for the executable, or interpretations. Tokenizing and Parsing, remains fairly comparable in both language types.

Tokenizing is responsible for taking a char iterable and then converting it into tokens. Tokens are chunks of chars that represent the keywords, and other primitives of the language, and syntactical components. Tokenizers will do things like ignore extra whitespace and comments.

The tokenizer provides a way to lookahead for the next token, as well as a way to consume that token from the stream, and proceed to reading the next token it creates from the stream.


