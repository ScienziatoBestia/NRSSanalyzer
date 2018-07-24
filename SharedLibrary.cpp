#include "SharedLibrary.h"




Double_t SharedLibrary::zero_position(TH1D &h, Int_t polarity){

        int idx;
        if(polarity > 0)
                idx = h.GetMaximumBin();
        else
                idx = h.GetMinimumBin();

        while(h.GetBinContent(idx)/h.GetBinContent(idx+1) >0) idx++;

        int x0 = idx;
        int x1 = idx+1;

        Double_t y0 = h.GetBinContent(idx);
        Double_t y1 = h.GetBinContent(idx+1);

        Double_t zeropos = x0 -y0*((x1-x0)/(y1-y0));

        return zeropos;
}


TH1D* SharedLibrary::subZero(TH1D &h, Int_t baseline, TH1D * hres){

        Double_t base = 0;
        for(int i = 0; i<baseline; i++)
                base += h.GetBinContent(i+1);
        base = base/(Double_t)baseline;


        for(int i = 0; i<1024; i++){
                        Double_t value = h.GetBinContent(i+1)-base;
                        hres->SetBinContent(i+1,value);
                }

        return hres;
}


Double_t SharedLibrary::CFDtime(TH1D &h0, Int_t nch, Int_t baseline, Double_t fraction, Int_t nshift, Int_t polarity){  //pol = +/-1

        TH1D h1("h1","h1",nshift+nch,0,nshift+nch);
        TH1D h2("h2","h2",nshift+nch,0,nshift+nch);
        TH1D h("hres","hres",1024,0,1024);
        subZero(h0, baseline, &h);

//Fill h1
        for(int i = 0; i<nch; i++){
                h1.SetBinContent(i+1, fraction*h.GetBinContent(i+1));
        }
        for(int i = nch; i<nshift+nch; i++){
                h1.SetBinContent(i+1, fraction*h.GetBinContent(nch+1));
        }
//Fill h2
        for(int i = 0; i<nshift; i++){
                h2.SetBinContent(i+1, 0);
        }
        for(int i = nshift; i<nshift+nch; i++){
                h2.SetBinContent(i+1, h.GetBinContent(i-nshift+1));
        }

        h1.Add(&h2,-1);




       // h2->Draw();
        //h1->Draw("same");


        Double_t zeropos = zero_position(h1, polarity);

        /*TLine *l = new TLine(zeropos,h1->GetMinimum(),zeropos,h1->GetMaximum());
        l->Draw("same");*/

        //cout<<"CFD time = "<<zeropos<<endl;

        return zeropos;
}


float **SharedLibrary::createMatrix(Int_t rows, Int_t columns)
{
    float ** matrix = new float*[rows];
    for(int i = 0; i < rows; i++)    {
        matrix[i] = new float[columns];
    }
    return matrix;
}

void SharedLibrary::copyDataBlock(float  dataBlock[][kNTimeSamples], int row, int rowSize, TH1D &container)
{
    for(int i=0; i<rowSize; ++i){
       container.SetBinContent(i+1, dataBlock[row][i]);
    }
}


void SharedLibrary::shiftToTrigger(TH1D &histSignal)
{
    Double_t triggerTime = SharedLibrary::CFDtime(histSignal, 100, 20, 0.5, 8, -1);
    Int_t triggerBin = (Int_t) histSignal.FindBin(triggerTime) + 10;
    for (int i=triggerBin; i < kNTimeSamples; i++){
        histSignal.SetBinContent(i-triggerBin +1, histSignal.GetBinContent(i+1));
    }
    for(int i=kNTimeSamples-triggerBin; i< kNTimeSamples; ++i){
        histSignal.SetBinContent(i+1, 0.);
    }
}

