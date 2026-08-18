#include "textformat.h"

// Implementation mirrors the original MainWindow::X() const methods exactly.
// See textformat.h for the rationale (extraction for unit testing).

namespace TextFormat {

QString text(const QString &en, const QString &zh, bool useChinese)
{
    return useChinese ? zh : en;
}

QString qualityLevelText(const QString &level, bool useChinese)
{
    if (!useChinese) {
        return level;
    }
    if (level == "high") return QStringLiteral("高");
    if (level == "medium") return QStringLiteral("中");
    if (level == "low") return QStringLiteral("低");
    return level;
}

QString semanticTypeText(const QString &type, bool useChinese)
{
    if (!useChinese) {
        return type;
    }
    if (type == "numeric") return QStringLiteral("数值");
    if (type == "date") return QStringLiteral("日期");
    if (type == "category") return QStringLiteral("分类");
    if (type == "text") return QStringLiteral("文本");
    if (type == "empty") return QStringLiteral("空字段");
    if (type == "high_cardinality") return QStringLiteral("高基数字段");
    return type;
}

QString roleHintText(const QString &role, bool useChinese)
{
    if (!useChinese) {
        return role;
    }
    if (role == "time_axis") return QStringLiteral("时间轴");
    if (role == "dimension") return QStringLiteral("维度");
    if (role == "measure") return QStringLiteral("指标");
    if (role == "business_measure") return QStringLiteral("业务指标");
    if (role == "identifier") return QStringLiteral("标识符");
    return role;
}

QString directionText(const QString &direction, bool useChinese)
{
    if (!useChinese) {
        return direction;
    }
    if (direction == "up") return QStringLiteral("上升");
    if (direction == "down") return QStringLiteral("下降");
    if (direction == "flat") return QStringLiteral("平稳");
    return direction;
}

QString toolTraceText(const QString &step, bool useChinese)
{
    if (!useChinese) {
        QString text = step;
        return text.replace("_", " ");
    }
    if (step == "load_table") return QStringLiteral("加载表格");
    if (step == "detect_encoding") return QStringLiteral("识别编码");
    if (step == "detect_delimiter") return QStringLiteral("识别分隔符");
    if (step == "infer_header") return QStringLiteral("识别表头");
    if (step == "infer_schema") return QStringLiteral("推断字段结构");
    if (step == "profile_quality") return QStringLiteral("评估数据质量");
    if (step == "detect_anomalies") return QStringLiteral("检测异常");
    if (step == "recommend_analysis") return QStringLiteral("生成分析建议");
    if (step == "compose_insight") return QStringLiteral("生成洞察摘要");
    return step;
}

QString limitationText(const QString &value, bool useChinese)
{
    if (!useChinese) {
        return value;
    }
    if (value.startsWith("The sample is small")) {
        return QStringLiteral("样本量偏小，趋势和异常信号只能作为复核线索。");
    }
    if (value.startsWith("Missing values")) {
        return QStringLiteral("缺失值可能改变总和、均值和排序结果。");
    }
    if (value.startsWith("No reliable time field")) {
        return QStringLiteral("未识别到可靠时间字段，趋势图会按记录顺序展示。");
    }
    if (value.startsWith("No clear grouping field")) {
        return QStringLiteral("未识别到清晰分组字段，分组对比能力有限。");
    }
    if (value.startsWith("Findings are exploratory")) {
        return QStringLiteral("当前结论属于探索性分析，正式决策前需要结合业务背景验证。");
    }
    return value;
}

} // namespace TextFormat
