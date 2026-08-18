#include <QtTest/QtTest>

#include "textformat.h"

// QtTest coverage for the pure localization helpers extracted from MainWindow.
//
// These tests pin the EN<->ZH mapping tables so a typo or a dropped branch in
// textformat.cpp is caught immediately rather than silently rendering the wrong
// label in the desktop UI. Each function is tested in both EN (useChinese=false)
// and ZH (useChinese=true) modes, covering known enum values plus the
// passthrough fallback for unknown inputs.

class TestTextFormat : public QObject {
    Q_OBJECT

private slots:
    void text_data();
    void text();

    void qualityLevelText_data();
    void qualityLevelText();

    void semanticTypeText_data();
    void semanticTypeText();

    void roleHintText_data();
    void roleHintText();

    void directionText_data();
    void directionText();

    void toolTraceText_data();
    void toolTraceText();

    void limitationText_data();
    void limitationText();
};

// --- text() ---------------------------------------------------------------

void TestTextFormat::text_data()
{
    QTestaddColumn<QString>("en");
    QTest.addColumn<QString>("zh");
    QTestaddColumn<bool>("useChinese");
    QTest.addColumn<QString>("expected");

    QTest.newRow("en-picks-en")   << "Hello" << "你好" << false << "Hello";
    QTest.newRow("zh-picks-zh")   << "Hello" << "你好" << true  << "你好";
}

void TestTextFormat::text()
{
    QFETCH(QString, en);
    QFETCH(QString, zh);
    QFETCH(bool, useChinese);
    QFETCH(QString, expected);
    QCOMPARE(TextFormat::text(en, zh, useChinese), expected);
}

// --- qualityLevelText() ---------------------------------------------------

void TestTextFormat::qualityLevelText_data()
{
    QTest.addColumn<QString>("level");
    QTest.addColumn<bool>("useChinese");
    QTest.addColumn<QString>("expected");

    // EN: passthrough
    QTest.newRow("en-high")   << "high"   << false << "high";
    QTest.newRow("en-medium") << "medium" << false << "medium";
    QTest.newRow("en-low")    << "low"    << false << "low";
    QTest.newRow("en-unknown") << "weird" << false << "weird";
    // ZH: mapped
    QTest.newRow("zh-high")   << "high"   << true << QStringLiteral("高");
    QTest.newRow("zh-medium") << "medium" << true << QStringLiteral("中");
    QTest.newRow("zh-low")    << "low"    << true << QStringLiteral("低");
    // ZH unknown: passthrough unchanged
    QTest.newRow("zh-unknown") << "weird" << true << "weird";
}

void TestTextFormat::qualityLevelText()
{
    QFETCH(QString, level);
    QFETCH(bool, useChinese);
    QFETCH(QString, expected);
    QCOMPARE(TextFormat::qualityLevelText(level, useChinese), expected);
}

// --- semanticTypeText() ---------------------------------------------------

void TestTextFormat::semanticTypeText_data()
{
    QTest.addColumn<QString>("type");
    QTest.addColumn<bool>("useChinese");
    QTest.addColumn<QString>("expected");

    QTest.newRow("en-numeric") << "numeric" << false << "numeric";
    QTest.newRow("zh-numeric") << "numeric" << true << QStringLiteral("数值");
    QTest.newRow("zh-date")    << "date"    << true << QStringLiteral("日期");
    QTest.newRow("zh-category") << "category" << true << QStringLiteral("分类");
    QTest.newRow("zh-text")    << "text"    << true << QStringLiteral("文本");
    QTest.newRow("zh-empty")   << "empty"   << true << QStringLiteral("空字段");
    QTest.newRow("zh-highcard") << "high_cardinality" << true << QStringLiteral("高基数字段");
    QTest.newRow("zh-unknown") << "weird"   << true << "weird";
}

void TestTextFormat::semanticTypeText()
{
    QFETCH(QString, type);
    QFETCH(bool, useChinese);
    QFETCH(QString, expected);
    QCOMPARE(TextFormat::semanticTypeText(type, useChinese), expected);
}

// --- roleHintText() -------------------------------------------------------

void TestTextFormat::roleHintText_data()
{
    QTestaddColumn<QString>("role");
    QTest.addColumn<bool>("useChinese");
    QTest.addColumn<QString>("expected");

    QTest.newRow("en-time_axis") << "time_axis" << false << "time_axis";
    QTest.newRow("zh-time_axis") << "time_axis" << true << QStringLiteral("时间轴");
    QTest.newRow("zh-dimension") << "dimension" << true << QStringLiteral("维度");
    QTest.newRow("zh-measure")  << "measure"  << true << QStringLiteral("指标");
    QTest.newRow("zh-biz")      << "business_measure" << true << QStringLiteral("业务指标");
    QTest.newRow("zh-identifier") << "identifier" << true << QStringLiteral("标识符");
    QTest.newRow("zh-unknown")  << "weird"    << true << "weird";
}

