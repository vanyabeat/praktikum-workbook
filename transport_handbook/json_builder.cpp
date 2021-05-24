#include "json_builder.h"

json::DictValueCtx json::Builder::Key(std::string key) {
    using namespace std;
    switch (state_) {
        case State::EMPTY:
            throw std::logic_error("[Key] empty node key"s);
            break;
        case State::STAGING: {
            if (node_stack_ptrs_.top()->IsDict()) {
                node_stack_ptrs_.push(std::make_unique<Node>(key));
            } else {
                throw std::logic_error(IsKeyOnTop() ? "[Key] repeat sequence"s : "[Key] not in a dict node"s);
            }
        }
            break;
        case State::ENDED:
            throw std::logic_error("[Key] ended node key"s);
            break;
        default:
            throw std::logic_error("[Key] error"s);
    }
    return DictValueCtx(*this);
}

json::Builder &json::Builder::Value(json::Node::Value value) {
    using namespace std;
    switch (state_) {
        case State::EMPTY: {
            node_stack_ptrs_.push(std::make_unique<Node>(value));
            state_ = State::ENDED;
        }
            break;
        case State::STAGING: {
            if (node_stack_ptrs_.top()->IsArray()) {
                json::Array tmp = std::move(node_stack_ptrs_.top()->AsArray());
                tmp.emplace_back(value);
                *node_stack_ptrs_.top() = Node(std::move(tmp));
            } else if (IsKeyOnTop()) {
                std::string key = std::move(node_stack_ptrs_.top()->AsString());
                node_stack_ptrs_.pop();
                json::Dict dict = std::move(node_stack_ptrs_.top()->AsDict());
                dict.insert({key, value});
                *node_stack_ptrs_.top() = Node(std::move(dict));
            } else {
                throw std::logic_error("[Value] dict value without key"s);
            }
        }
            break;
        case State::ENDED:
            throw std::logic_error("[Value] ended node"s);
            break;
        default:
            throw std::logic_error("[Value] error"s);
    }
    return *this;
}

json::DictItemCtx json::Builder::StartDict() {
    using namespace std;
    switch (state_) {
        case State::EMPTY:
            state_ = State::STAGING;
            node_stack_ptrs_.push(std::make_unique<Node>(Dict()));
            break;
        case State::STAGING:
            if (!node_stack_ptrs_.top()->IsDict()) {
                node_stack_ptrs_.push(std::make_unique<Node>(Dict()));
            } else {
                throw std::logic_error("[Dict] start dict in another dict"s);
            }
            break;
        case State::ENDED:
            throw std::logic_error("[Dict] ready node start dict"s);
            break;
        default:
            throw std::logic_error("[Dict] error"s);
    }
    return DictItemCtx(*this);
}

json::Builder &json::Builder::EndDict() {
    using namespace std;
    switch (state_) {
        case State::EMPTY:
            throw std::logic_error("[Dict] empty node"s);
            break;
        case State::STAGING: {
            if (node_stack_ptrs_.top()->IsDict()) {
                if (node_stack_ptrs_.size() == 1) {
                    state_ = State::ENDED;
                } else {
                    json::Dict value = std::move(node_stack_ptrs_.top()->AsDict());
                    node_stack_ptrs_.pop();
                    this->Value(value);
                }
            } else {
                throw std::logic_error(node_stack_ptrs_.top()->IsString() ? "[Dict] dict value expected"s
                                                                          : "[Dict] it is not a dict"s);
            }
        }
            break;
        case State::ENDED:
            throw std::logic_error("[Dict] ready node"s);
            break;
        default:
            throw std::logic_error("[Dict] error"s);
    }
    return *this;
}

json::ArrayItemContext json::Builder::StartArray() {
    using namespace std;
    switch (state_) {
        case State::EMPTY:
            state_ = State::STAGING;
            node_stack_ptrs_.push(std::make_unique<Node>(Array()));
            break;
        case State::STAGING: {
            if (node_stack_ptrs_.top()->IsDict()) {
                throw std::logic_error("[Array] start array error"s);
            }
            node_stack_ptrs_.push(std::make_unique<Node>(Array()));
        }
            break;
        case State::ENDED:
            throw std::logic_error("[Array] start array error"s);
            break;
        default:
            throw std::logic_error("[Array] start array error"s);
    }
    return ArrayItemContext(*this);
}

json::Builder &json::Builder::EndArray() {
    using namespace std;
    switch (state_) {
        case State::EMPTY:
            throw std::logic_error("[Array] empty node end array"s);
            break;
        case State::STAGING: {
            if (node_stack_ptrs_.top()->IsArray()) {
                if (node_stack_ptrs_.size() == 1) {
                    state_ = State::ENDED;
                } else {
                    json::Array value = std::move(node_stack_ptrs_.top()->AsArray());
                    node_stack_ptrs_.pop();
                    this->Value(value);
                }
            } else {
                throw std::logic_error("[Array] non-array node end array"s);
            }
        }
            break;
        case State::ENDED:
            throw std::logic_error("[Array] ready node"s);
            break;
        default:
            throw std::logic_error("[Array] error"s);
    }
    return *this;
}

json::Node json::Builder::Build() {
    using namespace std;
    if (state_ == State::ENDED) {
        return *node_stack_ptrs_.top();
    } else {
        throw std::logic_error("[Build] building of ended node"s);
    }
}

json::DictItemCtx json::ArrayItemContext::StartDict() {
    return b_.StartDict();
}

json::DictValueCtx json::DictItemCtx::Key(const std::string &key) {
    return b_.Key(key);
}