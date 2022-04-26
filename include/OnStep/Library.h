// -----------------------------------------------------------------------------------
// Object libraries

#pragma once

#include <Arduino.h>

#pragma pack(1)
const int rec_size = 16;
typedef struct {
  char name[11]; // 11
  byte code;     // 1 (low 4 bits are object class, high are catalog #)
  uint16_t RA;   // 2
  uint16_t Dec;  // 2
} libRecBase_t;

typedef union {
  libRecBase_t libRec;
  byte libRecBytes[rec_size];
} libRec_t;
#pragma pack()

class Library
{
  public:
    Library();
    ~Library();
    
    void init();

    bool setCatalog(int num);

    // 16 byte record
    libRec_t list;
    
    void writeVars(char* name, int code, double RA, double Dec);
    void readVars(char* name, int* code, double* RA, double* Dec);

    bool firstRec();
    bool nameRec();
    bool firstFreeRec();
    bool prevRec();
    bool nextRec();
    bool gotoRec(long num);

    void clearCurrentRec(); // clears this record
    void clearLib();        // clears this library
    void clearAll();        // clears all libraries

    long recCount();        // actual number of records for this catalog
    long recFree();         // number records available for this catalog
    long recCountAll();     // actual number of records for this library
    long recFreeAll();      // number records available for this library
    long recPos;            // currently selected record#
    long recMax;            // last record#
    
  private:
    libRec_t readRec(long address);
    void writeRec(long address, libRec_t data);
    void clearRec(long address);
    inline double degRange(double d) { while (d >= 360.0) d-=360.0; while (d < 0.0)  d+=360.0; return d; }

    int catalog;

    long byteMin;
    long byteMax;
};

Library Lib;
char const * objectStr[] = {"UNK", "OC", "GC", "PN", "DN", "SG", "EG", "IG", "KNT", "SNR", "GAL", "CN", "STR", "PLA", "CMT", "AST"};

Library::Library()
{
  catalog=0;

  byteMin=200+pecBufferSize;
  byteMax=GSB;

  long byteCount=(byteMax-byteMin)+1;
  if (byteCount < 0) byteCount=0;
  if (byteCount > 262143) byteCount=262143; // maximum 256KB

  recMax=byteCount/rec_size; // maximum number of records
}

Library::~Library()
{
}

void Library::init() {
  // This is now in the Init() function, because on boards
  // with an I2C EEPROM nv.init() has to be called before
  // anything else
  firstRec();
}

bool Library::setCatalog(int num)
{
  if (num < 0 || num > 14) return false;

  catalog=num;
  return firstRec();
}

void Library::writeVars(char* name, int code, double RA, double Dec)
{
  libRec_t work;
  for (int l=0; l < 11; l++) work.libRec.name[l] = name[l];
  work.libRec.code = (code | (catalog<<4));

  // convert into ulong, RA=0..360
  RA=degRange(RA)/360.0;
  // convert into ulong, Dec=0..180
  if (Dec > 90.0) Dec=90.0; if (Dec < -90.0) Dec=-90.0; Dec=Dec+90.0; Dec=Dec/180.0;
  uint16_t r=round(RA*65536.0);
  uint16_t d=round(Dec*65536.0);
  
  work.libRec.RA   = r;
  work.libRec.Dec  = d;

  writeRec(recPos,work);
}

void Library::readVars(char* name, int* code, double* RA, double* Dec)
{
  libRec_t work;
  work=readRec(recPos);

  int cat = work.libRec.code>>4;

  // empty? or not found
  if (cat == 15 || cat != catalog) { name[0]=0; *code=0; *RA=0.0; *Dec=0.0; return; }

  for (int l=0; l < 11; l++) name[l]=work.libRec.name[l]; name[11]=0;
  
  *code = work.libRec.code & 15;
  uint16_t r = work.libRec.RA;
  uint16_t d = work.libRec.Dec;
  
  // convert from ulong
  *RA=(double)r;
  *RA=(*RA/65536.0)*360.0;
  *Dec=(double)d;
  *Dec=((*Dec/65536.0)*180.0)-90.0;
}

libRec_t Library::readRec(long address)
{
  libRec_t work;
  long l=address*rec_size+byteMin;
  nv.readBytes(l,(uint8_t*)&work.libRecBytes,16);
  return work;
}

