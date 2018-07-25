#include "mainwindow.h"
#include "calibrationwindow.h"
#include "SharedLibrary.h"
#include "constants.h"

#include <array>

#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TH1D.h>
#include <TLine.h>
#include <iostream>
#include <TTree.h>
#include <TFile.h>
#include <TSpectrum.h>
#include <Math/PdfFuncMathCore.h>


#include <RQ_OBJECT.h>
#include <TGClient.h>
#include <TGComboBox.h>
#include <TGNumberEntry.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <TGLabel.h>
#include <TGTextView.h>
#include <TApplication.h>
#include <TGDockableFrame.h>
#include <TGMenu.h>
#include <TGFileDialog.h>
#include <TGStatusBar.h>



using namespace std;

const char *filetypes[] = { "All files",     "*",
                            "ROOT files",    "*.root",
                            "ROOT macros",   "*.C",
                            "Text files",    "*.[tT][xX][tT]",
                            0,               0 };


char * MainWindow::openFileDialog()
{
    static TString dir(".");
    static TGFileInfo fi;
    fi.fFileTypes = filetypes;
    fi.fIniDir    = StrDup(dir);
    //printf("fIniDir = %s\n", fi.fIniDir);
    new TGFileDialog(gClient->GetRoot(), fMain, kFDOpen, &fi);
    //printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
    dir = fi.fIniDir;

    return fi.fFilename;
}

