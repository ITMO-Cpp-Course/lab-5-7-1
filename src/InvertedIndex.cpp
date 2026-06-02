#include "invertedIndex.hpp"
#include "documentBuilder.hpp"

namespace indexer
{

void InvertedIndex::add(Document doc)
{
    DocumentId id = doc.id;
    auto tokens = DocumentBuilder::tokenize(doc.text);

    // Подсчёт вхождений
    auto& word_counts = counts_[id];
    for (const auto& token : tokens)
    {
        word_counts[token]++;
        index_[token].insert(id);
    }

    docs_.emplace(id, std::move(doc));
}

void InvertedIndex::remove(DocumentId id)
{
    auto doc_it = docs_.find(id);
    if (doc_it == docs_.end())
        return;

    // Убираем id из всех постингов
    auto counts_it = counts_.find(id);
    if (counts_it != counts_.end())
    {
        for (const auto& [word, _] : counts_it->second)
        {
            auto idx_it = index_.find(word);
            if (idx_it != index_.end())
            {
                idx_it->second.erase(id);
                if (idx_it->second.empty())
                {
                    index_.erase(idx_it);
                }
            }
        }
        counts_.erase(counts_it);
    }

    docs_.erase(doc_it);
}

std::vector<DocumentId> InvertedIndex::search(const std::string& word) const
{
    auto it = index_.find(word);
    if (it == index_.end())
        return {};
    return {it->second.begin(), it->second.end()};
}

std::size_t InvertedIndex::count(DocumentId id, const std::string& word) const
{
    auto doc_it = counts_.find(id);
    if (doc_it == counts_.end())
        return 0;
    auto word_it = doc_it->second.find(word);
    if (word_it == doc_it->second.end())
        return 0;
    return word_it->second;
}

std::optional<Document> InvertedIndex::get(DocumentId id) const
{
    auto it = docs_.find(id);
    if (it == docs_.end())
        return std::nullopt;
    return it->second;
}

std::size_t InvertedIndex::size() const
{
    return docs_.size();
}

void InvertedIndex::clear()
{
    docs_.clear();
    index_.clear();
    counts_.clear();
}

} // namespace indexer