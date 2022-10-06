/**
 * @file SEEGContactsROIPipeline.cpp
 *
 * Implementation of the SEEGContactsROIPipeline class
 * based on TrajectoryROIPipeline
 *
 * @author Silvain Beriault & Rina Zelmann
 */


// Header files to include
#include "SEEGContactsROIPipeline.h"
#include "itkCastImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"
#include <qglobal.h>



namespace seeg {
/**** CONSTRUCTORS / DESTRUCTOR ****/
SEEGContactsROIPipeline::SEEGContactsROIPipeline (const string& templateVolumeFile, SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE type) {
    m_TemplateVolume = ReadFloatVolume(templateVolumeFile);
    m_ElectrodeModel = SEEGElectrodeModel::New(type);
    this->InitPipeline();
}

SEEGContactsROIPipeline::SEEGContactsROIPipeline (FloatVolume::Pointer templateVolume, SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE type) {
    m_TemplateVolume = templateVolume;
    m_ElectrodeModel = SEEGElectrodeModel::New(type);
    this->InitPipeline();
}

SEEGContactsROIPipeline::SEEGContactsROIPipeline (IntVolume::Pointer templateVolume, SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE type) {
    typedef CastImageFilter<IntVolume, FloatVolume> CastFilterType;
    CastFilterType::Pointer castFilter = CastFilterType::New();
    castFilter->SetInput(templateVolume);
    castFilter->Update();
    m_TemplateVolume = castFilter->GetOutput();
    m_ElectrodeModel = SEEGElectrodeModel::New(type);
    this->InitPipeline();
}

SEEGContactsROIPipeline::SEEGContactsROIPipeline (ByteVolume::Pointer templateVolume, SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE type) {
    typedef CastImageFilter<ByteVolume, FloatVolume> CastFilterType;
    CastFilterType::Pointer castFilter = CastFilterType::New();
    castFilter->SetInput(templateVolume);
    castFilter->Update();
    m_TemplateVolume = castFilter->GetOutput();
    m_ElectrodeModel = SEEGElectrodeModel::New(type);
    this->InitPipeline();
}


SEEGContactsROIPipeline::~SEEGContactsROIPipeline() {
    // Do nothing
}


/**** PUBLIC FUNCTIONS ****/

    // Getters and setters
    FloatVolume::Pointer SEEGContactsROIPipeline::GetTemplateVol() {
        return m_TemplateVolume;
    }

    void SEEGContactsROIPipeline::SetTemplateVol(FloatVolume::Pointer templateVol) {
        m_TemplateVolume = templateVol;
    }

    void SEEGContactsROIPipeline::EmptyTemplateVol() {
        m_TemplateVolume->FillBuffer(0);
        /*ImageRegionIterator<FloatVolume> it2c(m_TemplateVolume, m_TemplateVolume->GetLargestPossibleRegion());
          it2c.GoToBegin();
          while(!it2c.IsAtEnd()) {
              m_TemplateVolume->SetPixel(0);
          }*/
    }

// functions that actually do something
     FloatVolume::Pointer SEEGContactsROIPipeline::CompDistMapFromMask(FloatVolume::Pointer inputVol) {
        typedef BinaryImageToShapeLabelMapFilter<FloatVolume> BinaryImageToShapeLabelMapFilterType;
//        typedef ExtractImageFilter< FloatVolume, FloatVolume > CropFilterType;
//        typedef MultiplyImageFilter <FloatVolume, FloatVolume > MultiplyImageFilterType;

        FloatVolume::RegionType regionMax;
        regionMax = inputVol->GetLargestPossibleRegion();

        // 2.Organize image
        // 2.1. Get bounding box of segmented region
        BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
        binaryImageToShapeLabelMapFilter->SetInput(inputVol);
        binaryImageToShapeLabelMapFilter->SetInputForegroundValue(1);
        binaryImageToShapeLabelMapFilter->Update();
        BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = binaryImageToShapeLabelMapFilter->GetOutput()->GetNthLabelObject(0);
        FloatVolume::RegionType region = labelObject->GetBoundingBox();
        region.PadByRadius(1);

        // 2.2. Crop image
        RegionOfInterestFilterType::Pointer regionOfInterestFilter = RegionOfInterestFilterType::New();
        regionOfInterestFilter->SetRegionOfInterest(region);
        regionOfInterestFilter->SetInput(inputVol);
        FloatVolume::Pointer smallVol = regionOfInterestFilter->GetOutput();

        // Compute info from EDGE
        // 3. Compute Distance Map from Edge
        FloatVolume::Pointer distMapFromEdgeVol = CompDistMapFromEdge(smallVol);
        return distMapFromEdgeVol;
    }

