#ifndef GLOBAL_H
#define GLOBAL_H

#include <QDir>

const QString str_learningRootDir = QDir::currentPath() + "/../learningFile/";
const QString str_newEventDir { str_learningRootDir + "newEvent/" };
const QString str_knownDir { str_learningRootDir + "known/" };

#endif // GLOBAL_H
