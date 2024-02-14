#include <__nullptr>
#include <iostream>
#include <sstream>
#include <optional>
#include <variant>

enum JsonSpecialChars {
	LeftParenthesis,
	RightParenthesis,
	LeftBracket,
	RightBracket,
	Comma,
	Null,
};

struct Number {
	std::string val;
};

typedef std::variant<
	std::string,
	JsonSpecialChars,
	Number
> Primitives;

class JsonTokenStream {
	public:

	JsonTokenStream(std::stringstream stream): m_stream(std::move(stream)) {}
	
	bool HasTokens() {
		return m_stream.eof();
	}
	
	// result, valid or invalid
	std::pair<Primitives, bool> Get() {
		// read in all useless whitespace
		char currChar = ' ';
		while (!m_stream.eof() && (currChar = m_stream.get()) == ' ');

		std::variant<std::string, JsonSpecialChars, Number> res;
		if (currChar == ' ') {
			return {res, false};
		}

		// at this point we know we are at a valid non EOF char to be read and have a token created from
		if (currChar == '{') {
			res = LeftParenthesis;
			return { res, true };
		}

		if (currChar == '}') {
			res = RightParenthesis;
			return { res, true };
		}

		if (currChar == '[') {
			res = LeftBracket;
			return { res, true };
		}

		if (currChar == ']') {
			res = RightBracket;
			return { res, true };
		}

		if (currChar == ',') {
			res = Comma;
			return { res, true };
		}
	
		if (currChar == '\"') {
			// start of a string
			std::string jsonStr;
			while (true) {
				if (m_stream.eof())	{
					return { res, false };
				}

				char next = m_stream.get();
				if (next == '\"') {
					// if the last element in our sstream is not an escape character and we have reached second quote 
					// this says we are at end of string
					if (jsonStr.back() != '\\') {
						res = jsonStr;
						return { res, true };
					}
					jsonStr += next;
				}
				
				jsonStr += next;
			}

		}

		// check if null symbol
		if (currChar == 'n') {
			const char nextAssertions[] = {'u', 'l', 'l'};		
			for (int i = 0; i < 3; i++) {
				if (m_stream.eof() || m_stream.get() != nextAssertions[i]) {
					return { res , false };
				}
			}
			res = Null;
			return { res, true };
		}

		// check if start of a number
		if (currChar == '-' || (currChar >= '0' && currChar <= '9')) {
			bool hadMyE = false;
			bool hadMyDecimal = false;	
			std::string numero(1, currChar);
			while (!m_stream.eof()) {
				char newChar = m_stream.peek();

				// based on last number we know if this is valid or not
				// we know that if the last character was a - or . the number after it must be digit
				if ((numero.back() == '-' || numero.back() == '.') &&  (newChar < '0' || newChar > '9')) {
					return { res, false };
				}

				if (numero.back() == 'E' || numero.back() == 'e') {
					// then this should be something that ends a number
					if (newChar == ' ' || newChar == ',' || newChar == ']' || newChar == '}') {
						res = Number {numero};
						return { res, true };
					} else {
						// otherwise we done fucked up
						return { res, false};
					}
				}

				// can add e, E, or number, or .
				if (newChar == 'e' || newChar == 'E' || (newChar >= '0' && newChar <= '9') || newChar == '.') {
					if (newChar == 'e' || newChar == 'E') {
						if (hadMyE)
							return { res, false };
						else
							hadMyE = true;
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

				// whatever we are adding is an invalid addition to ongoing number string
				return { res, false };
			}
			res = numero;
			return { res, true };
		}
		
		// invalid token
		return {res, false};
	}

	private:
		std::stringstream m_stream;	
};

int main() {
	std::stringstream t1("\"fuckity\", \"foo\", \"mama\"");
	JsonTokenStream tokenstrm(std::move(t1));

	while (!tokenstrm.HasTokens()) {
		std::pair<Primitives, bool> tokenRes = tokenstrm.Get();	
		if (tokenRes.second) {
			if (tokenRes.first.index() == 0) {
				std::cout << "String Token " << std::get<std::string>(tokenRes.first) << std::endl;
			} else if (tokenRes.first.index() == 1) {
				std::cout << "SpecialChar Token " << std::get<JsonSpecialChars>(tokenRes.first) << std::endl;
			} else {
				std::cout << "Number Token " << std::get<Number>(tokenRes.first).val << std::endl;
			}
		} else {
			std::cout << "wtf " << std::endl;
			return -1;
		}
	}
};
