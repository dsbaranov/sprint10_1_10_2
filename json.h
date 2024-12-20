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

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
 public:
  using runtime_error::runtime_error;
};

using Number = std::variant<int, double>;
Number LoadNumber(std::istream &input);
// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString(std::istream &input);

struct ValuePrinter {
  std::ostream &os_;
  void operator()(std::nullptr_t);
  void operator()(Array);
  void operator()(Dict);
  void operator()(bool value);
  void operator()(int value);
  void operator()(double value);
  void operator()(std::string value);
};

class Node {
 public:
  using Value =
      std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
  const Value &GetValue() const { return value_; }

  Node() = default;
  Node(Node &&) = default;
  Node(const Node &) = default;

  Node(Array array);
  Node(Dict map);
  Node(int value);
  Node(std::string value);
  Node(double value);
  Node(std::nullptr_t);
  Node(bool value);

  int AsInt() const;
  bool AsBool() const;
  double AsDouble() const;
  const std::string &AsString() const;
  const Array &AsArray() const;
  const Dict &AsMap() const;

  bool IsNull() const { return IsType<std::nullptr_t>(); }
  bool IsInt() const { return IsType<int>(); }
  bool IsPureDouble() const { return IsType<double>(); }
  bool IsDouble() const { return IsInt() || IsPureDouble(); }
  bool IsBool() const { return IsType<bool>(); }
  bool IsString() const { return IsType<std::string>(); }
  bool IsArray() const { return IsType<Array>(); }
  bool IsMap() const { return IsType<Dict>(); }

 private:
  template <typename T>
  const T &ExtractValue() const {
    if (!IsType<T>()) {
      throw std::logic_error("Wrong type");
    }
    return std::get<T>(value_);
  }

  template <typename T>
  bool IsType() const {
    return std::holds_alternative<T>(value_);
  }

  Value value_;
};

class Document {
 public:
  explicit Document(Node root);

  const Node &GetRoot() const;

 private:
  Node root_;
};

bool operator==(const Node &lft, const Node &rgt);
bool operator!=(const Node &lft, const Node &rgt);

Document Load(std::istream &input);

void Print(const Document &doc, std::ostream &output);

}  // namespace json