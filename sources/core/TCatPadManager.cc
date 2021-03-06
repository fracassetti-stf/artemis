/* $Id:$ */
/**
 * @file   TCatPadManager.cc
 * @date   Created : Feb 06, 2012 19:06:29 JST
 *   Last Modified : 2019-11-15 17:42:07 JST (ota)
 * @author Shinsuke OTA <ota@cns.s.u-tokyo.ac.jp>
 *  
 *  
 *    Copyright (C)2012
 */
#include "TCatPadManager.h"
#include <TPaveLabel.h>
#include <TLatex.h>
#include <TList.h>
#include <TDatime.h>
#include <TFolder.h>
#include <TROOT.h>
#include <TRunInfo.h>
#include <TAnalysisInfo.h>
#include <TArtemisUtil.h>
#include <TSystem.h>

TCatPadManager::TCatPadManager()
   : fCanvas(NULL), fMainPad(NULL), fCurrentPadId(0), fNumSubPads(0), fTitleLabel(NULL), fDateLabel(NULL)
{
   //CreateCanvas();
}
TCatPadManager::~TCatPadManager()
{
   if (fTitleLabel) delete fTitleLabel;
   if (fDateLabel) delete fDateLabel;
   
}

TCatPadManager* TCatPadManager::Instance()
{
   static TCatPadManager instance;
   return &instance;
}

void TCatPadManager::CreateCanvas()
{
   TDatime now;
   TString name = "";
   if (!fCanvas) {
      fCanvas = new TCanvas("artcanvas","canvas",800,800);
      fCanvas->Connect("Closed()","TCatPadManager",this,"Closed()");
   }
   fCanvas->Clear();
   fCanvas->cd();
   if (!fTitleLabel) {
      fTitleLabel = new TPaveLabel(0.05,0.96,0.95,0.99,"");
   }
   
   if (!fDateLabel) {
      fDateLabel  = new TPaveLabel(0.5,0.92,0.95,0.95,"");
      fDateLabel->SetBorderSize(0);
      fDateLabel->SetFillStyle(0);
   }
   if (!fCommentLabel) {
      fCommentLabel = new TPaveLabel(0.08,0.92,0.5,0.95,"");
      fCommentLabel->SetBorderSize(0);
      fCommentLabel->SetFillStyle(0);
      
   }
   fDateLabel->Draw();
   fTitleLabel->Draw();
   fCommentLabel->Draw();
   
   fMainPad = new TPad("graphs","graphs",0.05,0.01,0.95,0.91);
   fMainPad->Draw();
   fMainPad->cd();
   fMainPad->SetGridy(kTRUE);
   fMainPad->SetGridx(kTRUE);
   

   fMainPad->Connect("Closed()","TCatPadManager",this,"MainPadClosed()");
   fCurrentPadId = 0;
   fNumSubPads = 0;
}

void TCatPadManager::SetTitle(const char* title)
{
   Instance()->GetTitleLabel()->SetLabel(title);
   Instance()->GetCanvas();
}
void TCatPadManager::SetComment(const char* comment)
{
   Instance()->GetCommentLabel()->SetLabel(comment);
   Instance()->GetCanvas();
}

Int_t TCatPadManager::GetNumChild()
{
   return fNumSubPads;
}

Bool_t TCatPadManager::HasChild()
{
   return GetNumChild() ? kTRUE : kFALSE;
}

TVirtualPad *TCatPadManager::Next()
{
   GetCanvas();
   if (!HasChild()) {
      fCurrentPadId = 0;
      return fMainPad->cd(0);
   } else if (fCurrentPadId + 1 <= GetNumChild()) {
      return fMainPad->cd(++fCurrentPadId);
   } else {
      return fMainPad->cd((fCurrentPadId=1));
   }
}

TVirtualPad *TCatPadManager::Previous()
{
   GetCanvas();
   if (!HasChild()) {
      fCurrentPadId = 0;
      return fMainPad->cd(0);
   } else if (fCurrentPadId <= 1) {
      // fCurrentPadId is 0 or 1
      fMainPad->cd((fCurrentPadId=GetNumChild()));
   } else {
      return fMainPad->cd(--fCurrentPadId);
   }
}

TVirtualPad *TCatPadManager::Current()
{
   GetCanvas();
   return fMainPad->cd(fCurrentPadId);
}

TVirtualPad* TCatPadManager::Get(Int_t idx)
{
   GetCanvas();
   return fMainPad->GetPad(idx);
}

TVirtualPad *TCatPadManager::GetCanvas() 
{
   if (!fMainPad) {
      if (fCanvas) {
         fCanvas->Close();
         fCanvas = 0;
      }
      CreateCanvas();      
   }
   TDatime now;
   TFolder *folder = (TFolder*) gROOT->FindObject("/artemis/loops/loop0");
   TFolder *topfolder = (TFolder*) gROOT->FindObject("/artemis");
   art::Util::LoadAnalysisInformation();
   art::TAnalysisInfo *info = (art::TAnalysisInfo*) topfolder->FindObject(art::TAnalysisInfo::kDefaultAnalysInfoName);
   if (info) {
      TString header;
      header.Append(gSystem->BaseName(info->GetSteeringFileName()));
      header.Append("  ");
      header.Append(info->GetRunName());
      header.Append(info->GetRunNumber());
      header.Append(TString::Format(" (%lld evts recorded)",info->GetAnalyzedEventNumber()));
      TString aid(info->GetStringData("AID"));
      if (!aid.IsNull()) {
         header.Append(Form("AID=%04d",aid.Atoi()));
      }
      TString rev(info->GetStringData("REV"));
      if (!rev.IsNull()) {
         header.Append(Form("[%s]",rev.Data()));
      }
      fTitleLabel->SetLabel(header);
   } else if (folder) {
      TString header = folder->GetTitle();
      TList *runheader = (TList*)folder->FindObjectAny("runheader");
      if (runheader) {
         art::TRunInfo *info = static_cast<art::TRunInfo*>(runheader->Last());
         if (info) {
            header.Append("   ");
            header.Append(info->GetName());
            header.Append(Form("  (%ld evt)",info->GetEventNumber()));
         }
      }
      fTitleLabel->SetLabel(header);
   }
   TString dateLabel = now.AsString();
   dateLabel += TString::Format("(%u)",now.Convert());
   
   fDateLabel->SetLabel(dateLabel);
   fCanvas->Modified();
   fCanvas->Update();
      
   return fCanvas;
}


void TCatPadManager::Closed()
{
   if (fMainPad) {
      delete fMainPad;      
      fMainPad = 0;
   }
   
   fCanvas = 0;
   fMainPad = 0;
   fTitleLabel = 0;
   fDateLabel = 0;
   fCurrentPadId = 0;
   fNumSubPads = 0;
}

void TCatPadManager::MainPadClosed()
{
   fMainPad = 0;
   fCurrentPadId = 0;
   fNumSubPads = 0;
}
void TCatPadManager::Divide(Int_t nx, Int_t ny, 
                            Float_t xmargin, Float_t ymargin)
{
   GetCanvas();
   fMainPad->Clear();
   fMainPad->Divide(nx,ny,xmargin,ymargin);
   fCurrentPadId = 0;
   fNumSubPads = nx * ny;
   for (Int_t i = 0; i < fNumSubPads; ++i) {
      fMainPad->GetPad(i+1)->SetGridy(kTRUE);
      fMainPad->GetPad(i+1)->SetGridx(kTRUE);
   }

}

void TCatPadManager::SetCurrentPadId(Int_t id)
{
   if (id < 1 || id > GetNumChild()) return;
   fCurrentPadId = id;
}