void TestTextFormat::roleHintText()
{
    QFETCH(QString, role);
    QFETCH(bool, useChinese);
    QFETCH(QString, expected);
    QCOMPARE(TextFormat::roleHintText(role, useChinese), expected);
}

// --- directionText() ------------------------------------------------------

void TestTextFormat::directionText_data()
{
    QTest.addColumn<QString>("direction");
    QTest.addColumn<bool>("useChinese");
    QTest.addColumn<QString>("expected");

    QTest.newRow("en-up")   << "up"   << false << "up";
    QTest.newRow("zh-up")   << "up"   << true << QStringLiteral("上升");
    QTest.newRow("zh-down") << "down" << true << QStringLiteral("下降");
    QTest.newRow("zh-flat") << "flat" << true << QStringLiteral("平稳");
    QTest.newRow("zh-unknown") << "weird" << true << "weird";
}

void TestTextFormat::directionText()
{
    QFETCH(QString, direction);
    QFETCH(bool, useChinese);
    QFETCH(QString, expected);
    QCOMPARE(TextFormat::directionText(direction, useChinese), expected);
}

// --- toolTraceText() ------------------------------------------------------

void TestTextFormat::toolTraceText_data()
{
    QTest.addColumn<QString>("step");
    QTest.addColumn<bool>("useChinese");
    QTest.addColumn<QString>("expected");

    // EN: underscores -> spaces
    QTest.newRow("en-load_table") << "load_table" << false << "load table";
    QTest.newRow("en-detect_anomalies") << "detect_anomalies" << false << "detect anomalies";
    // ZH: mapped
    QTest.newRow("zh-load_table") << "load_table" << true << QStringLiteral("加载表格");
    QTest.newRow("zh-detect_encoding") << "detect_encoding" << true << QStringLiteral("识别编码");
    QTest.newRow("zh-detect_delimiter") << "detect_delimiter" << true << QStringLiteral("识别分隔符");
    QTest.newRow("zh-infer_header") << "infer_header" << true << QStringLiteral("识别表头");
    QTest.newRow("zh-infer_schema") << "infer_schema" << true << QStringLiteral("推断字段结构");
    QTest.newRow("zh-profile_quality") << "profile_quality" << true << QStringLiteral("评估数据质量");
    QTest.newRow("zh-detect_anomalies") << "detect_anomalies" << true << QStringLiteral("检测异常");
    QTest.newRow("zh-recommend_analysis") << "recommend_analysis" << true << QStringLiteral("生成分析建议");
    QTest.newRow("zh-compose_insight") << "compose_insight" << true << QStringLiteral("生成洞察摘要");
    // ZH unknown: passthrough unchanged (no underscore replacement in ZH)
    QTest.newRow("zh-unknown") << "weird_step" << true << "weird_step";
}

void TestTextFormat::toolTraceText()
{
    QFETCH(QString, step);
    QFETCH(bool, useChinese);
    QFETCH(QString, expected);
    QCOMPARE(TextFormat::toolTraceText(step, useChinese), expected);
}

// --- limitationText() -----------------------------------------------------

void TestTextFormat::limitationText_data()
{
    QTest.addColumn<QString>("value");
    QTest.addColumn<bool>("useChinese");
    QTest.addColumn<QString>("expected");

    // EN: passthrough
    QTest.newRow("en-sample") << "The sample is small" << false << "The sample is small";
    // ZH: prefix-mapped to full sentence
    QTest.newRow("zh-sample") << "The sample is small" << true
        << QStringLiteral("样本量偏小，趋势和异常信号只能作为复核线索。");
    QTest.newRow("zh-missing") << "Missing values found" << true
        << QStringLiteral("缺失值可能改变总和、均值和排序结果。");
    QTest.newRow("zh-no-time") << "No reliable time field" << true
        << QStringLiteral("未识别到可靠时间字段，趋势图会按记录顺序展示。");
    QTest.newRow("zh-no-group") << "No clear grouping field" << true
        << QStringLiteral("未识别到清晰分组字段，分组对比能力有限。");
    QTest.newRow("zh-exploratory") << "Findings are exploratory" << true
        << QStringLiteral("当前结论属于探索性分析，正式决策前需要结合业务背景验证。");
    // ZH unknown: passthrough unchanged
    QTest.newRow("zh-unknown") << "Some other limitation" << true << "Some other limitation";
}

void TestTextFormat::limitationText()
{
    QFETCH(QString, value);
    QFETCH(bool, useChinese);
    QFETCH(QString, expected);
    QCOMPARE(TextFormat::limitationText(value, useChinese), expected);
}

QTEST_MAIN(TestTextFormat)
#include "test_textformat.moc"
