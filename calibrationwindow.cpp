#include "calibrationwindow.h"
#include "SharedLibrary.h"
#include "constants.h"

#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TH1D.h>
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

using namespace std;


CalibrationWindow::CalibrationWindow(const TGWindow *p, const TGWindow *main, UInt_t w,UInt_t h) {
    // Create a main frame
    fMain = new TGTransientFrame(p,main, w,h);
    fMain->Connect("CloseWindow()","CalibrationWindow",this,"close()");

    TGHorizontalFrame *h1frame = new TGHorizontalFrame(fMain,w,40);
    TGLabel *labelChSwitch = new TGLabel(h1frame, "Show Channel: ");
    h1frame->AddFrame(labelChSwitch,  new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5,5));

    shownChSwitch = new TGComboBox (h1frame);
    shownChSwitch->SetWidth(100);
    shownChSwitch->SetHeight(20);
    h1frame->AddFrame(shownChSwitch, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5,5));

    shownChSwitch->AddEntry("Ch0 (BaF 1A)", 0);
    shownChSwitch->AddEntry("Ch1 (BaF 1B)", 1);
    shownChSwitch->AddEntry("Ch2 (BaF 2A)", 2);
    shownChSwitch->AddEntry("Ch3 (BaF 2B)", 3);
    shownChSwitch->AddEntry("Ch4 (BaF 3A)", 4);
    shownChSwitch->AddEntry("Ch5 (BaF 3B)", 5);
    shownChSwitch->AddEntry("Ch6 (BaF 4)", 6);
    shownChSwitch->AddEntry("Ch7 (LySO)", 7);
    shownChSwitch->Select(0);

    TGTextButton *plotCalibrationButton = new TGTextButton(h1frame,"&Show");
    h1frame->AddFrame(plotCalibrationButton, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5,5));
    fMain->AddFrame(h1frame, new TGLayoutHints(kLHintsCenterX, 2,2,2,2));
    plotCalibrationButton->Connect("Clicked()","CalibrationWindow",this,"showCalibration()");

    // Create canvas widget
    TGHorizontalFrame *h2frame = new TGHorizontalFrame(fMain,w,200);
    fEcanvas = new TRootEmbeddedCanvas("Ecanvas",h2frame,w/2,200);

   /* static Int_t wid = fEcanvas->GetCanvasWindowId();
    TCanvas *fCanvas = new TCanvas("MyCanvas", 100,100,wid);
    fEcanvas->AdoptCanvas(fCanvas);*/
    h2frame->AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10));

    textView = new TGTextView (h2frame, w/2,200);
    h2frame->AddFrame(textView,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2,2,2,2));
    fMain->AddFrame(h2frame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2,2,2,2));

    // Create a horizontal frame widget with buttons
    TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain,w,40);
    TGTextButton *calibrationButton = new TGTextButton(hframe,"&Calibrate");
    calibrationButton->Connect("Clicked()","CalibrationWindow",this,"calibrate()");


    hframe->AddFrame(calibrationButton, new TGLayoutHints(kLHintsCenterX,
                                                          5,5,3,4));
    calibEvtNE = new TGNumberEntry(hframe,100,6,-1, (TGNumberFormat::EStyle) 0,
                                   (TGNumberFormat::EAttribute) 2, (TGNumberFormat::ELimit) 1, 10., 1e6);
    hframe->AddFrame(calibEvtNE, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));
    TGTextButton *exit = new TGTextButton(hframe,"&Exit",
                                          "gApplication->Terminate(0)");
    hframe->AddFrame(exit, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));

    // Set a name to the main frame
    fMain->SetWindowName("Calibration tool");

    // Map all subwindows of main frame
    fMain->MapSubwindows();

    // Initialize the layout algorithm
    fMain->Resize(fMain->GetDefaultSize());

    // Map main frame
    fMain->MapWindow();
}
void CalibrationWindow::calibrate() {
    //start calibration

    //construct matrix to return
    vector <vector<eventTime>> timePeaksCh(kNCh, vector<eventTime>());


    Int_t eventsToSum = calibEvtNE->GetIntNumber();

    vector<Int_t> nCalPointsCh(kNCh, 0);
    vector<TH1D> hsumsumCh;
    hsumsumCh.reserve(kNCh);
    for(int i=0; i< kNCh; ++i)
    {
        TH1D tempHist(Form("hsumsum%d",i) , "sum of the sum", kNTimeSamples, 0., (Double_t) kNTimeSamples);
        hsumsumCh.push_back(tempHist);


    }
    nCalPointsCh =  calibration("RunFile.root", eventsToSum, timePeaksCh, hsumsumCh);



    //cout << nCalPointsCh0 << " " << nCalPointsCh1 << endl;

    //save to file
    TTree *calibration_tree = new TTree("calibration_tree", "Calibration Tree");
    TFile *calibration_file = new TFile (CALIBRATIONFILE, "RECREATE");
    vector <vector<Double_t>> peaksTimeCh (kNCh);
    vector <vector<Double_t>> peaksTimeErrorCh (kNCh);
    for (int ch =0 ; ch < kNCh; ch++){
        for(int i=0; i< nCalPointsCh[ch]; i++){
            peaksTimeCh[ch].push_back(timePeaksCh[ch][i].time);
            peaksTimeErrorCh[ch].push_back(timePeaksCh[ch][i].error);
        }

    calibration_tree->Branch(Form("nCalPointsCh%d",ch), &nCalPointsCh[ch], Form("nCalPointsCh%d/I",ch));
    calibration_tree->Branch(Form("timeCh%dtime",ch), &(peaksTimeCh[ch]) ); //Form("timeCh%dtime[nCalPointsCh%d]/D",ch,ch));
    calibration_tree->Branch(Form("timeCh%derror",ch), &(peaksTimeErrorCh[ch]));// Form("timeCh%derror[nCalPointsCh%d]/D",ch,ch));
    calibration_file->WriteTObject(&hsumsumCh[ch]);
    }
    calibration_tree->Fill();
    calibration_file->WriteTObject(calibration_tree);
    calibration_file->Close();
    showCalibration();
}
CalibrationWindow::~CalibrationWindow() {
    // Clean up used widgets: frames, buttons, layout hints
    fMain->Cleanup();
    delete fMain;
}

