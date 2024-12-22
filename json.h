#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json
{

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    struct ValuePrinter
    {
        std::ostream &out;
        void operator()(std::nullptr_t);
        void operator()(Array);
        void operator()(Dict);
        void operator()(bool);
        void operator()(int);
        void operator()(double);
        void operator()(std::string);
    };

    class Node
    {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        const Value &GetValue() const;

        Node() = default;
        Node(nullptr_t);
        Node(Array);
        Node(Dict);
        Node(bool);
        Node(int);
        Node(double);
        Node(std::string);

        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsPureDouble() const;
        bool IsDouble() const;
        bool IsString() const;

        const Array &AsArray() const;
        const Dict &AsMap() const;
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string &AsString() const;

    private:
        template <typename T>
        bool Is() const
        {
            return std::holds_alternative<T>(value_);
        }

        template <typename T>
        const T &ExtractValue() const
        {
            if (!Is<T>())
            {
                throw(std::logic_error("value holds different type"));
            }
            return std::get<T>(value_);
        }

        Value value_;
        Array as_array_;
        Dict as_map_;
        int as_int_ = 0;
        std::string as_string_;
    };

    bool operator==(const Node &lft, const Node &rgt);
    bool operator!=(const Node &lft, const Node &rgt);

    class Document
    {
    public:
        explicit Document(Node root);

        const Node &GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream &input);

    void Print(const Document &doc, std::ostream &output);

} // namespace json