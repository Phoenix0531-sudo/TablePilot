#include "mainwindow.h"   // 包含自定义的MainWindow类的头文件
#include "ui_mainwindow.h" // 包含自动生成的ui界面文件的头文件
#include <QAbstractItemView>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>     // 包含文件对话框类的头文件
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>          // 包含标签类的头文件，用于创建标签部件
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QSettings>
#include <QStyle>
#include <QSet>
#include <QToolButton>
#include <QUrl>
#include <QEventLoop>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QFont>
#include <QTimer>
#include <QSizePolicy>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace {
QString ProjectPath(const QString &relativePath)
{
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 6; ++i) {
        QString candidate = dir.filePath(relativePath);
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
        dir.cdUp();
    }
    return QDir::current().filePath(relativePath);
}
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), // 调用基类的构造函数
    ui(new Ui::MainWindow), // 初始化ui对象指针，指向自动生成的ui界面
    reviewDock(nullptr),
    cleanCompareText(nullptr),
    anomalyTable(nullptr),
    trendChartTitleLabel(nullptr),
    trendChartSubtitleLabel(nullptr),
    distributionChartTitleLabel(nullptr),
    distributionChartSubtitleLabel(nullptr),
    metricSelectorLabel(nullptr),
    sheetSelectorLabel(nullptr),
    toolbarServiceLabel(nullptr),
    toolbarModelLabel(nullptr),
    toolbarLanguageButton(nullptr),
    localAiToggleButton(nullptr),
    regenerateAiButton(nullptr),
    recentFilesButton(nullptr),
    cleanExportButton(nullptr),
    cleanCompareButton(nullptr),
    suggestTrendButton(nullptr),
    suggestDistributionButton(nullptr),
    suggestQualityButton(nullptr),
    exportReportButton(nullptr),
    chartTypeSelector(nullptr),
    chartMetricSelector(nullptr),
    chartDimensionSelector(nullptr),
    sheetSelector(nullptr),
    updatingSheetSelector(false),
    localAiRequested(false),
    useChinese(false),
    isExit(false), // 初始化是否退出标志为false
    SaveType(0)   // 初始化保存类型为0
{
    ui->setupUi(this); // 设置ui界面，将自动生成的ui界面应用到当前窗口上
    setWindowTitle(QStringLiteral("TablePilot"));

    InitWidget(); // 调用初始化窗口部件和对象的函数
    InitObject(); // 调用初始化对象的函数
}

MainWindow::~MainWindow(){
    delete ui; // 删除ui对象指针，释放内存
}

void MainWindow::InitWidget(){
    createActions();    // 调用创建各个功能标签的动作函数
    createToolBar();    // 调用创建工具栏的函数
    createStatusBar(); // 调用创建状态栏的函数
    createStyle();      // 调用创建样式的函数
    createChartHeaders();
    createOverviewPanel();
    createInsightPanel();
    LoadRecentFiles();
    ApplyLanguage();
}

void MainWindow::InitObject(){
    // 初始化对象
    CheckAnalysisService();
}

void MainWindow::createToolBar(){// 创建工具栏

    ui->mainToolBar->setMovable(false); // 设置工具栏不可移动
    ui->mainToolBar->clear();
    ui->mainToolBar->setIconSize(QSize(1, 1));
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    ui->mainToolBar->setFixedHeight(58);

    QWidget *bar = new QWidget(ui->mainToolBar);
    bar->setObjectName(QStringLiteral("commandBar"));
    QHBoxLayout *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->setSpacing(8);

    QLabel *brand = new QLabel(QStringLiteral("<b>TablePilot</b><br><span>Messy Table Autopilot</span>"), bar);
    brand->setObjectName(QStringLiteral("toolbarBrand"));
    brand->setTextFormat(Qt::RichText);
    brand->setMinimumWidth(190);
    layout->addWidget(brand);

    auto makeButton = [bar](const QString &key, QAction *action) {
        QPushButton *button = new QPushButton(bar);
        button->setObjectName(QStringLiteral("commandButton"));
        button->setProperty("actionKey", key);
        button->setText(action->text());
        button->setToolTip(action->toolTip());
        button->setCursor(Qt::PointingHandCursor);
        QObject::connect(button, &QPushButton::clicked, action, &QAction::trigger);
        return button;
    };

    layout->addWidget(makeButton(QStringLiteral("open_excel"), m_pAction1));
    layout->addWidget(makeButton(QStringLiteral("open_text"), m_pAction2));
    layout->addWidget(makeButton(QStringLiteral("analyze"), m_pAction8));
    layout->addWidget(makeButton(QStringLiteral("chart_studio"), m_pAction4));
    layout->addWidget(makeButton(QStringLiteral("profile"), m_pAction3));

    recentFilesButton = new QPushButton(bar);
    recentFilesButton->setObjectName(QStringLiteral("commandButtonSecondary"));
    recentFilesButton->setProperty("actionKey", QStringLiteral("recent_files"));
    recentFilesButton->setCursor(Qt::PointingHandCursor);
    recentFilesButton->setMenu(new QMenu(recentFilesButton));
    layout->addWidget(recentFilesButton);

    cleanExportButton = new QPushButton(bar);
    cleanExportButton->setObjectName(QStringLiteral("commandButtonSecondary"));
    cleanExportButton->setProperty("actionKey", QStringLiteral("clean_export"));
    cleanExportButton->setCursor(Qt::PointingHandCursor);
    cleanExportButton->setEnabled(false);
    layout->addWidget(cleanExportButton);
    connect(cleanExportButton, &QPushButton::clicked, this, [this]() {
        ExportCleanedDataset();
    });

    cleanCompareButton = new QPushButton(bar);
    cleanCompareButton->setObjectName(QStringLiteral("commandButtonSecondary"));
    cleanCompareButton->setProperty("actionKey", QStringLiteral("clean_compare"));
    cleanCompareButton->setCursor(Qt::PointingHandCursor);
    cleanCompareButton->setEnabled(false);
    layout->addWidget(cleanCompareButton);
    connect(cleanCompareButton, &QPushButton::clicked, this, [this]() {
        ShowCleanCompare();
    });

    layout->addStretch(1);

    toolbarServiceLabel = new QLabel(bar);
    toolbarServiceLabel->setObjectName(QStringLiteral("toolbarPill"));
    toolbarModelLabel = new QLabel(bar);
    toolbarModelLabel->setObjectName(QStringLiteral("toolbarPill"));
    layout->addWidget(toolbarServiceLabel);
    layout->addWidget(toolbarModelLabel);

    localAiToggleButton = new QPushButton(bar);
    localAiToggleButton->setObjectName(QStringLiteral("commandButtonSecondary"));
    localAiToggleButton->setProperty("actionKey", QStringLiteral("ai_toggle"));
    localAiToggleButton->setCursor(Qt::PointingHandCursor);
    layout->addWidget(localAiToggleButton);
    connect(localAiToggleButton, &QPushButton::clicked, this, [this]() {
        localAiRequested = !localAiRequested;
        UpdateToolbarState(lastProfile);
        if (!currentFilePath.isEmpty()) {
            AnalyzeFileWithService(currentFilePath, sheetSelector && sheetSelector->isVisible() ? sheetSelector->currentText() : QString());
        }
    });

    regenerateAiButton = new QPushButton(bar);
    regenerateAiButton->setObjectName(QStringLiteral("commandButtonSecondary"));
    regenerateAiButton->setProperty("actionKey", QStringLiteral("regenerate_ai"));
    regenerateAiButton->setCursor(Qt::PointingHandCursor);
    regenerateAiButton->setEnabled(false);
    layout->addWidget(regenerateAiButton);
    connect(regenerateAiButton, &QPushButton::clicked, this, [this]() {
        if (!currentFilePath.isEmpty()) {
            AnalyzeFileWithService(currentFilePath, sheetSelector && sheetSelector->isVisible() ? sheetSelector->currentText() : QString());
        }
    });

    toolbarLanguageButton = new QPushButton(bar);
    toolbarLanguageButton->setObjectName(QStringLiteral("commandButtonSecondary"));
    toolbarLanguageButton->setProperty("actionKey", QStringLiteral("language"));
    toolbarLanguageButton->setCursor(Qt::PointingHandCursor);
    layout->addWidget(toolbarLanguageButton);
    connect(toolbarLanguageButton, &QPushButton::clicked, m_pAction9, &QAction::trigger);

    layout->addWidget(makeButton(QStringLiteral("quit"), m_pAction7));

    ui->mainToolBar->addWidget(bar);
}

void MainWindow::createActions(){// 创建各个功能标签的动作
    //设置功能标签的图标，文字，以及该动作属于哪个父窗口

    m_pAction1 = new QAction(QString("Open Excel"), this);
    m_pAction1->setToolTip("打开 Excel 工作簿并调用本地分析服务");
    m_pAction2 = new QAction(QString("Open Text"), this);
    m_pAction2->setToolTip("打开 TXT/CSV 表格并自动识别分隔符");
    m_pAction3 = new QAction(QString("Profile"), this);
    m_pAction3->setToolTip("刷新数值字段统计画像");
    m_pAction4 = new QAction(QString("Charts"), this);
    m_pAction4->setToolTip("打开 Chart Studio 推荐图表");
    m_pAction5 = new QAction(QString("Distribution"), this);
    m_pAction5->setToolTip("查看首个数值字段分布");
    m_pAction6 = new QAction(QString("Export"), this);
    m_pAction6->setToolTip("导出当前图表为图片或 PDF");
    m_pAction7 = new QAction(QString("Quit"), this);
    m_pAction7->setToolTip("退出系统");
    m_pAction8 = new QAction(QString("Analyze"), this);
    m_pAction8->setToolTip("打开任意 Excel、CSV 或 TXT 表格进行智能画像");
    m_pAction9 = new QAction(QString("中文"), this);
    m_pAction9->setToolTip("Switch language / 切换语言");

    // 连接各个动作的触发信号到槽函数
    connect(m_pAction1, SIGNAL(triggered()),this, SLOT(Slot1()));
    connect(m_pAction2, SIGNAL(triggered()),this, SLOT(Slot2()));
    connect(m_pAction3, SIGNAL(triggered()),this, SLOT(Slot3()));
    connect(m_pAction4, SIGNAL(triggered()),this, SLOT(Slot4()));
    connect(m_pAction5, SIGNAL(triggered()),this, SLOT(Slot5()));
    connect(m_pAction6, SIGNAL(triggered()),this, SLOT(Slot6()));
    connect(m_pAction7, SIGNAL(triggered()),this, SLOT(Slot7()));
    connect(m_pAction9, &QAction::triggered, this, [this]() {
        useChinese = !useChinese;
        ApplyLanguage();
        if (!lastProfile.isEmpty() && insightText) {
            insightText->setHtml(FormatInsightHtml(lastProfile));
        }
    });
    connect(m_pAction8, &QAction::triggered, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(
            this,
            Text(QStringLiteral("Choose data file"), QStringLiteral("选择数据文件")),
            QString(),
            Text(QStringLiteral("Data file (*.xls *.xlsx *.csv *.txt)"), QStringLiteral("数据文件 (*.xls *.xlsx *.csv *.txt)"))
        );
        if (!filePath.isEmpty()) {
            AnalyzeFileWithService(filePath);
        }
    });
}

void MainWindow::createStatusBar(){// 创建状态栏并添加信息标签

    info_Label  = new QLabel; // 创建标签对象
    info_Label->setObjectName(tr("StatusLabel")); // 设置标签对象名称
    info_Label->setText(tr("")); // 设置标签初始文本为空
    ui->statusBar->addWidget(info_Label); // 将标签添加到状态栏
}

void MainWindow::createOverviewPanel()
{
    QLayout *rootLayout = ui->centralWidget->layout();
    if (!rootLayout || rootLayout->count() == 0) {
        return;
    }

    QLayoutItem *contentItem = rootLayout->takeAt(0);
    QVBoxLayout *shell = new QVBoxLayout;
    shell->setContentsMargins(14, 14, 14, 14);
    shell->setSpacing(12);

    QFrame *hero = new QFrame(ui->centralWidget);
    hero->setObjectName(QStringLiteral("heroPanel"));
    QHBoxLayout *heroLayout = new QHBoxLayout(hero);
    heroLayout->setContentsMargins(18, 16, 18, 16);
    heroLayout->setSpacing(14);

    QVBoxLayout *titleBlock = new QVBoxLayout;
    eyebrowLabel = new QLabel(QStringLiteral("PRIVATE AI DATA WORKBENCH"), hero);
    eyebrowLabel->setObjectName(QStringLiteral("eyebrowLabel"));
    titleLabel = new QLabel(QStringLiteral("TablePilot"), hero);
    titleLabel->setObjectName(QStringLiteral("heroTitle"));
    subtitleLabel = new QLabel(QStringLiteral("A local, explainable workbench for messy spreadsheets and table-like files."), hero);
    subtitleLabel->setObjectName(QStringLiteral("heroSubtitle"));
    titleBlock->addWidget(eyebrowLabel);
    titleBlock->addWidget(titleLabel);
    titleBlock->addWidget(subtitleLabel);
    heroLayout->addLayout(titleBlock, 3);

    auto makeCard = [hero](const QString &objectName, const QString &text) {
        QLabel *card = new QLabel(text, hero);
        card->setObjectName(objectName);
        card->setMinimumWidth(132);
        card->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        card->setTextFormat(Qt::RichText);
        return card;
    };

    serviceBadge = makeCard(QStringLiteral("statusCard"), QStringLiteral("<b>Service</b><br><span>checking</span>"));
    datasetCard = makeCard(QStringLiteral("metricCard"), QStringLiteral("<b>Dataset</b><br><span>No file</span>"));
    qualityCard = makeCard(QStringLiteral("metricCard"), QStringLiteral("<b>Quality</b><br><span>-</span>"));
    schemaCard = makeCard(QStringLiteral("metricCard"), QStringLiteral("<b>Schema</b><br><span>-</span>"));
    recommendationCard = makeCard(QStringLiteral("metricCardWide"), QStringLiteral("<b>Next best analysis</b><br><span>Open a table to start</span>"));

    heroLayout->addWidget(serviceBadge);
    heroLayout->addWidget(datasetCard);
    heroLayout->addWidget(qualityCard);
    heroLayout->addWidget(schemaCard);
    heroLayout->addWidget(recommendationCard, 2);

    shell->addWidget(hero);
    shell->addItem(contentItem);
    rootLayout->addItem(shell);
}

void MainWindow::createInsightPanel()
{
    insightDock = new QDockWidget(QStringLiteral("Analysis Panel"), this);
    insightDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    insightDock->setMinimumWidth(410);

    QWidget *panel = new QWidget(insightDock);
    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(0, 0, 0, 0);
    panelLayout->setSpacing(10);

    insightText = new QTextEdit(insightDock);
    insightText->setReadOnly(true);
    insightText->setPlaceholderText(QStringLiteral("选择数据文件后，这里会显示智能分析摘要、数据质量和复核线索。"));
    insightText->setObjectName(QStringLiteral("insightText"));
    panelLayout->addWidget(insightText, 1);

    QFrame *actionsPanel = new QFrame(panel);
    actionsPanel->setObjectName(QStringLiteral("suggestionPanel"));
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsPanel);
    actionsLayout->setContentsMargins(10, 8, 10, 8);
    actionsLayout->setSpacing(8);

    suggestTrendButton = new QPushButton(actionsPanel);
    suggestTrendButton->setObjectName(QStringLiteral("suggestionButton"));
    suggestDistributionButton = new QPushButton(actionsPanel);
    suggestDistributionButton->setObjectName(QStringLiteral("suggestionButton"));
    suggestQualityButton = new QPushButton(actionsPanel);
    suggestQualityButton->setObjectName(QStringLiteral("suggestionButton"));
    exportReportButton = new QPushButton(actionsPanel);
    exportReportButton->setObjectName(QStringLiteral("suggestionButton"));
    actionsLayout->addWidget(suggestTrendButton);
    actionsLayout->addWidget(suggestDistributionButton);
    actionsLayout->addWidget(suggestQualityButton);
    actionsLayout->addWidget(exportReportButton);
    panelLayout->addWidget(actionsPanel);

    connect(suggestTrendButton, &QPushButton::clicked, this, [this]() {
        Slot4();
        RenderChartStudio();
        statusBar()->showMessage(Text(QStringLiteral("Chart Studio opened"), QStringLiteral("已打开图表工作台")), 3000);
    });
    connect(suggestDistributionButton, &QPushButton::clicked, this, [this]() {
        Slot5();
        RenderDynamicBarChart();
        statusBar()->showMessage(Text(QStringLiteral("Distribution view opened"), QStringLiteral("已切换到分布视图")), 3000);
    });
    connect(suggestQualityButton, &QPushButton::clicked, this, [this]() {
        if (reviewDock) {
            reviewDock->show();
            reviewDock->raise();
        }
        FocusDataQuality();
    });
    connect(exportReportButton, &QPushButton::clicked, this, [this]() {
        if (lastProfile.isEmpty()) {
            return;
        }
        QString filename = QFileDialog::getSaveFileName(
            this,
            Text(QStringLiteral("Export session report"), QStringLiteral("导出会话报告")),
            QStringLiteral("tablepilot-report.html"),
            Text(QStringLiteral("HTML report (*.html);;Markdown report (*.md)"), QStringLiteral("HTML 报告 (*.html);;Markdown 报告 (*.md)"))
        );
        if (filename.isEmpty()) {
            return;
        }
        QString content = insightText ? (filename.endsWith(QStringLiteral(".md"), Qt::CaseInsensitive) ? insightText->toPlainText() : insightText->toHtml()) : QString();
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(content.toUtf8());
            file.close();
            statusBar()->showMessage(Text(QStringLiteral("Report exported"), QStringLiteral("报告已导出")), 4000);
        }
    });

    insightDock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, insightDock);

    reviewDock = new QDockWidget(QStringLiteral("Review drawer"), this);
    reviewDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    reviewDock->setMinimumWidth(430);
    QWidget *reviewPanel = new QWidget(reviewDock);
    QVBoxLayout *reviewLayout = new QVBoxLayout(reviewPanel);
    reviewLayout->setContentsMargins(10, 10, 10, 10);
    reviewLayout->setSpacing(10);

    QLabel *anomalyTitle = new QLabel(QStringLiteral("Anomaly review"), reviewPanel);
    anomalyTitle->setObjectName(QStringLiteral("drawerTitle"));
    reviewLayout->addWidget(anomalyTitle);
    anomalyTable = new QTableWidget(reviewPanel);
    anomalyTable->setObjectName(QStringLiteral("drawerTable"));
    anomalyTable->setColumnCount(5);
    anomalyTable->setHorizontalHeaderLabels(QStringList() << QStringLiteral("Row") << QStringLiteral("Field") << QStringLiteral("Value") << QStringLiteral("z") << QStringLiteral("Action"));
    anomalyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    anomalyTable->verticalHeader()->setVisible(false);
    anomalyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    anomalyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    reviewLayout->addWidget(anomalyTable, 2);

    QLabel *compareTitle = new QLabel(QStringLiteral("Clean preview"), reviewPanel);
    compareTitle->setObjectName(QStringLiteral("drawerTitle"));
    reviewLayout->addWidget(compareTitle);
    cleanCompareText = new QTextEdit(reviewPanel);
    cleanCompareText->setObjectName(QStringLiteral("insightText"));
    cleanCompareText->setReadOnly(true);
    cleanCompareText->setPlaceholderText(QStringLiteral("Use Compare after opening a file to review before/after clean-up."));
    reviewLayout->addWidget(cleanCompareText, 3);

    connect(anomalyTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        FocusAnomaly(row);
    });
    reviewDock->setWidget(reviewPanel);
    addDockWidget(Qt::RightDockWidgetArea, reviewDock);
    tabifyDockWidget(insightDock, reviewDock);
    reviewDock->hide();
}

