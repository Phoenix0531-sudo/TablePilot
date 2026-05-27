#include "mainwindow.h"   // 包含自定义的MainWindow类的头文件
#include "ui_mainwindow.h" // 包含自动生成的ui界面文件的头文件
#include <QFileDialog>     // 包含文件对话框类的头文件
#include <QLabel>          // 包含标签类的头文件，用于创建标签部件

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), // 调用基类的构造函数
    ui(new Ui::MainWindow), // 初始化ui对象指针，指向自动生成的ui界面
    isExit(false), // 初始化是否退出标志为false
    SaveType(0)   // 初始化保存类型为0
{
    ui->setupUi(this); // 设置ui界面，将自动生成的ui界面应用到当前窗口上

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
}

void MainWindow::InitObject(){
    // 初始化对象
}

void MainWindow::createToolBar(){// 创建工具栏

    ui->mainToolBar->setMovable(false); // 设置工具栏不可移动
    ui->mainToolBar->setIconSize(QSize(36,36)); // 设置工具栏图标大小
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon); // 设置工具栏按钮样式为图标下方显示文本

    ui->mainToolBar->addSeparator(); // 添加分隔符
    ui->mainToolBar->addAction(m_pAction1); // 添加打开Excel的动作
    ui->mainToolBar->addSeparator(); // 添加分隔符
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
    ui->mainToolBar->addAction(m_pAction7); // 添加退出系统的动作
}

void MainWindow::createActions(){// 创建各个功能标签的动作
    //设置功能标签的图标，文字，以及该动作属于哪个父窗口

    m_pAction1 = new QAction(QIcon(("D:/information/Projects/Statistical_Analysis/qss/1.png")),QString("打开excel(&O)"), this);
    m_pAction1->setToolTip("打开excel");
    m_pAction2 = new QAction(QIcon(("D:/information/Projects/Statistical_Analysis/qss/2.png")),QString("打开txt(&O)"), this);
    m_pAction2->setToolTip("打开txt");
    m_pAction3 = new QAction(QIcon(("D:/information/Projects/Statistical_Analysis/qss/3.png")),QString("数据统计(&S)"), this);
    m_pAction3->setToolTip("数据统计");
    m_pAction4 = new QAction(QIcon(("D:/information/Projects/Statistical_Analysis/qss/4.png")),QString("折线图(&L)"), this);
    m_pAction4->setToolTip("折线图");
    m_pAction5 = new QAction(QIcon(("D:/information/Projects/Statistical_Analysis/qss/5.png")),QString("柱状图(&L)"), this);
    m_pAction5->setToolTip("柱状图");
    m_pAction6 = new QAction(QIcon(("D:/information/Projects/Statistical_Analysis/qss/6.png")),QString("另存为图片(&S)"), this);
    m_pAction6->setToolTip("另存为图片");
    m_pAction7 = new QAction(QIcon(("D:/information/Projects/Statistical_Analysis/qss/7.png")),QString("退出系统(&T)"), this);
    m_pAction7->setToolTip("退出系统");

    // 连接各个动作的触发信号到槽函数
    connect(m_pAction1, SIGNAL(triggered()),this, SLOT(Slot1()));
    connect(m_pAction2, SIGNAL(triggered()),this, SLOT(Slot2()));
    connect(m_pAction3, SIGNAL(triggered()),this, SLOT(Slot3()));
    connect(m_pAction4, SIGNAL(triggered()),this, SLOT(Slot4()));
    connect(m_pAction5, SIGNAL(triggered()),this, SLOT(Slot5()));
    connect(m_pAction6, SIGNAL(triggered()),this, SLOT(Slot6()));
    connect(m_pAction7, SIGNAL(triggered()),this, SLOT(Slot7()));
}

void MainWindow::createStatusBar(){// 创建状态栏并添加信息标签

    info_Label  = new QLabel; // 创建标签对象
    info_Label->setObjectName(tr("StatusLabel")); // 设置标签对象名称
    info_Label->setText(tr("")); // 设置标签初始文本为空
    ui->statusBar->addWidget(info_Label); // 将标签添加到状态栏
}

