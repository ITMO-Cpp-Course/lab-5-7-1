#include "IndexStore.hpp"
#include <utility>

namespace indexer
{

// ─── IndexStore ───────────────────────────────────────────────

Result<> IndexStore::add(Document doc)
{
    if (index_.get(doc.id).has_value())
        return std::unexpected(IndexError::DocumentAlreadyExists);
    index_.add(std::move(doc));
    return {};
}

Result<> IndexStore::remove(DocumentId id)
{
    if (!index_.get(id).has_value())
        return std::unexpected(IndexError::DocumentNotFound);
    index_.remove(id);
    return {};
}

Result<std::vector<DocumentId>> IndexStore::search(const std::string& word) const
{
    if (word.empty())
        return std::unexpected(IndexError::InvalidArgument);
    return index_.search(word);
}

Result<Document> IndexStore::get(DocumentId id) const
{
    auto doc = index_.get(id);
    if (!doc.has_value())
        return std::unexpected(IndexError::DocumentNotFound);
    return *doc;
}

Result<std::size_t> IndexStore::count(DocumentId id, const std::string& word) const
{
    if (!index_.get(id).has_value())
        return std::unexpected(IndexError::DocumentNotFound);
    return index_.count(id, word);
}

std::size_t IndexStore::size() const
{
    return index_.size();
}

UpdateTransaction IndexStore::begin_transaction()
{
    return UpdateTransaction(*this);
}

// ─── UpdateTransaction ────────────────────────────────────────

UpdateTransaction::UpdateTransaction(IndexStore& store) : store_(&store), snapshot_(store.index_), committed_(false) {}

UpdateTransaction::~UpdateTransaction() = default;

UpdateTransaction::UpdateTransaction(UpdateTransaction&& other) noexcept
    : store_(other.store_), snapshot_(std::move(other.snapshot_)), committed_(other.committed_)
{
    other.store_ = nullptr;
    other.committed_ = true;
}

UpdateTransaction& UpdateTransaction::operator=(UpdateTransaction&& other) noexcept
{
    if (this != &other)
    {
        store_ = other.store_;
        snapshot_ = std::move(other.snapshot_);
        committed_ = other.committed_;
        other.store_ = nullptr;
        other.committed_ = true;
    }
    return *this;
}

Result<> UpdateTransaction::add(Document doc)
{
    if (committed_)
        return std::unexpected(IndexError::TransactionAlreadyCommitted);
    if (snapshot_.get(doc.id).has_value())
        return std::unexpected(IndexError::DocumentAlreadyExists);
    snapshot_.add(std::move(doc));
    return {};
}

Result<> UpdateTransaction::remove(DocumentId id)
{
    if (committed_)
        return std::unexpected(IndexError::TransactionAlreadyCommitted);
    if (!snapshot_.get(id).has_value())
        return std::unexpected(IndexError::DocumentNotFound);
    snapshot_.remove(id);
    return {};
}

Result<> UpdateTransaction::commit()
{
    if (committed_)
        return std::unexpected(IndexError::TransactionAlreadyCommitted);
    store_->index_ = std::move(snapshot_);
    committed_ = true;
    return {};
}

bool UpdateTransaction::is_committed() const
{
    return committed_;
}

} // namespace indexer
