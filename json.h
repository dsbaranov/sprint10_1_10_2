#pragma once

#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {
using namespace std::literals;

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Value =
    std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

struct ValuePrinter {
  std::ostream& os_;
  void operator()(std::nullptr_t);
  void operator()(Array);
  void operator()(Dict);
  void operator()(bool value);
  void operator()(int value);
  void operator()(double value);
  void operator()(std::string value);
};

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
 public:
  using runtime_error::runtime_error;
};

class Node {
 public:
  const Value& GetValue() const { return value_; }

  Node() = default;
  explicit Node(Array array);
  explicit Node(Dict map);
  explicit Node(int value);
  explicit Node(std::string value);
  explicit Node(double value);
  explicit Node(std::nullptr_t) : value_(nullptr) {}

  int AsInt() const;
  bool AsBool() const;
  double AsDouble() const;
  const std::string& AsString() const;
  const Array& AsArray() const;
  const Dict& AsMap() const;

  bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
  bool IsInt() const { return std::holds_alternative<int>(value_); }
  bool IsDouble() const {
    return IsInt() || std::holds_alternative<double>(value_);
  }
  bool IsPureDouble() const { return std::holds_alternative<double>(value_); }
  bool IsBool() const { return std::holds_alternative<bool>(value_); }
  bool IsString() const { return std::holds_alternative<std::string>(value_); }
  bool IsArray() const { return std::holds_alternative<Array>(value_); }
  bool IsMap() const { return std::holds_alternative<Dict>(value_); }

 private:
  Value value_;
  Array as_array_;
  Dict as_map_;
  int as_int_ = 0;
  std::string as_string_;
};

class Document {
 public:
  explicit Document(Node root);

  const Node& GetRoot() const;

 private:
  Node root_;
};

bool operator==(const Node& lft, const Node& rgt);
bool operator!=(const Node& lft, const Node& rgt);

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json