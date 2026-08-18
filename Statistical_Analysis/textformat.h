#ifndef TEXTFORMAT_H
#define TEXTFORMAT_H

// Pure, dependency-free localization helpers extracted from MainWindow.
//
// These map short service-level enum strings ("high", "numeric", "up",
// "load_table", ...) to user-facing EN/ZH labels. They were private const
// methods on MainWindow that depended only on the `useChinese` member; lifting
// them into a free namespace lets QtTest exercise the mapping tables directly
// without instantiating the full MainWindow (which pulls in the QSS loader,
// the service client, QCustomPlot, and every dock widget).
//
// Each function mirrors the original MainWindow::X(const QString&) const
// behavior exactly: when useChinese is false the input is returned unchanged
// (English passthrough); when true the known enum values are translated and
// unknown values are passed through unchanged.

#include <QString>

namespace TextFormat {

// Pick the EN or ZH string based on the language flag.
QString text(const QString &en, const QString &zh, bool useChinese);

// quality.level -> localized label.
QString qualityLevelText(const QString &level, bool useChinese);

// schema semantic type -> localized label.
QString semanticTypeText(const QString &type, bool useChinese);

// schema role hint -> localized label.
QString roleHintText(const QString &role, bool useChinese);

// trend direction -> localized label.
QString directionText(const QString &direction, bool useChinese);

// tool_trace step -> localized label (EN: underscores -> spaces).
QString toolTraceText(const QString &step, bool useChinese);

// analysis limitation sentence prefix -> localized full sentence.
QString limitationText(const QString &value, bool useChinese);

} // namespace TextFormat

#endif // TEXTFORMAT_H
