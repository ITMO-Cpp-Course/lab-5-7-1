#pragma once
#include "InvertedIndex.hpp"
#include "document.hpp"
#include "result.hpp"
#include <memory>
#include <string>
#include <vector>

namespace indexer
{

class UpdateTransaction; // forward declaration

// Высокоуровневое хранилище с явной обработкой ошибок
class IndexStore
{
  public:
    IndexStore() = default;

    // Добавить документ (ошибка если id уже существует)
    Result<> add(Document doc);

    // Удалить документ по id (ошибка если не найден)
    Result<> remove(DocumentId id);

    // Найти документы по слову
    Result<std::vector<DocumentId>> search(const std::string& word) const;

    // Получить документ по id
    Result<Document> get(DocumentId id) const;

    // Количество вхождений слова в документе
    Result<std::size_t> count(DocumentId id, const std::string& word) const;

    // Статистика: количество документов
    std::size_t size() const;

    // Начать транзакцию обновления
    UpdateTransaction begin_transaction();

  private:
    friend class UpdateTransaction;

    InvertedIndex index_;
};

// RAII-транзакция: автоматически откатывается если не был вызван commit()
class UpdateTransaction
{
  public:
    explicit UpdateTransaction(IndexStore& store);
    ~UpdateTransaction();

    // Запрещаем копирование
    UpdateTransaction(const UpdateTransaction&) = delete;
    UpdateTransaction& operator=(const UpdateTransaction&) = delete;

    // Разрешаем перемещение
    UpdateTransaction(UpdateTransaction&&) noexcept;
    UpdateTransaction& operator=(UpdateTransaction&&) noexcept;

    // Операции над «рабочей копией» индекса
    Result<> add(Document doc);
    Result<> remove(DocumentId id);

    // Применить изменения к IndexStore
    Result<> commit();

    bool is_committed() const;

  private:
    IndexStore* store_;      // указатель на оригинальное хранилище
    InvertedIndex snapshot_; // рабочая копия индекса на момент begin_transaction
    bool committed_;
};

} // namespace indexer