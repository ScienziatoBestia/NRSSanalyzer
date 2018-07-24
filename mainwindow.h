#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <TQObject.h>
#include <RQ_OBJECT.h>

enum ETestCommandIdentifiers {
    M_START_CALIBRATION,
    M_RUN_FILE_OPEN,
    M_CALIBRATION_FILE_OPEN
};

class TGPopupMenu;
class TGMenuBar;
class TGMainFrame;
class TGWindow;
class TGTextButton;
class TGTextEntry;
class TGCompositeFrame;
class TGStatusBar;
class TGTextView;
class TH1D;
class TGCheckButton;
class TGNumberEntryField;
class TTree;
class TFile;
#include <vector>

class MainWindow {
    RQ_OBJECT("MainWindow")
private:

    TString                 fRunFileName;
    TString                 fCalibrationFileName;

    std::vector <Int_t>         fNCalPointsCh;         //array of the number of calibration points for each channel
    //matrix of 8 row (one for each channel) with the lenght of each row equal to the number
    //of calibration points for that channel
    std::vector< std::vector<Double_t> *> fCalibrationTimeCh;
    std::vector< std::vector<Double_t> *> fCalibrationTimeErrorCh;

    //matrix of the peaks time and amplitude of the signal for each channel
    std::vector< std::vector< std::pair<Double_t, Double_t> > > fSignalPeaksCh;

    //matrix of the peaks time and amplitude  of the signal in time with Calibration of the channel
    std::vector < std::vector < std::pair<Double_t, Double_t> > > fInTimePeaksCh;

    int                     fEvNumber;

    int                     fNBaF1Scint;
    int                     fNBaF1Cher;
    int                     fNBaF1Double;

    int                     fNBaF2Scint;
    int                     fNBaF2Cher;
    int                     fNBaF2Double;

    int                     fNBaF3Scint;
    int                     fNBaF3Cher;
    int                     fNBaF3Double;

    int                     fNBaF4Scint;

    int                     fNLySOScint;

    std::vector<TH1D>       fSumSignalCh;


    

    TFile                   *fRunFile;

    TGMainFrame             *fMain;
    TGMenuBar               *fMenuBar;
    TGPopupMenu             *fToolMenu;

    TGCheckButton           *fBaF1ScintCheckButton, *fBaF2ScintCheckButton,
                            *fBaF3ScintCheckButton, *fBaF4ScintCheckButton;
    TGNumberEntryField      *fBaF1CountsNumberEntry, *fBaF2CountsNumberEntry,
                            *fBaF3CountsNumberEntry, *fBaF4CountsNumberEntry;
    TGCheckButton           *fBaF1CherenkovCheckButton, *fBaF2CherenkovCheckButton,
                            *fBaF3CherenkovCheckButton, *fBaF4CherenkovCheckButton;
    TGNumberEntryField      *fBaF1RateNumberEntry, *fBaF2RateNumberEntry,
                            *fBaF3RateNumberEntry, *fBaF4RateNumberEntry;


    TGStatusBar             *fStatusBar;

    TGTextView              *fLogTextView;



    char *openFileDialog();
    void findPeaks(std::vector<TH1D> &histSignalCh);
    Int_t findInTimePeaks();
    void createBaFGroup(TGCompositeFrame *p, const char *groupName, TGCheckButton *& fBaFScintCheckButton,
                        TGNumberEntryField *& fBaFCountsNumberEntry, TGCheckButton *& fBaFCherenkovCheckButton,
                        TGNumberEntryField *& fBaFRateNumberEntry);
    void readCalibration();
    void openRunFile();
    void closeRunFile();
    void analyzeEvent(int evNumber);
    int analyzeDetector(int scintCh, int cherCh);
    void visualization();
public:
    MainWindow(const TGWindow *p,UInt_t w,UInt_t h);
    virtual ~MainWindow();



    //slots
    void onStartButtonClicked();
    void handleMenu(Int_t id);
    void close();
};

#endif // MAINWINDOW_H
