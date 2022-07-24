//
// Created by sun on 8/26/16.
//

#include <QtMultimedia>
#include "Plant.h"
#include "ImageManager.h"
#include "GameScene.h"
#include "GameLevelData.h"
#include "MouseEventPixmapItem.h"
#include "Timer.h"
#include "Animate.h"

//Cleaner
LawnCleaner::LawnCleaner()
{
    eName = "oLawnCleaner";
    cName = tr("Lawn Cleaner");
    beAttackedPointL = 0;
    beAttackedPointR = 71;
    sunNum = 0;
    staticGif = normalGif = "interface/LawnCleaner.png";
    canEat = 0;
    stature = 1;
    toolTip = tr("Normal lawn cleaner");
}


LawnCleanerInstance::LawnCleanerInstance(const Plant *plant)
    : PlantInstance(plant)
{}

void LawnCleanerInstance::initTrigger()
{
    Trigger *trigger = new Trigger(this, attackedLX, attackedRX, 0, 0);
    triggers.insert(row, QList<Trigger *>{ trigger } );
    plantProtoType->scene->addTrigger(row, trigger);
}

void LawnCleanerInstance::triggerCheck(ZombieInstance *zombieInstance, Trigger *trigger)
{
    if (zombieInstance->beAttacked && zombieInstance->altitude > 0) {
        canTrigger = 0;
        normalAttack(nullptr);
    }
}

void LawnCleanerInstance::normalAttack(ZombieInstance *zombieInstance)
{
    QMediaPlayer *player = new QMediaPlayer(plantProtoType->scene);
    player->setMedia(QUrl("qrc:/audio/lawnmower.mp3"));
    player->play();
    QSharedPointer<std::function<void(void)> > crush(new std::function<void(void)>);
    *crush = [this, crush] {
        for (auto zombie: plantProtoType->scene->getZombieOnRowRange(row, attackedLX, attackedRX)) {
            zombie->crushDie();
        }
        if (attackedLX > 900)
            plantProtoType->scene->plantDie(this);
        else {
            attackedLX += 10;
            attackedRX += 10;
            picture->setPos(picture->pos() + QPointF(10, 0));
            (new Timer(picture, 10, *crush))->start();
        }
    };
    (*crush)();
}

PoolCleaner::PoolCleaner()
{
    eName = "oPoolCleaner";
    cName = tr("Pool Cleaner");

    beAttackedPointR = 47;

    staticGif = normalGif = "interface/PoolCleaner.png";
    toolTip = tr("Pool Cleaner");
    update();
}

//Plant

Plant::Plant()
        : hp(300),
          pKind(1), bKind(0),
          beAttackedPointL(20), beAttackedPointR(20),
          zIndex(0),
          canEat(true), canSelect(true), night(false),
          coolTime(1), stature(0), sleep(0), scene(nullptr) //测试：coolTime = 7.5
{}

double Plant::getDX() const
{
    return -0.5 * width;
}

double Plant::getDY(int x, int y) const
{
    return scene->getPlant(x, y).contains(0) ? -21 : -15;
}

bool Plant::canGrow(int x, int y) const
{
    if (x < 1 || x > 9 || y < 1 || y > this->scene->getCoordinate().colCount())
        return false;
    if (scene->isCrater(x, y) || scene->isTombstone(x, y))
        return false;
    int groundType = scene->getGameLevelData()->LF[y];
    QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
    if (groundType == 2 and plants.contains(1))
        return !plants.contains(2);
    if (groundType == 1  )
        return !plants.contains(1);
    return plants.contains(0) && !plants.contains(1);
}


void Plant::update()
{
    QPixmap pic = gImageCache->load(staticGif);
    width = pic.width();
    height = pic.height();
}

PlantInstance::PlantInstance(const Plant *plant) : plantProtoType(plant)
{
    die = false;
    goingDie = false; //难蚌！！！
    uuid = QUuid::createUuid();
    hp = plantProtoType->hp;
    canTrigger = true;
    picture = new MoviePixmapItem;
}

PlantInstance::~PlantInstance()
{
    picture->deleteLater();
}

void PlantInstance::birth(int c, int r)
{
    Coordinate &coordinate = plantProtoType->scene->getCoordinate();
    double x = coordinate.getX(c) + plantProtoType->getDX(), y = coordinate.getY(r) + plantProtoType->getDY(c, r) - plantProtoType->height;
    col = c, row = r;
    attackedLX = x + plantProtoType->beAttackedPointL;
    attackedRX = x + plantProtoType->beAttackedPointR;
    picture->setMovie(plantProtoType->normalGif);
    picture->setPos(x, y);
    picture->setZValue(plantProtoType->zIndex + 3 * r);
    shadowPNG = new QGraphicsPixmapItem(gImageCache->load("interface/plantShadow.png"));
    shadowPNG->setPos(plantProtoType->width * 0.5 - 48, plantProtoType->height - 22);
    shadowPNG->setFlag(QGraphicsItem::ItemStacksBehindParent);
    shadowPNG->setParentItem(picture);
    picture->start();
    plantProtoType->scene->addToGame(picture);
    initTrigger();
}

void PlantInstance::initTrigger()
{
    Trigger *trigger = new Trigger(this, attackedLX, 880, 0, 0);
    triggers.insert(row, QList<Trigger *>{ trigger } );
    plantProtoType->scene->addTrigger(row, trigger);
}

bool PlantInstance::contains(const QPointF &pos)
{
    return picture->contains(pos);
}

void PlantInstance::triggerCheck(ZombieInstance *zombieInstance, Trigger *trigger)
{
    if (zombieInstance->altitude > 0) {
        canTrigger = false;
        QSharedPointer<std::function<void(QUuid)> > triggerCheck(new std::function<void(QUuid)>);
        *triggerCheck = [this, triggerCheck] (QUuid zombieUuid) {
            (new Timer(picture, 1400, [this, zombieUuid, triggerCheck] {
                ZombieInstance *zombie = this->plantProtoType->scene->getZombie(zombieUuid);
                if (zombie) {
                    for (auto i: triggers[zombie->row]) {
                        if (zombie->hp > 0 && i->from <= zombie->ZX && i->to >= zombie->ZX && zombie->altitude > 0) {
                            normalAttack(zombie);
                            (*triggerCheck)(zombie->uuid);
                            return;
                        }
                    }
                }
                canTrigger = true;
            }))->start();
        };
        normalAttack(zombieInstance);
        (*triggerCheck)(zombieInstance->uuid);
    }
}

void PlantInstance::normalAttack(ZombieInstance *zombieInstance)
{
    qDebug() << plantProtoType->cName << uuid << "Attack" << zombieInstance->zombieProtoType->cName << zombieInstance;
}

void PlantInstance::getHurt(ZombieInstance *zombie, int aKind, int attack)
{
    if (aKind == 0)
        hp -= attack;
    if (hp < 1 || aKind != 0)
        plantProtoType->scene->plantDie(this);
}

//Bullet

Bullet::Bullet(GameScene *scene, int type, int row, qreal from, qreal x, qreal y, qreal zvalue, int direction)
        : scene(scene), type(type), row(row), direction(direction), from(from)
{
    count = 0;
    picture = new QGraphicsPixmapItem(gImageCache->load(QString("Plants/PB%1%2.gif").arg(type).arg(direction)));
    picture->setPos(x, y);
    picture->setZValue(zvalue);
}

Bullet::~Bullet()
{
    delete picture;
}

void Bullet::start()
{
    (new Timer(scene, 10, [this] {
        move();
    }))->start();
}


void Bullet::move()
{
    if (count++ == 5)
        scene->addItem(picture);
    int col = scene->getCoordinate().getCol(from);
    QMap<int, PlantInstance *> plants = scene->getPlant(col, row);

    /*
    //保龄球
    if(type == -2){
        picture->setPixmap(gImageCache->load(QString("Plants/ShroomBullet.gif")));
    }
    */
    if(type == 0 && plants.contains(1) && plants[1]->plantProtoType->eName == "oTorchwood" && uuid != plants[1]->uuid){
        uuid = plants[1]->uuid;
        picture->setPixmap(gImageCache->load(QString("Plants/PB10.gif")));
    }
    if (type == -1 && plants.contains(1) && plants[1]->plantProtoType->eName == "oTorchwood" && uuid != plants[1]->uuid) {
        ++type;
        uuid = plants[1]->uuid;
        picture->setPixmap(gImageCache->load(QString("Plants/PB%1%2.gif").arg(type).arg(direction)));
    }
    ZombieInstance *zombie = nullptr;
    if (direction == 0) {
        QList<ZombieInstance *> zombies = scene->getZombieOnRow(row);
        for (auto iter = zombies.end(); iter-- != zombies.begin() && (*iter)->attackedLX <= from;) {
            if ((*iter)->hp > 0 && (*iter)->attackedRX >= from) {
                zombie = *iter;
                break;
            }
        }
    }
    //another direction
    else if(direction == 1){
        QList<ZombieInstance *> zombies = scene->getZombieOnRow(row);
        for (auto iter = zombies.end(); iter-- != zombies.begin() && (*iter)->attackedLX >= from;) {
            if ((*iter)->hp > 0 && (*iter)->attackedRX >= from) {
                zombie = *iter;
                break;
            }
        }
    }
    if (zombie && zombie->altitude == 1 && !zombie->goingDie) {
        // TODO: other attacks

        /*
        //保龄球
        if(type == -2)
            zombie->getBaoling();
        */

        if(type == 0)
            zombie->getPea(20, direction);
        else if(type == -1)
            zombie->getSnowPea(20, direction);
        if(type == 1)
            zombie->getFirePea(40, direction);
        //zombie->getPea(20, direction);
        picture->setPos(picture->pos() + QPointF(28, 0));
        picture->setPixmap(gImageCache->load("Plants/PeaBulletHit.gif"));
        (new Timer(scene, 100, [this] {
            delete this;
        }))->start();
    }
    else {
        from += direction ? -5 : 5;
        if (from < 900 && from > 100) {
            picture->setPos(picture->pos() + QPointF(direction ? -5 : 5, 0));
            (new Timer(scene, 10, [this] {
                move();
            }))->start();
        }
        else
            delete this;
    }
}

//SunFlower

SunFlower::SunFlower()
{
    eName = "oSunflower";
    cName = tr("Sunflower");

    beAttackedPointR = 53;
    sunNum = 50;

    cardGif = "Card/Plants/SunFlower.png";
    staticGif = "Plants/SunFlower/0.gif";
    normalGif = "Plants/SunFlower/SunFlower1.gif";
    toolTip = tr("产生阳光");
}

SunFlowerInstance::SunFlowerInstance(const Plant *plant)
        : PlantInstance(plant),
          lightedGif("Plants/SunFlower/SunFlower2.gif")
{

}