MainWindow::MainWindow(const TGWindow *p,UInt_t w,UInt_t h)
    : fRunFileName(""), fCalibrationFileName(0), fRunFile(0)
{

    //inizialize
    fNBaF1Cher = fNBaF1Double = fNBaF1Scint = fNBaF2Cher = fNBaF2Double =
            fNBaF2Scint = fNBaF3Cher = fNBaF3Double = fNBaF3Scint = fNBaF4Scint = fNLySOScint = 0;

    fNCalPointsCh.resize(kNCh);

    fSumSignalCh.reserve(kNCh);
    for(int ch=0; ch < kNCh; ch++){
        TH1D temp(Form("SumSignal%d", ch), "", kNTimeSamples, 0., (Double_t) kNTimeSamples);
        fSumSignalCh.push_back(temp);
    }


    fCalibrationTimeCh.resize(kNCh);
    fCalibrationTimeErrorCh.resize(kNCh);

    fInTimePeaksCh.resize(kNCh);

    fSignalPeaksCh.resize(kNCh);



    // Create a main frame
    fMain = new TGMainFrame(p,w,h);
    fMain->SetWindowName("NRSS Analyzer");
    fMain->Connect("CloseWindow()","MainWindow",this,"close()");


    //menu
    TGLayoutHints *menuLHints =  new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 0);

    fToolMenu = new TGPopupMenu(gClient->GetRoot());

    fToolMenu->AddEntry("&Start Calibration", M_START_CALIBRATION);
    fToolMenu->AddEntry("Open &Calibration", M_CALIBRATION_FILE_OPEN);
    fToolMenu->AddEntry("Open &Run", M_RUN_FILE_OPEN);

    fToolMenu->Connect("Activated(Int_t)", "MainWindow", this, "handleMenu(Int_t)");

    fMenuBar = new TGMenuBar(fMain);
    fMenuBar->AddPopup("&Tools", fToolMenu, menuLHints);

    fMain->AddFrame(fMenuBar, menuLHints);

    //Run Gruops and canvas
    TGHorizontalFrame *fHorizontalFrame0 = new TGHorizontalFrame(fMain);
    TGVerticalFrame *fVerticalFrame1 = new TGVerticalFrame(fHorizontalFrame0);

    //Start Button
    TGTextButton *fStartButton = new TGTextButton(fVerticalFrame1, "Start !");
    fStartButton->Connect("Clicked()", "MainWindow", this, "onStartButtonClicked()");
    fVerticalFrame1->AddFrame(fStartButton, new TGLayoutHints(kLHintsCenterX | kLHintsTop |
                                                              kLHintsExpandX | kLHintsExpandY ,2,2,2,2));
    //end Start Button


    //BaF1 Groups
    createBaFGroup(fVerticalFrame1, "BaF 1 Detector", fBaF1ScintCheckButton, fBaF1CountsNumberEntry,
                   fBaF1CherenkovCheckButton, fBaF1RateNumberEntry);
    createBaFGroup(fVerticalFrame1, "BaF 2 Detector", fBaF2ScintCheckButton, fBaF2CountsNumberEntry,
                   fBaF2CherenkovCheckButton, fBaF2RateNumberEntry);
    createBaFGroup(fVerticalFrame1, "BaF 3 Detector", fBaF3ScintCheckButton, fBaF3CountsNumberEntry,
                   fBaF3CherenkovCheckButton, fBaF3RateNumberEntry);
    createBaFGroup(fVerticalFrame1, "BaF 4 Detector", fBaF4ScintCheckButton, fBaF4CountsNumberEntry,
                   fBaF4CherenkovCheckButton, fBaF4RateNumberEntry);


    //log widget
    TGDockableFrame *fDockableFrame = new TGDockableFrame (fVerticalFrame1 );
    fDockableFrame->SetFixedSize(kFALSE);
    fDockableFrame->SetWindowName("Log");
    fLogTextView = new TGTextView(fDockableFrame);
    fDockableFrame->AddFrame(fLogTextView, new TGLayoutHints(kLHintsBottom |kLHintsExpandY| kLHintsExpandX , 2,2,2,2 ));
    fVerticalFrame1->AddFrame(fDockableFrame, new TGLayoutHints(kLHintsBottom |kLHintsExpandY| kLHintsExpandX , 2,2,2,2 ) );

    fHorizontalFrame0->AddFrame(fVerticalFrame1, new TGLayoutHints(kLHintsExpandY , 2,2,2,2 ));


    //Canvas
    TGLayoutHints *canvasLayoutHints = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2,2,2,2);
    TGVerticalFrame *fCanvasVFrame = new TGVerticalFrame(fHorizontalFrame0);

    TGHorizontalFrame *fCanvasHFrame1 = new TGHorizontalFrame(fCanvasVFrame);
    fBaF1Canvas = new TRootEmbeddedCanvas("BaF1Canvas", fCanvasHFrame1, 200, 200);
    fCanvasHFrame1->AddFrame(fBaF1Canvas, canvasLayoutHints);

    fBaF2Canvas = new TRootEmbeddedCanvas("BaF2Canvas", fCanvasHFrame1, 200, 200);
    fCanvasHFrame1->AddFrame(fBaF2Canvas, canvasLayoutHints);

    fCanvasVFrame->AddFrame(fCanvasHFrame1, canvasLayoutHints);

    TGHorizontalFrame *fCanvasHFrame2 = new TGHorizontalFrame(fCanvasVFrame);
    fBaF3Canvas = new TRootEmbeddedCanvas("BaF3Canvas", fCanvasHFrame2, 200, 200);
    fCanvasHFrame2->AddFrame(fBaF3Canvas, canvasLayoutHints);

    fBaF4Canvas = new TRootEmbeddedCanvas("BaF4Canvas", fCanvasHFrame2, 200, 200);
    fCanvasHFrame2->AddFrame(fBaF4Canvas, canvasLayoutHints);

    fCanvasVFrame->AddFrame(fCanvasHFrame2, canvasLayoutHints);

    fHorizontalFrame0->AddFrame(fCanvasVFrame, canvasLayoutHints);

    fMain->AddFrame(fHorizontalFrame0, canvasLayoutHints);
    //End Canvas


    //status Bar
    Int_t parts[] = {30, 30, 10, 30};
    fStatusBar = new TGStatusBar(fMain, 50, 10, kVerticalFrame);
    fStatusBar->SetParts(parts, 4);
    fMain->AddFrame(fStatusBar, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));

    if(fCalibrationFileName == "")
        fStatusBar->AddText("Please set a calibration file!", 0);
    if(fRunFileName == "")
        fStatusBar->AddText("Please set a run file!", 1);



    fMain->MapSubwindows();

    fMain->Resize(fMain->GetDefaultSize());
    fMain->MapWindow();



}


