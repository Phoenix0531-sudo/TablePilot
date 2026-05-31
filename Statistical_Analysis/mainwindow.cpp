#include "mainwindow.h"   // 包含自定义的MainWindow类的头文件
#include "ui_mainwindow.h" // 包含自动生成的ui界面文件的头文件
#include <QAbstractItemView>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>     // 包含文件对话框类的头文件
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>          // 包含标签类的头文件，用于创建标签部件
#include <QMap>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QStyle>
#include <QEventLoop>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QTimer>
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
    createOverviewPanel();
    createInsightPanel();
    ApplyLanguage();
}

void MainWindow::InitObject(){
    // 初始化对象
    CheckAnalysisService();
}

void MainWindow::createToolBar(){// 创建工具栏

    ui->mainToolBar->setMovable(false); // 设置工具栏不可移动
    ui->mainToolBar->setIconSize(QSize(1, 1)); // 文本化命令栏，避免旧式小图标干扰
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    ui->mainToolBar->addAction(m_pAction1); // 添加打开Excel的动作
    ui->mainToolBar->addAction(m_pAction2); // 添加打开txt的动作
    ui->mainToolBar->addSeparator(); // 添加分隔符
    ui->mainToolBar->addAction(m_pAction3); // 添加数据统计的动作
    ui->mainToolBar->addSeparator(); // 添加分隔符
    ui->mainToolBar->addAction(m_pAction4); // 添加折线图的动作
    ui->mainToolBar->addSeparator(); // 添加分隔符
    ui->mainToolBar->addAction(m_pAction5); // 添加柱状图的动作
    ui->mainToolBar->addSeparator(); // 添加分隔符
    ui->mainToolBar->addAction(m_pAction6); // 添加另存为图片的动作
    ui->mainToolBar->addSeparator(); // 添加分隔符
    ui->mainToolBar->addAction(m_pAction8); // 添加智能分析的动作
    ui->mainToolBar->addSeparator(); // 添加分隔符
    ui->mainToolBar->addAction(m_pAction9); // 添加语言切换动作
    ui->mainToolBar->addSeparator(); // 添加分隔符
    ui->mainToolBar->addAction(m_pAction7); // 添加退出系统的动作
}

void MainWindow::createActions(){// 创建各个功能标签的动作
    //设置功能标签的图标，文字，以及该动作属于哪个父窗口

    m_pAction1 = new QAction(QString("Open Excel"), this);
    m_pAction1->setToolTip("打开 Excel 工作簿并调用本地分析服务");
    m_pAction2 = new QAction(QString("Open Text"), this);
    m_pAction2->setToolTip("打开 TXT/CSV 表格并自动识别分隔符");
    m_pAction3 = new QAction(QString("Profile"), this);
    m_pAction3->setToolTip("刷新数值字段统计画像");
    m_pAction4 = new QAction(QString("Trend"), this);
    m_pAction4->setToolTip("查看 Top 数值字段趋势");
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
            QStringLiteral("选择数据文件"),
            QString(),
            QStringLiteral("Data file(*.xls *.xlsx *.csv *.txt)")
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

    insightText = new QTextEdit(insightDock);
    insightText->setReadOnly(true);
    insightText->setPlaceholderText(QStringLiteral("选择数据文件后，这里会显示智能分析摘要、数据质量和复核线索。"));
    insightText->setObjectName(QStringLiteral("insightText"));

    insightDock->setWidget(insightText);
    addDockWidget(Qt::RightDockWidgetArea, insightDock);
}

