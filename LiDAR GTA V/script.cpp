#define _USE_MATH_DEFINES
#include "script.h"
#include "keyboard.h"
#include <string>
#include <ctime>
#include <fstream>
#include <math.h>

#pragma warning(disable : 4244 4305) // double <-> float conversions

void notificationOnLeft(std::string notificationText) {
	UI::_SET_NOTIFICATION_TEXT_ENTRY("CELL_EMAIL_BCON");
	const int maxLen = 99;
	for (int i = 0; i < notificationText.length(); i += maxLen) {
		std::string divideText = notificationText.substr(i, min(maxLen, notificationText.length() - i));
		const char* divideTextAsConstCharArray = divideText.c_str();
		char* divideTextAsCharArray = new char[divideText.length() + 1];
		strcpy_s(divideTextAsCharArray, divideText.length() + 1, divideTextAsConstCharArray);
		UI::_ADD_TEXT_COMPONENT_STRING(divideTextAsCharArray);
	}
	int handle = UI::_DRAW_NOTIFICATION(false, 1);
}

struct ray {
	bool hit;
	Vector3 hitCoordinates;
	Vector3 surfaceNormal;
	std::string entityTypeName;
	int rayResult;
	int hitEntityHandle;
	int class_instance = 0;
	Vector3 vertex1, vertex2, vertex3, vertex4, vertex5, vertex6, vertex7, vertex8;
	float length, width, height;
	Vector3 vehicle_position;
};

