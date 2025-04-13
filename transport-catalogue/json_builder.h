#pragma once

#include "json.h"

#include <optional>

namespace json {

class DictItemContext;
class DictKeyContext;
class ArrayItemContext;

class Builder {
public:
    Builder();
    DictKeyContext Key(std::string key);
    Builder& Value(NodeType value);
    DictItemContext StartDict();
    Builder& EndDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    Node Build();
    Node GetNode(NodeType value);

private:
    Node root_{ nullptr };
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> key_{ std::nullopt };
};

class DictItemContext {
public:
    DictItemContext(Builder& builder);

    DictKeyContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class ArrayItemContext {
public:
    ArrayItemContext(Builder& builder);

    ArrayItemContext Value(NodeType value);
    DictItemContext StartDict();
    Builder& EndArray();
    ArrayItemContext StartArray();

private:
    Builder& builder_;
};

class DictKeyContext {
public:
    DictKeyContext(Builder& builder);
    
    DictItemContext Value(NodeType value);
    ArrayItemContext StartArray();
    DictItemContext StartDict();
    
private:
    Builder& builder_;
};

} // namespace json