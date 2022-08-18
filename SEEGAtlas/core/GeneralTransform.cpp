#include "GeneralTransform.h"
#include "vtkTransform.h" //RIZ: changed on 20140806 to be compatible with IBIS 2.2
#include <iostream>
#include <stdlib.h>
#include "vtkXFMReader.h"

using namespace std;

namespace seeg {

    GeneralTransform::GeneralTransform(const string& xfmfile, bool invert) {
    vtkTransform *transform, *transformInv; //RIZ: change to VTK class


        string xfm_file_cpy = xfmfile;
        m_GeneralTransform = vtkTransform::New();
        m_GeneralTransformInv = vtkTransform::New();


        if (invert) {
            transform = m_GeneralTransformInv;
            transformInv = m_GeneralTransform;
        } else {
            transform = m_GeneralTransform;
            transformInv = m_GeneralTransformInv;
        }


        vtkXFMReader * reader = vtkXFMReader::New();
        if( reader->CanReadFile( xfmfile.c_str() ) )
        {
            reader->SetFileName( xfmfile.c_str() );
            vtkMatrix4x4 * mat = vtkMatrix4x4::New();
            reader->SetMatrix(mat);
            reader->Update();

            transform->SetMatrix( mat );

            reader->Delete();
            mat->Delete();
        }
        else
        {
            cerr << "Cannot open transform file " << xfm_file_cpy << endl;
            reader->Delete();
            exit(1);
        }

         transformInv->DeepCopy(transform);
         transformInv->Inverse();

    }

    GeneralTransform::~GeneralTransform() {
        m_GeneralTransform->Delete();
        m_GeneralTransformInv->Delete();
    }

    void GeneralTransform::TransformPoint(const Point3D& pointIn, Point3D& pointOut) {
        double *xyz;
        xyz = m_GeneralTransform->TransformPoint( pointIn[0], pointIn[1], pointIn[2]);
        pointOut[0] = xyz[0];
        pointOut[1] = xyz[1];
        pointOut[2] = xyz[2];
    }


    void GeneralTransform::TransformPointInv(const Point3D& pointIn, Point3D& pointOut) {
        double *xyz;
        xyz = m_GeneralTransformInv->TransformPoint( pointIn[0], pointIn[1], pointIn[2]);
        pointOut[0] = xyz[0];
        pointOut[1] = xyz[1];
        pointOut[2] = xyz[2];
    }

}