void MainWindow::Slot1(){//打开Excel
    ////ui->tableWidget->clear();  // 清除表格控件中的内容（该行代码被注释掉了）

    isExit = true;  // 标记 isExit 为 true，表示退出状态（这个变量在代码中未定义，可能是 MainWindow 类的成员变量）

    // 打开文件对话框，让用户选择 Excel 文件，获取选择的文件路径
    QString filePath = QFileDialog::getOpenFileName(this, QStringLiteral("选择Excel文件"), "D:\\information\\Projects\\Statistical_Analysis\\Statistical_Analysis", QStringLiteral("Exel file(*.xls *.xlsx)"));
    if(filePath.isEmpty())  // 如果文件路径为空，说明用户取消了选择，直接返回
        return;

    // 清空状态栏标签的内容
    info_Label->clear();
    // 设置状态栏标签的文本为选择的文件路径
    info_Label->setText(filePath);

    // 如果 p_xlsx 对象不存在，则创建一个新的 QXlsx::Document 对象，并指定文件路径和父对象为当前窗口
    if(!p_xlsx){
        p_xlsx = new QXlsx::Document(filePath, this);
    }

    // 如果成功加载 Excel 文件
    if(p_xlsx->load()){
        qInfo() << "open execel is ok!";  // 输出信息，表示成功打开 Excel 文件
    }
    else{
        qWarning() << "open execel is error!";  // 输出警告信息，表示打开 Excel 文件出错
    }

    // 获取 Excel 表格的行数和列数
    int rowLen    = p_xlsx->dimension().rowCount();
    int columnLen = p_xlsx->dimension().columnCount();

    // 遍历 Excel 表格的所有单元格
    for(int i = 0; i <= rowLen; i++){
        for(int j = 0; j <= columnLen; j++){
            // 读取当前单元格的值
            QVariant value = p_xlsx->read(i, j);
            if(!value.isNull())  {// 如果值不为空
                QString valstr = value.toString();  // 将值转换为字符串

                qDebug() << "i:" << i << "j:" << j << "valstr:" << valstr;  // 输出调试信息，显示单元格的行列及值

                // 创建一个新的表格项，并设置其文本为单元格的值
                QTableWidgetItem *itemlog = new QTableWidgetItem();
                itemlog->setText(valstr);

                // 将表格项添加到表格控件中的指定位置
                ui->tableWidget->setItem(i - 1, j - 1, itemlog);
            }
        }
    }

    delete p_xlsx;  // 删除 QXlsx::Document 对象
    p_xlsx = nullptr;  // 将指针置空，防止悬空指针
}

void MainWindow::Slot2(){//打开TXT
    isExit = true;  // 标记 isExit 为 true，表示退出状态

    QString str[6];  // 创建一个长度为 6 的字符串数组 str，用于存储每行的文本内容
    QList<QByteArray> list;  // 创建一个 QByteArray 类型的列表 list，用于存储每行文本内容的分割结果

    ////ui->tableWidget->clear();  // 清空表格控件中的内容（该行代码被注释掉了）

    // 打开文件对话框，让用户选择 TXT 文件，获取选择的文件路径
    QString filePath = QFileDialog::getOpenFileName(this, QStringLiteral("选择TXT文件"), "D:\\information\\Projects\\Statistical_Analysis\\Statistical_Analysis", QStringLiteral("Exel file(*.txt)"));
    if(filePath.isEmpty())  // 如果文件路径为空，说明用户取消了选择，直接返回
        return;

    info_Label->clear();  // 清空状态栏标签的内容
    info_Label->setText(filePath);  // 设置状态栏标签的文本为选择的文件路径

    QFile file;  // 创建 QFile 对象，用于操作文件
    file.setFileName(filePath);  // 设置文件名

    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)){  // 如果无法以只读文本模式打开文件，输出错误信息并返回
        qDebug()<<"open file error!";
        return;
    }

    int rowId = 0;  // 初始化行号为 0

    while (!file.atEnd()){  // 循环读取文件的每一行内容，直到文件末尾
        QByteArray line = file.readLine();  // 读取一行文本内容，并存储为 QByteArray 类型的对象 line
        list = line.split(',');  // 使用逗号分割每行文本内容，并存储到列表 list 中

        if(list.size() != 6){  // 如果分割后的列表长度不为 6，说明数据格式错误，输出错误信息并返回
            qDebug()<<"txt data size is error!";
            return;
        }
        if(rowId < 0 || rowId >= 6){  // 如果行号小于 0 或大于等于 6，重置为 0
            rowId = 0;
        }
        for(int i=0;i<6;i++){  // 遍历列表中的每个元素
            str[i] = list.at(i);  // 将列表中的元素存储到字符串数组 str 中的对应位置
            QTableWidgetItem *itemlog = new QTableWidgetItem();  // 创建一个新的表格项
            itemlog->setText(str[i]);  // 设置表格项的文本为当前字符串数组中的值

            ui->tableWidget->setItem(rowId, i, itemlog);  // 将表格项添加到表格控件中的指定位置
        }

        rowId = rowId + 1;  // 行号加一，准备处理下一行数据
    }
    file.close();  // 关闭文件
}

