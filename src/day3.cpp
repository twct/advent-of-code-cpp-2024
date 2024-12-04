#include "app.h"
#include "result.h"

enum class TokenType { Identifier, NumberLiteral, Operator, Punctuation, Eof };

class TokenizeError {
  public:
    template<typename FmtStr, typename... Args>
    TokenizeError(size_t line, size_t column, FmtStr&& format, Args&&... args) :
        m_line(line),
        m_column(column),
        m_message(fmt::format(format, std::forward<Args>(args)...)) {}

    std::string_view message() const {
        return m_message;
    }

    operator std::string() const {
        return m_message;
    }

    size_t line() const {
        return m_line;
    }

    size_t column() const {
        return m_column;
    }

  private:
    std::string m_message;
    size_t m_line;
    size_t m_column;
};

struct Token {
    TokenType type;
    std::string lexeme;
    size_t line;
    size_t column;

    Token(
        TokenType type,
        const std::string& lexeme,
        size_t line,
        size_t column
    ) :
        type(type),
        lexeme(lexeme),
        line(line),
        column(column) {}
};

class Tokenizer {
  public:
    Tokenizer() = default;
    Tokenizer(Tokenizer&&) noexcept = default;
    Tokenizer& operator=(Tokenizer&&) noexcept = default;

    Result<std::vector<Token>, TokenizeError> tokenize(const std::string& code
    ) {
        m_line = 1;
        m_column = 1;
        m_current = 0;
        m_tokens.clear();

        TRY(scan_tokens(code));

        m_tokens.emplace_back(TokenType::Eof, "", m_line, m_column);

        return m_tokens;
    }

  private:
    size_t m_line {0};
    size_t m_column {0};
    size_t m_current {0};
    std::vector<Token> m_tokens;

    bool is_at_end(const std::string& code) const {
        return m_current >= code.length();
    }

    char peek(const std::string& code) {
        if (is_at_end(code)) {
            return '\0';
        }
        return code[m_current];
    }

    char peek_next(const std::string& code) const {
        if (m_current + 1 >= code.length()) {
            return '\0';
        }
        return code[m_current + 1];
    }

    char advance(const std::string& code) {
        char c = code[m_current];
        m_current++;
        if (c == '\n') {
            m_line++;
            m_column = 1;
        } else {
            m_column++;
        }
        return c;
    }

    void add_token(TokenType type, const std::string& lexeme) {
        m_tokens.emplace_back(type, lexeme, m_line, m_column - lexeme.length());
    }

    void skip_whitespace(const std::string& code) {
        while (!is_at_end(code)) {
            char c = peek(code);
            if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
                advance(code);
            } else {
                break;
            }
        }
    }

    Result<Unit, TokenizeError> scan_tokens(const std::string& code) {
        while (!is_at_end(code)) {
            skip_whitespace(code);
            if (is_at_end(code)) {
                break;
            }

            size_t token_start = m_current;
            char c = peek(code);

            if (std::isalpha(c) || c == '_') {
                auto res = identifier(code);
                if (res.is_err()) {
                    return res.unwrap_err();
                }
            } else if (std::isdigit(c)) {
                auto res = number(code);
                if (res.is_err()) {
                    return res.unwrap_err();
                }
            } else if (std::ispunct(c)) {
                auto res = operator_or_punct(code);
                if (res.is_err()) {
                    return res.unwrap_err();
                }
            } else {
                return TokenizeError(
                    m_line,
                    m_column,
                    "Unexepcted character: '{}'",
                    c
                );
            }
        }

        return Unit {};
    }

    Result<Unit, TokenizeError> identifier(const std::string& code) {
        size_t start = m_current;
        while (!is_at_end(code)
               && (std::isalnum(peek(code)) || peek(code) == '_')) {
            advance(code);
        }
        std::string lexeme = code.substr(start, m_current - start);

        add_token(TokenType::Identifier, lexeme);

        return Unit {};
    }

    Result<Unit, TokenizeError> number(const std::string& code) {
        size_t start = m_current;

        while (!is_at_end(code) && std::isdigit(peek(code))) {
            advance(code);
        }

        std::string lexeme = code.substr(start, m_current - start);
        add_token(TokenType::NumberLiteral, lexeme);

        return Unit {};
    }

    Result<Unit, TokenizeError> operator_or_punct(const std::string& code) {
        char c = peek(code);
        std::string lexeme(1, c);
        advance(code);

        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&'
            || c == '|' || c == '^' || c == '~' || c == '@' || c == ':'
            || c == '$' || c == '?' || c == '#' || c == '>' || c == '<'
            || c == '!') {
            add_token(TokenType::Operator, lexeme);

            return Unit {};
        }

        if (c == ';' || c == ',' || c == '\'' || c == '.' || c == '('
            || c == ')' || c == '{' || c == '}' || c == '[' || c == ']'
            || c == '\"') {
            add_token(TokenType::Punctuation, lexeme);

            return Unit {};
        }

        return TokenizeError(
            m_line,
            m_column,
            "Unrecognized character: {}",
            lexeme
        );
    }
};

