#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <optional>
#include <vector>
#include <queue>
#include <variant>

// -------------------- Tokenizer and corresponding data types
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

class JsonTokenStream {
	public:

	JsonTokenStream(std::stringstream stream): m_stream(std::move(stream)), m_peeked(std::nullopt){}
	
	bool HasTokens() {
		return m_stream.good() && m_stream.peek() != EOF || m_peeked != std::nullopt;
	}
	
	std::pair<Token, bool> Peek() {
		if (m_peeked == std::nullopt) {
			m_peeked = _readInToken();
		}
		
		return m_peeked.value();
	}

	// result, valid or invalid
	std::pair<Token, bool> Get() {
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
			
			// read in all useless whitespace
			while (!m_stream.eof() && (currChar = m_stream.get()) == ' ');

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



// ------------ TODO: Parser -------------------

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
union JsonNodeValue {

};

struct JsonNode {
	JsonNodeType type;
	JsonNodeValue data;
}


class Parser {
	public:
		Parser(std::unique_ptr<JsonTokenStream> scanner): m_scanner(std::move(scanner)) {}
		
		JsonNode Parse() {}

	private:
		JsonTokenStream m_scanner;		
};


// ------------ End of parser ------------

void runTests(std::vector<std::pair<std::string, bool>>& tests) {
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
}
void testTokenizer() {
	std::vector<std::pair<std::string, bool> > tests = {
		{ "null", true },
		{ "true", true },
		{ "false", true },
		{ "\"meow meow meow\"", true },
		{ "1738", true },
		{ "[ \"fuckity\", null, 7]", true },
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
	
	runTests(tests);
};

int main() {
	testTokenizer();
}