//初始化触发器
void SunFlowerInstance::initTrigger()
{
    (new Timer(picture, 5000, [this] {
        QSharedPointer<std::function<void(void)> > generateSun(new std::function<void(void)>);
        *generateSun = [this, generateSun] {
            picture->setMovieOnNewLoop(lightedGif, [this, generateSun] {
                (new Timer(picture, 1000, [this, generateSun] {
                    auto sunGifAndOnFinished = plantProtoType->scene->newSun(25);
                    MoviePixmapItem *sunGif = sunGifAndOnFinished.first;
                    std::function<void(bool)> onFinished = sunGifAndOnFinished.second;
                    Coordinate &coordinate = plantProtoType->scene->getCoordinate();
                    double fromX = coordinate.getX(col) - sunGif->boundingRect().width() / 2 + 15,
                            toX = coordinate.getX(col) - qrand() % 80,
                            toY = coordinate.getY(row) - sunGif->boundingRect().height();
                    sunGif->setScale(0.6);
                    sunGif->setPos(fromX, toY - 25);
                    sunGif->start();
                    Animate(sunGif).move(QPointF((fromX + toX) / 2, toY - 50)).scale(0.9).speed(0.2).shape(
                                    QTimeLine::EaseOutCurve).finish()
                            .move(QPointF(toX, toY)).scale(1.0).speed(0.2).shape(QTimeLine::EaseInCurve).finish(
                                    onFinished);
                    picture->setMovieOnNewLoop(plantProtoType->normalGif, [this, generateSun] {
                        (new Timer(picture, 24000, [this, generateSun] {
                            (*generateSun)();
                        }))->start();
                    });
                }))->start();
            });
        };
        (*generateSun)();
    }))->start();
}


//SunShroom

SunShroom::SunShroom()
{
    eName = "oSunShroom";
    cName = tr("SunShroom");

    beAttackedPointR = 53;
    sunNum = 0;

    cardGif = "Card/Plants/SunShroom.png";
    staticGif = "Plants/SunShroom/0.gif";
    normalGif = "Plants/SunShroom/SunShroom.gif";
    toolTip = tr("产生阳光");
}

SunShroomInstance::SunShroomInstance(const Plant *plant)
        : PlantInstance(plant),
          lightedGif("Plants/SunShroom/SunShroom2.gif")
{}

//初始化触发器
void SunShroomInstance::initTrigger()
{
    (new Timer(picture, 5000, [this] {
        QSharedPointer<std::function<void(void)> > generateSun(new std::function<void(void)>);
        *generateSun = [this, generateSun] {
            picture->setMovieOnNewLoop(lightedGif, [this, generateSun] {
                (new Timer(picture, 1000, [this, generateSun] {
                    auto sunGifAndOnFinished = plantProtoType->scene->newSun(25);
                    MoviePixmapItem *sunGif = sunGifAndOnFinished.first;
                    std::function<void(bool)> onFinished = sunGifAndOnFinished.second;
                    Coordinate &coordinate = plantProtoType->scene->getCoordinate();
                    double fromX = coordinate.getX(col) - sunGif->boundingRect().width() / 2 + 15,
                            toX = coordinate.getX(col) - qrand() % 80,
                            toY = coordinate.getY(row) - sunGif->boundingRect().height();
                    sunGif->setScale(0.6);
                    sunGif->setPos(fromX, toY - 25);
                    sunGif->start();
                    Animate(sunGif).move(QPointF((fromX + toX) / 2, toY - 50)).scale(0.9).speed(0.2).shape(
                                    QTimeLine::EaseOutCurve).finish()
                            .move(QPointF(toX, toY)).scale(1.0).speed(0.2).shape(QTimeLine::EaseInCurve).finish(
                                    onFinished);
                    picture->setMovieOnNewLoop(plantProtoType->normalGif, [this, generateSun] {
                        (new Timer(picture, 24000, [this, generateSun] {
                            (*generateSun)();
                        }))->start();
                    });
                }))->start();
            });
        };
        (*generateSun)();
    }))->start();
}


//TwinSunFlower

TwinSunflower::TwinSunflower()
{
    eName = "oTwinSunflower";
    cName = tr("TwinSunflower");

    beAttackedPointR = 53;
    sunNum = 0;

    cardGif = "Card/Plants/TwinSunflower.png";
    staticGif = "Plants/TwinSunflower/0.gif";
    normalGif = "Plants/TwinSunflower/TwinSunflower1.gif";
    toolTip = tr("一次产生两个阳光（种植在太阳花上）");
}

TwinSunflowerInstance::TwinSunflowerInstance(const Plant *plant)
        : SunFlowerInstance(plant),
          lightedGif("Plants/TwinSunflower/TwinSunflower2.gif")
{}

void TwinSunflowerInstance::initTrigger()
{
    for(int i = 0; i < 2; i++){
    (new Timer(picture, 5000, [this] {
        QSharedPointer<std::function<void(void)> > generateSun(new std::function<void(void)>);
        *generateSun = [this, generateSun] {
            picture->setMovieOnNewLoop(lightedGif, [this, generateSun] {
                (new Timer(picture, 1000, [this, generateSun] {
                    auto sunGifAndOnFinished = plantProtoType->scene->newSun(25);
                    MoviePixmapItem *sunGif = sunGifAndOnFinished.first;
                    std::function<void(bool)> onFinished = sunGifAndOnFinished.second;
                    Coordinate &coordinate = plantProtoType->scene->getCoordinate();
                    double fromX = coordinate.getX(col) - sunGif->boundingRect().width() / 2 + 15,
                            toX = coordinate.getX(col) - qrand() % 80,
                            toY = coordinate.getY(row) - sunGif->boundingRect().height();
                    sunGif->setScale(0.6);
                    sunGif->setPos(fromX, toY - 25);
                    sunGif->start();
                    Animate(sunGif).move(QPointF((fromX + toX) / 2, toY - 50)).scale(0.9).speed(0.2).shape(
                                    QTimeLine::EaseOutCurve).finish()
                            .move(QPointF(toX, toY)).scale(1.0).speed(0.2).shape(QTimeLine::EaseInCurve).finish(
                                    onFinished);
                    picture->setMovieOnNewLoop(plantProtoType->normalGif, [this, generateSun] {
                        (new Timer(picture, 24000, [this, generateSun] {
                            (*generateSun)();
                        }))->start();
                    });
                }))->start();
            });
        };
        (*generateSun)();
    }))->start();
    }
}

//种植在太阳花上
bool TwinSunflower::canGrow(int x, int y) const
{
    if (x < 1 || x > 9 || y < 1 || y > 5)
         return false;
     if (scene->isCrater(x, y) || scene->isTombstone(x, y))
         return false;
     int groundType = scene->getGameLevelData()->LF[y];
     QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
     //contains
     //Returns true if the map contains an item with key key; otherwise returns false.
     //如果映射包含具有键的项，则返回true；否则返回false。
     if(groundType == 1 && plants.contains(1) == 0)
         return  false;
     if (groundType == 1)
         return !plants.contains(1) || plants[1]->plantProtoType->eName == "oSunflower";
     return plants.contains(0) && (!plants.contains(1) || plants[1]->plantProtoType->eName == "oSunflower");
}



//Peashooter

Peashooter::Peashooter()
{
    eName = "oPeashooter";
    cName = tr("Peashooter");

    beAttackedPointR = 51;
    sunNum = 0; //100

    cardGif = "Card/Plants/Peashooter.png";
    staticGif = "Plants/Peashooter/0.gif";
    normalGif = "Plants/Peashooter/Peashooter.gif";
    toolTip = tr("发射豌豆");
}

PeashooterInstance::PeashooterInstance(const Plant *plant)
    : PlantInstance(plant), firePea(new QMediaPlayer(picture))
{
    firePea->setMedia(QUrl("qrc:/audio/firepea.mp3"));
}

void PeashooterInstance::normalAttack(ZombieInstance *zombieInstance)
{
    firePea->play();
    (new Bullet(plantProtoType->scene, 0, row, attackedLX, attackedLX - 40, picture->y() + 3, picture->zValue() + 2, 0))->start();
}


//Repeater

Repeater::Repeater()
{
    eName = "oRepeater";
    cName = tr("Repeater");
    beAttackedPointR = 51;
    sunNum = 200;
    cardGif = "Card/Plants/Repeater.png";
    staticGif = "Plants/Repeater/0.gif";
    normalGif = "Plants/Repeater/Repeater.gif";
    toolTip = tr("双发豌豆（种植在豌豆射手上）");
}

//种植在豌豆射手上
bool Repeater::canGrow(int x, int y) const
{
    if (x < 1 || 9 < x|| y < 1 || this->scene->getCoordinate().colCount() < y)
        return false;
     if (scene->isCrater(x, y) || scene->isTombstone(x, y))
         return false;
     int groundType = scene->getGameLevelData()->LF[y];
     QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
     if(groundType == 1 &&plants.contains(1) == 0)
         return  false;
     if (groundType == 1)
         return !plants.contains(1) || plants[1]->plantProtoType->eName == "oPeashooter";
     return plants.contains(0) && (!plants.contains(1) || plants[1]->plantProtoType->eName == "oPeashooter");
}

RepeaterInstance::RepeaterInstance(const Plant *plant)
    : PlantInstance(plant), firePea(new QMediaPlayer(picture))
{
    firePea->setMedia(QUrl("qrc:/audio/firepea.mp3"));
}

//延迟
void sleep(unsigned int mm)
{
    QTime dieTime = QTime::currentTime().addMSecs(mm);
    while(QTime::currentTime() <= dieTime){
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
    }
}

void RepeaterInstance::normalAttack(ZombieInstance *zombieInstance)
{
    firePea->play();
    (new Bullet(plantProtoType->scene, 0, row, attackedLX, attackedLX - 40, picture->y() + 3, picture->zValue() + 2, 0))->start();
    sleep(250); //延迟0.25s
    (new Bullet(plantProtoType->scene, 0, row, attackedLX, attackedLX - 80, picture->y() + 3, picture->zValue() + 2, 0))->start();
}

//GatlingPea

GatlingPea::GatlingPea()
{
    eName = "oGatlingPea";
    cName = tr("GatlingPea");
    beAttackedPointR = 51;
    sunNum = 300;
    cardGif = "Card/Plants/GatlingPea.png";
    staticGif = "Plants/GatlingPea/0.gif";
    normalGif = "Plants/GatlingPea/GatlingPea.gif";
    toolTip = tr("射击僵尸（种植在双射豌豆上）");
}

//种植在双射豌豆上
bool GatlingPea::canGrow(int x, int y) const
{
    if (x < 1 ||  9 < x || y < 1 || this->scene->getCoordinate().colCount() < y)
         return false;
     if (scene->isCrater(x, y) || scene->isTombstone(x, y))
         return false;
     int groundType = scene->getGameLevelData()->LF[y];
     QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
     if(groundType == 1 &&plants.contains(1) == 0)
         return  false;
     if (groundType == 1)
         return !plants.contains(1) || plants[1]->plantProtoType->eName == "oRepeater";
     return plants.contains(0) && (!plants.contains(1) || plants[1]->plantProtoType->eName == "oRepeater");
}


GatlingPeaInstance::GatlingPeaInstance(const Plant *plant)
    : PlantInstance(plant), firePea(new QMediaPlayer(picture))
{
    firePea->setMedia(QUrl("qrc:/audio/firepea.mp3"));
}

