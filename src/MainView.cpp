//
// Created by sun on 8/25/16.
//

#include "MainView.h"
#include "GameLevelData.h"
#include "SelectorScene.h"
#include "GameScene.h"
#include "AspectRatioLayout.h"

MainView *gMainView;

MainView::MainView(MainWindow *mainWindow)
        : width(900), height(600), //一像素 = 0.04cm   36cm * 24cm
          usernameSettingEntry("Global/Username"),
          selectorScene(nullptr), gameScene(nullptr),
          mainWindow(mainWindow)
{
    gMainView = this;

    setMouseTracking(true);

    //setRenderHint 反走样（图像），QPainter是用于执行绘图操作的类
    //API->Application Program Interface
    //Anti-aliasing（抗锯齿处理）
    setRenderHint(QPainter::Antialiasing, true);
    //绘制的字体抗锯齿
    setRenderHint(QPainter::TextAntialiasing, true);
    //指示引擎应使用平滑的pixmap转换算法（如双线性）而不是最邻近的。
    setRenderHint(QPainter::SmoothPixmapTransform, true);

    setFrameStyle(0);//  ? 关闭框架样式 || QFrame类
    // 水平滚动 ScrollBarAlwaysOff 不显示滚动条 ||QAbstractScrollArea类 滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//垂直滚动
    setMinimumSize(width, height); //最小窗口
    // Set up username
    if (getUsername().isEmpty()) {
        //Qt有一个包装器getenv()『取得环境变量名称』,叫做qgetenv().
        QString username = qgetenv("USER"); // Linux or Mac
        if (username.isEmpty())
            username = qgetenv("USERNAME"); // Windows
        if (username.isEmpty())
            username = tr("Guest");
        setUsername(username);
    }
}

MainView::~MainView()
{
    if (selectorScene)
        selectorScene->deleteLater();
    //if (gameScene)
    //    gameScene->deleteLater();
}

QString MainView::getUsername() const
{
    //QSettings使用户可以保存应用程序设置，并且支持用户自定义存储格式 ||类似Windows注册表
    return QSettings().value(usernameSettingEntry, "").toString();
}

void MainView::setUsername(const QString &username)
{
    return QSettings().setValue(usernameSettingEntry, username);
}

MainWindow *MainView::getMainWindow() const
{
    return mainWindow;
}

//切换游戏场景(下一关)
//不支持游戏中切换场景
void MainView::switchToGameScene(const QString &eName)
{
    GameScene *newGameScene = new GameScene(GameLevelDataFactory(eName)); //获取关卡场景
    //进入新场景
    setScene(newGameScene);
    //if (gameScene)
    //    gameScene->deleteLater();
    gameScene = newGameScene;
    gameScene->loadReady();
}

//切换菜单界面
void MainView::switchToMenuScene()
{
    /*
    QGraphicsScene *oldScene = nullptr;
    if (this->scene())
        oldScene = this->scene();
    //setScene(scene);
    if (oldScene)
        oldScene->deleteLater();
    */
    SelectorScene *newSelectorScene = new SelectorScene;
    setScene(newSelectorScene);
    if (selectorScene)
        selectorScene->deleteLater();
    selectorScene = newSelectorScene;
    selectorScene->loadReady();
}

//QGraphicsScene类用于在场景中操作大量的2D图形元素，作为一个容器QGraphicsItems存在的
/*
void MainView::switchToScene(QGraphicsScene *scene)
{
    QGraphicsScene *oldScene = nullptr;
    if (this->scene())
        oldScene = this->scene();
    setScene(scene);
    if (oldScene)
        oldScene->deleteLater();
}
*/
void MainView::resizeEvent(QResizeEvent *event)
{
    // `fitInView` has a bug causing extra margin.
    // see "https://bugreports.qt.io/browse/QTBUG-11945"
    QRectF viewRect = frameRect();
    QTransform trans;
    trans.scale(viewRect.width() / width, viewRect.height() / height);
    setTransform(trans);
}

MainWindow::MainWindow()
    : fullScreenSettingEntry("UI/FullScreen"),
      mainView(new MainView(this)),
      fullScreenAction(new QAction)
{
    // Layout 布局
    QWidget *centralWidget = new QWidget; //QWidget 类是所有用户界面对象的基类。
    QLayout *layout = new AspectRatioLayout;
    layout->addWidget(mainView);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // Full Screen action triggered with "F11"  全屏
    fullScreenAction->setCheckable(true);
    fullScreenAction->setShortcut(Qt::Key_F11);
    addAction(fullScreenAction);
    connect(fullScreenAction, &QAction::toggled, [this] (bool checked) {
        if (checked)
            setWindowState(Qt::WindowFullScreen);
        else
            setWindowState(Qt::WindowNoState);
        QSettings().setValue(fullScreenSettingEntry, checked);
    });
    fullScreenAction->setChecked(QSettings().value(fullScreenSettingEntry, false).toBool());

    // Set background color to black
    setPalette(Qt::black);
    setAutoFillBackground(true);

    adjustSize();
}

QAction *MainWindow::getFullScreenAction() const
{
    return fullScreenAction;
}
