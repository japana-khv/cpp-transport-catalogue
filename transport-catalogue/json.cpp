#include "json.h"
#include <exception>
#include <cmath>

using namespace std;

namespace json {

Node LoadNode(istream &input);

Node LoadArray(istream &input) {
    Array result;
    char ch = input.peek();
    if (ch == ',') {
        throw ParsingError("array parsing error : unexpected ',' after '['");
    }
    input >> ch;
    while (input.good() && ch != ']') {
        if (ch != ',') {
            input.putback(ch);
        }
        result.push_back(LoadNode(input));
        input >> ch;
    }
    if (ch != ']') {
        throw ParsingError("array parsing error : end ']' required but not found");
    }

    return Node(move(result));
}

Node LoadString(istream &input) {
    string line;
    char ch = input.get();
    while (input.good() && ch != '"') {
        if (ch == '\\') {
            // special chars parsing
            char next_ch = input.peek(); // see to next char
            switch (next_ch) {
            case 'r':
                ch = '\r';
                input.get();
                break;
            case 'n':
                ch = '\n';
                input.get();
                break;
            case '\\':
                ch = '\\';
                input.get();
                break;
            case 't':
                ch = '\t';
                input.get();
                break;
            case '"':
                ch = '"';
                input.get();
                break;
            }
        }
        line += ch;

        ch = input.get();
    }
    if (ch != '"') {
        throw ParsingError("string parsing error : ending '\"' required but not found");
    }
    return Node(move(line));
}

Node LoadDict(istream &input) {
    Dict result;

    char c = 0;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        if (c != ':') {
            throw ParsingError("Map parsing error : ':' required but not found");
        }
        result.insert( { move(key), LoadNode(input) });
        c = 0;
    }
    if (c != '}') {
        throw ParsingError("Map parsing error : '}' required but not found");
    }

    return Node(move(result));
}
Node LoadNull(istream &input) {
    string line;
    const std::string null("null"s);
    char ch;
    for (size_t i = 0; i < null.size() && (input >> ch); ++i) {
        line += ch;
    }
    if (line != null || !input.good()) {
        throw ParsingError("n char is not null Node");
    }
    return Node();
}

Node LoadBoolTrue(istream &input) {
    string line;
    const std::string null("true"s);
    char ch;
    for (size_t i = 0; i < null.size() && (input >> ch); ++i) {
        line += ch;
    }
    if (line != null || !input.good()) {
        throw ParsingError("t char is not true bool Node");
    }
    return Node(true);
}

Node LoadBoolFalse(istream &input) {
    string line;
    const std::string null("false"s);
    char ch;
    for (size_t i = 0; i < null.size() && (input >> ch); ++i) {
        line += ch;
    }
    if (line != null || !input.good()) {
        throw ParsingError("f char is not false bool Node");
    }
    return Node(false);
}

Node LoadNumber(istream &input) {
// get sign of number
    int sign = input.peek() == '-' ? (input.get(), -1) : 1;

    // read integer part of number
    int result = 0;
    while (isdigit(input.peek())) {
        result *= 10;
        result += input.get() - '0';
    }

    // if we see dot or e E
    char ch = input.peek();
    if (ch == '.' || ch == 'e' || ch == 'E') {
        // we read double
        double result_double = 1.0 * sign * result;
        if (ch == '.') {
            input.get(); // read dot '.'
            // read fractional part
            std::string line("0.");
            while (isdigit(input.peek())) {
                line += input.get();
            }
            if (line.size() > 2) { // we have fractional part
                //append it to integer part
                if (sign > 0) {
                    result_double += std::stod(line);
                } else {
                    result_double -= std::stod(line);
                }
            } else {
                throw ParsingError(" number parsing error : empty fractional part in number  after '.' ");
            }
            ch = input.peek();
        }
        // we have exponent
        if (ch == 'e' || ch == 'E') {
            input.get(); // read e or E
            ch = input.peek();
            // exponent sign
            sign = 1;
            if (ch == '+' || ch == '-') {
                sign = input.get() == '-' ? -1 : 1;
            }
            // read integer exponent part
            result = 0;
            while (isdigit(input.peek())) {
                result *= 10;
                result += input.get() - '0';
            }
            result = result * sign; // exponent was read
            result_double *= std::pow(10.0, result);
        }

        return Node(result_double);

    } else {
        // we have read integer number
        return Node(sign * result);
    }
}

Node LoadNode(istream &input) {
    char c = 0;

    input >> c;
    if (input.eof()) {
        throw ParsingError("unexpected input end or input stream error");
    }

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == 't') {
        input.putback(c);
        return LoadBoolTrue(input);
    } else if (c == 'f') {
        input.putback(c);
        return LoadBoolFalse(input);
    } else if (c == '-' || (isdigit(c))) {
        input.putback(c);
        return LoadNumber(input);
    }

    throw ParsingError("Wrong input");
}