void MainWindow::ApplyLanguage()
{
    setWindowTitle(QStringLiteral("TablePilot"));
    if (useChinese) {
        m_pAction1->setText(QStringLiteral("打开 Excel"));
        m_pAction2->setText(QStringLiteral("打开文本"));
        m_pAction3->setText(QStringLiteral("数据画像"));
        m_pAction4->setText(QStringLiteral("趋势"));
        m_pAction5->setText(QStringLiteral("分布"));
        m_pAction6->setText(QStringLiteral("导出"));
        m_pAction7->setText(QStringLiteral("退出"));
        m_pAction8->setText(QStringLiteral("分析"));
        m_pAction9->setText(QStringLiteral("EN"));
        ui->groupBox->setTitle(QStringLiteral("数据预览"));
        ui->groupBox_2->setTitle(QStringLiteral("数值画像"));
        ui->pushButton->setText(QStringLiteral("刷新趋势"));
        ui->pushButton_2->setText(QStringLiteral("刷新分布"));
        if (eyebrowLabel) eyebrowLabel->setText(QStringLiteral("本地 AI 数据工作台"));
        if (titleLabel) titleLabel->setText(QStringLiteral("TablePilot"));
        if (subtitleLabel) subtitleLabel->setText(QStringLiteral("面向复杂表格、销售数据和 TXT/CSV 文件的本地可解释分析工作台。"));
        if (insightDock) insightDock->setWindowTitle(QStringLiteral("洞察面板"));
        if (insightText) insightText->setPlaceholderText(QStringLiteral("选择数据文件后，这里会显示分析摘要、数据质量、字段结构和下一步建议。"));
    } else {
        m_pAction1->setText(QStringLiteral("Open Excel"));
        m_pAction2->setText(QStringLiteral("Open Text"));
        m_pAction3->setText(QStringLiteral("Profile"));
        m_pAction4->setText(QStringLiteral("Trend"));
        m_pAction5->setText(QStringLiteral("Distribution"));
        m_pAction6->setText(QStringLiteral("Export"));
        m_pAction7->setText(QStringLiteral("Quit"));
        m_pAction8->setText(QStringLiteral("Analyze"));
        m_pAction9->setText(QStringLiteral("中文"));
        ui->groupBox->setTitle(QStringLiteral("Data Preview"));
        ui->groupBox_2->setTitle(QStringLiteral("Numeric Profile"));
        ui->pushButton->setText(QStringLiteral("Refresh trend"));
        ui->pushButton_2->setText(QStringLiteral("Refresh distribution"));
        if (eyebrowLabel) eyebrowLabel->setText(QStringLiteral("PRIVATE AI DATA WORKBENCH"));
        if (titleLabel) titleLabel->setText(QStringLiteral("TablePilot"));
        if (subtitleLabel) subtitleLabel->setText(QStringLiteral("A local, explainable workbench for messy spreadsheets and table-like files."));
        if (insightDock) insightDock->setWindowTitle(QStringLiteral("Analysis Panel"));
        if (insightText) insightText->setPlaceholderText(QStringLiteral("Open a data file to see the analysis brief, data quality, schema, and next moves."));
    }
}

