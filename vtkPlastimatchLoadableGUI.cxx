/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkPlastimatchLoadableGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkPlastimatchLoadableGUI.h"

#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkKWPushButton.h"

//------------------------------------------------------------------------------
vtkPlastimatchLoadableGUI* vtkPlastimatchLoadableGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPlastimatchLoadableGUI");
  if(ret)
    {
      return (vtkPlastimatchLoadableGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPlastimatchLoadableGUI;
}


//----------------------------------------------------------------------------
vtkPlastimatchLoadableGUI::vtkPlastimatchLoadableGUI()
{
  this->ConductanceScale = vtkKWScaleWithEntry::New();
  this->TimeStepScale = vtkKWScaleWithEntry::New();
  this->NumberOfIterationsScale = vtkKWScaleWithEntry::New();
  this->VolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->OutVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->GADNodeSelector = vtkSlicerNodeSelectorWidget::New();
  this->ApplyButton = vtkKWPushButton::New();
  this->Logic = NULL;
  this->PlastimatchLoadableNode = NULL;
}

//----------------------------------------------------------------------------
vtkPlastimatchLoadableGUI::~vtkPlastimatchLoadableGUI()
{
    
    if ( this->ConductanceScale ) {
        this->ConductanceScale->SetParent(NULL);
        this->ConductanceScale->Delete();
        this->ConductanceScale = NULL;
    }
    if ( this->TimeStepScale ) {
        this->TimeStepScale->SetParent(NULL);
        this->TimeStepScale->Delete();
        this->TimeStepScale = NULL;
    }
    if ( this->NumberOfIterationsScale ) {
        this->NumberOfIterationsScale->SetParent(NULL);
        this->NumberOfIterationsScale->Delete();
        this->NumberOfIterationsScale = NULL;
    }
    if ( this->VolumeSelector ) {
        this->VolumeSelector->SetParent(NULL);
        this->VolumeSelector->Delete();
        this->VolumeSelector = NULL;
    }
    if ( this->OutVolumeSelector ) {
        this->OutVolumeSelector->SetParent(NULL);
        this->OutVolumeSelector->Delete();
        this->OutVolumeSelector = NULL;
    }
    if ( this->GADNodeSelector ) {
        this->GADNodeSelector->SetParent(NULL);
        this->GADNodeSelector->Delete();
        this->GADNodeSelector = NULL;
    }
    if ( this->ApplyButton ) {
        this->ApplyButton->SetParent(NULL);
        this->ApplyButton->Delete();
        this->ApplyButton = NULL;
    }

  this->SetLogic (NULL);
  vtkSetMRMLNodeMacro(this->PlastimatchLoadableNode, NULL);

}

//----------------------------------------------------------------------------
void vtkPlastimatchLoadableGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkPlastimatchLoadableGUI::AddGUIObservers ( ) 
{
  this->ConductanceScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ConductanceScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->TimeStepScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TimeStepScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NumberOfIterationsScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NumberOfIterationsScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->VolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutVolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->GADNodeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkPlastimatchLoadableGUI::RemoveGUIObservers ( )
{
  this->ConductanceScale->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ConductanceScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->TimeStepScale->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TimeStepScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NumberOfIterationsScale->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NumberOfIterationsScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->VolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutVolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->GADNodeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkPlastimatchLoadableGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
  vtkKWMenu *v = vtkKWMenu::SafeDownCast(caller);
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if ( s == this->ConductanceScale && event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    this->UpdateMRML();
    }
  else if (s == this->TimeStepScale && event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    this->UpdateMRML();
    }
  else if (s == this->NumberOfIterationsScale && event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    this->UpdateMRML();
    }
  else if (selector == this->VolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->VolumeSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML();
    }
  else if (selector == this->OutVolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->OutVolumeSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML();
    }
  if (selector == this->GADNodeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->GADNodeSelector->GetSelected() != NULL) 
    { 
    vtkMRMLPlastimatchLoadableNode* n = vtkMRMLPlastimatchLoadableNode::SafeDownCast(this->GADNodeSelector->GetSelected());
    this->Logic->SetAndObservePlastimatchLoadableNode(n);
    vtkSetAndObserveMRMLNodeMacro( this->PlastimatchLoadableNode, n);
    this->UpdateGUI();
    }
  else if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->UpdateMRML();
    this->Logic->Apply();
    }
  
}

//---------------------------------------------------------------------------
void vtkPlastimatchLoadableGUI::UpdateMRML ()
{
  vtkMRMLPlastimatchLoadableNode* n = this->GetPlastimatchLoadableNode();
  if (n == NULL)
    {
    // no parameter node selected yet, create new
    this->GADNodeSelector->SetSelectedNew("vtkMRMLPlastimatchLoadableNode");
    this->GADNodeSelector->ProcessNewNodeCommand("vtkMRMLPlastimatchLoadableNode", "GADParameters");
    n = vtkMRMLPlastimatchLoadableNode::SafeDownCast(this->GADNodeSelector->GetSelected());

    // set an observe new node in Logic
    this->Logic->SetAndObservePlastimatchLoadableNode(n);
    vtkSetAndObserveMRMLNodeMacro(this->PlastimatchLoadableNode, n);
   }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);

  // set node parameters from GUI widgets
  n->SetConductance(this->ConductanceScale->GetValue());
  
  n->SetTimeStep(this->TimeStepScale->GetValue());
  
  n->SetNumberOfIterations((int)floor(this->NumberOfIterationsScale->GetValue()));
  
  if (this->VolumeSelector->GetSelected() != NULL)
    {
    n->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
    }

  if (this->OutVolumeSelector->GetSelected() != NULL)
    {
    n->SetOutputVolumeRef(this->OutVolumeSelector->GetSelected()->GetID());
    }
}

