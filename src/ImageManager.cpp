//
// Created by sun on 8/26/16.
//

#include "ImageManager.h"

//图像管理器
ImageManager *gImageCache;

//QPixmap 是为在屏幕上显示图像而设计和优化的
QPixmap ImageManager::load(const QString &path)
{
    if (pixmaps.find(path) == pixmaps.end())
        pixmaps.insert(path, QPixmap(":/images/" + path));
    return pixmaps[path];
}

void InitImageManager()
{
    gImageCache = new ImageManager;
}

void DestoryImageManager()
{
    delete gImageCache;
}
