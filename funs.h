#ifndef FUNS_H
#define FUNS_H
#include "datastr.h"
#include "publics.h"
#include <QTextStream>

/**
 * @brief readFile: read the descriptive xml file and store its informations to 'dat'
 * @param dat: the output variable for storing the informations
 * @param filename: the name of the descriptive xml file
 * @param err: error code during the file reading
 * @return whether the reading is successful
 */
bool readFile(DataStr &dat, const QString &filename, FileCode &err);
/**
 * @brief transform: create cpp(qt) codes for the desired xml files
 * @param dat: the variable storing all necessary informations
 * @param filename: name of original descriptive xml file
 * @param err: error code during the file writing
 * @return whether the transformation is successful
 */
bool transform(DataStr &dat, const QString &filename, FileCode &err);
/**
 * @brief transformGroup: write codes for encoding/coding a complex xml type
 * @param p: the variable storing all necessary informations
 * @param writer_h: the write-stream for header file
 * @param writer_cpp: the write-stream for cpp file
 */
void transformGroup(Group &p, QTextStream &writer_h, QTextStream &writer_cpp);

void wrt_h_D(Element &e, QTextStream &writer);
void wrt_h_R(Element &e, QTextStream &writer);
void wrt_h_W(Element &e, QTextStream &writer);
void wrt_cpp(Group &p, QTextStream &writer);
void wrt_cpp_L(Element &e, int i, QTextStream &writer);
void wrt_cpp_S(Element &e, QTextStream &writer);

#endif // FUNS_H
