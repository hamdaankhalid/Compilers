#include <iostream>

namespace CharacterCategories {

namespace CharacterEscapes {
const char escapeChar = '\\';
const char tab = 't';
const char carriageReturn = 'r';
const char newLine = 'n';
} // namespace CharacterEscapes

namespace CharacterClasses {

const char negation = '^';
const char wildcard = '.';
const char anyWord = 'w';
const char notAnyWord = 'W';
const char whiteSpace = 's';
const char nonWhiteSpace = 'S';
const char decimalDigit = 'd';
const char nonDecimalDigit = 'D';

template <typename T> class CharacterGroup {
  std::vector<T> innerGroup;
  
  bool isMatch(T candidate) { return true; }

  bool isNotMatch(T candidate) { return false; }
};
} // namespace CharacterClasses

namespace Anchors {
	const char startOfStringMultiLine = '^';
	const char endOfStringMultiLine = '$';
	const char startOfStringSingleLine = 'A';
	const char endOfStringSingleLine = 'Z';
} // namespace Anchors

namespace Quantifiers {
	const char zeroOrMore = '*';
	const char oneOrMore = '+';
	const char zeroOrOne = '?';
} // namespace Quantifiers
}; // namespace CharacterCategories

// Read a string and create a vector of tokens

class RegexEngine {
public:
  static RegexEngine from(const std::string &expr) {
    RegexEngine ex;
    return ex;
  }

  bool is_match(const std::string &expr) { return true; }
};

int main() { 
	const std::string phoneNumExpr = "\d{3}\-\d{3}-\d{4}";
	const std::string emailExpr = "(\d|\w\@(\d|\w)\.\c\o\m";

	std::cout << "la la lala" << std::endl; 
}