void MainWindow::createChartHeaders()
{
    QVBoxLayout *trendLayout = findChild<QVBoxLayout*>(QStringLiteral("verticalLayout_2"));
    if (trendLayout && !trendChartTitleLabel) {
        QFrame *header = new QFrame(ui->centralWidget);
        header->setObjectName(QStringLiteral("chartHeader"));
        QHBoxLayout *layout = new QHBoxLayout(header);
        layout->setContentsMargins(12, 8, 12, 8);
        layout->setSpacing(10);
        QVBoxLayout *copy = new QVBoxLayout;
        copy->setContentsMargins(0, 0, 0, 0);
        copy->setSpacing(2);
        trendChartTitleLabel = new QLabel(header);
        trendChartTitleLabel->setObjectName(QStringLiteral("chartTitle"));
        trendChartSubtitleLabel = new QLabel(header);
        trendChartSubtitleLabel->setObjectName(QStringLiteral("chartSubtitle"));
        copy->addWidget(trendChartTitleLabel);
        copy->addWidget(trendChartSubtitleLabel);
        layout->addLayout(copy, 1);
        sheetSelectorLabel = new QLabel(header);
        sheetSelectorLabel->setObjectName(QStringLiteral("fieldLabel"));
        sheetSelector = new QComboBox(header);
        sheetSelector->setObjectName(QStringLiteral("sheetSelector"));
        sheetSelector->setMinimumWidth(140);
        sheetSelector->setVisible(false);
        sheetSelectorLabel->setVisible(false);
        chartTypeSelector = new QComboBox(header);
        chartTypeSelector->setObjectName(QStringLiteral("chartSelector"));
        chartTypeSelector->setMinimumWidth(130);
        chartMetricSelector = new QComboBox(header);
        chartMetricSelector->setObjectName(QStringLiteral("chartSelector"));
        chartMetricSelector->setMinimumWidth(150);
        chartDimensionSelector = new QComboBox(header);
        chartDimensionSelector->setObjectName(QStringLiteral("chartSelector"));
        chartDimensionSelector->setMinimumWidth(150);
        layout->addWidget(chartTypeSelector);
        layout->addWidget(chartMetricSelector);
        layout->addWidget(chartDimensionSelector);
        layout->addWidget(sheetSelectorLabel);
        layout->addWidget(sheetSelector);
        layout->addWidget(ui->pushButton);
        trendLayout->insertWidget(0, header);
        connect(chartTypeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
            RenderChartStudio();
        });
        connect(chartMetricSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
            RenderChartStudio();
        });
        connect(chartDimensionSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
            RenderChartStudio();
        });
        connect(sheetSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
            if (updatingSheetSelector || currentFilePath.isEmpty() || !sheetSelector || sheetSelector->currentIndex() < 0) {
                return;
            }
            AnalyzeFileWithService(currentFilePath, sheetSelector->currentText());
        });
    }

    QHBoxLayout *distributionControls = findChild<QHBoxLayout*>(QStringLiteral("horizontalLayout"));
    if (distributionControls && !distributionChartTitleLabel) {
        QVBoxLayout *copy = new QVBoxLayout;
        copy->setContentsMargins(0, 0, 8, 0);
        copy->setSpacing(2);
        distributionChartTitleLabel = new QLabel(ui->centralWidget);
        distributionChartTitleLabel->setObjectName(QStringLiteral("chartTitle"));
        distributionChartSubtitleLabel = new QLabel(ui->centralWidget);
        distributionChartSubtitleLabel->setObjectName(QStringLiteral("chartSubtitle"));
        copy->addWidget(distributionChartTitleLabel);
        copy->addWidget(distributionChartSubtitleLabel);
        distributionControls->insertLayout(0, copy, 1);
        metricSelectorLabel = new QLabel(ui->centralWidget);
        metricSelectorLabel->setObjectName(QStringLiteral("fieldLabel"));
        distributionControls->insertWidget(2, metricSelectorLabel);
    }
    if (ui->pushButton_3 && ui->comboBox_2) {
        ui->pushButton_3->setText(QStringLiteral("Refresh relationship"));
        ui->comboBox_2->setToolTip(Text(QStringLiteral("Choose a relationship view"), QStringLiteral("选择关系图")));
    }
}

QString MainWindow::Text(const QString &en, const QString &zh) const
{
    return useChinese ? zh : en;
}

void MainWindow::ApplyLanguage()
{
    setWindowTitle(QStringLiteral("TablePilot"));
    if (useChinese) {
        m_pAction1->setText(QStringLiteral("打开 Excel"));
        m_pAction1->setToolTip(QStringLiteral("打开 Excel 工作簿并调用本地分析服务"));
        m_pAction2->setText(QStringLiteral("打开文本"));
        m_pAction2->setToolTip(QStringLiteral("打开 TXT/CSV 表格并自动识别分隔符"));
        m_pAction3->setText(QStringLiteral("数据画像"));
        m_pAction3->setToolTip(QStringLiteral("刷新数值字段统计画像"));
        m_pAction4->setText(QStringLiteral("图表"));
        m_pAction4->setToolTip(QStringLiteral("打开 Chart Studio 推荐图表"));
        m_pAction5->setText(QStringLiteral("分布"));
        m_pAction5->setToolTip(QStringLiteral("查看所选数值字段分布"));
        m_pAction6->setText(QStringLiteral("导出"));
        m_pAction6->setToolTip(QStringLiteral("导出当前图表为图片或 PDF"));
        m_pAction7->setText(QStringLiteral("退出"));
        m_pAction7->setToolTip(QStringLiteral("退出程序"));
        m_pAction8->setText(QStringLiteral("分析"));
        m_pAction8->setToolTip(QStringLiteral("打开任意 Excel、CSV 或 TXT 表格进行智能画像"));
        m_pAction9->setText(QStringLiteral("EN"));
        m_pAction9->setToolTip(QStringLiteral("Switch language"));
        ui->groupBox->setTitle(QStringLiteral("数据预览"));
        ui->groupBox_2->setTitle(QStringLiteral("数值画像"));
        ui->pushButton->setText(QStringLiteral("刷新趋势"));
        ui->pushButton_2->setText(QStringLiteral("刷新分布"));
        ui->comboBox->setToolTip(QStringLiteral("选择分布图使用的数值字段"));
        if (trendChartTitleLabel) trendChartTitleLabel->setText(QStringLiteral("趋势视图"));
        if (trendChartSubtitleLabel) trendChartSubtitleLabel->setText(QStringLiteral("按记录顺序展示最重要的数值指标变化。"));
        if (distributionChartTitleLabel) distributionChartTitleLabel->setText(QStringLiteral("分布视图"));
        if (distributionChartSubtitleLabel) distributionChartSubtitleLabel->setText(QStringLiteral("查看单个指标在各条记录中的分布。"));
        if (metricSelectorLabel) metricSelectorLabel->setText(QStringLiteral("指标"));
        if (sheetSelectorLabel) sheetSelectorLabel->setText(QStringLiteral("工作表"));
        if (suggestTrendButton) suggestTrendButton->setText(QStringLiteral("查看趋势"));
        if (suggestDistributionButton) suggestDistributionButton->setText(QStringLiteral("查看分布"));
        if (suggestQualityButton) suggestQualityButton->setText(QStringLiteral("复核质量"));
        if (exportReportButton) exportReportButton->setText(QStringLiteral("导出报告"));
        if (eyebrowLabel) eyebrowLabel->setText(QStringLiteral("本地 AI 数据工作台"));
        if (titleLabel) titleLabel->setText(QStringLiteral("TablePilot"));
        if (subtitleLabel) subtitleLabel->setText(QStringLiteral("面向复杂表格、销售数据和 TXT/CSV 文件的本地可解释分析工作台。"));
        if (insightDock) insightDock->setWindowTitle(QStringLiteral("洞察面板"));
        if (reviewDock) reviewDock->setWindowTitle(QStringLiteral("复核抽屉"));
        if (insightText) insightText->setPlaceholderText(QStringLiteral("选择数据文件后，这里会显示分析摘要、数据质量、字段结构和下一步建议。"));
    } else {
        m_pAction1->setText(QStringLiteral("Open Excel"));
        m_pAction1->setToolTip(QStringLiteral("Open an Excel workbook and send it to the local analysis service"));
        m_pAction2->setText(QStringLiteral("Open Text"));
        m_pAction2->setToolTip(QStringLiteral("Open TXT/CSV tables with automatic delimiter detection"));
        m_pAction3->setText(QStringLiteral("Profile"));
        m_pAction3->setToolTip(QStringLiteral("Refresh numeric field profiling"));
        m_pAction4->setText(QStringLiteral("Charts"));
        m_pAction4->setToolTip(QStringLiteral("Open the recommended Chart Studio view"));
        m_pAction5->setText(QStringLiteral("Distribution"));
        m_pAction5->setToolTip(QStringLiteral("Show distribution for the selected numeric field"));
        m_pAction6->setText(QStringLiteral("Export"));
        m_pAction6->setToolTip(QStringLiteral("Export the current chart as an image or PDF"));
        m_pAction7->setText(QStringLiteral("Quit"));
        m_pAction7->setToolTip(QStringLiteral("Quit TablePilot"));
        m_pAction8->setText(QStringLiteral("Analyze"));
        m_pAction8->setToolTip(QStringLiteral("Open any Excel, CSV, or TXT table for profiling"));
        m_pAction9->setText(QStringLiteral("中文"));
        m_pAction9->setToolTip(QStringLiteral("切换语言"));
        ui->groupBox->setTitle(QStringLiteral("Data Preview"));
        ui->groupBox_2->setTitle(QStringLiteral("Numeric Profile"));
        ui->pushButton->setText(QStringLiteral("Refresh trend"));
        ui->pushButton_2->setText(QStringLiteral("Refresh distribution"));
        ui->comboBox->setToolTip(QStringLiteral("Choose the numeric field used by the distribution chart"));
        if (trendChartTitleLabel) trendChartTitleLabel->setText(QStringLiteral("Trend View"));
        if (trendChartSubtitleLabel) trendChartSubtitleLabel->setText(QStringLiteral("Track the most useful numeric measures by record order."));
        if (distributionChartTitleLabel) distributionChartTitleLabel->setText(QStringLiteral("Distribution View"));
        if (distributionChartSubtitleLabel) distributionChartSubtitleLabel->setText(QStringLiteral("Inspect one selected metric across records."));
        if (metricSelectorLabel) metricSelectorLabel->setText(QStringLiteral("Metric"));
        if (sheetSelectorLabel) sheetSelectorLabel->setText(QStringLiteral("Sheet"));
        if (suggestTrendButton) suggestTrendButton->setText(QStringLiteral("Open trend"));
        if (suggestDistributionButton) suggestDistributionButton->setText(QStringLiteral("Open distribution"));
        if (suggestQualityButton) suggestQualityButton->setText(QStringLiteral("Review quality"));
        if (exportReportButton) exportReportButton->setText(QStringLiteral("Export report"));
        if (eyebrowLabel) eyebrowLabel->setText(QStringLiteral("PRIVATE AI DATA WORKBENCH"));
        if (titleLabel) titleLabel->setText(QStringLiteral("TablePilot"));
        if (subtitleLabel) subtitleLabel->setText(QStringLiteral("A local, explainable workbench for messy spreadsheets and table-like files."));
        if (insightDock) insightDock->setWindowTitle(QStringLiteral("Analysis Panel"));
        if (reviewDock) reviewDock->setWindowTitle(QStringLiteral("Review Drawer"));
        if (insightText) insightText->setPlaceholderText(QStringLiteral("Open a data file to see the analysis brief, data quality, schema, and next moves."));
    }
    QMap<QString, QString> commandText;
    commandText.insert(QStringLiteral("open_excel"), m_pAction1->text());
    commandText.insert(QStringLiteral("open_text"), m_pAction2->text());
    commandText.insert(QStringLiteral("analyze"), m_pAction8->text());
    commandText.insert(QStringLiteral("chart_studio"), Text(QStringLiteral("Charts"), QStringLiteral("图表")));
    commandText.insert(QStringLiteral("profile"), m_pAction3->text());
    commandText.insert(QStringLiteral("recent_files"), Text(QStringLiteral("Recent"), QStringLiteral("最近文件")));
    commandText.insert(QStringLiteral("clean_export"), Text(QStringLiteral("Clean export"), QStringLiteral("清洗导出")));
    commandText.insert(QStringLiteral("clean_compare"), Text(QStringLiteral("Compare"), QStringLiteral("清洗对比")));
    commandText.insert(QStringLiteral("ai_toggle"), localAiRequested ? Text(QStringLiteral("Local AI on"), QStringLiteral("本地模型开")) : Text(QStringLiteral("Rules only"), QStringLiteral("规则分析")));
    commandText.insert(QStringLiteral("regenerate_ai"), Text(QStringLiteral("Regenerate"), QStringLiteral("重生成")));
    commandText.insert(QStringLiteral("language"), useChinese ? QStringLiteral("EN") : QStringLiteral("中文"));
    commandText.insert(QStringLiteral("quit"), m_pAction7->text());
    for (QPushButton *button : ui->mainToolBar->findChildren<QPushButton*>()) {
        QString key = button->property("actionKey").toString();
        if (commandText.contains(key)) {
            button->setText(commandText.value(key));
        }
    }
    if (chartTypeSelector) {
        int current = chartTypeSelector->currentIndex();
        chartTypeSelector->blockSignals(true);
        chartTypeSelector->clear();
        chartTypeSelector->addItem(Text(QStringLiteral("Auto chart"), QStringLiteral("自动图表")), QStringLiteral("auto"));
        chartTypeSelector->addItem(Text(QStringLiteral("Trend"), QStringLiteral("趋势")), QStringLiteral("trend"));
        chartTypeSelector->addItem(Text(QStringLiteral("Grouped bar"), QStringLiteral("分组柱状")), QStringLiteral("grouped_bar"));
        chartTypeSelector->addItem(Text(QStringLiteral("Scatter"), QStringLiteral("散点")), QStringLiteral("scatter"));
        chartTypeSelector->addItem(Text(QStringLiteral("Heatmap"), QStringLiteral("相关热力")), QStringLiteral("heatmap"));
        chartTypeSelector->addItem(Text(QStringLiteral("Box plot"), QStringLiteral("箱线图")), QStringLiteral("box"));
        chartTypeSelector->setCurrentIndex(std::max(0, current));
        chartTypeSelector->blockSignals(false);
    }
    if (chartMetricSelector) {
        chartMetricSelector->setToolTip(Text(QStringLiteral("Choose the measure to visualize"), QStringLiteral("选择要分析的指标")));
    }
    if (chartDimensionSelector) {
        chartDimensionSelector->setToolTip(Text(QStringLiteral("Choose a group, date, or second measure"), QStringLiteral("选择分组、日期或第二个指标")));
    }
    UpdateToolbarState(lastProfile);
    RefreshRecentFilesMenu();
    if (lastProfile.isEmpty()) {
        ui->comboBox->blockSignals(true);
        ui->comboBox->clear();
        ui->comboBox->addItem(Text(QStringLiteral("Recommended metric"), QStringLiteral("推荐指标")), -1);
        ui->comboBox->blockSignals(false);
        UpdateDefaultOverviewCards(QStringLiteral("idle"));
    } else {
        UpdateFieldSelectors(lastProfile);
        PopulateStatsFromService(lastProfile);
        UpdateOverviewCards(lastProfile);
        UpdateRecommendationActions(lastProfile);
        RenderChartStudio();
        RenderDynamicBarChart();
    }
}

void MainWindow::CheckAnalysisService()
{
    if (!IsAnalysisServiceHealthy()) {
        statusBar()->showMessage(Text(QStringLiteral("Starting local analysis service..."), QStringLiteral("正在启动本地分析服务...")));
        UpdateDefaultOverviewCards(QStringLiteral("starting"));
        TryStartAnalysisService();
    }

    if (IsAnalysisServiceHealthy(6000)) {
        statusBar()->showMessage(Text(QStringLiteral("Analysis service connected"), QStringLiteral("本地分析服务已连接")));
        UpdateDefaultOverviewCards(QStringLiteral("connected"));
        UpdateToolbarState(lastProfile);
        if (insightText) {
            if (useChinese) {
                insightText->setHtml(QStringLiteral(
                    "<h2>TablePilot</h2>"
                    "<p class='muted'>本地分析服务已连接。</p>"
                    "<div class='callout'>打开 Excel、CSV 或 TXT 表格后，系统会自动识别字段、评分数据质量、规划下一步分析并生成动态图表。</div>"
                ));
            } else {
                insightText->setHtml(QStringLiteral(
                    "<h2>TablePilot</h2>"
                    "<p class='muted'>Analysis service is connected.</p>"
                    "<div class='callout'>Open an Excel, CSV, or TXT table. "
                    "TablePilot will infer schema, score data quality, plan the next analysis, and render dynamic charts.</div>"
                ));
            }
        }
        return;
    }

    statusBar()->showMessage(Text(QStringLiteral("Analysis service offline"), QStringLiteral("本地分析服务离线")));
    UpdateDefaultOverviewCards(QStringLiteral("offline"));
    if (toolbarServiceLabel) {
        toolbarServiceLabel->setText(Text(QStringLiteral("Service offline"), QStringLiteral("服务离线")));
    }
    if (insightText) {
        insightText->setHtml(useChinese
            ? QStringLiteral("<h2>TablePilot</h2><p class='muted'>本地分析服务未连接。</p><div class='callout warn'>程序已尝试自动启动 Docker Compose。如果 Docker Desktop 未运行，请先启动 Docker Desktop。</div><p>手动方式：在项目根目录运行 <code>docker compose up --build</code>。</p>")
            : QStringLiteral("<h2>TablePilot</h2><p class='muted'>Analysis service is offline.</p><div class='callout warn'>The app tried to start Docker Compose automatically. If Docker Desktop is not running, start it and reopen a file.</div><p>Manual fallback: run <code>docker compose up --build</code> from the project root.</p>")
        );
    }
}

bool MainWindow::IsAnalysisServiceHealthy(int timeoutMs)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl("http://127.0.0.1:8000/health"));
    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
    } else if (reply->isRunning()) {
        reply->abort();
    }
    const bool ok = reply->error() == QNetworkReply::NoError;
    reply->deleteLater();
    return ok;
}

bool MainWindow::TryStartAnalysisService()
{
    QString projectRoot = ProjectPath("docker-compose.yml");
    if (projectRoot.endsWith("docker-compose.yml")) {
        projectRoot = QFileInfo(projectRoot).absolutePath();
    }

    QString program = QStringLiteral("docker");
    QStringList args = {QStringLiteral("compose"), QStringLiteral("up"), QStringLiteral("-d")};
    const bool started = QProcess::startDetached(program, args, projectRoot);
    if (!started && insightText) {
        insightText->setHtml(useChinese
            ? QStringLiteral("<h2>服务启动失败</h2><div class='callout warn'>TablePilot 无法自动启动 Docker Compose。请先启动 Docker Desktop，然后运行 <code>docker compose up --build</code>。</div>")
            : QStringLiteral("<h2>Service Start Failed</h2><div class='callout warn'>TablePilot could not start Docker Compose automatically. Start Docker Desktop, then run <code>docker compose up --build</code>.</div>")
        );
    }
    return started;
}

void MainWindow::Slot1(){//打开Excel
    isExit = true;  // 标记 isExit 为 true，表示退出状态（这个变量在代码中未定义，可能是 MainWindow 类的成员变量）

    QString filePath = QFileDialog::getOpenFileName(
        this,
        Text(QStringLiteral("Choose Excel file"), QStringLiteral("选择 Excel 文件")),
        QString(),
        Text(QStringLiteral("Excel file (*.xls *.xlsx)"), QStringLiteral("Excel 文件 (*.xls *.xlsx)"))
    );
    if(filePath.isEmpty())  // 如果文件路径为空，说明用户取消了选择，直接返回
        return;

    info_Label->clear();
    info_Label->setText(filePath);
    AnalyzeFileWithService(filePath);
}

void MainWindow::Slot2(){//打开TXT
    QString filePath = QFileDialog::getOpenFileName(
        this,
        Text(QStringLiteral("Choose TXT/CSV file"), QStringLiteral("选择 TXT/CSV 文件")),
        QString(),
        Text(QStringLiteral("Table text file (*.txt *.csv)"), QStringLiteral("表格文本文件 (*.txt *.csv)"))
    );
    if(filePath.isEmpty())  // 如果文件路径为空，说明用户取消了选择，直接返回
        return;

    isExit = true;
    info_Label->clear();
    info_Label->setText(filePath);
    AnalyzeFileWithService(filePath);
}

void MainWindow::Slot3(){//数据统计
    PopulateStatsFromService(QJsonObject());
}

