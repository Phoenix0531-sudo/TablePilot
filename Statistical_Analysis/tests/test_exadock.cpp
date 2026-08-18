#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "webresearchdock.h"

// QtTest coverage for the Exa API response parser extracted from
// WebResearchDock::onResult into the pure static parseResults().
//
// The dock's network layer (QNetworkAccessManager, live Exa API) is intentionally
// not exercised in CI — that would need a real API key and live network. Instead
// these tests feed canned JSON payloads to the parser, pinning the field
// extraction (title/url/text/publishedDate/score) and the safe defaults for
// missing fields and malformed payloads.

class TestExaParser : public QObject {
    Q_OBJECT

private slots:
    void emptyResultsArray();
    void missingResultsKey();
    void singleResultAllFields();
    void multipleResultsPreserveOrder();
    void missingFieldsDefaultSafely();
    void scoreDefaultsToZeroWhenAbsent();
    void scoreParsedAsDouble();
    void nonObjectArrayElementsSkipped();
    void emptyDocument();
    void malformedJsonReturnsEmpty();
};

static QJsonDocument docFrom(const QByteArray &json)
{
    return QJsonDocument::fromJson(json);
}

void TestExaParser::emptyResultsArray()
{
    auto out = WebResearchDock::parseResults(docFrom(R"({"results": []})"));
    QCOMPARE(out.size(), 0);
}

void TestExaParser::missingResultsKey()
{
    // No "results" key -> treated as empty array.
    auto out = WebResearchDock::parseResults(docFrom(R"({"request_id": "abc"})"));
    QCOMPARE(out.size(), 0);
}

void TestExaParser::singleResultAllFields()
{
    auto out = WebResearchDock::parseResults(docFrom(
        R"({"results":[{"title":"T1","url":"https://example.com/a",)"
        R"("text":"body text","publishedDate":"2026-01-02","score":0.91}]}))"));
    QCOMPARE(out.size(), 1);
    QCOMPARE(out[0].title, QStringLiteral("T1"));
    QCOMPARE(out[0].url, QStringLiteral("https://example.com/a"));
    QCOMPARE(out[0].text, QStringLiteral("body text"));
    QCOMPARE(out[0].publishedDate, QStringLiteral("2026-01-02"));
    QCOMPARE(out[0].score, 0.91);
}

void TestExaParser::multipleResultsPreserveOrder()
{
    auto out = WebResearchDock::parseResults(docFrom(
        R"({"results":[)"
        R"({"title":"first","url":"u1"},)"
        R"({"title":"second","url":"u2"},)"
        R"({"title":"third","url":"u3"}]})"));
    QCOMPARE(out.size(), 3);
    QCOMPARE(out[0].title, QStringLiteral("first"));
    QCOMPARE(out[1].title, QStringLiteral("second"));
    QCOMPARE(out[2].title, QStringLiteral("third"));
}

void TestExaParser::missingFieldsDefaultSafely()
{
    auto out = WebResearchDock::parseResults(docFrom(
        R"({"results":[{"title":"only-title"}]})"));
    QCOMPARE(out.size(), 1);
    QCOMPARE(out[0].title, QStringLiteral("only-title"));
    QCOMPARE(out[0].url, QString());        // absent -> empty
    QCOMPARE(out[0].text, QString());
    QCOMPARE(out[0].publishedDate, QString());
}

void TestExaParser::scoreDefaultsToZeroWhenAbsent()
{
    auto out = WebResearchDock::parseResults(docFrom(
        R"({"results":[{"title":"t","url":"u"}]})"));
    QCOMPARE(out.size(), 1);
    QCOMPARE(out[0].score, 0.0);  // absent -> default 0.0
}

void TestExaParser::scoreParsedAsDouble()
{
    auto out = WebResearchDock::parseResults(docFrom(
        R"({"results":[{"title":"t","score":0.5555}]}))"));
    QCOMPARE(out.size(), 1);
    QCOMPARE(out[0].score, 0.5555);
}

void TestExaParser::nonObjectArrayElementsSkipped()
{
    // Mixed array: a string and a number are not objects, but toObject()
    // yields an empty object -> an empty ResultItem is still appended.
    // The key invariant: the parser must not crash on non-object elements.
    auto out = WebResearchDock::parseResults(docFrom(
        R"({"results":["not-an-object", 42, {"title":"real"}]})"));
    // QJsonValue::toObject() on a non-object returns {} -> we still append
    // empty ResultItems for them; the real object lands last.
    QCOMPARE(out.size(), 3);
    QCOMPARE(out[2].title, QStringLiteral("real"));
    QCOMPARE(out[0].title, QString());  // string element -> empty object
}

void TestExaParser::emptyDocument()
{
    auto out = WebResearchDock::parseResults(QJsonDocument());
    QCOMPARE(out.size(), 0);
}

void TestExaParser::malformedJsonReturnsEmpty()
{
    // Invalid JSON -> fromJson returns a null/empty document -> empty results.
    auto out = WebResearchDock::parseResults(docFrom("{ not json"));
    QCOMPARE(out.size(), 0);
}

QTEST_MAIN(TestExaParser)
#include "test_exadock.moc"
