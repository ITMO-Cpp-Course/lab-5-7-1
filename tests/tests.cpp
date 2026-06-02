#include "DocumentBuilder.hpp"
#include "IndexStore.hpp"
#include "InvertedIndex.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace indexer;

// ═══════════════════════════════════════════════════════════════
//  ЛР5 — In-memory индекс
// ═══════════════════════════════════════════════════════════════

// ─── DocumentBuilder ──────────────────────────────────────────

TEST_CASE("DocumentBuilder::tokenize splits words and lowercases", "[builder]")
{
    SECTION("basic words")
    {
        auto tokens = DocumentBuilder::tokenize("Hello World");
        REQUIRE(tokens.size() == 2);
        CHECK(tokens[0] == "hello");
        CHECK(tokens[1] == "world");
    }

    SECTION("strips punctuation")
    {
        auto tokens = DocumentBuilder::tokenize("foo, bar! baz.");
        REQUIRE(tokens.size() == 3);
        CHECK(tokens[0] == "foo");
        CHECK(tokens[1] == "bar");
        CHECK(tokens[2] == "baz");
    }

    SECTION("empty string returns no tokens")
    {
        CHECK(DocumentBuilder::tokenize("").empty());
    }

    SECTION("digits-only string returns no tokens")
    {
        CHECK(DocumentBuilder::tokenize("123 456").empty());
    }
}

TEST_CASE("DocumentBuilder::build assigns unique ids", "[builder]")
{
    DocumentBuilder builder;
    auto d1 = builder.build("a", "text");
    auto d2 = builder.build("b", "text");
    CHECK(d1.id != d2.id);
    CHECK(d1.name == "a");
    CHECK(d2.name == "b");
}

// ─── InvertedIndex ────────────────────────────────────────────

struct IndexFixture
{
    InvertedIndex index;
    DocumentBuilder builder;

    IndexFixture()
    {
        index.add(builder.build("doc1", "the cat sat on the mat"));
        index.add(builder.build("doc2", "the dog sat on the log"));
        index.add(builder.build("doc3", "a cat and a dog"));
    }
};

TEST_CASE_METHOD(IndexFixture, "size reflects added documents", "[index]")
{
    CHECK(index.size() == 3);
}

TEST_CASE_METHOD(IndexFixture, "search finds documents containing word", "[index]")
{
    SECTION("word in two documents")
    {
        CHECK(index.search("cat").size() == 2);
    }

    SECTION("missing word returns empty")
    {
        CHECK(index.search("elephant").empty());
    }

    SECTION("common word across two docs")
    {
        CHECK(index.search("the").size() == 2);
    }
}

TEST_CASE_METHOD(IndexFixture, "count returns correct occurrence count", "[index]")
{
    auto results = index.search("the");
    for (auto id : results)
    {
        auto doc = index.get(id);
        REQUIRE(doc.has_value());
        if (doc->name == "doc1" || doc->name == "doc2")
        {
            CHECK(index.count(id, "the") == 2);
        }
    }
}

TEST_CASE_METHOD(IndexFixture, "count returns 0 for missing word", "[index]")
{
    auto results = index.search("cat");
    REQUIRE_FALSE(results.empty());
    CHECK(index.count(results[0], "elephant") == 0);
}

TEST_CASE_METHOD(IndexFixture, "remove deletes document and updates index", "[index]")
{
    DocumentId doc1_id = 0;
    for (auto id : index.search("cat"))
    {
        auto doc = index.get(id);
        if (doc && doc->name == "doc1")
            doc1_id = id;
    }
    REQUIRE(doc1_id != 0);

    index.remove(doc1_id);

    CHECK(index.size() == 2);
    CHECK_FALSE(index.get(doc1_id).has_value());
    CHECK(index.search("cat").size() == 1);
}

TEST_CASE_METHOD(IndexFixture, "remove non-existent id is a no-op", "[index]")
{
    CHECK_NOTHROW(index.remove(9999));
    CHECK(index.size() == 3);
}

TEST_CASE_METHOD(IndexFixture, "clear empties the index", "[index]")
{
    index.clear();
    CHECK(index.size() == 0);
    CHECK(index.search("cat").empty());
}

TEST_CASE_METHOD(IndexFixture, "get returns document by id", "[index]")
{
    auto results = index.search("cat");
    REQUIRE_FALSE(results.empty());
    auto doc = index.get(results[0]);
    REQUIRE(doc.has_value());
    CHECK_FALSE(doc->text.empty());
}

// ═══════════════════════════════════════════════════════════════
//  ЛР6 — Обработка ошибок и транзакции
// ═══════════════════════════════════════════════════════════════

struct StoreFixture
{
    IndexStore store;
    DocumentBuilder builder;
};

// ─── IndexStore — базовые операции ───────────────────────────

TEST_CASE_METHOD(StoreFixture, "add and get document", "[store]")
{
    auto doc = builder.build("doc1", "hello world");
    auto id = doc.id;

    REQUIRE(store.add(std::move(doc)).has_value());

    auto got = store.get(id);
    REQUIRE(got.has_value());
    CHECK(got->name == "doc1");
}

TEST_CASE_METHOD(StoreFixture, "add duplicate returns DocumentAlreadyExists", "[store]")
{
    auto doc = builder.build("doc1", "hello");
    auto id = doc.id;
    store.add(doc);

    auto res = store.add(Document{id, "doc1_copy", "world"});
    REQUIRE_FALSE(res.has_value());
    CHECK(res.error() == IndexError::DocumentAlreadyExists);
}

