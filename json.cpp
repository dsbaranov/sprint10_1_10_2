#include "json.h"

using namespace std;

namespace json {

// ValuePrinter

void ValuePrinter::operator()(std::nullptr_t) { os_ << "null"s; }
void ValuePrinter::operator()(Array array) {
  os_ << "[ "s;
  bool is_first = true;
  for (const auto &node : array) {
    if (!is_first) {
      os_ << ", "s;
    }
    visit(ValuePrinter{os_}, node.GetValue());
    is_first = false;
  }
  os_ << " ]"s;
}
void ValuePrinter::operator()(Dict dict) {
  os_ << "{"s;
  bool is_first = true;
  for (const auto &[key, node] : dict) {
    if (!is_first) {
      os_ << ", "s;
    }
    os_ << "\""s << key << "\" : "s;
    visit(ValuePrinter{os_}, node.GetValue());
    is_first = false;
  }
  os_ << "}"s;
}
void ValuePrinter::operator()(bool value) {
  os_ << (value ? "true"s : "false"s);
}
void ValuePrinter::operator()(int value) { os_ << value; }
void ValuePrinter::operator()(double value) { os_ << value; }
void ValuePrinter::operator()(std::string value) {
  os_ << "\""sv;
  for (const char c : value) {
    if (c == '\n') {
      os_ << "\\n"s;
    } else if (c == '\r') {
      os_ << "\\r"s;
    } else if (c == '\"') {
      os_ << "\\\""s;
    } else if (c == '\t') {
      os_ << "\\t"s;
    } else if (c == '\\') {
      os_ << "\\\\"s;
    } else {
      os_ << c;
    }
  }
  os_ << "\""sv;
}

// Node

Node LoadNode(istream &input);

Node LoadArray(istream &input) {
  if (!input) {
    throw(ParsingError("Empty [] input"));
  }
  Array result;
  while (input) {
    for (char c; input >> c && c != ']';) {
      if (c != ',') {
        input.putback(c);
      }
      result.push_back(LoadNode(input));
    }
  }
  if (result.empty()) {
    throw(ParsingError("Incorrect []"));
  }
  return Node(move(result));
}

Node LoadInt(istream &input) {
  int result = 0;
  while (isdigit(input.peek())) {
    result *= 10;
    result += input.get() - '0';
  }
  return Node(result);
}

Number LoadNumber(std::istream &input) {
  using namespace std::literals;

  std::string parsed_num;

  // Считывает в parsed_num очередной символ из input
  auto read_char = [&parsed_num, &input] {
    parsed_num += static_cast<char>(input.get());
    if (!input) {
      throw ParsingError("Failed to read number from stream"s);
    }
  };

  // Считывает одну или более цифр в parsed_num из input
  auto read_digits = [&input, read_char] {
    if (!std::isdigit(input.peek())) {
      throw ParsingError("A digit is expected"s);
    }
    while (std::isdigit(input.peek())) {
      read_char();
    }
  };

  if (input.peek() == '-') {
    read_char();
  }
  // Парсим целую часть числа
  if (input.peek() == '0') {
    read_char();
    // После 0 в JSON не могут идти другие цифры
  } else {
    read_digits();
  }

  bool is_int = true;
  // Парсим дробную часть числа
  if (input.peek() == '.') {
    read_char();
    read_digits();
    is_int = false;
  }

  // Парсим экспоненциальную часть числа
  if (int ch = input.peek(); ch == 'e' || ch == 'E') {
    read_char();
    if (ch = input.peek(); ch == '+' || ch == '-') {
      read_char();
    }
    read_digits();
    is_int = false;
  }

  try {
    if (is_int) {
      // Сначала пробуем преобразовать строку в int
      try {
        return std::stoi(parsed_num);
      } catch (...) {
        // В случае неудачи, например, при переполнении,
        // код ниже попробует преобразовать строку в double
      }
    }
    return std::stod(parsed_num);
  } catch (...) {
    throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
  }
}

std::string LoadString(std::istream &input) {
  using namespace std::literals;

  auto it = std::istreambuf_iterator<char>(input);
  auto end = std::istreambuf_iterator<char>();
  std::string s;
  while (true) {
    if (it == end) {
      // Поток закончился до того, как встретили закрывающую кавычку?
      throw ParsingError("String parsing error");
    }
    const char ch = *it;
    if (ch == '"') {
      // Встретили закрывающую кавычку
      ++it;
      break;
    } else if (ch == '\\') {
      // Встретили начало escape-последовательности
      ++it;
      if (it == end) {
        // Поток завершился сразу после символа обратной косой черты
        throw ParsingError("String parsing error");
      }
      const char escaped_char = *(it);
      // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
      switch (escaped_char) {
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
    } else if (ch == '\n' || ch == '\r') {
      // Строковый литерал внутри- JSON не может прерываться символами \r или \n
      throw ParsingError("Unexpected end of line"s);
    } else {
      // Просто считываем очередной символ и помещаем его в результирующую
      // строку
      s.push_back(ch);
    }
    ++it;
  }

  return s;
}

Node LoadDict(istream &input) {
  if (!input) {
    throw(ParsingError("Empty {} input"));
  }
  Dict result;
  for (char c; input >> c && c != '}';) {
    if (c == ',') {
      input >> c;
    }

    string key = LoadString(input);
    input >> c;
    result.insert({move(key), LoadNode(input)});
  }
  if (result.empty()) {
    throw(ParsingError("Wrong {} input"));
  }
  return Node(move(result));
}

Node LoadNode(istream &input) {
  char c;
  input >> c;
  if (c == '[') {
    return LoadArray(input);
  } else if (c == '{') {
    return LoadDict(input);
  } else if (c == '"' || c == '\n' || c == '\t' || c == '\\' || c == 'r') {
    return Node(LoadString(input));
  } else if (c == 'n') {
    input.putback(c);
    std::string line;
    getline(input, line);
    if (line.find("null"s) == std::string::npos) {
      throw ParsingError("wrong null-like value");
    }
    return Node();
  } else if (c == 't') {
    input.putback(c);
    std::string line;
    getline(input, line);
    if (line.find("true"s) == std::string::npos) {
      throw ParsingError("wrong true-like value");
    }
    return Node(true);
  } else if (c == 'f') {
    input.putback(c);
    std::string line;
    getline(input, line);
    if (line.find("false"s) == std::string::npos) {
      throw ParsingError("wrong false-like value");
    }
    return Node(false);
  } else {
    input.putback(c);
    auto number = LoadNumber(input);
    if (std::holds_alternative<double>(number)) {
      return Node(get<double>(number));
    } else {
      return Node(get<int>(number));
    }
  }
}

Node::Node(Array array) { value_ = std::move(array); }

Node::Node(Dict map) : value_(move(map)) {}

Node::Node(int value) : value_(value) {}

Node::Node(string value) : value_(move(value)) {}

Node::Node(double value) : value_(move(value)) {}

Node::Node(std::nullptr_t) : value_(nullptr) {}

Node::Node(bool value) : value_(value) {}

int Node::AsInt() const { return ExtractValue<int>(); }
bool Node::AsBool() const { return ExtractValue<bool>(); }
double Node::AsDouble() const {
  if (!IsPureDouble()) {
    if (IsInt()) {
      return static_cast<double>(get<int>(value_));
    } else {
      throw std::logic_error("Wrong type");
    }
  }
  return get<double>(value_);
}
const std::string &Node::AsString() const {
  return ExtractValue<std::string>();
}
const Array &Node::AsArray() const { return ExtractValue<Array>(); }
const Dict &Node::AsMap() const { return ExtractValue<Dict>(); }

Document::Document(Node root) : root_(move(root)) {}

const Node &Document::GetRoot() const { return root_; }

bool operator==(const Node &lft, const Node &rgt) {
  return lft.IsArray() == rgt.IsArray() && lft.IsBool() == rgt.IsBool() &&
         lft.IsDouble() == rgt.IsDouble() && lft.IsInt() == rgt.IsInt() &&
         lft.IsMap() == rgt.IsMap() && lft.IsNull() == rgt.IsNull() &&
         lft.IsPureDouble() == rgt.IsPureDouble() &&
         lft.IsString() == rgt.IsString() && lft.GetValue() == rgt.GetValue();
}
bool operator!=(const Node &lft, const Node &rgt) { return !(lft == rgt); }

Document Load(istream &input) { return Document{LoadNode(input)}; }

void Print(const Document &doc, std::ostream &output) {
  std::visit(ValuePrinter{output}, doc.GetRoot().GetValue());
}

}  // namespace json