MainWindow::~MainWindow() {
    // Clean up used widgets: frames, buttons, layout hints
    if(fRunFile)
        closeRunFile();
    fNCalPointsCh.clear();
    fMain->Cleanup();
    delete fMain;
}


void MainWindow::createBaFGroup(TGCompositeFrame *p, const char *groupName, TGCheckButton *&fBaFScintCheckButton,
                                TGNumberEntryField *&fBaFCountsNumberEntry, TGCheckButton *&fBaFCherenkovCheckButton,
                                TGNumberEntryField *&fBaFRateNumberEntry)
{
    //BaF Group
    TGGroupFrame *fBaFGroup = new TGGroupFrame(p,groupName);

    // horizontal frame 1
    TGHorizontalFrame *fHorizontalFrame1 = new TGHorizontalFrame(fBaFGroup);
    fBaFScintCheckButton = new TGCheckButton(fHorizontalFrame1,"Scintillation");
    fHorizontalFrame1->AddFrame(fBaFScintCheckButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 20, 2, 2));

    fBaFCountsNumberEntry = new TGNumberEntryField(fHorizontalFrame1,-1,0,TGNumberEntry::EStyle::kNESInteger);
    //TGTextView *fBaFCountsNumberEntry = new TGTextView(fHorizontalFrame1);
    fBaFCountsNumberEntry->SetWidth(60);
    fBaFCountsNumberEntry->SetEnabled(kFALSE);
    fBaFCountsNumberEntry->SetBackgroundColor((Pixel_t)0xffffff);   //white
    fHorizontalFrame1->AddFrame(fBaFCountsNumberEntry, new TGLayoutHints(kLHintsRight | kLHintsExpandY,2,2,2,2));

    TGLabel *fLabel2 = new TGLabel(fHorizontalFrame1, "Counts");
    fHorizontalFrame1->AddFrame(fLabel2, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 2, 2, 2, 2));


    fBaFGroup->AddFrame(fHorizontalFrame1, new TGLayoutHints(kLHintsExpandX | kLHintsTop,2,2,2,2));

    // horizontal frame 2
    TGHorizontalFrame *fHorizontalFrame2 = new TGHorizontalFrame(fBaFGroup);
    fBaFCherenkovCheckButton = new TGCheckButton(fHorizontalFrame2,"Cherenkov");
    fHorizontalFrame2->AddFrame(fBaFCherenkovCheckButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 20, 2, 2));

    fBaFRateNumberEntry = new TGNumberEntryField(fHorizontalFrame2,-1,0,TGNumberEntry::EStyle::kNESInteger);
    fBaFRateNumberEntry->SetEnabled(kFALSE);
    fBaFRateNumberEntry->SetBackgroundColor((Pixel_t)0xffffff);   //white
    fBaFRateNumberEntry->SetWidth(60);
    fHorizontalFrame2->AddFrame(fBaFRateNumberEntry, new TGLayoutHints(kLHintsRight  | kLHintsCenterY,2,2,2,2));

    TGLabel *fLabel3 = new TGLabel(fHorizontalFrame2, "Rate");
    fHorizontalFrame2->AddFrame(fLabel3, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 2, 2, 2, 2));

    fBaFGroup->AddFrame(fHorizontalFrame2, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY,2,2,2,2));


    p->AddFrame(fBaFGroup, new TGLayoutHints(kLHintsNormal,2,2,2,2));
    //End BaF1 Group


}

void MainWindow::close()
{
    gApplication->Terminate(0);
}

