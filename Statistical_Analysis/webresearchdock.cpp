#include "webresearchdock.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QNetworkRequest>
#include <QAction>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QClipboard>
#include <QProcessEnvironment>
#include <QMenu>

WebResearchDock::WebResearchDock(QWidget *parent)
    : QDockWidget(parent)
    , m_useChinese(false)
{
    setObjectName(QStringLiteral("webResearchDock"));
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    setMinimumWidth(300);

    m_container = new QWidget(this);
    QVBoxLayout *root = new QVBoxLayout(m_container);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    // --- query row -----------------------------------------------------------
    QHBoxLayout *queryRow = new QHBoxLayout;
    queryRow->setSpacing(6);
    m_queryEdit = new QLineEdit(m_container);
    m_queryEdit->setObjectName(QStringLiteral("webResearchQuery"));
    m_queryEdit->setPlaceholderText(QStringLiteral("Web research query"));
    queryRow->addWidget(m_queryEdit, 1);
    m_searchButton = new QPushButton(m_container);
    m_searchButton->setObjectName(QStringLiteral("webResearchSearch"));
    queryRow->addWidget(m_searchButton);
    root->addLayout(queryRow);

    // --- status line ---------------------------------------------------------
    m_statusLabel = new QLabel(m_container);
    m_statusLabel->setObjectName(QStringLiteral("webResearchStatus"));
    m_statusLabel->setWordWrap(true);
    root->addWidget(m_statusLabel);

    // --- results list --------------------------------------------------------
    m_resultsList = new QListWidget(m_container);
    m_resultsList->setObjectName(QStringLiteral("webResearchResults"));
    m_resultsList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultsList->setContextMenuPolicy(Qt::CustomContextMenu);
    root->addWidget(m_resultsList, 1);

    // --- hint footer ---------------------------------------------------------
    m_hintLabel = new QLabel(m_container);
    m_hintLabel->setObjectName(QStringLiteral("webResearchHint"));
    m_hintLabel->setWordWrap(true);
    root->addWidget(m_hintLabel);

    setWidget(m_container);

    m_net = new QNetworkAccessManager(this);
    connect(m_searchButton, &QPushButton::clicked, this, &WebResearchDock::onSearch);
    connect(m_queryEdit, &QLineEdit::returnPressed, this, &WebResearchDock::onSearch);
    connect(m_net, &QNetworkAccessManager::finished, this, &WebResearchDock::onResult);
    connect(m_resultsList, &QListWidget::itemActivated, this, &WebResearchDock::onItemActivated);
    connect(m_resultsList, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QListWidgetItem *item = m_resultsList->itemAt(pos);
        if (!item) return;
        int idx = m_resultsList->row(item);
        if (idx < 0 || idx >= m_results.size()) return;
        QMenu menu(this);
        QAction *open = menu.addAction(tr2("Open in browser", "在浏览器中打开"));
        QAction *copy = menu.addAction(tr2("Copy URL", "复制链接"));
        QAction *chosen = menu.exec(m_resultsList->viewport()->mapToGlobal(pos));
        if (chosen == open) {
            QDesktopServices::openUrl(QUrl(m_results[idx].url));
        } else if (chosen == copy) {
            QGuiApplication::clipboard()->setText(m_results[idx].url);
        }
    });

    applyStrings();
}

WebResearchDock::~WebResearchDock() = default;

void WebResearchDock::setUseChinese(bool zh)
{
    if (m_useChinese == zh) return;
    m_useChinese = zh;
    retranslateUi();
}

void WebResearchDock::retranslateUi() { applyStrings(); }

QString WebResearchDock::tr2(const QString &en, const QString &zh) const
{
    return m_useChinese ? zh : en;
}

