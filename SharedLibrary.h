#ifndef SHAREDLIBRARY_H
#define SHAREDLIBRARY_H

#include <TH1D.h>
#include "constants.h"
namespace SharedLibrary{

Double_t zero_position(TH1D &h, Int_t polarity);
TH1D *subZero(TH1D &h, Int_t baseline, TH1D *hres);
Double_t CFDtime(TH1D &h0, Int_t nch, Int_t baseline, Double_t fraction, Int_t nshift, Int_t polarity);

float ** createMatrix (Int_t rows, Int_t columns);
void copyDataBlock(float dataBlock[][kNTimeSamples], int row, int rowSize, TH1D &container);
void shiftToTrigger(TH1D &histSignal);
}
#endif // SHAREDLIBRARY_H
