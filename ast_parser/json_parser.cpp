#include <iostream>
#include <unordered_map>
#include <memory>
#include <ostream>
#include <sstream>
#include <optional>
#include <vector>

// -------------------- Tokenizer and corresponding data types --------

enum TokenType {
	String,
	Number,
	LeftParenthesis,
	RightParenthesis,
	LeftBracket,
	RightBracket,
	Colon,
	Comma,
	Null,
	True,
	False,
};

struct Token {
	TokenType type;
	std::string lexeme;
};

// Lots of copying here, need to move this to using smart pointers
class JsonTokenStream {
	public:

	JsonTokenStream(std::stringstream stream): m_stream(std::move(stream)), m_peeked(std::nullopt){}
	
	bool HasTokens() {
		return m_stream.good() && m_stream.peek() != EOF || m_peeked != std::nullopt;
	}
	
	std::pair<Token, bool> Peek() {
		// if nothing staged already, read in and stage it for later
		if (m_peeked == std::nullopt) {
			m_peeked = _readInToken();
		}
		
		return m_peeked.value();
	}

	// result, valid or invalid
	std::pair<Token, bool> Get() {
		// if something was staged from previoud call to peek use that and reset staging area for peek
		// if something was already staged we dont need to read anything string stream
		if (m_peeked != std::nullopt) {
			std::pair<Token, bool> tkn = m_peeked.value();
			m_peeked = std::nullopt;
			return tkn;
		}

		return _readInToken();
	}

	private:
		std::stringstream m_stream;	
		
		std::optional<std::pair<Token, bool> > m_peeked;

		std::pair<Token, bool> _readInKnownString(TokenType type, char currChar, const std::string& knownStr) {
			Token res;
			res.type = type;
			
			// we know first char already matches
			for (int i = 1; i < knownStr.size(); i++) {
				if (m_stream.eof() || m_stream.get() != knownStr[i]) {
					return { res , false };
				}
			}

			res.lexeme = knownStr;
			return { res, true };
		}

		std::pair<Token, bool> _readInToken() {
			char currChar = ' ';
			
			// read in all useless whitespace and line skip, and other ignored white space-ish chars
			while (!m_stream.eof() && 
					((currChar = m_stream.get()) == ' ' || currChar == '\n' || currChar == '\t'));

			Token res;
			if (currChar == ' ') {
				return {res, false};
			}

			// at this point we know we are at a valid non EOF char to be read and have a token created from
			if (currChar == '{') {
				res.type = LeftParenthesis;
				res.lexeme = '{';
				return { res, true };
			}

			if (currChar == '}') {
				res.type = RightParenthesis;
				res.lexeme = '}';
				return { res, true };
			}

			if (currChar == '[') {
				res.type = LeftBracket;
				res.lexeme = '[';
				return { res, true };
			}

			if (currChar == ']') {
				res.type = RightBracket;
				res.lexeme = ']';
				return { res, true };
			}

			if (currChar == ',') {
				res.type = Comma;
				res.lexeme = ',';
				return { res, true };
			}
		
			if (currChar == ':') {
				res.type = Colon;
				res.lexeme = ':';
				return { res, true };
			}

			if (currChar == '\"') {
				// start of a string
				std::string jsonStr;
				res.type = String;
				while (true) {
					if (m_stream.eof())	{
						return { res, false };
					}

					char next = m_stream.get();
					if (next == '\"') {
						// if the last element in our sstream is not an escape character and
						// we have reached second quote this says we are at end of string
						if (jsonStr.back() != '\\') {
							res.lexeme = jsonStr;
							return { res, true };
						}
						jsonStr += next;
					}
					
					jsonStr += next;
				}

			}
			
			// check if keyword true
			if (currChar == 't') {
				return _readInKnownString(True, currChar, "true");
			}

			if (currChar == 'f') {
				return _readInKnownString(False, currChar, "false");
			}
			
			// check if null symbol
			if (currChar == 'n') {
				return _readInKnownString(Null, currChar, "null");
			}

			// check if start of a number
			if (currChar == '-' || (currChar >= '0' && currChar <= '9')) {
				std::pair<Token, bool> numResult = tokenizeNumber(currChar);
				return numResult;
			}

			// invalid token
			return {res, false};
		}

