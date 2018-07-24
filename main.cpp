#include <TF1.h>
#include <TH1D.h>
#include <TFile.h>
#include <iostream>
#include <Math/Math.h>
#include <TSpectrum.h>
#include <TVirtualFitter.h>
#include <Math/PdfFuncMathCore.h>
#include <TCanvas.h>
#include <TTree.h>
#include <TLine.h>
#include <TRandom3.h>
#include <TApplication.h>
#include <TGClient.h>
#include "mainwindow.h"
#include "calibrationwindow.h"
#include <TStyle.h>

#include "constants.h"



using namespace std;


Int_t findPeaks(TH1D* h, TH1D * hback, Int_t nPeaksCh, Double_t* xpeaks, Double_t * timeChtime );
Int_t findInTimePeaks(Double_t * xPeaks, TH1D *hback, Double_t * calibrationTimes, Int_t nCalPoints,
                      std::pair<Double_t, Double_t> * inTimePeaks);
Int_t addTrigger(TH1D * hsum, Int_t trig_type);

void screma(TString fin, TString fout, int n_ev){
    TFile *f0 = new TFile(fin,"READ");
    TFile *f1 = new TFile(fout,"CREATE");

    int j = 0;
    for (int i = 0; i < n_ev; i++){
        TString name;
        name.Form("ev_%d_ch_0",i);
        TH1D *h = (TH1D*)f0->Get(name);
        if(h->GetMinimum()<=-0.3){
            TString name2;
            name2.Form("ev_%d_ch_0",j);
            TH1D *h2 = new TH1D(name2, name2, 1024,0,1024);
            for (int k = 1; k < 1024; k++) {
                h2->SetBinContent(k,h->GetBinContent(k));
            }
            f1->WriteTObject(h2);
            j++;
        }
    }
    return;
}


double inverse_hist(TH1D *h, int base_ch){

    //caluculate baseline as average
    double baseline = 0;
    for(int i = 1; i<base_ch; i++) baseline = baseline + h->GetBinContent(i);
    baseline = baseline/base_ch;

    //cout<<"Baseline = "<<baseline<<endl;
    for(int k = 0; k<1024; k++){
        h->SetBinContent(k+1,baseline-h->GetBinContent(k+1));
    }

    //also normalize peak to 0.4
    /*double n_factor = 0.4/h->GetMaximum();
for(int k = 0; k<1024; k++){
  h->SetBinContent(k+1,n_factor*h->GetBinContent(k+1));
}
*/
    return baseline;
}

TH1D *shift_hist(TH1D *h, int ch_shift, TString name){

    TH1D *hs = new TH1D(name,name,1024,0,1024);

    if(ch_shift>=0){
        for(int k = 1; k<=ch_shift; k++) hs->SetBinContent(k,0);
        for(int k = ch_shift+1; k<1024; k++) hs->SetBinContent(k,h->GetBinContent(k-ch_shift));
    }
    else{
        for(int k = 1; k<=1023 + ch_shift; k++) hs->SetBinContent(k,h->GetBinContent(k-ch_shift));
        for(int k = 1024 + ch_shift; k<1024; k++) hs->SetBinContent(k,0);
    }

    return hs;
}

TH1D *make_event(TFile *f, int seq, int start_ev, int stop_ev, int base_ch, int ch_shift, TFile *ftemp, int chNumber){

    TString sname;
    sname.Form("hsum_%d",seq);

    TH1D *hsum = new TH1D(sname,sname,1024,0,1024);
    Int_t baseShift = addTrigger(hsum, 0);

    for (int i = start_ev; i < stop_ev; i++) {
        TString iname, iname2;
        iname.Form("ev_%d_ch_%d",i+start_ev, chNumber);
        iname2.Form("histSh_%d",i);
        TH1D *h = (TH1D *)f->Get(iname);
        double base = inverse_hist(h,base_ch);
        int max_pos = h->GetMaximumBin();
        int ch_shift_tot = baseShift + 30 - max_pos + (i-start_ev)*ch_shift;
        TH1D *hs = shift_hist(h,ch_shift_tot,iname2);
        ftemp->WriteTObject(hs);
        hsum->Add(hs);
    }

    return hsum;
}


Double_t trig_funct(Int_t ch, Int_t trig_type, Int_t nch_rise ){
    Double_t value;
    if(trig_type == 0)						//squarewave
        value = 0.8 - (0.8/nch_rise)*ch;
    else{
        //prelevare la forma da sample registrato (da implementare)
    }

return value;
}