void MainWindow::Slot3(){//数据统计
    // 获取表格控件的行数和列数
    int rowLen    = ui->tableWidget->rowCount();
    int columnLen = ui->tableWidget->columnCount();

    // 创建一个长度为 6 的字符串数组 str 和用于存储最大值和最小值的字符串变量 strmax 和 strmin
    QString str[6];
    QString strmax,strmin;

    // 如果行数和列数不等于 6，则直接返回，不进行统计操作
    if(rowLen != 6 && columnLen != 6){
        return;
    }

    for(int j=0;j<columnLen;j++){// 遍历每列数据，进行统计操作

        // 如果第 0 行第 j 列的单元格不为空，则将其转换为整数并存储到对应的数组中，否则不进行处理
        if(ui->tableWidget->item(0,j) != 0)
        A[j] = ui->tableWidget->item(0,j)->text().toInt();
        if(ui->tableWidget->item(1,j) != 0)
        B[j] = ui->tableWidget->item(1,j)->text().toInt();
        if(ui->tableWidget->item(2,j) != 0)
        C[j] = ui->tableWidget->item(2,j)->text().toInt();
        if(ui->tableWidget->item(3,j) != 0)
        D[j] = ui->tableWidget->item(3,j)->text().toInt();
        if(ui->tableWidget->item(4,j) != 0)
        E[j] = ui->tableWidget->item(4,j)->text().toInt();
        if(ui->tableWidget->item(5,j) != 0)
        F[j] = ui->tableWidget->item(5,j)->text().toInt();

        // 求和
        sumA = sumA + A[j];
        sumB = sumB + B[j];
        sumC = sumC + C[j];
        sumD = sumD + D[j];
        sumE = sumE + E[j];
        sumF = sumF + F[j];
    }
    //sum
    // 将每列的和设置到tablewidget_2的第四行
    str[0].setNum(sumA);
    QTableWidgetItem *itemlog = new QTableWidgetItem();
    itemlog->setText(str[0]);
    ui->tableWidget_2->setItem(3,0,itemlog);

    str[1].setNum(sumB);
    QTableWidgetItem *itemlog1 = new QTableWidgetItem();
    itemlog1->setText(str[1]);
    ui->tableWidget_2->setItem(3,1,itemlog1);

    str[2].setNum(sumC);
    QTableWidgetItem *itemlog2 = new QTableWidgetItem();
    itemlog2->setText(str[2]);
    ui->tableWidget_2->setItem(3,2,itemlog2);

    str[3].setNum(sumD);
    QTableWidgetItem *itemlog3 = new QTableWidgetItem();
    itemlog3->setText(str[3]);
    ui->tableWidget_2->setItem(3,3,itemlog3);

    str[4].setNum(sumE);
    QTableWidgetItem *itemlog4 = new QTableWidgetItem();
    itemlog4->setText(str[4]);
    ui->tableWidget_2->setItem(3,4,itemlog4);

    str[5].setNum(sumF);
    QTableWidgetItem *itemlog5 = new QTableWidgetItem();
    itemlog5->setText(str[5]);
    ui->tableWidget_2->setItem(3,5,itemlog5);

    //reg
    // 计算每列数据的平均值
    regA = sumA/columnLen;
    regB = sumB/columnLen;
    regC = sumC/columnLen;
    regD = sumD/columnLen;
    regE = sumE/columnLen;
    regF = sumF/columnLen;

    // 将平均值转换为字符串并设置到字符串数组中
    str[0].setNum(regA);
    // 创建一个新的表格项，并设置其文本为平均值
    QTableWidgetItem *itemlog00 = new QTableWidgetItem();
    itemlog00->setText(str[0]);
    // 将表格项添加到表格控件中的指定位置
    ui->tableWidget_2->setItem(0,0,itemlog00);

    str[1].setNum(regB);
    QTableWidgetItem *itemlog111 = new QTableWidgetItem();
    itemlog111->setText(str[1]);
    ui->tableWidget_2->setItem(0,1,itemlog111);

    str[2].setNum(regC);
    QTableWidgetItem *itemlog222 = new QTableWidgetItem();
    itemlog222->setText(str[2]);
    ui->tableWidget_2->setItem(0,2,itemlog222);

    str[3].setNum(regD);
    QTableWidgetItem *itemlog33 = new QTableWidgetItem();
    itemlog33->setText(str[3]);
    ui->tableWidget_2->setItem(0,3,itemlog33);

    str[4].setNum(regE);
    QTableWidgetItem *itemlog44 = new QTableWidgetItem();
    itemlog44->setText(str[4]);
    ui->tableWidget_2->setItem(0,4,itemlog44);

    str[5].setNum(regF);
    QTableWidgetItem *itemlog55 = new QTableWidgetItem();
    itemlog55->setText(str[5]);
    ui->tableWidget_2->setItem(0,5,itemlog55);

    //min max
    // 定义六个 QVector 对象，分别用于存储每一列的数据
    QVector <int> AVect,BVect,CVect,DVect,EVect,FVect;

    //A
    // 对于数组 A 的每个元素，将其值存储到 AVect 中
    for(int i = 0; i < 6; i++) {
        int aa = A[i];  // 获取数组 A 中索引为 i 的元素的值
        AVect.push_back(aa);  // 将该值添加到 AVect 中
    }

    // 使用标准库函数 std::max_element 找到 AVect 中的最大元素
    auto maxa = std::max_element(std::begin(AVect), std::end(AVect));
    // 使用标准库函数 std::min_element 找到 AVect 中的最小元素
    auto mina = std::min_element(std::begin(AVect), std::end(AVect));

    // 将找到的最小值和最大值分别赋值给 minA 和 maxA
    minA = *mina;  // 将 mina 指向的元素值赋给 minA
    maxA = *maxa;  // 将 maxa 指向的元素值赋给 maxA

    // 将最大值转换为字符串，并设置给 strmax
    strmax.setNum(maxA);
    // 创建一个新的 QTableWidgetItem 对象，设置其文本为 strmax
    QTableWidgetItem *itemlog10 = new QTableWidgetItem();
    itemlog10->setText(strmax);
    // 将该 QTableWidgetItem 对象添加到表格控件 ui->tableWidget_2 的第 1 行、第 0 列
    ui->tableWidget_2->setItem(1, 0, itemlog10);

    // 将最小值转换为字符串，并设置给 strmin
    strmin.setNum(minA);
    // 创建一个新的 QTableWidgetItem 对象，设置其文本为 strmin
    QTableWidgetItem *itemlog20 = new QTableWidgetItem();
    itemlog20->setText(strmin);
    // 将该 QTableWidgetItem 对象添加到表格控件 ui->tableWidget_2 的第 2 行、第 0 列
    ui->tableWidget_2->setItem(2, 0, itemlog20);

    //B
    for(int i=0;i<6;i++)
    {
      int bb = B[i];
      BVect.push_back(bb);
    }

    auto maxb = std::max_element(std::begin(BVect), std::end(BVect));
    auto minb = std::min_element(std::begin(BVect), std::end(BVect));

    minB = *minb;
    maxB = *maxb;

    strmax.setNum(maxB);
    QTableWidgetItem *itemlog11 = new QTableWidgetItem();
    itemlog11->setText(strmax);
    ui->tableWidget_2->setItem(1,1,itemlog11);

    strmin.setNum(minB);
    QTableWidgetItem *itemlog21 = new QTableWidgetItem();
    itemlog21->setText(strmin);
    ui->tableWidget_2->setItem(2,1,itemlog21);

    //C
    for(int i=0;i<6;i++)
    {
      int cc = C[i];
      CVect.push_back(cc);
    }

    auto maxc = std::max_element(std::begin(CVect), std::end(CVect));
    auto minc = std::min_element(std::begin(CVect), std::end(CVect));

    minC = *minc;
    maxC = *maxc;

    strmax.setNum(maxC);
    QTableWidgetItem *itemlog12 = new QTableWidgetItem();
    itemlog12->setText(strmax);
    ui->tableWidget_2->setItem(1,2,itemlog12);

    strmin.setNum(minC);
    QTableWidgetItem *itemlog22 = new QTableWidgetItem();
    itemlog22->setText(strmin);
    ui->tableWidget_2->setItem(2,2,itemlog22);

    //D
    for(int i=0;i<6;i++)
    {
      int dd = D[i];
      DVect.push_back(dd);
    }

    auto maxd = std::max_element(std::begin(DVect), std::end(DVect));
    auto mind = std::min_element(std::begin(DVect), std::end(DVect));

    minD = *mind;
    maxD = *maxd;

    strmax.setNum(maxD);
    QTableWidgetItem *itemlog13 = new QTableWidgetItem();
    itemlog13->setText(strmax);
    ui->tableWidget_2->setItem(1,3,itemlog13);

    strmin.setNum(minD);
    QTableWidgetItem *itemlog23 = new QTableWidgetItem();
    itemlog23->setText(strmin);
    ui->tableWidget_2->setItem(2,3,itemlog23);

    //E
    for(int i=0;i<6;i++)
    {
      int ee = E[i];
      EVect.push_back(ee);
    }

    auto maxe = std::max_element(std::begin(EVect), std::end(EVect));
    auto mine = std::min_element(std::begin(EVect), std::end(EVect));

    minE = *mine;
    maxE = *maxe;

    strmax.setNum(maxE);
    QTableWidgetItem *itemlog14 = new QTableWidgetItem();
    itemlog14->setText(strmax);
    ui->tableWidget_2->setItem(1,4,itemlog14);

    strmin.setNum(minE);
    QTableWidgetItem *itemlog24 = new QTableWidgetItem();
    itemlog24->setText(strmin);
    ui->tableWidget_2->setItem(2,4,itemlog24);

    //F
    for(int i=0;i<6;i++)
    {
      int ff = F[i];
      FVect.push_back(ff);
    }

    auto maxf = std::max_element(std::begin(FVect), std::end(FVect));
    auto minf = std::min_element(std::begin(FVect), std::end(FVect));

    minF = *minf;
    maxF = *maxf;

    strmax.setNum(maxF);
    QTableWidgetItem *itemlog15 = new QTableWidgetItem();
    itemlog15->setText(strmax);
    ui->tableWidget_2->setItem(1,5,itemlog15);

    strmin.setNum(minF);
    QTableWidgetItem *itemlog25 = new QTableWidgetItem();
    itemlog25->setText(strmin);
    ui->tableWidget_2->setItem(2,5,itemlog25);

    //clear
    sumA = 0;
    sumB = 0;
    sumC = 0;
    sumD = 0;
    sumE = 0;
    sumF = 0;

    regA = 0;
    regB = 0;
    regC = 0;
    regD = 0;
    regE = 0;
    regF = 0;

    AVect.clear();
    BVect.clear();
    CVect.clear();
    DVect.clear();
    EVect.clear();
    FVect.clear();

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

bool MainWindow::ItemEmpty(int x, int y){
    // 返回指定表格项的文本是否为空，这里使用了tableWidget_2指向的QTableWidget对象的item()方法获取表格项，
    // 并调用text()方法检查文本是否为空。
    return ui->tableWidget_2->item(x, y)->text().isEmpty();
}

void MainWindow::on_pushButton_2_clicked(){//柱状图
    if(!isExit){
        return;
    }
    //如果有柱状图，则清除
    int plottableCount = ui->widget_2->plottableCount();
    //plottableCount是已有的柱状图数量，如果不为0，则表示已经有了柱状图数据
    if (plottableCount != 0){
        ui->widget_2->clearPlottables();
    }
    //新建柱状图bars，并与指定的x轴y轴联系起来
    //QCPAxis来自QCustomPlot，用于绘制坐标轴
    QCPAxis *xAxis = ui->widget_2->xAxis;
    QCPAxis *yAxis = ui->widget_2->yAxis;
    QCPBars *bars  = new QCPBars(xAxis,yAxis);

    bars->setAntialiased(false);//禁用抗锯齿
    bars->setName("销售量");//设置柱状图名称为销售量

//    bars->setPen(QPen(QColor(0,160,140)));
//    bars->setBrush(QColor(20,68,106));

    //向量==动态数组，是STL中的的一种容器，可以无需管理内存
    // 定义用于存储柱状图 X 轴刻度值的向量
    QVector<double> ticks;
    // 定义用于存储柱状图 Y 轴数据值的向量
    QVector<double> buyData;
    // 定义用于存储柱状图 X 轴刻度标签的向量
    QVector<QString> labels;

    // 清空之前可能存在的数据
    ticks.clear();
    buyData.clear();
    labels.clear();

    // 设置 X 轴刻度值和刻度标签
    ticks << 1 << 2 << 3 << 4 << 5 << 6;
    labels << "A" << "B" << "C" << "D" << "E" << "F";

    // 创建一个智能指针，用于管理自定义的文本刻度
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    // 添加刻度和标签
    textTicker->addTicks(ticks, labels);
    // 将自定义文本刻度设置给 X 轴
    xAxis->setTicker(textTicker);

    // 设置 X 轴刻度标签旋转角度为 60 度
    xAxis->setTickLabelRotation(60);
    // 关闭 X 轴次刻度
    xAxis->setSubTicks(false);
    // 设置 X 轴刻度长度，第一个参数为主刻度长度，第二个参数为次刻度长度
    xAxis->setTickLength(0, 4);
    // 设置 X 轴范围
    xAxis->setRange(0, 7);
    // 设置 X 轴标签
    xAxis->setLabel("种类");
    // 设置 X 轴上端箭头样式
    xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);

    // 设置 Y 轴填充距离
    yAxis->setPadding(36);
    // 设置 Y 轴标签
    yAxis->setLabel("销售量");
    // 设置 Y 轴上端箭头样式
    yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);



    if(ui->comboBox->currentIndex() == 0){
        // 如果下拉框当前选中的索引为0，表示选择了某种操作方式
        // 循环遍历6次，将每行第一列的数据提取出来并转换为整数，放入buyData向量中
        for(int i=0; i<6; i++){
            int ff;
            // 检查单元格是否为空
            if(ui->tableWidget->item(i,0) != 0){
                // 获取单元格的文本内容并转换为整数
                ff = ui->tableWidget->item(i,0)->text().toInt();
            }

            // 将提取的整数值转换为double类型，并存入buyData向量
            buyData.push_back((double)ff);
        }

        // 使用标准库函数max_element和min_element找到buyData向量中的最大值和最小值
        auto maxf = std::max_element(std::begin(buyData), std::end(buyData));
        auto minf = std::min_element(std::begin(buyData), std::end(buyData));

        // 将找到的最大值和最小值保存到minF和maxF变量中
        minF = *minf;
        maxF = *maxf;

        // 设置y轴的范围，使得柱状图能够容纳最大值和最小值，并在上下各增加50的空间
        yAxis->setRange(minF-50, maxF+50);

        // 设置柱状图的数据，x轴的刻度为ticks向量，y轴的数据为buyData向量
        bars->setData(ticks, buyData);

        // 设置柱状图的画笔颜色为红色（RGB为(180,0,120)），并设置透明度为70%
        bars->setPen(QPen(QColor(180,0,120)));
        // 设置柱状图的填充颜色为红色（RGB为(180,0,120)），并设置透明度为70%
        bars->setBrush(QColor(180,0,120,70));
    }


    else if(ui->comboBox->currentIndex() == 1){
        //z
        for(int i=0;i<6;i++)
        {
          int ff;
          if(ui->tableWidget->item(i,1) != 0){
             ff = ui->tableWidget->item(i,1)->text().toInt();
          }

          buyData.push_back((double)ff);
        }

        auto maxf = std::max_element(std::begin(buyData), std::end(buyData));
        auto minf = std::min_element(std::begin(buyData), std::end(buyData));

        minF = *minf;
        maxF = *maxf;

        yAxis->setRange(minF-50,maxF+50);
        bars->setData(ticks,buyData);

        bars->setPen(QPen(QColor(255,156,0)));
        bars->setBrush(QColor(255,156,0,70));
    }

    else if(ui->comboBox->currentIndex() == 2){
        //z
        for(int i=0;i<6;i++){
          int ff;
          if(ui->tableWidget->item(i,2) != 0)
          {
             ff = ui->tableWidget->item(i,2)->text().toInt();
          }

          buyData.push_back((double)ff);
        }

        auto maxf = std::max_element(std::begin(buyData), std::end(buyData));
        auto minf = std::min_element(std::begin(buyData), std::end(buyData));

        minF = *minf;
        maxF = *maxf;

        yAxis->setRange(minF-50,maxF+50);
        bars->setData(ticks,buyData);

        bars->setPen(QPen(QColor(0,255,255)));
        bars->setBrush(QColor(0,255,255,70));
    }

    else if(ui->comboBox->currentIndex() == 3){
        //z
        for(int i=0;i<6;i++){
          int ff;
          if(ui->tableWidget->item(i,3) != 0)
          {
             ff = ui->tableWidget->item(i,3)->text().toInt();
          }

          buyData.push_back((double)ff);
        }

        auto maxf = std::max_element(std::begin(buyData), std::end(buyData));
        auto minf = std::min_element(std::begin(buyData), std::end(buyData));

        minF = *minf;
        maxF = *maxf;

        yAxis->setRange(minF-50,maxF+50);
        bars->setData(ticks,buyData);

        bars->setPen(QPen(QColor(0,168,150)));
        bars->setBrush(QColor(0,168,150,70));
    }

    else if(ui->comboBox->currentIndex() == 4){
        //z
        for(int i=0;i<6;i++){
          int ff;
          if(ui->tableWidget->item(i,4) != 0)
          {
             ff = ui->tableWidget->item(i,4)->text().toInt();
          }

          buyData.push_back((double)ff);
        }

        auto maxf = std::max_element(std::begin(buyData), std::end(buyData));
        auto minf = std::min_element(std::begin(buyData), std::end(buyData));

        minF = *minf;
        maxF = *maxf;

        yAxis->setRange(minF-50,maxF+50);
        bars->setData(ticks,buyData);

        bars->setPen(QPen(QColor(10,100,65)));
        bars->setBrush(QColor(10,100,65,70));
    }

    else if(ui->comboBox->currentIndex() == 5){
        //z
        for(int i=0;i<6;i++){
          int ff;
          if(ui->tableWidget->item(i,5) != 0)
          {
             ff = ui->tableWidget->item(i,5)->text().toInt();
          }

          buyData.push_back((double)ff);
        }

        auto maxf = std::max_element(std::begin(buyData), std::end(buyData));
        auto minf = std::min_element(std::begin(buyData), std::end(buyData));

        minF = *minf;
        maxF = *maxf;

        yAxis->setRange(minF-50,maxF+50);
        bars->setData(ticks,buyData);

        bars->setPen(QPen(QColor(0,0,255)));
        bars->setBrush(QColor(0,0,255,70));
    }

    else
    {}
    bars->setData(ticks,buyData);//将数据设置到bars中
    ui->widget_2->replot();
}