void GatlingPeaInstance::normalAttack(ZombieInstance *zombieInstance)
{
    firePea->play();
    (new Bullet(plantProtoType->scene, 0, row, attackedLX, attackedLX - 40, picture->y() + 3, picture->zValue() + 2, 0))->start();
    sleep(250);
    (new Bullet(plantProtoType->scene, 0, row, attackedLX, attackedLX - 80, picture->y() + 3, picture->zValue() + 2, 0))->start();
    sleep(250);
    (new Bullet(plantProtoType->scene, 0, row, attackedLX, attackedLX - 80, picture->y() + 3, picture->zValue() + 2, 0))->start();
    sleep(250);
    (new Bullet(plantProtoType->scene, 0, row, attackedLX, attackedLX - 80, picture->y() + 3, picture->zValue() + 2, 0))->start();
}

//SplitPea

SplitPea::SplitPea()
{
    eName = "oSplitPea";
    cName = tr("SplitPea");

    beAttackedPointR = 51;
    sunNum = 0;

    cardGif = "Card/Plants/SplitPea.png";
    staticGif = "Plants/SplitPea/0.gif";
    normalGif = "Plants/SplitPea/SplitPea.gif";
    toolTip = tr("两个方向攻击僵尸");
}

SplitPeaInstance::SplitPeaInstance(const Plant *plant)
    :PlantInstance(plant), firePea(new QMediaPlayer(picture))
{
    firePea->setMedia(QUrl("qrc:/audio/firepea.mp3"));
}

void SplitPeaInstance::normalAttack(ZombieInstance *zombieInstance)
{
    firePea->play();
    QTime time0;
    (new Bullet(plantProtoType->scene, 0, row, attackedLX, attackedLX - 40, picture->y() + 3, picture->zValue() + 2, 0))->start();
    time0.start();
    while(time0.elapsed()<80)
        QCoreApplication :: processEvents();
    (new Bullet(plantProtoType->scene, 0, row, attackedLX, attackedLX - 40, picture->y() + 3, picture->zValue() + 2, 1))->start();
}


//Threepeater

Threepeater::Threepeater()
{
    eName = "oThreepeater";
    cName = tr("Threepeater");

    beAttackedPointR = 51;
    sunNum = 0;

    cardGif = "Card/Plants/Threepeater.png";
    staticGif = "Plants/Threepeater/0.gif";
    normalGif = "Plants/Threepeater/Threepeater.gif";
    toolTip = tr("攻击三行僵尸");
}

ThreepeaterInstance::ThreepeaterInstance(const Plant *plant)
    : PlantInstance(plant)
{}
void ThreepeaterInstance::initTrigger(){
    bool flag = 0;
    for(int i=-1;i<=1;i++){
        if(row + i >= 1 && row +i <=5){
            QList<ZombieInstance *> zombies = plantProtoType->scene->getZombieOnRow(row+i);
            if(zombies.empty()) continue;
            Coordinate &coordinate = plantProtoType->scene->getCoordinate();
            double x= coordinate.getX(col) + plantProtoType->getDX();
            for(auto zombie: zombies){
                if(zombie->hp > 0 && zombie->ZX >= x && zombie->ZX <= 900 && zombie->altitude > 0){
                    flag = 1;
                    break;
                }
            }
        }
    }
    if(flag){
        for(int i=-1;i<=1;i++){
            if(row + i >= 1 and row +i <=5){
                (new Bullet(plantProtoType->scene, 0, row+i, attackedLX,
                            attackedLX - 70, picture->y()+10+90*i, picture->zValue() + 2, 0))->start();
            }
        }
    }
    (new Timer(picture, 1900, [this]{
        initTrigger();
    }))->start();
    return;
}


//SnowPeaInstance

SnowPea::SnowPea()
{
    eName = "oSnowPea";
    cName = tr("Snow Pea");
    bKind = -1;
    beAttackedPointR = 51;
    sunNum = 175;
    cardGif = "Card/Plants/SnowPea.png";
    staticGif = "Plants/SnowPea/0.gif";
    normalGif = "Plants/SnowPea/SnowPea.gif";
    toolTip = tr("使僵尸减速");
}

SnowPeaInstance::SnowPeaInstance(const Plant *plant)
    : PlantInstance(plant), firePea(new QMediaPlayer(picture))
{
    firePea->setMedia(QUrl("qrc:/audio/firepea.mp3"));
}

void SnowPeaInstance::normalAttack(ZombieInstance *zombieInstance)
{
    //QSound::play(":/audio/firepea.wav");
    (new Bullet(plantProtoType->scene, -1, row, attackedLX, attackedLX - 40, picture->y() + 3, picture->zValue() + 2, 0))->start();
}

//Torchwood

Torchwood::Torchwood()
{
    eName = "oTorchwood";
    cName = tr("Torchwood");
    beAttackedPointR = 53;
    sunNum = 175;
    toolTip = tr("使豌豆变成火豌豆");
    cardGif = "Card/Plants/Torchwood.png";
    staticGif = "Plants/Torchwood/0.gif";
    normalGif = "Plants/Torchwood/Torchwood.gif";
}

TorchwoodInstance::TorchwoodInstance(const Plant *plant)
    : PlantInstance(plant)
{}

void TorchwoodInstance::initTrigger()
{}


//WallNut

WallNut::WallNut()
{
    eName = "oWallNut";
    cName = tr("Wall-nut");
    hp = 4000;
    beAttackedPointR = 45;
    sunNum = 50;
    cardGif = "Card/Plants/WallNut.png";
    staticGif = "Plants/WallNut/0.gif";
    normalGif = "Plants/WallNut/WallNut.gif";
    toolTip = tr("用外壳阻止僵尸");
}

bool WallNut::canGrow(int x, int y) const
{
    if (x < 1 || 9 < x|| y < 1 || this->scene->getCoordinate().colCount() < y)
        return false;
    if (scene->isCrater(x, y) || scene->isTombstone(x, y))
        return false;
    int groundType = scene->getGameLevelData()->LF[y];
    QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
    if (groundType == 1)
        return !plants.contains(1) || plants[1]->plantProtoType->eName == "oWallNut";
    return plants.contains(0) && (!plants.contains(1) || plants[1]->plantProtoType->eName == "oWallNut");

}


void WallNutInstance::initTrigger()
{}

WallNutInstance::WallNutInstance(const Plant *plant)
    : PlantInstance(plant)
{
    hurtStatus = 0;
    crackedGif1 = "Plants/WallNut/Wallnut_cracked1.gif";
    crackedGif2 = "Plants/WallNut/Wallnut_cracked2.gif";
}

void WallNutInstance::getHurt(ZombieInstance *zombie, int aKind, int attack)
{
    PlantInstance::getHurt(zombie, aKind, attack);
    if (hp > 0) {
        if (hp < 1334) {
            if (hurtStatus < 2) {
                hurtStatus = 2;
                picture->setMovie(crackedGif2);
                picture->start();
            }
        }
        else if (hp < 2667) {
            if (hurtStatus < 1) {
                hurtStatus = 1;
                picture->setMovie(crackedGif1);
                picture->start();
            }
        }
    }
}

//TallWallNut

TallNut::TallNut()
{
    eName = "oTallNut";
    cName = tr("Tall Nut");
    beAttackedPointR = 63;
    sunNum = 125;
    hp = 8000;
    toolTip = tr("Heavy-duty wall palnts that can't be vaulted or jumped over");
    cardGif = "Card/Plants/TallNut.png";
    staticGif = "Plants/TallNut/0.gif";
    normalGif = "Plants/TallNut/TallNut.gif";
    stature = 1;
}

bool TallNut::canGrow(int x, int y) const
{
    if (x < 1 || 9 < x|| y < 1 || this->scene->getCoordinate().colCount() < y)
        return false;
    if(scene->isCrater(x, y) || scene->isTombstone(x, y))
        return false;
    int groundType = scene->getGameLevelData()->LF[y];
    QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
    if(groundType == 1)
        return !plants.contains(1) || plants[1]->plantProtoType->eName == "oTallNut";
    return plants.contains(0) && (!plants.contains(1) || plants[1]->plantProtoType->eName == "oTallNut");
}

void TallNutInstance::getHurt(ZombieInstance *zombie, int aKind, int attack)
{
    PlantInstance::getHurt(zombie, aKind, attack);
    if(hp > 0){
        if(hp < 1334){
            if(hurtStatus < 2){
                hurtStatus = 2;
                picture->setMovie("Plants/TallNut/TallnutCracked2.gif");
                picture->start();
            }
        }
        else if(hp < 2667){
            if(hurtStatus < 1){
                hurtStatus = 1;
                picture->setMovie("Plants/TallNut/TallnutCracked1.gif");
                picture->start();
            }
        }
    }
}

TallNutInstance::TallNutInstance(const Plant *plant)
    :WallNutInstance(plant)
{
    hurtStatus = 0;
}

//PumpkinHead

PumpkinHead::PumpkinHead()
{
    eName = "oPumpkinHead";
    cName = tr("Pumplin Head");
    beAttackedPointL = 15;
    beAttackedPointR = 82;
    sunNum = 125;
    pKind = 2;
    hp = 4000;
    zIndex = 1;
    toolTip = tr("保护在里面的植物");
    cardGif = "Card/Plants/PumpkinHead.png";
    staticGif = "Plants/PumpkinHead/0.gif";
    normalGif = "Plants/PumpkinHead/PumpkinHead1.gif";
}

bool PumpkinHead::canGrow(int x, int y) const
{
    QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
    if(plants.contains(pKind))
      return true;
    if (x < 1 || 9 < x|| y < 1 || this->scene->getCoordinate().colCount() < y)
        return false;
    if(scene->isCrater(x, y) || scene->isTombstone(x, y))
      return false;
    int groundType = scene->getGameLevelData()->LF[y];
    if(groundType == 2)
        return plants.contains(0);
    return true;
}

PumpkinHeadInstance::PumpkinHeadInstance(const Plant *plant)
    : PlantInstance(plant), picture2(new MoviePixmapItem)
{
    hurtStatus = 0;
}

double PumpkinHead::getDY(int x, int y) const
{
    return scene->getPlant(x, y).contains(0) ? -12 : -5;
}

void PumpkinHeadInstance::birth(int c, int r)
{
    Coordinate &coordinate = plantProtoType->scene->getCoordinate();
    double x = coordinate.getX(c) + plantProtoType->getDX(), y = coordinate.getY(r) + plantProtoType->getDY(c, r) - plantProtoType->height;
    col = c, row = r;
    attackedLX = x + plantProtoType->beAttackedPointL;
    attackedRX = x + plantProtoType->beAttackedPointR;
    picture->setMovie(plantProtoType->normalGif);
    picture->setPos(x, y);
    picture->setZValue(plantProtoType->zIndex + 3 * r);
    shadowPNG = new QGraphicsPixmapItem(gImageCache->load("interface/plantShadow.png"));
    shadowPNG->setPos(plantProtoType->width * 0.5 -48, plantProtoType->height - 22);
    shadowPNG->setFlag(QGraphicsItem::ItemStacksBehindParent);
    shadowPNG->setParentItem(picture);
    picture->start();
    plantProtoType->scene->addToGame(picture);
    picture2->setMovie("Plants/PumpkinHead/PumpkinHead2.gif");
    picture2->setPos(picture->pos());
    picture2->setZValue(picture->zValue() - 2);
    plantProtoType->scene->addToGame(picture2);
    picture2->start();
}

