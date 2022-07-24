//
// Created by sun on 8/26/16.
//

#ifndef PLANTS_VS_ZOMBIES_PLANT_H
#define PLANTS_VS_ZOMBIES_PLANT_H

#include <QtCore>
#include <QtWidgets>
#include <QtMultimedia>

#include "Zombie.h"

class MoviePixmapItem;
class GameScene;
class ZombieInstance;
class Trigger;

class Plant
{
    Q_DECLARE_TR_FUNCTIONS(Plant)

public:
    Plant();
    virtual  ~Plant() {}

    QString eName, cName;
    int width, height;
    int hp, pKind, bKind;
    int beAttackedPointL, beAttackedPointR;
    int zIndex;
    QString cardGif, staticGif, normalGif;
    bool canEat, canSelect, night;
    double coolTime;
    int stature, sleep;
    int sunNum;
    QString toolTip;

    virtual double getDX() const;
    virtual double getDY(int x, int y) const;
    virtual bool canGrow(int x, int y) const;

    GameScene *scene;
    void update();
};

class PlantInstance
{
public:
    bool die; //难蚌！！！

    bool goingDie;
    PlantInstance(const Plant *plant);
    virtual ~PlantInstance();

    virtual void birth(int c, int r);
    virtual void initTrigger(); //初始化触发器
    virtual void triggerCheck(ZombieInstance *zombieInstance, Trigger *trigger);
    virtual void normalAttack(ZombieInstance *zombieInstance);
    virtual void getHurt(ZombieInstance *zombie, int aKind, int attack);

    bool contains(const QPointF &pos);

    const Plant *plantProtoType;

    QUuid uuid; //唯一码
    int row, col;
    int hp;
    bool canTrigger;
    qreal attackedLX, attackedRX;
    QMap<int, QList<Trigger *> > triggers;

    QGraphicsPixmapItem *shadowPNG;
    MoviePixmapItem *picture;
};


//SunFlower
class SunFlower: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(SunFlower)
public:
    SunFlower();
};

class SunFlowerInstance: public PlantInstance
{
public:
    SunFlowerInstance(const Plant *plant);
    virtual void initTrigger();
private:
    QString lightedGif;
};


//SunShroom
class SunShroom: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(SunShroom)
public:
    SunShroom();
};

class SunShroomInstance: public PlantInstance
{
public:
    SunShroomInstance(const Plant *plant);
    virtual void initTrigger();
private:
    QString lightedGif;
};


//TwinSunFlower
class TwinSunflower: public SunFlower
{
    Q_DECLARE_TR_FUNCTIONS(TwinSunFlower)
public:
    TwinSunflower();
    virtual bool canGrow(int x, int y) const;
};

class TwinSunflowerInstance: public SunFlowerInstance
{
public:
    TwinSunflowerInstance(const Plant *plant);
    virtual void initTrigger();
private:
    QString lightedGif;
};


//Peashooter
class Peashooter: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Peashooter)
public:
    Peashooter();
};

class PeashooterInstance: public PlantInstance
{
public:
    PeashooterInstance(const Plant *plant);
    virtual void normalAttack(ZombieInstance *zombieInstance);
private:
    QString bulletGif, bulletHitGif;
    QMediaPlayer *firePea;
};


//Repeater
class Repeater: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Repeater)
public:
    Repeater();
    virtual bool canGrow(int x, int y) const;
};

class RepeaterInstance: public PlantInstance
{
public:
    RepeaterInstance(const Plant *plant);
    virtual void normalAttack(ZombieInstance *zombieInstance);
private:
    QString bulletGif, bulletHitGif;
    QMediaPlayer *firePea;
};


//GatlingPea
class GatlingPea: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Repeater)
public:
    GatlingPea();
    virtual bool canGrow(int x, int y) const;
};

class GatlingPeaInstance: public PlantInstance
{
public:
    GatlingPeaInstance(const Plant *plant);
    virtual void normalAttack(ZombieInstance *zombieInstance);
private:
    QString bulletGif, bulletHitGif;
    QMediaPlayer *firePea;
};


//SplitPea
class SplitPea: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(SplitPea)
public:
    SplitPea();
};

class SplitPeaInstance: public PlantInstance
{
public:
    SplitPeaInstance(const Plant *plant);
    virtual void normalAttack(ZombieInstance *zombieInstance);
private:
    QString bulletGif, bulletHitGif;
    QMediaPlayer *firePea;
};


//Threepeater
class Threepeater: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Threepeater)
public:
    Threepeater();
};

class ThreepeaterInstance: public PlantInstance
{
public:
    ThreepeaterInstance(const Plant *plant);
    virtual void initTrigger();
};


