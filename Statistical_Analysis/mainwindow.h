#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"   // 引入绘图库的头文件
#include <QDockWidget>
#include <QFrame>
#include <QLabel>          // 引入QLabel类
#include <QTextEdit>
#include <QJsonArray>
#include <QJsonObject>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr); // 构造函数
    ~MainWindow(); // 析构函数

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

    // 创建工具栏、标签、状态栏、样式等
    void createToolBar();
    void createActions();
    void createStatusBar();
    void createStyle();
    void createOverviewPanel();
    void createInsightPanel();

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
    QTextEdit *insightText;
    QLabel *serviceBadge;
    QLabel *datasetCard;
    QLabel *qualityCard;
    QLabel *schemaCard;
    QLabel *recommendationCard;
    QJsonObject lastProfile;
    bool isExit;        // 是否退出标志
    int  SaveType;      // 保存类型
    bool SavePic(QString fileName, QCustomPlot *p_save); // 保存绘图到文件
    void SetStyleSheet(QWidget *pWidget, QString strQSS); // 设置样式表
    void AnalyzeFileWithService(const QString &filePath);
    void CheckAnalysisService();
    bool IsAnalysisServiceHealthy(int timeoutMs = 2500);
    bool TryStartAnalysisService();
    void ShowServiceAnalysis(const QByteArray &payload);
    void PopulateTableFromService(const QJsonObject &root);
    void PopulateStatsFromService(const QJsonObject &root);
    void UpdateOverviewCards(const QJsonObject &root);
    void UpdateFieldSelectors(const QJsonObject &root);
    void ApplyTableQualityDecorations(const QJsonObject &root);
    QString FormatServiceAnalysis(const QJsonObject &root) const;
    QString FormatInsightHtml(const QJsonObject &root) const;
    QList<int> NumericTableColumns(int limit = -1) const;
    QVector<double> NumericColumnValues(int column) const;
    QVector<double> NumericRowIndex(int size) const;
    QString ColumnLabel(int column) const;
    void RenderDynamicLineChart();
    void RenderDynamicBarChart();
    void StylePlot(QCustomPlot *plot);
};

#endif // MAINWINDOW_H
