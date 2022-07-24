//
// Created by sun on 8/26/16.
//
//关卡

#include "GameLevelData.h"
#include "GameScene.h"
#include "ImageManager.h"
#include "Timer.h"
#include "QList"

GameLevelData::GameLevelData() : cardKind(0),
                                 dKind(1),
                                 sunNum(50),
                                 backgroundImage("interface/background1.jpg"),
                                 LF{ 0, 1, 1, 1, 1, 1 },
                                 canSelectCard(true),
                                 staticCard(true),
                                 showScroll(true),
                                 produceSun(true),
                                 hasShovel(true),
                                 maxSelectedCards(10), //8 测试
                                 coord(0),
                                 flagNum(0)
{}

void  GameLevelData::loadAccess(GameScene *gameScene)
{
    gameScene->loadAcessFinished();
}

void GameLevelData::startGame(GameScene *gameScene)
{
    initLawnMower(gameScene);
    gameScene->prepareGrowPlants( [gameScene] {
        gameScene->beginBGM();
        gameScene->beginMonitor();
        gameScene->beginCool();
        gameScene->beginSun(25);
        (new Timer(gameScene, 3000/*15000*/, [gameScene] {
            gameScene->beginZombies();
        }))->start();
    });
}

void GameLevelData::initLawnMower(GameScene *gameScene)
{
    for (int i = 0; i < LF.size(); ++i) {
        if (LF[i] == 1)
            gameScene->customSpecial("oLawnCleaner", -1, i);
        else if (LF[i] == 2)
            gameScene->customSpecial("oPoolCleaner", -1, i);
    }
}

void GameLevelData::endGame(GameScene *gameScene)
{}

//房前，白天
GameLevelData_1::GameLevelData_1()
{
    backgroundImage = "interface/background1.jpg";
    backgroundMusic = "qrc:/audio/UraniwaNi.mp3";
    sunNum = 10000; //用于测试
    canSelectCard = true;
    showScroll = true;

    eName = "1";
    cName = tr("Level 1");
    pName = {   "oSunflower", "oTwinSunflower", "oPeashooter", \
                "oRepeater", "oGatlingPea", "oSplitPea", "oThreepeater", "oSnowPea",
                "oTorchwood", "oWallNut", "oTallNut", "oPumpkinHead",
                "oPotatoMine", "oJalapeno",  "oCherryBomb", "oSquash", \
                "oChomper", "oSpikeweed", "oSpikerock", "oSunShroom",\
                "oLilyPad", "oTangleKlep", "oFlowerPot", "oHypnoShroom"};
    zombieData = { { "oZombie3", 1, 1, {} }, { "oFlagZombie", 1, 1, {} }, \
                   { "oConeheadZombie", 3, 3, {} }, { "oBucketheadZombie", 3, 3, {} }, \
                   { "oPoleVaultingZombie", 3, 3, {} }, { "oImpZombie", 3, 3, {}}, \
                   { "oFootBallZombie", 3, 3, {}}, { "oNewspaperZombie", 3, 3, {}},\
                   { "oScreenDoorZombie", 3, 3, {}}, { "oJackBoxZombie", 3, 3, {}}};

    flagNum = 5;
    largeWaveFlag = { 4 };
    flagToSumNum = QPair<QList<int>, QList<int> >({ 3, 4, 8, 9, 10, 13, 15, 19 }, { 1, 2, 3, 5, 15, 6, 8, 10, 20 });
}

