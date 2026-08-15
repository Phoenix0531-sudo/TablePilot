#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"   // 引入绘图库的头文件
#include <QDockWidget>
#include <QFrame>
#include <QLabel>          // 引入QLabel类
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QJsonArray>
#include <QJsonObject>

class WebResearchDock;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr); // 构造函数
    ~MainWindow(); // 析构函数
    void OpenFileFromPath(const QString &filePath);

private:
    Ui::MainWindow *ui;  // UI对象指针,Qt Designer自动生成的一个指向ui界面中定义的所有部件的指针

    // 初始化窗口部件和对象
    void InitWidget();
    void InitObject();

    QAction* m_pAction1; // 功能标签1的动作
    QAction* m_pAction2; // 功能标签2的动作
    QAction* m_pAction3; // 功能标签3的动作
    QAction* m_pAction4; // 功能标签4的动作
    QAction* m_pAction5; // 功能标签5的动作
    QAction* m_pAction6; // 功能标签6的动作
    QAction* m_pAction7; // 功能标签7的动作
    QAction* m_pAction8; // 智能分析动作
    QAction* m_pAction9; // 语言切换动作

    // 创建工具栏、标签、状态栏、样式等
    void createToolBar();
    void createActions();
    void createStatusBar();
    void createStyle();
    void createOverviewPanel();
    void createInsightPanel();
    void createChartHeaders();
    void ApplyLanguage();

// 槽函数，处理各个功能标签的点击事件
private slots:
    void Slot1();
    void Slot2();
    void Slot3();
    void Slot4();
    void Slot5();
    void Slot6();
    void Slot7();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_clicked();

private:
    QLabel *info_Label; // 信息标签
    QDockWidget *insightDock;
    QDockWidget *reviewDock;
    QTextEdit *insightText;
    QTextEdit *cleanCompareText;
    QTableWidget *anomalyTable;
    QLabel *serviceBadge;
    QLabel *datasetCard;
    QLabel *qualityCard;
    QLabel *schemaCard;
    QLabel *recommendationCard;
    QLabel *eyebrowLabel;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QLabel *trendChartTitleLabel;
    QLabel *trendChartSubtitleLabel;
    QLabel *distributionChartTitleLabel;
    QLabel *distributionChartSubtitleLabel;
    QLabel *metricSelectorLabel;
    QLabel *sheetSelectorLabel;
    QLabel *toolbarServiceLabel;
    QLabel *toolbarModelLabel;
    QPushButton *toolbarLanguageButton;
    QPushButton *localAiToggleButton;
    QPushButton *regenerateAiButton;
    QPushButton *recentFilesButton;
    QPushButton *cleanExportButton;
    QPushButton *cleanCompareButton;
    QPushButton *suggestTrendButton;
    QPushButton *suggestDistributionButton;
    QPushButton *suggestQualityButton;
    QPushButton *exportReportButton;
    QComboBox *chartTypeSelector;
    QComboBox *chartMetricSelector;
    QComboBox *chartDimensionSelector;
    QComboBox *sheetSelector;
    QJsonObject lastProfile;
    QJsonObject lastCleanCompare;
    WebResearchDock *m_webResearchDock = nullptr;
    QPushButton *m_webResearchToolbarButton = nullptr;
    QString currentFilePath;
    QStringList recentFiles;
    bool updatingSheetSelector;
    bool localAiRequested;
    bool useChinese;
    bool isExit;        // 是否退出标志
    int  SaveType;      // 保存类型
    bool SavePic(QString fileName, QCustomPlot *p_save); // 保存绘图到文件
    void SetStyleSheet(QWidget *pWidget, QString strQSS); // 设置样式表
    void AnalyzeFileWithService(const QString &filePath, const QString &sheetName = QString());
    void CheckAnalysisService();
    bool IsAnalysisServiceHealthy(int timeoutMs = 2500);
    bool TryStartAnalysisService();
    void ShowServiceAnalysis(const QByteArray &payload);
    void PopulateTableFromService(const QJsonObject &root);
    void PopulateStatsFromService(const QJsonObject &root);
    void UpdateOverviewCards(const QJsonObject &root);
    void UpdateDefaultOverviewCards(const QString &serviceState);
    void UpdateFieldSelectors(const QJsonObject &root);
    void UpdateToolbarState(const QJsonObject &root = QJsonObject());
    void UpdateReviewDrawer(const QJsonObject &root);
    void UpdateSheetSelector(const QJsonObject &root);
    void UpdateRecommendationActions(const QJsonObject &root);
    void ApplyTableQualityDecorations(const QJsonObject &root);
    QString FormatServiceAnalysis(const QJsonObject &root) const;
    QString FormatInsightHtml(const QJsonObject &root) const;
    QString Text(const QString &en, const QString &zh) const;
    QString QualityLevelText(const QString &level) const;
    QString SemanticTypeText(const QString &type) const;
    QString RoleHintText(const QString &role) const;
    QString DirectionText(const QString &direction) const;
    QString ToolTraceText(const QString &step) const;
    QString RecommendationTitle(const QJsonObject &item) const;
    QString RecommendationReason(const QJsonObject &item) const;
    QString PlanTitle(const QJsonObject &step) const;
    QString PlanReason(const QJsonObject &step) const;
    QString InsightText(const QString &value, const QJsonObject &root) const;
    QString DecisionTitleText(const QJsonObject &finding) const;
    QString DecisionExplanationText(const QJsonObject &finding) const;
    QString DecisionEvidenceText(const QJsonObject &finding) const;
    QString DecisionActionText(const QJsonObject &finding) const;
    QString LimitationText(const QString &value) const;
    QString ViewReasonText(const QJsonObject &view) const;
    QString CardTitleText(const QJsonObject &card) const;
    QString CardSummaryText(const QJsonObject &card) const;
    QString CardEvidenceText(const QJsonObject &card) const;
    QString RepairTitleText(const QJsonObject &item) const;
    QString RepairRecommendationText(const QJsonObject &item) const;
    QString RepairImpactText(const QJsonObject &item) const;
    QString ViewLabelText(const QJsonObject &view) const;
    QString FormatCleanCompareHtml(const QJsonObject &root) const;
    void LoadRecentFiles();
    void SaveRecentFile(const QString &filePath);
    void RefreshRecentFilesMenu();
    void OpenRecentFile(const QString &filePath);
    QList<int> NumericTableColumns(int limit = -1) const;
    QList<int> CategoryTableColumns(int limit = -1) const;
    QVector<double> NumericColumnValues(int column) const;
    QVector<double> NumericColumnValuesForRows(int column, QVector<int> *rows = nullptr) const;
    QVector<double> NumericRowIndex(int size) const;
    QString ColumnLabel(int column) const;
    int ColumnIndexByName(const QString &name) const;
    void RenderChartStudio();
    void RenderDynamicLineChart();
    void RenderDynamicBarChart();
    void RenderGroupedBarChart();
    void RenderScatterChart();
    void RenderCorrelationHeatmap();
    void RenderBoxPlot();
    void RenderEmptyChart(QCustomPlot *plot, const QString &message);
    void ExportCleanedDataset();
    void ShowCleanCompare();
    void FocusAnomaly(int anomalyIndex);
    void FocusDataQuality();
    void StylePlot(QCustomPlot *plot);
};

#endif // MAINWINDOW_H