ray raycast(Vector3 source, Vector3 direction, float maxDistance, int intersectFlags) {
	ray result;
	float targetX = source.x + (direction.x * maxDistance);
	float targetY = source.y + (direction.y * maxDistance);
	float targetZ = source.z + (direction.z * maxDistance);
	int rayHandle = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(source.x, source.y, source.z, targetX, targetY, targetZ, intersectFlags, 0, 7);
	int hit = 0;
	int hitEntityHandle = -1;
	/*bool occlusion;
	int classid;*/
	Hash model;		/*Model of the car just hit by the ray cast*/
	/*Vehicle bounding box data*/
	Vector3 FUR; //Front Upper Right
	Vector3 BLL; //Back Lower Lelft
	Vector3 vehicle_centroid; //Vehicle vehicle_centroidensions
	Vector3 upVector, rightVector, forwardVector, position; //Vehicle position
	Vector3 min;
	Vector3 max;
	Vector3 hitCoordinates;

	hitCoordinates.x = 0;
	hitCoordinates.y = 0;
	hitCoordinates.z = 0;
	Vector3 surfaceNormal;
	surfaceNormal.x = 0;
	surfaceNormal.y = 0;
	surfaceNormal.z = 0;
	
	int rayResult = WORLDPROBE::_GET_RAYCAST_RESULT(rayHandle, &hit, &hitCoordinates, &surfaceNormal, &hitEntityHandle);
	int entityInstance = ENTITY::GET_OBJECT_INDEX_FROM_ENTITY_INDEX(result.hitEntityHandle);			//Instance ID of the class of the object hit
	result.rayResult = rayResult;
	result.hit = hit;
	result.hitCoordinates = hitCoordinates;
	result.surfaceNormal = surfaceNormal;
	result.hitEntityHandle = hitEntityHandle;
	std::string entityTypeName = "Unknown";
	/*Here comes the golden information THANK YOU ROCKSTAR GAMES*/
	model = ENTITY::GET_ENTITY_MODEL(hitEntityHandle);
	/*NOTE! here the right ,forward,upvector, min and max might need to be translated with respect to LIDAR origin too.*/
	GAMEPLAY::GET_MODEL_DIMENSIONS(model, &min, &max);
	//ENTITY::GET_ENTITY_MATRIX(hitEntityHandle, &rightVector, &forwardVector, &upVector, &position); //Blue or red pill
	forwardVector = ENTITY::GET_ENTITY_FORWARD_VECTOR(hitEntityHandle);
	rightVector = forwardVector;		/*DEBUG modify after debug*/
	upVector = forwardVector;			/*DEBUG modify after debug*/
	
	/*Get vehicle dimensions*/
	vehicle_centroid.x = 0.5*(max.x - min.x);
	vehicle_centroid.y = 0.5*(max.y - min.y);
	vehicle_centroid.z = 0.5*(max.z - min.z);
	result.vehicle_position = vehicle_centroid;

	/*Following the convention of length > width*/
	if ((max.x - min.x) > (max.y - min.y)) { 
		result.length = max.x - min.x;
		result.width = max.y - min.y;
		result.height = max.z - min.z;
		}
	else {
		result.length = max.y - min.y; 
		result.width = max.x - min.x;
		result.height = max.z - min.z;
	}
	/*Get vehicle bounding box vertices for ground truth*/
	/*TODO these vertices value might be with respect to the WORLD origin, need to shift the origin to LIDAR origin*/
	FUR.x = position.x + vehicle_centroid.y*rightVector.x + vehicle_centroid.x*forwardVector.x + vehicle_centroid.z*upVector.x;
	FUR.y = position.y + vehicle_centroid.y*rightVector.y + vehicle_centroid.x*forwardVector.y + vehicle_centroid.z*upVector.y;
	FUR.z = position.z + vehicle_centroid.y*rightVector.z + vehicle_centroid.x*forwardVector.z + vehicle_centroid.z*upVector.z;
	//GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(FUR.x, FUR.y, 1000.0, &(FUR.z), 0);
	//FUR.z += 2 * vehicle_centroid.z;

	BLL.x = position.x - vehicle_centroid.y*rightVector.x - vehicle_centroid.x*forwardVector.x - vehicle_centroid.z*upVector.x;
	BLL.y = position.y - vehicle_centroid.y*rightVector.y - vehicle_centroid.x*forwardVector.y - vehicle_centroid.z*upVector.y;
	BLL.z = position.z - vehicle_centroid.y*rightVector.z - vehicle_centroid.x*forwardVector.z - vehicle_centroid.z*upVector.z;

	result.vertex1 = BLL;
	result.vertex5 = FUR;
	 
	result.vertex2.x = result.vertex1.x + 2 * vehicle_centroid.y*rightVector.x;
	result.vertex2.y = result.vertex1.y + 2 * vehicle_centroid.y*rightVector.y;
	result.vertex2.z = result.vertex1.z + 2 * vehicle_centroid.y*rightVector.z;

	result.vertex3.x = result.vertex2.x + 2 * vehicle_centroid.z*upVector.x;
	result.vertex3.y = result.vertex2.y + 2 * vehicle_centroid.z*upVector.y;
	result.vertex3.z = result.vertex2.z + 2 * vehicle_centroid.z*upVector.z;

	result.vertex4.x = result.vertex1.x + 2 * vehicle_centroid.z*upVector.x;
	result.vertex4.y = result.vertex1.y + 2 * vehicle_centroid.z*upVector.y;
	result.vertex4.z = result.vertex1.z + 2 * vehicle_centroid.z*upVector.z;

	result.vertex6.x = result.vertex5.x - 2 * vehicle_centroid.y*rightVector.x;
	result.vertex6.y = result.vertex5.y - 2 * vehicle_centroid.y*rightVector.y;
	result.vertex6.z = result.vertex5.z - 2 * vehicle_centroid.y*rightVector.z;

	result.vertex7.x = result.vertex6.x - 2 * vehicle_centroid.z*upVector.x;
	result.vertex7.y = result.vertex6.y - 2 * vehicle_centroid.z*upVector.y;
	result.vertex7.z = result.vertex6.z - 2 * vehicle_centroid.z*upVector.z;

	result.vertex8.x = result.vertex5.x - 2 * vehicle_centroid.z*upVector.x;
	result.vertex8.y = result.vertex5.y - 2 * vehicle_centroid.z*upVector.y;
	result.vertex8.z = result.vertex5.z - 2 * vehicle_centroid.z*upVector.z;

	/*Get point wise label*/
	if (ENTITY::DOES_ENTITY_EXIST(hitEntityHandle)) {
		int entityType = ENTITY::GET_ENTITY_TYPE(hitEntityHandle);
		if (entityType == 1) {entityTypeName = "GTA.Ped";}
		/*NOTE! Here vehicle means only car modify this if you need all/other vehicles*/
		else if (entityType == 2 && VEHICLE::IS_THIS_MODEL_A_CAR(model))  {entityTypeName = "GTA.Vehicle";result.class_instance = entityInstance;}
		else if (entityType == 3) {entityTypeName = "GTA.Prop";}
	}
	result.entityTypeName = entityTypeName;
	return result;
}