//房前，夜晚
GameLevelData_2::GameLevelData_2()
{
    backgroundImage = "interface/background2.jpg";
    backgroundMusic = "qrc:/audio/UraniwaNi.mp3";
    sunNum = 10000; //用于测试
    canSelectCard = true;
    showScroll = true;

    eName = "2";
    cName = tr("Level 2");
    pName = {   "oSunflower", "oTwinSunflower", "oPeashooter", \
                "oRepeater", "oGatlingPea", "oSplitPea", "oThreepeater", "oSnowPea",
                "oTorchwood", "oWallNut", "oTallNut", "oPumpkinHead",
                "oPotatoMine", "oJalapeno",  "oCherryBomb", "oSquash", \
                "oChomper", "oSpikeweed", "oSpikerock", "oSunShroom",\
                "oLilyPad", "oTangleKlep", "oFlowerPot"};
    zombieData = { { "oZombie3", 1, 1, {} }, { "oFlagZombie", 1, 1, {} }, \
                   { "oConeheadZombie", 3, 3, {} }, { "oBucketheadZombie", 3, 3, {} }, \
                   { "oPoleVaultingZombie", 3, 3, {} }, { "oImpZombie", 3, 3, {}}, \
                   { "oFootBallZombie", 3, 3, {}}, { "oNewspaperZombie", 3, 3, {}},\
                   { "oScreenDoorZombie", 3, 3, {}}, { "oJackBoxZombie", 3, 3, {}}};
    flagNum = 20;
    largeWaveFlag = { 6, 12, 19 };
    flagToSumNum = QPair<QList<int>, QList<int> >({ 3, 4, 8, 9, 10, 13, 15, 19 }, { 1, 2, 3, 5, 15, 6, 8, 10, 20 });
}

//后花园，白天
GameLevelData_3::GameLevelData_3()
{
    backgroundImage = "interface/background3.jpg";
    backgroundMusic = "qrc:/audio/UraniwaNi.mp3";

    sunNum = 10000; //用于测试
    coord = 1;  //后花园坐标
    LF = {0, 1, 1, 2, 2, 1, 1};
    canSelectCard = true;
    showScroll = true;

    eName = "3";
    cName = tr("Level 3");
    pName = {   "oSunflower", "oTwinSunflower", "oPeashooter", \
                "oRepeater", "oGatlingPea", "oSplitPea", "oThreepeater", "oSnowPea",
                "oTorchwood", "oWallNut", "oTallNut", "oPumpkinHead",
                "oPotatoMine", "oJalapeno",  "oCherryBomb", "oSquash", \
                "oChomper", "oSpikeweed", "oSpikerock", "oSunShroom",\
                "oLilyPad", "oTangleKlep", "oFlowerPot"};
    zombieData = { { "oZombie3", 1, 1, {} }, { "oFlagZombie", 1, 1, {} }, \
                   { "oConeheadZombie", 3, 3, {} }, { "oBucketheadZombie", 3, 3, {} }, \
                   { "oPoleVaultingZombie", 3, 3, {} }, { "oImpZombie", 3, 3, {}}, \
                   { "oFootBallZombie", 3, 3, {}}, { "oNewspaperZombie", 3, 3, {}},\
                   { "oScreenDoorZombie", 3, 3, {}}, { "oJackBoxZombie", 3, 3, {}},\
                   { "oDuckyTubeZombie", 3, 3, {}}};
    flagNum = 20;
    largeWaveFlag = { 6, 12, 19 };
    flagToSumNum = QPair<QList<int>, QList<int> >({ 3, 4, 8, 9, 10, 13, 15, 19 }, { 1, 2, 3, 5, 15, 6, 8, 10, 20 });
}

//后花园， 夜晚
GameLevelData_4::GameLevelData_4()
{
    backgroundImage = "interface/background4.jpg";
    backgroundMusic = "qrc:/audio/Mountains.mp3";

    sunNum = 10000; //用于测试
    coord = 1;  //后花园坐标
    LF = {0, 1, 1, 2, 2, 1, 1};
    canSelectCard = true;
    showScroll = true;

    eName = "4";
    cName = tr("Level 4");
    pName = {   "oSunflower", "oTwinSunflower", "oPeashooter", \
                "oRepeater", "oGatlingPea", "oSplitPea", "oThreepeater", "oSnowPea",
                "oTorchwood", "oWallNut", "oTallNut", "oPumpkinHead",
                "oPotatoMine", "oJalapeno",  "oCherryBomb", "oSquash", \
                "oChomper", "oSpikeweed", "oSpikerock", "oSunShroom",\
                "oLilyPad", "oTangleKlep", "oFlowerPot"};
    zombieData = { { "oZombie3", 1, 1, {} }, { "oFlagZombie", 1, 1, {} }, \
                   { "oConeheadZombie", 3, 3, {} }, { "oBucketheadZombie", 3, 3, {} }, \
                   { "oPoleVaultingZombie", 3, 3, {} }, { "oImpZombie", 3, 3, {}}, \
                   { "oFootBallZombie", 3, 3, {}}, { "oNewspaperZombie", 3, 3, {}},\
                   { "oScreenDoorZombie", 3, 3, {}}, { "oJackBoxZombie", 3, 3, {}}};
    flagNum = 20;
    largeWaveFlag = { 6, 12, 19 };
    flagToSumNum = QPair<QList<int>, QList<int> >({ 3, 4, 8, 9, 10, 13, 15, 19 }, { 1, 2, 3, 5, 15, 6, 8, 10, 20 });
}