void MainWindow::CheckAnalysisService()
{
    if (!IsAnalysisServiceHealthy()) {
        statusBar()->showMessage(QStringLiteral("Starting local analysis service..."));
        if (serviceBadge) {
            serviceBadge->setText(QStringLiteral("<b>Service</b><br><span>starting</span>"));
        }
        TryStartAnalysisService();
    }

    if (IsAnalysisServiceHealthy(6000)) {
        statusBar()->showMessage(QStringLiteral("Analysis service connected"));
        if (serviceBadge) {
            serviceBadge->setText(QStringLiteral("<b>Service</b><br><span class='ok'>connected</span>"));
        }
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

    statusBar()->showMessage(QStringLiteral("Analysis service offline"));
    if (serviceBadge) {
        serviceBadge->setText(QStringLiteral("<b>Service</b><br><span>offline</span>"));
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
        insightText->setHtml(QStringLiteral(
            "<h2>Service Start Failed</h2>"
            "<div class='callout warn'>TablePilot could not start Docker Compose automatically. "
            "Start Docker Desktop, then run <code>docker compose up --build</code>.</div>"
        ));
    }
    return started;
}

void MainWindow::Slot1(){//打开Excel
    isExit = true;  // 标记 isExit 为 true，表示退出状态（这个变量在代码中未定义，可能是 MainWindow 类的成员变量）

    QString filePath = QFileDialog::getOpenFileName(this, QStringLiteral("选择Excel文件"), QString(), QStringLiteral("Excel file(*.xls *.xlsx)"));
    if(filePath.isEmpty())  // 如果文件路径为空，说明用户取消了选择，直接返回
        return;

    info_Label->clear();
    info_Label->setText(filePath);
    AnalyzeFileWithService(filePath);
}

void MainWindow::Slot2(){//打开TXT
    QString filePath = QFileDialog::getOpenFileName(this, QStringLiteral("选择TXT/CSV文件"), QString(), QStringLiteral("Table text file(*.txt *.csv)"));
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
        QMessageBox::information(this, "fail", "保存失败");
        return false;
    }

    // 如果文件名以 ".png" 结尾
    if (fileName.endsWith(".png")){
        // 显示成功保存为png文件的消息框
        QMessageBox::information(this, "success", "成功保存为png文件");
        // 调用QCustomPlot对象的savePng()方法，保存为png文件，返回保存结果
        return p_save->savePng(fileName, p_save->width(), p_save->height());
    }
    // 如果文件名以 ".jpg" 或 ".jpeg" 结尾
    else if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg")){
        // 显示成功保存为jpg文件的消息框
        QMessageBox::information(this, "success", "成功保存为jpg文件");
        // 调用QCustomPlot对象的saveJpg()方法，保存为jpg文件，返回保存结果
        return p_save->saveJpg(fileName, p_save->width(), p_save->height());
    }
    // 如果文件名以 ".bmp" 结尾
    else if (fileName.endsWith(".bmp")){
        // 显示成功保存为bmp文件的消息框
        QMessageBox::information(this, "success", "成功保存为bmp文件");
        // 调用QCustomPlot对象的saveBmp()方法，保存为bmp文件，返回保存结果
        return p_save->saveBmp(fileName, p_save->width(), p_save->height());
    }
    // 如果文件名以 ".pdf" 结尾
    else if (fileName.endsWith(".pdf")){
        // 显示成功保存为pdf文件的消息框
        QMessageBox::information(this, "success", "成功保存为pdf文件");
        // 调用QCustomPlot对象的savePdf()方法，保存为pdf文件，返回保存结果
        return p_save->savePdf(fileName, p_save->width(), p_save->height());
    }
    // 如果文件名不符合以上格式
    else{
        // 显示默认保存为png文件的消息框
        QMessageBox::information(this, "success", "成功保存,默认保存为png文件");
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
    RenderDynamicLineChart();
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
    ui->comboBox->addItem(QStringLiteral("Auto measure"), -1);
    ui->comboBox->setToolTip(QStringLiteral("选择分布图使用的数值字段"));
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

void MainWindow::AnalyzeFileWithService(const QString &filePath)
{
    if (!IsAnalysisServiceHealthy()) {
        TryStartAnalysisService();
        if (!IsAnalysisServiceHealthy(7000)) {
            QMessageBox::warning(
                this,
                QStringLiteral("TablePilot"),
                QStringLiteral("本地分析服务暂未就绪。请确认 Docker Desktop 已启动，或在项目根目录运行 docker compose up --build。")
            );
            return;
        }
    }

    QFileInfo fileInfo(filePath);
    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("TablePilot"), QStringLiteral("无法读取所选文件。"));
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
    QNetworkRequest request(QUrl("http://127.0.0.1:8000/api/analyze-upload"));
    QNetworkReply *reply = manager.post(request, multiPart);
    multiPart->setParent(reply);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        QString message = QStringLiteral("无法连接本地分析服务。请先运行：\n\ndocker compose up --build\n\n错误：%1")
                              .arg(reply->errorString());
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
        QMessageBox::warning(this, QStringLiteral("TablePilot"), QStringLiteral("分析服务返回了无法解析的数据。"));
        return;
    }

    QJsonObject root = document.object();
    lastProfile = root;
    PopulateTableFromService(root);
    ApplyTableQualityDecorations(root);
    UpdateFieldSelectors(root);
    UpdateOverviewCards(root);

    insightText->setHtml(FormatInsightHtml(root));
    insightDock->show();
    info_Label->setText(QStringLiteral("智能分析完成"));
    statusBar()->showMessage(QStringLiteral("TablePilot analysis completed"), 5000);
}