    FloatVolume::Pointer SEEGContactsROIPipeline::CompDistMapFromEdge(FloatVolume::Pointer inputVol) {
        // locally define filters
        typedef BinaryContourImageFilter <FloatVolume, FloatVolume > binaryContourImageFilterType;
        typedef AddImageFilter <FloatVolume, FloatVolume, FloatVolume> AddImageFilterType;

        // 1. get contour
        binaryContourImageFilterType::Pointer edgeFilter = binaryContourImageFilterType::New();
        edgeFilter->SetInput(inputVol);
        edgeFilter->SetForegroundValue(1);
        edgeFilter->SetBackgroundValue(0);
        edgeFilter->Update();

        // 2. obtain distance map for contour
        DistanceWithinStructureFilterType::Pointer distFilter = DistanceWithinStructureFilterType::New();
        distFilter->SetInput(edgeFilter->GetOutput());
        distFilter->Update();
        //IntVolType::Pointer outputVoronoiVol = distFilter->GetVoronoiMap();
        //IntVolType::Pointer outputVecDistMapVol = distFilter->GetVectorDistanceMap();

        // 3. keep only internal distance map + the broder (thus, change zero to 1)
        AddImageFilterType::Pointer addOneFilter = AddImageFilterType::New();
        addOneFilter->SetInput1(distFilter->GetOutput());
        addOneFilter->SetConstant2(1);
        addOneFilter->Update();

        MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New ();
        multiplyFilter->SetInput1(inputVol);
        multiplyFilter->SetInput2(addOneFilter->GetOutput());
        multiplyFilter->Update();
        FloatVolume::Pointer outputVol = multiplyFilter->GetOutput();
        return outputVol;
    }

    FloatVolume::Pointer SEEGContactsROIPipeline::CompSquareDistMapFromPoints(FloatVolume::Pointer inputVol) {

        // 1. obtain distance map for contour - using MAURER distance maps
        DistanceSignedMauerWithinStructureFilterType::Pointer distFilter = DistanceSignedMauerWithinStructureFilterType::New();
        distFilter->SetInput(inputVol);
        distFilter->SetBackgroundValue(0);
      //  distFilter->SetUseImageSpacing(true);
        distFilter->InsideIsPositiveOff();
        distFilter->SquaredDistanceOn();
        distFilter->Update();

        // 2. Get max intesity
        MaxMinFilterType::Pointer maxMinFilter = MaxMinFilterType::New ();
        maxMinFilter->SetImage(distFilter->GetOutput());
        maxMinFilter->ComputeMaximum();
        //cout << "Max: " << maxMinFilter->GetMaximum() << std::endl;

        // 3. invert values to have largest value at center
        InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
        invertIntensityFilter->SetInput(distFilter->GetOutput());
        invertIntensityFilter->SetMaximum(maxMinFilter->GetMaximum());
        invertIntensityFilter->Update();

        // 4. Normalize by dividing by MAX
        MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New ();
        multiplyFilter->SetInput1(invertIntensityFilter->GetOutput());
   //     multiplyFilter->SetInput1(distFilter->GetOutput());
        double normFactor = maxMinFilter->GetMaximum();
        multiplyFilter->SetInput2(1/normFactor); // normalize by 1
        FloatVolume::Pointer outputVol = multiplyFilter->GetOutput();
        multiplyFilter->Update();

        return outputVol;
    }

    FloatVolume::Pointer SEEGContactsROIPipeline::CompSquareDistMapFromPoint(FloatVolume::Pointer inputVol) {

        // 1. obtain distance map for contour (Danielsson Distance)
        DistanceWithinStructureFilterType::Pointer distFilter = DistanceWithinStructureFilterType::New();
        distFilter->SetInput(inputVol);
        distFilter->Update();

        // 2. Get max intesity
        MaxMinFilterType::Pointer maxMinFilter = MaxMinFilterType::New ();
        maxMinFilter->SetImage(distFilter->GetOutput());
        //imageCalculatorFilter->SetRegion(recDistanceMap->GetRequestedRegion());
        maxMinFilter->ComputeMaximum();
        //cout << "Max: " << maxMinFilter->GetMaximum() << std::endl;


        // 3. invert values to have largest value at center
        InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
        invertIntensityFilter->SetInput(distFilter->GetOutput());
        invertIntensityFilter->SetMaximum(maxMinFilter->GetMaximum());
        invertIntensityFilter->Update();

        // 4. square of the distance -> multiply by itself -- see how to normalize!
        MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New ();
        multiplyFilter->SetInput1(invertIntensityFilter->GetOutput());
        double normFactor = maxMinFilter->GetMaximum();
        multiplyFilter->SetInput2(1/normFactor ); // normalize by 1
        multiplyFilter->Update();

        multiplyFilter->Modified();
        multiplyFilter->SetInput1(multiplyFilter->GetOutput());
        multiplyFilter->SetInput2(multiplyFilter->GetOutput());// square
        multiplyFilter->Update();

        FloatVolume::Pointer outputVol = multiplyFilter->GetOutput();
        return outputVol;
    }

