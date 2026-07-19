#pragma once

#include "crewgle/Database.h"

namespace crewgle {

void migrate(Database& db);
void seed(Database& db);

}
