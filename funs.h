#ifndef FUNS_H
#define FUNS_H
#include "datastr.h"
#include "publics.h"
#include <QTextStream>

bool readFile(DataStr &dat, const QString &filename, FileCode &err);
bool transform(DataStr &dat, const QString &filename, FileCode &err);
void transformGroup(Group &p, QTextStream &writer_h, QTextStream &writer_cpp);

void wrt_h_D(Element &e, QTextStream &writer);
void wrt_h_R(Element &e, QTextStream &writer);
void wrt_h_W(Element &e, QTextStream &writer);
void wrt_cpp(Group &p, QTextStream &writer);
void wrt_cpp_L(Element &e, int i, QTextStream &writer);
void wrt_cpp_S(Element &e, QTextStream &writer);

#endif // FUNS_H