//根据用户的操作，切换显示不同类型的图表，提供不同的数据可视化展示
void MainWindow::Slot4(){//折线图
    // 如果不是退出状态，则直接返回，不执行后续操作
    if (!isExit) {
        return;
    }

    // 设置保存类型为 0，表示当前显示的是折线图
    SaveType = 0;

    // 设置 stackedWidget 的当前页面为索引为 0 的页面，即显示折线图页面
    ui->stackedWidget->setCurrentIndex(0);
    RenderChartStudio();
}

void MainWindow::Slot5(){//柱状图
    // 检查是否处于退出状态，如果不是，则直接返回
    if(!isExit){
        return;
    }

    // 将保存类型设为1，表示柱状图
    SaveType = 1;

    // 切换显示到索引为1的页面，即柱状图页面
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::Slot6(){//保存图片功能

    // 获取用户选择的文件路径，用于保存图像文件
    QString filename = QFileDialog::getSaveFileName();

    // 根据保存类型选择对应的绘图部件进行重新绘制和保存图像
    if(SaveType == 0){ // 如果保存类型为0，表示当前是折线图
        // 重新绘制折线图部件
        ui->widget->replot();
        // 调用保存图像的函数，将折线图部件保存为图像文件
        SavePic(filename, ui->widget);
    }
    else if(SaveType == 1){ // 如果保存类型为1，表示当前是柱状图
        // 重新绘制柱状图部件
        ui->widget_2->replot();
        // 调用保存图像的函数，将柱状图部件保存为图像文件
        SavePic(filename, ui->widget_2);
    }
    else{ // 如果保存类型不是0或1，即其他类型，直接保存当前活动的部件为图像文件
        // 调用保存图像的函数，将当前活动的部件保存为图像文件
        SavePic(filename, ui->widget);
    }
}

void MainWindow::Slot7(){//退出程序
    // 关闭当前主窗口
    close();
}

bool MainWindow::SavePic(QString fileName, QCustomPlot *p_save){//保存图片
    // 检查指针是否为空或文件名是否为空字符串，如果是，则显示保存失败消息框并返回false
    if (p_save == nullptr || fileName == ""){
        QMessageBox::information(this, Text(QStringLiteral("Failed"), QStringLiteral("失败")), Text(QStringLiteral("Export failed."), QStringLiteral("保存失败。")));
        return false;
    }

    // 如果文件名以 ".png" 结尾
    if (fileName.endsWith(".png")){
        // 显示成功保存为png文件的消息框
        QMessageBox::information(this, Text(QStringLiteral("Success"), QStringLiteral("成功")), Text(QStringLiteral("Saved as PNG."), QStringLiteral("已保存为 PNG 文件。")));
        // 调用QCustomPlot对象的savePng()方法，保存为png文件，返回保存结果
        return p_save->savePng(fileName, p_save->width(), p_save->height());
    }
    // 如果文件名以 ".jpg" 或 ".jpeg" 结尾
    else if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg")){
        // 显示成功保存为jpg文件的消息框
        QMessageBox::information(this, Text(QStringLiteral("Success"), QStringLiteral("成功")), Text(QStringLiteral("Saved as JPG."), QStringLiteral("已保存为 JPG 文件。")));
        // 调用QCustomPlot对象的saveJpg()方法，保存为jpg文件，返回保存结果
        return p_save->saveJpg(fileName, p_save->width(), p_save->height());
    }
    // 如果文件名以 ".bmp" 结尾
    else if (fileName.endsWith(".bmp")){
        // 显示成功保存为bmp文件的消息框
        QMessageBox::information(this, Text(QStringLiteral("Success"), QStringLiteral("成功")), Text(QStringLiteral("Saved as BMP."), QStringLiteral("已保存为 BMP 文件。")));
        // 调用QCustomPlot对象的saveBmp()方法，保存为bmp文件，返回保存结果
        return p_save->saveBmp(fileName, p_save->width(), p_save->height());
    }
    // 如果文件名以 ".pdf" 结尾
    else if (fileName.endsWith(".pdf")){
        // 显示成功保存为pdf文件的消息框
        QMessageBox::information(this, Text(QStringLiteral("Success"), QStringLiteral("成功")), Text(QStringLiteral("Saved as PDF."), QStringLiteral("已保存为 PDF 文件。")));
        // 调用QCustomPlot对象的savePdf()方法，保存为pdf文件，返回保存结果
        return p_save->savePdf(fileName, p_save->width(), p_save->height());
    }
    // 如果文件名不符合以上格式
    else{
        // 显示默认保存为png文件的消息框
        QMessageBox::information(this, Text(QStringLiteral("Success"), QStringLiteral("成功")), Text(QStringLiteral("Saved as PNG by default."), QStringLiteral("已按默认 PNG 格式保存。")));
        // 使用".png"作为文件扩展名，调用QCustomPlot对象的savePng()方法，保存为png文件，返回保存结果
        return p_save->savePng(fileName.arg(".png"), p_save->width(), p_save->height());
    }
}

void MainWindow::on_pushButton_2_clicked(){//柱状图
    RenderDynamicBarChart();
}

void MainWindow::on_pushButton_3_clicked(){
    if(!isExit)
    {
        return;
    }
}

void MainWindow::on_pushButton_clicked() {
    RenderChartStudio();
}

//
void MainWindow::createStyle() {
    SetStyleSheet(this, ProjectPath("qss/blue1.qss"));
    ui->groupBox->setTitle(QStringLiteral("Data Preview"));
    ui->groupBox_2->setTitle(QStringLiteral("Numeric Profile"));
    ui->stackedWidget->setCurrentIndex(0);
    ui->pushButton->setText(QStringLiteral("Refresh trend"));
    ui->pushButton_2->setText(QStringLiteral("Refresh distribution"));
    ui->comboBox->clear();
    ui->comboBox->addItem(Text(QStringLiteral("Recommended metric"), QStringLiteral("推荐指标")), -1);
    ui->comboBox->setToolTip(Text(QStringLiteral("Choose the numeric field used by the distribution chart"), QStringLiteral("选择分布图使用的数值字段")));
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        if (ui->stackedWidget->currentIndex() == 1) {
            RenderDynamicBarChart();
        }
    });
    ui->comboBox_2->hide();
    ui->pushButton_3->hide();
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget_2->setAlternatingRowColors(true);
    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget_2->setSortingEnabled(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectColumns);
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);
    ui->tableWidget_2->clear();
    ui->tableWidget_2->setRowCount(0);
    ui->tableWidget_2->setColumnCount(0);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget_2->verticalHeader()->setDefaultSectionSize(34);
    ui->tableWidget->horizontalHeader()->setMinimumSectionSize(92);
    ui->tableWidget_2->horizontalHeader()->setMinimumSectionSize(92);
    StylePlot(ui->widget);
    StylePlot(ui->widget_2);
    StylePlot(ui->widget_3);
    RenderEmptyChart(ui->widget, Text(QStringLiteral("Open a data file to generate recommended charts."), QStringLiteral("打开数据文件后生成推荐图表。")));
    RenderEmptyChart(ui->widget_2, Text(QStringLiteral("Open a data file to review metric distribution."), QStringLiteral("打开数据文件后查看指标分布。")));
}

void MainWindow::SetStyleSheet(QWidget* pWidget, QString strQSS) {
    // 检查部件是否为空
    if (nullptr == pWidget) {
        return;
    }

    // 打开样式表文件
    QFile qss(strQSS);
    if (!qss.open(QFile::ReadOnly)) {
        return;
    }

    // 读取样式表内容并设置给部件
    QByteArray content = qss.readAll();

    // 检查是否成功读取到样式表内容
    if (content.isEmpty()) {
        return;
    }

    pWidget->setStyleSheet(content);

    // 关闭文件
    qss.close();
}

void MainWindow::AnalyzeFileWithService(const QString &filePath, const QString &sheetName)
{
    if (!IsAnalysisServiceHealthy()) {
        TryStartAnalysisService();
        if (!IsAnalysisServiceHealthy(7000)) {
            QMessageBox::warning(
                this,
                QStringLiteral("TablePilot"),
                Text(QStringLiteral("The local analysis service is not ready. Start Docker Desktop, or run docker compose up --build from the project root."),
                     QStringLiteral("本地分析服务暂未就绪。请确认 Docker Desktop 已启动，或在项目根目录运行 docker compose up --build。"))
            );
            return;
        }
    }

    currentFilePath = filePath;
    QFileInfo fileInfo(filePath);
    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("TablePilot"), Text(QStringLiteral("Could not read the selected file."), QStringLiteral("无法读取所选文件。")));
        delete file;
        return;
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileInfo.fileName()))
    );
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    QNetworkAccessManager manager;
    QString url = QStringLiteral("http://127.0.0.1:8000/api/analyze-upload?local_ai=%1").arg(localAiRequested ? QStringLiteral("true") : QStringLiteral("false"));
    if (!sheetName.trimmed().isEmpty()) {
        url += QStringLiteral("&sheet=%1").arg(QString::fromUtf8(QUrl::toPercentEncoding(sheetName)));
    }
    QNetworkRequest request{QUrl(url)};
    QNetworkReply *reply = manager.post(request, multiPart);
    multiPart->setParent(reply);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        QString message = Text(
            QStringLiteral("Could not connect to the local analysis service. Run:\n\ndocker compose up --build\n\nError: %1"),
            QStringLiteral("无法连接本地分析服务。请先运行：\n\ndocker compose up --build\n\n错误：%1")
        ).arg(reply->errorString());
        reply->deleteLater();
        QMessageBox::warning(this, QStringLiteral("TablePilot"), message);
        return;
    }

    QByteArray payload = reply->readAll();
    reply->deleteLater();
    ShowServiceAnalysis(payload);
}

void MainWindow::ShowServiceAnalysis(const QByteArray &payload)
{
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        QMessageBox::warning(this, QStringLiteral("TablePilot"), Text(QStringLiteral("The analysis service returned data that could not be parsed."), QStringLiteral("分析服务返回了无法解析的数据。")));
        return;
    }

    QJsonObject root = document.object();
    lastProfile = root;
    PopulateTableFromService(root);
    ApplyTableQualityDecorations(root);
    UpdateSheetSelector(root);
    UpdateFieldSelectors(root);
    UpdateOverviewCards(root);
    UpdateRecommendationActions(root);
    UpdateToolbarState(root);
    UpdateReviewDrawer(root);

    insightText->setHtml(FormatInsightHtml(root));
    insightDock->show();
    info_Label->setText(Text(QStringLiteral("Analysis completed"), QStringLiteral("智能分析完成")));
    statusBar()->showMessage(Text(QStringLiteral("TablePilot analysis completed"), QStringLiteral("TablePilot 分析完成")), 5000);
    SaveRecentFile(currentFilePath);
}

void MainWindow::PopulateTableFromService(const QJsonObject &root)
{
    QJsonObject preview = root.value("preview").toObject();
    QJsonArray rows = preview.value("rows").toArray();
    QJsonArray columns = preview.value("columns").toArray();
    if (rows.isEmpty()) {
        ui->tableWidget->clear();
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setColumnCount(0);
        RenderEmptyChart(ui->widget, Text(QStringLiteral("The selected table has no rows to visualize."), QStringLiteral("当前表格没有可展示的数据行。")));
        RenderEmptyChart(ui->widget_2, Text(QStringLiteral("The selected table has no rows to visualize."), QStringLiteral("当前表格没有可展示的数据行。")));
        return;
    }

    int rowCount = rows.size();
    int columnCount = rows.first().toArray().size();
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(rowCount);
    ui->tableWidget->setColumnCount(columnCount);

    QStringList headers;
    for (int column = 0; column < columnCount; ++column) {
        headers << (column < columns.size() ? columns.at(column).toString() : QString::number(column + 1));
    }
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    QJsonArray schema = root.value("schema").toArray();
    for (int column = 0; column < columnCount && column < schema.size(); ++column) {
        QJsonObject item = schema.at(column).toObject();
        QTableWidgetItem *headerItem = ui->tableWidget->horizontalHeaderItem(column);
        if (headerItem) {
            headerItem->setToolTip(
                Text(QStringLiteral("%1 | role: %2 | missing: %3"), QStringLiteral("%1 | 角色：%2 | 缺失：%3"))
                    .arg(SemanticTypeText(item.value("semantic_type").toString()))
                    .arg(RoleHintText(item.value("role_hint").toString()))
                    .arg(item.value("missing_count").toInt())
            );
        }
    }

    for (int row = 0; row < rowCount; ++row) {
        QJsonArray cells = rows.at(row).toArray();
        for (int column = 0; column < columnCount; ++column) {
            QString text;
            if (column < cells.size()) {
                QJsonValue value = cells.at(column);
                if (value.isDouble()) {
                    text = QString::number(value.toDouble(), 'g', 12);
                } else {
                    text = value.toVariant().toString();
                }
            }
            ui->tableWidget->setItem(row, column, new QTableWidgetItem(text));
        }
    }

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    PopulateStatsFromService(root);
    RenderChartStudio();
    RenderDynamicBarChart();
}

void MainWindow::ApplyTableQualityDecorations(const QJsonObject &root)
{
    QJsonArray schema = root.value("schema").toArray();
    for (int column = 0; column < ui->tableWidget->columnCount() && column < schema.size(); ++column) {
        QJsonObject field = schema.at(column).toObject();
        QColor headerColor(238, 242, 232);
        const QString type = field.value("semantic_type").toString();
        if (type == "numeric") {
            headerColor = QColor(231, 244, 239);
        } else if (type == "date") {
            headerColor = QColor(232, 240, 250);
        } else if (type == "category") {
            headerColor = QColor(246, 240, 226);
        }
        QTableWidgetItem *header = ui->tableWidget->horizontalHeaderItem(column);
        if (header) {
            header->setBackground(headerColor);
        }

        for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
            QTableWidgetItem *cell = ui->tableWidget->item(row, column);
            if (cell && cell->text().trimmed().isEmpty()) {
                cell->setBackground(QColor(255, 242, 217));
                cell->setToolTip(Text(QStringLiteral("Missing value"), QStringLiteral("缺失值")));
            }
        }
    }

    QJsonArray anomalies = root.value("anomalies").toArray();
    QMap<QString, int> columnIndex;
    for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
        columnIndex.insert(ColumnLabel(column), column);
    }
    for (const QJsonValue &value : anomalies) {
        QJsonObject anomaly = value.toObject();
        int row = anomaly.value("row").toInt(-1);
        int column = columnIndex.value(anomaly.value("column").toString(), -1);
        if (row >= 0 && row < ui->tableWidget->rowCount() && column >= 0) {
            QTableWidgetItem *cell = ui->tableWidget->item(row, column);
            if (cell) {
                cell->setBackground(QColor(255, 229, 229));
                cell->setToolTip(Text(QStringLiteral("Anomaly candidate, z-score %1"), QStringLiteral("异常候选，z-score %1")).arg(anomaly.value("z_score").toDouble()));
            }
        }
    }
}

void MainWindow::UpdateFieldSelectors(const QJsonObject &root)
{
    QJsonArray schema = root.value("schema").toArray();
    ui->comboBox->blockSignals(true);
    ui->comboBox->clear();
    ui->comboBox->addItem(Text(QStringLiteral("Recommended metric"), QStringLiteral("推荐指标")), -1);
    if (chartMetricSelector) {
        chartMetricSelector->blockSignals(true);
        chartMetricSelector->clear();
        chartMetricSelector->addItem(Text(QStringLiteral("Recommended metric"), QStringLiteral("推荐指标")), -1);
    }
    if (chartDimensionSelector) {
        chartDimensionSelector->blockSignals(true);
        chartDimensionSelector->clear();
        chartDimensionSelector->addItem(Text(QStringLiteral("Recommended dimension"), QStringLiteral("推荐维度")), -1);
    }
    for (int column = 0; column < schema.size(); ++column) {
        QJsonObject field = schema.at(column).toObject();
        if (field.value("semantic_type").toString() == "numeric") {
            ui->comboBox->addItem(field.value("name").toString(), column);
            if (chartMetricSelector) {
                chartMetricSelector->addItem(field.value("name").toString(), column);
            }
        }
        if (chartDimensionSelector && field.value("semantic_type").toString() != "empty") {
            chartDimensionSelector->addItem(field.value("name").toString(), column);
        }
    }
    ui->comboBox->blockSignals(false);
    if (chartMetricSelector) {
        chartMetricSelector->blockSignals(false);
    }
    if (chartDimensionSelector) {
        chartDimensionSelector->blockSignals(false);
    }
}

void MainWindow::UpdateToolbarState(const QJsonObject &root)
{
    if (toolbarServiceLabel) {
        toolbarServiceLabel->setText(Text(QStringLiteral("Service ready"), QStringLiteral("服务已连接")));
    }
    QString modelText = localAiRequested
        ? Text(QStringLiteral("Local AI requested"), QStringLiteral("本地模型已请求"))
        : Text(QStringLiteral("Rules only"), QStringLiteral("规则分析"));
    if (!root.isEmpty()) {
        QJsonObject localAi = root.value("local_ai").toObject();
        QString model = localAi.value("model").toString(QStringLiteral("qwen3-4b"));
        QString status = localAi.value("status").toString(localAiRequested ? QStringLiteral("requested") : QStringLiteral("disabled"));
        modelText = QStringLiteral("%1 · %2").arg(model, status);
    }
    if (toolbarModelLabel) {
        toolbarModelLabel->setText(modelText);
    }
    if (cleanExportButton) {
        cleanExportButton->setEnabled(!currentFilePath.isEmpty());
    }
    if (cleanCompareButton) {
        cleanCompareButton->setEnabled(!currentFilePath.isEmpty());
    }
    if (regenerateAiButton) {
        regenerateAiButton->setEnabled(!currentFilePath.isEmpty());
    }
    if (localAiToggleButton) {
        localAiToggleButton->setText(localAiRequested ? Text(QStringLiteral("Local AI on"), QStringLiteral("本地模型开")) : Text(QStringLiteral("Rules only"), QStringLiteral("规则分析")));
    }
}

void MainWindow::UpdateReviewDrawer(const QJsonObject &root)
{
    if (!anomalyTable) {
        return;
    }
    QJsonArray anomalies = root.value("anomalies").toArray();
    anomalyTable->clearSpans();
    anomalyTable->setRowCount(anomalies.size());
    for (int i = 0; i < anomalies.size(); ++i) {
        QJsonObject item = anomalies.at(i).toObject();
        QStringList values;
        values << QString::number(item.value("row").toInt() + 1)
               << item.value("column").toString()
               << QString::number(item.value("value").toDouble(), 'g', 10)
               << QString::number(item.value("z_score").toDouble(), 'f', 2)
               << Text(QStringLiteral("Double-click to locate"), QStringLiteral("双击定位"));
        for (int column = 0; column < values.size(); ++column) {
            QTableWidgetItem *cell = new QTableWidgetItem(values.at(column));
            if (column == 3) {
                cell->setTextAlignment(Qt::AlignCenter);
            }
            anomalyTable->setItem(i, column, cell);
        }
    }
    if (anomalies.isEmpty()) {
        anomalyTable->setRowCount(1);
        anomalyTable->setSpan(0, 0, 1, anomalyTable->columnCount());
        anomalyTable->setItem(0, 0, new QTableWidgetItem(Text(QStringLiteral("No anomaly candidate was detected."), QStringLiteral("当前没有检测到异常候选。"))));
    }
}

void MainWindow::LoadRecentFiles()
{
    QSettings settings(QStringLiteral("TablePilot"), QStringLiteral("TablePilot"));
    recentFiles = settings.value(QStringLiteral("recentFiles")).toStringList();
    QStringList existing;
    for (const QString &path : recentFiles) {
        if (QFileInfo::exists(path) && !existing.contains(path)) {
            existing << path;
        }
    }
    recentFiles = existing.mid(0, 8);
    RefreshRecentFilesMenu();
}

void MainWindow::SaveRecentFile(const QString &filePath)
{
    if (filePath.trimmed().isEmpty() || !QFileInfo::exists(filePath)) {
        return;
    }
    recentFiles.removeAll(filePath);
    recentFiles.prepend(filePath);
    while (recentFiles.size() > 8) {
        recentFiles.removeLast();
    }
    QSettings settings(QStringLiteral("TablePilot"), QStringLiteral("TablePilot"));
    settings.setValue(QStringLiteral("recentFiles"), recentFiles);
    RefreshRecentFilesMenu();
}