//SnowPea
class SnowPea: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(SnowPea)
public:
    SnowPea();
};

class SnowPeaInstance: public PlantInstance
{
public:
    SnowPeaInstance(const Plant *plant);
    virtual void normalAttack(ZombieInstance *zombieInstance);
private:
    QString bulletGif, bulletHitGif;
    QMediaPlayer *firePea;
};


//Torchwood
class Torchwood: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Torchwood)
public:
    Torchwood();
};

class TorchwoodInstance: public PlantInstance
{
public:
    TorchwoodInstance(const Plant *plant);
    virtual void initTrigger();
};


//WallNut
class WallNut: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(WallNut)
public:
    WallNut();
    virtual bool canGrow(int x, int y) const;
};

class WallNutInstance: public PlantInstance
{
public:
    WallNutInstance(const Plant *plant);
    virtual void initTrigger();
    virtual void getHurt(ZombieInstance *zombie, int aKind, int attack);
private:
    int hurtStatus;
    QString crackedGif1, crackedGif2;
};


//TallNut
class TallNut: public WallNut
{
    Q_DECLARE_TR_FUNCTIONS(TallNut)
public:
    TallNut();
    virtual bool canGrow(int x, int y) const;
};

class TallNutInstance: public WallNutInstance
{
public:
    TallNutInstance(const Plant *plant);
    virtual void getHurt(ZombieInstance *zombie, int aKind, int attack);
private:
    int hurtStatus;
};


//PumpkinHead
class PumpkinHead: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(PumpkinHead)
public:
    PumpkinHead();
    virtual double getDY(int x, int y) const;
    virtual bool canGrow(int x, int y) const;
};

class PumpkinHeadInstance: public PlantInstance
{
public:
    PumpkinHeadInstance(const Plant *plant);
    virtual ~PumpkinHeadInstance();
    virtual void birth(int c, int r);
    virtual void getHurt(ZombieInstance *zombie, int aKind, int attack);

    int hurtStatus;
    MoviePixmapItem *picture2;
};


//Cleaner
class LawnCleaner: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(LawnCleaner)
public:
    LawnCleaner();
};

class LawnCleanerInstance: public PlantInstance
{
public:
    LawnCleanerInstance(const Plant *plant);
    virtual void initTrigger();
    virtual void triggerCheck(ZombieInstance *zombieInstance, Trigger *trigger);
    virtual void normalAttack(ZombieInstance *zombieInstance);
};

class PoolCleaner: public LawnCleaner
{
    Q_DECLARE_TR_FUNCTIONS(PoolCleaner)
public:
    PoolCleaner();
};


//PotatoneMine
class PotatoMine: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(PotatoMine)
public:
   PotatoMine();
};

class PotatoMineInstance: public PlantInstance
{
public:
    PotatoMineInstance(const Plant *plant);
    virtual void initTrigger();
    virtual void getHurt(ZombieInstance *zombie,int aKind, int attack);
private:
    QString red0Gif,red1Gif,red2Gif;
    QMediaPlayer *boomMusic;
    bool isBoom;
};


//CherryBomb
class CherryBomb: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(CherryBomb)
public:
   CherryBomb();
};

class CherryBombInstance: public PlantInstance
{
public:
    CherryBombInstance(const Plant *plant);
    virtual void initTrigger();
    virtual void normalAttack();
private:
    QString boomGif;
    QMediaPlayer *boom;
};


//Jalapeno
class Jalapeno: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Jalapeno)
public:
    Jalapeno();
};

class JalapenoInstance: public PlantInstance
{
public:
    JalapenoInstance(const Plant *plant);
    virtual void initTrigger();
    virtual void normalAttack();
private:
    QString burnGif;
    QMediaPlayer *burn;
};


//Squash
class Squash: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Squash)
public:
    Squash();
};

class SquashInstance: public PlantInstance
{
public:
    SquashInstance(const Plant *plant);
    virtual void initTrigger();
    virtual void bmmAttack(ZombieInstance *zombie);
private:
    QString bmml, bmmr, bmmattack;
    QMediaPlayer *bmmMusic;
};


//Chomper
class Chomper:public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Chomper)
public:
    Chomper();
};

class ChomperInstance: public PlantInstance
{
public:
    ChomperInstance(const Plant *plant);
    virtual void initTrigger();
    //virtual void trackTrigger()
private:
    QString AttackGif1,DigestGif1;
    QMediaPlayer *digest1;
    QMediaPlayer *digest2;
    bool digesting;
};


//LilyPad
class LilyPad: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(LilyPad)
public:
    LilyPad();
    virtual bool canGrow(int x, int y) const;
};

