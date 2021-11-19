#ifndef __CINT__
//put LCIO includes here
#include <lcio.h>
#include <IO/LCReader.h>
#include <IOIMPL/LCFactory.h>
#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>
#include <EVENT/ReconstructedParticle.h>
#include <UTIL/LCIterator.h>
#include <UTIL/LCRelationNavigator.h>
#endif

// put ROOT includes here
#include <TH2F.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TFile.h>

#include <numeric>

using namespace lcio;

void calcConfusion(const char *fileName = "/afs/desy.de/group/flc/pool/reichenl/myMarlinProcessors/SLDLeptonSelector/test_out.slcio", const TString outname = "confusion_test")
{
    const char *mcColName = "MCSLDLeptons";
    const char *relColname = "SLDMCRecoLink";

    std::map<int, Int_t> rmap = {
        {11, 0},
        {-11, 1},
        {13, 2},
        {-13, 3},
        {-211, 4},
        {211, 5},
        {22, 6},
        {0, 7}
    };

    Int_t n_xbins = rmap.size();
    Int_t n_ybins = 4; // e, eb, mu, mub
    auto xbins = std::vector<Float_t>(n_xbins + 1);
    std::iota(xbins.begin(), xbins.end(), 0.0);
    auto ybins = std::vector<Float_t>(n_ybins + 1);
    std::iota(ybins.begin(), ybins.end(), 0.0);
    auto h = new TH2F("confusion", ";Reco; MC", n_xbins, xbins.data(), n_ybins, ybins.data());
    h->SetStats(0);

    auto lcReader = LCFactory::getInstance()->createLCReader();
    lcReader->open(fileName);

    while (const auto evt = lcReader->readNextEvent()) {
        auto rel = evt->getCollection(relColname);
        auto nav = std::make_unique<LCRelationNavigator>(rel);

        //iterate over mc sld leptons
        LCIterator<MCParticle> mcp_iter(evt, mcColName);
        while (const auto mcp = mcp_iter.next()) {

            //get related rec pfo
            auto reco = dynamic_cast<ReconstructedParticle *>(nav->getRelatedToObjects(mcp)[0]);

            int recPDG = 0;
            if (reco != NULL) {
                recPDG = reco->getType();
            };
            int mcPDG = mcp->getPDG();

            // std::cout << "[mc, rec]: [" << mcPDG << ", " << recPDG << "]" << std::endl;

            h->Fill(rmap[recPDG], rmap[mcPDG]);

        }
    }
    lcReader->close();

    gStyle->SetPalette(112);

    auto c1 = new TCanvas("confc", "foo", 800, 400);

    // TODO: set labels
    auto xaxis = h->GetXaxis();
    auto yaxis = h->GetYaxis();
    xaxis->SetBinLabel(1, "11");
    xaxis->SetBinLabel(2, "-11");
    xaxis->SetBinLabel(3, "13");
    xaxis->SetBinLabel(4, "-13");
    xaxis->SetBinLabel(5, "-211");
    xaxis->SetBinLabel(6, "211");
    xaxis->SetBinLabel(7, "22");
    xaxis->SetBinLabel(8, "none");
    yaxis->SetBinLabel(1, "11");
    yaxis->SetBinLabel(2, "-11");
    yaxis->SetBinLabel(3, "13");
    yaxis->SetBinLabel(4, "-13");
    h->SetMarkerSize(4);
    h->SetMarkerColor(0);
    c1->cd();
    h->Draw("COLZ TEXT");

    c1->SaveAs(outname + ".pdf");

    auto outfile = TFile(outname + ".root", "RECREATE");
    h->Write();
    outfile.Close();
}