const char* token_to_str(TokenType type) {
    switch (type) {
        case TokenType::Identifier:
            return "Identifier";
        case TokenType::NumberLiteral:
            return "NumberLiteral";
        case TokenType::Operator:
            return "Operator";
        case TokenType::Punctuation:
            return "Punctuation";
        case TokenType::Eof:
            return "Eof";
    }
}

void print_tokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        spdlog::info(
            "Token({}, \"{}\", {}, {})",
            token_to_str(token.type),
            token.lexeme,
            token.line,
            token.column
        );
    }
}

bool ends_with(const std::string& value, const std::string& ending) {
    if (ending.size() > value.size()) {
        return false;
    }
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

class Parser {
  public:
    Parser() : m_amount(0) {}

    void parse(const std::vector<Token>& tokens) {
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (i + 2 < tokens.size() && tokens[i].type == TokenType::Identifier
                && tokens[i].lexeme == "do"
                && tokens[i + 1].type == TokenType::Punctuation
                && tokens[i + 1].lexeme == "("
                && tokens[i + 2].type == TokenType::Punctuation
                && tokens[i + 2].lexeme == ")") {
                m_enabled = true;
            }

            if (i + 5 < tokens.size() && tokens[i].type == TokenType::Identifier
                && tokens[i + 1].type == TokenType::Punctuation
                && tokens[i + 1].lexeme == "'"
                && tokens[i + 2].type == TokenType::Identifier
                && tokens[i + 2].lexeme == "t"
                && tokens[i + 3].type == TokenType::Punctuation
                && tokens[i + 3].lexeme == "("
                && tokens[i + 4].type == TokenType::Punctuation
                && tokens[i + 4].lexeme == ")") {
                m_enabled = false;
            }

            if (i + 4 < tokens.size() && tokens[i].type == TokenType::Identifier
                && ends_with(tokens[i].lexeme, "mul")
                && tokens[i + 1].type == TokenType::Punctuation
                && tokens[i + 1].lexeme == "("
                && tokens[i + 2].type == TokenType::NumberLiteral
                && tokens[i + 3].type == TokenType::Punctuation
                && tokens[i + 3].lexeme == ","
                && tokens[i + 4].type == TokenType::NumberLiteral) {
                if (i + 5 < tokens.size()
                    && tokens[i + 5].type == TokenType::Punctuation
                    && tokens[i + 5].lexeme == ")") {
                    int num1 = std::stoi(tokens[i + 2].lexeme);
                    int num2 = std::stoi(tokens[i + 4].lexeme);

                    if (m_enabled) {
                        m_amount += num1 * num2;
                    }

                    m_uncorrected_amount += num1 * num2;
                }
            }
        }
    }

    int amount() const {
        return m_amount;
    }

    int uncorrected_amount() const {
        return m_uncorrected_amount;
    }

  private:
    bool m_enabled {true};
    int m_amount {0};
    int m_uncorrected_amount {0};
};

int day3(const std::optional<std::string>& maybe_input) {
    std::string input = maybe_input
        ? *maybe_input
        : "xmul(2,4)%&mul[3,7]!@^do_not_mul(5,5)+mul(32,64]then(mul(11,8)mul(8,5))";

    Tokenizer tokenizer;
    auto maybe_tokens = tokenizer.tokenize(input);

    if (maybe_tokens.is_err()) {
        auto& err = maybe_tokens.unwrap_err();
        spdlog::error(
            "Failed to tokenize: {}\n {}:{}",
            err.message(),
            err.line(),
            err.column()
        );
        return 1;
    }

    auto tokens = maybe_tokens.unwrap_ok();

    // print_tokens(tokens);

    Parser parser;
    parser.parse(tokens);

    spdlog::info("Uncorrected amount: {}", parser.uncorrected_amount());
    spdlog::info("Corrected amount: {}", parser.amount());

    return 0;
}

int main(int argc, char** argv) {
    return App(day3).run(argc, argv);
}