void MainWindow::RefreshRecentFilesMenu()
{
    if (!recentFilesButton) {
        return;
    }
    QMenu *menu = recentFilesButton->menu();
    if (!menu) {
        menu = new QMenu(recentFilesButton);
        recentFilesButton->setMenu(menu);
    }
    menu->clear();
    if (recentFiles.isEmpty()) {
        QAction *emptyAction = menu->addAction(Text(QStringLiteral("No recent files"), QStringLiteral("暂无最近文件")));
        emptyAction->setEnabled(false);
        return;
    }
    for (const QString &path : recentFiles) {
        QFileInfo info(path);
        QAction *action = menu->addAction(info.fileName());
        action->setToolTip(path);
        connect(action, &QAction::triggered, this, [this, path]() {
            OpenRecentFile(path);
        });
    }
}

void MainWindow::OpenRecentFile(const QString &filePath)
{
    if (!QFileInfo::exists(filePath)) {
        recentFiles.removeAll(filePath);
        QSettings settings(QStringLiteral("TablePilot"), QStringLiteral("TablePilot"));
        settings.setValue(QStringLiteral("recentFiles"), recentFiles);
        RefreshRecentFilesMenu();
        QMessageBox::information(this, QStringLiteral("TablePilot"), Text(QStringLiteral("This recent file no longer exists."), QStringLiteral("这个最近文件已经不存在。")));
        return;
    }
    isExit = true;
    info_Label->setText(filePath);
    AnalyzeFileWithService(filePath);
}

void MainWindow::UpdateSheetSelector(const QJsonObject &root)
{
    if (!sheetSelector || !sheetSelectorLabel) {
        return;
    }
    QJsonObject source = root.value("source").toObject();
    QJsonArray sheets = source.value("sheets").toArray();
    QString activeSheet = source.value("sheet_name").toString();
    const bool hasMultipleSheets = sheets.size() > 1;
    updatingSheetSelector = true;
    sheetSelector->clear();
    for (const QJsonValue &value : sheets) {
        sheetSelector->addItem(value.toString());
    }
    int activeIndex = sheetSelector->findText(activeSheet);
    if (activeIndex >= 0) {
        sheetSelector->setCurrentIndex(activeIndex);
    }
    sheetSelector->setVisible(hasMultipleSheets);
    sheetSelectorLabel->setVisible(hasMultipleSheets);
    updatingSheetSelector = false;
}

