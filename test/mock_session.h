#pragma once

#include <websocket/websocket.h>

#include <gmock/gmock.h>

#include <chrono>
#include <thread>

class out_of_messages_exception : public std::exception {};

class mock_session : public websocket::session_base {
public:
    mock_session(std::vector<std::string> messages_sequence)
        : messages_stack_(std::move(messages_sequence))
    {
        std::reverse(messages_stack_.begin(), messages_stack_.end());
    }

    MOCK_METHOD(void, start, (), (override));
    MOCK_METHOD(void, write, (const std::string& message), (override));
    MOCK_METHOD(void, close, (), (override));

    std::string read() override {
        using namespace std::chrono_literals;

        if (messages_stack_.empty()) {
            throw out_of_messages_exception();
        }

        std::string message = messages_stack_.back();
        messages_stack_.pop_back();
        return message;
    }

private:
    std::vector<std::string> messages_stack_;
};
