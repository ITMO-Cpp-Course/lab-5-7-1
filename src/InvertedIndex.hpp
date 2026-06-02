#pragma once
#include "document.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace indexer
{

class InvertedIndex
{
  public:
    // Добавить документ в индекс
    void add(Document doc);

    // Удалить документ по id
    void remove(DocumentId id);

    // Найти все документы, содержащие слово
    std::vector<DocumentId> search(const std::string& word) const;

    // Количество вхождений слова в документе
    std::size_t count(DocumentId id, const std::string& word) const;

    // Получить документ по id
    std::optional<Document> get(DocumentId id) const;

    // Количество документов в индексе
    std::size_t size() const;

    // Очистить индекс
    void clear();

  private:
    // id -> документ
    std::unordered_map<DocumentId, Document> docs_;
    // слово -> множество id документов
    std::unordered_map<std::string, std::unordered_set<DocumentId>> index_;
    // (id, слово) -> количество вхождений
    std::unordered_map<DocumentId, std::unordered_map<std::string, std::size_t>> counts_;
};

} // namespace indexer