void MainWindow::PopulateStatsFromService(const QJsonObject &root)
{
    QJsonArray columns = root.value("columns").toArray();
    QVector<QJsonObject> numericProfiles;
    for (const QJsonValue &value : columns) {
        QJsonObject item = value.toObject();
        if (item.value("semantic_type").toString() == "numeric") {
            numericProfiles.push_back(item);
        }
    }

    if (numericProfiles.isEmpty()) {
        QList<int> numericColumns = NumericTableColumns(12);
        ui->tableWidget_2->clear();
        ui->tableWidget_2->setRowCount(5);
        ui->tableWidget_2->setColumnCount(numericColumns.size());
        ui->tableWidget_2->setVerticalHeaderLabels(useChinese
            ? (QStringList() << QStringLiteral("数量") << QStringLiteral("均值") << QStringLiteral("最小值") << QStringLiteral("中位数") << QStringLiteral("最大值"))
            : (QStringList() << QStringLiteral("count") << QStringLiteral("mean") << QStringLiteral("min") << QStringLiteral("median") << QStringLiteral("max")));
        QStringList headers;
        for (int column : numericColumns) {
            headers << ColumnLabel(column);
            QVector<double> values = NumericColumnValues(column);
            std::sort(values.begin(), values.end());
            if (values.isEmpty()) {
                continue;
            }
            int outColumn = headers.size() - 1;
            double sum = std::accumulate(values.begin(), values.end(), 0.0);
            QList<double> stats = {
                static_cast<double>(values.size()),
                sum / values.size(),
                values.first(),
                values.at(values.size() / 2),
                values.last(),
            };
            for (int row = 0; row < stats.size(); ++row) {
                ui->tableWidget_2->setItem(row, outColumn, new QTableWidgetItem(QString::number(stats[row], 'g', 10)));
            }
        }
        ui->tableWidget_2->setHorizontalHeaderLabels(headers);
        return;
    }

    int visibleColumns = std::min(static_cast<int>(numericProfiles.size()), 12);
    ui->tableWidget_2->clear();
    ui->tableWidget_2->setRowCount(6);
    ui->tableWidget_2->setColumnCount(visibleColumns);
    ui->tableWidget_2->setVerticalHeaderLabels(useChinese
        ? (QStringList() << QStringLiteral("数量") << QStringLiteral("均值") << QStringLiteral("标准差") << QStringLiteral("最小值") << QStringLiteral("中位数") << QStringLiteral("最大值"))
        : (QStringList() << QStringLiteral("count") << QStringLiteral("mean") << QStringLiteral("std") << QStringLiteral("min") << QStringLiteral("median") << QStringLiteral("max")));

    QStringList headers;
    for (int column = 0; column < visibleColumns; ++column) {
        QJsonObject profile = numericProfiles.at(column);
        headers << profile.value("column").toString();
        QStringList keys = {"count", "mean", "std", "min", "median", "max"};
        for (int row = 0; row < keys.size(); ++row) {
            QJsonValue value = profile.value(keys[row]);
            QString text = value.isDouble() ? QString::number(value.toDouble(), 'g', 10) : value.toVariant().toString();
            ui->tableWidget_2->setItem(row, column, new QTableWidgetItem(text));
        }
    }
    ui->tableWidget_2->setHorizontalHeaderLabels(headers);
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::UpdateOverviewCards(const QJsonObject &root)
{
    QJsonObject dataset = root.value("dataset").toObject();
    QJsonObject quality = root.value("quality").toObject();
    QJsonArray schema = root.value("schema").toArray();
    QJsonArray recommendations = root.value("analysis_recommendations").toArray();
    QJsonObject plan = root.value("analysis_plan").toObject();
    QJsonObject source = root.value("source").toObject();

    if (datasetCard) {
        datasetCard->setText(QStringLiteral("<b>%1</b><br><span>%2 x %3</span><br><small>%4</small>")
                                 .arg(Text(QStringLiteral("Dataset"), QStringLiteral("数据集")))
                                 .arg(dataset.value("rows").toInt())
                                 .arg(dataset.value("columns").toInt())
                                 .arg(dataset.value("filename").toString().toHtmlEscaped()));
    }
    if (qualityCard) {
        qualityCard->setText(QStringLiteral("<b>%1</b><br><span>%2 / 100</span><br><small>%3</small>")
                                 .arg(Text(QStringLiteral("Quality"), QStringLiteral("质量")))
                                 .arg(quality.value("score").toInt())
                                 .arg(QualityLevelText(quality.value("level").toString()).toHtmlEscaped()));
    }
    if (schemaCard) {
        schemaCard->setText(QStringLiteral("<b>%1</b><br><span>%2 %3</span><br><small>%4 / %5</small>")
                                .arg(Text(QStringLiteral("Schema"), QStringLiteral("结构")))
                                .arg(schema.size())
                                .arg(Text(QStringLiteral("fields"), QStringLiteral("个字段")))
                                .arg(Text(QStringLiteral("%1 numeric").arg(dataset.value("numeric_columns").toInt()),
                                          QStringLiteral("%1 个数值字段").arg(dataset.value("numeric_columns").toInt())))
                                .arg(Text(QStringLiteral("%1 date").arg(dataset.value("date_columns").toInt()),
                                          QStringLiteral("%1 个日期字段").arg(dataset.value("date_columns").toInt()))));
    }
    if (recommendationCard) {
        QString title = Text(QStringLiteral("Review data quality and chart recommendations"), QStringLiteral("复核数据质量和图表建议"));
        if (!recommendations.isEmpty()) {
            title = RecommendationTitle(recommendations.first().toObject());
        }
        QString story = useChinese
            ? Text(QStringLiteral(""), QStringLiteral("根据字段结构、质量和趋势生成下一步建议"))
            : plan.value("dataset_story").toString();
        recommendationCard->setText(QStringLiteral("<b>%1</b><br><span>%2</span><br><small>%3</small>")
                                        .arg(Text(QStringLiteral("Next best analysis"), QStringLiteral("下一步分析")))
                                        .arg(title.toHtmlEscaped())
                                        .arg((story.isEmpty() ? source.value("parser").toString() : story).toHtmlEscaped()));
    }
}

void MainWindow::UpdateDefaultOverviewCards(const QString &serviceState)
{
    QString state = serviceState;
    if (state == "idle") {
        state = QStringLiteral("checking");
    }
    QString serviceText = Text(QStringLiteral("checking"), QStringLiteral("检查中"));
    if (state == "starting") {
        serviceText = Text(QStringLiteral("starting"), QStringLiteral("启动中"));
    } else if (state == "connected") {
        serviceText = Text(QStringLiteral("connected"), QStringLiteral("已连接"));
    } else if (state == "offline") {
        serviceText = Text(QStringLiteral("offline"), QStringLiteral("离线"));
    }
    if (serviceBadge) {
        serviceBadge->setText(QStringLiteral("<b>%1</b><br><span>%2</span>")
                                  .arg(Text(QStringLiteral("Service"), QStringLiteral("服务")))
                                  .arg(serviceText));
    }
    if (datasetCard) {
        datasetCard->setText(QStringLiteral("<b>%1</b><br><span>%2</span>")
                                 .arg(Text(QStringLiteral("Dataset"), QStringLiteral("数据集")))
                                 .arg(Text(QStringLiteral("No file"), QStringLiteral("未打开文件"))));
    }
    if (qualityCard) {
        qualityCard->setText(QStringLiteral("<b>%1</b><br><span>-</span>")
                                 .arg(Text(QStringLiteral("Quality"), QStringLiteral("质量"))));
    }
    if (schemaCard) {
        schemaCard->setText(QStringLiteral("<b>%1</b><br><span>-</span>")
                                .arg(Text(QStringLiteral("Schema"), QStringLiteral("结构"))));
    }
    if (recommendationCard) {
        recommendationCard->setText(QStringLiteral("<b>%1</b><br><span>%2</span>")
                                        .arg(Text(QStringLiteral("Next best analysis"), QStringLiteral("下一步分析")))
                                        .arg(Text(QStringLiteral("Open a table to start"), QStringLiteral("打开表格后开始"))));
    }
    if (sheetSelector) {
        sheetSelector->setVisible(false);
    }
    if (sheetSelectorLabel) {
        sheetSelectorLabel->setVisible(false);
    }
    UpdateRecommendationActions(QJsonObject());
}

void MainWindow::UpdateRecommendationActions(const QJsonObject &root)
{
    const bool hasData = !root.isEmpty();
    if (suggestTrendButton) {
        suggestTrendButton->setEnabled(hasData && root.value("dataset").toObject().value("numeric_columns").toInt() > 0);
        suggestTrendButton->setText(Text(QStringLiteral("Open trend"), QStringLiteral("查看趋势")));
    }
    if (suggestDistributionButton) {
        suggestDistributionButton->setEnabled(hasData && root.value("dataset").toObject().value("numeric_columns").toInt() > 0);
        suggestDistributionButton->setText(Text(QStringLiteral("Open distribution"), QStringLiteral("查看分布")));
    }
    if (suggestQualityButton) {
        suggestQualityButton->setEnabled(hasData);
        int missing = root.value("dataset").toObject().value("missing_cells").toInt();
        int anomalies = root.value("quality").toObject().value("anomaly_count").toInt();
        QString label = missing > 0 || anomalies > 0
            ? Text(QStringLiteral("Review risks"), QStringLiteral("复核风险"))
            : Text(QStringLiteral("Review quality"), QStringLiteral("复核质量"));
        suggestQualityButton->setText(label);
    }
    if (exportReportButton) {
        exportReportButton->setEnabled(hasData);
        exportReportButton->setText(Text(QStringLiteral("Export report"), QStringLiteral("导出报告")));
    }
}

QString MainWindow::QualityLevelText(const QString &level) const
{
    if (!useChinese) {
        return level;
    }
    if (level == "high") return QStringLiteral("高");
    if (level == "medium") return QStringLiteral("中");
    if (level == "low") return QStringLiteral("低");
    return level;
}

QString MainWindow::SemanticTypeText(const QString &type) const
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

QString MainWindow::RoleHintText(const QString &role) const
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

QString MainWindow::DirectionText(const QString &direction) const
{
    if (!useChinese) {
        return direction;
    }
    if (direction == "up") return QStringLiteral("上升");
    if (direction == "down") return QStringLiteral("下降");
    if (direction == "flat") return QStringLiteral("平稳");
    return direction;
}

QString MainWindow::ToolTraceText(const QString &step) const
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

QString MainWindow::RecommendationTitle(const QJsonObject &item) const
{
    if (!useChinese) {
        return item.value("title").toString();
    }
    const QString type = item.value("type").toString();
    if (type == "trend") return QStringLiteral("分析时间趋势");
    if (type == "group_compare") return QStringLiteral("按维度对比关键指标");
    if (type == "correlation") return QStringLiteral("复核数值字段之间的关系");
    if (type == "quality") return QStringLiteral("优先复核数据质量");
    return item.value("title").toString();
}

QString MainWindow::RecommendationReason(const QJsonObject &item) const
{
    if (!useChinese) {
        return item.value("reason").toString();
    }
    const QString type = item.value("type").toString();
    if (type == "trend") return QStringLiteral("检测到日期字段和数值指标，适合观察随时间变化。");
    if (type == "group_compare") return QStringLiteral("检测到分类维度，可用于解释不同分组之间的指标差异。");
    if (type == "correlation") return QStringLiteral("存在多个数值字段，适合检查相关性和共同变化。");
    if (type == "quality") return QStringLiteral("数据质量信号会影响后续分析可信度，应先复核。");
    return item.value("reason").toString();
}

QString MainWindow::PlanTitle(const QJsonObject &step) const
{
    if (!useChinese) {
        return step.value("title").toString();
    }
    const QString stage = step.value("stage").toString();
    if (stage == "trend") return QStringLiteral("趋势复核");
    if (stage == "segment") return QStringLiteral("分组对比");
    if (stage == "relationship") return QStringLiteral("关系复核");
    if (stage == "quality") return QStringLiteral("质量复核");
    return step.value("title").toString();
}

QString MainWindow::PlanReason(const QJsonObject &step) const
{
    if (!useChinese) {
        return step.value("why").toString();
    }
    const QString stage = step.value("stage").toString();
    if (stage == "trend") return QStringLiteral("检测到时间轴和数值指标，时间变化可能具有分析价值。");
    if (stage == "segment") return QStringLiteral("分类维度可以帮助解释关键指标在不同分组中的差异。");
    if (stage == "relationship") return QStringLiteral("数值字段之间存在明显相关性，建议进一步复核业务含义。");
    if (stage == "quality") return QStringLiteral("数据质量风险会影响结论可信度，需要先排查。");
    return step.value("why").toString();
}

QString MainWindow::InsightText(const QString &value, const QJsonObject &root) const
{
    if (!useChinese) {
        return value;
    }
    QJsonObject dataset = root.value("dataset").toObject();
    QJsonObject quality = root.value("quality").toObject();
    QJsonArray trends = root.value("trends").toArray();
    QJsonArray recommendations = root.value("analysis_recommendations").toArray();

    if (value.startsWith("Loaded ")) {
        return QStringLiteral("已加载 %1 行、%2 列。")
            .arg(dataset.value("rows").toInt())
            .arg(dataset.value("columns").toInt());
    }
    if (value.startsWith("Detected ")) {
        return QStringLiteral("检测到 %1 个数值字段、%2 个日期字段、%3 个分类字段。")
            .arg(dataset.value("numeric_columns").toInt())
            .arg(dataset.value("date_columns").toInt())
            .arg(dataset.value("category_columns").toInt());
    }
    if (value.startsWith("Data quality is ")) {
        return QStringLiteral("数据质量为%1，评分 %2/100。")
            .arg(QualityLevelText(quality.value("level").toString()))
            .arg(quality.value("score").toInt());
    }
    if (value.startsWith("No missing cells")) {
        return QStringLiteral("加载的表格中未检测到缺失单元格。");
    }
    if (value.startsWith("No high z-score anomalies")) {
        return QStringLiteral("默认阈值下未检测到高 z-score 异常。");
    }
    if (value.startsWith("The strongest simple trend") && !trends.isEmpty()) {
        QJsonObject trend = trends.first().toObject();
        return QStringLiteral("最明显的简单趋势是字段 %1 %2。")
            .arg(trend.value("column").toString())
            .arg(DirectionText(trend.value("direction").toString()));
    }
    if (value.startsWith("Recommended next analysis") && !recommendations.isEmpty()) {
        return QStringLiteral("建议下一步：%1。").arg(RecommendationTitle(recommendations.first().toObject()));
    }
    return value;
}

QString MainWindow::DecisionTitleText(const QJsonObject &finding) const
{
    if (!useChinese) {
        return finding.value("title").toString();
    }
    const QString id = finding.value("id").toString();
    if (id == "segment_leader") {
        return QStringLiteral("%1 是 %2 的领先分组")
            .arg(finding.value("segment").toString(), finding.value("measure").toString());
    }
    if (id == "trend_signal") {
        return QStringLiteral("%1 正在%2")
            .arg(finding.value("column").toString(), DirectionText(finding.value("direction").toString()));
    }
    if (id == "relationship_signal") {
        return QStringLiteral("%1 与 %2 存在联动")
            .arg(finding.value("left").toString(), finding.value("right").toString());
    }
    if (id == "anomaly_queue") {
        return QStringLiteral("第 %1 行需要复核").arg(finding.value("row").toInt());
    }
    if (id == "quality_gate") {
        return QStringLiteral("先完成清洗复核，再做最终结论");
    }
    if (id == "next_view") {
        return QStringLiteral("建议先打开系统推荐视图");
    }
    return finding.value("title").toString();
}

QString MainWindow::DecisionExplanationText(const QJsonObject &finding) const
{
    if (!useChinese) {
        return finding.value("explanation").toString();
    }
    const QString id = finding.value("id").toString();
    if (id == "segment_leader") {
        QString text = QStringLiteral("%1 贡献了 %2 总量的 %3%。")
            .arg(finding.value("segment").toString(), finding.value("measure").toString())
            .arg(finding.value("share").toDouble());
        if (!finding.value("second_segment").isNull()) {
            text += QStringLiteral("第二名是 %1，数值为 %2。")
                .arg(finding.value("second_segment").toString())
                .arg(finding.value("second_value").toDouble());
        }
        return text;
    }
    if (id == "trend_signal") {
        return QStringLiteral("%1 从 %2 变化到 %3，简单斜率为 %4。")
            .arg(finding.value("column").toString())
            .arg(finding.value("first").toDouble())
            .arg(finding.value("last").toDouble())
            .arg(finding.value("slope").toDouble());
    }
    if (id == "relationship_signal") {
        return QStringLiteral("相关系数为 %1，说明两个字段在当前样本中存在较明显的共同变化。")
            .arg(finding.value("correlation").toDouble());
    }
    if (id == "anomaly_queue") {
        return QStringLiteral("%1 的数值 %2 偏离较大，z-score 为 %3。")
            .arg(finding.value("column").toString())
            .arg(finding.value("value").toDouble())
            .arg(finding.value("z_score").toDouble());
    }
    if (id == "quality_gate") {
        return QStringLiteral("缺失比例为 %1，重复行数量为 %2，这些问题可能影响汇总、排序和图表。")
            .arg(finding.value("missing_ratio").toDouble())
            .arg(finding.value("duplicate_rows").toInt());
    }
    return finding.value("explanation").toString();
}

QString MainWindow::DecisionEvidenceText(const QJsonObject &finding) const
{
    if (!useChinese) {
        return finding.value("evidence").toString();
    }
    const QString id = finding.value("id").toString();
    if (id == "segment_leader") {
        return QStringLiteral("按 %1 汇总 %2，最高分组数值为 %3。")
            .arg(finding.value("dimension").toString(), finding.value("measure").toString())
            .arg(finding.value("value").toDouble());
    }
    if (id == "trend_signal") {
        return QStringLiteral("基于当前记录顺序计算 %1 的变化方向。").arg(finding.value("column").toString());
    }
    if (id == "relationship_signal") {
        return QStringLiteral("基于 %1 和 %2 的有效数值记录计算相关性。")
            .arg(finding.value("left").toString(), finding.value("right").toString());
    }
    if (id == "anomaly_queue") {
        return QStringLiteral("共检测到 %1 个异常候选单元格。").arg(finding.value("count").toInt());
    }
    if (id == "quality_gate") {
        return QStringLiteral("质量评分发现了会影响统计结论的数据问题。");
    }
    return finding.value("evidence").toString();
}

QString MainWindow::DecisionActionText(const QJsonObject &finding) const
{
    if (!useChinese) {
        return finding.value("action").toString();
    }
    const QString id = finding.value("id").toString();
    if (id == "segment_leader") {
        return QStringLiteral("建议对比领先分组和后续分组，确认差距是否符合业务预期。");
    }
    if (id == "trend_signal") {
        return QStringLiteral("建议打开图表工作台，确认变化是稳定趋势，还是少数记录造成的波动。");
    }
    if (id == "relationship_signal") {
        return QStringLiteral("建议使用散点图或相关热力图复核，避免把相关性误读为因果关系。");
    }
    if (id == "anomaly_queue") {
        return QStringLiteral("建议打开异常复核抽屉，跳转到原始单元格后决定保留、修正或备注。");
    }
    if (id == "quality_gate") {
        return QStringLiteral("建议先查看清洗对比，再导出清洗后的数据或报告。");
    }
    return finding.value("action").toString();
}

QString MainWindow::LimitationText(const QString &value) const
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

QString MainWindow::ViewReasonText(const QJsonObject &view) const
{
    if (!useChinese) {
        return view.value("reason").toString();
    }
    const QString id = view.value("id").toString();
    if (id == "trend") return QStringLiteral("检测到可用于趋势观察的数值指标，适合先看变化方向。");
    if (id == "segment") return QStringLiteral("检测到分类维度和关键指标，适合比较不同分组的表现。");
    if (id == "correlation") return QStringLiteral("存在多个数值字段，适合复核字段之间的联动关系。");
    if (id == "distribution") return QStringLiteral("检测到可分析指标，适合查看分布、离散程度和异常值。");
    if (id == "quality") return QStringLiteral("在解释结果前，应先确认缺失、重复和异常候选。");
    return view.value("reason").toString();
}

QString MainWindow::CardTitleText(const QJsonObject &card) const
{
    if (!useChinese) {
        return card.value("title").toString();
    }
    const QString id = card.value("id").toString();
    if (id == "dataset_fingerprint") return QStringLiteral("数据类型识别");
    if (id == "quality_score") return QStringLiteral("数据质量评分");
    if (id == "top_trend") return QStringLiteral("关键趋势");
    if (id == "top_correlation") return QStringLiteral("字段关系");
    if (id == "anomaly_review") return QStringLiteral("异常复核");
    if (id == "distribution_ready") return QStringLiteral("分布分析可用");
    if (id == "segment_opportunity") return QStringLiteral("分组对比机会");
    return card.value("title").toString();
}

QString MainWindow::CardSummaryText(const QJsonObject &card) const
{
    if (!useChinese) {
        return card.value("summary").toString();
    }
    const QString id = card.value("id").toString();
    if (id == "dataset_fingerprint") return QStringLiteral("系统已根据字段名称、字段类型和质量信号判断数据用途。");
    if (id == "quality_score") return QStringLiteral("质量评分综合考虑缺失、重复、异常值和可分析字段比例。");
    if (id == "top_trend") return QStringLiteral("检测到一个变化最明显的数值指标，建议优先查看趋势图。");
    if (id == "top_correlation") return QStringLiteral("检测到数值字段之间存在较明显的相关关系，需要结合业务语境复核。");
    if (id == "anomaly_review") return QStringLiteral("部分单元格偏离字段分布较远，建议在生成结论前先复核。");
    if (id == "distribution_ready") return QStringLiteral("已检测到可用于分布分析的数值指标。");
    if (id == "segment_opportunity") return QStringLiteral("分类维度和关键指标可以组成分组对比视图。");
    return card.value("summary").toString();
}

QString MainWindow::CardEvidenceText(const QJsonObject &card) const
{
    if (!useChinese) {
        return card.value("evidence").toString();
    }
    const QString id = card.value("id").toString();
    if (id == "dataset_fingerprint") return QStringLiteral("根据行列规模、字段角色和质量评分生成。");
    if (id == "quality_score") return QStringLiteral("综合缺失比例、重复行、异常候选和可分析字段比例。");
    if (id == "top_trend") return QStringLiteral("基于首尾数值和记录顺序计算简单趋势。");
    if (id == "top_correlation") return QStringLiteral("基于有效数值记录计算字段之间的相关性。");
    if (id == "anomaly_review") return QStringLiteral("基于数值字段的 z-score 检测高偏离单元格。");
    if (id == "distribution_ready") return QStringLiteral("至少有一个数值指标可用于查看分布。");
    if (id == "segment_opportunity") return QStringLiteral("已识别到分类维度和可汇总指标。");
    return card.value("evidence").toString();
}

QString MainWindow::RepairTitleText(const QJsonObject &item) const
{
    if (!useChinese) {
        return item.value("title").toString();
    }
    const QString id = item.value("id").toString();
    if (id == "empty_structure") return QStringLiteral("确认空行空列清理");
    if (id == "duplicate_columns") return QStringLiteral("复核重复字段");
    if (id == "missing_values") return QStringLiteral("处理缺失值");
    if (id == "duplicate_rows") return QStringLiteral("复核重复记录");
    if (id == "anomaly_review") return QStringLiteral("复核异常候选");
    if (id == "sample_size") return QStringLiteral("样本量偏小");
    if (id == "quality_clear") return QStringLiteral("暂无阻塞性质量问题");
    return item.value("title").toString();
}

QString MainWindow::RepairRecommendationText(const QJsonObject &item) const
{
    if (!useChinese) {
        return item.value("recommendation").toString();
    }
    const QString id = item.value("id").toString();
    if (id == "empty_structure") return QStringLiteral("分析服务已自动排除全空行和全空列，但仍建议确认这些区域不是隐藏说明或二级表头。");
    if (id == "duplicate_columns") return QStringLiteral("建议合并或删除重复字段，避免相关性和字段数量被误读。");
    if (id == "missing_values") return QStringLiteral("建议根据字段类型选择填补、排除或保留缺失值，并在报告里说明影响范围。");
    if (id == "duplicate_rows") return QStringLiteral("建议确认重复行是合法重复事件还是导入错误。");
    if (id == "anomaly_review") return QStringLiteral("建议先检查高偏离值，再使用均值、趋势或分布结论。");
    if (id == "sample_size") return QStringLiteral("当前样本量较小，适合探索，不适合做最终统计结论。");
    if (id == "quality_clear") return QStringLiteral("可以继续做趋势、分组和关系分析。");
    return item.value("recommendation").toString();
}

QString MainWindow::RepairImpactText(const QJsonObject &item) const
{
    if (!useChinese) {
        return item.value("impact").toString();
    }
    const QString id = item.value("id").toString();
    if (id == "empty_structure") return QStringLiteral("当前分析已经排除了全空行列。");
    if (id == "duplicate_columns") return QStringLiteral("重复字段会让结构识别和相关性判断失真。");
    if (id == "missing_values") return QStringLiteral("该问题会影响汇总、均值、排序和图表解释。");
    if (id == "duplicate_rows") return QStringLiteral("重复记录可能抬高计数和求和结果。");
    if (id == "anomaly_review") return QStringLiteral("异常值可能改变均值、趋势斜率和图表缩放。");
    if (id == "sample_size") return QStringLiteral("样本较小时，结论只能作为探索线索。");
    if (id == "quality_clear") return QStringLiteral("当前无需先做阻塞性清洗。");
    return item.value("impact").toString();
}

QString MainWindow::ViewLabelText(const QJsonObject &view) const
{
    if (!useChinese) {
        return view.value("label").toString();
    }
    const QString id = view.value("id").toString();
    if (id == "trend") return QStringLiteral("趋势视图");
    if (id == "segment") return QStringLiteral("分组对比");
    if (id == "correlation") return QStringLiteral("相关性视图");
    if (id == "distribution") return QStringLiteral("分布视图");
    if (id == "quality") return QStringLiteral("质量复核");
    return view.value("label").toString();
}

QString MainWindow::FormatServiceAnalysis(const QJsonObject &root) const
{
    QJsonObject dataset = root.value("dataset").toObject();
    QJsonObject quality = root.value("quality").toObject();
    QJsonArray insights = root.value("insights").toArray();
    QJsonArray anomalies = root.value("anomalies").toArray();
    QJsonArray trends = root.value("trends").toArray();
    QJsonArray correlations = root.value("correlations").toArray();
    QJsonArray recommendations = root.value("analysis_recommendations").toArray();
    QJsonArray charts = root.value("chart_recommendations").toArray();
    QJsonArray schema = root.value("schema").toArray();
    QJsonArray toolTrace = root.value("tool_trace").toArray();
    QJsonObject source = root.value("source").toObject();

    QStringList lines;
    lines << QStringLiteral("TablePilot Analysis");
    lines << QStringLiteral("======================");
    lines << QStringLiteral("Dataset: %1").arg(dataset.value("filename").toString());
    lines << QStringLiteral("Shape: %1 rows x %2 columns")
                 .arg(dataset.value("rows").toInt())
                 .arg(dataset.value("columns").toInt());
    lines << QStringLiteral("Numeric columns: %1").arg(dataset.value("numeric_columns").toInt());
    lines << QStringLiteral("Date columns: %1").arg(dataset.value("date_columns").toInt());
    lines << QStringLiteral("Category columns: %1").arg(dataset.value("category_columns").toInt());
    lines << QStringLiteral("Quality: %1 / 100 (%2)")
                 .arg(quality.value("score").toInt())
                 .arg(quality.value("level").toString());
    lines << QStringLiteral("Missing ratio: %1").arg(quality.value("missing_ratio").toDouble());
    lines << QStringLiteral("Duplicate rows: %1").arg(quality.value("duplicate_rows").toInt());
    lines << QStringLiteral("Anomalies: %1").arg(quality.value("anomaly_count").toInt());
    if (!source.isEmpty()) {
        lines << QStringLiteral("Parser: %1, delimiter: %2, encoding: %3, header: %4")
                     .arg(source.value("parser").toString())
                     .arg(source.value("delimiter").toString("-"))
                     .arg(source.value("encoding").toString("-"))
                     .arg(source.value("has_header").toBool() ? "yes" : "no");
    }

    if (!schema.isEmpty()) {
        lines << "";
        lines << QStringLiteral("Schema");
        for (int i = 0; i < schema.size() && i < 8; ++i) {
            QJsonObject item = schema.at(i).toObject();
            lines << QStringLiteral("- %1: %2 (%3)")
                         .arg(item.value("name").toString())
                         .arg(item.value("semantic_type").toString())
                         .arg(item.value("role_hint").toString());
        }
    }

    if (!trends.isEmpty()) {
        QJsonObject trend = trends.first().toObject();
        lines << "";
        lines << QStringLiteral("Top Trend");
        lines << QStringLiteral("- Column %1 is moving %2, slope %3")
                     .arg(trend.value("column").toString())
                     .arg(trend.value("direction").toString())
                     .arg(trend.value("slope").toDouble());
    }

    if (!correlations.isEmpty()) {
        QJsonObject corr = correlations.first().toObject();
        lines << "";
        lines << QStringLiteral("Top Correlation");
        lines << QStringLiteral("- Column %1 vs %2: %3 (%4)")
                     .arg(corr.value("left").toString())
                     .arg(corr.value("right").toString())
                     .arg(corr.value("correlation").toDouble())
                     .arg(corr.value("strength").toString());
    }

    if (!recommendations.isEmpty()) {
        lines << "";
        lines << QStringLiteral("Recommended Analysis");
        for (int i = 0; i < recommendations.size() && i < 5; ++i) {
            QJsonObject item = recommendations.at(i).toObject();
            lines << QStringLiteral("- %1: %2")
                         .arg(item.value("type").toString())
                         .arg(item.value("title").toString());
        }
    }

    if (!charts.isEmpty()) {
        QJsonObject chart = charts.first().toObject();
        lines << "";
        lines << QStringLiteral("Chart Recommendation");
        lines << QStringLiteral("- %1, x=%2, y=%3")
                     .arg(chart.value("chart_type").toString())
                     .arg(chart.value("x").toString())
                     .arg(chart.value("y").toString());
        lines << QStringLiteral("- %1").arg(chart.value("reason").toString());
    }

    lines << "";
    lines << QStringLiteral("Evidence Summary");
    for (const QJsonValue &value : insights) {
        lines << QStringLiteral("- %1").arg(value.toString());
    }

    if (!anomalies.isEmpty()) {
        lines << "";
        lines << QStringLiteral("Review Queue");
        for (int i = 0; i < anomalies.size() && i < 5; ++i) {
            QJsonObject anomaly = anomalies.at(i).toObject();
            lines << QStringLiteral("- Row %1, column %2, value %3, z-score %4")
                         .arg(anomaly.value("row").toInt() + 1)
                         .arg(anomaly.value("column").toString())
                         .arg(anomaly.value("value").toDouble())
                         .arg(anomaly.value("z_score").toDouble());
        }
    }

    if (!toolTrace.isEmpty()) {
        lines << "";
        lines << QStringLiteral("Tool Trace");
        for (const QJsonValue &value : toolTrace) {
            lines << QStringLiteral("- %1").arg(value.toString());
        }
    }

    lines << "";
    lines << QStringLiteral("Note: This is an analytical summary, not a business recommendation.");
    return lines.join("\n");
}

QString MainWindow::FormatInsightHtml(const QJsonObject &root) const
{
    QJsonObject dataset = root.value("dataset").toObject();
    QJsonObject quality = root.value("quality").toObject();
    QJsonObject brief = root.value("executive_brief").toObject();
    QJsonObject plan = root.value("analysis_plan").toObject();
    QJsonArray schema = root.value("schema").toArray();
    QJsonArray insights = root.value("insights").toArray();
    QJsonArray anomalies = root.value("anomalies").toArray();
    QJsonArray recommendations = root.value("analysis_recommendations").toArray();
    QJsonArray charts = root.value("chart_recommendations").toArray();
    QJsonArray insightCards = root.value("insight_cards").toArray();
    QJsonObject decision = root.value("decision_brief").toObject();
    QJsonObject business = root.value("business_analysis").toObject();
    QJsonArray repairPlan = root.value("quality_repair_plan").toArray();
    QJsonArray recommendedViews = root.value("recommended_views").toArray();
    QJsonObject fingerprint = root.value("dataset_fingerprint").toObject();
    QJsonObject diagnostics = root.value("table_diagnostics").toObject();
    QJsonObject localAi = root.value("local_ai").toObject();
    QJsonObject source = root.value("source").toObject();

    {
        auto label = [this](const QString &en, const QString &zh) {
            return Text(en, zh);
        };
        auto chip = [](const QString &name, const QString &value) {
            return QStringLiteral("<span class='chip'><b>%1</b> %2</span>")
                .arg(name.toHtmlEscaped(), value.toHtmlEscaped());
        };
        auto localAiStatusText = [this](const QJsonObject &ai) {
            const QString status = ai.value("status").toString();
            const QString model = ai.value("model").toString(QStringLiteral("qwen3-4b"));
            if (status == "generated") {
                return Text(QStringLiteral("%1 generated a wording layer from the structured evidence."),
                            QStringLiteral("%1 已基于结构化证据生成自然语言解释。")).arg(model);
            }
            if (status == "guardrail_failed") {
                return Text(QStringLiteral("%1 responded, but the output referenced unsupported evidence, so TablePilot kept the deterministic report."),
                            QStringLiteral("%1 有返回结果，但引用了数据中不存在的证据，系统已自动保留规则报告。")).arg(model);
            }
            if (status == "unavailable") {
                return Text(QStringLiteral("Local model is configured but not reachable. Deterministic analysis remains available."),
                            QStringLiteral("本地模型已配置但暂时无法连接，规则分析仍可正常使用。"));
            }
            return Text(QStringLiteral("Local model wording is disabled for this run. The report is generated by deterministic rules."),
                        QStringLiteral("本次未启用本地模型润色，报告由确定性规则生成。"));
        };
        auto statusText = [this](const QString &status) {
            if (!useChinese) {
                return status;
            }
            if (status == "generated") return QStringLiteral("已生成");
            if (status == "guardrail_failed") return QStringLiteral("护栏拦截");
            if (status == "unavailable") return QStringLiteral("不可用");
            if (status == "disabled") return QStringLiteral("未启用");
            return status;
        };

        QStringList html;
        html << QStringLiteral(
            "<style>"
            "body{font-family:'SF Pro Text','Segoe UI','Microsoft YaHei',Arial;color:#1d1d1f;background:#fff;}"
            "h1{font-size:24px;margin:0 0 4px 0;font-weight:750;}"
            "h2{font-size:15px;margin:18px 0 8px 0;font-weight:700;}"
            "p{line-height:1.55;margin:5px 0 9px 0;}"
            ".muted{color:#6e6e73;}"
            ".hero{border:1px solid #e5e5ea;background:#fbfbfd;border-radius:16px;padding:14px;margin:12px 0;}"
            ".good{border-left:4px solid #34c759;}.warn{border-left:4px solid #ff9f0a;}.info{border-left:4px solid #007aff;}"
            ".grid{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin:12px 0;}"
            ".card{border:1px solid #e5e5ea;background:#fff;border-radius:14px;padding:11px;}"
            ".card b{font-size:13px;}.num{font-size:22px;font-weight:750;color:#007aff;margin-top:4px;}"
            ".chip{display:inline-block;border:1px solid #e5e5ea;border-radius:999px;padding:4px 8px;margin:3px;background:#f5f5f7;font-size:12px;}"
            ".item{border-bottom:1px solid #f0f0f2;padding:9px 0;}.item:last-child{border-bottom:none;}"
            ".action{border:1px solid #d2e7ff;background:#f6fbff;border-radius:14px;padding:10px 12px;margin:8px 0;}"
            "ul{padding-left:18px;}li{margin:5px 0;}"
            "</style>"
        );

        const int rows = dataset.value("rows").toInt();
        const int columns = dataset.value("columns").toInt();
        const int score = quality.value("score").toInt();
        const int missing = dataset.value("missing_cells").toInt();
        const int anomalyCount = quality.value("anomaly_count").toInt();
        const QString scoreTone = score >= 80 ? QStringLiteral("good") : (score >= 60 ? QStringLiteral("info") : QStringLiteral("warn"));
        QString dataType = fingerprint.value("label").toString(QStringLiteral("tabular dataset"));
        if (useChinese) {
            if (dataType == "Sales or operations table") {
                dataType = QStringLiteral("销售/运营表");
            } else if (dataType == "Financial or transaction table") {
                dataType = QStringLiteral("财务/交易表");
            } else if (dataType == "Time-series measurement table") {
                dataType = QStringLiteral("时间序列表");
            } else if (dataType == "Dimensional analysis table") {
                dataType = QStringLiteral("维度分析表");
            } else if (dataType == "Generic tabular dataset") {
                dataType = QStringLiteral("通用表格数据");
            }
        }
        QJsonObject businessOverview = business.value("overview").toObject();
        QString primaryQuestion = decision.value("primary_question").toString(QStringLiteral("What changed, what stands out, and what should be checked before reporting?"));
        if (useChinese) {
            const QString measure = businessOverview.value("primary_measure").toString();
            const QString dimension = businessOverview.value("primary_dimension").toString();
            const QString timeAxis = businessOverview.value("primary_time_axis").toString();
            if (!measure.isEmpty() && !dimension.isEmpty() && !timeAxis.isEmpty()) {
                primaryQuestion = QStringLiteral("%1 的变化主要由哪些 %2 分组驱动？这种变化在 %3 上是否稳定？")
                    .arg(measure, dimension, timeAxis);
            } else if (!measure.isEmpty() && !dimension.isEmpty()) {
                primaryQuestion = QStringLiteral("哪些 %1 分组最能解释 %2 的差异？").arg(dimension, measure);
            } else if (!measure.isEmpty()) {
                primaryQuestion = QStringLiteral("%1 里有哪些趋势、风险和分布信号？").arg(measure);
            } else {
                primaryQuestion = QStringLiteral("这份表有哪些可分析结构？在深入分析前需要先清理什么？");
            }
        }

        html << QStringLiteral("<h1>%1</h1>").arg(label(QStringLiteral("Analysis Brief"), QStringLiteral("分析简报")));
        html << QStringLiteral("<p class='muted'>%1 <b>%2</b></p>")
                    .arg(label(QStringLiteral("File"), QStringLiteral("文件：")), dataset.value("filename").toString().toHtmlEscaped());
        html << QStringLiteral("<div>%1%2%3%4</div>")
                    .arg(chip(label(QStringLiteral("Rows"), QStringLiteral("行数")), QString::number(rows)))
                    .arg(chip(label(QStringLiteral("Columns"), QStringLiteral("列数")), QString::number(columns)))
                    .arg(chip(label(QStringLiteral("Quality"), QStringLiteral("质量")), QStringLiteral("%1/100").arg(score)))
                    .arg(chip(label(QStringLiteral("Type"), QStringLiteral("类型")), dataType));

        QString headline = useChinese
            ? QStringLiteral("这份表可以用于初步分析。系统识别出 %1 个数值字段、%2 个日期字段和 %3 个分组字段。系统建议先回答：%4")
                .arg(dataset.value("numeric_columns").toInt())
                .arg(dataset.value("date_columns").toInt())
                .arg(dataset.value("category_columns").toInt())
                .arg(primaryQuestion)
            : QStringLiteral("This table is ready for exploratory analysis. TablePilot found %1 numeric fields, %2 date fields, and %3 grouping fields. The first question is: %4")
                .arg(dataset.value("numeric_columns").toInt())
                .arg(dataset.value("date_columns").toInt())
                .arg(dataset.value("category_columns").toInt())
                .arg(primaryQuestion);
        html << QStringLiteral("<div class='hero %1'><b>%2</b><p>%3</p></div>")
                    .arg(scoreTone)
                    .arg(label(QStringLiteral("What TablePilot found"), QStringLiteral("TablePilot 发现了什么")).toHtmlEscaped())
                    .arg(headline.toHtmlEscaped());

        html << QStringLiteral("<div class='grid'>");
        html << QStringLiteral("<div class='card'><b>%1</b><div class='num'>%2</div><span class='muted'>%3</span></div>")
                    .arg(label(QStringLiteral("Data quality"), QStringLiteral("数据质量")), QString::number(score), QualityLevelText(quality.value("level").toString()));
        html << QStringLiteral("<div class='card'><b>%1</b><div class='num'>%2</div><span class='muted'>%3</span></div>")
                    .arg(label(QStringLiteral("Cells to review"), QStringLiteral("待复核单元格")), QString::number(missing + anomalyCount), label(QStringLiteral("missing + anomaly candidates"), QStringLiteral("缺失 + 异常候选")));
        html << QStringLiteral("<div class='card'><b>%1</b><div class='num'>%2</div><span class='muted'>%3</span></div>")
                    .arg(label(QStringLiteral("Recommended views"), QStringLiteral("推荐视图")), QString::number(recommendedViews.size()), label(QStringLiteral("chart options detected"), QStringLiteral("可用图表方案")));
        html << QStringLiteral("<div class='card'><b>%1</b><div class='num'>%2</div><span class='muted'>%3</span></div>")
                    .arg(label(QStringLiteral("Model status"), QStringLiteral("模型状态")), statusText(localAi.value("status").toString("disabled")), localAi.value("model").toString("qwen3-4b"));
        html << QStringLiteral("</div>");

        QJsonArray decisionFindings = decision.value("findings").toArray();
        if (!decisionFindings.isEmpty()) {
            html << QStringLiteral("<h2>%1</h2>").arg(label(QStringLiteral("Decision brief"), QStringLiteral("决策简报")));
            for (int i = 0; i < decisionFindings.size() && i < 5; ++i) {
                QJsonObject finding = decisionFindings.at(i).toObject();
                html << QStringLiteral("<div class='item'><b>%1</b><p>%2</p><span class='muted'>%3</span><p>%4</p></div>")
                            .arg(DecisionTitleText(finding).toHtmlEscaped())
                            .arg(DecisionExplanationText(finding).toHtmlEscaped())
                            .arg(DecisionEvidenceText(finding).toHtmlEscaped())
                            .arg(DecisionActionText(finding).toHtmlEscaped());
            }
        }

        QJsonObject segment = business.value("segment_summary").toObject();
        QJsonArray drivers = business.value("driver_candidates").toArray();
        QJsonArray priorities = business.value("review_priorities").toArray();
        if (!business.isEmpty()) {
            html << QStringLiteral("<h2>%1</h2>").arg(label(QStringLiteral("Business reading"), QStringLiteral("业务解读")));
            if (!segment.isEmpty()) {
                QJsonArray topSegments = segment.value("top_segments").toArray();
                if (!topSegments.isEmpty()) {
                    QJsonObject top = topSegments.first().toObject();
                    QString segmentText = useChinese
                        ? QStringLiteral("%1 在 %2 中排名第一，贡献 %3%，总量为 %4。")
                            .arg(top.value("segment").toString(), segment.value("measure").toString())
                            .arg(top.value("share").toDouble())
                            .arg(top.value("value").toDouble())
                        : QStringLiteral("%1 ranks first for %2, contributing %3% with a total of %4.")
                            .arg(top.value("segment").toString(), segment.value("measure").toString())
                            .arg(top.value("share").toDouble())
                            .arg(top.value("value").toDouble());
                    html << QStringLiteral("<div class='action'><b>%1</b><p>%2</p></div>")
                                .arg(label(QStringLiteral("Segment signal"), QStringLiteral("分组信号")).toHtmlEscaped())
                                .arg(segmentText.toHtmlEscaped());
                }
            }
            if (!drivers.isEmpty()) {
                html << QStringLiteral("<ul>");
                for (int i = 0; i < drivers.size() && i < 3; ++i) {
                    QJsonObject driver = drivers.at(i).toObject();
                    QString text;
                    if (driver.value("type").toString() == "trend") {
                        text = useChinese
                            ? QStringLiteral("%1 的趋势方向为%2，斜率为 %3。")
                                .arg(driver.value("metric").toString(), DirectionText(driver.value("direction").toString()))
                                .arg(driver.value("slope").toDouble())
                            : QStringLiteral("%1 is moving %2 with slope %3.")
                                .arg(driver.value("metric").toString(), driver.value("direction").toString())
                                .arg(driver.value("slope").toDouble());
                    } else {
                        text = useChinese
                            ? QStringLiteral("%1 与 %2 的相关系数为 %3。")
                                .arg(driver.value("metric").toString(), driver.value("with").toString())
                                .arg(driver.value("correlation").toDouble())
                            : QStringLiteral("%1 correlates with %2 at %3.")
                                .arg(driver.value("metric").toString(), driver.value("with").toString())
                                .arg(driver.value("correlation").toDouble());
                    }
                    html << QStringLiteral("<li>%1</li>").arg(text.toHtmlEscaped());
                }
                html << QStringLiteral("</ul>");
            }
            if (!priorities.isEmpty()) {
                QJsonObject priority = priorities.first().toObject();
                QString priorityLabel = priority.value("title").toString();
                if (useChinese) {
                    const QString type = priority.value("type").toString();
                    if (type == "missing") priorityLabel = QStringLiteral("缺失值");
                    else if (type == "duplicate") priorityLabel = QStringLiteral("重复行");
                    else if (type == "anomaly") priorityLabel = QStringLiteral("异常候选");
                    else if (type == "sample") priorityLabel = QStringLiteral("样本量偏小");
                    else if (type == "ready") priorityLabel = QStringLiteral("暂无阻塞问题");
                }
                QString priorityText = useChinese
                    ? QStringLiteral("当前最优先复核项：%1，数量 %2。")
                        .arg(priorityLabel)
                        .arg(priority.value("count").toInt())
                    : QStringLiteral("Top review priority: %1, count %2.")
                        .arg(priority.value("title").toString())
                        .arg(priority.value("count").toInt());
                html << QStringLiteral("<p class='muted'>%1</p>").arg(priorityText.toHtmlEscaped());
            }
        }

        if (!insightCards.isEmpty()) {
            html << QStringLiteral("<h2>%1</h2>").arg(label(QStringLiteral("Supporting evidence"), QStringLiteral("支撑证据")));
            for (int i = 0; i < insightCards.size() && i < 5; ++i) {
                QJsonObject card = insightCards.at(i).toObject();
                html << QStringLiteral("<div class='item'><b>%1</b><p>%2</p><span class='muted'>%3</span></div>")
                            .arg(CardTitleText(card).toHtmlEscaped())
                            .arg(CardSummaryText(card).toHtmlEscaped())
                            .arg(CardEvidenceText(card).toHtmlEscaped());
            }
        }

        if (!repairPlan.isEmpty()) {
            html << QStringLiteral("<h2>%1</h2>").arg(label(QStringLiteral("Clean-up plan"), QStringLiteral("清洗建议")));
            for (int i = 0; i < repairPlan.size() && i < 4; ++i) {
                QJsonObject item = repairPlan.at(i).toObject();
                html << QStringLiteral("<div class='action'><b>%1</b><p>%2</p><span class='muted'>%3</span></div>")
                            .arg(RepairTitleText(item).toHtmlEscaped())
                            .arg(RepairRecommendationText(item).toHtmlEscaped())
                            .arg(RepairImpactText(item).toHtmlEscaped());
            }
        }

        if (!recommendedViews.isEmpty()) {
            html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Best next charts"), QStringLiteral("下一步最适合看的图")));
            for (int i = 0; i < recommendedViews.size() && i < 5; ++i) {
                QJsonObject view = recommendedViews.at(i).toObject();
                html << QStringLiteral("<li><b>%1</b>：%2</li>")
                            .arg(ViewLabelText(view).toHtmlEscaped())
                            .arg(ViewReasonText(view).toHtmlEscaped());
            }
            html << QStringLiteral("</ul>");
        }

        html << QStringLiteral("<h2>%1</h2><div class='hero info'><p>%2</p></div>")
                    .arg(label(QStringLiteral("Local model"), QStringLiteral("本地模型")))
                    .arg(localAiStatusText(localAi).toHtmlEscaped());
        if (!localAi.value("summary").toString().isEmpty()) {
            QString summary = localAi.value("summary").toString();
            if (useChinese) {
                summary = QStringLiteral("本地模型已基于结构化证据生成补充说明；如需查看模型原文，可切换到英文界面。");
            }
            html << QStringLiteral("<p>%1</p>").arg(summary.toHtmlEscaped());
        }

        QJsonArray limitations = decision.value("limitations").toArray();
        if (!limitations.isEmpty()) {
            html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Reading limits"), QStringLiteral("解读边界")));
            for (const QJsonValue &value : limitations) {
                html << QStringLiteral("<li>%1</li>").arg(LimitationText(value.toString()).toHtmlEscaped());
            }
            html << QStringLiteral("</ul>");
        }

        html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Recommended workflow"), QStringLiteral("推荐工作流")));
        if (missing > 0 || anomalyCount > 0) {
            html << QStringLiteral("<li>%1</li>").arg(label(QStringLiteral("Use Clean export first, then compare the repaired file with the original."), QStringLiteral("先使用“清洗导出”，再把清洗后的文件和原始文件对比。")));
        }
        html << QStringLiteral("<li>%1</li>").arg(label(QStringLiteral("Open Chart Studio and start with the automatically recommended view."), QStringLiteral("打开图表工作台，先看系统自动推荐的图。")));
        html << QStringLiteral("<li>%1</li>").arg(label(QStringLiteral("If the result is for reporting, export the analysis brief and the cleaned dataset together."), QStringLiteral("如果要汇报，建议同时导出分析简报和清洗后的数据。")));
        html << QStringLiteral("</ul>");

        QString sourceText = useChinese
            ? QStringLiteral("来源：%1；工作表：%2；编码：%3；分隔符：%4")
                .arg(source.value("parser").toString("-"), source.value("sheet_name").toString("-"), source.value("encoding").toString("-"), source.value("delimiter").toString("-"))
            : QStringLiteral("Source: %1; sheet: %2; encoding: %3; delimiter: %4")
                .arg(source.value("parser").toString("-"), source.value("sheet_name").toString("-"), source.value("encoding").toString("-"), source.value("delimiter").toString("-"));
        html << QStringLiteral("<p class='muted'>%1</p>").arg(sourceText.toHtmlEscaped());
        return html.join(QString());
    }

    auto pill = [](const QString &label, const QString &value) {
        return QStringLiteral("<span class='pill'><b>%1</b> %2</span>")
            .arg(label.toHtmlEscaped())
            .arg(value.toHtmlEscaped());
    };
    auto label = [this](const QString &en, const QString &zh) {
        return Text(en, zh);
    };

    QStringList html;
    html << QStringLiteral(
        "<style>"
        "body{font-family:'SF Pro Text','Segoe UI','Microsoft YaHei',Arial;color:#1d1d1f;background:#fbfbfd;}"
        "h1{font-size:22px;margin:0 0 4px 0;color:#1d1d1f;font-weight:650;}"
        "h2{font-size:15px;margin:18px 0 8px 0;color:#141713;}"
        "p{line-height:1.45;margin:4px 0 8px 0;}"
        ".muted{color:#6e6e73;}"
        ".grid{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin:12px 0;}"
        ".card{border:1px solid #e5e5ea;background:#ffffff;border-radius:12px;padding:10px;}"
        ".value{font-size:20px;font-weight:650;color:#007aff;}"
        ".pill{display:inline-block;border:1px solid #e5e5ea;border-radius:999px;padding:4px 8px;margin:3px;background:#f5f5f7;}"
        ".callout{border:1px solid #d2e7ff;background:#f5faff;border-radius:12px;padding:10px 12px;margin:10px 0;}"
        ".warn{border-color:#ffd6a5;background:#fff8ec;}"
        ".ok{border-color:#b7ebc6;background:#f3fff6;}"
        ".step{border:1px solid #ececec;background:#ffffff;margin:8px 0;padding:9px 11px;border-radius:12px;}"
        ".cardTitle{font-weight:650;margin-bottom:4px;}"
        ".small{font-size:12px;color:#6e6e73;}"
        "li{margin:4px 0;}"
        "code{background:#f5f5f7;padding:2px 4px;border-radius:5px;}"
        "</style>"
    );

    html << QStringLiteral("<h1>%1</h1>").arg(label(QStringLiteral("TablePilot Report"), QStringLiteral("TablePilot 分析报告")));
    html << QStringLiteral("<p class='muted'>%1 <b>%2</b></p>")
                .arg(label(QStringLiteral("Prepared for"), QStringLiteral("分析对象：")))
                .arg(dataset.value("filename").toString().toHtmlEscaped());
    html << QStringLiteral("<div>")
                + pill(label(QStringLiteral("Rows"), QStringLiteral("行数")), QString::number(dataset.value("rows").toInt()))
                + pill(label(QStringLiteral("Columns"), QStringLiteral("列数")), QString::number(dataset.value("columns").toInt()))
                + pill(label(QStringLiteral("Quality"), QStringLiteral("质量")), QStringLiteral("%1/100").arg(quality.value("score").toInt()))
                + pill(label(QStringLiteral("Confidence"), QStringLiteral("置信度")), QualityLevelText(quality.value("level").toString()))
                + pill(label(QStringLiteral("Messy score"), QStringLiteral("复杂度")), QStringLiteral("%1/100").arg(diagnostics.value("messy_score").toInt()))
                + QStringLiteral("</div>");

    if (!brief.isEmpty()) {
        QString headline = brief.value("headline").toString();
        if (useChinese) {
            headline = QStringLiteral("已加载 %1 行、%2 列，检测到 %3 个可分析字段。数据质量为%4，评分 %5/100。")
                .arg(dataset.value("rows").toInt())
                .arg(dataset.value("columns").toInt())
                .arg(dataset.value("numeric_columns").toInt() + dataset.value("date_columns").toInt() + dataset.value("category_columns").toInt())
                .arg(QualityLevelText(quality.value("level").toString()))
                .arg(quality.value("score").toInt());
        }
        html << QStringLiteral("<div class='callout'><b>%1</b><br>%2<br><span class='muted'>%3: %4</span></div>")
                    .arg(label(QStringLiteral("Executive Summary"), QStringLiteral("执行摘要")))
                    .arg(headline.toHtmlEscaped())
                    .arg(label(QStringLiteral("Confidence"), QStringLiteral("置信度")))
                    .arg(QualityLevelText(brief.value("confidence").toString()).toHtmlEscaped());
        QJsonArray watchouts = brief.value("watchouts").toArray();
        if (!watchouts.isEmpty()) {
            html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Quality Notes"), QStringLiteral("质量说明")));
            for (const QJsonValue &value : watchouts) {
                QString watchout = value.toString();
                if (useChinese && watchout == "No major structural data quality warning was detected.") {
                    watchout = QStringLiteral("未检测到明显的结构性数据质量风险。");
                }
                html << QStringLiteral("<li>%1</li>").arg(watchout.toHtmlEscaped());
            }
            html << QStringLiteral("</ul>");
        }
    }

    if (!fingerprint.isEmpty()) {
        QString fingerprintTitle = useChinese
            ? QStringLiteral("数据类型：%1").arg(fingerprint.value("label").toString() == "Sales or operations table" ? QStringLiteral("销售/运营表") : fingerprint.value("label").toString())
            : QStringLiteral("Dataset type: %1").arg(fingerprint.value("label").toString());
        QString sourceLine = useChinese
            ? QStringLiteral("解析方式：%1；工作表：%2；编码：%3；分隔符：%4")
                .arg(source.value("parser").toString("-"))
                .arg(source.value("sheet_name").toString("-"))
                .arg(source.value("encoding").toString("-"))
                .arg(source.value("delimiter").toString("-"))
            : QStringLiteral("Parser: %1; sheet: %2; encoding: %3; delimiter: %4")
                .arg(source.value("parser").toString("-"))
                .arg(source.value("sheet_name").toString("-"))
                .arg(source.value("encoding").toString("-"))
                .arg(source.value("delimiter").toString("-"));
        html << QStringLiteral("<div class='callout ok'><b>%1</b><br><span class='muted'>%2</span></div>")
                    .arg(fingerprintTitle.toHtmlEscaped())
                    .arg(sourceLine.toHtmlEscaped());
    }

    if (!insightCards.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2><div class='grid'>").arg(label(QStringLiteral("Insight Cards"), QStringLiteral("洞察卡片")));
        for (int i = 0; i < insightCards.size() && i < 6; ++i) {
            QJsonObject card = insightCards.at(i).toObject();
            html << QStringLiteral("<div class='card'><div class='cardTitle'>%1</div><p>%2</p><div class='small'>%3</div></div>")
                        .arg(CardTitleText(card).toHtmlEscaped())
                        .arg(CardSummaryText(card).toHtmlEscaped())
                        .arg(card.value("evidence").toString().toHtmlEscaped());
        }
        html << QStringLiteral("</div>");
    }

    if (!repairPlan.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2>").arg(label(QStringLiteral("Data Quality Repair Plan"), QStringLiteral("数据质量修复计划")));
        for (int i = 0; i < repairPlan.size() && i < 5; ++i) {
            QJsonObject item = repairPlan.at(i).toObject();
            QString klass = item.value("severity").toString() == "info" ? QStringLiteral("callout ok") : QStringLiteral("callout warn");
            html << QStringLiteral("<div class='%1'><b>%2</b><br>%3<br><span class='muted'>%4</span></div>")
                        .arg(klass)
                        .arg(RepairTitleText(item).toHtmlEscaped())
                        .arg(RepairRecommendationText(item).toHtmlEscaped())
                        .arg(item.value("impact").toString().toHtmlEscaped());
        }
    }

    if (!recommendedViews.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Recommended Views"), QStringLiteral("推荐视图")));
        for (int i = 0; i < recommendedViews.size() && i < 5; ++i) {
            QJsonObject view = recommendedViews.at(i).toObject();
            html << QStringLiteral("<li><b>%1</b> <span class='muted'>%2: %3 → %4</span></li>")
                        .arg(ViewLabelText(view).toHtmlEscaped())
                        .arg(view.value("chart_type").toString().toHtmlEscaped())
                        .arg(view.value("x").toString("-").toHtmlEscaped())
                        .arg(view.value("y").toString("-").toHtmlEscaped());
        }
        html << QStringLiteral("</ul>");
    }

    if (!localAi.isEmpty()) {
        QString status = localAi.value("status").toString();
        QString summary = localAi.value("summary").toString();
        html << QStringLiteral("<h2>%1</h2><div class='callout'><b>%2</b><br><span class='muted'>%3</span>%4</div>")
                    .arg(label(QStringLiteral("Local AI Layer"), QStringLiteral("本地 AI 层")))
                    .arg(label(QStringLiteral("Local AI status"), QStringLiteral("本地 AI 状态")).toHtmlEscaped() + QStringLiteral(": ") + status.toHtmlEscaped())
                    .arg(localAi.value("guardrail").toString().toHtmlEscaped())
                    .arg(summary.isEmpty() ? QString() : QStringLiteral("<p>%1</p>").arg(summary.toHtmlEscaped()));
    }

    if (!plan.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2>").arg(label(QStringLiteral("Recommended Workflow"), QStringLiteral("推荐分析流程")));
        QString story = plan.value("dataset_story").toString();
        if (useChinese) {
            story = QStringLiteral("这份数据包含可分析的字段结构，适合先做质量复核，再做趋势、分组和关系分析。");
        }
        html << QStringLiteral("<div class='callout'><b>%1</b><br><span class='muted'>%2: %3</span></div>")
                    .arg(story.toHtmlEscaped())
                    .arg(label(QStringLiteral("Planner confidence"), QStringLiteral("规划置信度")))
                    .arg(QualityLevelText(plan.value("confidence").toString()).toHtmlEscaped());
        QJsonArray steps = plan.value("steps").toArray();
        for (int i = 0; i < steps.size() && i < 4; ++i) {
            QJsonObject step = steps.at(i).toObject();
            html << QStringLiteral("<div class='step'><b>%1</b><br><span class='muted'>%2</span></div>")
                        .arg(PlanTitle(step).toHtmlEscaped())
                        .arg(PlanReason(step).toHtmlEscaped());
        }
    }

    html << QStringLiteral("<div class='grid'>");
    html << QStringLiteral("<div class='card'><div class='muted'>%1</div><div class='value'>%2</div></div>")
                .arg(label(QStringLiteral("Numeric fields"), QStringLiteral("数值字段")))
                .arg(dataset.value("numeric_columns").toInt());
    html << QStringLiteral("<div class='card'><div class='muted'>%1</div><div class='value'>%2</div></div>")
                .arg(label(QStringLiteral("Date fields"), QStringLiteral("日期字段")))
                .arg(dataset.value("date_columns").toInt());
    html << QStringLiteral("<div class='card'><div class='muted'>%1</div><div class='value'>%2</div></div>")
                .arg(label(QStringLiteral("Missing cells"), QStringLiteral("缺失单元格")))
                .arg(dataset.value("missing_cells").toInt());
    html << QStringLiteral("<div class='card'><div class='muted'>%1</div><div class='value'>%2</div></div>")
                .arg(label(QStringLiteral("Anomalies"), QStringLiteral("异常候选")))
                .arg(quality.value("anomaly_count").toInt());
    html << QStringLiteral("</div>");

    if (!recommendations.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Action Plan"), QStringLiteral("可执行建议")));
        for (int i = 0; i < recommendations.size() && i < 4; ++i) {
            QJsonObject item = recommendations.at(i).toObject();
            html << QStringLiteral("<li><b>%1</b><br><span class='muted'>%2</span></li>")
                        .arg(RecommendationTitle(item).toHtmlEscaped())
                        .arg(RecommendationReason(item).toHtmlEscaped());
        }
        html << QStringLiteral("</ul>");
    }

    if (!charts.isEmpty()) {
        QJsonObject chart = charts.first().toObject();
        html << QStringLiteral("<h2>%1</h2>").arg(label(QStringLiteral("Suggested Chart"), QStringLiteral("推荐图表")));
        html << QStringLiteral("<div class='callout'><b>%1</b>: x=%2, y=%3<br><span class='muted'>%4</span></div>")
                    .arg(label(chart.value("chart_type").toString(), chart.value("chart_type").toString() == "line" ? QStringLiteral("折线图") : chart.value("chart_type").toString()).toHtmlEscaped())
                    .arg(chart.value("x").toString("-").toHtmlEscaped())
                    .arg(chart.value("y").toString("-").toHtmlEscaped())
                    .arg((useChinese ? QStringLiteral("适合展示指标随时间或行序的变化。") : chart.value("reason").toString()).toHtmlEscaped());
    }

    if (!schema.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Data Structure"), QStringLiteral("数据结构")));
        for (int i = 0; i < schema.size() && i < 8; ++i) {
            QJsonObject item = schema.at(i).toObject();
            html << QStringLiteral("<li><b>%1</b> %2 <span class='muted'>%3</span></li>")
                        .arg(item.value("name").toString().toHtmlEscaped())
                        .arg(pill(label(QStringLiteral("type"), QStringLiteral("类型")), SemanticTypeText(item.value("semantic_type").toString())))
                        .arg(RoleHintText(item.value("role_hint").toString()).toHtmlEscaped());
        }
        html << QStringLiteral("</ul>");
    }

    if (!insights.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Key Evidence"), QStringLiteral("关键证据")));
        for (const QJsonValue &value : insights) {
            html << QStringLiteral("<li>%1</li>").arg(InsightText(value.toString(), root).toHtmlEscaped());
        }
        html << QStringLiteral("</ul>");
    }

    if (!anomalies.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Review Queue"), QStringLiteral("复核队列")));
        for (int i = 0; i < anomalies.size() && i < 5; ++i) {
            QJsonObject item = anomalies.at(i).toObject();
            html << (useChinese
                ? QStringLiteral("<li>第 %1 行，字段 <b>%2</b>，数值 %3，z-score %4</li>")
                : QStringLiteral("<li>Row %1, field <b>%2</b>, value %3, z-score %4</li>"))
                    .arg(item.value("row").toInt() + 1)
                    .arg(item.value("column").toString().toHtmlEscaped())
                    .arg(item.value("value").toDouble())
                    .arg(item.value("z_score").toDouble());
        }
        html << QStringLiteral("</ul>");
    }

    html << QStringLiteral("<p class='muted'>%1</p>").arg(label(QStringLiteral("This is an analytical summary, not a business recommendation."), QStringLiteral("这是分析摘要，不构成业务建议。")));
    return html.join(QString());
}

int MainWindow::ColumnIndexByName(const QString &name) const
{
    for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
        if (ColumnLabel(column) == name) {
            return column;
        }
    }
    return -1;
}