ray angleOffsetRaycast(double angleOffsetX, double angleOffsetZ, int range) {
	Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(2);
	double rotationX = (rot.x + angleOffsetX) * (M_PI / 180.0);
	double rotationZ = (rot.z + angleOffsetZ) * (M_PI / 180.0);
	double multiplyXY = abs(cos(rotationX));
	Vector3 direction;
	direction.x = sin(rotationZ) * multiplyXY * -1;
	direction.y = cos(rotationZ) * multiplyXY;
	direction.z = sin(rotationX);
	ray result = raycast(CAM::GET_GAMEPLAY_CAM_COORD(), direction, range, -1);
	return result;
}

void lidar(double horiFovMin, double horiFovMax, double vertFovMin, double vertFovMax, double horiStep, double vertStep, int range, std::string filePath, std::string filePath_label, std::string kitti_label_path)
{
	GAMEPLAY::SET_GAME_PAUSED(true);
	TIME::PAUSE_CLOCK(true);
	double vertexCount = (horiFovMax - horiFovMin) * (1 / horiStep) * (vertFovMax - vertFovMin) * (1 / vertStep);
	Vector3 origin = CAM::GET_GAMEPLAY_CAM_COORD();
	std::ofstream fileOutput;
	std::ofstream fileOutput_label_data;
	std::ofstream fileOutput_kitti_labels;
	fileOutput.open(filePath);
	fileOutput_label_data.open(filePath_label);
	fileOutput_kitti_labels.open(kitti_label_path);
	/*Deep GTA 5 params for camera initiation*/
	/*=======================================*/
	/*Vehicle vehicle = NULL;
	Player player = NULL;
	Ped ped = NULL;
	Cam camera = NULL;
	Vector3 dir;
	Vector3 pos, rotation;
	float heading;*/
	
	/*======================================*/
	//int classid;
	///*Host vehicle data*/
	//Vector3 upVector, rightVector, forwardVector, position; //Vehicle position
	//Hash vehicleHash = 0x50732C82;
	//Vector3 speedVector;
	
	/*=======================================*/
	
	//while (!ENTITY::DOES_ENTITY_EXIST(vehicle)) {
	//	vehicle = VEHICLE::CREATE_VEHICLE(vehicleHash, pos.x, pos.y, pos.z, heading, FALSE, FALSE);
	//	WAIT(0);
	//}
	//VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(vehicle);
	// 

	//while (!ENTITY::DOES_ENTITY_EXIST(ped)) {
	//	ped = PLAYER::PLAYER_PED_ID();
	//	WAIT(0);
	//}
	//ENTITY::GET_ENTITY_MATRIX(ped, &rightVector, &forwardVector, &upVector, &position);

	//player = PLAYER::PLAYER_ID();
	//PLAYER::START_PLAYER_TELEPORT(player, pos.x, pos.y, pos.z, heading, 0, 0, 0);

	//PED::SET_PED_INTO_VEHICLE(ped, vehicle, -1);
	////STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(vehicleHash);

	//rotation = ENTITY::GET_ENTITY_ROTATION(vehicle, 1);
	//CAM::DESTROY_ALL_CAMS(TRUE);
	//camera = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", TRUE);
	///*if (strcmp(_vehicle, "packer") == 0) CAM::ATTACH_CAM_TO_ENTITY(camera, vehicle, 0, 2.35, 1.7, TRUE);
	//else*/ 
	//CAM::ATTACH_CAM_TO_ENTITY(camera, vehicle, 0.2, 0.5, 1.5, TRUE); /*TODO if it works this is what I want to modify*/
	//CAM::SET_CAM_FOV(camera, 60);
	//CAM::SET_CAM_ACTIVE(camera, TRUE);
	//CAM::SET_CAM_ROT(camera, rotation.x, rotation.y, rotation.z, 1);
	//CAM::SET_CAM_INHERIT_ROLL_VEHICLE(camera, TRUE);
	//CAM::RENDER_SCRIPT_CAMS(TRUE, FALSE, 0, TRUE, TRUE);

	for (double z = horiFovMin; z < horiFovMax; z += horiStep)
	{
		std::string vertexData = "";
		std::string label_Data = "";
		std::string kitti_label = "";
		for (double x = vertFovMin; x < vertFovMax; x += vertStep)
		{
			std::string entityName3 = "None";
			int entityHash = 0;
			unsigned char r = 0; unsigned char g = 0; unsigned char b = 0;
			ray result = angleOffsetRaycast(x, z, range);
			if (result.hit)
			{
				r = 255; g = 255; b = 255;
			}
			if (result.hitEntityHandle != -1)
			{
				entityName3 = result.entityTypeName;
				if (entityName3 == "GTA.Vehicle")
				{
					r = 255; g = 0; b = 0;
				}
				else if (entityName3 == "GTA.Ped")
				{
					r = 0; g = 255; b = 0;
				}
				else if (entityName3 == "GTA.Prop")
				{
					r = 0; g = 0; b = 255;
				}
			}
			/*result.hitCoordinates.x = result.hitCoordinates.x - origin.x;
			result.hitCoordinates.y = result.hitCoordinates.y - origin.y;
			result.hitCoordinates.z = result.hitCoordinates.z - origin.z;*/
			// TODO Change the x axis to be aligned with forward vector and +y axis to be left of vehicle. 
			vertexData += std::to_string(result.hitCoordinates.x) + " " + std::to_string(result.hitCoordinates.y) + " " + std::to_string(result.hitCoordinates.z) + " " + std::to_string(0) + "\n";	//place holder values for reflectance
			label_Data += std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b) + " " + std::to_string(result.class_instance) +  "\n";		//placeholder value for instance values	
			kitti_label += std::to_string(result.vertex1.x) + " " + std::to_string(result.vertex1.y) + " " + std::to_string(result.vertex1.z) + " " +
				std::to_string(result.vertex2.x) + " " + std::to_string(result.vertex2.y) + " " + std::to_string(result.vertex2.z) + " " +
				std::to_string(result.vertex3.x) + " " + std::to_string(result.vertex3.y) + " " + std::to_string(result.vertex3.z) + " " +
				std::to_string(result.vertex4.x) + " " + std::to_string(result.vertex4.y) + " " + std::to_string(result.vertex4.z) + " " +
				std::to_string(result.vertex5.x) + " " + std::to_string(result.vertex5.y) + " " + std::to_string(result.vertex5.z) + " " +
				std::to_string(result.vertex6.x) + " " + std::to_string(result.vertex6.y) + " " + std::to_string(result.vertex6.z) + " " +
				std::to_string(result.vertex7.x) + " " + std::to_string(result.vertex7.y) + " " + std::to_string(result.vertex7.z) + " " +
				std::to_string(result.vertex8.x) + " " + std::to_string(result.vertex8.y) + " " + std::to_string(result.vertex8.z) + "\n";
		}
		fileOutput << vertexData;
		fileOutput_label_data << label_Data;
		fileOutput_kitti_labels << kitti_label;
	}
	fileOutput.close();
	fileOutput_label_data.close();
	fileOutput_kitti_labels.close();
	GAMEPLAY::SET_GAME_PAUSED(false);
	TIME::PAUSE_CLOCK(false);
	//notificationOnLeft("LiDAR Point Cloud written to file.");
}

