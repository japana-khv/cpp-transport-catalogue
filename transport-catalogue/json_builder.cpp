/*
 * json_builder.cpp
 *
 *  Created on: May 27, 2023
 *      Author: rvk
 */

#include "json_builder.h"

namespace json {

Builder::Builder() {
    PushBackState(std::make_unique<BuilderInitState>(*this));
}

Builder& Builder::Value(json::Node node) {
    GetState().Value(std::move(node));
    return *this;
}

StartDictResult Builder::StartDict() {
    GetState().StartDict();
    return {*this};
}

StartArrayResult Builder::StartArray() {
    GetState().StartArray();
    return {*this};
}

Builder& Builder::EndDict() {
    GetState().EndDict();
    PopBackState();
    GetState().ProcessBackNode();
    return *this;
}

Builder& Builder::EndArray() {
    GetState().EndArray();
    PopBackState();
    GetState().ProcessBackNode();
    return *this;
}

Builder& Builder::KeyPrimary(std::string key) {
    GetState().Key(std::move(key));
    return *this;
}

DictKeyResult Builder::Key(std::string key) {
    return {KeyPrimary(key)};
}

BuilderState& Builder::GetState() {
    return *state_stack.back();
}

void Builder::PushBackState(std::unique_ptr<BuilderState> &&state) {
    state_stack.emplace_back(std::move(state));
}

void Builder::PopBackState() {
    state_stack.pop_back();
}

std::unique_ptr<json::Node>& Builder::GetBackNode() {
    return nodes_stack_.back();
}

void Builder::PushNode(std::unique_ptr<json::Node> &&node) {
    nodes_stack_.emplace_back(std::move(node));
}

void Builder::PopNode() {
    nodes_stack_.pop_back();
}

json::Node Builder::Build() {
    GetState().Build();
    return root_.value();
}

void BuilderInitState::Value(json::Node &&node) {
    builder_.PushBackState(std::make_unique<BuilderValueState>(builder_, json::Node(std::move(node))));
}

void BuilderInitState::StartDict() {
    builder_.PushBackState(std::make_unique<BuilderDictState>(builder_));
}

void BuilderInitState::StartArray() {
    builder_.PushBackState(std::make_unique<BuilderArrayState>(builder_));
}

void BuilderInitState::EndDict() {
    throw std::logic_error("EndDict called in Init state");
}

void BuilderInitState::EndArray() {
    throw std::logic_error("EndArray called in Init state");
}

void BuilderInitState::Key(std::string&&) {
    throw std::logic_error("Key called in Init state");
}

void BuilderInitState::Build() {
    if (builder_.GetNodeStackSize() != 1) {
        throw std::logic_error("Build called in Init state");
    } else {
        builder_.root_ = (*builder_.GetBackNode());
        builder_.PopNode();
    }
}

void BuilderValueState::Value(json::Node&&) {
    throw std::logic_error("Value called in Value state");
}

void BuilderValueState::StartDict() {
    throw std::logic_error("StartDict called in Value state");
}

void BuilderValueState::StartArray() {
    throw std::logic_error("StartArray called in Value state");
}

void BuilderValueState::EndDict() {
    throw std::logic_error("EndDict called in Value state");
}

void BuilderValueState::EndArray() {
    throw std::logic_error("EndArray called in Value state");
}

void BuilderValueState::Key(std::string&&) {
    throw std::logic_error("Key called in Value state");
}

void BuilderValueState::Build() {
    builder_.root_ = std::move(node_);
}

void BuilderDictState::Value(json::Node &&node) {
    if (current_key) {
        node_.AsDict()[current_key.value()] = json::Node(std::move(node));
        current_key.reset();
    } else {
        throw std::logic_error("Value called without Key in Dict state");
    }
}

void BuilderDictState::StartDict() {
    if (current_key) {
        builder_.PushBackState(std::make_unique<BuilderDictState>(builder_));
    } else {
        throw std::logic_error("Key value not assigned. Unexpected call StartDict");
    }
}

void BuilderDictState::StartArray() {
    if (current_key) {
        builder_.PushBackState(std::make_unique<BuilderArrayState>(builder_));
    } else {
        throw std::logic_error("Key value not assigned. Unexpected call StartArray");
    }
}

void BuilderDictState::EndDict() {
    builder_.PushNode(std::make_unique<json::Node>(std::move(node_)));
}

void BuilderDictState::EndArray() {
    throw std::logic_error("Unexpected call EndArray for Dict state");
}

void BuilderDictState::Key(std::string &&key) {
    if (!current_key) {
        current_key = std::move(key);
    } else {
        throw std::logic_error("secondary Key call in Dict state");
    }
}

void BuilderDictState::Build() {
    throw std::logic_error("Unexpected Build call");

}

void BuilderDictState::ProcessBackNode() {
    if (current_key) {
        node_.AsDict()[current_key.value()] = *(builder_.GetBackNode());
        builder_.PopNode();
        current_key.reset();
    } else {
        throw std::logic_error("Key not ready for value placement in Dict state");
    }
}

void BuilderArrayState::Value(json::Node &&node) {
    node_.AsArray().emplace_back(std::move(node));
}

void BuilderArrayState::StartDict() {
    builder_.PushBackState(std::make_unique<BuilderDictState>(builder_));
}

void BuilderArrayState::StartArray() {
    builder_.PushBackState(std::make_unique<BuilderArrayState>(builder_));
}

void BuilderArrayState::EndDict() {
    throw std::logic_error("Unexpected call EndDict for Array state");
}

void BuilderArrayState::EndArray() {
    builder_.PushNode(std::make_unique<json::Node>(std::move(node_)));
}

void BuilderArrayState::Key(std::string&&) {
    throw std::logic_error("Unexpected call Key for Array state");
}

void BuilderArrayState::Build() {
    throw std::logic_error("Unexpected Build call");
}

void BuilderArrayState::ProcessBackNode() {
    node_.AsArray().emplace_back(*builder_.GetBackNode());
    builder_.PopNode();
}

DictKeyResult DictKeyValueResult::Key(std::string key) {
    return {builder_.Key(key)};
}

Builder& DictKeyValueResult::EndDict() {
    return builder_.EndDict();
}

DictKeyValueResult DictKeyResult::Value(json::Node node) {
    return {builder_.Value(node)};
}

StartDictResult DictKeyResult::StartDict() {
    return {builder_.StartDict()};
}

StartArrayResult DictKeyResult::StartArray() {
    return builder_.StartArray();
}

DictKeyResult StartDictResult::Key(std::string key) {
    return {builder_.Key(key)};
}

Builder& StartDictResult::EndDict() {
    return builder_.EndDict();
}

ArrayValueResult StartArrayResult::Value(json::Node node) {
    return {builder_.Value(node)};
}

StartDictResult StartArrayResult::StartDict() {
    return {builder_.StartDict()};
}

StartArrayResult StartArrayResult::StartArray() {
    return builder_.StartArray();
}

Builder& StartArrayResult::EndArray() {
    return builder_.EndArray();
}

ArrayValueResult ArrayValueResult::Value(json::Node node) {
    return {builder_.Value(node)};
}

StartDictResult ArrayValueResult::StartDict() {
    return {builder_.StartDict()};
}

StartArrayResult ArrayValueResult::StartArray() {
    return builder_.StartArray();
}

Builder& ArrayValueResult::EndArray() {
    return builder_.EndArray();
}

} /* namespace json */
