#include <TH2F.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TBox.h>
#include <iostream>
#include <memory>

void plotOverview(const TString ifname = "sld_out.root", const TString ofname = "overview", enum EColorPalette palette = kAvocado)
{
    std::unique_ptr<TFile> f(TFile::Open(ifname));
    
    std::unique_ptr<TH2F> mupip(f->Get<TH2F>("mupip"));
    std::unique_ptr<TH2F> mumup(f->Get<TH2F>("mumup"));
    std::unique_ptr<TH2F> mup_eff(f->Get<TH2F>("mup_eff"));
    
    auto c1 = std::make_unique<TCanvas>("mupi");
    //auto c1 = new TCanvas("mupi");
    c1->Divide(3,2);

    c1->cd(1)->SetLogy();
    mupip->SetStats(0);
    mupip->SetTitle("#mu^{#pm} detected as #pi^{#pm}; cos(#theta); Momentum (GeV)   ");
    mupip->Draw("COLZ");

    c1->cd(2)->SetLogy();
    auto mup_diff = (TH2F*) mumup->Clone();
    //*mup_diff = *mup_diff - *mupip;
    mup_diff->SetStats(0);
    mup_diff->SetTitle("#mu^{#pm} correct; cos(#theta); Momentum (GeV)   ");
    mup_diff->Draw("COLZ");
    
    c1->cd(3)->SetLogy();
    auto mup_sum = (TH2F*) mumup->Clone();
    *mup_sum = *mup_sum + *mupip;
    mup_sum->SetStats(0);
    mup_sum->SetTitle("#mu^{#pm} correct + #pi; cos(#theta); Momentum (GeV)   ");
    mup_sum->Draw("COLZ");
    
    c1->cd(4)->SetLogy();
    mup_eff->SetStats(0);
    mup_eff->SetTitle("#mu^{#pm} eff; cos(#theta); Momentum (GeV)   ");
    mup_eff->Draw("COLZ");

    // create 1
    auto one = (TH2F*) mumup->Clone();
    for (int i = 0; i < one->GetNcells(); i++) {
        one->SetBinContent(i, 1.0);
    }

    c1->cd(5)->SetLogy();
    auto mup_ieff = (TH2F *) one->Clone();
    *mup_ieff = *mup_ieff - *mup_eff;
    mup_ieff->SetStats(0);
    mup_ieff->SetTitle("#mu^{#pm} 1 - eff; cos(#theta); Momentum (GeV)   ");
    mup_ieff->Draw("COLZ");
    
    gStyle->SetPalette(palette);
    
    c1->SaveAs(ofname + std::to_string(palette) + ".pdf");
}