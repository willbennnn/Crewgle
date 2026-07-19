#include "crewgle/Email.h"
#include <iostream>

namespace crewgle {
    void Email::sendPasswordResetEmail(const std::string& to, const std::string& resetLink) {
        // Devpath for now, implement email API later
        std::cout << "Password reset link for " << to << ": " << resetLink << "\n";
    }
}