		std::pair<Token, bool> tokenizeNumber(char currChar) {
			Token res;
			res.type = Number;
			bool hadMyE = false;
			bool hadMyDecimal = false;	
			bool hadMyNegaitveBeforeE = currChar == '-';

			std::string numero(1, currChar);

			while (HasTokens()) {
				char newChar = m_stream.peek();
				// based on last number we know if this is valid or not
				// we know that if the last character was a - or . the number after it must be digit
				if ((numero.back() == '-' || numero.back() == '.') &&  (newChar < '0' || newChar > '9')) {
					return { res, false };
				}
				
				if (
						newChar == '-' && 
						(numero.back() != 'e' && numero.back() != 'E')
				) {
					return { res, false };
				}

				// can add e, E, or number, or .
				if (newChar =='-' || newChar == 'e' || newChar == 'E' || (newChar >= '0' && newChar <= '9') || newChar == '.') {
					if (newChar == 'e' || newChar == 'E') {
						if (hadMyE)
							return { res, false };
						else
							hadMyE = true;
							hadMyDecimal = false; // allow decimal again after E
					}

					if (newChar == '.') {
						if (hadMyDecimal)
							return { res, false };
						else
							hadMyDecimal = true;
					}

					m_stream.get(); // consume
					numero += newChar;
					continue;
				}

				// whatever we are adding is an invalid addition to ongoing number string, so it's end of this number string by now
				// if it is invalid then new token will not be formed after this :)
				res.lexeme = numero;
				return { res, true };
			}
			res.lexeme = numero;
			return { res, true };
		}
};

// ------------ End of tokenizer -----------


// ------------ Parser -------------------

enum JsonNodeType {
	// Internal node that we only use in the parser to show errors
	ErrorNodeType,
	ObjectNodeType,
	ArrayNodeType,
	StringNodeType,
	NumberNodeType,
	BooleanNodeType,
	NullNodeType,
};

// Different type of nodes such
// as Object, Err, Arr, String, Number, Bool, Null
struct JsonNode;

struct ObjectNode {
	std::unordered_map<std::string, std::unique_ptr<JsonNode>> properties;
};

struct ArrayNode {
	std::vector<std::unique_ptr<JsonNode>> vals;
};

using FlatVal = std::string;

// Would have used union or std::variant this fucker keeps messing up the default constructor issue
// Lots of wasted space in this struct that can be avoided if we use Union type
struct JsonNodeValue {
	FlatVal val;
	ObjectNode objNode;
	ArrayNode arrNode;
};

struct JsonNode {
	JsonNodeType type;
	JsonNodeValue data;
	
	// recursive tree repr print to screen
	void PrintTreeRepr(int indent = 0) {
		std::string output = "";
		switch (type) {
			case ErrorNodeType:
			case StringNodeType:
			case NumberNodeType:
			case BooleanNodeType:
			case NullNodeType:
				for (int i = 0; i < indent; i++)
					output += ' ';
				output += data.val;
				std::cout << output << std::endl;
				break;
			case ArrayNodeType:
				for (int i = 0; i < indent; i++)
					output += ' ';
				std::cout << output << '[' << std::endl;
				for (int i = 0; i < data.arrNode.vals.size(); i++) {
					data.arrNode.vals[i]->PrintTreeRepr(indent+1);
					if (i != data.arrNode.vals.size()-1)
						std::cout << output << ",";
				}
				std::cout << output << ']' << std::endl;
				break;
			case ObjectNodeType:
				for (int i = 0; i < indent; i++)
					output += ' ';
				std::cout << output << "{ \n";

				for (std::pair<const std::string&, const std::unique_ptr<JsonNode>& >pairs : data.objNode.properties) {
					std::cout << output << " " << pairs.first << ":\n";
					pairs.second->PrintTreeRepr(indent+1);
				}

				std::cout << output << "} \n";

				break;
		}
	}
};

class Parser {
	public:
		Parser(std::unique_ptr<JsonTokenStream> scanner): m_scanner(std::move(scanner)) {}
	
		std::unique_ptr<JsonNode> MakeJsonNode() 
		{
			std::pair<Token, bool> token = 
				m_scanner->Peek();

			if (!token.second) {
				std::unique_ptr<JsonNode> node = 
					std::make_unique<JsonNode>();
				node->type = ErrorNodeType;
				node->data.val = "End of tokens from parser before Node formed";
				return node;
			}

			switch (token.first.type) {
				case LeftParenthesis:
					return makeObject();
				case LeftBracket:
					return makeArray();
				case String:
					return makeString();
				case Number:
					return makeNumber();
				case True:
				case False:
					return makeBoolean();
				case Null:
					return makeNull();
				default:
					std::unique_ptr<JsonNode> node = 
						std::make_unique<JsonNode>();
					node->type = ErrorNodeType;
					node->data.val = "Unexpected token";
					return node;
			};
		}

	private:
		std::unique_ptr<JsonTokenStream> m_scanner;		
		
