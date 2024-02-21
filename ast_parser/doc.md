# Writing a Parser for JSON grammar

Transforming human understood language into a representation for the computer is an intimidating challenge. Compiler that take such languages and perform these transformations are black magic written by Wizards not software engineers. Languages range from Programming Languages such as Python, Java, C etc... that are written by humans, and through a chain of steps brought down to instructions for your CPU to execute. Another set of "language" that has a new flavor every month is data definition languages ->  YAML, TOML, JSON, etc...

Both the data and Programming languages have the following common traits:
- Clear Syntax Definitions
- Custom Data Representations that the human readable languages transform into.(Think variables, classes, functions, JSON Arrays, JSON objects etc...)

So in my quest to uncover the abstractions of modern day Programming I decided to explore writing a Parser that would take a popular Language and be able to transform it into Node Representations that respect the language syntax rules. I chose JSON since this is a data type that has come to dominate the industry (and also because I wanted to learn parsing but not spend a month, I want to continue on my HTTP lib after this detour).

Parsing of all\*(that i know of) languages can be broken into 2 steps that are present in all cases: Tokenizing, and Parsing. The 2 phases are responsible for taking a program from a state of char iterables into Nodes that can be traveresed for further processing. In Data Languages such as JSON, parsing can often be the end goal, where as in Programming languages, we will create a Parse Tree, and then pass the tree to another set of chained operations that will then eventually be responsible for the executable, or interpretations. Tokenizing and Parsing, remains fairly comparable in both language types.

Tokenizing is responsible for taking a char iterable and then converting it into tokens. Tokens are chunks of chars that represent the keywords, and other primitives of the language, and syntactical components. Tokenizers will do things like ignore extra whitespace and comments.

The tokenizer provides a way to lookahead for the next token, as well as a way to consume that token from the stream, and proceed to reading the next token it creates from the stream. This gives us the ability to "lookahead".

The parser takes the whitespace stripped tokens and uses them to enforce certain syntax rules, and create nodes corresponding to a "Grammar". The nodes are basically what you are used to seeing in the things like Json parsing libraries that let you read in strings into Json Objects.

Now before we get into the algorithm we need to understand the core building block behind writing a parsing algorithm for a language such as ours. The language in the context of parsing stems from it's grammar. We use context free grammar as a way to represent the rules of our language formally, these rules. Each rule is called a production rule and it is composed of a non-terminal symbol that is described by a sequence of terminal and non-terminal symbols. Terminals are the basic elements of the language, while non-terminals represent syntactic structures composed of terminals and other non-terminals.

The grammar we will be using to implement our parser describes JSON using production rules!


- Value    -> Object | Array | String | Number | True | False | Null
- Object   -> '{' Members '}'
- Members  -> Pair | Pair ',' Members
- Pair     -> String ':' Value
- Array    -> '[' Elements ']'
- Elements -> Value | Value ',' Elements
- String   -> '"' characters '"'
- Number   -> '-'? digit+ ('.' digit+)? (('E'|'e') ('+'|'-')? digit+)?
- True     -> 'true'
- False    -> 'false'
- Null     -> 'null'
- characters -> any valid characters except '"'
- digit    -> '0' | '1' | ... | '9'

Based on the above grammar we are using the left values or the non-terminals to describe their structure using a combination of other terminals, and non-terminals such as '{' and '}' and ','. We can read our grammar left to right and we can figure out which "option" from the possible disjoint union of options to pick from by looking one token ahead!

In terms of our tokenizer we need something that can sort of strip away everything else from the input stream and give us a sequence of non-terminals that we can also do lookaheads on.

Recursive Descent Parsing is a top down parsing technique (starts from a start symbol, in our case "Value"), which then uses the token scanner's abiltiy to looahead for the next token and make a decision for what type of production rule to use. Since the Grammar can be Recursive in nature we rely on recursion to expand certain nodes as we parse through the streams of token left to right. The recursion is what allows the parser to handle nested structures such as an object that holds an array of objects...

In my code you will see the idea of peeking to make a decision or syntax checking and consuming a token by calling Get on the tokenizer to making progress in our token stream. Overall, I think the trick is in understandin your grammar and defining the right data structures. Once I had that, the code was fairly a breeze.

We can start with the tokenizer that sort of works on accumulating characters from a string stream based on an understanding of what type of token we are building. At any point we look for violations for the rule and cases where we can safely terminate a token. The accumulating is only performed when someone wants us to read in a a token.

Peeking can be implementing by sort of caching the last read token, and invalidating the "cache" when Get is called aka moving the cursor.

Since tokens work in a stream-like manner and we don't need to get all tokens before parsing. We can use the tokenizer directly in the parser to do things such as lookaheads and make appropriate decisions. As we make decisions we keep progressing the tokenizer till we eventuall reach the end of the token stream.
