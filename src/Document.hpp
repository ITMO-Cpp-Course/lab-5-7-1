#pragma once
#include <cstdint>
#include <string>

namespace indexer
{

using DocumentId = std::uint64_t;

struct Document
{
    DocumentId id;
    std::string name;
    std::string text;
};

} // namespace indexer