void PumpkinHeadInstance:: getHurt(ZombieInstance *zombie, int aKind, int attack)
{
    PlantInstance::getHurt(zombie, aKind, attack);
    if(hp > 0){
        if(hp < 1334){
            if(hurtStatus < 2){
                hurtStatus = 2;
                picture->setMovie("Plants/PumpkinHead/Pumpkin_damage2.gif");
                picture->start();
            }
        }
        else if(hp < 2667){
            if(hurtStatus < 1){
                hurtStatus = 1;
                picture->setMovie("Plants/PumpkinHead/pumpkin_damage1.gif");
                picture->start();
                picture2->setMovie("Plants/PumpkinHead/Pumpkin_back.gif");
                picture2->start();
            }
        }
    }
}

PumpkinHeadInstance::~PumpkinHeadInstance()
{
    picture2->deleteLater();
}

//PotatoMine

PotatoMine::PotatoMine()
{
    eName = "oPotatoMine";
    cName = tr("PotatoMine");

    beAttackedPointR = 35;
    sunNum = 25;

    cardGif = "Card/Plants/PotatoMine.png";
    staticGif = "Plants/PotatoMine/0.gif";
    normalGif = "Plants/PotatoMine/PotatoMineNotReady.gif";
    toolTip = tr("炸飞踩在上面的僵尸");
}

PotatoMineInstance::PotatoMineInstance(const Plant *plant)
    :PlantInstance(plant),isBoom(false),
      boomMusic(new QMediaPlayer)
{
    red1Gif="Plants/PotatoMine/PotatoMine_mashed.gif";
    red2Gif="Plants/PotatoMine/ExplosionSpudow.gif";
}

void PotatoMineInstance::initTrigger()
{
    (new Timer(picture,4000,[this]{
            picture->setMovie("Plants/PotatoMine/PotatoMine.gif");
            picture->start();
            isBoom=!isBoom;
    }))->start();
}

void PotatoMineInstance::getHurt(ZombieInstance *zombie,int aKind, int attack)
{
    if(isBoom){
        boomMusic->setMedia(QUrl("qrc:/audio/potato_mine.mp3"));
        boomMusic->play();
        picture->setMovie(red1Gif);
        picture->start();
        (new Timer(picture,1000,[this]{
            this->plantProtoType->scene->plantDie(this);
        }))->start();
        zombie->boomDie();  //在此调用动画效果更好
    }
}

//CherryBomb

CherryBomb::CherryBomb()
{
    eName = "oCherryBomb";
    cName = tr("CherryBomb");

    sunNum = 0;

    cardGif = "Card/Plants/CherryBomb.png";
    staticGif = "Plants/CherryBomb/0.gif";
    normalGif = "Plants/CherryBomb/CherryBomb.gif";
    toolTip = tr("炸飞四周的僵尸");
}

CherryBombInstance::CherryBombInstance(const Plant *plant)
    :PlantInstance(plant), boom(new QMediaPlayer)
{
    boom->setMedia(QUrl("qrc:/audio/cherrybomb.mp3"));
    boomGif = "Plants/CherryBomb/Boom.gif";
}

void CherryBombInstance::initTrigger()
{
    boom->play();
    (new Timer(picture, 500, [this]{
        Coordinate &coordinate = plantProtoType->scene->getCoordinate();
        double y = coordinate.getY(row) + plantProtoType->getDY(col, row) - plantProtoType->height,\
               x = coordinate.getX(col) + plantProtoType->getDX();
        picture->setPos(x - 30, y - 30);
        picture->setMovie(boomGif);
        normalAttack(); //攻击四周
        (new Timer(picture, 1000, [this]{
            this->plantProtoType->scene->plantDie(this);
        }))->start();
    }))->start();
}

void CherryBombInstance::normalAttack()
{
    //四周攻击
    //Y
    for(int i = -1; i <= 1; i++){
        if(1 <= row + i && row + i <= this->plantProtoType->scene->getCoordinate().colCount() ){ //后花园BUG（解决）
            QList<ZombieInstance *> zombies = plantProtoType->scene->getZombieOnRow(row + i);
            //X
            for(auto zombie: zombies){
                int z_x = zombie->zombieProtoType->scene->getCoordinate().getCol(zombie->ZX);
                if(col - 1 <= z_x && z_x <= col + 1)
                    if(zombie) //难蚌！!!
                        zombie->boomDie();
            }
        }
    }
}


//Jalapeno

Jalapeno::Jalapeno()
{
    eName = "oJalapeno";
    cName = tr("Jalapeno");

    sunNum = 0;

    cardGif = "Card/Plants/Jalapeno.png";
    staticGif = "Plants/Jalapeno/0.gif";
    normalGif = "Plants/Jalapeno/Jalapeno.gif";
    toolTip = tr("燃烧一行的僵尸");
}

JalapenoInstance::JalapenoInstance(const Plant *plant)
    :PlantInstance(plant),
    burn(new QMediaPlayer)
{
    burn->setMedia(QUrl("qrc:/audio/jalapeno.mp3"));
    burnGif="Plants/Jalapeno/JalapenoAttack.gif";
}

void JalapenoInstance::initTrigger()
{
    burn->play(); //先播放声音更流畅
    (new Timer(picture, 500, [this]{
        Coordinate &coordinate = plantProtoType->scene->getCoordinate();
        double y = coordinate.getY(row) + plantProtoType->getDY(col, row) - plantProtoType->height,\
               x = coordinate.getX(col) + plantProtoType->getDX();
        picture->setPos(130, y - 20);
        picture->setMovie(burnGif);
        normalAttack(); //攻击一行
        (new Timer(picture, 1000, [this]{
            this->plantProtoType->scene->plantDie(this);
        }))->start();
    }))->start();
}

void JalapenoInstance::normalAttack()
{
    QList<ZombieInstance *> zombies = plantProtoType->scene->getZombieOnRow(row);
    for(auto zombie: zombies){
        zombie->boomDie();
    }
}


//Squash

Squash::Squash()
{
    eName = "oSquash";
    cName = tr("Squash");

    sunNum = 0;

    cardGif = "Card/Plants/Squash.png";
    staticGif = "Plants/Squash/0.gif";
    normalGif = "Plants/Squash/Squash.gif";
    toolTip = tr("Crush the zombies nearby");
}

SquashInstance::SquashInstance(const Plant *plant)
    :PlantInstance(plant), bmmMusic(new QMediaPlayer(picture))
{
    bmml = "Plants/Squash/SquashL.PNG";
    bmmr = "Plants/Squash/SquashR.png";
    bmmattack = "Plants/Squash/SquashAttack.gif";
    bmmMusic->setMedia(QUrl("qrc:/audio/squash_hmm.mp3"));
}

void SquashInstance::initTrigger()
{
    QList<ZombieInstance *> zombies = plantProtoType->scene->getZombieOnRow(row);
    for(auto zombie: zombies){
        int z_col = zombie->zombieProtoType->scene->getCoordinate().getCol(zombie->ZX);
        if(z_col >= col - 1 && z_col <= col + 1){
            Coordinate &coordinate = plantProtoType->scene->getCoordinate();
            double x = coordinate.getX(col) + plantProtoType->getDX();
            if(zombie->ZX >= x)
                picture->setMovie(bmmr);
            else
                picture->setMovie(bmml);
            bmmAttack(zombie);
            break;
        }
    }
    (new Timer(picture, 400, [this]{
        initTrigger();
    }))->start();
}

void SquashInstance::bmmAttack(ZombieInstance *zombie)
{
    bmmMusic->play();
    Coordinate &coordinate = plantProtoType->scene->getCoordinate();
    double y = coordinate.getY(row) + plantProtoType->getDY(col, row) - plantProtoType->height;
    (new Timer(picture, 500, [this , zombie, y]{
          picture->setPos(zombie->ZX,y);
          picture->setMovie(bmmattack);
          picture->start();
          (new Timer(picture, 200, [this , zombie]{
              zombie->normalDie();
              this->plantProtoType->scene->plantDie(this);
          }))->start();
      }))->start();
}


//Chomper

Chomper::Chomper()
{
    eName = "oChomper";
    cName = tr("Chomper");

    beAttackedPointR = 85;
    sunNum = 0;

    cardGif = "Card/Plants/Chomper.png";
    staticGif = "Plants/Chomper/0.gif";
    normalGif = "Plants/Chomper/Chomper.gif";
    toolTip = tr("吃掉旁边的僵尸");
}

ChomperInstance::ChomperInstance(const Plant *plant)
    : PlantInstance(plant), digest1(new QMediaPlayer(picture)), digest2(new QMediaPlayer(picture)),
      digesting(false)
{
    AttackGif1 = "Plants/Chomper/ChomperAttack.gif";
    DigestGif1 = "Plants/Chomper/ChomperDigest.gif";
    digest1->setMedia(QUrl("qrc:/audio/chomp.mp3"));
    digest2->setMedia(QUrl("qrc:/audio/chompsoft.mp3"));
}

void ChomperInstance::initTrigger()
{
    bool can = false;

    //找到被吃的僵尸
    QList<ZombieInstance *> zombies = plantProtoType->scene->getZombieOnRow(row);
    ZombieInstance *zombieBeEaten = nullptr;
    for(auto zombie:zombies){
        if(zombie->hp > 0){ //防止被多吃
            int z_col = zombie->zombieProtoType->scene->getCoordinate().getCol(zombie->ZX);
            if(z_col == col+1 || z_col == col){ //?吃哪一行
                can = true;
                zombieBeEaten = zombie;
                break;
            }
        }
    }

    //开吃
    if(!digesting && can && zombieBeEaten){
        zombieBeEaten->normalDie();//防止一个僵尸被两个食人花吃，崩溃
        picture->setMovie(AttackGif1);
        picture->start();

       // (new Timer(picture,500,[this,zombieBeEaten]{
       //     delete zombieBeEaten;
       // }))->start();

        //咀嚼
        (new Timer(picture,1000,[this]{
            picture->setMovie(DigestGif1);
            picture->start();
            if(qrand() % 2 == 0) //交换咀嚼声音
                digest1->play();
            else
                digest2->play();
        }))->start();
        digesting = true;
        //吃完
        (new Timer(picture,10000,[this]{
            picture->setMovie(this->plantProtoType->normalGif);
            picture->start();
            digesting=false;
        }))->start();
        initTrigger(); //继续
    }
    else
        (new Timer(picture,500,[this]{
        initTrigger();
    }))->start();
}


//LilyPad