void MainWindow::handleMenu(Int_t id)
{
    // Handle menu items.

    switch (id) {
    case M_START_CALIBRATION:
        new CalibrationWindow(gClient->GetRoot(), (TGWindow *) this, 600, 400);
        break;

    case M_RUN_FILE_OPEN:
    {
        fRunFileName = openFileDialog();
        TString nfile;
        nfile = fRunFileName(fRunFileName.Last('/') + 1, 100);
        fStatusBar->AddText((TString)("Run File: ") + nfile, 1);
    }
        break;

    case M_CALIBRATION_FILE_OPEN:
        fCalibrationFileName = openFileDialog();

        TString nfile;
        nfile = fCalibrationFileName(fCalibrationFileName.Last('/') + 1, 100);
        fStatusBar->AddText((TString)("Calibration File: " + nfile), 0);
        break;


    }
}

//start analysis
void MainWindow::onStartButtonClicked()
{
    fLogTextView->AddLine("Starting analysis...");

    //start analysis
    //reading calibration file
    readCalibration();

    openRunFile();

    //to re do 100 times
    fEvNumber = 1;
    analyzeEvent(fEvNumber);
    findInTimePeaks();

    fNBaF1Double += analyzeDetector(0, 1);
    fNBaF2Double += analyzeDetector(2, 3);
    fNBaF3Double += analyzeDetector(4, 5);

    visualization();



}

void MainWindow::visualization()
{

    //Baf1 visualization
    if(fBaF1ScintCheckButton->IsOn() && !fBaF1CherenkovCheckButton->IsOn() ){
        fBaF1CountsNumberEntry->SetNumber(fNBaF1Scint);
        TCanvas *fCanvas1 = fBaF1Canvas->GetCanvas();
        fCanvas1->cd();
        fSumSignalCh[0].SetLineColor(2);
        fSumSignalCh[0].Draw();
        for(int i=0; i< fNCalPointsCh[0]; i++)
        {
            TLine *timeLine = new TLine(fCalibrationTimeCh[0]->at(i), fSumSignalCh[0].GetMinimum(), fCalibrationTimeCh[0]->at(i), fSumSignalCh[0].GetMaximum());
            timeLine->SetLineColor(12);
            timeLine->SetLineWidth(2);
            timeLine->SetLineStyle(7);
            timeLine->Draw("same");
        }

        fCanvas1->Modified();
        fCanvas1->Update();
    }
    else if(!fBaF1ScintCheckButton->IsOn() && fBaF1CherenkovCheckButton->IsOn()){
        fBaF1CountsNumberEntry->SetNumber(fNBaF1Cher);
        TCanvas *fCanvas1 = fBaF1Canvas->GetCanvas();
        fCanvas1->cd();
        fSumSignalCh[1].SetLineColor(6);
        fSumSignalCh[1].Draw();
        for(int i=0; i< fNCalPointsCh[1]; i++)
        {
            TLine *timeLine = new TLine(fCalibrationTimeCh[1]->at(i), fSumSignalCh[1].GetMinimum(), fCalibrationTimeCh[1]->at(i), fSumSignalCh[1].GetMaximum());
            timeLine->SetLineColor(12);
            timeLine->SetLineWidth(2);
            timeLine->SetLineStyle(7);
            timeLine->Draw("same");
        }
        fCanvas1->Modified();
        fCanvas1->Update();
    }
    else if(fBaF1ScintCheckButton->IsOn() && fBaF1CherenkovCheckButton->IsOn()){
        fBaF1CountsNumberEntry->SetNumber(fNBaF1Double);
    }

    //BaF2 visualization
    if(fBaF2ScintCheckButton->IsOn() && !fBaF2CherenkovCheckButton->IsOn()){
        fBaF2CountsNumberEntry->SetNumber(fNBaF2Scint);
    }
    else if(!fBaF2ScintCheckButton->IsOn() && fBaF2CherenkovCheckButton->IsOn()){
        fBaF2CountsNumberEntry->SetNumber(fNBaF2Cher);
    }
    else if(fBaF2ScintCheckButton->IsOn() && fBaF2CherenkovCheckButton->IsOn()){
        fBaF2CountsNumberEntry->SetNumber(fNBaF2Double);
    }

    //BaF3 visualization
    if(fBaF3ScintCheckButton->IsOn() && !fBaF3CherenkovCheckButton->IsOn()){
        fBaF3CountsNumberEntry->SetNumber(fNBaF3Scint);
    }
    else if(!fBaF3ScintCheckButton->IsOn() && fBaF3CherenkovCheckButton->IsOn()){
        fBaF3CountsNumberEntry->SetNumber(fNBaF3Cher);
    }
    else if(fBaF3ScintCheckButton->IsOn() && fBaF3CherenkovCheckButton->IsOn()){
        fBaF3CountsNumberEntry->SetNumber(fNBaF3Double);
    }

    //BaF4 visualization
    if(fBaF4ScintCheckButton->IsOn() && !fBaF4CherenkovCheckButton->IsOn()){
        fBaF4CountsNumberEntry->SetNumber(fNBaF4Scint);
    }
    else if(!fBaF4ScintCheckButton->IsOn() && fBaF4CherenkovCheckButton->IsOn()){
        fBaF4CountsNumberEntry->SetNumber(0);
    }
    else if(fBaF4ScintCheckButton->IsOn() && fBaF4CherenkovCheckButton->IsOn()){
        fBaF4CountsNumberEntry->SetNumber(0);
    }

}

