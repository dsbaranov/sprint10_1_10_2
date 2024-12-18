#include "json.h"

using namespace std;

namespace json {

// ValuePrinter

void ValuePrinter::operator()(std::nullptr_t) { os_ << "null"s; }
void ValuePrinter::operator()(Array) { os_ << ""s; }
void ValuePrinter::operator()(Dict) { os_ << ""s; }
void ValuePrinter::operator()(bool value) { os_ << value ? "true"s : "false"s; }
void ValuePrinter::operator()(int value) { os_ << value; }
void ValuePrinter::operator()(double value) { os_ << value; }
void ValuePrinter::operator()(std::string value) { os_ << value; }

// Node

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
  Array result;

  for (char c; input >> c && c != ']';) {
    if (c != ',') {
      input.putback(c);
    }
    result.push_back(LoadNode(input));
  }

  return Node(move(result));
}

Node LoadInt(istream& input) {
  int result = 0;
  while (isdigit(input.peek())) {
    result *= 10;
    result += input.get() - '0';
  }
  return Node(result);
}

Node LoadString(istream& input) {
  string line;
  getline(input, line, '"');
  return Node(move(line));
}

Node LoadDict(istream& input) {
  Dict result;

  for (char c; input >> c && c != '}';) {
    if (c == ',') {
      input >> c;
    }

    string key = LoadString(input).AsString();
    input >> c;
    result.insert({move(key), LoadNode(input)});
  }

  return Node(move(result));
}

Node LoadNode(istream& input) {
  char c;
  input >> c;

  if (c == '[') {
    return LoadArray(input);
  } else if (c == '{') {
    return LoadDict(input);
  } else if (c == '"') {
    return LoadString(input);
  } else {
    input.putback(c);
    return LoadInt(input);
  }
}

Node::Node(Array array) : value_(move(array)) {}

Node::Node(Dict map) : value_(move(map)) {}

Node::Node(int value) : value_(value) {}

Node::Node(string value) : value_(move(value)) {}

Node::Node(double value) : value_(move(value)) {}

const Array& Node::AsArray() const { return as_array_; }

const Dict& Node::AsMap() const { return as_map_; }

int Node::AsInt() const { return as_int_; }

const string& Node::AsString() const { return as_string_; }

Document::Document(Node root) : root_(move(root)) {}

const Node& Document::GetRoot() const { return root_; }

bool operator==(const Node& lft, const Node& rgt) {
  return lft.IsArray() == rgt.IsArray() && lft.IsBool() == rgt.IsBool() &&
         lft.IsDouble() == rgt.IsDouble() && lft.IsInt() == rgt.IsInt() &&
         lft.IsMap() == rgt.IsMap() && lft.IsNull() == rgt.IsNull() &&
         lft.IsPureDouble() == rgt.IsPureDouble() &&
         lft.IsString() == rgt.IsString() && lft.GetValue() == rgt.GetValue();
}
bool operator!=(const Node& lft, const Node& rgt) { return !(lft == rgt); }

Document Load(istream& input) { return Document{LoadNode(input)}; }

void Print(const Document& doc, std::ostream& output) {
  (void)&doc;
  (void)&output;

  // Реализуйте функцию самостоятельно
}

}  // namespace json