LilyPad::LilyPad()
{
    eName = "oLilyPad";
    cName = tr("LilyPad");
    hp = 400;

    beAttackedPointR = 45;
    sunNum = 5;
    coolTime = 1;
    pKind = 0;

    cardGif = "Card/Plants/LilyPad.png";
    staticGif = "Plants/LilyPad/0.gif";
    normalGif = "Plants/LilyPad/LilyPad.gif";
    toolTip = tr("漂浮在水面上，种植其他植物");
}

bool LilyPad::canGrow(int x, int y) const
{
   if (x < 1 || 9 < x || y < 3 || 4 < y)
        return false;
    if (scene->isCrater(x, y) || scene->isTombstone(x, y))
        return false;
    int groundType = scene->getGameLevelData()->LF[y];
    QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
    if (groundType == 2)
        return !plants.contains(1) || plants[1]->plantProtoType->eName == "oLilyPad";
    return plants.contains(0) && (!plants.contains(1) || plants[1]->plantProtoType->eName == "oLilyPad");

}


LilyPadInstance::LilyPadInstance(const Plant *plant)
    : PlantInstance(plant)
{}


//TangleKlep

TangleKlep::TangleKlep()
{
    eName = "oTangleKlep";
    cName = tr("TangleKlep");

    beAttackedPointR = 53;
    sunNum = 0;

    cardGif = "Card/Plants/TangleKlep.png";
    staticGif = "Plants/TangleKlep/0.gif";
    normalGif = "Plants/TangleKlep/Float.gif";
    toolTip = tr("TangleKlep");
}

bool TangleKlep::canGrow(int x, int y) const
{
   if (x < 1 ||  9 < x || y < 1 || this->scene->getCoordinate().colCount() < y)
        return false;
    if (scene->isCrater(x, y) || scene->isTombstone(x, y))
        return false;
    int groundType = scene->getGameLevelData()->LF[y];
    QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
    if (groundType == 2)
        return !plants.contains(1);
    return plants.contains(0);
}

TangleKlepInstance::TangleKlepInstance(const Plant *plant)
    : PlantInstance(plant),
      Grab(new MoviePixmapItem)
{}

void TangleKlepInstance::initTrigger()
{
    QList<ZombieInstance *> zombies = plantProtoType->scene->getZombieOnRow(row);
    for (auto zombie: zombies){
        int z_col = zombie->zombieProtoType->scene->getCoordinate().getCol(zombie->ZX);
        if(z_col >= col-1 && z_col <= col+1 ){
            Coordinate &coordinate = plantProtoType->scene->getCoordinate();
            double x = coordinate.getX(col) + plantProtoType->getDX();
            Attack(zombie); //抓住
            break;
        }
    }
    (new Timer(picture, 900, [this]{
        initTrigger();
    }))->start();
}

void TangleKlepInstance::Attack(ZombieInstance *zombie)
{
    Coordinate &coordinate = plantProtoType->scene->getCoordinate();
    double y = coordinate.getY(row) + plantProtoType->getDY(col, row) - plantProtoType->height;
    (new Timer(picture, 500, [this , zombie, y]{
        Grab->setPos(zombie->ZX,y);
        Grab->setMovie("Plants/TangleKlep/Grab.gif");
        Grab->start();
        (new Timer(picture, 200, [this , zombie]{
            if(zombie->hp>500)
                zombie->getHit(500);
            else
                this->plantProtoType->scene->zombieDie(zombie);
            picture->stop();
            (new Timer(picture, 500, [this]{
                this->plantProtoType->scene->plantDie(this);
            }))->start();
        }))->start();
    }))->start();
}


//Spikeweed

Spikeweed::Spikeweed()
{
    eName = "oSpikeweed";
    cName = tr("Spikeweed");

    beAttackedPointR = 0;
    sunNum = 0;

    cardGif = "Card/Plants/Spikeweed.png";
    staticGif = "Plants/Spikeweed/0.gif";
    normalGif = "Plants/Spikeweed/Spikeweed.gif";
    toolTip = tr("扎刺走在上面的僵尸");
}

SpikeweedInstance::SpikeweedInstance(const Plant *plant)
    : PlantInstance(plant)
{}

void SpikeweedInstance::initTrigger()
{
    QList<ZombieInstance *> zombies = plantProtoType->scene->getZombieOnRow(row);
    for (auto zombie: zombies){
        int z_col = zombie->zombieProtoType->scene->getCoordinate().getCol(zombie->ZX);
        if(z_col == col){
            zombie->getHit(20);
        }
    }
    (new Timer(picture, 1000, [this]{
        initTrigger();
    }))->start();
}


//Spikerock

Spikerock::Spikerock()
{
    eName = "oSpikerock";
    cName = tr("Spikerock");

    beAttackedPointR = 0;
    sunNum = 0;

    cardGif = "Card/Plants/Spikerock.png";
    staticGif = "Plants/Spikerock/0.gif";
    normalGif = "Plants/Spikerock/Spikerock.gif";
    toolTip = tr("扎刺走在上面的僵尸");
}

//种植在普通地刺上
bool Spikerock::canGrow(int x, int y) const
{
    if (x < 1 ||  9 < x || y < 1 || this->scene->getCoordinate().colCount() < y)
         return false;
     if (scene->isCrater(x, y) || scene->isTombstone(x, y))
         return false;
     int groundType = scene->getGameLevelData()->LF[y];
     QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
     if(groundType == 1 &&plants.contains(1) == 0)
         return  false;
     if (groundType == 1)
         return !plants.contains(1) || plants[1]->plantProtoType->eName == "oSpikeweed";
     return plants.contains(0) && (!plants.contains(1) || plants[1]->plantProtoType->eName == "oSpikeweed");
}

SpikerockInstance::SpikerockInstance(const Plant *plant)
    : PlantInstance(plant)
{}

void SpikerockInstance::initTrigger()
{
    QList<ZombieInstance *> zombies = plantProtoType->scene->getZombieOnRow(row);
    for (auto zombie: zombies){
        int z_col = zombie->zombieProtoType->scene->getCoordinate().getCol(zombie->ZX);
        if(z_col == col){
            zombie->getHit(40);
        }
    }
    (new Timer(picture, 1000, [this]{
        initTrigger();
    }))->start();
}


//FlowerPot

FlowerPot::FlowerPot()
{
    eName = "oFlowerPot";
    cName = tr("FlowerPot");
    hp = 400;

    beAttackedPointR = 45;
    sunNum = 0;
    coolTime = 1;
    pKind = 0;

    cardGif = "Card/Plants/FlowerPot.png";
    staticGif = "Plants/FlowerPot/0.gif";
    normalGif = "Plants/FlowerPot/FlowerPot.gif";
    toolTip = tr("放在屋顶，种植其他植物");
}

FlowerPotInstance::FlowerPotInstance(const Plant *plant)
    : PlantInstance(plant)
{}


//HypnoShroom

HypnoShroom::HypnoShroom()
{
    eName = "oHypnoShroom";
    cName = tr("HypnoShroom");

    cardGif = "Card/Plants/HypnoShroom.png";
    staticGif = "Plants/HypnoShroom/0.gif";
    normalGif = "Plants/HypnoShroom/HypnoShroom.gif";
    toolTip = tr("随机产生一种植物");
}

HypnoShroomInstance::HypnoShroomInstance(const Plant *plant)
    : PlantInstance(plant)
{}

//1
HypnoShroom1::HypnoShroom1()
{
    eName = "oHypnoShroom1";
    cName = tr("HypnoShroom1");

    cardGif = "Card/Plants/HypnoShroom.png";
    staticGif = "Plants/HypnoShroom/0.gif";
    normalGif = "Plants/HypnoShroom/HypnoShroom.gif";
    toolTip = tr("随机产生一种植物");
}

HypnoShroomInstance1::HypnoShroomInstance1(const Plant *plant)
    : PlantInstance(plant)
{}
//2
HypnoShroom2::HypnoShroom2()
{
    eName = "oHypnoShroom2";
    cName = tr("HypnoShroom2");

    cardGif = "Card/Plants/HypnoShroom.png";
    staticGif = "Plants/HypnoShroom/0.gif";
    normalGif = "Plants/HypnoShroom/HypnoShroom.gif";
    toolTip = tr("随机产生一种植物");
}

HypnoShroomInstance2::HypnoShroomInstance2(const Plant *plant)
    : PlantInstance(plant)
{}
//3
HypnoShroom3::HypnoShroom3()
{
    eName = "oHypnoShroom3";
    cName = tr("HypnoShroom3");

    cardGif = "Card/Plants/HypnoShroom.png";
    staticGif = "Plants/HypnoShroom/0.gif";
    normalGif = "Plants/HypnoShroom/HypnoShroom.gif";
    toolTip = tr("随机产生一种植物");
}

HypnoShroomInstance3::HypnoShroomInstance3(const Plant *plant)
    : PlantInstance(plant)
{}
//4
HypnoShroom4::HypnoShroom4()
{
    eName = "oHypnoShroom4";
    cName = tr("HypnoShroom4");

    cardGif = "Card/Plants/HypnoShroom.png";
    staticGif = "Plants/HypnoShroom/0.gif";
    normalGif = "Plants/HypnoShroom/HypnoShroom.gif";
    toolTip = tr("随机产生一种植物");
}

HypnoShroomInstance4::HypnoShroomInstance4(const Plant *plant)
    : PlantInstance(plant)
{}
//5
HypnoShroom5::HypnoShroom5()
{
    eName = "oHypnoShroom5";
    cName = tr("HypnoShroom5");

    cardGif = "Card/Plants/HypnoShroom.png";
    staticGif = "Plants/HypnoShroom/0.gif";
    normalGif = "Plants/HypnoShroom/HypnoShroom.gif";
    toolTip = tr("随机产生一种植物");
}

HypnoShroomInstance5::HypnoShroomInstance5(const Plant *plant)
    : PlantInstance(plant)
{}
//6
HypnoShroom6::HypnoShroom6()
{
    eName = "oHypnoShroom6";
    cName = tr("HypnoShroom6");

    cardGif = "Card/Plants/HypnoShroom.png";
    staticGif = "Plants/HypnoShroom/0.gif";
    normalGif = "Plants/HypnoShroom/HypnoShroom.gif";
    toolTip = tr("随机产生一种植物");
}

HypnoShroomInstance6::HypnoShroomInstance6(const Plant *plant)
    : PlantInstance(plant)
{}
//7
HypnoShroom7::HypnoShroom7()
{
    eName = "oHypnoShroom7";
    cName = tr("HypnoShroom7");

    cardGif = "Card/Plants/HypnoShroom.png";
    staticGif = "Plants/HypnoShroom/0.gif";
    normalGif = "Plants/HypnoShroom/HypnoShroom.gif";
    toolTip = tr("随机产生一种植物");
}

HypnoShroomInstance7::HypnoShroomInstance7(const Plant *plant)
    : PlantInstance(plant)
{}


//baolingqiu
baolingqiu::baolingqiu()
{
    eName = "obaolingqiu";
    cName = tr("baolingqiu");

    sunNum = 0;

    cardGif = "Card/Plants/WallNut.png";
    staticGif = "Plants/WallNut/0.gif";
    normalGif = "Plants/WallNut/WallNut.gif";
    toolTip = tr("《保 龄 攻 击》");
}