void MainWindow::on_pushButton_3_clicked(){
    if(!isExit)
    {
        return;
    }
}

void MainWindow::on_pushButton_clicked() {
    // 获取指向 QCustomPlot 控件的指针
    QCustomPlot *p_line = ui->widget;

    // 如果指针为空或者程序不处于退出状态，则直接返回，不执行后续操作
    if(p_line == nullptr || !isExit) {
        return;
    }

    // 获取当前绘图区域中的绘图项数量
    int plottableCount = ui->widget->plottableCount();
    // 如果绘图项数量不为0，则清除所有的绘图项
    if (plottableCount != 0) {
        ui->widget->clearPlottables();
    }

    // 获取表格的行数和列数
    int rowLen    = ui->tableWidget->rowCount();
    int columnLen = ui->tableWidget->columnCount();

    // 定义用于存储数据的向量
    QVector <double> xData;
    QVector <double> yData1, yData2, yData3, yData4, yData5, yData6;
    QVector <double> saleVal;

    // 清空存储数据的向量
    xData.clear();
    yData1.clear();
    yData2.clear();
    yData3.clear();
    yData4.clear();
    yData5.clear();
    yData6.clear();
    saleVal.clear();

    // 设置 x 轴的标签和范围
    p_line->xAxis->setLabel("月份");
    p_line->xAxis->setRange(1, 6);

    // 设置 y 轴的标签
    p_line->yAxis->setLabel("销售量");

    // 从表格中提取数据并存储到 saleVal 向量中
    for(int i = 0; i < rowLen; i++) {
        for(int j = 0; j < columnLen; j++) {
            int sv = ui->tableWidget->item(i, j)->text().toInt();
            saleVal.push_back((double)sv);
        }
    }

    // 获取 saleVal 向量中的最大值和最小值
    auto maxf = std::max_element(std::begin(saleVal), std::end(saleVal));
    auto minf = std::min_element(std::begin(saleVal), std::end(saleVal));

    int minL = *minf;
    int maxL = *maxf;

    // 设置 y 轴的范围
    p_line->yAxis->setRange(minL - 10, maxL + 10);

    p_line->addGraph();
    p_line->addGraph();
    p_line->addGraph();
    p_line->addGraph();
    p_line->addGraph();
    p_line->addGraph();

    // 添加绘图对象
    for(int i = 1; i < 7; i++) {
        xData << i;
    }

    // 从表格中提取数据并存储到对应的 yData 向量中
    for(int j = 0; j < columnLen; j++) {
        if(ui->tableWidget->item(0, j) != 0)
            yData1 << ui->tableWidget->item(0, j)->text().toInt();

        if(ui->tableWidget->item(1, j) != 0)
            yData2 << ui->tableWidget->item(1, j)->text().toInt();

        if(ui->tableWidget->item(2, j) != 0)
            yData3 << ui->tableWidget->item(2, j)->text().toInt();

        if(ui->tableWidget->item(3, j) != 0)
            yData4 << ui->tableWidget->item(3, j)->text().toInt();

        if(ui->tableWidget->item(4, j) != 0)
            yData5 << ui->tableWidget->item(4, j)->text().toInt();

        if(ui->tableWidget->item(5, j) != 0)
            yData6 << ui->tableWidget->item(5, j)->text().toInt();
    }

    // 将数据设置到绘图对象中
    p_line->graph(0)->setData(xData, yData1);
    p_line->graph(1)->setData(xData, yData2);
    p_line->graph(2)->setData(xData, yData3);
    p_line->graph(3)->setData(xData, yData4);
    p_line->graph(4)->setData(xData, yData5);
    p_line->graph(5)->setData(xData, yData6);

    // 设置图例和绘图样式
    p_line->graph(0)->setPen(QPen(QColor(255,165,0)));
    p_line->graph(0)->setName("A");

    p_line->graph(1)->setPen(QPen(QColor(0,128,0)));
    p_line->graph(1)->setName("B");

    p_line->graph(2)->setPen(QPen(QColor(65,105,225)));
    p_line->graph(2)->setName("C");

    p_line->graph(3)->setPen(QPen(QColor(149,0,234)));
    p_line->graph(3)->setName("D");

    p_line->graph(4)->setPen(QPen(QColor(128,0,128)));
    p_line->graph(4)->setName("E");

    p_line->graph(5)->setPen(QPen(QColor(255,165,0)));
    p_line->graph(5)->setName("F");

    // 重新绘制图形
    p_line->replot();
}

