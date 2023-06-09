#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Map = Dict;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError: public std::runtime_error {
public:
    using runtime_error::runtime_error;
    ParsingError(const std::string &why) :
            runtime_error(why) {
    }
};

using Value =std::variant<std::nullptr_t, bool, double, int, json::Array, json::Dict,
std::string>;

class Node {
public:
    Node() = default;
    Node(Array array);
    Node(Dict map);
    Node(int value);
    Node(double value);
    Node(bool value);
    Node(const std::string &value);
    Node(std::nullptr_t value);

    // variant
    bool IsInt() const;
    // Возвращает true, если в Node хранится int либо double.
    bool IsDouble() const;
    // Возвращает true, если в Node хранится double.
    bool IsPureDouble() const;
    bool IsNull() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsArray() const;
    bool IsMap() const;
    bool IsDict() const;

    int AsInt() const;
    bool AsBool() const;
    //. Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    Array& AsArray();
    const Map& AsMap() const;
    const Dict& AsDict() const;
    Map& AsMap();
    Dict& AsDict();

    const Value& GetValue() const {
        return value_;
    }

    friend bool operator==(const Node &lhs, const Node &rhs);

private:
    Value value_;
};

inline bool operator==(const Node &lhs, const Node &rhs) {
    return lhs.value_ == rhs.value_;
}
inline bool operator!=(const Node &lhs, const Node &rhs) {
    return !(lhs == rhs);
}

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};
inline bool operator==(const Document &lhs, const Document &rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}
inline bool operator!=(const Document &lhs, const Document &rhs) {
    return !(lhs == rhs);
}

struct PrintContext {
    std::ostream &os;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() {
        for (int i = 0; i < indent; ++i) {
            os.put(' ');
        }
    }

    PrintContext Indented() {
        return {os, indent_step, indent + indent_step};
    }
};

Document Load(std::istream &input);

void Print(const Document &doc, std::ostream &output);
void PrintValue(std::nullptr_t, PrintContext context);
void PrintValue(bool value, PrintContext context);
void PrintValue(const std::string &value, PrintContext context);
void PrintValue(const Array &value, PrintContext context);
void PrintValue(const Map &value, PrintContext context);

template<typename T>
void PrintValue(const T &value, PrintContext context) {
    context.os << value;
}

void PrintNode(const Node &node, PrintContext context);

}  // namespace json