baolingqiuInstance::baolingqiuInstance(const Plant *plant)
    : PlantInstance(plant), baoling(new QMediaPlayer(picture))
{
    baoling->setMedia(QUrl("qrc:/audio/bowling.mp3"));
}

void baolingqiuInstance::normalAttack(ZombieInstance *zombieInstance)
{
    /*
    baoling->play();
    (new Bullet(plantProtoType->scene, -2, row, attackedLX, attackedLX - 40, picture->y() + 3, picture->zValue() + 2, 1))->start();
    this->plantProtoType->scene->plantDie(this);
    */
    QMediaPlayer *player = new QMediaPlayer(plantProtoType->scene);
    player->setMedia(QUrl("qrc:/audio/bowling.mp3"));
    player->play();
    QSharedPointer<std::function<void(void)> > crush(new std::function<void(void)>);
    *crush = [this, crush] {
        for (auto zombie: plantProtoType->scene->getZombieOnRowRange(row, attackedLX, attackedRX)) {
            zombie->crushDie();
        }
        if (attackedLX > 900)
            plantProtoType->scene->plantDie(this);
        else {
            attackedLX += 10;
            attackedRX += 10;
            picture->setPos(picture->pos() + QPointF(10, 0));
            (new Timer(picture, 10, *crush))->start();
        }
    };
    (*crush)();
}

/*
bool FlowerPot::canGrow(int x, int y) const
{
    if (x < 1 || 9 < x|| y < 1 || this->scene->getCoordinate().colCount() < y)
        return false;
    if (scene->isCrater(x, y) || scene->isTombstone(x, y))
        return false;
    int groundType = scene->getGameLevelData()->LF[y];
    QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
    if (groundType == 1)
        return !plants.contains(1);
    return plants.contains(0) && !plants.contains(1);
}
*/


/*
//Garlic

Garlic::Garlic()
{
    eName = "oGarlic";
    cName = tr("Garlic");

    beAttackedPointR = 35;
    sunNum = 0;

    cardGif = "Card/Plants/Garlic.png";
    staticGif = "Plants/Garlic/0.gif";
    normalGif = "Plants/Garlic.gif";
    toolTip = tr("使僵尸去另一行");
}

GarlicInstance::GarlicInstance(const Plant *plant)
    :PlantInstance(plant)
{}

void GarlicInstance::initTrigger()
{}
*/


/*******************************僵尸植物****************************************/

//。。。。。

//PlantFactory

int randnum;

Plant *PlantFactory(GameScene *scene, const QString &eName)
{
    Plant *plant = nullptr;
    if (eName == "oSunflower")
        plant = new SunFlower;
    else if (eName == "oSunShroom")
        plant = new SunShroom;
    else if (eName == "oTwinSunflower")
        plant = new TwinSunflower;
    else if (eName == "oPeashooter")
        plant = new Peashooter;
    else if(eName == "oRepeater")
        plant = new Repeater;
    else if(eName == "oGatlingPea")
        plant = new GatlingPea;
    else if(eName == "oSplitPea")
        plant = new SplitPea;
    else if(eName == "oThreepeater")
        plant = new Threepeater;
    else if (eName == "oSnowPea")
        plant = new SnowPea;
    else if(eName == "oTorchwood")
        plant = new Torchwood;
    else if (eName == "oWallNut")
        plant = new WallNut;
    else if(eName == "oTallNut")
        plant = new TallNut;
    else if(eName == "oPumpkinHead")
        plant = new PumpkinHead;
    else if(eName == "oPotatoMine")
        plant = new PotatoMine;
    else if(eName == "oCherryBomb")
        plant = new CherryBomb;
    else if(eName == "oJalapeno")
        plant = new Jalapeno;
    else if(eName == "oSquash")
        plant = new Squash;
    else if(eName == "oChomper")
        plant = new Chomper;
    else if(eName == "oLilyPad")
        plant = new LilyPad;
    else if(eName == "oFlowerPot")
        plant = new FlowerPot;
    else if(eName == "oTangleKlep")
        plant = new TangleKlep;
    else if(eName == "oSpikeweed")
        plant = new Spikeweed;
    else if(eName == "oSpikerock")
        plant = new Spikerock;
    else if (eName == "oLawnCleaner")
        plant = new LawnCleaner;
    else if (eName == "oPoolCleaner")
        plant = new PoolCleaner;

    else if(eName == "obaolingqiu")
        plant = new baolingqiu;

    else if (eName == "oHypnoShroom"){
        randnum = (qrand() % 15);
        if (randnum == 1)
            plant = new SunFlower;
        else if (randnum == 2)
            plant = new SunShroom;
        else if (randnum == 3)
            plant = new TwinSunflower;
        else if (randnum == 4)
            plant = new Peashooter;
        else if(randnum == 4)
            plant = new Repeater;
        else if(randnum == 5)
            plant = new GatlingPea;
        else if(randnum == 6)
            plant = new SplitPea;
        else if(randnum == 7)
            plant = new Threepeater;
        else if(randnum == 8)
            plant = new SnowPea;
        else if(randnum == 9)
            plant = new Torchwood;
        else if (randnum == 10)
            plant = new WallNut;
        else if(randnum == 11)
            plant = new TallNut;
        else if(randnum == 12)
            plant = new PumpkinHead;
        else if(randnum == 13)
            plant = new PotatoMine;
        else if(randnum == 14)
            plant = new CherryBomb;
        else if(randnum == 15)
            plant = new Jalapeno;
        else if(randnum == 16)
            plant = new Squash;
        else if(randnum == 17)
            plant = new Chomper;
        else if(randnum == 18)
            plant = new LilyPad;
        else if(randnum == 19)
            plant = new FlowerPot;
        else if(randnum == 20)
            plant = new TangleKlep;
        else if(randnum == 21)
            plant = new Spikeweed;
        else if(randnum == 22)
            plant = new Spikerock;
        else if (randnum == 23)
            plant = new LawnCleaner;
    }
    else if (eName == "oHypnoShroom1"){
        randnum = (qrand() % 15);
        if (randnum == 1)
            plant = new SunFlower;
        else if (randnum == 2)
            plant = new SunShroom;
        else if (randnum == 3)
            plant = new TwinSunflower;
        else if (randnum == 4)
            plant = new Peashooter;
        else if(randnum == 4)
            plant = new Repeater;
        else if(randnum == 5)
            plant = new GatlingPea;
        else if(randnum == 6)
            plant = new SplitPea;
        else if(randnum == 7)
            plant = new Threepeater;
        else if(randnum == 8)
            plant = new SnowPea;
        else if(randnum == 9)
            plant = new Torchwood;
        else if (randnum == 10)
            plant = new WallNut;
        else if(randnum == 11)
            plant = new TallNut;
        else if(randnum == 12)
            plant = new PumpkinHead;
        else if(randnum == 13)
            plant = new PotatoMine;
        else if(randnum == 14)
            plant = new CherryBomb;
        else if(randnum == 15)
            plant = new Jalapeno;
        else if(randnum == 16)
            plant = new Squash;
        else if(randnum == 17)
            plant = new Chomper;
        else if(randnum == 18)
            plant = new LilyPad;
        else if(randnum == 19)
            plant = new FlowerPot;
        else if(randnum == 20)
            plant = new TangleKlep;
        else if(randnum == 21)
            plant = new Spikeweed;
        else if(randnum == 22)
            plant = new Spikerock;
        else if (randnum == 23)
            plant = new LawnCleaner;
    }
    else if (eName == "oHypnoShroom2"){
        randnum = (qrand() % 15);
        if (randnum == 1)
            plant = new SunFlower;
        else if (randnum == 2)
            plant = new SunShroom;
        else if (randnum == 3)
            plant = new TwinSunflower;
        else if (randnum == 4)
            plant = new Peashooter;
        else if(randnum == 4)
            plant = new Repeater;
        else if(randnum == 5)
            plant = new GatlingPea;
        else if(randnum == 6)
            plant = new SplitPea;
        else if(randnum == 7)
            plant = new Threepeater;
        else if(randnum == 8)
            plant = new SnowPea;
        else if(randnum == 9)
            plant = new Torchwood;
        else if (randnum == 10)
            plant = new WallNut;
        else if(randnum == 11)
            plant = new TallNut;
        else if(randnum == 12)
            plant = new PumpkinHead;
        else if(randnum == 13)
            plant = new PotatoMine;
        else if(randnum == 14)
            plant = new CherryBomb;
        else if(randnum == 15)
            plant = new Jalapeno;
        else if(randnum == 16)
            plant = new Squash;
        else if(randnum == 17)
            plant = new Chomper;
        else if(randnum == 18)
            plant = new LilyPad;
        else if(randnum == 19)
            plant = new FlowerPot;
        else if(randnum == 20)
            plant = new TangleKlep;
        else if(randnum == 21)
            plant = new Spikeweed;
        else if(randnum == 22)
            plant = new Spikerock;
        else if (randnum == 23)
            plant = new LawnCleaner;
    }
    else if (eName == "oHypnoShroom3"){
        randnum = (qrand() % 15);
        if (randnum == 1)
            plant = new SunFlower;
        else if (randnum == 2)
            plant = new SunShroom;
        else if (randnum == 3)
            plant = new TwinSunflower;
        else if (randnum == 4)
            plant = new Peashooter;
        else if(randnum == 4)
            plant = new Repeater;
        else if(randnum == 5)
            plant = new GatlingPea;
        else if(randnum == 6)
            plant = new SplitPea;
        else if(randnum == 7)
            plant = new Threepeater;
        else if(randnum == 8)
            plant = new SnowPea;
        else if(randnum == 9)
            plant = new Torchwood;
        else if (randnum == 10)
            plant = new WallNut;
        else if(randnum == 11)
            plant = new TallNut;
        else if(randnum == 12)
            plant = new PumpkinHead;
        else if(randnum == 13)
            plant = new PotatoMine;
        else if(randnum == 14)
            plant = new CherryBomb;
        else if(randnum == 15)
            plant = new Jalapeno;
        else if(randnum == 16)
            plant = new Squash;
        else if(randnum == 17)
            plant = new Chomper;
        else if(randnum == 18)
            plant = new LilyPad;
        else if(randnum == 19)
            plant = new FlowerPot;
        else if(randnum == 20)
            plant = new TangleKlep;
        else if(randnum == 21)
            plant = new Spikeweed;
        else if(randnum == 22)
            plant = new Spikerock;
        else if (randnum == 23)
            plant = new LawnCleaner;
    }
    else if (eName == "oHypnoShroom4"){
        randnum = (qrand() % 15);
        if (randnum == 1)
            plant = new SunFlower;
        else if (randnum == 2)
            plant = new SunShroom;
        else if (randnum == 3)
            plant = new TwinSunflower;
        else if (randnum == 4)
            plant = new Peashooter;
        else if(randnum == 4)
            plant = new Repeater;
        else if(randnum == 5)
            plant = new GatlingPea;
        else if(randnum == 6)
            plant = new SplitPea;
        else if(randnum == 7)
            plant = new Threepeater;
        else if(randnum == 8)
            plant = new SnowPea;
        else if(randnum == 9)
            plant = new Torchwood;
        else if (randnum == 10)
            plant = new WallNut;
        else if(randnum == 11)
            plant = new TallNut;
        else if(randnum == 12)
            plant = new PumpkinHead;
        else if(randnum == 13)
            plant = new PotatoMine;
        else if(randnum == 14)
            plant = new CherryBomb;
        else if(randnum == 15)
            plant = new Jalapeno;
        else if(randnum == 16)
            plant = new Squash;
        else if(randnum == 17)
            plant = new Chomper;
        else if(randnum == 18)
            plant = new LilyPad;
        else if(randnum == 19)
            plant = new FlowerPot;
        else if(randnum == 20)
            plant = new TangleKlep;
        else if(randnum == 21)
            plant = new Spikeweed;
        else if(randnum == 22)
            plant = new Spikerock;
        else if (randnum == 23)
            plant = new LawnCleaner;
    }
    else if (eName == "oHypnoShroom5"){
        randnum = (qrand() % 15);
        if (randnum == 1)
            plant = new SunFlower;
        else if (randnum == 2)
            plant = new SunShroom;
        else if (randnum == 3)
            plant = new TwinSunflower;
        else if (randnum == 4)
            plant = new Peashooter;
        else if(randnum == 4)
            plant = new Repeater;
        else if(randnum == 5)
            plant = new GatlingPea;
        else if(randnum == 6)
            plant = new SplitPea;
        else if(randnum == 7)
            plant = new Threepeater;
        else if(randnum == 8)
            plant = new SnowPea;
        else if(randnum == 9)
            plant = new Torchwood;
        else if (randnum == 10)
            plant = new WallNut;
        else if(randnum == 11)
            plant = new TallNut;
        else if(randnum == 12)
            plant = new PumpkinHead;
        else if(randnum == 13)
            plant = new PotatoMine;
        else if(randnum == 14)
            plant = new CherryBomb;
        else if(randnum == 15)
            plant = new Jalapeno;
        else if(randnum == 16)
            plant = new Squash;
        else if(randnum == 17)
            plant = new Chomper;
        else if(randnum == 18)
            plant = new LilyPad;
        else if(randnum == 19)
            plant = new FlowerPot;
        else if(randnum == 20)
            plant = new TangleKlep;
        else if(randnum == 21)
            plant = new Spikeweed;
        else if(randnum == 22)
            plant = new Spikerock;
        else if (randnum == 23)
            plant = new LawnCleaner;
    }
    else if (eName == "oHypnoShroom6"){
        randnum = (qrand() % 15);
        if (randnum == 1)
            plant = new SunFlower;
        else if (randnum == 2)
            plant = new SunShroom;
        else if (randnum == 3)
            plant = new TwinSunflower;
        else if (randnum == 4)
            plant = new Peashooter;
        else if(randnum == 4)
            plant = new Repeater;
        else if(randnum == 5)
            plant = new GatlingPea;
        else if(randnum == 6)
            plant = new SplitPea;
        else if(randnum == 7)
            plant = new Threepeater;
        else if(randnum == 8)
            plant = new SnowPea;
        else if(randnum == 9)
            plant = new Torchwood;
        else if (randnum == 10)
            plant = new WallNut;
        else if(randnum == 11)
            plant = new TallNut;
        else if(randnum == 12)
            plant = new PumpkinHead;
        else if(randnum == 13)
            plant = new PotatoMine;
        else if(randnum == 14)
            plant = new CherryBomb;
        else if(randnum == 15)
            plant = new Jalapeno;
        else if(randnum == 16)
            plant = new Squash;
        else if(randnum == 17)
            plant = new Chomper;
        else if(randnum == 18)
            plant = new LilyPad;
        else if(randnum == 19)
            plant = new FlowerPot;
        else if(randnum == 20)
            plant = new TangleKlep;
        else if(randnum == 21)
            plant = new Spikeweed;
        else if(randnum == 22)
            plant = new Spikerock;
        else if (randnum == 23)
            plant = new LawnCleaner;
    }
    else if (eName == "oHypnoShroom7"){
        randnum = (qrand() % 15);
        if (randnum == 1)
            plant = new SunFlower;
        else if (randnum == 2)
            plant = new SunShroom;
        else if (randnum == 3)
            plant = new TwinSunflower;
        else if (randnum == 4)
            plant = new Peashooter;
        else if(randnum == 4)
            plant = new Repeater;
        else if(randnum == 5)
            plant = new GatlingPea;
        else if(randnum == 6)
            plant = new SplitPea;
        else if(randnum == 7)
            plant = new Threepeater;
        else if(randnum == 8)
            plant = new SnowPea;
        else if(randnum == 9)
            plant = new Torchwood;
        else if (randnum == 10)
            plant = new WallNut;
        else if(randnum == 11)
            plant = new TallNut;
        else if(randnum == 12)
            plant = new PumpkinHead;
        else if(randnum == 13)
            plant = new PotatoMine;
        else if(randnum == 14)
            plant = new CherryBomb;
        else if(randnum == 15)
            plant = new Jalapeno;
        else if(randnum == 16)
            plant = new Squash;
        else if(randnum == 17)
            plant = new Chomper;
        else if(randnum == 18)
            plant = new LilyPad;
        else if(randnum == 19)
            plant = new FlowerPot;
        else if(randnum == 20)
            plant = new TangleKlep;
        else if(randnum == 21)
            plant = new Spikeweed;
        else if(randnum == 22)
            plant = new Spikerock;
        else if (randnum == 23)
            plant = new LawnCleaner;
    }

    if (plant) {
        plant->scene = scene;
        plant->update();
    }
    return plant;
}