QList<int> MainWindow::CategoryTableColumns(int limit) const
{
    QList<int> columns;
    const int rowCount = ui->tableWidget->rowCount();
    const int columnCount = ui->tableWidget->columnCount();
    for (int column = 0; column < columnCount; ++column) {
        int nonEmptyCount = 0;
        int numericCount = 0;
        QSet<QString> uniqueValues;
        for (int row = 0; row < rowCount; ++row) {
            QTableWidgetItem *item = ui->tableWidget->item(row, column);
            if (!item || item->text().trimmed().isEmpty()) {
                continue;
            }
            nonEmptyCount++;
            bool ok = false;
            item->text().toDouble(&ok);
            if (ok) {
                numericCount++;
            }
            uniqueValues.insert(item->text().trimmed());
        }
        if (nonEmptyCount > 0 && numericCount < nonEmptyCount * 0.6 && uniqueValues.size() <= std::max(12, rowCount / 2)) {
            columns << column;
            if (limit > 0 && columns.size() >= limit) {
                break;
            }
        }
    }
    return columns;
}

QList<int> MainWindow::NumericTableColumns(int limit) const
{
    QList<int> columns;
    int rowCount = ui->tableWidget->rowCount();
    int columnCount = ui->tableWidget->columnCount();
    for (int column = 0; column < columnCount; ++column) {
        int numericCount = 0;
        int nonEmptyCount = 0;
        for (int row = 0; row < rowCount; ++row) {
            QTableWidgetItem *item = ui->tableWidget->item(row, column);
            if (!item || item->text().trimmed().isEmpty()) {
                continue;
            }
            nonEmptyCount++;
            bool ok = false;
            item->text().toDouble(&ok);
            if (ok) {
                numericCount++;
            }
        }
        if (nonEmptyCount > 0 && numericCount >= std::max(1, static_cast<int>(nonEmptyCount * 0.8))) {
            columns << column;
            if (limit > 0 && columns.size() >= limit) {
                break;
            }
        }
    }
    return columns;
}