//look for in time peak of scintillation and Cherenkov channel at the same detector
int MainWindow::analyzeDetector(int scintCh, int cherCh)
{
    //trovare le coincidenze tra i due canali
    //per il momento ci limitiamo a controllare che vi sia coincidenza senza valutare
    //cooerenza energetica tra i due canali
    Int_t goodCounts = 0;
    Double_t scintThres = 0.;
    Double_t cherThres = 0.;
    for(int i=0; i< fNCalPointsCh[scintCh]; ++i){
        if(fInTimePeaksCh[scintCh][i].second > scintThres &&
                fInTimePeaksCh[cherCh][i].second > cherThres){
            goodCounts++;
        }
    }
    return goodCounts;
}

//this function find peaks in the histSignalCH (for each channel) and fill fSignalPeaksCh
void MainWindow::findPeaks(vector<TH1D> &histSignalCh)
{
    TCanvas *canv = new TCanvas("canv", "Titolo", 3200, 2400);
    for(int ch = 0; ch < kNCh; ch++){
        int nCalibrationPeaksCh = fCalibrationTimeCh[ch]->size();
        //set range where to look for peaks
        histSignalCh[ch].SetAxisRange( fCalibrationTimeCh[ch]->at(0) -20, fCalibrationTimeCh[ch]->at(nCalibrationPeaksCh-1) + 20);
        //Use TSpectrum to find the peak candidates
        TSpectrum s(3 * nCalibrationPeaksCh);   //initializzation value for TSpectrum
        Int_t nfound = s.Search(& histSignalCh[ch], 2,"nodraw",0.10);   //parameters to search for peaks


        //Estimate background using TSpectrum::Background
        TH1 *hb = s.Background(& histSignalCh[ch],20,"same");

        TH1D hback("hback", "hback", kNTimeSamples, 0., (Double_t) kNTimeSamples);
        hback.Add(&histSignalCh[ch]);
        hback.Add(hb,-1);          //sottrazione del background
        vector<Double_t> temp;
        temp.reserve(nfound);
        for(int i=0; i< nfound; i++){                           //Possible time consuming !!!!
            temp.push_back(s.GetPositionX()[i]);
        }
        sort(temp.begin() , temp.end());

        //prepara gli array dei valori da restituire
        fSignalPeaksCh[ch].resize(nfound);
        for(int i=0; i< nfound; i++){
            fSignalPeaksCh[ch][i].first = temp[i];
            fSignalPeaksCh[ch][i].second = hback.GetBinContent(hback.FindBin(temp[i]));
        }



      /*  for(int i=0; i< nfound; ++i){
            cout << fSignalPeaksCh[ch][i] << "\t\t";
        }
        cout << endl;*/

     /*   histSignalCh[ch].Draw();
        hb->Draw("same");
        // hsumback->Draw("same");
        for(int i=0; i< fNCalPointsCh[ch]; i++)
        {
            TLine *timeLine = new TLine(fCalibrationTimeCh[ch]->at(i), histSignalCh[ch].GetMinimum(), fCalibrationTimeCh[ch]->at(i), histSignalCh[ch].GetMaximum());
            timeLine->SetLineColor(12);
            timeLine->SetLineWidth(2);
            timeLine->SetLineStyle(7);
            timeLine->Draw("same");
        }
        TString nomefile = histSignalCh[ch].GetName();
        nomefile.Append(".png");
        canv->SaveAs(nomefile);*/

        //s->Clear();
    }
    //delete s;
    //delete hb;
    // delete canv;


}



