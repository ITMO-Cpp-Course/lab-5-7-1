#pragma once
#include <expected>
#include <string>

namespace indexer
{

enum class IndexError
{
    DocumentNotFound,
    DocumentAlreadyExists,
    TransactionAlreadyCommitted,
    InvalidArgument,
};

inline std::string to_string(IndexError err)
{
    switch (err)
    {
    case IndexError::DocumentNotFound:
        return "document not found";
    case IndexError::DocumentAlreadyExists:
        return "document already exists";
    case IndexError::TransactionAlreadyCommitted:
        return "transaction already committed";
    case IndexError::InvalidArgument:
        return "invalid argument";
    }
    return "unknown error";
}

template <typename T = void> using Result = std::expected<T, IndexError>;

} // namespace indexer