void ScriptMain()
{
	srand(GetTickCount());
	DWORD start_time;
	DWORD elapsed_time;
	int file_number = 0;
	bool init = TRUE;
	int range;
	std::string filename;
	std::string label_filename;
	std::ifstream inputFile;
	std::string ignore;
	double parameters[6];
	while (true)
	{
		if (IsKeyJustUp(VK_F6))
			if (init) {
				inputFile.open("LiDAR GTA V/LiDAR GTA V.cfg");
				if (inputFile.bad()) {
					notificationOnLeft("Input file not found. Please re-install the plugin.");
					continue;
				}
				/*Ignore the first line of the config file*/
				inputFile >> ignore >> ignore >> ignore >> ignore >> ignore;
				for (int i = 0; i < 6; i++) {
					/*Read in parameter values of all 5 LiDAR parameters*/
					inputFile >> ignore >> ignore >> parameters[i];
				}
				/*LiDAR range and filenames*/
				inputFile >> ignore >> ignore >> range;
				inputFile >> ignore >> ignore >> filename;
				inputFile >> ignore >> ignore >> label_filename;
				inputFile.close();
			}
		elapsed_time = GetTickCount() - start_time;
		if (elapsed_time > 30000)				/*in milli seconds. if more than 30 milliseconds take a lidar screen shot*/
		{
			start_time = GetTickCount();
			lidar(parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5], range,
				"LiDAR GTA V/velodyne/" + filename + "_" + std::to_string(file_number) + ".txt",
				"LiDAR GTA V/labels/" + label_filename + "_" + std::to_string(file_number) + ".txt",
				"LiDAR GTA V/kitti_labels/" + label_filename + "_kitti_" + std::to_string(file_number) + ".txt");
			file_number++;
			notificationOnLeft("Point Cloud sample number " + std::to_string(file_number) + " Generated.");
			notificationOnLeft("Resumed game.");
		}
		WAIT(0);
	}
}