void Library::writeRec(long address, libRec_t data)
{
  if (address >= 0 && address < recMax) {
    long l=address*rec_size+byteMin;
    for (int m=0; m < 16; m++) nv.write(l+m,data.libRecBytes[m]);
  }
}

void Library::clearRec(long address)
{
  if (address >= 0 && address < recMax) {
    long l=address*rec_size+byteMin;
    int code=15<<4;
    nv.write(l+11,(byte)code); // catalog code 15 = deleted
  }
}

bool Library::firstRec()
{
  libRec_t work;

  // see if first record is for the currentLib
  recPos=0;
  work=readRec(recPos);
  int cat=(int)work.libRec.code>>4;
  if (work.libRec.name[0] != '$' && cat == catalog) return true;

  // otherwise find the first one, if it exists
  return nextRec();
}

// move to the catalog name rec
bool Library::nameRec()
{
  libRec_t work;

  int cat;
  recPos=-1;
  
  do
  {
    recPos++; if (recPos >= recMax) break;
    work=readRec(recPos);

    cat=(int)work.libRec.code>>4;

    if (work.libRec.name[0] == '$' && cat == catalog) break;
  } while (recPos < recMax);
  if (recPos >= recMax) { recPos=recMax-1; return false; }

  return true;
}

// move to first unused record for this catalog
bool Library::firstFreeRec()
{
  libRec_t work;

  int cat;
  recPos=-1;
  
  do
  {
    recPos++; if (recPos >= recMax) break;
    work=readRec(recPos);

    cat=(int)work.libRec.code>>4;
  
    if (cat == 15) break; // unused?
  } while (recPos < recMax);
  if (recPos >= recMax) { recPos=recMax-1; return false; }

  return true;
}

// read the previous record, if it exists
bool Library::prevRec()
{
  libRec_t work;

  int cat;
  
  do
  {
    recPos--; if (recPos < 0) break;
    work=readRec(recPos);

    cat=(int)work.libRec.code>>4;
    if (work.libRec.name[0] != '$' && cat == catalog) break;
  } while (recPos >= 0);
  if (recPos < 0) { recPos=0; return false; }

  return true;
}

// read the next record, if it exists
bool Library::nextRec()
{
  libRec_t work;

  int cat;
 
  do
  {
    recPos++; if (recPos >= recMax) break;
    work=readRec(recPos);

    cat=(int)work.libRec.code>>4;
    if (work.libRec.name[0] != '$' && cat == catalog) break;
  } while (recPos < recMax);
  if (recPos >= recMax) { recPos=recMax-1; return false; }

  return true;
}

// read the specified record (of this catalog), if it exists
bool Library::gotoRec(long num)
{
  libRec_t work;

  int cat;
  long r=0;
  long c=0;
  
  for (long l=0; l < recMax; l++) {
    work=readRec(l); r=l;

    cat=(int)work.libRec.code>>4;
    if (work.libRec.name[0] != '$' && cat == catalog) c++;
    if (c == num) break;
  }
  if (c == num) { recPos=r; return true; } else return false;
}

// count all catalog records
long Library::recCount()
{
  libRec_t work;

  int cat;
  long c=0;
  
  for (long l=0; l < recMax; l++) {
    work=readRec(l);

    cat=(int)work.libRec.code>>4;
    if (work.libRec.name[0] != '$' && cat == catalog) c++;
  }
  
  return c;
}

// count all library records (index or otherwise)
long Library::recCountAll()
{
  libRec_t work;

  int cat;
  long c=0;
  
  for (long l=0; l < recMax; l++) {
    work=readRec(l);

    cat=(int)work.libRec.code>>4;
    if (cat >= 0 && cat <= 14) c++;
  }
  
  return c;
}

// library records available
long Library::recFreeAll()
{
  return recMax-recCountAll();
}

// mark this catalog record as empty
void Library::clearCurrentRec()
{
  libRec_t work;

  int cat;

  work=readRec(recPos);

  cat=(int)work.libRec.code>>4;
  if (cat == catalog) clearRec(recPos);
}

// mark all catalog records as empty
void Library::clearLib()
{
  libRec_t work;

  int cat;

  for (long l=0; l < recMax; l++) {
    work=readRec(l);

    cat=(int)work.libRec.code>>4;
    if (cat == catalog) clearRec(l);
  }
}

// mark all records as empty
void Library::clearAll()
{
  for (long l=0;l < recMax;l++) clearRec(l);
}