void MainWindow::PopulateTableFromService(const QJsonObject &root)
{
    QJsonObject preview = root.value("preview").toObject();
    QJsonArray rows = preview.value("rows").toArray();
    QJsonArray columns = preview.value("columns").toArray();
    if (rows.isEmpty()) {
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
                QStringLiteral("%1 | role: %2 | missing: %3")
                    .arg(item.value("semantic_type").toString())
                    .arg(item.value("role_hint").toString())
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
    RenderDynamicLineChart();
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
                cell->setToolTip(QStringLiteral("Missing value"));
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
                cell->setToolTip(QStringLiteral("Anomaly candidate, z-score %1").arg(anomaly.value("z_score").toDouble()));
            }
        }
    }
}

void MainWindow::UpdateFieldSelectors(const QJsonObject &root)
{
    QJsonArray schema = root.value("schema").toArray();
    ui->comboBox->blockSignals(true);
    ui->comboBox->clear();
    ui->comboBox->addItem(QStringLiteral("Auto measure"), -1);
    for (int column = 0; column < schema.size(); ++column) {
        QJsonObject field = schema.at(column).toObject();
        if (field.value("semantic_type").toString() == "numeric") {
            ui->comboBox->addItem(field.value("name").toString(), column);
        }
    }
    ui->comboBox->blockSignals(false);
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
        ui->tableWidget_2->setVerticalHeaderLabels(QStringList() << "count" << "mean" << "min" << "median" << "max");
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
    ui->tableWidget_2->setVerticalHeaderLabels(QStringList() << "count" << "mean" << "std" << "min" << "median" << "max");

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
        datasetCard->setText(QStringLiteral("<b>Dataset</b><br><span>%1 x %2</span><br><small>%3</small>")
                                 .arg(dataset.value("rows").toInt())
                                 .arg(dataset.value("columns").toInt())
                                 .arg(dataset.value("filename").toString().toHtmlEscaped()));
    }
    if (qualityCard) {
        qualityCard->setText(QStringLiteral("<b>Quality</b><br><span>%1 / 100</span><br><small>%2</small>")
                                 .arg(quality.value("score").toInt())
                                 .arg(quality.value("level").toString().toHtmlEscaped()));
    }
    if (schemaCard) {
        schemaCard->setText(QStringLiteral("<b>Schema</b><br><span>%1 fields</span><br><small>%2 numeric / %3 date</small>")
                                .arg(schema.size())
                                .arg(dataset.value("numeric_columns").toInt())
                                .arg(dataset.value("date_columns").toInt()));
    }
    if (recommendationCard) {
        QString title = QStringLiteral("Review data quality and chart recommendations");
        if (!recommendations.isEmpty()) {
            title = recommendations.first().toObject().value("title").toString();
        }
        QString story = plan.value("dataset_story").toString();
        recommendationCard->setText(QStringLiteral("<b>Next best analysis</b><br><span>%1</span><br><small>%2</small>")
                                        .arg(title.toHtmlEscaped())
                                        .arg((story.isEmpty() ? source.value("parser").toString() : story).toHtmlEscaped()));
    }
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
    QJsonObject source = root.value("source").toObject();
    QJsonObject brief = root.value("executive_brief").toObject();
    QJsonObject plan = root.value("analysis_plan").toObject();
    QJsonArray schema = root.value("schema").toArray();
    QJsonArray insights = root.value("insights").toArray();
    QJsonArray anomalies = root.value("anomalies").toArray();
    QJsonArray recommendations = root.value("analysis_recommendations").toArray();
    QJsonArray charts = root.value("chart_recommendations").toArray();
    QJsonArray toolTrace = root.value("tool_trace").toArray();

    auto pill = [](const QString &label, const QString &value) {
        return QStringLiteral("<span class='pill'><b>%1</b> %2</span>")
            .arg(label.toHtmlEscaped())
            .arg(value.toHtmlEscaped());
    };
    auto label = [this](const QString &en, const QString &zh) {
        return useChinese ? zh : en;
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
        ".step{border:1px solid #ececec;background:#ffffff;margin:8px 0;padding:9px 11px;border-radius:12px;}"
        "li{margin:4px 0;}"
        "code{background:#f5f5f7;padding:2px 4px;border-radius:5px;}"
        "</style>"
    );

    html << QStringLiteral("<h1>%1</h1>").arg(label(QStringLiteral("Analysis Brief"), QStringLiteral("分析简报")));
    html << QStringLiteral("<p class='muted'>%1 <b>%2</b></p>")
                .arg(label(QStringLiteral("Explainable local profiling for"), QStringLiteral("本地可解释分析：")))
                .arg(dataset.value("filename").toString().toHtmlEscaped());
    html << QStringLiteral("<div>")
                + pill(label(QStringLiteral("Rows"), QStringLiteral("行数")), QString::number(dataset.value("rows").toInt()))
                + pill(label(QStringLiteral("Columns"), QStringLiteral("列数")), QString::number(dataset.value("columns").toInt()))
                + pill(label(QStringLiteral("Quality"), QStringLiteral("质量")), QStringLiteral("%1/100").arg(quality.value("score").toInt()))
                + pill(label(QStringLiteral("Parser"), QStringLiteral("解析器")), source.value("parser").toString("-"))
                + QStringLiteral("</div>");

    if (!brief.isEmpty()) {
        html << QStringLiteral("<div class='callout'><b>%1</b><br>%2<br><span class='muted'>%3: %4</span></div>")
                    .arg(label(QStringLiteral("Executive Brief"), QStringLiteral("执行摘要")))
                    .arg(brief.value("headline").toString().toHtmlEscaped())
                    .arg(label(QStringLiteral("Confidence"), QStringLiteral("置信度")))
                    .arg(brief.value("confidence").toString().toHtmlEscaped());
        QJsonArray watchouts = brief.value("watchouts").toArray();
        if (!watchouts.isEmpty()) {
            html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Watchouts"), QStringLiteral("注意事项")));
            for (const QJsonValue &value : watchouts) {
                html << QStringLiteral("<li>%1</li>").arg(value.toString().toHtmlEscaped());
            }
            html << QStringLiteral("</ul>");
        }
    }

    if (!plan.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2>").arg(label(QStringLiteral("Analysis Planner"), QStringLiteral("分析规划器")));
        html << QStringLiteral("<div class='callout'><b>%1</b><br><span class='muted'>%2: %3</span></div>")
                    .arg(plan.value("dataset_story").toString().toHtmlEscaped())
                    .arg(label(QStringLiteral("Planner confidence"), QStringLiteral("规划置信度")))
                    .arg(plan.value("confidence").toString().toHtmlEscaped());
        QJsonArray steps = plan.value("steps").toArray();
        for (int i = 0; i < steps.size() && i < 4; ++i) {
            QJsonObject step = steps.at(i).toObject();
            html << QStringLiteral("<div class='step'><b>%1</b><br><span class='muted'>%2</span></div>")
                        .arg(step.value("title").toString().toHtmlEscaped())
                        .arg(step.value("why").toString().toHtmlEscaped());
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
        html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Recommended Next Moves"), QStringLiteral("下一步建议")));
        for (int i = 0; i < recommendations.size() && i < 4; ++i) {
            QJsonObject item = recommendations.at(i).toObject();
            html << QStringLiteral("<li><b>%1</b><br><span class='muted'>%2</span></li>")
                        .arg(item.value("title").toString().toHtmlEscaped())
                        .arg(item.value("reason").toString().toHtmlEscaped());
        }
        html << QStringLiteral("</ul>");
    }

    if (!charts.isEmpty()) {
        QJsonObject chart = charts.first().toObject();
        html << QStringLiteral("<h2>%1</h2>").arg(label(QStringLiteral("Chart Recommendation"), QStringLiteral("图表建议")));
        html << QStringLiteral("<div class='callout'><b>%1</b>: x=%2, y=%3<br><span class='muted'>%4</span></div>")
                    .arg(chart.value("chart_type").toString().toHtmlEscaped())
                    .arg(chart.value("x").toString("-").toHtmlEscaped())
                    .arg(chart.value("y").toString("-").toHtmlEscaped())
                    .arg(chart.value("reason").toString().toHtmlEscaped());
    }

    if (!schema.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Schema Snapshot"), QStringLiteral("字段结构")));
        for (int i = 0; i < schema.size() && i < 8; ++i) {
            QJsonObject item = schema.at(i).toObject();
            html << QStringLiteral("<li><b>%1</b> %2 <span class='muted'>%3</span></li>")
                        .arg(item.value("name").toString().toHtmlEscaped())
                        .arg(pill(QStringLiteral("type"), item.value("semantic_type").toString()))
                        .arg(item.value("role_hint").toString().toHtmlEscaped());
        }
        html << QStringLiteral("</ul>");
    }

    if (!insights.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Evidence Summary"), QStringLiteral("证据摘要")));
        for (const QJsonValue &value : insights) {
            html << QStringLiteral("<li>%1</li>").arg(value.toString().toHtmlEscaped());
        }
        html << QStringLiteral("</ul>");
    }

    if (!anomalies.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2><ul>").arg(label(QStringLiteral("Review Queue"), QStringLiteral("复核队列")));
        for (int i = 0; i < anomalies.size() && i < 5; ++i) {
            QJsonObject item = anomalies.at(i).toObject();
            html << QStringLiteral("<li>Row %1, <b>%2</b>, value %3, z-score %4</li>")
                        .arg(item.value("row").toInt() + 1)
                        .arg(item.value("column").toString().toHtmlEscaped())
                        .arg(item.value("value").toDouble())
                        .arg(item.value("z_score").toDouble());
        }
        html << QStringLiteral("</ul>");
    }

    if (!toolTrace.isEmpty()) {
        html << QStringLiteral("<h2>%1</h2><p>").arg(label(QStringLiteral("Tool Trace"), QStringLiteral("工具轨迹")));
        for (const QJsonValue &value : toolTrace) {
            html << pill(QStringLiteral("step"), value.toString());
        }
        html << QStringLiteral("</p>");
    }

    html << QStringLiteral("<p class='muted'>%1</p>").arg(label(QStringLiteral("This is an analytical summary, not a business recommendation."), QStringLiteral("这是分析摘要，不构成业务建议。")));
    return html.join(QString());
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
    return QStringLiteral("Column %1").arg(column + 1);
}

