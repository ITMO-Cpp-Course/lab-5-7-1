#include "DocumentBuilder.hpp"
#include <cctype>

namespace indexer
{

DocumentBuilder::DocumentBuilder() : next_id_(1) {}

Document DocumentBuilder::build(std::string name, std::string text) const
{
    return Document{next_id_++, std::move(name), std::move(text)};
}

std::vector<std::string> DocumentBuilder::tokenize(const std::string& text)
{
    std::vector<std::string> tokens;
    std::string token;

    for (char raw : text)
    {
        const auto ch = static_cast<unsigned char>(raw);
        if (std::isalpha(ch))
        {
            token += static_cast<char>(std::tolower(ch));
        }
        else if (!token.empty())
        {
            tokens.push_back(std::move(token));
            token.clear();
        }
    }
    if (!token.empty())
    {
        tokens.push_back(std::move(token));
    }
    return tokens;
}

} // namespace indexer