Node::Node(bool value) :
        value_(value) {
}

Node::Node(Array array) :
        value_(move(array)) {
}

Node::Node(Dict map) :
        value_(move(map)) {
}

Node::Node(int value) :
        value_(value) {
}

Node::Node(double value) :
        value_(value) {
}

Node::Node(std::nullptr_t value) :
        value_(value) {
}

Node::Node(const string &value) :
        value_(value) {
}

Document::Document(Node root) :
        root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream &input) {
    return Document { LoadNode(input) };
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}
// Возвращает true, если в Node хранится int либо double.
bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}
// Возвращает true, если в Node хранится double.
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Map>(value_);
}

bool Node::IsDict() const {
    return std::holds_alternative<Dict>(value_);
}

int Node::AsInt() const {
    if (IsInt()) {
        return std::get<int>(value_);
    }
    throw std::logic_error("Value type is not int");
}

bool Node::AsBool() const {
    if (IsBool()) {
        return std::get<bool>(value_);
    }
    throw std::logic_error("Value is type is not bool");
}
//. Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
double Node::AsDouble() const {
    if (IsPureDouble()) {
        return std::get<double>(value_);
    } else if (IsInt()) {
        return static_cast<double>(std::get<int>(value_));
    }
    throw std::logic_error("Value is type is not double or int");
}

const Array& Node::AsArray() const {
    if (IsArray()) {
        return std::get<Array>(value_);
    }
    throw std::logic_error("Value is type is not Array");
}
Array& Node::AsArray() {
    if (IsArray()) {
        return std::get<Array>(value_);
    }
    throw std::logic_error("Value is type is not Array");
}

const Map& Node::AsMap() const {
    if (IsMap()) {
        return std::get<Map>(value_);
    }
    throw std::logic_error("Value is type is not Map");
}

const Dict& Node::AsDict() const {
    if (IsDict()) {
        return std::get<Dict>(value_);
    }
    throw std::logic_error("Value is type is not Map");
}
Map& Node::AsMap() {
    if (IsMap()) {
        return std::get<Map>(value_);
    }
    throw std::logic_error("Value is type is not Map");
}

Dict& Node::AsDict() {
    if (IsDict()) {
        return std::get<Dict>(value_);
    }
    throw std::logic_error("Value is type is not Map");
}

const string& Node::AsString() const {
    if (IsString()) {
        return std::get<std::string>(value_);
    }
    throw std::logic_error("Value is type is not string");
}

void Print(const Document &doc, std::ostream &output) {
    PrintNode(doc.GetRoot(), PrintContext { output });
}

void PrintValue(const std::nullptr_t, PrintContext context) {
    context.os << "null";
}

void PrintValue(bool value, PrintContext context) {
    context.os << (value ? "true"s : "false"s);
}
void PrintValue(const std::string &value, PrintContext context) {
    auto &out = context.os;
    out << "\""s;
    for (const auto ch : value) {
        switch (ch) {
        case '\r': {
            out << "\\r";
            break;
        }
        case '\\': {
            out << "\\\\";
            break;
        }
        case '\n': {
            out << "\\n";
            break;
        }
        case '"': {
            out << "\\\"";
            break;
        }
        default: {
            out << ch;
        }
        }
    }
    out << "\""s;
}

void PrintValue(const Array &value, PrintContext context) {
    auto &out = context.os;
    out << "[";
    out << std::endl;

    auto newContext = context.Indented();
    bool first = true;
    for (const auto &node : value) {
        if (first) {
            first = false;
        } else {
            out << ",";
            out << std::endl;
        }

        newContext.PrintIndent();
        PrintNode(node, newContext);
    }

    out << std::endl;
    context.PrintIndent();
    out << "]";
}

void PrintValue(const Map &value, PrintContext context) {
    auto &out = context.os;

    out << "{" << std::endl;

    auto newContext = context.Indented();
    bool first = true;
    for (const auto& [key, node] : value) {
        if (first) {
            first = false;
        } else {
            out << ",";
            out << std::endl;
        }
        newContext.PrintIndent();
        newContext.os << "\"" << key << "\": ";
        PrintNode(node, newContext);
    }

    out << std::endl;
    context.PrintIndent();
    out << "}";
}

void PrintNode(const Node &node, PrintContext context) {
    std::visit([&context](const auto &value) {
        PrintValue(value, context);
    }, node.GetValue());
}

}  // namespace json

