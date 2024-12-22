#include "json.h"

using namespace std;

namespace json
{

    namespace
    {
        using Number = std::variant<int, double>;

        Node LoadNode(istream &input);

        Node LoadArray(istream &input)
        {
            Array result;
            char c;

            for (; input >> c && c != ']';)
            {
                if (c != ',')
                {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']')
            {
                throw ParsingError("Unexpected end if Array");
            }
            return Node(move(result));
        }

        std::string ParseString(istream &input)
        {
            string line;
            while (isalpha(input.peek()))
            {
                line.push_back(input.get());
            }
            return line;
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString(std::istream &input)
        {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true)
            {
                if (it == end)
                {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"')
                {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\')
                {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end)
                    {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char)
                    {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r')
                {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else
                {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadDict(istream &input)
        {
            Dict result;
            char c;
            for (; input >> c && c != '}';)
            {
                if (c == ',')
                {
                    input >> c;
                }

                string key = Node(LoadString(input)).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }
            if (c != '}')
            {
                throw ParsingError("Unexpected end of Dict");
            }

            return Node(move(result));
        }
        Number LoadNumber(std::istream &input)
        {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input]
            {
                parsed_num += static_cast<char>(input.get());
                if (!input)
                {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char]
            {
                char peek_ = input.peek();
                if (!std::isdigit(peek_))
                {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek()))
                {
                    read_char();
                }
            };

            if (input.peek() == '-')
            {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0')
            {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else
            {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.')
            {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E')
            {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-')
                {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try
            {
                if (is_int)
                {
                    // Сначала пробуем преобразовать строку в int
                    try
                    {
                        return std::stoi(parsed_num);
                    }
                    catch (...)
                    {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...)
            {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadNode(istream &input)
        {
            char c;
            input >> c;

            if (c == '[')
            {
                return LoadArray(input);
            }
            else if (c == '{')
            {
                return LoadDict(input);
            }
            else if (c == '"')
            {
                // FROM HERE
                return Node(LoadString(input));
            }
            else if (isalpha(c))
            {
                input.putback(c);
                std::string word = ParseString(input);
                if (word == "null"sv)
                {
                    return Node(nullptr);
                }
                else if (word == "true"sv)
                {
                    return Node(true);
                }
                else if (word == "false"sv)
                {
                    return Node(false);
                }
                else
                {
                    throw ParsingError("Couldn't parse null, true or false");
                }
            }
            else if (isdigit(c) || c == '-')
            {
                input.putback(c);
                auto number = LoadNumber(input);
                if (std::holds_alternative<int>(number))
                {
                    return Node(get<int>(number));
                }
                else if (std::holds_alternative<double>(number))
                {
                    return Node(get<double>(number));
                }
                throw ParsingError("Couldn't recognize number");
            }
            else
            {
                throw ParsingError("Couldn't recognize type");
            }
        }

    } // namespace

    void ValuePrinter::operator()(std::nullptr_t)
    {
        out << "null"s;
    }
    void ValuePrinter::operator()(Array array)
    {
        out << "["s;
        bool is_first = true;
        for (const auto &node : array)
        {
            if (!is_first)
            {
                out << ","s;
            }
            visit(ValuePrinter{out}, node.GetValue());
            is_first = false;
        }
        out << "]"s;
    }
    void ValuePrinter::operator()(Dict dict)
    {
        out << "{"s;
        bool is_first = true;
        for (const auto &[key, node] : dict)
        {
            if (!is_first)
            {
                out << ","s;
            }
            out << "\""s << key << "\":"s;
            visit(ValuePrinter{out}, node.GetValue());
            is_first = false;
        }
        out << "}"s;
    }
    void ValuePrinter::operator()(bool value)
    {
        out << (value ? "true"s : "false"s);
    }
    void ValuePrinter::operator()(int value) { out << value; }
    void ValuePrinter::operator()(double value) { out << value; }
    void ValuePrinter::operator()(std::string value)
    {
        out << "\""sv;
        for (char c : value)
        {
            if (c == '\\')
            {
                out << "\\\\"s;
            }
            else if (c == '\"')
            {
                out << "\\\""sv;
            }
            else if (c == '\r')
            {
                out << "\\r"sv;
            }
            else if (c == '\n')
            {
                out << "\\n"sv;
            }
            else if (c == '\t')
            {
                out << "\\t"sv;
            }
            else
            {
                out << c;
            }
        }
        out << "\""sv;
    }

    const Node::Value &Node::GetValue() const
    {
        return value_;
    }

    // Node::Ctors
    Node::Node(nullptr_t) : value_(nullptr) {}
    Node::Node(Array array) : value_(move(array)) {}
    Node::Node(Dict map) : value_(move(map)) {}
    Node::Node(bool value) : value_(value) {}
    Node::Node(int value) : value_(value) {}
    Node::Node(double value) : value_(value) {}
    Node::Node(string value) : value_(move(value)) {}

    // Node::Is
    bool Node::IsNull() const { return Is<nullptr_t>(); }
    bool Node::IsArray() const { return Is<Array>(); }
    bool Node::IsMap() const { return Is<Dict>(); }
    bool Node::IsBool() const { return Is<bool>(); }
    bool Node::IsInt() const { return Is<int>(); }
    bool Node::IsPureDouble() const { return Is<double>(); }
    bool Node::IsDouble() const { return IsPureDouble() || Is<int>(); }
    bool Node::IsString() const { return Is<std::string>(); }

    // Node::As
    const Array &Node::AsArray() const { return ExtractValue<Array>(); }
    const Dict &Node::AsMap() const { return ExtractValue<Dict>(); }
    bool Node::AsBool() const { return ExtractValue<bool>(); }
    int Node::AsInt() const { return ExtractValue<int>(); }
    double Node::AsDouble() const
    {
        if (Is<int>())
        {
            return static_cast<double>(AsInt());
        }
        return ExtractValue<double>();
    }
    const std::string &Node::AsString() const
    {
        return ExtractValue<std::string>();
    }

    bool operator==(const Node &lft, const Node &rgt)
    {
        return lft.IsNull() == rgt.IsNull() &&
               lft.IsArray() == rgt.IsArray() &&
               lft.IsMap() == rgt.IsMap() &&
               lft.IsBool() == rgt.IsBool() &&
               lft.IsPureDouble() == rgt.IsPureDouble() &&
               lft.IsInt() == rgt.IsInt() &&
               lft.IsString() == rgt.IsString() &&
               lft.GetValue() == rgt.GetValue();
    }
    bool operator!=(const Node &lft, const Node &rgt) { return !(lft == rgt); }

    Document::Document(Node root)
        : root_(move(root))
    {
    }

    const Node &Document::GetRoot() const
    {
        return root_;
    }

    Document Load(istream &input)
    {
        return Document{LoadNode(input)};
    }

    void Print(const Document &doc, std::ostream &output)
    {
        visit(ValuePrinter{output}, doc.GetRoot().GetValue());
    }

} // namespace json