		std::unique_ptr<JsonNode> makeObject() {
			// consume "{" that we know definitely existed
			m_scanner->Get();
			
			std::unique_ptr<JsonNode> node = std::make_unique<JsonNode>();
			// Pair | Pair ','  Members
			node->type = ObjectNodeType;
			
			// store Pair | Pair, Members in the node
			std::pair<Token, bool> lookahead = m_scanner->Peek();
			while (lookahead.second && lookahead.first.type != RightParenthesis) {
				// Should be a parse-able pair, aka STRING ":" JsonNode
				std::pair<Token, bool> shouldBeStr = m_scanner->Peek();
				if (!shouldBeStr.second || shouldBeStr.first.type != String) {
					node->type = ErrorNodeType;
					node->data.val = "Failed to build object, keys can only be strings";
					return node;
				}
				// consume Key string
				m_scanner->Get();
				// Should have a comma that we parse next
				std::pair<Token, bool> shouldBeColon = m_scanner->Peek();
				if (!shouldBeColon.second || shouldBeColon.first.type != Colon) {
					node->type = ErrorNodeType;
					node->data.val = "A string key in a Json Pair in an object should be followed by a ':'";
					return node;
				}
				// consume COLON
				m_scanner->Get();
				std::unique_ptr<JsonNode> pairValueNode = MakeJsonNode();
				if (pairValueNode->type == ErrorNodeType) {
					return pairValueNode;
				}
				
				node->data.objNode.properties[shouldBeStr.first.lexeme] = std::move(pairValueNode);

				// potentially have a comma so check and consume
				lookahead = m_scanner->Peek();
				if (!lookahead.second) {
					node->type = ErrorNodeType;
					node->data.val = "Unexpected end of token stream";
					return node;
				}
				
				if (lookahead.first.type == Comma) {
					// consume the comma!
					m_scanner->Get();
				}

				lookahead = m_scanner->Peek();
			}

			if (!lookahead.second || lookahead.first.type != RightParenthesis) {
				node->type = ErrorNodeType;
				node->data.val = "Unexpected end of object while parsing the tokens, valid tokens finished before object ended";
				return node;
			}
			
			// consume closing parenthesis
			m_scanner->Get();
			
			return node;
		}
		
		// '[' JsonNode ',' .. ']'
		std::unique_ptr<JsonNode> makeArray() {
			std::unique_ptr<JsonNode> node = std::make_unique<JsonNode>();
			node->type = ArrayNodeType;
			
			// consume the '['
			m_scanner->Get();

			std::pair<Token, bool> lookahead = m_scanner->Peek();
			while (lookahead.second && lookahead.first.type != RightBracket) {
				std::unique_ptr<JsonNode> jsonNode = MakeJsonNode();
				if (jsonNode->type == ErrorNodeType) {
					return jsonNode;
				}

				node->data.arrNode.vals.push_back(std::move(jsonNode));

				lookahead = m_scanner->Peek();
				if (!lookahead.second) {
					node->type = ErrorNodeType;
					node->data.val = "Unexpected end of token stream while building array";
					return node;
				}

				if (lookahead.first.type == Comma) {
					// consume the comma!
					m_scanner->Get();
				}
			}

			if (!lookahead.second || lookahead.first.type != RightBracket) {
				node->type = ErrorNodeType;
				node->data.val = "Array ended without valid Right Bracket Token";
				return node;
			}

			return node;
		}

		std::unique_ptr<JsonNode>  makeString() {
			std::pair<Token, bool> shouldBeString = m_scanner->Get();
			std::unique_ptr<JsonNode> node = std::make_unique<JsonNode>();
			node->type = StringNodeType;

			if (!shouldBeString.second || shouldBeString.first.type != String) {
				node->type = ErrorNodeType;
				node->data.val = "Expected String Node";
				return node;
			}

			node->data.val = shouldBeString.first.lexeme;
			return node;
		}

		std::unique_ptr<JsonNode> makeBoolean() {
			std::pair<Token, bool> shoudlBeBool = m_scanner->Get();
			std::unique_ptr<JsonNode> node = std::make_unique<JsonNode>();
			node->type = BooleanNodeType;

			if (!shoudlBeBool.second ||( shoudlBeBool.first.type != True && shoudlBeBool.first.type != False)) {
				node->type = ErrorNodeType;
				node->data.val = "Expected Boolean Node";
				return node;
			}
		
			node->data.val = shoudlBeBool.first.type == True ? "true" : "false";

			return node;
		}

		std::unique_ptr<JsonNode> makeNumber() {
			std::pair<Token, bool> shouldBeNumber = m_scanner->Get();
			std::unique_ptr<JsonNode> node = std::make_unique<JsonNode>();
			node->type = NumberNodeType;

			if (!shouldBeNumber.second || shouldBeNumber.first.type != Number) {
				node->type = ErrorNodeType;
				node->data.val = "Expected Number";
				return node;
			}
			
			node->data.val = shouldBeNumber.first.lexeme;
			return node;
		}