//---------------------------------------------------------------------------
void vtkPlastimatchLoadableGUI::UpdateGUI ()
{
  vtkMRMLPlastimatchLoadableNode* n = this->GetPlastimatchLoadableNode();
  if (n != NULL)
    {
    // set GUI widgest from parameter node
    this->ConductanceScale->SetValue(n->GetConductance());
    
    this->TimeStepScale->SetValue(n->GetTimeStep());
    
    this->NumberOfIterationsScale->SetValue(n->GetNumberOfIterations());
    }
}

//---------------------------------------------------------------------------
void vtkPlastimatchLoadableGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLPlastimatchLoadableNode* node = vtkMRMLPlastimatchLoadableNode::SafeDownCast(caller);
  if (node != NULL && this->GetPlastimatchLoadableNode() == node) 
    {
    this->UpdateGUI();
    }
}




//---------------------------------------------------------------------------
void vtkPlastimatchLoadableGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  vtkMRMLPlastimatchLoadableNode* gadNode = vtkMRMLPlastimatchLoadableNode::New();
  this->Logic->GetMRMLScene()->RegisterNodeClass(gadNode);
  gadNode->Delete();

  this->UIPanel->AddPage ( "PlastimatchLoadable", "PlastimatchLoadable", NULL );
  // ---
  // MODULE GUI FRAME 
  // ---
   // Define your help text and build the help frame here.
    const char *help = "The PlastimatchLoadable module....";
    const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "PlastimatchLoadable" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    
  vtkSlicerModuleCollapsibleFrame *moduleFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( "PlastimatchLoadable" ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText ("Gradient Anisotropic Diffusion Filter");
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("PlastimatchLoadable")->GetWidgetName());
  
  this->GADNodeSelector->SetNodeClass("vtkMRMLPlastimatchLoadableNode", NULL, NULL, "GADParameters");
  this->GADNodeSelector->SetNewNodeEnabled(1);
  this->GADNodeSelector->NoneEnabledOn();
  this->GADNodeSelector->SetShowHidden(1);
  this->GADNodeSelector->SetParent( moduleFrame->GetFrame() );
  this->GADNodeSelector->Create();
  this->GADNodeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->GADNodeSelector->UpdateMenu();

  this->GADNodeSelector->SetBorderWidth(2);
  this->GADNodeSelector->SetLabelText( "GAD Parameters");
  this->GADNodeSelector->SetBalloonHelpString("select a GAD node from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->GADNodeSelector->GetWidgetName());


  this->ConductanceScale->SetParent( moduleFrame->GetFrame() );
  this->ConductanceScale->SetLabelText("Conductance");
  this->ConductanceScale->Create();
  int w = this->ConductanceScale->GetScale()->GetWidth ( );
  this->ConductanceScale->SetRange(0,10);
  this->ConductanceScale->SetResolution (0.1);
  this->ConductanceScale->SetValue(1.0);
  
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->ConductanceScale->GetWidgetName());

  this->TimeStepScale->SetParent( moduleFrame->GetFrame() );
  this->TimeStepScale->SetLabelText("Time Step");
  this->TimeStepScale->Create();
  this->TimeStepScale->GetScale()->SetWidth ( w );
  this->TimeStepScale->SetRange(0.0, 1.0);
  this->TimeStepScale->SetValue(0.1);
  this->TimeStepScale->SetResolution (0.01);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->TimeStepScale->GetWidgetName());

  this->NumberOfIterationsScale->SetParent( moduleFrame->GetFrame() );
  this->NumberOfIterationsScale->SetLabelText("Iterations");
  this->NumberOfIterationsScale->Create();
  this->NumberOfIterationsScale->GetScale()->SetWidth ( w );
  this->NumberOfIterationsScale->SetValue(1);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->NumberOfIterationsScale->GetWidgetName());

  this->VolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->VolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->VolumeSelector->UpdateMenu();

  this->VolumeSelector->SetBorderWidth(2);
  this->VolumeSelector->SetLabelText( "Input Volume: ");
  this->VolumeSelector->SetBalloonHelpString("select an input volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->VolumeSelector->GetWidgetName());
  
  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, "GADVolumeOut");
  this->OutVolumeSelector->SetNewNodeEnabled(1);
  this->OutVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->OutVolumeSelector->Create();
  this->OutVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->OutVolumeSelector->UpdateMenu();

  this->OutVolumeSelector->SetBorderWidth(2);
  this->OutVolumeSelector->SetLabelText( "Output Volume: ");
  this->OutVolumeSelector->SetBalloonHelpString("select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->OutVolumeSelector->GetWidgetName());


  this->ApplyButton->SetParent( moduleFrame->GetFrame() );
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth ( 8 );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                this->ApplyButton->GetWidgetName());

  moduleFrame->Delete();

  
}