void CalibrationWindow::showCalibration()
{
    Int_t chIndex = shownChSwitch->GetSelected();
    static TFile f(CALIBRATIONFILE, "READ");
    TH1D *hist = (TH1D *) f.Get(Form("hsumsum%d", chIndex ));
    if(hist==NULL) {cerr << "Calibration histogram not found"; return;}


    TCanvas *fCanvas = fEcanvas->GetCanvas();
    hist->Draw();
    fCanvas->cd();
    fCanvas->Modified();
    fCanvas->Update();

    TTree *calibration_tree = (TTree *) f.Get("calibration_tree");
    Int_t nCalPoints;
    calibration_tree->SetBranchAddress(Form("nCalPointsCh%d",chIndex),&nCalPoints);
    vector<Double_t> *chTime=0;
    vector<Double_t> *chError=0;
    calibration_tree->SetBranchAddress(Form("timeCh%dtime", chIndex), &chTime);
    calibration_tree->SetBranchAddress(Form("timeCh%derror", chIndex), &chError);
    calibration_tree->GetEntry(0);

    textView->Clear();
    textView->AddLine(Form("Channel %d:\n", chIndex));
    textView->AddLine(Form("Found %d peaks\n", nCalPoints));

    textView->AddLine("| IDX\t\t|Time(ns)\t|Error(ns)\t|DeltaT(ns)\t|");
    for(int i=0; i<nCalPoints; ++i){
        double_t time = chTime->at(i) / SAMPLING_RATE;
        double_t error = chError->at(i) / SAMPLING_RATE;
        double_t deltaT = 0.;
        if(i>0) deltaT = time - chTime->at(i-1)/SAMPLING_RATE;
        textView->AddLine(Form("| %2d)\t\t| %6.2f\t\t| %7.2f\t\t| %7.2f\t\t\t|", i , time, error, deltaT));
    }


}

void CalibrationWindow::close()
{

}

