#pragma once
#include "Document.hpp"
#include "InvertedIndex.hpp"
#include "Result.hpp"
#include <string>
#include <vector>

namespace indexer
{

class UpdateTransaction;

class IndexStore
{
  public:
    IndexStore() = default;

    Result<> add(Document doc);
    Result<> remove(DocumentId id);
    Result<std::vector<DocumentId>> search(const std::string& word) const;
    Result<Document> get(DocumentId id) const;
    Result<std::size_t> count(DocumentId id, const std::string& word) const;
    std::size_t size() const;

    UpdateTransaction begin_transaction();

  private:
    friend class UpdateTransaction;
    InvertedIndex index_;
};

class UpdateTransaction
{
  public:
    explicit UpdateTransaction(IndexStore& store);
    ~UpdateTransaction();

    UpdateTransaction(const UpdateTransaction&) = delete;
    UpdateTransaction& operator=(const UpdateTransaction&) = delete;
    UpdateTransaction(UpdateTransaction&&) noexcept;
    UpdateTransaction& operator=(UpdateTransaction&&) noexcept;

    Result<> add(Document doc);
    Result<> remove(DocumentId id);
    Result<> commit();
    bool is_committed() const;

  private:
    IndexStore* store_;
    InvertedIndex snapshot_;
    bool committed_;
};

} // namespace indexer
