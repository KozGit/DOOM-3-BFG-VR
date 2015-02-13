#ifndef VR_MOTION_SENSOR
#define VR_MOTION_SENSOR

typedef struct _hydraData { // koz 
		idVec3 position;
		idMat3 hydraRotation; // id rotation matrix. Derived from sixense quat swapped to id coord space then converted to idMat3
		float joystick_x;
		float joystick_y;
		float trigger;
		unsigned int buttons;
} hydraData;

#define hydra_zero {vec3_zero, \
					mat3_identity, \
					0.0f, \
					0.0f, \
					0.0f, \
					0 } ;

void VR_Hydra_Init(void);
void VR_MotionSensor_Init(void);
void VR_MotionSensor_Read(float &roll, float &pitch, float &yaw);

void VR_MotionSensor_Get_Offset(float &offset); //koz 
void VR_MotionSensor_Set_Offset(float offset);
void VR_MotionSensor_Set_Left_Hydra_Offset(hydraData hydraOffset);
void VR_MotionSensor_Get_Left_Hydra_Offset(hydraData &hydraOffset);
void VR_MotionSensor_Get_Left_Hydra(hydraData &leftHydra);
void VR_MotionSensor_Get_Left_Hydra_With_Offset(hydraData &leftOffsetHydra);
void VR_MotionSensor_Set_Right_Hydra_Offset(hydraData hydraOffset);
void VR_MotionSensor_Get_Right_Hydra_Offset(hydraData &hydraOffset);
void VR_MotionSensor_Get_Right_Hydra(hydraData &rightHydra);
void VR_MotionSensor_Get_Right_Hydra_With_Offset(hydraData &rightOffsetHydra);


#endif