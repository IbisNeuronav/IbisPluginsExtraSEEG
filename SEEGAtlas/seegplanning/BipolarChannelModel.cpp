#include "BipolarChannelModel.h"


namespace seeg {

    //BipolarChannelModel::BipolarChannelModel(const double cylinderHeight, const double cylinderRadius, const double sphereRadius, const int insideValue) {
    BipolarChannelModel::BipolarChannelModel(Point3D contact1Center, Point3D contact2Center, const double cylinderRadius, const double sphereRadius, const int insideValue) {

    // 1. Assign contact center variables
    m_Contact1Center = contact1Center;
    m_Contact2Center = contact2Center;

    //2. Get Height and direction to create and position model
    Vector3D_lf pt1 (contact1Center[0], contact1Center[1], contact1Center[2]);
    Vector3D_lf pt2 (contact2Center[0], contact2Center[1], contact2Center[2]);
    double cylinderHeight = norm(pt2-pt1);                  // Cylinder's height equals the distance between points
  //  Vector3D_lf unit_vect = (pt2-pt1)/cylinderHeight;       // unit_vec indicates model's direction (vector's direction)
    Point3D centralPtContacts;
    centralPtContacts[0]=(contact1Center[0]+contact2Center[0])/2;
    centralPtContacts[1]=(contact1Center[1]+contact2Center[1])/2;
    centralPtContacts[2]=(contact1Center[2]+contact2Center[2])/2;

    // 3. Create Model's components
    //Cylinder
    CylinderType::Pointer cylinder = CylinderType::New();
    cylinder->SetRadius(cylinderRadius);
    cylinder->SetHeight(cylinderHeight);
    //cylinder->IsInside(insidePoint)
    //Sphere1
    EllipseType::Pointer sphere1    = EllipseType::New();
    sphere1->SetRadius(sphereRadius);
    //Sphere2
    EllipseType::Pointer sphere2    = EllipseType::New();
    sphere2->SetRadius(sphereRadius);

    // 4. Assign inside value
    sphere1->SetDefaultInsideValue(  insideValue );
    cylinder->SetDefaultInsideValue( insideValue );
    sphere2->SetDefaultInsideValue( insideValue );

    // 5. Position the different objects relative to each other and with respect to contact centers - sphere1 stays in place the others are moved in X
    // Position all objects
    EllipseType::TransformType::OffsetType  offsetSphere1, offsetSphere2;
    CylinderType::TransformType::OffsetType offsetCylinder;
    CylinderType::TransformType::CenterType centerCylinder;
    // Sphere1
    offsetSphere1[ 0 ] =  contact1Center[0];
    offsetSphere1[ 1 ] =  contact1Center[1];
    offsetSphere1[ 2 ] =  contact1Center[2];
    sphere1->GetObjectToParentTransform()->SetOffset(offsetSphere1);
    sphere1->ComputeObjectToWorldTransform();
    //Cylinder
    offsetCylinder[ 0 ] = contact1Center[0];
    offsetCylinder[ 1 ] = contact1Center[1];
    offsetCylinder[ 2 ] = contact1Center[2];
    cylinder->GetObjectToParentTransform()->SetOffset(offsetCylinder);
    centerCylinder[ 0 ] = centralPtContacts[0];
    centerCylinder[ 1 ] = centralPtContacts[1];
    centerCylinder[ 2 ] = centralPtContacts[2];
    cylinder->GetObjectToParentTransform()->SetCenter(centerCylinder);
    cylinder->ComputeObjectToWorldTransform();
    //Sphere2
    offsetSphere2[ 0 ] =  contact2Center[0];
    offsetSphere2[ 1 ] =  contact2Center[1];
    offsetSphere2[ 2 ] =  contact2Center[2];
    sphere2->GetObjectToParentTransform()->SetOffset(offsetSphere2);
    sphere2->ComputeObjectToWorldTransform();
    // Rotate Cylinder which axis??
  //  typedef GroupType::TransformType TransformType;
  //  TransformType::Pointer transform = TransformType::New();
 //   transform->SetIdentity();
//    TransformType::OutputVectorType  translation;
  //  TransformType::CenterType        center;
  //  transform->Rotate( 1, 2, itk::Math::pi / 2.0 ); //RIZ: MAL!!!!! see how to estimate proper rotation!
//    transform->Translate( translation, false );


    // 6. Group objects to create model
    GroupType::Pointer group = GroupType::New();
    group->AddSpatialObject( sphere1 );
    group->AddSpatialObject( cylinder );
    group->AddSpatialObject( sphere2 );
    //m_Model contains the grouped objects
    m_Model = group;

    //Save as FloatVolume Image ??
    //GenerateImage();
    }

/*    BipolarChannelModel::BipolarChannelModel(Point3D contact1Center, Point3D contact2Center, const double cylinderRadius, const double sphereRadius, const int insideValue) {

        // Assign contact center variables
        m_Contact1Center = contact1Center;
        m_Contact2Center = contact2Center;

        //Get Height and create model
        Vector3D_lf pt1 (contact1Center[0], contact1Center[1], contact1Center[2]);
        Vector3D_lf pt2 (contact2Center[0], contact2Center[1], contact2Center[2]);
        double cylinderHeight = norm(pt2-pt1);                  // Cylinder's height equals the distance between points
        Vector3D_lf unit_vect = (pt2-pt1)/distCentralPts;       // unit_vec indicates model's direction (vector's direction)

        BipolarChannelModel(cylinderHeight, cylinderRadius, sphereRadius, insideValue);

        // Position model based on center of contacts
        PositionModel();
    }
*/