TEST_CASE_METHOD(StoreFixture, "remove existing document", "[store]")
{
    auto doc = builder.build("doc1", "hello world");
    auto id = doc.id;
    store.add(std::move(doc));

    REQUIRE(store.remove(id).has_value());
    CHECK_FALSE(store.get(id).has_value());
}

TEST_CASE_METHOD(StoreFixture, "remove non-existent returns DocumentNotFound", "[store]")
{
    auto res = store.remove(9999);
    REQUIRE_FALSE(res.has_value());
    CHECK(res.error() == IndexError::DocumentNotFound);
}

TEST_CASE_METHOD(StoreFixture, "search empty word returns InvalidArgument", "[store]")
{
    auto res = store.search("");
    REQUIRE_FALSE(res.has_value());
    CHECK(res.error() == IndexError::InvalidArgument);
}

TEST_CASE_METHOD(StoreFixture, "search returns matching documents", "[store]")
{
    store.add(builder.build("a", "quick brown fox"));
    store.add(builder.build("b", "the fox jumped"));

    auto res = store.search("fox");
    REQUIRE(res.has_value());
    CHECK(res->size() == 2);
}

TEST_CASE_METHOD(StoreFixture, "count returns correct occurrences", "[store]")
{
    auto doc = builder.build("doc1", "fox fox fox");
    auto id = doc.id;
    store.add(std::move(doc));

    auto res = store.count(id, "fox");
    REQUIRE(res.has_value());
    CHECK(*res == 3);
}

TEST_CASE_METHOD(StoreFixture, "count on unknown doc returns DocumentNotFound", "[store]")
{
    auto res = store.count(9999, "word");
    REQUIRE_FALSE(res.has_value());
    CHECK(res.error() == IndexError::DocumentNotFound);
}

// ─── UpdateTransaction ────────────────────────────────────────

TEST_CASE_METHOD(StoreFixture, "committed transaction applies changes to store", "[transaction]")
{
    auto doc = builder.build("doc1", "hello world");
    auto id = doc.id;

    auto tx = store.begin_transaction();
    REQUIRE(tx.add(std::move(doc)).has_value());
    REQUIRE(tx.commit().has_value());

    CHECK(store.get(id).has_value());
    CHECK(store.size() == 1);
}

TEST_CASE_METHOD(StoreFixture, "transaction without commit rolls back on destroy", "[transaction]")
{
    auto doc = builder.build("doc1", "hello world");
    auto id = doc.id;

    {
        auto tx = store.begin_transaction();
        tx.add(std::move(doc));
        // tx уничтожается без commit — автоматический откат
    }

    CHECK(store.size() == 0);
    CHECK_FALSE(store.get(id).has_value());
}

TEST_CASE_METHOD(StoreFixture, "transaction add duplicate returns DocumentAlreadyExists", "[transaction]")
{
    auto doc = builder.build("doc1", "hello");
    auto id = doc.id;
    store.add(doc);

    auto tx = store.begin_transaction();
    auto res = tx.add(Document{id, "dup", "world"});
    REQUIRE_FALSE(res.has_value());
    CHECK(res.error() == IndexError::DocumentAlreadyExists);
}

TEST_CASE_METHOD(StoreFixture, "transaction remove non-existent returns DocumentNotFound", "[transaction]")
{
    auto tx = store.begin_transaction();
    auto res = tx.remove(9999);
    REQUIRE_FALSE(res.has_value());
    CHECK(res.error() == IndexError::DocumentNotFound);
}

TEST_CASE_METHOD(StoreFixture, "double commit returns TransactionAlreadyCommitted", "[transaction]")
{
    auto tx = store.begin_transaction();
    REQUIRE(tx.commit().has_value());

    auto res = tx.commit();
    REQUIRE_FALSE(res.has_value());
    CHECK(res.error() == IndexError::TransactionAlreadyCommitted);
}

TEST_CASE_METHOD(StoreFixture, "partial failure in transaction does not change store", "[transaction]")
{
    auto existing = builder.build("existing", "some text");
    auto existing_id = existing.id;
    store.add(existing);

    auto new_doc = builder.build("new", "new text");
    auto new_id = new_doc.id;

    {
        auto tx = store.begin_transaction();
        REQUIRE(tx.add(std::move(new_doc)).has_value());
        // Дубликат — ошибка, commit не вызываем
        auto dup = tx.add(Document{existing_id, "dup", "dup text"});
        CHECK_FALSE(dup.has_value());
        // tx уничтожается без commit
    }

    CHECK(store.size() == 1);
    CHECK_FALSE(store.get(new_id).has_value());
}

TEST_CASE_METHOD(StoreFixture, "transaction remove then commit removes document", "[transaction]")
{
    auto doc = builder.build("doc1", "hello world");
    auto id = doc.id;
    store.add(doc);

    auto tx = store.begin_transaction();
    REQUIRE(tx.remove(id).has_value());
    REQUIRE(tx.commit().has_value());

    CHECK(store.size() == 0);
    CHECK_FALSE(store.get(id).has_value());
}

TEST_CASE_METHOD(StoreFixture, "multiple adds in one transaction all apply on commit", "[transaction]")
{
    auto d1 = builder.build("d1", "alpha beta");
    auto d2 = builder.build("d2", "beta gamma");

    auto tx = store.begin_transaction();
    tx.add(std::move(d1));
    tx.add(std::move(d2));
    REQUIRE(tx.commit().has_value());

    CHECK(store.size() == 2);
    auto res = store.search("beta");
    REQUIRE(res.has_value());
    CHECK(res->size() == 2);
}