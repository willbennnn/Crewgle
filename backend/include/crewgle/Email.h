#pragma once

#include <string>

namespace crewgle {

class Email {
public:
    static void sendPasswordResetEmail(const std::string& to, const std::string& resetLink);
};

}