#pragma once
#include "document.hpp"
#include <atomic>
#include <string>
#include <vector>

namespace indexer
{

class DocumentBuilder
{
  public:
    DocumentBuilder();

    // Создать документ из имени и текста (id генерируется автоматически)
    Document build(std::string name, std::string text) const;

    // Разбить текст на слова (lowercase, только буквы)
    static std::vector<std::string> tokenize(const std::string& text);

  private:
    mutable std::atomic<DocumentId> next_id_;
};

} // namespace indexer