//屋顶，白天
GameLevelData_5::GameLevelData_5()
{
    backgroundImage = "interface/background5.jpg";
    backgroundMusic = "qrc:/audio/Mountains.mp3";

    coord = 2;
    //LF = {3, 3, 3, 3, 3};
    sunNum = 10000; //用于测试
    canSelectCard = true;
    showScroll = true;

    eName = "5";
    cName = tr("Level 5");
    pName = {   "oSunflower", "oTwinSunflower", "oPeashooter", \
                "oRepeater", "oGatlingPea", "oSplitPea", "oThreepeater", "oSnowPea",
                "oTorchwood", "oWallNut", "oTallNut", "oPumpkinHead",
                "oPotatoMine", "oJalapeno",  "oCherryBomb", "oSquash", \
                "oChomper", "oSpikeweed", "oSpikerock", "oSunShroom",\
                "oLilyPad", "oTangleKlep", "oFlowerPot"};
    zombieData = { { "oZombie3", 1, 1, {} }, { "oFlagZombie", 1, 1, {} }, \
                   { "oConeheadZombie", 3, 3, {} }, { "oBucketheadZombie", 3, 3, {} }, \
                   { "oPoleVaultingZombie", 3, 3, {} }, { "oImpZombie", 3, 3, {}}, \
                   { "oFootBallZombie", 3, 3, {}}, { "oNewspaperZombie", 3, 3, {}},\
                   { "oScreenDoorZombie", 3, 3, {}}, { "oJackBoxZombie", 3, 3, {}}};
    flagNum = 20;
    largeWaveFlag = { 6, 12, 19 };
    flagToSumNum = QPair<QList<int>, QList<int> >({ 3, 4, 8, 9, 10, 13, 15, 19 }, { 1, 2, 3, 5, 15, 6, 8, 10, 20 });
}

//屋顶，夜晚
GameLevelData_6::GameLevelData_6()
{
    backgroundImage = "interface/background6boss.jpg";
    backgroundMusic = "qrc:/audio/Mountains.mp3";

    coord = 2;
    sunNum = 10000; //用于测试
    canSelectCard = true;
    showScroll = true;

    eName = "6";
    cName = tr("Level 6");
    pName = {   "oSunflower", "oTwinSunflower", "oPeashooter", \
                "oRepeater", "oGatlingPea", "oSplitPea", "oThreepeater", "oSnowPea",
                "oTorchwood", "oWallNut", "oTallNut", "oPumpkinHead",
                "oPotatoMine", "oJalapeno",  "oCherryBomb", "oSquash", \
                "oChomper", "oSpikeweed", "oSpikerock", "oSunShroom",\
                "oLilyPad", "oTangleKlep", "oFlowerPot"};
    zombieData = { { "oZombie3", 1, 1, {} }, { "oFlagZombie", 1, 1, {} }, \
                   { "oConeheadZombie", 3, 3, {} }, { "oBucketheadZombie", 3, 3, {} }, \
                   { "oPoleVaultingZombie", 3, 3, {} }, { "oImpZombie", 3, 3, {}}, \
                   { "oFootBallZombie", 3, 3, {}}, { "oNewspaperZombie", 3, 3, {}},\
                   { "oScreenDoorZombie", 3, 3, {}}, { "oJackBoxZombie", 3, 3, {}}};
    flagNum = 20;
    largeWaveFlag = { 6, 12, 19 };
    flagToSumNum = QPair<QList<int>, QList<int> >({ 3, 4, 8, 9, 10, 13, 15, 19 }, { 1, 2, 3, 5, 15, 6, 8, 10, 20 });
}