void MainWindow::RenderDynamicLineChart()
{
    QCustomPlot *plot = ui->widget;
    if (!plot || ui->tableWidget->rowCount() == 0) {
        return;
    }
    plot->clearGraphs();
    plot->clearItems();
    StylePlot(plot);
    plot->legend->setVisible(true);
    QList<int> columns = NumericTableColumns(5);
    if (columns.isEmpty()) {
        plot->replot();
        return;
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
    plot->xAxis->setLabel("Row");
    plot->yAxis->setLabel("Value");
    plot->xAxis->setRange(1, std::max(2, ui->tableWidget->rowCount()));
    if (minY <= maxY) {
        double padding = std::max(1.0, (maxY - minY) * 0.1);
        plot->yAxis->setRange(minY - padding, maxY + padding);
    }
    plot->replot();
}

void MainWindow::RenderDynamicBarChart()
{
    QCustomPlot *plot = ui->widget_2;
    if (!plot || ui->tableWidget->rowCount() == 0) {
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
        plot->replot();
        return;
    }

    QVector<double> values = NumericColumnValues(columns.first());
    QVector<double> ticks = NumericRowIndex(values.size());
    QCPBars *bars = new QCPBars(plot->xAxis, plot->yAxis);
    bars->setData(ticks, values);
    bars->setName(ColumnLabel(columns.first()));
    bars->setPen(QPen(QColor(22, 119, 255)));
    bars->setBrush(QColor(22, 119, 255, 90));

    auto minmax = std::minmax_element(values.begin(), values.end());
    plot->xAxis->setLabel("Row");
    plot->yAxis->setLabel(ColumnLabel(columns.first()));
    plot->xAxis->setRange(0, std::max(2, static_cast<int>(values.size()) + 1));
    if (minmax.first != values.end()) {
        double padding = std::max(1.0, (*minmax.second - *minmax.first) * 0.1);
        plot->yAxis->setRange(*minmax.first - padding, *minmax.second + padding);
    }
    plot->replot();
}

void MainWindow::StylePlot(QCustomPlot *plot)
{
    if (!plot) {
        return;
    }
    plot->setBackground(QColor(255, 255, 255));
    plot->axisRect()->setBackground(QColor(255, 255, 255));
    plot->axisRect()->setAutoMargins(QCP::msAll);
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