QVector<double> MainWindow::NumericColumnValues(int column) const
{
    return NumericColumnValuesForRows(column, nullptr);
}

QVector<double> MainWindow::NumericColumnValuesForRows(int column, QVector<int> *rows) const
{
    QVector<double> values;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = ui->tableWidget->item(row, column);
        if (!item) {
            continue;
        }
        bool ok = false;
        double value = item->text().toDouble(&ok);
        if (ok) {
            values << value;
            if (rows) {
                rows->append(row);
            }
        }
    }
    return values;
}

QVector<double> MainWindow::NumericRowIndex(int size) const
{
    QVector<double> index;
    for (int i = 0; i < size; ++i) {
        index << i + 1;
    }
    return index;
}

QString MainWindow::ColumnLabel(int column) const
{
    QTableWidgetItem *header = ui->tableWidget->horizontalHeaderItem(column);
    if (header && !header->text().isEmpty()) {
        return header->text();
    }
    return Text(QStringLiteral("Column %1"), QStringLiteral("第 %1 列")).arg(column + 1);
}

void MainWindow::RenderChartStudio()
{
    if (!ui->widget || ui->tableWidget->rowCount() == 0) {
        RenderEmptyChart(ui->widget, Text(QStringLiteral("Open a table to let TablePilot recommend the best chart."), QStringLiteral("打开表格后，TablePilot 会自动推荐最合适的图表。")));
        return;
    }

    QString chartType = chartTypeSelector ? chartTypeSelector->currentData().toString() : QStringLiteral("auto");
    QList<int> numericColumns = NumericTableColumns();
    QList<int> categoryColumns = CategoryTableColumns();
    if (chartType.isEmpty() || chartType == "auto") {
        if (!categoryColumns.isEmpty() && !numericColumns.isEmpty()) {
            chartType = QStringLiteral("grouped_bar");
        } else if (numericColumns.size() >= 3) {
            chartType = QStringLiteral("heatmap");
        } else if (numericColumns.size() >= 2) {
            chartType = QStringLiteral("scatter");
        } else if (numericColumns.size() == 1) {
            chartType = QStringLiteral("trend");
        }
    }

    if (trendChartTitleLabel) {
        trendChartTitleLabel->setText(Text(QStringLiteral("Chart Studio"), QStringLiteral("图表工作台")));
    }
    if (chartType == "grouped_bar") {
        RenderGroupedBarChart();
    } else if (chartType == "scatter") {
        RenderScatterChart();
    } else if (chartType == "heatmap") {
        RenderCorrelationHeatmap();
    } else if (chartType == "box") {
        RenderBoxPlot();
    } else {
        RenderDynamicLineChart();
    }
}

void MainWindow::RenderDynamicLineChart()
{
    QCustomPlot *plot = ui->widget;
    if (!plot || ui->tableWidget->rowCount() == 0) {
        RenderEmptyChart(plot, Text(QStringLiteral("Open a data file to generate recommended charts."), QStringLiteral("打开数据文件后生成推荐图表。")));
        return;
    }
    plot->clearGraphs();
    plot->clearPlottables();
    plot->clearItems();
    StylePlot(plot);
    plot->legend->setVisible(true);
    QList<int> columns = NumericTableColumns(3);
    if (columns.isEmpty()) {
        if (trendChartSubtitleLabel) {
            trendChartSubtitleLabel->setText(Text(QStringLiteral("No numeric metric is available for trend visualization."), QStringLiteral("当前数据没有可用于趋势图的数值指标。")));
        }
        RenderEmptyChart(plot, Text(QStringLiteral("No numeric metric is available for trend visualization."), QStringLiteral("当前数据没有可用于趋势图的数值指标。")));
        return;
    }
    if (trendChartSubtitleLabel) {
        QStringList names;
        for (int column : columns) {
            names << ColumnLabel(column);
        }
        trendChartSubtitleLabel->setText(Text(QStringLiteral("Showing recommended metrics: %1"), QStringLiteral("正在展示推荐指标：%1")).arg(names.join(QStringLiteral(", "))));
    }

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    QVector<QColor> colors = {QColor(22, 119, 255), QColor(0, 150, 136), QColor(255, 152, 0), QColor(156, 39, 176), QColor(76, 175, 80)};
    for (int i = 0; i < columns.size(); ++i) {
        QVector<double> y = NumericColumnValues(columns[i]);
        QVector<double> x = NumericRowIndex(y.size());
        if (y.isEmpty()) {
            continue;
        }
        for (double value : y) {
            minY = std::min(minY, value);
            maxY = std::max(maxY, value);
        }
        plot->addGraph();
        plot->graph(plot->graphCount() - 1)->setData(x, y);
        plot->graph(plot->graphCount() - 1)->setName(ColumnLabel(columns[i]));
        plot->graph(plot->graphCount() - 1)->setPen(QPen(colors[i % colors.size()], 2));
    }
    plot->xAxis->setLabel(Text(QStringLiteral("Record"), QStringLiteral("记录序号")));
    plot->yAxis->setLabel(Text(QStringLiteral("Metric value"), QStringLiteral("指标值")));
    plot->xAxis->setRange(1, std::max(2, ui->tableWidget->rowCount()));
    if (minY <= maxY) {
        double padding = std::max(1.0, (maxY - minY) * 0.1);
        plot->yAxis->setRange(minY - padding, maxY + padding);
    }
    plot->replot();
}

void MainWindow::RenderGroupedBarChart()
{
    QCustomPlot *plot = ui->widget;
    QList<int> numericColumns = NumericTableColumns();
    QList<int> categoryColumns = CategoryTableColumns();
    if (!plot || numericColumns.isEmpty() || categoryColumns.isEmpty()) {
        RenderDynamicLineChart();
        return;
    }

    int metricColumn = chartMetricSelector ? chartMetricSelector->currentData().toInt() : -1;
    if (metricColumn < 0 || !numericColumns.contains(metricColumn)) {
        metricColumn = numericColumns.first();
    }
    int dimensionColumn = chartDimensionSelector ? chartDimensionSelector->currentData().toInt() : -1;
    if (dimensionColumn < 0 || !categoryColumns.contains(dimensionColumn)) {
        dimensionColumn = categoryColumns.first();
    }

    QMap<QString, QVector<double>> groups;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *dimensionItem = ui->tableWidget->item(row, dimensionColumn);
        QTableWidgetItem *metricItem = ui->tableWidget->item(row, metricColumn);
        if (!dimensionItem || !metricItem || dimensionItem->text().trimmed().isEmpty()) {
            continue;
        }
        bool ok = false;
        double value = metricItem->text().toDouble(&ok);
        if (ok) {
            groups[dimensionItem->text().trimmed()].append(value);
        }
    }
    if (groups.isEmpty()) {
        RenderEmptyChart(plot, Text(QStringLiteral("No category and metric pair can be charted."), QStringLiteral("没有可绘制的分组和指标组合。")));
        return;
    }

    plot->clearPlottables();
    plot->clearGraphs();
    plot->clearItems();
    StylePlot(plot);
    plot->legend->setVisible(false);

    QVector<double> ticks;
    QVector<double> values;
    QStringList labels;
    int index = 1;
    for (auto it = groups.constBegin(); it != groups.constEnd() && index <= 10; ++it, ++index) {
        double sum = std::accumulate(it.value().begin(), it.value().end(), 0.0);
        ticks << index;
        values << sum / std::max(1, static_cast<int>(it.value().size()));
        labels << it.key();
    }

    QCPBars *bars = new QCPBars(plot->xAxis, plot->yAxis);
    bars->setData(ticks, values);
    bars->setPen(QPen(QColor(0, 122, 255)));
    bars->setBrush(QColor(0, 122, 255, 95));
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    for (int i = 0; i < labels.size(); ++i) {
        textTicker->addTick(i + 1, labels.at(i).left(12));
    }
    plot->xAxis->setTicker(textTicker);
    plot->xAxis->setRange(0, ticks.size() + 1);
    auto minmax = std::minmax_element(values.begin(), values.end());
    if (minmax.first != values.end()) {
        double padding = std::max(1.0, (*minmax.second - *minmax.first) * 0.12);
        plot->yAxis->setRange(std::min(0.0, *minmax.first - padding), *minmax.second + padding);
    }
    plot->xAxis->setLabel(ColumnLabel(dimensionColumn));
    plot->yAxis->setLabel(ColumnLabel(metricColumn));
    if (trendChartSubtitleLabel) {
        trendChartSubtitleLabel->setText(Text(QStringLiteral("Average %1 grouped by %2."), QStringLiteral("按 %2 分组查看 %1 的平均值。")).arg(ColumnLabel(metricColumn), ColumnLabel(dimensionColumn)));
    }
    plot->replot();
}

void MainWindow::RenderScatterChart()
{
    QCustomPlot *plot = ui->widget;
    QList<int> numericColumns = NumericTableColumns();
    if (!plot || numericColumns.size() < 2) {
        RenderEmptyChart(plot, Text(QStringLiteral("Scatter needs at least two numeric fields."), QStringLiteral("散点图至少需要两个数值字段。")));
        return;
    }
    int yColumn = chartMetricSelector ? chartMetricSelector->currentData().toInt() : -1;
    if (yColumn < 0 || !numericColumns.contains(yColumn)) {
        yColumn = numericColumns.at(1);
    }
    int xColumn = chartDimensionSelector ? chartDimensionSelector->currentData().toInt() : -1;
    if (xColumn < 0 || !numericColumns.contains(xColumn) || xColumn == yColumn) {
        xColumn = numericColumns.first() == yColumn ? numericColumns.at(1) : numericColumns.first();
    }

    QVector<double> x;
    QVector<double> y;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *xItem = ui->tableWidget->item(row, xColumn);
        QTableWidgetItem *yItem = ui->tableWidget->item(row, yColumn);
        if (!xItem || !yItem) {
            continue;
        }
        bool xOk = false;
        bool yOk = false;
        double xValue = xItem->text().toDouble(&xOk);
        double yValue = yItem->text().toDouble(&yOk);
        if (xOk && yOk) {
            x << xValue;
            y << yValue;
        }
    }
    if (x.isEmpty()) {
        RenderEmptyChart(plot, Text(QStringLiteral("The selected fields do not overlap enough for scatter analysis."), QStringLiteral("所选字段没有足够的重叠数值，无法绘制散点图。")));
        return;
    }

    plot->clearPlottables();
    plot->clearGraphs();
    plot->clearItems();
    StylePlot(plot);
    plot->legend->setVisible(false);
    plot->addGraph();
    plot->graph(0)->setData(x, y);
    plot->graph(0)->setLineStyle(QCPGraph::lsNone);
    plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QColor(0, 122, 255), QColor(0, 122, 255, 80), 7));
    plot->xAxis->setLabel(ColumnLabel(xColumn));
    plot->yAxis->setLabel(ColumnLabel(yColumn));
    plot->rescaleAxes();
    if (trendChartSubtitleLabel) {
        trendChartSubtitleLabel->setText(Text(QStringLiteral("Relationship view: %1 vs %2."), QStringLiteral("关系视图：%1 与 %2。")).arg(ColumnLabel(xColumn), ColumnLabel(yColumn)));
    }
    plot->replot();
}

