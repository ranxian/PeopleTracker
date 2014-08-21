#ifndef __XML_HELPER_H
#define __XML_HELPER_H
#include "tinyxml2.h"
#include "dataReader.h"
void writeFrameToXml(tinyxml2::XMLPrinter &printer, vector<Result2D> &results);
#endif