//
void MainWindow::createStyle() {
    // 设置主工具栏的样式表
    SetStyleSheet(ui->mainToolBar, "D:/information/Projects/Statistical_Analysis/qss/blue1.qss");

    // 设置中央窗口部件的样式表
    SetStyleSheet(ui->centralWidget, "D:/information/Projects/Statistical_Analysis/qss/blue1.qss");

    // 设置状态栏的样式表
    SetStyleSheet(ui->statusBar, "D:/information/Projects/Statistical_Analysis/qss/blue1.qss");

    // 设置表格部件的列宽
//    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
//    ui->tableWidget->setColumnWidth(0,30);

    // 设置另一个表格部件的列宽
//    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed);
//    ui->tableWidget_2->setColumnWidth(0,30);
}

void MainWindow::SetStyleSheet(QWidget* pWidget, QString strQSS) {
    // 检查部件是否为空
    if (nullptr == pWidget) {
        return;
    }

    // 打开样式表文件
    QFile qss(strQSS);
    qss.open(QFile::ReadOnly);

    // 读取样式表内容并设置给部件
    pWidget->setStyleSheet(qss.readAll());

    // 检查是否成功读取到样式表内容
    if (qss.readAll() == "") {
        return;
    }

    // 关闭文件
    qss.close();
}
