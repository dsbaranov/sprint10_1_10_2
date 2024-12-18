#pragma once

#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json
{
  using namespace std::literals;

  class Node;
  // Сохраните объявления Dict и Array без изменения
  using Dict = std::map<std::string, Node>;
  using Array = std::vector<Node>;
  using Value =
      std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

  std::string FormatString(std::string_view std);

  struct ValuePrinter
  {
    std::ostream &os_;
    void operator()(std::nullptr_t);
    void operator()(Array);
    void operator()(Dict);
    void operator()(bool value);
    void operator()(int value);
    void operator()(double value);
    void operator()(std::string value);
  };

  // Эта ошибка должна выбрасываться при ошибках парсинга JSON
  class ParsingError : public std::runtime_error
  {
  public:
    using runtime_error::runtime_error;
  };

  using Number = std::variant<int, double>;
  Number LoadNumber(std::istream &input);
  // Считывает содержимое строкового литерала JSON-документа
  // Функцию следует использовать после считывания открывающего символа ":
  std::string LoadString(std::istream &input);

  class Node
  {
  public:
    const Value &GetValue() const { return value_; }

    Node() = default;
    explicit Node(Array array);
    explicit Node(Dict map);
    explicit Node(int value);
    explicit Node(std::string value);
    explicit Node(double value);
    explicit Node(std::nullptr_t);
    explicit Node(bool value);

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string &AsString() const;
    const Array &AsArray() const;
    const Dict &AsMap() const;

    bool IsNull() const { return IsType<std::nullptr_t>(); }
    bool IsInt() const { return IsType<int>(); }
    bool IsPureDouble() const { return IsType<double>(); }
    bool IsDouble() const
    {
      return IsInt() || IsPureDouble();
    }
    bool IsBool() const { return IsType<bool>(); }
    bool IsString() const { return IsType<std::string>(); }
    bool IsArray() const { return IsType<Array>(); }
    bool IsMap() const { return IsType<Dict>(); }

  private:
    template <typename T>
    const T &ExtractValue() const
    {
      if (!IsType<T>())
      {
        throw std::logic_error("Wrong type");
      }
      return std::get<T>(value_);
    }

    template <typename T>
    bool IsType() const
    {
      return std::holds_alternative<T>(value_);
    }

    Value value_;
    Array as_array_;
    Dict as_map_;
    int as_int_ = 0;
    std::string as_string_;
  };

  class Document
  {
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

} // namespace json