    void BipolarChannelModel::PositionModel() {
        //Using the contacts' center points, Positions the model and returns the volume with the model on it

    //    m_Model

        //Save as FloatVolume Image
        GenerateImage();
    }

    void BipolarChannelModel::PositionModel(Point3D point1, Point3D point2) {
        //Given the pair of points, Positions the model and updates the volume with the model on it


        //Save as FloatVolume Image
        GenerateImage();
    }

    void BipolarChannelModel::GenerateImage() {
        // Generate corresponding image that consists of model as group of cylinders and spheres positioned as defined in group object
        // Assumes that spacing and size are already assigned

        //Get info from volume

        FloatVolume::RegionType region =  m_VolModel->GetRequestedRegion();
        FloatVolume::SizeType regionSize = region.GetSize();
        FloatVolume::SpacingType volSpacing = m_VolModel->GetSpacing();
        FloatVolume::IndexType regionIndex = region.GetIndex();
        Point3D regionOriginPt;
        m_VolModel->TransformIndexToPhysicalPoint(regionIndex,regionOriginPt);

        // Use ImageFilter to convert from model object to volume
        // inside value is 1 by default / outside is always 0
        SpatialObjectToImageFilterType::Pointer imageFilter = SpatialObjectToImageFilterType::New();
        imageFilter->SetInput(  m_Model  );
        imageFilter->SetUseObjectValue( true );
        imageFilter->SetOutsideValue( 0 );
        imageFilter->SetOrigin(regionOriginPt);
        imageFilter->SetSpacing( volSpacing );
        imageFilter->SetSize( regionSize);
        m_VolModel = imageFilter->GetOutput();
        imageFilter->Update();

      //  string filename = "/Users/rzelmann/Desktop/modelVol.mnc"; //RIZ: 2022
       // WriteFloatVolume(filename, m_VolModel);
    }


    void BipolarChannelModel::GenerateImage(FloatVolume::Pointer templateVolume, FloatVolume::RegionType channelRegion) {
        // Generate corresponding image that consists of model as group of cylinders and spheres positioned as defined in group object
        // Use this option ONLY the first time to get image size and spacing

        // First, get Image information from input template
        FloatVolumeDuplicator::Pointer volDuplicator = FloatVolumeDuplicator::New();
        volDuplicator->SetInputImage(templateVolume); //templateVolume could be empty! --> only to have appropriate size
        volDuplicator->Update();
        m_VolModel = volDuplicator->GetOutput();
        m_VolModel->DisconnectPipeline();

        m_VolModel->SetRequestedRegion(channelRegion);
        //m_VolSpacing = templateVolume->GetSpacing();
        //FloatVolume::RegionType region =  templateVolume->GetLargestPossibleRegion();
        //m_VolSize = region.GetSize();

        // Now generate volume from model
       // cout << "BipolarChannelModel Origin: " << m_VolModel->GetOrigin() <<  " - Spacing: " << m_VolModel->GetSpacing() << " - Size: " << m_VolModel->GetRequestedRegion()<<endl;
        GenerateImage();
       // cout << "BipolarChannelModel Origin: " << m_VolModel->GetOrigin() << " - Spacing: " << m_VolModel->GetSpacing() << " - Size: " << m_VolModel->GetRequestedRegion()<<endl;

    }

    int BipolarChannelModel::GetNumberVoxelsInChannel() {
        int voxelsCount = 0;
        FloatVolumeRegionIterator it (m_VolModel, m_VolModel->GetRequestedRegion());
        for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
            if (it.Get()>0) {
                voxelsCount++; // = labelsCount[labelValue] + 1/regionSize;
            }
        }
        return voxelsCount;

    }

}