Int_t addTrigger(TH1D * hsum, Int_t trig_type = 0){
Double_t risetime = 5.;
Double_t glitch = 10;   				//glitch in ns
Double_t nch_rise = (Int_t)SAMPLING_RATE*risetime;
Double_t nch_glitch = (Int_t)glitch*SAMPLING_RATE;

TRandom3 random(0);
Int_t base = 50 + random.Uniform(-nch_glitch/2, nch_glitch/2 );

for(int i = 0; i<base; i++)
    hsum->SetBinContent(i+1,0.8);
for(int i = base; i<base + nch_rise; i++)
    hsum->SetBinContent(i+1,trig_funct(i+1-base, trig_type, nch_rise));

return base + nch_rise;
}



Double_t fpeaks(Double_t *x, Double_t *par) {

    Double_t norm  = par[0];
    Double_t alpha  = par[1];
    Double_t n = par[2];
    Double_t sigma = par[3];
    Double_t x0 = par[4];
    Double_t result = norm*ROOT::Math::crystalball_function(x[0], alpha, n, sigma, x0);

    return result;
}

void make_sample(TString in_name, int base_ch, int ns_shift, int tot_ev, int ev_per_sum,
                 int chNumber){

    TFile *f = new TFile(in_name,"READ");
    TString outname = "ShSample_";
    outname.Append(in_name);
    TFile *fout = new TFile(outname,"RECREATE");

    TFile *ftemp = new TFile("ftemp.root","RECREATE");

    int ch_shift = (int)(ns_shift*SAMPLING_RATE);

    if(ev_per_sum >= (1024-2*base_ch)){
        cout<<"NB: resampling!"<<endl;
        ev_per_sum = (1024-2*base_ch);
    }

    int n_final = (int)tot_ev/ev_per_sum;
    cout<<n_final<<" events will be created"<<endl;


    for(int i = 0; i<n_final; i++){
        // cout<<"====MAKING EV. "<<i<<endl;
        TH1D *hsum = make_event(f,i,i*ev_per_sum,(i+1)*ev_per_sum,base_ch,ch_shift, ftemp, chNumber);
        fout->WriteTObject(hsum);
        delete hsum;
    }

    fout->Close();
    ftemp->Close();
    f->Close();
    return;
}