void MainWindow::readCalibration(){
    if(fCalibrationFileName == ""){
        fLogTextView->AddLine("Please set a calibration file, run aborted");
        return;
    }
    TFile calibrationFile(fCalibrationFileName, "READ");
    TTree *calibrationTree = (TTree *) calibrationFile.Get("calibration_tree");


    //retrive the number of calibration poins
    for (int i=0; i< kNCh; i++){
        calibrationTree->SetBranchAddress(Form("nCalPointsCh%d", i) ,&fNCalPointsCh[i]);
    }
    //calibrationTree->GetEntry(0);

   // std::vector<Double_t> * row1=0;
   // std::vector<Double_t> * row2=0;

    for (int i=0; i< kNCh; i++){
        //fCalibrationTimeCh[i].resize(fNCalPointsCh[i], 0);
        //fCalibrationTimeErrorCh[i].resize(fNCalPointsCh[i], 0);

        //row1->resize(fNCalPointsCh[i], 0.);
        //row2.resize(16, 0.);
        calibrationTree->SetBranchAddress(Form("timeCh%dtime", i) ,  &fCalibrationTimeCh[i]);

        calibrationTree->SetBranchAddress(Form("timeCh%derror",i) ,  &fCalibrationTimeErrorCh[i]);


        //fCalibrationTimeCh[i]= *row1;
        //fCalibrationTimeErrorCh[i]= *row2;
    }
    calibrationTree->GetEntry(0);   //retrive data

    calibrationFile.Close();
    //end of calibration file reading


    for(int ch = 0; ch < kNCh; ch++){
        fInTimePeaksCh[ch].reserve(fNCalPointsCh[ch]);
    }

}

void MainWindow::openRunFile()
{
    if(fRunFileName == ""){
        fLogTextView->AddLine("Please set a run file, run aborted");
        return;
    }
    fRunFile = new TFile(fRunFileName ,"READ");

}

void MainWindow::closeRunFile()
{
    fRunFile->Close();
    delete fRunFile;
}

void MainWindow::analyzeEvent(int evNumber)
{
    //read run file
    if(!fRunFile){
        openRunFile();
        return;
    }

    //read data block from file
    float fNRSSdataBlock[kNCh][kNTimeSamples];

    TTree *tree = (TTree *) fRunFile->Get("rawdata");
    tree->SetBranchAddress("dataNRSS", fNRSSdataBlock);
    tree->GetEntry(evNumber);


    //matrix of the peak instants for each channel
    //vector< vector<Double_t>> peaksTimeCh(kNCh, vector<Double_t>());

    //coping the datas inside histograms to treat his
    vector<TH1D> histSignalCh(kNCh, TH1D());
    //vector <TH1D> histBackgroundCh(kNCh, TH1D());
    //vector <Int_t> nPeaksFoundCh(kNCh, 0);

    for(int ch=0; ch<kNCh; ++ch ){
        histSignalCh[ch].SetName(Form("h%d",ch));
        histSignalCh[ch].SetBins(kNTimeSamples, 0., (Double_t) kNTimeSamples);
        for(int j=0; j< kNTimeSamples; ++j){
            histSignalCh[ch].SetBinContent(j, fNRSSdataBlock[ch][j]);
        }


        //search for the position of the trigger and set to zero first histogram bins
        SharedLibrary::shiftToTrigger(histSignalCh[ch]);
        fSumSignalCh[ch].Add(&histSignalCh[ch]);



        //histBackgroundCh[ch].SetName(Form("histBackground%d", i));
        //histBackgroundCh[ch].SetTitle(Form("channel0 less background"));
        //histBackgroundCh[ch].SetBins(kNTimeSamples, 0., (Double_t) kNTimeSamples);


    }
    findPeaks(histSignalCh);




    /*
    std::pair<Double_t, Double_t> *inTimePeaksCh0 = new std::pair<Double_t, Double_t>[nCalPointsCh0];
    std::pair<Double_t, Double_t> *inTimePeaksCh1 = new std::pair<Double_t, Double_t>[nCalPointsCh1];
*/

}



