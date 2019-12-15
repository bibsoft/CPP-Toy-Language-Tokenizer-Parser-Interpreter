//Aviv Maman
#include <cctype>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <utility>

using namespace std;

//http://www.sci.brooklyn.cuny.edu/~zhou/teaching/cis3160/project.html
/*The following defines a simple language, in which a program consists of assignments and each variable
is assumed to be of the integer type. For the sake of simplicity, only operators that give integer values are included.
 Write an interpreter for the language in a language of your choice. Your interpreter should be able to do the
  following for a given program: (1) detect syntax errors; (2) report uninitialized variables; and (3) perform the assignments
  if there is no error and print out the values of all the variables after all the assignments are done.
*/

enum Token {
	literal, identifier, plus_symbol, minus_symbol, times_symbol, equals, lparen, rparen, error, eof, newline, terminal
};

struct Lexeme {
    Token token;
    string word;
};

/*
Program:    Assignment*
Assignment: Identifier = Exp;

Exp: Term Exp' | Term
Exp': + Exp | - Exp

Term:   Fact Term'  | Fact
Term': * Term

Fact:   ( Exp ) | - Fact | + Fact | Literal | Identifier
Identifier:         Letter [Letter | Digit]*
Letter: a|...|z|A|...|Z|_
Literal:    0 | NonZeroDigit Digit*
NonZeroDigit:   1|...|9
Digit:  0|1|...|9
*/

//method to output enum members neatly
string translate(Token t) {
    switch(t) {
        case literal: return "literal token"; break;
        case identifier: return "identifier token"; break;
        case plus_symbol: return "+ token"; break;
        case minus_symbol: return "- token"; break;
        case times_symbol: return "* token"; break;
        case lparen: return "( token"; break;
        case rparen: return ") token"; break;
        case equals: return "= token"; break;
        case error: return "error token\n"; break;
        case eof: return "eof token"; break;
        case newline: return "\n"; break;
        case terminal: return "terminal token"; break;
    }
    return "bad token";
}

class Lexer {
	private:
		ifstream inFile;
		pair<Token, string> curr_token;
		pair<Token, string> get_token() {
			char c;
			string word, line;
            while(inFile.get(c)) {
                if(isdigit(c)) {
                    if(c == '0') {
                        char next = inFile.peek();
                        if(isdigit(next)) { //001 is not valid
                            getline(inFile, line);
                            return make_pair(error, "error"); //Literal:  0 | NonZeroDigit Digit*
                        }
                        else return make_pair(literal, "0");
                    }
                    else if(c >= '1' && c <= '9') { //Literal:	0 | NonZeroDigit Digit*
                        while(isdigit(c)) {
                            word += c;
                            inFile.get(c);
                        }
                        inFile.putback(c);
                        return make_pair(literal, word);
                    }
                }
                else if(isalpha(c) || c == '_') { //Identifier:         Letter [Letter | Digit]*
                    while(isalnum(c) || c == '_') {
                        word += c;
                        inFile.get(c);
                    }
                    inFile.putback(c);
                    return make_pair(identifier, word);
                }
                else if(c == '+') return make_pair(plus_symbol, "+");
                else if(c == '-') return make_pair(minus_symbol, "-");
                else if(c == '*') return make_pair(times_symbol, "*");
                else if(c == '=') return make_pair(equals, "=");
                else if(c == '(') return make_pair(lparen, "(");
                else if(c == ')') return make_pair(rparen, ")");
                else if(c == ';') return make_pair(terminal, ";");
            }
            return make_pair(eof, "End of file");
		}
    public:
        Lexer() = delete;
        Lexer(string filename) {
            inFile.open(filename);
        }
        ~Lexer() {
            inFile.close();
        }
        deque<pair<Token, string>> tokenizer() {
            deque<pair<Token, string>> t;
            curr_token = get_token();
            while(curr_token.first != eof) {
                t.push_back(curr_token);
                curr_token = get_token();
            }
            t.push_back(make_pair(eof, "End of file"));
            return t;
		}
};

class Parser {
    private:
        deque<pair<Token, string>> tokens;
        pair<Token, string> curr_token;
        std::map<std::string, int> state;

     public:
        Parser(deque<pair<Token, string>> tokens) {
            this->tokens = tokens;
        }
        void get_token() {
            curr_token = tokens.front();
            tokens.pop_front();
        }
        //pattern matching method. if encounters a match, we proceed to pop the next token off the queue and store it in curr_token
        void match(Token t) {
            if(curr_token.first == t) {
                get_token();
            }
            else {
                cout << "Error: Expected token: " << translate(t) << ". Given token: " << translate(curr_token.first) << "\n";
                exit(-1);
            }
        }
        //method to print out all Token/Word pairs
        void printSymbolTable() {
            for(pair<Token, string> r : tokens) {
                cout << r.second << "\t" << setw(20) << translate(r.first) << endl;
            }
        }

        //Program:    Assignment*
        void program() {
            get_token();
            while(!tokens.empty()) {
                assignment();
            }
            for (auto const& x : state)
            {
                cout << x.first  // string (key)
                          << " = "
                          << x.second // string's value
                          << endl ;
            }
            //cout << "done\n";
        }

        //Assignment: Identifier = Exp;
        void assignment() {
            if(curr_token.first != identifier) {
                cout << "Not a valid program\n"; exit(-1);
            }
            else {
                string foo = curr_token.second;
                match(identifier);
                match(equals);
                int x = exp();
                state[foo] = x;
                match(terminal);
            }
        }

        //Exp': + Exp | - Exp
        int expPrime() {
            if(curr_token.first == plus_symbol) {
                match(plus_symbol);
                return exp();
            }
            else if(curr_token.first == minus_symbol) {
                match(minus_symbol);
                return -exp();
            }
        }

        //Exp: Term Exp' | Term
        int exp() {
            int i = term();
            if(curr_token.first == plus_symbol || curr_token.first == minus_symbol) {
                i += expPrime();
            }
            return i;
        }

        //Term:   Fact Term'  | Fact
        int term() {
            int i = fact();
            if(curr_token.first == times_symbol) {
                i *= termPrime();
            }
            return i;
        }

        //Term': * Term
        int termPrime() {
            match(times_symbol);
            return term();
        }

        //Fact:   ( Exp ) | - Fact | + Fact | Literal | Identifier
        int fact() {
            if(curr_token.first == lparen) {
                match(lparen);
                int i = exp();
                match(rparen);
                return i;
            }
            else if(curr_token.first == minus_symbol) {
                match(minus_symbol);
                return -fact();
            }
            else if(curr_token.first == plus_symbol) {
                match(plus_symbol);
                return fact();
            }
            else if(curr_token.first == literal) {
                int i = stoi(curr_token.second);
                match(literal);
                return i;
            }
            else if(curr_token.first == identifier) {
                if(state.find(curr_token.second) == state.end()) { //variable not defined
                    match(terminal);
                    cout << "Variable " << curr_token.second << " was not found.\n"; exit(-1);
                }
                string tmp = curr_token.second;
                match(identifier);
                return state[tmp];
            }
        }
};

int main() {
    string filename = "testfile.txt";
    /*
    x = 1;
    y = 2;
    z = ---(x+y)*(x+-y); */
    Lexer p(filename);
    deque<pair<Token, string>> t = p.tokenizer();
    Parser q(t);
    q.program();

	return 0;
}