int main(int argc, char* argv[])
{

    TApplication theApp("App",&argc,argv);
    gStyle->SetOptStat("");

    new MainWindow(gClient->GetRoot(),600,400);

    theApp.Run();

    /*
    int myc = 0; bool sampleflag = false;
    bool calibrationflag = false;
    int eventi_da_sommare=0;
    //lettura riga di comando
    while ((myc = getopt (argc, argv, "sc:")) != -1) {
        switch (myc)
        {
        case 's':   //attiva creazione sample
            sampleflag = true;
            break;

        case 'c':   //attiva calibrazione
            calibrationflag = true;
            eventi_da_sommare = atoi(optarg);
            break;
        case '?': cout << "usage is \n [-s] : for enabling sample creation \n [-c value]: for enabling calibration " << endl;
            break;
        }
    }




    //prima fase: creazione dei sample simili a quelli che immaginiamo di ottenere dal fascio
    if(sampleflag){
        cout << "Creating sample..." << endl;
        TString nomeFile1, nomeFile2;
        nomeFile1= "OutRun1";
        nomeFile1.Append("_ch0.root");
        nomeFile2= "OutRun1";
        nomeFile2.Append("_ch1.root");
        int base_ch = 20;
        int ns_shift = 16;
        int tot_ev = 1600;
        int ev_per_sum = 16;
        make_sample(nomeFile1, base_ch, ns_shift, tot_ev, ev_per_sum, 0);
        make_sample(nomeFile2, base_ch, ns_shift, tot_ev, ev_per_sum, 1);
    }






    //iniziamo la fase di calibrazione
    eventTime *timeCh0, *timeCh1;
    timeCh0= new eventTime[50];     //costruisce l'array da restituire
    timeCh1= new eventTime[50];     //costruisce l'array da restituire

    if(calibrationflag){
        Int_t nCalPointsCh0, nCalPointsCh1;
        TH1D *hsumsum0 = new TH1D("hsumsum0", "sum of the sum", 1024, 0, 1024);
        TH1D *hsumsum1 = new TH1D("hsumsum1", "sum of the sum", 1024, 0, 1024);
        nCalPointsCh0 =  calibrazione("ShSample_OutRun1_ch0.root", eventi_da_sommare, timeCh0, hsumsum0);
        nCalPointsCh1 =  calibrazione("ShSample_OutRun1_ch1.root", eventi_da_sommare, timeCh1, hsumsum1);
        cout << nCalPointsCh0 << " " << nCalPointsCh1 << endl;

        //salvataggio su file
        Double_t *timeCh0time = new Double_t[nCalPointsCh0];
        Double_t *timeCh0error = new Double_t[nCalPointsCh0];
        for(int i=0; i< nCalPointsCh0; i++){
            timeCh0time[i] = timeCh0[i].time;
            timeCh0error[i] = timeCh0[i].error;
        }
        Double_t *timeCh1time = new Double_t[nCalPointsCh1];
        Double_t *timeCh1error = new Double_t[nCalPointsCh1];
        for(int i=0; i< nCalPointsCh1; i++){
            timeCh1time[i] = timeCh1[i].time;
            timeCh1error[i] = timeCh1[i].error;
        }
        TTree *calibration_tree = new TTree("calibration_tree", "Calibration Tree");
        calibration_tree->Branch("nCalPointsCh0", &nCalPointsCh0, "nCalPointsCh0/I");
        calibration_tree->Branch("nCalPointsCh1", &nCalPointsCh1, "nCalPointsCh1/I");
        calibration_tree->Branch("timeCh0time", timeCh0time, "timeCh0time[nCalPointsCh0]/D");
        calibration_tree->Branch("timeCh0error", timeCh0error, "timeCh0error[nCalPointsCh0]/D");
        calibration_tree->Branch("timeCh1time", timeCh1time, "timeCh1time[nCalPointsCh1]/D");
        calibration_tree->Branch("timeCh1error", timeCh1error, "timeCh1error[nCalPointsCh1]/D");
        calibration_tree->Fill();
        TFile *calibration_file = new TFile (CALIBRATIONFILE, "RECREATE");
        calibration_file->WriteTObject(calibration_tree);
        calibration_file->WriteTObject(hsumsum0);
        calibration_file->WriteTObject(hsumsum1);
        calibration_file->Close();

    }


    //iniziamo la fase di analisi
    //leggiamo il file della calibrazione
    TFile *calibration_file = new TFile (CALIBRATIONFILE, "READ");
    TTree *calibration_tree = (TTree *) calibration_file->Get("calibration_tree");
    Int_t nCalPointsCh0, nCalPointsCh1;
    calibration_tree->SetBranchAddress("nCalPointsCh0",&nCalPointsCh0);
    calibration_tree->SetBranchAddress("nCalPointsCh1",&nCalPointsCh1);
    calibration_tree->GetEntry(0);
    Double_t *timeCh0time = new Double_t[nCalPointsCh0];
    Double_t *timeCh0error = new Double_t[nCalPointsCh0];
    Double_t *timeCh1time = new Double_t[nCalPointsCh1];
    Double_t *timeCh1error = new Double_t[nCalPointsCh1];
    calibration_tree->SetBranchAddress("timeCh0time", timeCh0time);
    calibration_tree->SetBranchAddress("timeCh0error", timeCh0error);
    calibration_tree->SetBranchAddress("timeCh1time", timeCh1time);
    calibration_tree->SetBranchAddress("timeCh1error", timeCh1error);
    calibration_tree->GetEntry(0);

    TFile *f0 = new TFile("ShSample_OutRun1_ch0.root","READ");
    TFile *f1 = new TFile("ShSample_OutRun1_ch1.root","READ");

    TH1D *h0 = (TH1D *)f0->Get("hsum_0");
    h0->SetName("h0");
    TH1D *h1 = (TH1D *)f1->Get("hsum_0");
    h1->SetName("h1");

    //search for the trigger time
    Double_t triggerTimeCh0 = CFDtime(h0, 100, 20, 0.5, 8, -1);
    Double_t triggerTimeCh1 = CFDtime(h1, 100, 20, 0.5, 8, -1);

    //shift of the calibration times
    for(int i = 0; i < nCalPointsCh0; i++){
        timeCh0time[i] += triggerTimeCh0 + 10;
    }
    for(int i = 0; i < nCalPointsCh1; i++){
        timeCh1time[i] += triggerTimeCh1 + 10;
    }
    Double_t * xPeaksCh0 = new double_t[3*nCalPointsCh0];
    Double_t * xPeaksCh1 = new double_t[3*nCalPointsCh1];
    TH1D *hback0= new TH1D("hback0", "h0 less background", 1024, 0, 1024);
    TH1D *hback1= new TH1D("hback1", "h1 less background", 1024, 0, 1024);
    Int_t nfound0 = findPeaks(h0, hback0, nCalPointsCh0, xPeaksCh0, timeCh0time );
    Int_t nfound1 = findPeaks(h1, hback1, nCalPointsCh1, xPeaksCh1, timeCh1time );


    std::pair<Double_t, Double_t> *inTimePeaksCh0 = new std::pair<Double_t, Double_t>[nCalPointsCh0];
    std::pair<Double_t, Double_t> *inTimePeaksCh1 = new std::pair<Double_t, Double_t>[nCalPointsCh1];
    Int_t nInTimePeaksCh0= findInTimePeaks(xPeaksCh0, hback0, timeCh0time, nCalPointsCh0, inTimePeaksCh0 );
    Int_t nInTimePeaksCh1= findInTimePeaks(xPeaksCh1, hback1, timeCh1time, nCalPointsCh1, inTimePeaksCh1 );

    //trovare le coincidenze tra i due canali
    //per il momento ci limitiamo a controllare che vi sia coincidenza senza valutare
    //cooerenza energetica tra i due canali
    Int_t goodCounts = 0;
    Double_t ch0Thres = 0.;
    Double_t ch1Thres = 0.;
    for(int i=0; i< nCalPointsCh0; ++i){
        if(inTimePeaksCh0[i].second > ch0Thres &&
           inTimePeaksCh1[i].second > ch1Thres){
            goodCounts++;
        }
    }
    cout << "Trovati " << goodCounts << " conteggi validi" << endl;
*/
    return 0;
}