    FloatVolume::Pointer SEEGContactsROIPipeline::CompSquareDistMapFromPoint(FloatVolume::Pointer inputVol, FloatVolume::RegionType region) {

        // 1. obtain distance map for contour
        DistanceWithinStructureFilterType::Pointer distFilter = DistanceWithinStructureFilterType::New();
        distFilter->SetInput(inputVol);
        DistanceWithinStructureFilterType::VoronoiImagePointer voroniVol = distFilter->GetVoronoiMap();
        voroniVol->SetRequestedRegion(region);
        FloatVolume::Pointer outDist = distFilter->GetOutput();
        // outDist->SetRequestedRegion(region); --> does not work from this output - bug in ITK
        distFilter->Update();

        // 2. Get max intesity
        MaxMinFilterType::Pointer maxMinFilter = MaxMinFilterType::New ();
        maxMinFilter->SetImage(outDist);
        maxMinFilter->SetRegion(region);
        maxMinFilter->ComputeMaximum();
    //    cout << "Max: " << maxMinFilter->GetMaximum() << std::endl;


        // 3. invert values to have largest value at center
        InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
        invertIntensityFilter->SetInput(outDist);
        invertIntensityFilter->SetMaximum(maxMinFilter->GetMaximum());
      //  invertIntensityFilter->Update();

        // 4. square of the distance -> multiply by itself -- see how to normalize!
        MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New ();
        multiplyFilter->SetInput1(invertIntensityFilter->GetOutput());
        double normFactor = maxMinFilter->GetMaximum();
        multiplyFilter->SetInput2(1/normFactor); // normalize by 1
        multiplyFilter->Update();

        multiplyFilter->Modified();
        multiplyFilter->SetInput1(multiplyFilter->GetOutput());
        multiplyFilter->SetInput2(multiplyFilter->GetOutput());// square
        FloatVolume::Pointer outputVol = multiplyFilter->GetOutput();
        outputVol->SetRequestedRegion(region);
        multiplyFilter->Update();

        return outputVol;
    }

    FloatVolume::Pointer SEEGContactsROIPipeline::GetRecordedVolPerElectrode(ElectrodeInfo::Pointer electrode){

        bool onlyInsdeBrain = false; // RIZ20151227 since entry point could be specified inside the brain, compute contact position also "outside"

        // duplicate volume to have same size -instead get from original volume
      /*  FloatVolume::Pointer vol = FloatVolume::New(); //volDuplicator->GetOutput();
        vol->CopyInformation(m_TemplateVolume);
        vol->SetRegions(vol->GetLargestPossibleRegion());
        vol->Allocate();
        */
        FloatVolumeDuplicator::Pointer volDuplicator = FloatVolumeDuplicator::New();
        volDuplicator->SetInputImage(m_TemplateVolume); //m_TemplateVolume should be clean! --> only to have appropriate size
        volDuplicator->Update();
        FloatVolume::Pointer vol = volDuplicator->GetOutput();
        vol->DisconnectPipeline();


        // create empty electrode to assign entry-target
        //SEEGElectrodeModel::Pointer electrode = SEEGElectrodeModel::New(SEEGElectrodeModel::MNI);
        double recRadius = m_ElectrodeModel->GetRecordingRadius();

        // define region of interest
        FloatVolume::RegionType electrodeRegion;
        FloatVolume::IndexType entryPointIndex, targetPointIndex;
        vol->TransformPhysicalPointToIndex(electrode->m_EntryPointWorld, entryPointIndex);
        vol->TransformPhysicalPointToIndex(electrode->m_TargetPointWorld, targetPointIndex);
        FloatVolume::SpacingType spacing = vol->GetSpacing();

        Point3D radiousPtIndex;
        radiousPtIndex[0] =recRadius * spacing[0];
        radiousPtIndex[1] =recRadius * spacing[1];
        radiousPtIndex[2] = recRadius * spacing[2];

        CalcTrajBoundingBox(entryPointIndex, targetPointIndex, radiousPtIndex, electrodeRegion);
        FloatVolume::SizeType regionSize = electrodeRegion.GetSize();
        FloatVolume::IndexType elecRegionStart = electrodeRegion.GetIndex();
        FloatVolume::RegionType volRegion = vol->GetRequestedRegion();
        FloatVolume::SizeType volRegionSize = volRegion.GetSize();
        FloatVolume::IndexType volRegionStart = volRegion.GetIndex();
        if (abs((int)regionSize[0]+elecRegionStart[0]) < abs((int)volRegionSize[0]+volRegionStart[0]) && abs((int)regionSize[1]+elecRegionStart[1]) < abs((int)volRegionSize[1]+volRegionStart[1]) && abs((int)regionSize[2]+elecRegionStart[2]) < abs((int)volRegionSize[2]+volRegionStart[2])) {
            electrodeRegion.PadByRadius(1); // Only add if region is smaller than limit of vol image
            // Check if index is smaller than zero -> put back to zero if that's the case
            elecRegionStart = electrodeRegion.GetIndex();
            if (elecRegionStart[0]<0) { elecRegionStart[0]=0; }
            if (elecRegionStart[1]<0) { elecRegionStart[1]=0; }
            if (elecRegionStart[2]<0) { elecRegionStart[2]=0; }
            electrodeRegion.SetIndex(elecRegionStart); // reassign with the negative in zero
        } else {
            cout << "No Padding possible EP:"<< electrode->m_EntryPointWorld<<" TP: "<<electrode->m_TargetPointWorld<<endl;
        }

        // get small volume
       // cout << "Electrode Region: " << electrodeRegion.GetSize();
        RegionOfInterestFilterType::Pointer regionOfInterestFilter = RegionOfInterestFilterType::New();
        regionOfInterestFilter->SetInput(vol); //vol
        regionOfInterestFilter->SetRegionOfInterest(electrodeRegion);
        FloatVolume::Pointer smallVol = regionOfInterestFilter->GetOutput();
        regionOfInterestFilter->Update();
      //  Q_ASSERT( smallVol );
        FloatVolume::RegionType region = smallVol->GetRequestedRegion();
        //create contour - it should be a sphere -- RIZ: for now computes recording in whole bounding box

        // locate each contact in vol and set the corresponding central pixel in smallVol
        vector<Point3D> allContactsWorld;
        m_ElectrodeModel->CalcAllContactPositions(electrode->m_TargetPointWorld, electrode->m_EntryPointWorld, allContactsWorld, onlyInsdeBrain); //RIZ20151227 - corrected! started from entry point!!!

        for (int iCont=0; iCont<m_ElectrodeModel->GetNumContacts(); iCont++){
            FloatVolume::IndexType index;
            smallVol->TransformPhysicalPointToIndex(allContactsWorld[iCont], index);
            if (region.IsInside(index)){
                smallVol->SetPixel(index, 1);
            } else {
                //cout << "No Region for this contact "<< iCont <<"- EP:"<< electrode->m_EntryPointWorld<<" TP: "<<electrode->m_TargetPointWorld<<endl;
                break;
            }
        }
    //    WriteFloatVolume("/Users/rzelmann/test/smallVol.mnc", smallVol);
    //    WriteFloatVolume("/Users/rzelmann/test/origVol.mnc", m_TemplateVolume);


  /*      for (int iCont=0; iCont<electrode->GetNumContacts(); iCont++){
            FloatVolume::IndexType index;
            vol->TransformPhysicalPointToIndex(allContactsWorld[iCont], index);
            if (region.IsInside(index)){
                vol->SetPixel(index, 1);
            } else {
                break;
            }
        }
*/
        // Compute Distance Map for each contact
        FloatVolume::Pointer distMapAllContacts = CompSquareDistMapFromPoint(smallVol);  //Danielsson Distance map
   //     FloatVolume::Pointer distMapAllContacts1 = CompSquareDistMapFromPoints(smallVol); //Maurer Distance map
       //DELETE!! FloatVolume::Pointer distMapAllContacts = CompSquareDistMapFromPoint(vol, electrodeRegion);
        // only to test if distMap is ok
   //     WriteFloatVolume("/Users/rzelmann/test/distMapAllContactsMaurer.mnc", distMapAllContacts1);
        //WriteFloatVolume("/Users/rzelmann/test/distMapAllContactsDanielsson.mnc", distMapAllContacts);


        // bring back to orignal size
        FloatVolume::IndexType destinationIndex =  electrodeRegion.GetIndex();
        //     cout << "Electrode Region: " << destinationIndex <<" - "<< electrodeRegion.GetSize() << std::endl;
        PasteImageFilterType::Pointer pasteFilter = PasteImageFilterType::New ();
        pasteFilter->SetSourceImage(distMapAllContacts);
        pasteFilter->SetDestinationImage(vol);
        pasteFilter->SetSourceRegion(distMapAllContacts->GetLargestPossibleRegion());
        pasteFilter->SetDestinationIndex(destinationIndex);
        FloatVolume::Pointer outputVol =  pasteFilter->GetOutput(); // RIZ20160322: changed before the update as in all other filters
        pasteFilter->Update();
        // output
//        FloatVolume::Pointer outputVol =  pasteFilter->GetOutput();
        outputVol->SetRequestedRegion(electrodeRegion);
       // Q_ASSERT( outputVol );
        // only to test if distMap is ok
  //      WriteFloatVolume("/home/rina/test/pasteDistMapAllContacts.mnc", outputVol);

        return outputVol;
    }