//PlantInstanceFactory

PlantInstance *PlantInstanceFactory(const Plant *plant)
{
    if (plant->eName == "oSunflower")
        return new SunFlowerInstance(plant);
    if (plant->eName == "oSunShroom")
        return new SunShroomInstance(plant);
    if (plant->eName == "oTwinSunflower")
        return new TwinSunflowerInstance(plant);
    if (plant->eName == "oPeashooter")
        return new PeashooterInstance(plant);
    if (plant->eName == "oRepeater")
        return new RepeaterInstance(plant);
    if(plant->eName == "oGatlingPea")
        return new GatlingPeaInstance(plant);
    if(plant->eName == "oSplitPea")
        return new SplitPeaInstance(plant);
    if(plant->eName == "oThreepeater")
        return new ThreepeaterInstance(plant);
    if(plant->eName == "oSnowPea")
        return new SnowPeaInstance(plant);
    if(plant->eName == "oTorchwood")
        return new TorchwoodInstance(plant);
    if (plant->eName == "oWallNut")
        return new WallNutInstance(plant);
    if(plant->eName == "oTallNut")
        return new TallNutInstance(plant);
    if(plant->eName == "oPumpkinHead")
        return new PumpkinHeadInstance(plant);
    if(plant->eName == "oPotatoMine")
        return new PotatoMineInstance(plant);
    if(plant->eName == "oCherryBomb")
        return new CherryBombInstance(plant);
    if(plant->eName == "oJalapeno")
        return new JalapenoInstance(plant);
    if(plant->eName == "oSquash")
        return new SquashInstance(plant);
    if(plant->eName == "oChomper")
        return new ChomperInstance(plant);
    if(plant->eName == "oLilyPad")
        return new LilyPadInstance(plant);
    if(plant->eName == "oFlowerPot")
        return new FlowerPotInstance(plant);
    if(plant->eName == "oTangleKlep")
        return new TangleKlepInstance(plant);
    if(plant->eName == "oSpikeweed")
        return new SpikeweedInstance(plant);
    if(plant->eName == "oSpikerock")
        return new SpikerockInstance(plant);
    if (plant->eName == "oLawnCleaner")
        return new LawnCleanerInstance(plant);

    if(plant->eName == "obaolingqiu")
        return new baolingqiuInstance(plant);

    if (plant->eName == "oHypnoShroom"){
        if (randnum == 1)
            return new SunFlowerInstance(plant);
        else if (randnum == 2)
            return new SunShroomInstance(plant);
        else if (randnum == 3)
            return new TwinSunflowerInstance(plant);
        else if (randnum == 4)
            return new PeashooterInstance(plant);
        else if(randnum == 4)
            return new RepeaterInstance(plant);
        else if(randnum == 5)
            return new GatlingPeaInstance(plant);
        else if(randnum == 6)
            return new SplitPeaInstance(plant);
        else if(randnum == 7)
            return new ThreepeaterInstance(plant);
        else if(randnum == 8)
            return new SnowPeaInstance(plant);
        else if(randnum == 9)
            return new TorchwoodInstance(plant);
        else if (randnum == 10)
            return new WallNutInstance(plant);
        else if(randnum == 11)
            return new TallNutInstance(plant);
        else if(randnum == 12)
            return new PumpkinHeadInstance(plant);
        else if(randnum == 13)
            return new PotatoMineInstance(plant);
        else if(randnum == 14)
            return new CherryBombInstance(plant);
        else if(randnum == 15)
            return new JalapenoInstance(plant);
        else if(randnum == 16)
            return new SquashInstance(plant);
        else if(randnum == 17)
            return new ChomperInstance(plant);
        else if(randnum == 18)
            return new LilyPadInstance(plant);
        else if(randnum == 19)
            return new FlowerPotInstance(plant);
        else if(randnum == 20)
            return new TangleKlepInstance(plant);
        else if(randnum == 21)
            return new SpikeweedInstance(plant);
        else if(randnum == 22)
            return new SpikerockInstance(plant);
        else if (randnum == 23)
            return new LawnCleanerInstance(plant);
    }

    if (plant->eName == "oHypnoShroom1"){
        if (randnum == 1)
            return new SunFlowerInstance(plant);
        else if (randnum == 2)
            return new SunShroomInstance(plant);
        else if (randnum == 3)
            return new TwinSunflowerInstance(plant);
        else if (randnum == 4)
            return new PeashooterInstance(plant);
        else if(randnum == 4)
            return new RepeaterInstance(plant);
        else if(randnum == 5)
            return new GatlingPeaInstance(plant);
        else if(randnum == 6)
            return new SplitPeaInstance(plant);
        else if(randnum == 7)
            return new ThreepeaterInstance(plant);
        else if(randnum == 8)
            return new SnowPeaInstance(plant);
        else if(randnum == 9)
            return new TorchwoodInstance(plant);
        else if (randnum == 10)
            return new WallNutInstance(plant);
        else if(randnum == 11)
            return new TallNutInstance(plant);
        else if(randnum == 12)
            return new PumpkinHeadInstance(plant);
        else if(randnum == 13)
            return new PotatoMineInstance(plant);
        else if(randnum == 14)
            return new CherryBombInstance(plant);
        else if(randnum == 15)
            return new JalapenoInstance(plant);
        else if(randnum == 16)
            return new SquashInstance(plant);
        else if(randnum == 17)
            return new ChomperInstance(plant);
        else if(randnum == 18)
            return new LilyPadInstance(plant);
        else if(randnum == 19)
            return new FlowerPotInstance(plant);
        else if(randnum == 20)
            return new TangleKlepInstance(plant);
        else if(randnum == 21)
            return new SpikeweedInstance(plant);
        else if(randnum == 22)
            return new SpikerockInstance(plant);
        else if (randnum == 23)
            return new LawnCleanerInstance(plant);
    }
    if (plant->eName == "oHypnoShroom2"){
        if (randnum == 1)
            return new SunFlowerInstance(plant);
        else if (randnum == 2)
            return new SunShroomInstance(plant);
        else if (randnum == 3)
            return new TwinSunflowerInstance(plant);
        else if (randnum == 4)
            return new PeashooterInstance(plant);
        else if(randnum == 4)
            return new RepeaterInstance(plant);
        else if(randnum == 5)
            return new GatlingPeaInstance(plant);
        else if(randnum == 6)
            return new SplitPeaInstance(plant);
        else if(randnum == 7)
            return new ThreepeaterInstance(plant);
        else if(randnum == 8)
            return new SnowPeaInstance(plant);
        else if(randnum == 9)
            return new TorchwoodInstance(plant);
        else if (randnum == 10)
            return new WallNutInstance(plant);
        else if(randnum == 11)
            return new TallNutInstance(plant);
        else if(randnum == 12)
            return new PumpkinHeadInstance(plant);
        else if(randnum == 13)
            return new PotatoMineInstance(plant);
        else if(randnum == 14)
            return new CherryBombInstance(plant);
        else if(randnum == 15)
            return new JalapenoInstance(plant);
        else if(randnum == 16)
            return new SquashInstance(plant);
        else if(randnum == 17)
            return new ChomperInstance(plant);
        else if(randnum == 18)
            return new LilyPadInstance(plant);
        else if(randnum == 19)
            return new FlowerPotInstance(plant);
        else if(randnum == 20)
            return new TangleKlepInstance(plant);
        else if(randnum == 21)
            return new SpikeweedInstance(plant);
        else if(randnum == 22)
            return new SpikerockInstance(plant);
        else if (randnum == 23)
            return new LawnCleanerInstance(plant);
    }
    if (plant->eName == "oHypnoShroom3"){
        if (randnum == 1)
            return new SunFlowerInstance(plant);
        else if (randnum == 2)
            return new SunShroomInstance(plant);
        else if (randnum == 3)
            return new TwinSunflowerInstance(plant);
        else if (randnum == 4)
            return new PeashooterInstance(plant);
        else if(randnum == 4)
            return new RepeaterInstance(plant);
        else if(randnum == 5)
            return new GatlingPeaInstance(plant);
        else if(randnum == 6)
            return new SplitPeaInstance(plant);
        else if(randnum == 7)
            return new ThreepeaterInstance(plant);
        else if(randnum == 8)
            return new SnowPeaInstance(plant);
        else if(randnum == 9)
            return new TorchwoodInstance(plant);
        else if (randnum == 10)
            return new WallNutInstance(plant);
        else if(randnum == 11)
            return new TallNutInstance(plant);
        else if(randnum == 12)
            return new PumpkinHeadInstance(plant);
        else if(randnum == 13)
            return new PotatoMineInstance(plant);
        else if(randnum == 14)
            return new CherryBombInstance(plant);
        else if(randnum == 15)
            return new JalapenoInstance(plant);
        else if(randnum == 16)
            return new SquashInstance(plant);
        else if(randnum == 17)
            return new ChomperInstance(plant);
        else if(randnum == 18)
            return new LilyPadInstance(plant);
        else if(randnum == 19)
            return new FlowerPotInstance(plant);
        else if(randnum == 20)
            return new TangleKlepInstance(plant);
        else if(randnum == 21)
            return new SpikeweedInstance(plant);
        else if(randnum == 22)
            return new SpikerockInstance(plant);
        else if (randnum == 23)
            return new LawnCleanerInstance(plant);
    }
    if (plant->eName == "oHypnoShroom4"){
        if (randnum == 1)
            return new SunFlowerInstance(plant);
        else if (randnum == 2)
            return new SunShroomInstance(plant);
        else if (randnum == 3)
            return new TwinSunflowerInstance(plant);
        else if (randnum == 4)
            return new PeashooterInstance(plant);
        else if(randnum == 4)
            return new RepeaterInstance(plant);
        else if(randnum == 5)
            return new GatlingPeaInstance(plant);
        else if(randnum == 6)
            return new SplitPeaInstance(plant);
        else if(randnum == 7)
            return new ThreepeaterInstance(plant);
        else if(randnum == 8)
            return new SnowPeaInstance(plant);
        else if(randnum == 9)
            return new TorchwoodInstance(plant);
        else if (randnum == 10)
            return new WallNutInstance(plant);
        else if(randnum == 11)
            return new TallNutInstance(plant);
        else if(randnum == 12)
            return new PumpkinHeadInstance(plant);
        else if(randnum == 13)
            return new PotatoMineInstance(plant);
        else if(randnum == 14)
            return new CherryBombInstance(plant);
        else if(randnum == 15)
            return new JalapenoInstance(plant);
        else if(randnum == 16)
            return new SquashInstance(plant);
        else if(randnum == 17)
            return new ChomperInstance(plant);
        else if(randnum == 18)
            return new LilyPadInstance(plant);
        else if(randnum == 19)
            return new FlowerPotInstance(plant);
        else if(randnum == 20)
            return new TangleKlepInstance(plant);
        else if(randnum == 21)
            return new SpikeweedInstance(plant);
        else if(randnum == 22)
            return new SpikerockInstance(plant);
        else if (randnum == 23)
            return new LawnCleanerInstance(plant);
    }
    if (plant->eName == "oHypnoShroom5"){
        if (randnum == 1)
            return new SunFlowerInstance(plant);
        else if (randnum == 2)
            return new SunShroomInstance(plant);
        else if (randnum == 3)
            return new TwinSunflowerInstance(plant);
        else if (randnum == 4)
            return new PeashooterInstance(plant);
        else if(randnum == 4)
            return new RepeaterInstance(plant);
        else if(randnum == 5)
            return new GatlingPeaInstance(plant);
        else if(randnum == 6)
            return new SplitPeaInstance(plant);
        else if(randnum == 7)
            return new ThreepeaterInstance(plant);
        else if(randnum == 8)
            return new SnowPeaInstance(plant);
        else if(randnum == 9)
            return new TorchwoodInstance(plant);
        else if (randnum == 10)
            return new WallNutInstance(plant);
        else if(randnum == 11)
            return new TallNutInstance(plant);
        else if(randnum == 12)
            return new PumpkinHeadInstance(plant);
        else if(randnum == 13)
            return new PotatoMineInstance(plant);
        else if(randnum == 14)
            return new CherryBombInstance(plant);
        else if(randnum == 15)
            return new JalapenoInstance(plant);
        else if(randnum == 16)
            return new SquashInstance(plant);
        else if(randnum == 17)
            return new ChomperInstance(plant);
        else if(randnum == 18)
            return new LilyPadInstance(plant);
        else if(randnum == 19)
            return new FlowerPotInstance(plant);
        else if(randnum == 20)
            return new TangleKlepInstance(plant);
        else if(randnum == 21)
            return new SpikeweedInstance(plant);
        else if(randnum == 22)
            return new SpikerockInstance(plant);
        else if (randnum == 23)
            return new LawnCleanerInstance(plant);
    }
    if (plant->eName == "oHypnoShroom6"){
        if (randnum == 1)
            return new SunFlowerInstance(plant);
        else if (randnum == 2)
            return new SunShroomInstance(plant);
        else if (randnum == 3)
            return new TwinSunflowerInstance(plant);
        else if (randnum == 4)
            return new PeashooterInstance(plant);
        else if(randnum == 4)
            return new RepeaterInstance(plant);
        else if(randnum == 5)
            return new GatlingPeaInstance(plant);
        else if(randnum == 6)
            return new SplitPeaInstance(plant);
        else if(randnum == 7)
            return new ThreepeaterInstance(plant);
        else if(randnum == 8)
            return new SnowPeaInstance(plant);
        else if(randnum == 9)
            return new TorchwoodInstance(plant);
        else if (randnum == 10)
            return new WallNutInstance(plant);
        else if(randnum == 11)
            return new TallNutInstance(plant);
        else if(randnum == 12)
            return new PumpkinHeadInstance(plant);
        else if(randnum == 13)
            return new PotatoMineInstance(plant);
        else if(randnum == 14)
            return new CherryBombInstance(plant);
        else if(randnum == 15)
            return new JalapenoInstance(plant);
        else if(randnum == 16)
            return new SquashInstance(plant);
        else if(randnum == 17)
            return new ChomperInstance(plant);
        else if(randnum == 18)
            return new LilyPadInstance(plant);
        else if(randnum == 19)
            return new FlowerPotInstance(plant);
        else if(randnum == 20)
            return new TangleKlepInstance(plant);
        else if(randnum == 21)
            return new SpikeweedInstance(plant);
        else if(randnum == 22)
            return new SpikerockInstance(plant);
        else if (randnum == 23)
            return new LawnCleanerInstance(plant);
    }
    if (plant->eName == "oHypnoShroom7"){
        if (randnum == 1)
            return new SunFlowerInstance(plant);
        else if (randnum == 2)
            return new SunShroomInstance(plant);
        else if (randnum == 3)
            return new TwinSunflowerInstance(plant);
        else if (randnum == 4)
            return new PeashooterInstance(plant);
        else if(randnum == 4)
            return new RepeaterInstance(plant);
        else if(randnum == 5)
            return new GatlingPeaInstance(plant);
        else if(randnum == 6)
            return new SplitPeaInstance(plant);
        else if(randnum == 7)
            return new ThreepeaterInstance(plant);
        else if(randnum == 8)
            return new SnowPeaInstance(plant);
        else if(randnum == 9)
            return new TorchwoodInstance(plant);
        else if (randnum == 10)
            return new WallNutInstance(plant);
        else if(randnum == 11)
            return new TallNutInstance(plant);
        else if(randnum == 12)
            return new PumpkinHeadInstance(plant);
        else if(randnum == 13)
            return new PotatoMineInstance(plant);
        else if(randnum == 14)
            return new CherryBombInstance(plant);
        else if(randnum == 15)
            return new JalapenoInstance(plant);
        else if(randnum == 16)
            return new SquashInstance(plant);
        else if(randnum == 17)
            return new ChomperInstance(plant);
        else if(randnum == 18)
            return new LilyPadInstance(plant);
        else if(randnum == 19)
            return new FlowerPotInstance(plant);
        else if(randnum == 20)
            return new TangleKlepInstance(plant);
        else if(randnum == 21)
            return new SpikeweedInstance(plant);
        else if(randnum == 22)
            return new SpikerockInstance(plant);
        else if (randnum == 23)
            return new LawnCleanerInstance(plant);
    }

    return new PlantInstance(plant);
}
