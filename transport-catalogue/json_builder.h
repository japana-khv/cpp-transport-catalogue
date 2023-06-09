#pragma once
#include <string>
#include <exception>
#include <vector>
#include <optional>
#include <memory>
#include "json.h"

namespace json {

/*
 *  For correct control Builder state we use Pattern "State".
 *
 *  BiderState contains value - json::Node node_ . This is value for build in current Builder State
 */

class Builder;

class BuilderState {
protected:
    Builder &builder_;
    json::Node node_;
public:
    BuilderState(Builder &builder) :
            builder_(builder) {
    }
    BuilderState() = delete;

    virtual void Value(json::Node&&) = 0;
    virtual void StartDict() = 0;
    virtual void StartArray() = 0;
    virtual void EndDict() = 0;
    virtual void EndArray() = 0;
    virtual void Key(std::string&&) = 0;
    virtual void Build() = 0;
    // For Dict and Array State we need to push current node Value to Builder node_stac_ and process it in a new Builder state
    virtual void ProcessBackNode() {
    }
    ;

    json::Node& GetNode() {
        return node_;
    }
    ;

    virtual ~BuilderState() {
    }
};

class BuilderInitState: public BuilderState {
public:
    BuilderInitState(Builder &builder) :
            BuilderState(builder) {
    }
    BuilderInitState() = delete;

    void Value(json::Node&&) override;
    void StartDict() override;
    void StartArray() override;
    void EndDict() override;
    void EndArray() override;
    void Key(std::string&&) override;
    void Build() override;
};

class BuilderValueState: public BuilderState {
public:
    BuilderValueState(Builder &builder, json::Node &&node) :
            BuilderState(builder) {
        node_ = std::move(node);
    }
    BuilderValueState() = delete;

    void Value(json::Node&&) override;
    void StartDict() override;
    void StartArray() override;
    void EndDict() override;
    void EndArray() override;
    void Key(std::string&&) override;
    void Build() override;
};

class BuilderDictState: public BuilderState {
public:
    BuilderDictState(Builder &builder) :
            BuilderState(builder) {
        this->node_ = json::Dict();
    }
    BuilderDictState() = delete;

    void Value(json::Node&&) override;
    void StartDict() override;
    void StartArray() override;
    void EndDict() override;
    void EndArray() override;
    void Key(std::string&&) override;
    void Build() override;
    void ProcessBackNode() override;

private:
    std::optional<std::string> current_key;
};

class BuilderArrayState: public BuilderState {
public:
    BuilderArrayState(Builder &builder) :
            BuilderState(builder) {
        node_ = json::Array();
    }
    BuilderArrayState() = delete;

    void Value(json::Node&&) override;
    void StartDict() override;
    void StartArray() override;
    void EndDict() override;
    void EndArray() override;
    void Key(std::string&&) override;
    void Build() override;
    void ProcessBackNode() override;
};

/*
 * Builder
 *
 * */
class DictKeyResult;
class StartDictResult;
class StartArrayResult;
class ArrayValueResult;

class Builder {
    friend class BuilderInitState;
    friend class BuilderValueState;
    friend class BuilderDictState;
    friend class BuilderArrayState;

public:
    Builder();

    Builder& Value(json::Node node);
    StartDictResult StartDict();
    StartArrayResult StartArray();
    Builder& EndDict();
    Builder& EndArray();
    DictKeyResult Key(std::string key);
    Builder& KeyPrimary(std::string key);
    json::Node Build();

    BuilderState& GetState();
    void PushBackState(std::unique_ptr<BuilderState> &&state);
    void PopBackState();

    std::unique_ptr<json::Node>& GetBackNode();
    void PushNode(std::unique_ptr<json::Node> &&node);
    void PopNode();
    size_t GetNodeStackSize() {
        return nodes_stack_.size();
    }

private:

    std::optional<json::Node> root_;
    std::vector<std::unique_ptr<json::Node>> nodes_stack_;
    std::vector<std::unique_ptr<BuilderState> > state_stack;
};

class DictKeyValueResult {
    Builder &builder_;
public:
    DictKeyValueResult(Builder &builder) :
            builder_(builder) {
    }

    DictKeyResult Key(std::string key);

    Builder& EndDict();
};

class DictKeyResult {
    Builder &builder_;
public:
    DictKeyResult(Builder &builder) :
            builder_(builder) {
    }

    DictKeyValueResult Value(json::Node node);

    StartDictResult StartDict();
    StartArrayResult StartArray();
};

class StartDictResult {
    Builder &builder_;
public:
    StartDictResult(Builder &builder) :
            builder_(builder) {
    }
    DictKeyResult Key(std::string key);
    Builder& EndDict();
};

class StartArrayResult {
    Builder &builder_;
public:
    StartArrayResult(Builder &builder) :
            builder_(builder) {
    }
    ArrayValueResult Value(json::Node node);
    StartDictResult StartDict();
    StartArrayResult StartArray();
    Builder& EndArray();
};

class ArrayValueResult {
    Builder &builder_;
public:
    ArrayValueResult(Builder &builder) :
            builder_(builder) {
    }
    ArrayValueResult Value(json::Node node);
    StartDictResult StartDict();
    StartArrayResult StartArray();
    Builder& EndArray();
};

} /* namespace json */