void WebResearchDock::applyStrings()
{
    setWindowTitle(tr2("Web research", "网络检索"));
    m_queryEdit->setPlaceholderText(tr2("Web research query", "输入检索词"));
    m_searchButton->setText(tr2("Search", "检索"));
    // Hint always tells the user how to enable the key.
    m_hintLabel->setText(tr2(
        "Set the EXA_API_KEY environment variable to enable Exa search. "
        "Results open in your browser; no key is stored in the app.",
        "设置 EXA_API_KEY 环境变量即可启用 Exa 检索。结果在浏览器打开，"
        "应用不保存任何密钥。"));

    // Re-emit an idle status if there are no results yet.
    if (m_results.isEmpty()) {
        m_statusLabel->setText(tr2("Idle — type a query and press Enter.",
                                   "空闲 —— 输入检索词后回车。"));
    }
}

void WebResearchDock::onSearch()
{
    const QString query = m_queryEdit->text().trimmed();
    if (query.isEmpty()) {
        m_statusLabel->setText(tr2("Type a query first.", "请先输入检索词。"));
        return;
    }

    // Read the key live from the environment so users can export it after
    // the app started; we never persist or hardcode it.
    const QString key = QProcessEnvironment::systemEnvironment().value(
        QStringLiteral("EXA_API_KEY")).trimmed();
    if (key.isEmpty()) {
        m_results.clear();
        m_resultsList->clear();
        m_statusLabel->setText(tr2(
            "EXA_API_KEY not set — set it and retry. See the hint below.",
            "未设置 EXA_API_KEY —— 设置后再检索（见下方提示）。"));
        return;
    }

    m_statusLabel->setText(tr2("Searching…", "检索中…"));
    m_searchButton->setEnabled(false);

    QJsonObject body;
    body.insert(QStringLiteral("query"), query);
    body.insert(QStringLiteral("numResults"), 10);
    QJsonObject contents;
    QJsonObject text;
    text.insert(QStringLiteral("maxCharacters"), 800);
    contents.insert(QStringLiteral("text"), text);
    body.insert(QStringLiteral("contents"), contents);

    QNetworkRequest req(QUrl(QStringLiteral("https://api.exa.ai/search")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(key).toUtf8());

    m_net->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
}

void WebResearchDock::onResult(QNetworkReply *reply)
{
    m_searchButton->setEnabled(true);
    reply->deleteLater();

    m_results.clear();
    m_resultsList->clear();

    if (reply->error() != QNetworkReply::NoError) {
        m_statusLabel->setText(tr2(
            "Request failed: ", "请求失败：") + reply->errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonArray arr = doc.object().value(QStringLiteral("results")).toArray();
    if (arr.isEmpty()) {
        m_statusLabel->setText(tr2("No results.", "无结果。"));
        return;
    }

    int idx = 0;
    for (const QJsonValue &v : arr) {
        const QJsonObject o = v.toObject();
        ResultItem r;
        r.title = o.value(QStringLiteral("title")).toString();
        r.url = o.value(QStringLiteral("url")).toString();
        r.text = o.value(QStringLiteral("text")).toString();
        r.publishedDate = o.value(QStringLiteral("publishedDate")).toString();
        r.score = o.value(QStringLiteral("score")).toDouble(0.0);
        m_results.append(r);

        QString label = QStringLiteral("%1. %2").arg(idx + 1).arg(r.title);
        if (!r.url.isEmpty()) label += QStringLiteral("  —  ") + r.url;
        QListWidgetItem *item = new QListWidgetItem(label, m_resultsList);
        item->setToolTip(r.text.isEmpty() ? r.url : r.text.left(400));
        m_resultsList->addItem(item);
        ++idx;
    }
    m_statusLabel->setText(tr2("%1 result(s). Double-click to open in browser.",
                              "%1 条结果。双击在浏览器打开。").arg(m_results.size()));
}

void WebResearchDock::onItemActivated(QListWidgetItem *item)
{
    if (!item) return;
    int idx = m_resultsList->row(item);
    if (idx < 0 || idx >= m_results.size()) return;
    QDesktopServices::openUrl(QUrl(m_results[idx].url));
}
