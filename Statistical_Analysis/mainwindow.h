#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"   // 引入绘图库的头文件
#include <QDockWidget>
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
    void createInsightPanel();

    // 判断指定位置的单元格是否为空
    bool ItemEmpty(int x, int y);

    // 一些统计数据的变量
    int A[6] = {0}, B[6] = {0}, C[6] = {0}, D[6] = {0}, E[6] = {0}, F[6] = {0};
    int sumA = 0, sumB = 0, sumC = 0, sumD = 0, sumE = 0, sumF = 0;
    int regA = 0, regB = 0, regC = 0, regD = 0, regE = 0, regF = 0;
    int minA = 0, minB = 0, minC = 0, minD = 0, minE = 0, minF = 0;
    int maxA = 0, maxB = 0, maxC = 0, maxD = 0, maxE = 0, maxF = 0;

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
    bool isExit;        // 是否退出标志
    int  SaveType;      // 保存类型
    bool SavePic(QString fileName, QCustomPlot *p_save); // 保存绘图到文件
    void SetStyleSheet(QWidget *pWidget, QString strQSS); // 设置样式表
    void AnalyzeFileWithService(const QString &filePath);
    void CheckAnalysisService();
    void ShowServiceAnalysis(const QByteArray &payload);
    void PopulateTableFromService(const QJsonObject &root);
    void PopulateStatsFromService(const QJsonObject &root);
    QString FormatServiceAnalysis(const QJsonObject &root) const;
    QList<int> NumericTableColumns(int limit = -1) const;
    QVector<double> NumericColumnValues(int column) const;
    QVector<double> NumericRowIndex(int size) const;
    QString ColumnLabel(int column) const;
    void RenderDynamicLineChart();
    void RenderDynamicBarChart();
};

#endif // MAINWINDOW_H

/*
这段代码定义了一个MainWindow类，继承自QMainWindow，用于主窗口的显示和交互。该类包括一些私有成员变量、私有方法、私有槽函数和一些用于处理用户交互的公有槽函数。
私有成员变量包括UI对象指针、Excel文档对象指针、一些统计数据的变量、保存类型等。
私有方法包括初始化窗口部件和对象、创建工具栏、标签、状态栏、样式等。私有槽函数处理各个功能标签的点击事件，以及按钮的点击事件。
*/