Int_t findPeaks(TH1D* h, TH1D * hback, Int_t nPeaksCh, Double_t * xpeaks, Double_t * timeChtime )
{
    h->SetAxisRange(timeChtime[0]-20, timeChtime[nPeaksCh-1]+20);
    //Use TSpectrum to find the peak candidates
    TSpectrum *s = new TSpectrum(3 * nPeaksCh);
    Int_t nfound = s->Search(h, 2,"nodraw",0.10);

    //Estimate background using TSpectrum::Background
    TH1 *hb = s->Background(h,20,"same");
    hback->Add(h); //sottrazione del background
    hback->Add(hb,-1);
    //TH1D *hsumback2[nfound];
    //prepara gli array dei valori da restituire
    for(int i=0; i< nfound; i++) xpeaks[i] = s->GetPositionX()[i];

    sort(xpeaks, xpeaks + nfound);
    TCanvas *canv = new TCanvas("canv", "Titolo", 3200, 2400);
    h->Draw();
    hb->Draw("same");
    // hsumback->Draw("same");
    for(int i=0; i< nPeaksCh; i++)
    {
        TLine *timeLine = new TLine(timeChtime[i], h->GetMinimum(), timeChtime[i], h->GetMaximum());
        timeLine->SetLineColor(12);
        timeLine->SetLineWidth(2);
        timeLine->SetLineStyle(7);
        timeLine->Draw("same");
    }
    TString nomefile = h->GetName();
    nomefile.Append(".png");
    canv->SaveAs(nomefile);

    delete s;
    delete hb;
    delete canv;

    return nfound;

}



Int_t findInTimePeaks(Double_t * xPeaks, TH1D * hback, Double_t * calibrationTimes, Int_t nCalPoints,
                      std::pair<Double_t, Double_t> * inTimePeaks)
{

    Double_t timeWidth = 2.0;   //nanosecondi
    Double_t binWidth = timeWidth * SAMPLING_RATE;
    Int_t piccotrovato=0;
    Int_t nInTimePeaks = 0;

    Int_t doppiPicchi = 0;  //doppi picchi di un canale all'interno della banda temporale
    Int_t picchiNonTrovati = 0;
    Int_t indicePicco = 0;
    int p=0;
    for(int i = 0; i<nCalPoints; ++i)
    {
        inTimePeaks[i].first = 0.;
        inTimePeaks[i].second = 0.;

        while(xPeaks[p] < calibrationTimes[i]){
            p++;
        }
        if(fabs(calibrationTimes[i]-xPeaks[p])< binWidth/2) {
            piccotrovato++;
            indicePicco = p;
        }
        if(p!=0 && fabs(calibrationTimes[i]-xPeaks[p-1])< binWidth/2) {
            piccotrovato++;
            indicePicco = p-1;
        }
        if(piccotrovato){
            if(piccotrovato == 2){
                doppiPicchi++;
                cout << "ATTENZIONE... rivelato un doppio picco" << endl;
            }
            else{
                nInTimePeaks++;
                cout << "Trovato picco in coincidenza con la calibrazione" <<  endl;
                inTimePeaks[i].first = xPeaks[indicePicco];
                inTimePeaks[i].second = hback->GetBinContent(hback->FindBin(xPeaks[indicePicco]));
            }
            piccotrovato = 0;
        }
        else{
            picchiNonTrovati++;
            cout << "ATTENZIONE... picco non trovato in coincidenza con il file di calibrazione" << endl;
            cout << fabs(calibrationTimes[i]-xPeaks[p-1]) << "\t" << fabs(calibrationTimes[i]-xPeaks[p]) << endl;
            cout << "i="  << i << "\tp=" << p << endl;
        }
    }
    return nInTimePeaks;
}