Int_t MainWindow::findInTimePeaks()
{


    Double_t timeWidth = 2.0;   //nanosecondi
    Double_t binWidth = timeWidth * SAMPLING_RATE;

    for(int ch =0; ch < kNCh; ch++){
        Int_t foundPeak=0;
        Int_t nInTimePeaks = 0;

        Int_t doublePeaks = 0;  //doppi picchi di un canale all'interno della banda temporale
        Int_t peakNotFound = 0;
        Int_t peakIndex = 0;
        int p=0;

        for(int i = 0; i< fNCalPointsCh[ch]; ++i)
        {
            std::pair<Double_t, Double_t> inTimePeak;


            if(p >= fSignalPeaksCh[ch].size()){
                break;
            }

            while( fSignalPeaksCh[ch][p].first < fCalibrationTimeCh[ch]->at(i) && p < fSignalPeaksCh[ch].size()){
                p++;
            }


            if(fabs(fCalibrationTimeCh[ch]->at(i)-fSignalPeaksCh[ch][p].first)< binWidth/2) {
                foundPeak++;
                peakIndex = p;
            }
            if(p!=0 && fabs(fCalibrationTimeCh[ch]->at(i)-fSignalPeaksCh[ch][p-1].first)< binWidth/2) {
                foundPeak++;
                peakIndex = p-1;
            }
            if(foundPeak){
                if(foundPeak == 2){
                    doublePeaks++;
                    fLogTextView->AddLine(Form("RunFile %s - Event %d - Channel %d - CalPoint %d:"
                                               , fRunFileName.Data(), fEvNumber, ch, i));
                    fLogTextView->AddLine("WARNING: multiple peak found");
                }
                else{
                    nInTimePeaks++;
                    inTimePeak.first = fSignalPeaksCh[ch][peakIndex].first;
                    inTimePeak.second = fSignalPeaksCh[ch][peakIndex].second;
                    fInTimePeaksCh[ch].push_back(inTimePeak);

                }
                foundPeak = 0;
            }
            else{
                peakNotFound++;
                fLogTextView->AddLine(Form("RunFile %s - Event %d - Channel %d - CalPoint %d:"
                                           , fRunFileName.Data(), fEvNumber, ch, i));
                fLogTextView->AddLine("No peaks in calibration point!");
            }
        }
        fLogTextView->AddLine(Form("p= %d",p));

        switch (ch){
        case 0:
            fNBaF1Scint += nInTimePeaks;
            break;
        case 1:
            fNBaF1Cher += nInTimePeaks;
            break;
        case 2:
            fNBaF2Scint += nInTimePeaks;
            break;
        case 3:
            fNBaF2Cher += nInTimePeaks;
            break;
        case 4:
            fNBaF3Scint += nInTimePeaks;
            break;
        case 5:
            fNBaF3Cher += nInTimePeaks;
            break;
        case 6:
            fNBaF4Scint += nInTimePeaks;
            break;
        case 7:
            fNLySOScint += nInTimePeaks;
            break;
        }

    }
}
