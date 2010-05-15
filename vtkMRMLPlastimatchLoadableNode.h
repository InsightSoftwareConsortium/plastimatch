/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLPlastimatchLoadableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLPlastimatchLoadableNode_h
#define __vtkMRMLPlastimatchLoadableNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

#include "vtkPlastimatchLoadable.h"

class vtkImageData;

class VTK_EXAMPLELOADABLEMODULE_EXPORT vtkMRMLPlastimatchLoadableNode 
    : public vtkMRMLNode
{
 public:
    static vtkMRMLPlastimatchLoadableNode *New();
    vtkTypeMacro(vtkMRMLPlastimatchLoadableNode,vtkMRMLNode);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Create instance of a GAD node.
    virtual vtkMRMLNode* CreateNodeInstance();

    // Description:
    // Set node attributes from name/value pairs
    virtual void ReadXMLAttributes( const char** atts);

    // Description:
    // Write this node's information to a MRML file in XML format.
    virtual void WriteXML(ostream& of, int indent);

    // Description:
    // Copy the node's attributes to this object
    virtual void Copy(vtkMRMLNode *node);

    // Description:
    // Get unique node XML tag name (like Volume, Model)
    virtual const char* GetNodeTagName() {return "GADParameters";};

    // Description:
    // Get/Set Number of iterations (module parameter)
    vtkGetMacro(NumberOfIterations, int);
    vtkSetMacro(NumberOfIterations, int);

    // Description:
    // Get/Set Conductance (module parameter)
    vtkGetMacro(Conductance, double);
    vtkSetMacro(Conductance, double);

    // Description:
    // Get/Set time step (module parameter)
    vtkGetMacro(TimeStep, double);
    vtkSetMacro(TimeStep, double);
 
    // Description:
    // Get/Set fixed volume MRML Id
    vtkGetStringMacro(FixedVolumeRef);
    vtkSetStringMacro(FixedVolumeRef);
  
    // Description:
    // Get/Set moving volume MRML Id
    vtkGetStringMacro(MovingVolumeRef);
    vtkSetStringMacro(MovingVolumeRef);
  
    // Description:
    // Get/Set output volume MRML Id
    vtkGetStringMacro(OutputVolumeRef);
    vtkSetStringMacro(OutputVolumeRef);

    // Description:
    // Update the stored reference to another node in the scene
    virtual void UpdateReferenceID(const char *oldID, const char *newID);

 
 protected:
    vtkMRMLPlastimatchLoadableNode();
    ~vtkMRMLPlastimatchLoadableNode();
    vtkMRMLPlastimatchLoadableNode(const vtkMRMLPlastimatchLoadableNode&);
    void operator=(const vtkMRMLPlastimatchLoadableNode&);

    double Conductance;
    double TimeStep;
    int NumberOfIterations;

    char* OutputVolumeRef;
    char* FixedVolumeRef;
    char* MovingVolumeRef;
};

#endif