		std::unique_ptr<JsonNode> makeNull() {
			std::pair<Token, bool> shoudlBeNull = m_scanner->Get();
			std::unique_ptr<JsonNode> node = std::make_unique<JsonNode>();
			node->type = NullNodeType;
			node->data.val = "null";

			if (!shoudlBeNull.second || shoudlBeNull.first.type != Null) {
				node->type = ErrorNodeType;
				node->data.val = "Expected NULL";
				return node;
			}
						
			return node;
		}
};


// ------------ End of parser ------------

void testTokenizer() {
	std::vector<std::pair<std::string, bool> > tests = {
		{ "null", true },
		{ "true", true },
		{ "false", true },
		{ "\"meow meow meow\"", true },
		{ "1738", true },
		{ "[ \"puckity\", null, 7]", true },
		{ "{ \"kit\": \"kat\" }", true },
		{ "{ \"kit\": 1745 }", true },
		{ "17.45", true },
		{ "-17.5", true },
		{ "-17.5", true },
		{ "1e3", true },
		{ "notstrenclosed", false },
		{ "1e3, 45, -13.4", true },
		{ "1ee3", false },
		{ "-1e3", true },
		{ "-1E3", true },
		{ "-1e-3", true },
		{ "--1e-3", false },
		{ "--1e--3", false },
		{ "-7.83e-3", true},
		{ "7.83e-3.5", true},
		{ "7.83e-3.5.68", false},
		{ "{ \"snickers\" : true, \"foo\": false }", true },
	};
	
	for (std::pair<std::string, bool> testCase : tests)
	{
		std::string test = testCase.first;
		bool expectedToParse = testCase.second;

		std::cout << "--- Test with --- -> " << test << std::endl;
		std::cout << "Should Pass ? -> " << expectedToParse << std::endl;
		std::cout << "\n";

		std::stringstream t(test);
		JsonTokenStream tokenstrm(std::move(t));
		
		bool passed = true;
		while (tokenstrm.HasTokens()) {
			std::pair<Token, bool> tokenRes = tokenstrm.Get();	
			if (tokenRes.second) {
				if (tokenRes.first.type == String) {
					std::cout << "String Token: " << tokenRes.first.lexeme << std::endl;
				} else if (tokenRes.first.type == Number) {
					std::cout << "Number Token: " << tokenRes.first.lexeme << std::endl;
				} else {
					std::cout << "Special Char Token: " << tokenRes.first.lexeme << std::endl;
				}
			} else {
				passed = false;
				break;
			}
		}

		std::cout << "\n";
		if (passed && expectedToParse || !passed && !expectedToParse) {
			std::cout << "Result -> ** Passed Test ** " << std::endl;
		} else {
			std::cout << "Result -> ** Failed Test ** " << std::endl;
		}

		std::cout << "\n\n";
	}

	std::cout << "--- Tokenizer Tests completed ---" << std::endl;
};

void testParser() {
	std::vector<std::pair<std::string, JsonNodeType> > tests = {
		{"null", NullNodeType},
		{"1738", NumberNodeType},
		{"\"work as a string\"", StringNodeType },
		{"{ \"keystr\" : 1.45 }", ObjectNodeType },
		{"{ \"keystr\" : 69, \"meow\": \"mystr\" }", ObjectNodeType },
		{"{ \"keystr\" : 69, \"meow\": \"mystr\", \"marinara\": { \"metakey\": null } }", ObjectNodeType },
		{"[ \
			{ \
			\"keystr\" : 69, \
			\"meow\": \"mystr\", \
			\"marinara\": { \
				\"metakey\": null \
			  } \
			}, \
			null , \
			124, \
			\"yay\" \
			[ null, \"i did it father\", \
			{ \
				\"foo\": 0.99 \
			} \
		  ]", ArrayNodeType },
	};
	
	for (std::pair<std::string, JsonNodeType> testCase : tests ) {
		std::string test = testCase.first;

		std::cout << "Testing " << test << std::endl;
	
		JsonNodeType expectedType = testCase.second;
		std::stringstream t(test);
		std::unique_ptr<JsonTokenStream> tokenstrm(new JsonTokenStream(std::move(t)));
		Parser parser(std::move(tokenstrm));
		std::unique_ptr<JsonNode> rootNode = parser.MakeJsonNode();

		std::cout << "Result -> ";
		if (rootNode->type != expectedType) {
			std::cout << "** Failed Test Case Expected " << expectedType << " Got "
				<< rootNode->type << " ** " << std::endl;
			
			if (rootNode->type == ErrorNodeType) {
				std::cout << rootNode->data.val << std::endl;
			}
		} else {
			rootNode->PrintTreeRepr();

			std::cout << "** Passed Test Case ** " << std::endl;
		}

		std::cout << "\n\n";
	}

	std::cout << "------ Parser Tests completed ---" << std::endl;
}

int main() {
	testTokenizer();
	
	std::cout << "********************************\n\n";

	testParser();
}