    void SEEGContactsROIPipeline::CalcRecordingMap(FloatVolume::Pointer targetDistMap, FloatVolume::Pointer &recordingMap, ElectrodeInfo::Pointer electrode){

        MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New ();

        // get volume recorded for this electrode
        FloatVolume::Pointer recVolume = GetRecordedVolPerElectrode(electrode);
        FloatVolume::RegionType region = recVolume->GetRequestedRegion();
        //cout << "CalcRecordingMap Region: " << region.GetIndex() <<" - "<< region.GetSize() << std::endl;
        //cout << "targetDistMap Region: " << targetDistMap->GetRequestedRegion().GetSize() << std::endl;

        // multiply by distance map of volume of interest
        multiplyFilter->SetInput1(targetDistMap);
        multiplyFilter->SetInput2(recVolume);
        multiplyFilter->Update();
        recordingMap = multiplyFilter->GetOutput();
        recordingMap->SetRequestedRegion(region);
    //    Q_ASSERT( recordingMap );
       // cout << "recordingMap Region: " << recordingMap->GetRequestedRegion().GetSize() << std::endl;

        // only to test if distMap is ok
 //       WriteFloatVolume("/home/rina/test/targetDistMap.mnc", targetDistMap);
 //      WriteFloatVolume("/home/rina/test/recordingMap.mnc", recordingMap);
        //   FloatVolume::Pointer recordingMap = multiplyFilter->GetOutput();
        //  return outputVol;
    }

