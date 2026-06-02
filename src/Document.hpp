#pragma once
#include <string>
#include <cstdint>

namespace indexer {

using DocumentId = std::uint64_t;

struct Document {
    DocumentId id;
    std::string name;
    std::string text;
};

} // namespace indexer