//随机游戏（场景随机， 植物随机）
//选择困难证首选
GameLevelData_7::GameLevelData_7()
{
    int randnum = qrand() % 4;
    if(randnum == 0){
        backgroundImage = "interface/background1.jpg";
        backgroundMusic = "qrc:/audio/UraniwaNi.mp3";
        sunNum = 10000; //用于测试
        canSelectCard = true;
        showScroll = true;
    }
    else if(randnum == 1){
        backgroundImage = "interface/background2.jpg";
        backgroundMusic = "qrc:/audio/UraniwaNi.mp3";
        sunNum = 10000; //用于测试
        canSelectCard = true;
        showScroll = true;
    }
    else if(randnum == 2){
        backgroundImage = "interface/background3.jpg";
        backgroundMusic = "qrc:/audio/UraniwaNi.mp3";

        sunNum = 10000; //用于测试
        coord = 1;  //后花园坐标
        LF = {0, 1, 1, 2, 2, 1, 1};
        canSelectCard = true;
        showScroll = true;
    }
    else if(randnum == 3){
        backgroundImage = "interface/background4.jpg";
        backgroundMusic = "qrc:/audio/Mountains.mp3";

        sunNum = 10000; //用于测试
        coord = 1;  //后花园坐标
        LF = {0, 1, 1, 2, 2, 1, 1};
        canSelectCard = true;
        showScroll = true;
    }

    eName = "7";
    cName = tr("Level 7");
    pName = {  "oHypnoShroom", "oHypnoShroom1", "oHypnoShroom2", "oHypnoShroom3", \
               "oHypnoShroom4", "oHypnoShroom5", "oHypnoShroom6", "oHypnoShroom7"};
    zombieData = { { "oZombie3", 1, 1, {} }, { "oFlagZombie", 1, 1, {} }, \
                   { "oConeheadZombie", 3, 3, {} }, { "oBucketheadZombie", 3, 3, {} }, \
                   { "oPoleVaultingZombie", 3, 3, {} }, { "oImpZombie", 3, 3, {}}, \
                   { "oFootBallZombie", 3, 3, {}}, { "oNewspaperZombie", 3, 3, {}},\
                   { "oScreenDoorZombie", 3, 3, {}}, { "oJackBoxZombie", 3, 3, {}}};

    flagNum = 20;
    largeWaveFlag = { 6, 12, 19 };
    flagToSumNum = QPair<QList<int>, QList<int> >({ 3, 4, 8, 9, 10, 13, 15, 19 }, { 1, 2, 3, 5, 15, 6, 8, 10, 20 });
}

//保龄球游戏
GameLevelData_8::GameLevelData_8()
{
    backgroundImage = "interface/background1.jpg";
    backgroundMusic = "qrc:/audio/UraniwaNi.mp3";
    sunNum = 10000; //用于测试
    canSelectCard = true;
    showScroll = true;

    eName = "8";
    cName = tr("Level 8");
    pName = {"obaolingqiu"};
    zombieData = { { "oZombie3", 1, 1, {} }, { "oFlagZombie", 1, 1, {} }, \
                   { "oConeheadZombie", 3, 3, {} }, { "oBucketheadZombie", 3, 3, {} }, \
                   { "oPoleVaultingZombie", 3, 3, {} }, { "oImpZombie", 3, 3, {}}, \
                   { "oFootBallZombie", 3, 3, {}}, { "oNewspaperZombie", 3, 3, {}},\
                   { "oScreenDoorZombie", 3, 3, {}}, { "oJackBoxZombie", 3, 3, {}}};

    flagNum = 20;
    largeWaveFlag = { 6, 12, 19 };
    flagToSumNum = QPair<QList<int>, QList<int> >({ 3, 4, 8, 9, 10, 13, 15, 19 }, { 1, 2, 3, 5, 15, 6, 8, 10, 20 });
}


GameLevelData *GameLevelDataFactory(const QString &eName)
{
    if(eName == "1")
        return new GameLevelData_1;
    if(eName == "2")
        return new GameLevelData_2;
    if(eName == "3")
        return new GameLevelData_3;
    if(eName == "4")
        return new GameLevelData_4;
    if(eName == "5")
        return new GameLevelData_5;
    if(eName == "6")
        return new GameLevelData_6;
    if(eName == "7")
        return new GameLevelData_7;
    if(eName == "8")
        return new GameLevelData_8;
    return nullptr;
}