    void SEEGContactsROIPipeline::CalcRecordingMap(vector<FloatVolume::Pointer> targetDistMaps, vector<FloatVolume::Pointer> &recordingMaps, ElectrodeInfo::Pointer electrode){
        // get volume recorded for this electrode
        FloatVolume::Pointer recVolume = GetRecordedVolPerElectrode(electrode);
        FloatVolume::RegionType region = recVolume->GetRequestedRegion();
      //  cout << "CalcRecordingMap Region: " << region.GetIndex() <<" - "<< region.GetSize() << std::endl;
   //     cout << "targetDistMap Region: " << targetDistMap->GetRequestedRegion().GetSize() << std::endl;

        // multiply by distance map of volume of interest for each target
        for (int iTarget=0; iTarget<targetDistMaps.size(); iTarget++){
            FloatVolume::Pointer targetDistMap = targetDistMaps[iTarget];
            //targetDistMap->SetRequestedRegion(region);
            MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New();
            multiplyFilter->SetInput2(recVolume);
            multiplyFilter->Modified(); //RIZ:: this should come back --works with this! maybe a bug in ITK
            //cout<<"Target" << iTarget<< multiplyFilter->GetMTime()<<endl;
            multiplyFilter->SetInput1(targetDistMap);
            FloatVolume::Pointer recordingMap = multiplyFilter->GetOutput();
            multiplyFilter->Update();
            recordingMap->SetRequestedRegion(region); //multiply FULL image --> there will be garbage if only consider requested region
      //      WriteFloatVolume("/home/rina/test/recMap.mnc", recordingMap);
            recordingMaps.push_back(recordingMap);
           // Q_ASSERT( recordingMap );

        //    cout << "recordingMap Region: " << recordingMap->GetRequestedRegion().GetSize() << std::endl;
        }


    }

    void SEEGContactsROIPipeline::CalcTrajBoundingBox(FloatVolume::IndexType entryPointIndex, FloatVolume::IndexType targetPointIndex,  Point3D maxRadius, FloatVolume::RegionType& region) {
    // Calculate the trajectory's bounding box.
    FloatVolume::RegionType regionMax;
    int min[3];
    int max[3];

    FloatVolume::SpacingType spacing;
    spacing = m_TemplateVolume->GetSpacing();

    for (int i=0; i<3; i++) {
        if (entryPointIndex[i] < targetPointIndex[i]) {
            min[i] = (((float)(entryPointIndex[i]*spacing[i]) - maxRadius[i]) / spacing[i]) + 0.5; //CHANGED RIZ20160424 - before it was +0.5 instead of -0.5
            max[i] = (((float)(targetPointIndex[i]*spacing[i]) + maxRadius[i]) / spacing[i]) + 0.5;
        } else {
            max[i] = (((float)(entryPointIndex[i]*spacing[i]) + maxRadius[i]) / spacing[i]) + 0.5;
            min[i] = (((float)(targetPointIndex[i]*spacing[i]) - maxRadius[i]) / spacing[i]) + 0.5; //CHANGED RIZ20160424 - before it was +0.5 instead of -0.5
        }
    }

    regionMax = m_TemplateVolume->GetLargestPossibleRegion();
    FloatVolume::IndexType start;
    FloatVolume::SizeType size;
    start = regionMax.GetIndex();
    size = regionMax.GetSize();

    for (int i=0; i<3; i++) {
        if (min[i] < start[i]) {
            min[i] = start[i];
        }
        if (max[i] >= (int) (start[i] + size[i])) {
            max[i] = start[i] + size[i] - 1;
        }

    }
    for (int i=0; i<3; i++) {
        start[i] = min[i];
        size[i] = max[i] - min[i] + 1;
    }
    region.SetIndex(start);
    region.SetSize(size);
}

// Contact based analysis of labels   
    vector <int> SEEGContactsROIPipeline::GetLabelsInContact(int contactIndex, ElectrodeInfo::Pointer electrode, FloatVolume::Pointer anatLabelsMap, int &voxelsInContactVol, bool useCylinder){
        //Get a vector with pairs containing labels and percentage for this contact

        //1. Find region that corresponds to contact
        FloatVolume::Pointer anatLabelsOfContact;
        if (useCylinder == true){ // if true use the whole contact's cylinder to estimate location /
            anatLabelsOfContact = CalcVolOfContact(contactIndex, electrode, anatLabelsMap);  //considers volume that could record EEG - looks for area with largest presence in recording volume
        } else { // if false use only central point and 1 voxel around it
            anatLabelsOfContact = CalcVolOfCenterOfContact(contactIndex, electrode, anatLabelsMap); //only considers central point of contact
        }
        //2. Get size of region
        FloatVolume::RegionType region = anatLabelsOfContact->GetRequestedRegion();
        FloatVolume::SizeType regionSize = region.GetSize();
        cout << "contactIndex: " << contactIndex << " - channelRecVol Size: " << regionSize <<endl;

        //3. iterate in region to obtain different labels
        vector<int> labelsCount;
        //Initialize labelCount vector
        MaxMinFilterType::Pointer maxMinFilter = MaxMinFilterType::New ();
        maxMinFilter->SetImage(anatLabelsOfContact);
        maxMinFilter->ComputeMaximum();
        cout <<"Contact index: "<< contactIndex<< " Maximum Label Value: " << maxMinFilter->GetMaximum() << std::endl;
        for (int iLabel=0; iLabel<=maxMinFilter->GetMaximum(); iLabel++){
            labelsCount.push_back(0);
        }
        //Count number of voxels per label
        FloatVolumeRegionIterator it (anatLabelsOfContact, region);
        int voxelCount = 0;
        for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
          //  FloatVolume::IndexType voxelIndex = it.GetIndex();
            voxelCount++;
            if (it.Get()>0) {
                //Point3D worldEntryPoint;
                //anatLabelsOfContact->TransformIndexToPhysicalPoint(voxelIndex, worldPoint);
                int labelValue = (int)it.Get(); //value is
                labelsCount[labelValue]++; // = labelsCount[labelValue] + 1/regionSize;
            }
        }