void MainWindow::RenderCorrelationHeatmap()
{
    QCustomPlot *plot = ui->widget;
    QList<int> numericColumns = NumericTableColumns(6);
    if (!plot || numericColumns.size() < 2) {
        RenderEmptyChart(plot, Text(QStringLiteral("Correlation heatmap needs at least two numeric fields."), QStringLiteral("相关热力图至少需要两个数值字段。")));
        return;
    }
    plot->clearPlottables();
    plot->clearGraphs();
    plot->clearItems();
    StylePlot(plot);
    plot->legend->setVisible(false);

    const int n = numericColumns.size();
    QCPColorMap *map = new QCPColorMap(plot->xAxis, plot->yAxis);
    map->data()->setSize(n, n);
    map->data()->setRange(QCPRange(0, n), QCPRange(0, n));
    for (int i = 0; i < n; ++i) {
        QVector<double> a = NumericColumnValues(numericColumns[i]);
        double meanA = a.isEmpty() ? 0.0 : std::accumulate(a.begin(), a.end(), 0.0) / a.size();
        for (int j = 0; j < n; ++j) {
            QVector<double> b = NumericColumnValues(numericColumns[j]);
            int size = std::min(a.size(), b.size());
            double meanB = b.isEmpty() ? 0.0 : std::accumulate(b.begin(), b.end(), 0.0) / b.size();
            double numerator = 0.0;
            double denomA = 0.0;
            double denomB = 0.0;
            for (int k = 0; k < size; ++k) {
                double da = a[k] - meanA;
                double db = b[k] - meanB;
                numerator += da * db;
                denomA += da * da;
                denomB += db * db;
            }
            double corr = (denomA > 0 && denomB > 0) ? numerator / std::sqrt(denomA * denomB) : 0.0;
            map->data()->setCell(i, j, corr);
        }
    }
    QCPColorGradient gradient;
    gradient.setColorStopAt(0.0, QColor(44, 123, 182));
    gradient.setColorStopAt(0.5, QColor(247, 247, 247));
    gradient.setColorStopAt(1.0, QColor(215, 25, 28));
    map->setGradient(gradient);
    map->setDataRange(QCPRange(-1, 1));

    QSharedPointer<QCPAxisTickerText> xTicker(new QCPAxisTickerText);
    QSharedPointer<QCPAxisTickerText> yTicker(new QCPAxisTickerText);
    for (int i = 0; i < n; ++i) {
        QString label = ColumnLabel(numericColumns[i]).left(10);
        xTicker->addTick(i + 0.5, label);
        yTicker->addTick(i + 0.5, label);
    }
    plot->xAxis->setTicker(xTicker);
    plot->yAxis->setTicker(yTicker);
    plot->xAxis->setRange(0, n);
    plot->yAxis->setRange(0, n);
    plot->xAxis->setLabel(Text(QStringLiteral("Fields"), QStringLiteral("字段")));
    plot->yAxis->setLabel(Text(QStringLiteral("Fields"), QStringLiteral("字段")));
    if (trendChartSubtitleLabel) {
        trendChartSubtitleLabel->setText(Text(QStringLiteral("Correlation strength across numeric fields."), QStringLiteral("数值字段之间的相关性强弱。")));
    }
    plot->replot();
}

void MainWindow::RenderBoxPlot()
{
    QCustomPlot *plot = ui->widget;
    QList<int> numericColumns = NumericTableColumns(5);
    if (!plot || numericColumns.isEmpty()) {
        RenderEmptyChart(plot, Text(QStringLiteral("Box plot needs numeric fields."), QStringLiteral("箱线图需要数值字段。")));
        return;
    }
    plot->clearPlottables();
    plot->clearGraphs();
    plot->clearItems();
    StylePlot(plot);
    plot->legend->setVisible(false);

    QVector<double> keys;
    QVector<double> minimum;
    QVector<double> lower;
    QVector<double> median;
    QVector<double> upper;
    QVector<double> maximum;
    QStringList labels;
    auto percentile = [](const QVector<double> &sorted, double p) {
        if (sorted.isEmpty()) {
            return 0.0;
        }
        double pos = (sorted.size() - 1) * p;
        int left = static_cast<int>(std::floor(pos));
        int right = static_cast<int>(std::ceil(pos));
        if (left == right) {
            return sorted[left];
        }
        return sorted[left] + (sorted[right] - sorted[left]) * (pos - left);
    };
    for (int i = 0; i < numericColumns.size(); ++i) {
        QVector<double> values = NumericColumnValues(numericColumns[i]);
        std::sort(values.begin(), values.end());
        if (values.isEmpty()) {
            continue;
        }
        keys << i + 1;
        minimum << values.first();
        lower << percentile(values, 0.25);
        median << percentile(values, 0.5);
        upper << percentile(values, 0.75);
        maximum << values.last();
        labels << ColumnLabel(numericColumns[i]).left(12);
    }
    QCPStatisticalBox *box = new QCPStatisticalBox(plot->xAxis, plot->yAxis);
    box->setData(keys, minimum, lower, median, upper, maximum);
    box->setBrush(QColor(0, 122, 255, 45));
    box->setPen(QPen(QColor(0, 122, 255), 2));
    box->setMedianPen(QPen(QColor(255, 149, 0), 2));
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    for (int i = 0; i < labels.size(); ++i) {
        textTicker->addTick(i + 1, labels.at(i));
    }
    plot->xAxis->setTicker(textTicker);
    plot->xAxis->setRange(0, labels.size() + 1);
    plot->rescaleAxes();
    if (trendChartSubtitleLabel) {
        trendChartSubtitleLabel->setText(Text(QStringLiteral("Spread and outlier review across key metrics."), QStringLiteral("查看关键指标的分布范围和异常可能性。")));
    }
    plot->replot();
}

void MainWindow::RenderDynamicBarChart()
{
    QCustomPlot *plot = ui->widget_2;
    if (!plot || ui->tableWidget->rowCount() == 0) {
        RenderEmptyChart(plot, Text(QStringLiteral("Open a data file to review metric distribution."), QStringLiteral("打开数据文件后查看指标分布。")));
        return;
    }
    plot->clearPlottables();
    plot->clearGraphs();
    plot->clearItems();
    StylePlot(plot);

    QList<int> columns;
    int selectedColumn = ui->comboBox->currentData().toInt();
    if (selectedColumn >= 0) {
        columns << selectedColumn;
    } else {
        columns = NumericTableColumns(1);
    }
    if (columns.isEmpty()) {
        if (distributionChartSubtitleLabel) {
            distributionChartSubtitleLabel->setText(Text(QStringLiteral("No numeric metric is available for distribution review."), QStringLiteral("当前数据没有可用于分布图的数值指标。")));
        }
        RenderEmptyChart(plot, Text(QStringLiteral("No numeric metric is available for distribution review."), QStringLiteral("当前数据没有可用于分布图的数值指标。")));
        return;
    }

    QVector<double> values = NumericColumnValues(columns.first());
    if (values.isEmpty()) {
        RenderEmptyChart(plot, Text(QStringLiteral("The selected metric has no numeric values to chart."), QStringLiteral("当前指标没有可绘制的数值。")));
        return;
    }
    QVector<double> ticks = NumericRowIndex(values.size());
    QCPBars *bars = new QCPBars(plot->xAxis, plot->yAxis);
    bars->setData(ticks, values);
    bars->setName(ColumnLabel(columns.first()));
    if (distributionChartSubtitleLabel) {
        distributionChartSubtitleLabel->setText(Text(QStringLiteral("Selected metric: %1"), QStringLiteral("当前指标：%1")).arg(ColumnLabel(columns.first())));
    }
    bars->setPen(QPen(QColor(22, 119, 255)));
    bars->setBrush(QColor(22, 119, 255, 90));

    auto minmax = std::minmax_element(values.begin(), values.end());
    plot->xAxis->setLabel(Text(QStringLiteral("Record"), QStringLiteral("记录序号")));
    plot->yAxis->setLabel(ColumnLabel(columns.first()));
    plot->xAxis->setRange(0, std::max(2, static_cast<int>(values.size()) + 1));
    if (minmax.first != values.end()) {
        double padding = std::max(1.0, (*minmax.second - *minmax.first) * 0.1);
        plot->yAxis->setRange(*minmax.first - padding, *minmax.second + padding);
    }
    plot->replot();
}

void MainWindow::RenderEmptyChart(QCustomPlot *plot, const QString &message)
{
    if (!plot) {
        return;
    }
    plot->clearGraphs();
    plot->clearPlottables();
    plot->clearItems();
    StylePlot(plot);
    plot->legend->setVisible(false);
    plot->xAxis->setVisible(false);
    plot->yAxis->setVisible(false);
    plot->xAxis2->setVisible(false);
    plot->yAxis2->setVisible(false);
    QCPItemText *text = new QCPItemText(plot);
    text->position->setType(QCPItemPosition::ptAxisRectRatio);
    text->position->setCoords(0.5, 0.5);
    text->setPositionAlignment(Qt::AlignCenter);
    text->setTextAlignment(Qt::AlignCenter);
    text->setText(message);
    text->setColor(QColor(99, 99, 102));
    text->setFont(QFont(QStringLiteral("Segoe UI"), 11));
    plot->xAxis->setRange(0, 1);
    plot->yAxis->setRange(0, 1);
    plot->replot();
}

void MainWindow::ExportCleanedDataset()
{
    if (currentFilePath.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("TablePilot"), Text(QStringLiteral("Open a table before exporting cleaned data."), QStringLiteral("请先打开表格，再导出清洗数据。")));
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        this,
        Text(QStringLiteral("Export cleaned dataset"), QStringLiteral("导出清洗后的数据")),
        QStringLiteral("tablepilot-cleaned.csv"),
        Text(QStringLiteral("CSV file (*.csv);;Excel workbook (*.xlsx)"), QStringLiteral("CSV 文件 (*.csv);;Excel 工作簿 (*.xlsx)"))
    );
    if (filename.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(currentFilePath);
    QFile *file = new QFile(currentFilePath);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("TablePilot"), Text(QStringLiteral("Could not read the selected file."), QStringLiteral("无法读取所选文件。")));
        delete file;
        return;
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileInfo.fileName()))
    );
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    QString format = filename.endsWith(QStringLiteral(".xlsx"), Qt::CaseInsensitive) ? QStringLiteral("xlsx") : QStringLiteral("csv");
    QString url = QStringLiteral("http://127.0.0.1:8000/api/clean-upload?format=%1").arg(format);
    if (sheetSelector && sheetSelector->isVisible() && sheetSelector->currentIndex() >= 0) {
        url += QStringLiteral("&sheet=%1").arg(QString::fromUtf8(QUrl::toPercentEncoding(sheetSelector->currentText())));
    }
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(QNetworkRequest(QUrl(url)), multiPart);
    multiPart->setParent(reply);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        QString message = Text(QStringLiteral("Clean export failed: %1"), QStringLiteral("清洗导出失败：%1")).arg(reply->errorString());
        reply->deleteLater();
        QMessageBox::warning(this, QStringLiteral("TablePilot"), message);
        return;
    }

    QByteArray payload = reply->readAll();
    reply->deleteLater();
    QFile output(filename);
    if (!output.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, QStringLiteral("TablePilot"), Text(QStringLiteral("Could not write the export file."), QStringLiteral("无法写入导出文件。")));
        return;
    }
    output.write(payload);
    output.close();
    statusBar()->showMessage(Text(QStringLiteral("Cleaned dataset exported"), QStringLiteral("清洗后的数据已导出")), 5000);
}

void MainWindow::ShowCleanCompare()
{
    if (currentFilePath.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("TablePilot"), Text(QStringLiteral("Open a table before comparing clean-up."), QStringLiteral("请先打开表格，再查看清洗对比。")));
        return;
    }

    QFileInfo fileInfo(currentFilePath);
    QFile *file = new QFile(currentFilePath);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("TablePilot"), Text(QStringLiteral("Could not read the selected file."), QStringLiteral("无法读取所选文件。")));
        delete file;
        return;
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileInfo.fileName()))
    );
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    QString url = QStringLiteral("http://127.0.0.1:8000/api/clean-preview-upload");
    if (sheetSelector && sheetSelector->isVisible() && sheetSelector->currentIndex() >= 0) {
        url += QStringLiteral("?sheet=%1").arg(QString::fromUtf8(QUrl::toPercentEncoding(sheetSelector->currentText())));
    }
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(QNetworkRequest(QUrl(url)), multiPart);
    multiPart->setParent(reply);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        QString message = Text(QStringLiteral("Clean compare failed: %1"), QStringLiteral("清洗对比失败：%1")).arg(reply->errorString());
        reply->deleteLater();
        QMessageBox::warning(this, QStringLiteral("TablePilot"), message);
        return;
    }

    QByteArray payload = reply->readAll();
    reply->deleteLater();
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        QMessageBox::warning(this, QStringLiteral("TablePilot"), Text(QStringLiteral("The clean preview response could not be parsed."), QStringLiteral("清洗预览结果无法解析。")));
        return;
    }
    if (cleanCompareText) {
        cleanCompareText->setHtml(FormatCleanCompareHtml(document.object()));
    }
    if (reviewDock) {
        reviewDock->show();
        reviewDock->raise();
    }
    statusBar()->showMessage(Text(QStringLiteral("Clean comparison ready"), QStringLiteral("清洗前后对比已生成")), 4000);
}

QString MainWindow::FormatCleanCompareHtml(const QJsonObject &root) const
{
    QJsonObject summary = root.value("summary").toObject();
    QJsonObject before = root.value("before").toObject();
    QJsonObject after = root.value("after").toObject();
    auto value = [](const QJsonObject &object, const QString &key) {
        return QString::number(object.value(key).toInt());
    };
    auto previewLine = [](const QJsonObject &preview) {
        QStringList columns;
        for (const QJsonValue &column : preview.value("columns").toArray()) {
            columns << column.toString();
        }
        return columns.mid(0, 8).join(QStringLiteral(", "));
    };

    QStringList html;
    html << QStringLiteral(
        "<style>"
        "body{font-family:'SF Pro Text','Segoe UI','Microsoft YaHei',Arial;color:#1d1d1f;}"
        "h2{font-size:17px;margin:0 0 8px 0;}p{line-height:1.5;}"
        ".grid{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin:10px 0;}"
        ".card{background:#fbfbfd;border:1px solid #e5e5ea;border-radius:12px;padding:10px;}"
        ".num{font-size:22px;font-weight:750;color:#007aff;}.muted{color:#6e6e73;}"
        ".ok{border-left:4px solid #34c759}.warn{border-left:4px solid #ff9f0a}"
        "li{margin:4px 0;}"
        "</style>"
    );
    html << QStringLiteral("<h2>%1</h2>").arg(Text(QStringLiteral("Clean-up comparison"), QStringLiteral("清洗前后对比")));
    html << QStringLiteral("<p class='muted'>%1</p>").arg(Text(
        QStringLiteral("TablePilot uses conservative repairs: remove empty structure and duplicate rows, fill safe missing values, and mark anomaly rows instead of deleting them."),
        QStringLiteral("TablePilot 使用保守清洗：删除空结构和重复行，安全填补缺失值，并标记异常行而不是直接删除。")
    ));
    html << QStringLiteral("<div class='grid'>");
    html << QStringLiteral("<div class='card'><b>%1</b><div class='num'>%2 x %3</div><span class='muted'>%4</span></div>")
                .arg(Text(QStringLiteral("Before"), QStringLiteral("清洗前")))
                .arg(value(summary, QStringLiteral("original_rows")))
                .arg(value(summary, QStringLiteral("original_columns")))
                .arg(previewLine(before).toHtmlEscaped());
    html << QStringLiteral("<div class='card ok'><b>%1</b><div class='num'>%2 x %3</div><span class='muted'>%4</span></div>")
                .arg(Text(QStringLiteral("After"), QStringLiteral("清洗后")))
                .arg(value(summary, QStringLiteral("cleaned_rows")))
                .arg(value(summary, QStringLiteral("cleaned_columns")))
                .arg(previewLine(after).toHtmlEscaped());
    html << QStringLiteral("</div><ul>");
    html << QStringLiteral("<li>%1: %2</li>").arg(Text(QStringLiteral("Removed empty rows"), QStringLiteral("删除空行")), value(summary, QStringLiteral("removed_empty_rows")));
    html << QStringLiteral("<li>%1: %2</li>").arg(Text(QStringLiteral("Removed empty columns"), QStringLiteral("删除空列")), value(summary, QStringLiteral("removed_empty_columns")));
    html << QStringLiteral("<li>%1: %2</li>").arg(Text(QStringLiteral("Removed duplicate rows"), QStringLiteral("删除重复行")), value(summary, QStringLiteral("removed_duplicate_rows")));
    html << QStringLiteral("<li>%1: %2</li>").arg(Text(QStringLiteral("Filled missing cells"), QStringLiteral("填补缺失单元格")), value(summary, QStringLiteral("filled_missing_cells")));
    html << QStringLiteral("<li>%1: %2</li>").arg(Text(QStringLiteral("Marked anomaly rows"), QStringLiteral("标记异常行")), value(summary, QStringLiteral("marked_anomaly_rows")));
    html << QStringLiteral("</ul>");
    html << QStringLiteral("<p class='muted'>%1</p>").arg(Text(
        QStringLiteral("Use Clean export when this comparison matches your expectation."),
        QStringLiteral("如果这个对比符合预期，再使用“清洗导出”。")
    ));
    return html.join(QString());
}

void MainWindow::FocusAnomaly(int anomalyIndex)
{
    QJsonArray anomalies = lastProfile.value("anomalies").toArray();
    if (anomalyIndex < 0 || anomalyIndex >= anomalies.size()) {
        return;
    }
    QJsonObject anomaly = anomalies.at(anomalyIndex).toObject();
    int row = anomaly.value("row").toInt(-1);
    int column = ColumnIndexByName(anomaly.value("column").toString());
    if (row < 0 || row >= ui->tableWidget->rowCount() || column < 0) {
        return;
    }
    ui->tableWidget->setCurrentCell(row, column);
    ui->tableWidget->scrollToItem(ui->tableWidget->item(row, column), QAbstractItemView::PositionAtCenter);
    ui->tableWidget->setFocus();
    statusBar()->showMessage(Text(
        QStringLiteral("Located anomaly candidate at row %1, field %2."),
        QStringLiteral("已定位异常候选：第 %1 行，字段 %2。")
    ).arg(row + 1).arg(ColumnLabel(column)), 5000);
}

void MainWindow::FocusDataQuality()
{
    ui->tableWidget->setFocus();
    ui->groupBox->setFocus();
    statusBar()->showMessage(Text(
        QStringLiteral("Data preview focused. Highlighted cells indicate missing values or anomaly candidates."),
        QStringLiteral("已定位到数据预览。高亮单元格代表缺失值或异常候选。")
    ), 5000);
}

void MainWindow::StylePlot(QCustomPlot *plot)
{
    if (!plot) {
        return;
    }
    plot->setBackground(QColor(255, 255, 255));
    plot->axisRect()->setBackground(QColor(255, 255, 255));
    plot->axisRect()->setAutoMargins(QCP::msAll);
    plot->xAxis->setVisible(true);
    plot->yAxis->setVisible(true);
    plot->xAxis->setTicker(QSharedPointer<QCPAxisTicker>(new QCPAxisTicker));
    plot->yAxis->setTicker(QSharedPointer<QCPAxisTicker>(new QCPAxisTicker));
    plot->xAxis2->setVisible(false);
    plot->yAxis2->setVisible(false);
    plot->xAxis->setBasePen(QPen(QColor(142, 142, 147)));
    plot->yAxis->setBasePen(QPen(QColor(142, 142, 147)));
    plot->xAxis->setTickPen(QPen(QColor(199, 199, 204)));
    plot->yAxis->setTickPen(QPen(QColor(199, 199, 204)));
    plot->xAxis->setSubTickPen(QPen(QColor(229, 229, 234)));
    plot->yAxis->setSubTickPen(QPen(QColor(229, 229, 234)));
    plot->xAxis->grid()->setPen(QPen(QColor(229, 229, 234), 1, Qt::DotLine));
    plot->yAxis->grid()->setPen(QPen(QColor(229, 229, 234), 1, Qt::DotLine));
    plot->xAxis->setTickLabelColor(QColor(99, 99, 102));
    plot->yAxis->setTickLabelColor(QColor(99, 99, 102));
    plot->xAxis->setLabelColor(QColor(58, 58, 60));
    plot->yAxis->setLabelColor(QColor(58, 58, 60));
    plot->legend->setBrush(QBrush(QColor(255, 255, 255, 235)));
    plot->legend->setBorderPen(QPen(QColor(229, 229, 234)));
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}
