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
#include <TVector3.h>
#include <TStyle.h>
#include <TFile.h>


using namespace lcio;

void calcMuEff(const char *fileName = "/afs/desy.de/group/flc/pool/reichenl/myMarlinProcessors/SLDLeptonSelector/test_out.slcio", const TString outname = "sld_out", Int_t cosbins = 10, Int_t pbins = 10, Bool_t abscost = false)
{
    const char *mcColName = "MCSLDLeptons";
    const char *relColname = "SLDMCRecoLink";

    double min_exp = -1;
    double max_exp = TMath::Log10(50);
    double width = (max_exp - min_exp) / pbins;

    std::vector<Double_t> logbins(pbins + 1);

    for (int i = 0; i <= pbins; i++) {
        logbins[i] = TMath::Power(10, min_exp + i * width);
    }
    
    Double_t costmin = abscost ? 0 : -1;

    auto mumup = new TH2F("mumup", "", cosbins, costmin, 1, logbins.size() - 1, logbins.data());
    auto mupip = new TH2F("mupip", "", cosbins, costmin, 1, logbins.size() - 1, logbins.data());
    auto muop = new TH2F("muop", "", cosbins, costmin, 1, logbins.size() - 1, logbins.data());

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

            TVector3 P(mcp->getMomentum());
            Double_t cos_th = abscost ? fabs(P.CosTheta()) : P.CosTheta();

            switch (abs(mcPDG))
            {
            case 13:
                switch (abs(recPDG))
                {
                case 13:
                    mumup->Fill(cos_th, P.Mag());
                    break;

                case 211:
                    mupip->Fill(cos_th, P.Mag());
                    break;

                case 0:
                    muop->Fill(cos_th, P.Mag());
                    break;

                default:
                    break;
                }
                break;

            default:
                break;
            }
        }
    }
    lcReader->close();

    auto mup_all = (TH2F*) muop->Clone();
    mup_all->Add(mupip);
    mup_all->Add(mumup);
    auto mup_eff = (TH2F*) mumup->Clone();
    mup_eff->Divide(mup_all);
    mup_eff->SetStats(0);
    mup_eff->SetTitle("#mu; cos(#theta);Momentum (GeV)");

    auto outfile = TFile(outname + ".root", "RECREATE");
    mup_all->SetName("mup_all");
    mup_eff->SetName("mup_eff");

    mumup->Write();
    mupip->Write();
    muop->Write();
    mup_all->Write();
    mup_eff->Write();

    outfile.Close();
}