        //4. return label count & voxel count to compute percentage
        voxelsInContactVol = voxelCount;
        return labelsCount;
    }

    FloatVolume::Pointer SEEGContactsROIPipeline::CalcVolOfContact(int contactIndex, ElectrodeInfo::Pointer electrode, FloatVolume::Pointer anatLabelsMap){
        //Find All labels within the volume of the contact and return a volume with the labels

        //Empty vol to have access to transforms
        FloatVolumeDuplicator::Pointer volDuplicator = FloatVolumeDuplicator::New();
        volDuplicator->SetInputImage(m_TemplateVolume); //m_TemplateVolume should be clean! --> only to have appropriate size
        volDuplicator->Update();
        FloatVolume::Pointer vol = volDuplicator->GetOutput();
        vol->DisconnectPipeline();
        
        double extraRadious; // to include partial voxels it should be 1 - computed as extraRadious * volSpacing
        extraRadious = m_ElectrodeModel->GetRecordingRadius(); // to consider recording radious as specified by model (defined following vonEllenrieder2014)
        FloatVolume::SpacingType spacing = vol->GetSpacing();
        Point3D extraRadiousIndex;
        extraRadiousIndex[0] =extraRadious * spacing[0];
        extraRadiousIndex[1] =extraRadious * spacing[1];
        extraRadiousIndex[2] = extraRadious * spacing[2];

        //1. Find start and end of electrode
        Point3D contactStartPt, contactEndPt;
        m_ElectrodeModel->CalcContactStartEnds(contactIndex, electrode->GetTargetPoint(), electrode->GetEntryPoint(), contactStartPt, contactEndPt);

        //2. convert to indexes
        FloatVolume::RegionType contactRegion;
        FloatVolume::IndexType contactStartPtIndex, contactEndPtIndex;
        vol->TransformPhysicalPointToIndex(contactStartPt, contactStartPtIndex);
        vol->TransformPhysicalPointToIndex(contactEndPt, contactEndPtIndex);

        //3. Calculate Bounding box (contact's region of interest)
        CalcTrajBoundingBox(contactEndPtIndex, contactStartPtIndex, extraRadiousIndex, contactRegion);

        //4. Get region of interest in anatLabelsMap
        RegionOfInterestFilterType::Pointer regionOfInterestFilter = RegionOfInterestFilterType::New();
        regionOfInterestFilter->SetInput(anatLabelsMap); //volume to reduce size
        regionOfInterestFilter->SetRegionOfInterest(contactRegion);
        FloatVolume::Pointer smallVol = regionOfInterestFilter->GetOutput();
        regionOfInterestFilter->Update();

        return smallVol;
}

    FloatVolume::Pointer SEEGContactsROIPipeline::CalcVolOfCenterOfContact(int contactIndex, ElectrodeInfo::Pointer electrode, FloatVolume::Pointer anatLabelsMap){
        //Find All labels within the volume of the contact and return a volume with the labels

        //Empty vol to have access to transforms
        FloatVolumeDuplicator::Pointer volDuplicator = FloatVolumeDuplicator::New();
        volDuplicator->SetInputImage(m_TemplateVolume); //m_TemplateVolume should be clean! --> only to have appropriate size
        volDuplicator->Update();
        FloatVolume::Pointer vol = volDuplicator->GetOutput();
        vol->DisconnectPipeline();

        double extraRadious = 1; // to include partial voxels it should be 1  (that gives 27 voxels) / to ONLY consider central point use zero - RIZ should be CHANGED to config value - computed as extraRadious * volSpacing
        FloatVolume::SpacingType spacing = vol->GetSpacing();
        Point3D extraRadiousIndex;
        extraRadiousIndex[0] =extraRadious * spacing[0];
        extraRadiousIndex[1] =extraRadious * spacing[1];
        extraRadiousIndex[2] = extraRadious * spacing[2];

        //1. Find start and end of electrode
        Point3D contactStartPt, contactEndPt;
        m_ElectrodeModel->CalcContactStartEnds(contactIndex, electrode->GetTargetPoint(), electrode->GetEntryPoint(), contactStartPt, contactEndPt);

        ContactInfo::Pointer contact = electrode->GetOneContact(contactIndex);
        Point3D contactCentralPt = contact->GetCentralPoint();

        //2. convert to indexes
        FloatVolume::RegionType contactRegion;
        FloatVolume::IndexType contactCentralPtIndex, contactStartPtIndex;
        vol->TransformPhysicalPointToIndex(contactStartPt, contactStartPtIndex);
        vol->TransformPhysicalPointToIndex(contactCentralPt, contactCentralPtIndex);

        //3. Calculate Bounding box (contact's region of interest)
        CalcTrajBoundingBox(contactCentralPtIndex, contactCentralPtIndex, extraRadiousIndex, contactRegion);

        //4. Get region of interest in anatLabelsMap
        RegionOfInterestFilterType::Pointer regionOfInterestFilter = RegionOfInterestFilterType::New();
        regionOfInterestFilter->SetInput(anatLabelsMap); //volume to reduce size
        regionOfInterestFilter->SetRegionOfInterest(contactRegion);
        FloatVolume::Pointer smallVol = regionOfInterestFilter->GetOutput();
        regionOfInterestFilter->Update();

        return smallVol;
}

