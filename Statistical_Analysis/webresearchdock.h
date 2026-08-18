#ifndef WEBRESEARCHDOCK_H
#define WEBRESEARCHDOCK_H

#include <QDockWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVector>

/**
 * Self-contained web research dock for the TablePilot desktop shell.
 *
 * Calls the Exa Search API (POST https://api.exa.ai/search) with a user
 * query and renders the returned results as a clickable list. The Exa API
 * key is read from the EXA_API_KEY environment variable at search time, so
 * no key is ever stored in source. If the key is absent the dock shows a
 * friendly "set EXA_API_KEY" message and does not fire any network request.
 *
 * Design goals:
 *   - zero coupling to MainWindow internals (no friendship, no shared state);
 *   - graceful no-key degradation so unconfigured users still see the dock;
 *   - results open in the system browser via QDesktopServices::openUrl.
 *
 * Localization is intentionally minimal: a small bilingual helper mirrors
 * MainWindow::Text() so the dock reads its strings the same way the rest
 * of the app does. Keeping it standalone means MainWindow does not need to
 * teach this class about useChinese.
 */
class WebResearchDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit WebResearchDock(QWidget *parent = nullptr);
    ~WebResearchDock() override;

    void setUseChinese(bool zh);
    void retranslateUi();

    // Public so QtTest can construct fixtures without going through a live
    // QNetworkReply. The Exa API returns { results: [ {title, url, text,
    // publishedDate, score} ] }; this pure parser turns that JSON into the
    // dock's internal result vector, defaulting missing fields safely.
    struct ResultItem {
        QString title;
        QString url;
        QString text;
        QString publishedDate;
        double score;
    };
    static QVector<ResultItem> parseResults(const QJsonDocument &doc);

private slots:
    void onSearch();
    void onResult(QNetworkReply *reply);
    void onItemActivated(QListWidgetItem *item);

private:
    void applyStrings();
    QString tr2(const QString &en, const QString &zh) const;

    QWidget *m_container;
    QLineEdit *m_queryEdit;
    QPushButton *m_searchButton;
    QLabel *m_statusLabel;
    QListWidget *m_resultsList;
    QLabel *m_hintLabel;
    QNetworkAccessManager *m_net;
    bool m_useChinese;

    QVector<ResultItem> m_results;
};

#endif // WEBRESEARCHDOCK_H
