#pragma once
#include "Document.hpp"
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
    void add(Document doc);
    void remove(DocumentId id);

    std::vector<DocumentId> search(const std::string& word) const;
    std::size_t count(DocumentId id, const std::string& word) const;
    std::optional<Document> get(DocumentId id) const;
    std::size_t size() const;
    void clear();

  private:
    std::unordered_map<DocumentId, Document> docs_;
    std::unordered_map<std::string, std::unordered_set<DocumentId>> index_;
    std::unordered_map<DocumentId, std::unordered_map<std::string, std::size_t>> counts_;
};

} // namespace indexer