// Channel based analysis of labels
    FloatVolume::Pointer SEEGContactsROIPipeline::CalcChannelRecordingArea(Point3D contact1Center, Point3D contact2Center,  double maxRadius){
        // if NO region is specified use LargestRegion in TemplateVolume
        FloatVolume::Pointer channelRecVol = CalcChannelRecordingArea(contact1Center, contact2Center, maxRadius, m_TemplateVolume->GetLargestPossibleRegion());
        return channelRecVol;
    }


    FloatVolume::Pointer SEEGContactsROIPipeline::CalcChannelRecordingArea(Point3D contact1Center, Point3D contact2Center,  double maxRadius, FloatVolume::RegionType channelRegion){
        // Calculates bipolar channel volume. The volume is modeled as a cilinder between contacts.
        // The length equals the distance between the outer tips of the two electrodes and 10mm diameter and one half sphere at each end.
        cout<< "Entering CalcChannelRecordingArea - P1:"<<contact1Center << "P2: " <<contact2Center <<endl;
        //Create bipolar channel model & position based on contacts centers
        //Recording volume consist of 1 cylnder and 2 spheres
        m_ChannelModel = BipolarChannelModel::New(contact1Center, contact2Center, maxRadius, maxRadius);
       // cout << "m_ChannelModel P1:" <<m_ChannelModel->GetContact1Point() << " P2: " <<m_ChannelModel->GetContact2Point() << endl;
        m_ChannelModel->GenerateImage(m_TemplateVolume, channelRegion);

        // Get Volume with model
        FloatVolume::Pointer channelRecVol = m_ChannelModel->GetBipolarChannelVolume();
       // cout << "channelRecVol Origin: " << channelRecVol->GetOrigin() << "channelRecVol Size: " << channelRecVol->GetRequestedRegion()<<endl;

        return channelRecVol;
    }

    vector <int> SEEGContactsROIPipeline::GetLabelsInChannel(int contact1Index, int contact2Index, ElectrodeInfo::Pointer electrode, FloatVolume::Pointer anatLabelsMap, bool useCylinder){
        //Get a vector with pairs containing labels and percentage for this contact

        //1. Find volume that corresponds to bipolar channel's recordings
        FloatVolume::Pointer anatLabelsOfChannel;
        anatLabelsOfChannel = CalcVolOfChannel(contact1Index, contact2Index, electrode, anatLabelsMap);  //considers volume that could record EEG - looks for area with largest presence in recording volume
        if (useCylinder == true){ // if true use the whole contact's cylinder to estimate location /
            anatLabelsOfChannel = CalcVolOfChannel(contact1Index, contact2Index, electrode, anatLabelsMap);  //considers volume that could record EEG - looks for area with largest presence in recording volume
        } else { // if false use only central point and 1 voxel around it
            anatLabelsOfChannel = CalcVolOfCenterOfContact((contact2Index - contact1Index), electrode, anatLabelsMap); //only considers central point of contact
        }

        //2. Get size of region - RIZ-> not sure how to define region... maybe use normal BoundingBox
        FloatVolume::RegionType region = anatLabelsOfChannel->GetRequestedRegion();
//        cout << "anatLabelsOfChannel Region: " << region.GetSize();

        //3. iterate in region to obtain different labels
        vector<int> labelsCount;
        //Initialize labelCount vector
        MaxMinFilterType::Pointer maxMinFilter = MaxMinFilterType::New ();
        maxMinFilter->SetImage(anatLabelsOfChannel);
        maxMinFilter->ComputeMaximum();
        cout <<"Contact 1 index: "<< contact1Index<<"Contact 2 index: "<< contact2Index<< " Maximum Label Value: " << maxMinFilter->GetMaximum() << std::endl;
        for (int iLabel=0; iLabel<=maxMinFilter->GetMaximum(); iLabel++){
            labelsCount.push_back(0);
        }
        //Count number of voxels per label
        FloatVolumeRegionIterator it (anatLabelsOfChannel, region);
        for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
          //  FloatVolume::IndexType voxelIndex = it.GetIndex();
            if (it.Get()>0) {
                //Point3D worldEntryPoint;
                //anatLabelsOfContact->TransformIndexToPhysicalPoint(voxelIndex, worldPoint);
                int labelValue = (int)it.Get(); //value is
                labelsCount[labelValue]++; // = labelsCount[labelValue] + 1/regionSize;
            }
        }

        //4. return label count
        return labelsCount;
    }

    FloatVolume::Pointer SEEGContactsROIPipeline::CalcVolOfChannel(int contact1Index, int contact2Index, ElectrodeInfo::Pointer electrode, FloatVolume::Pointer anatLabelsMap){
        //Find All labels within the volume of the contact and return a volume with the labels

        //Empty vol to have access to transforms
        FloatVolumeDuplicator::Pointer volDuplicator = FloatVolumeDuplicator::New();
        volDuplicator->SetInputImage(m_TemplateVolume); //m_TemplateVolume should be clean! --> only to have appropriate size
        volDuplicator->Update();
        FloatVolume::Pointer vol = volDuplicator->GetOutput();
        vol->DisconnectPipeline();

        double maxRadius;
        maxRadius = m_ElectrodeModel->GetRecordingRadius(); // to consider recording radious as specified by model (defined following vonEllenrieder2014) - Default 5mm
        FloatVolume::SpacingType spacing = vol->GetSpacing();

        Point3D extraRadiousIndex; // to include partial voxels it should be 1 - computed as extraRadious * volSpacing
        extraRadiousIndex[0] =maxRadius * spacing[0];
        extraRadiousIndex[1] =maxRadius * spacing[1];
        extraRadiousIndex[2] = maxRadius * spacing[2];
    //    cout << "Spacing: " << spacing << endl;

        //1. Find start and end of electrode
        Point3D contact1StartPt, contact1EndPt;
        Point3D contact2StartPt, contact2EndPt;
        m_ElectrodeModel->CalcContactStartEnds(contact1Index, electrode->GetTargetPoint(), electrode->GetEntryPoint(), contact1StartPt, contact1EndPt);
        m_ElectrodeModel->CalcContactStartEnds(contact2Index, electrode->GetTargetPoint(), electrode->GetEntryPoint(), contact2StartPt, contact2EndPt);
        Point3D contact1Center = m_ElectrodeModel->SEEGCalcContactPosition(contact1Index, electrode->GetTargetPoint(), electrode->GetEntryPoint(), 0);
        Point3D contact2Center = m_ElectrodeModel->SEEGCalcContactPosition(contact2Index, electrode->GetTargetPoint(), electrode->GetEntryPoint(), 0);

        //2. convert to indexes
        FloatVolume::RegionType channelRegion;
        FloatVolume::IndexType contact1StartPtIndex, contact2EndPtIndex;
        vol->TransformPhysicalPointToIndex(contact1StartPt, contact1StartPtIndex); // from beginning of contact1 to end of contact2
        vol->TransformPhysicalPointToIndex(contact2EndPt, contact2EndPtIndex);

        //3. Calculate Channel's Bounding box (channel's largest region of interest) - normal box around area of interest to define "region"
        CalcTrajBoundingBox(contact1StartPtIndex, contact2EndPtIndex, extraRadiousIndex, channelRegion);
 //       cout << "channelRegion Region: " << channelRegion<<endl;

        //4. Calculate Channel's recording area - represented as cilinder from start of contact 1 to end of contact2 + half cylinder at the ends
        FloatVolume::Pointer channelRecVol = CalcChannelRecordingArea(contact1Center, contact2Center, maxRadius, channelRegion);
//        cout << "channelRecVol Origin: " << channelRecVol->GetOrigin() <<  "- Spacing: " << channelRecVol->GetSpacing() << " - Size: " << channelRecVol->GetRequestedRegion()<<endl;
//        cout << "anatLabelsMap Origin: " << anatLabelsMap->GetOrigin() <<  "- Spacing: " << anatLabelsMap->GetSpacing() << " - Size: " << anatLabelsMap->GetRequestedRegion()<<endl;

        //5. Get bounding box - region of interest in anatLabelsMap
        RegionOfInterestFilterType::Pointer regionOfInterestFilter = RegionOfInterestFilterType::New();
        regionOfInterestFilter->SetInput(anatLabelsMap); //volume to reduce size
        regionOfInterestFilter->SetRegionOfInterest(channelRegion);
        FloatVolume::Pointer smallVolLabels = regionOfInterestFilter->GetOutput();
        regionOfInterestFilter->Update();
//        cout << "smallVolLabels Origin: " << smallVolLabels->GetOrigin() << "- Spacing: " << smallVolLabels->GetSpacing() << " - Size: " << smallVolLabels->GetRequestedRegion()<<endl;

        //6. Multiply model of recording by anatLabelsMap to obtain labels within the recording volume
        MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New ();
        multiplyFilter->SetInput1(smallVolLabels);
        multiplyFilter->SetInput2(channelRecVol);
        FloatVolume::Pointer outputVol = multiplyFilter->GetOutput();
        multiplyFilter->Update();


 /*       stringstream strContactsIndex;
        strContactsIndex << contact1Index << contact2Index;
        WriteFloatVolume("/Users/rzelmann/test/channelRecordings_"+ strContactsIndex.str() +".mnc", channelRecVol);
        WriteFloatVolume("/Users/rzelmann/test/channelLabels_"+ strContactsIndex.str() +".mnc", outputVol);
*/
        return outputVol;
    }


    FloatVolume::Pointer SEEGContactsROIPipeline::GetChannelModelVol(){
        FloatVolume::Pointer channelRecVol = m_ChannelModel->GetBipolarChannelVolume();
        return channelRecVol;
    }

    int SEEGContactsROIPipeline::GetNumberVoxelsInChannelModel(){
        int channelModelVoxels = m_ChannelModel->GetNumberVoxelsInChannel();
        return channelModelVoxels;
    }


    
    /**** PRIVATE FUNCTIONS ****/
    void SEEGContactsROIPipeline::InitPipeline() {
    // do nothing
    }

}
