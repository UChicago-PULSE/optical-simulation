// CppUserExtensionApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <ctime>
#include <functional>
#include <assert.h>
#include <fstream>

// Note - .tlh files will be generated from the .tlb files (above) once the project is compiled.
// Visual Studio will incorrectly continue to report IntelliSense error messages however until it is restarted.
#include "zosapi.h"

using namespace std;
using namespace ZOSAPI;
using namespace ZOSAPI_Interfaces;

void handleError(std::string msg);
void logInfo(std::string msg);
void finishUserExtension(IZOSAPI_ApplicationPtr TheApplication);

bool nextTraining(INCERowPtr mirror, INCERowPtr source) {
	/* Your implementation here */
	return 0;
}

bool nextTesting(INCERowPtr mirror, INCERowPtr source) {
	/* Your implementation here */
	return 0;
}

void runAction(INCERowPtr mirror, double qc_data[2][2]) {
	

}


void runRayTrace(IOpticalSystemPtr TheSystem) {
	INSCRayTracePtr TheRayTrace = TheSystem->Tools->OpenNSCRayTrace();
	TheRayTrace->ClearDetectorObject(13);
	TheRayTrace->IgnoreErrors = true;

	ISystemToolPtr baseTool = TheRayTrace;
	baseTool->RunAndWaitForCompletion();
	baseTool->Close();
}


int RunExtension()
{
	CoInitialize(nullptr);

		// Create the initial connection class
		IZOSAPI_ConnectionPtr TheConnection(__uuidof(ZOSAPI_Connection));


	// Attempt to connect to the existing OpticStudio instance
	IZOSAPI_ApplicationPtr TheApplication = nullptr;
	try
	{
		TheApplication = TheConnection->ConnectToApplication(); // this will throw an exception if not launched from OpticStudio
	}
	catch (exception& ex)
	{
		handleError(ex.what());
		return -1;
	}
	if (TheApplication == nullptr)
	{
		handleError("An unknown connection error occurred!");
		return -1;
	}
	if (TheApplication->Mode != ZOSAPI_Mode::ZOSAPI_Mode_Plugin)
	{
		handleError("User Extension started in wrong mode!");
		return -1;
	}

	// Check the connection status
	if (!TheApplication->IsValidLicenseForAPI)
	{
		handleError("License check failed!");
		return -1;
	}

	TheApplication->ProgressPercent = 0;
	TheApplication->ProgressMessage = L"Running Extension...";

	IOpticalSystemPtr TheSystem = TheApplication->PrimarySystem;
	if (!TheApplication->TerminateRequested) // This will be 'true' if the user clicks on the Cancel button
	{

		std::cout << "Begining Program \n";

		INonSeqEditorPtr TheNCE = TheSystem->NCE;


		INCERowPtr source = TheNCE->GetObjectAt(1);
		INCERowPtr mirror = TheNCE->GetObjectAt(9);
		INCERowPtr quadcell = TheNCE->GetObjectAt(13);

		int x_pix = 2;
		int y_pix = 2;

		double source_bound = 0.95;

		double mirror_range = 5.5;
		double source_range = 0.95; //arbitrary

		double mirror_current_x = mirror->GetTiltAboutX();
		double mirror_current_y = mirror->GetTiltAboutY();

		double mirror_action_x = mirror->GetTiltAboutX();
		double mirror_action_y = mirror->GetTiltAboutY();

		double source_current_x = source->GetTiltAboutX();
		double source_current_y = source->GetTiltAboutY();



		double original_data[2][2];
		double post_data[2][2];

		const int mirror_test_ranges = 100;
		const int qdcell_test_ranges = 100;

		//int qMatrix[mirror_test_ranges][mirror_test_ranges][qdcell_test_ranges][qdcell_test_ranges][qdcell_test_ranges][qdcell_test_ranges];

		fstream textfile;
		string filepath = TheApplication->SamplesDir + "\\API\\CPP\\q_matrix.csv";
		textfile.open(filepath, fstream::trunc | ios::out);

		std::cout << "Beginning Training \n";



		while (nextTraining(mirror, source)) {

			//Run the ray Tracing
			runRayTrace(TheSystem);

			//Save quad cell pixel read out.
			for (int i = 0; i < x_pix; i++)
				for (int j = 0; j < y_pix; j++)
					TheNCE->GetDetectorData(4, j + i * 2, 1, &original_data[i][j]);


			//Save mirror position before action is decided
			mirror_current_x = mirror->GetTiltAboutX();
			source_current_x = source->GetTiltAboutX();

			mirror_current_y = mirror->GetTiltAboutY();
			source_current_y = source->GetTiltAboutY();


			runAction(mirror, original_data);

			//Save mirror after
			double mirror_action_x = mirror->GetTiltAboutX();
			double mirror_action_y = mirror->GetTiltAboutY();


			//Run the ray Tracing
			runRayTrace(TheSystem);

			//Save quad cell pixel read out.
			for (int i = 0; i < x_pix; i++)
				for (int j = 0; j < y_pix; j++)
					TheNCE->GetDetectorData(4, j + i * 2, 1, &post_data[i][j]);








		}














	}

	// Clean up
	finishUserExtension(TheApplication);

	return 0;
}





void handleError(std::string msg)
{
	throw new exception(msg.c_str());
}

void logInfo(std::string msg)
{
	printf("%s", msg.c_str());
}

void finishUserExtension(IZOSAPI_ApplicationPtr TheApplication)
{
	// Note - OpticStudio will stay in User Extension mode until this application exits
	if (TheApplication != nullptr)
	{
		TheApplication->ProgressMessage = L"Complete";
		TheApplication->ProgressPercent = 100;
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	return RunExtension();
}

int _tmain(int argc, _TCHAR* argv[])
{
	return RunExtension();
}