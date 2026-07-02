#pragma once
#include "phonedb/phonesearch.hpp"

inline Command create_phonedb_command() {
    // Return the phone search command as the primary phonedb command for now
    return create_phone_search_command();
}