vector<Int_t> CalibrationWindow::calibration(TString in_name, int ev_to_sum,
                                             vector<vector<eventTime>> &timeCh, vector<TH1D> &hsumsumCh )
{

    //create data block
    //float **fNRSSdataBlock = SharedLibrary::createMatrix(kNCh, kNTimeSamples);

    float fNRSSdataBlock[kNCh][kNTimeSamples];

    TFile *f =  new TFile(in_name, "READ");
    TTree *tree = (TTree *) f->Get("rawdata");
    tree->SetBranchAddress("dataNRSS", fNRSSdataBlock);


    for(int indexEvent=0; indexEvent< ev_to_sum ; ++indexEvent){
        //retrive the data block
        tree->GetEntry(indexEvent);
        for(int ch = 0; ch < kNCh; ch++)
        {
            TH1D hsignal(Form("hsum%d", ch), "hsum", kNTimeSamples, 0., (Double_t) kNTimeSamples);
            SharedLibrary::copyDataBlock(fNRSSdataBlock, ch, kNTimeSamples, hsignal);

            //search for the position of the trigger and set to zero first histogram bins
            SharedLibrary::shiftToTrigger(hsignal);
            hsumsumCh[ch].Add(&hsignal);
        }
    }

    //set range to avoid probled with TSpectrum
    Int_t cutoff = 875;
    TSpectrum *s = new TSpectrum(2 * EX_PEAKS);
    TH1D *hsumback= new TH1D("hsumback", "hsum less background", kNTimeSamples, 0., (Double_t) kNTimeSamples);
    vector<Int_t> nfoundCh(kNCh,0);
    for(int ch=0; ch < kNCh; ch++)
    {
        hsumsumCh[ch].SetAxisRange(0, cutoff);

        //Use TSpectrum to find the peak candidates
        s->Clear();
        nfoundCh[ch] = s->Search(&hsumsumCh[ch], 4,"nodraw",0.30);
        //printf("Found %d candidate peaks to fit\n",nfoundCh[ch]);
        //Estimate background using TSpectrum::Background

        TH1 *hb = s->Background(&hsumsumCh[ch],20,"same");
        //if (hb) c1->Update();
        //if (np <0) return;
        hsumback->Reset();
        hsumback->Add(&hsumsumCh[ch]); //sottrazione del background
        hsumback->Add(hb,-1);

        TH1D *hsumback2[nfoundCh[ch]];
        //estimate linear background using a fitting method
        //c1->cd(2);
        Double_t par[5];

        Double_t *xpeaks = s->GetPositionX();

        sort(xpeaks, xpeaks + nfoundCh[ch]);

        //printf("Found %d useful peaks to fit\n",nfoundCh[ch]);
        //printf("Now fitting: Be patient\n");
        //TCanvas *canv = new TCanvas("canv", "Titolo", 3200, 2400);
        //hsumsum->Draw();
        //hb->Draw("same");
        for (Int_t p=0;p<nfoundCh[ch];p++) {
            hsumback2[p] = (TH1D*)hsumback->Clone(Form("hsumback%i" , p));
            Double_t xp = xpeaks[p];
            Int_t bin = hsumsumCh[ch].GetXaxis()->FindBin(xp);
            Double_t yp = hsumback->GetBinContent(bin);
            //printf("picco %f %f \n", xp, yp);
            //if (yp-TMath::Sqrt(yp) < fline->Eval(xp)) continue;
            par[0] = yp;    //norm  Parameters of the crystalball function
            par[1] = -0.5;  //alpha
            par[2] = 2;     //n
            par[3] = 3;     //sigma
            par[4] = xp;    //x0


            TF1 fit("fit",fpeaks,xp-par[3],xp+par[3], 5);

            //we may have more than the default 25 parameters
            //TVirtualFitter::Fitter(hsumback2[p],5);     //non sappiamo a che serve :-)
            fit.SetParameters(par);
            fit.SetNpx(1024);
            hsumback2[p]->Fit("fit", "MQ", "same", xp-1.5*par[3],xp+2*par[3]);
            eventTime peak;
            peak.time = fit.GetParameter(4);
            peak.error = fit.GetParError(4);
            timeCh[ch].push_back(peak);

            // hsumback2[p]->Draw("same");
        }

    }
    // canv->SaveAs("canv.png");
    f->Close();
    /*delete s;
     delete hsumback;
    for(int i=0; i< nfoundCh[ch]; ++i ){
        delete hsumback2[i];
    }*/

    return nfoundCh;  //return the number of peaks found

}

Double_t CalibrationWindow::fpeaks(Double_t *x, Double_t *par) {

    Double_t norm  = par[0];
    Double_t alpha  = par[1];
    Double_t n = par[2];
    Double_t sigma = par[3];
    Double_t x0 = par[4];
    Double_t result = norm*ROOT::Math::crystalball_function(x[0], alpha, n, sigma, x0);

    return result;
}
