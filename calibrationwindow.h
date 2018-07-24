#ifndef CALIBRATIONWINDOW_H
#define CALIBRATIONWINDOW_H

#include <TQObject.h>
#include <RQ_OBJECT.h>


#include <vector>

class TGTransientFrame;
class TRootEmbeddedCanvas;
class TGWindow;
class TGNumberEntry;
class TH1D;
class TGComboBox;
class TGTextView;

struct eventTime{
    Double_t time;      //istante dell'evento
    Double_t error;     //errore sul fit
};

class CalibrationWindow {
   RQ_OBJECT("CalibrationWindow")
private:
   TGTransientFrame          *fMain;
   TRootEmbeddedCanvas  *fEcanvas;
   TGNumberEntry        *calibEvtNE;
   TGComboBox           *shownChSwitch;
   TGTextView           *textView;

   static Double_t fpeaks(Double_t *x, Double_t *par);
   std::vector<Int_t> calibration(TString in_name, int ev_to_sum,
                     std::vector<std::vector<eventTime> > &timeCh, std::vector<TH1D> &hsumsumCh);
public:
   CalibrationWindow(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
   virtual ~CalibrationWindow();
   void calibrate();
   void showCalibration();
   void close();
};

#endif // CalibrationWINDOW_H