class LilyPadInstance: public PlantInstance
{
public:
    LilyPadInstance(const Plant *plant);
};


//TangleKlep
class TangleKlep: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(TangleKlep)
public:
    TangleKlep();
    virtual bool canGrow(int x, int y) const;
};

class TangleKlepInstance: public PlantInstance
{
public:
    TangleKlepInstance(const Plant *plant);
    virtual void initTrigger();
    virtual void Attack(ZombieInstance *zombie);
private:
    MoviePixmapItem *Grab;
};


//Spikeweed
class Spikeweed: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Spikeweed)
public:
    Spikeweed();
};

class SpikeweedInstance: public PlantInstance
{
public:
    SpikeweedInstance(const Plant *plant);
    virtual void initTrigger();
};


//Spikerock
class Spikerock: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Spikerock)
public:
    Spikerock();
    virtual bool canGrow(int x, int y) const;
};

class SpikerockInstance: public PlantInstance
{
public:
    SpikerockInstance(const Plant *plant);
    virtual void initTrigger();
};


//Bullet
class Bullet
{
public:
    Bullet(GameScene *scene, int type, int row, qreal from, qreal x, qreal y, qreal zvalue,  int direction);
    ~Bullet();
    void start();
private:
    void move();

    QUuid uuid;
    GameScene *scene;
    int count, type, row, direction;
    qreal from;
    QGraphicsPixmapItem *picture;
};


//Garlic
class Garlic: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Garlic)
public:
    Garlic();
};

class GarlicInstance: public PlantInstance
{
public:
    GarlicInstance(const Plant *plant);
    virtual void initTrigger();
};


//FlowerPot
class FlowerPot: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(FlowerPot)
public:
    FlowerPot();
};

class FlowerPotInstance: public PlantInstance
{
public:
    FlowerPotInstance(const Plant *plant);
};



/************************随机植物游戏***************************************/

//HypnoShroom
class HypnoShroom: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(HypnoShroom)
public:
    HypnoShroom();
};

class HypnoShroomInstance: public PlantInstance
{
public:
    HypnoShroomInstance(const Plant *plant);
};

//1
class HypnoShroom1: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(HypnoShroom1)
public:
    HypnoShroom1();
};

class HypnoShroomInstance1: public PlantInstance
{
public:
    HypnoShroomInstance1(const Plant *plant);
};

//2
class HypnoShroom2: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(HypnoShroom2)
public:
    HypnoShroom2();
};

class HypnoShroomInstance2: public PlantInstance
{
public:
    HypnoShroomInstance2(const Plant *plant);
};

//3
class HypnoShroom3: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(HypnoShroom3)
public:
    HypnoShroom3();
};

class HypnoShroomInstance3: public PlantInstance
{
public:
    HypnoShroomInstance3(const Plant *plant);
};

//4
class HypnoShroom4: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(HypnoShroom4)
public:
    HypnoShroom4();
};

class HypnoShroomInstance4: public PlantInstance
{
public:
    HypnoShroomInstance4(const Plant *plant);
};

//5
class HypnoShroom5: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(HypnoShroom5)
public:
    HypnoShroom5();
};

class HypnoShroomInstance5: public PlantInstance
{
public:
    HypnoShroomInstance5(const Plant *plant);
};

//6
class HypnoShroom6: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(HypnoShroom6)
public:
    HypnoShroom6();
};

class HypnoShroomInstance6: public PlantInstance
{
public:
    HypnoShroomInstance6(const Plant *plant);
};

//7
class HypnoShroom7: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(HypnoShroom7)
public:
    HypnoShroom7();
};

class HypnoShroomInstance7: public PlantInstance
{
public:
    HypnoShroomInstance7(const Plant *plant);
};

/******************************保龄球游戏*************************************/

//baolingqiu
class baolingqiu: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(baolingqiu)
public:
    baolingqiu();
};

class baolingqiuInstance: public PlantInstance
{
public:
    baolingqiuInstance(const Plant *plant);
    virtual void normalAttack(ZombieInstance *zombieInstance);
private:
    QMediaPlayer *baoling;
};
/*******************************僵尸植物**************************************/
/*
class ZombiePlant: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(ZombiePlant)
public:
    ZombiePlant();
};

class ZombiePlantInstance: public PlantInstance
{
public:
    ZombiePlantInstance(const Plant *plant);
    virtual void normalAttack(ZombieInstance *zombieInstance);
};
*/


Plant *PlantFactory(GameScene *scene, const QString &eName);
PlantInstance *PlantInstanceFactory(const Plant *plant);

#endif //PLANTS_VS_ZOMBIES_PLANT_H
