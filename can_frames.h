/*
	Can Bus Frames as defined in "RoboSub 17 CAN Frames Rev.3.xlsx"
*/


/* struct tCAN internals.
typedef struct
{
	uint16_t id;
	struct {
		int8_t rtr : 1;
		uint8_t length : 4;
	} header;
	uint8_t data[8];
} tCAN;
*/

//Critical Interrupts					
tCAN CI_Kill_Switch 		= {.id = 8, .header.rtr = 1, .header.length = 0};
tCAN CI_Leak 			= {.id = 16, .header.rtr = 0, .header.length = 3};
tCAN CI_Depth 			= {.id = 24, .header.rtr = 0, .header.length = 2};
tCAN CI_SIB_General 		= {.id = 32, .header.rtr = 0, .header.length = 1};
tCAN CI_ESC1 			= {.id = 40, .header.rtr = 0, .header.length = 1};
tCAN CI_ESC2 			= {.id = 48, .header.rtr = 0, .header.length = 1};
tCAN CI_ESC3 			= {.id = 56, .header.rtr = 0, .header.length = 1};
tCAN CI_ESC4 			= {.id = 64, .header.rtr = 0, .header.length = 1};
tCAN CI_ESC5 			= {.id = 72, .header.rtr = 0, .header.length = 1};
tCAN CI_ESC6 			= {.id = 80, .header.rtr = 0, .header.length = 1};
tCAN CI_ESC7 			= {.id = 88, .header.rtr = 0, .header.length = 1};
tCAN CI_ESC8 			= {.id = 96, .header.rtr = 0, .header.length = 1};
tCAN CI_Backplane_Current 	= {.id = 104, .header.rtr = 1, .header.length = 2};
tCAN CI_Auto 			= {.id = 112, .header.rtr = 1, .header.length = 0};

//Weapon Commands
tCAN Weapon1_Command 		= {.id = 122, .header.rtr = 1, .header.length = 0};
tCAN Weapon2_Command 		= {.id = 130, .header.rtr = 1, .header.length = 0};
tCAN Weapon3_Command 		= {.id = 138, .header.rtr = 1, .header.length = 0};
tCAN Weapon4_Command 		= {.id = 146, .header.rtr = 1, .header.length = 0};
tCAN Weapon5_Command 		= {.id = 154, .header.rtr = 1, .header.length = 0};
tCAN Weapon6_Command 		= {.id = 162, .header.rtr = 1, .header.length = 0};
tCAN Weapon7_Command 		= {.id = 170, .header.rtr = 1, .header.length = 0};
tCAN Weapon8_Command 		= {.id = 178, .header.rtr = 1, .header.length = 0};
tCAN Weapon9_Command 		= {.id = 186, .header.rtr = 1, .header.length = 0};
tCAN Weapon10_Command 		= {.id = 194, .header.rtr = 1, .header.length = 0};
tCAN Weapon11_Command 		= {.id = 202, .header.rtr = 1, .header.length = 0};
tCAN Weapon12_Command 		= {.id = 210, .header.rtr = 1, .header.length = 0};
tCAN Weapon13_Command 		= {.id = 218, .header.rtr = 1, .header.length = 0};
tCAN Weapon1_Confirm 		= {.id = 224, .header.rtr = 1, .header.length = 0};
tCAN Weapon2_Confirm 		= {.id = 232, .header.rtr = 1, .header.length = 0};
tCAN Weapon3_Confirm 		= {.id = 240, .header.rtr = 1, .header.length = 0};
tCAN Weapon4_Confirm 		= {.id = 248, .header.rtr = 1, .header.length = 0};
tCAN Weapon5_Confirm 		= {.id = 256, .header.rtr = 1, .header.length = 0};
tCAN Weapon6_Confirm 		= {.id = 264, .header.rtr = 1, .header.length = 0};
tCAN Weapon7_Confirm 		= {.id = 272, .header.rtr = 1, .header.length = 0};
tCAN Weapon8_Confirm 		= {.id = 280, .header.rtr = 1, .header.length = 0};
tCAN Weapon9_Confirm 		= {.id = 288, .header.rtr = 1, .header.length = 0};
tCAN Weapon10_Confirm 		= {.id = 296, .header.rtr = 1, .header.length = 0};
tCAN Weapon11_Confirm 		= {.id = 304, .header.rtr = 1, .header.length = 0};
tCAN Weapon12_Confirm 		= {.id = 312, .header.rtr = 1, .header.length = 0};
tCAN Weapon13_Confirm 		= {.id = 320, .header.rtr = 1, .header.length = 0};

//ESC Control(TEGRA -> WCB)
//	may not need now with TEGRA interface board.
tCAN ESC1 = {.id = 330, .header.rtr = 0, .header.length = 2};
tCAN ESC2 = {.id = 338, .header.rtr = 0, .header.length = 2};
tCAN ESC3 = {.id = 346, .header.rtr = 0, .header.length = 2};
tCAN ESC4 = {.id = 356, .header.rtr = 0, .header.length = 2};
tCAN ESC5 = {.id = 362, .header.rtr = 0, .header.length = 2};
tCAN ESC6 = {.id = 370, .header.rtr = 0, .header.length = 2};
tCAN ESC7 = {.id = 378, .header.rtr = 0, .header.length = 2};
tCAN ESC8 = {.id = 386, .header.rtr = 0, .header.length = 2};

tCAN Request_Response_SIB_Pressure 	= {.id = 392, .header.rtr = 0, .header.length = 6};
tCAN Request_Response_SIB_Temp 		= {.id = 400, .header.rtr = 0, .header.length = 3};
tCAN Request_Response_HYDRAS 		= {.id = 408, .header.rtr = 0, .header.length = 6};
tCAN Request_Response_HYDRAS_RAW 	= {.id = 416, .header.rtr = 0, .header.length = 6};
tCAN Request_Response_ESC1 		= {.id = 424, .header.rtr = 0, .header.length = 8};
tCAN Request_Response_ESC2 		= {.id = 432, .header.rtr = 0, .header.length = 8};
tCAN Request_Response_ESC3 		= {.id = 440, .header.rtr = 0, .header.length = 8};
tCAN Request_Response_ESC4 		= {.id = 448, .header.rtr = 0, .header.length = 8};
tCAN Request_Response_ESC5 		= {.id = 456, .header.rtr = 0, .header.length = 8};
tCAN Request_Response_ESC6 		= {.id = 464, .header.rtr = 0, .header.length = 8};
tCAN Request_Response_ESC7 		= {.id = 472, .header.rtr = 0, .header.length = 8};
tCAN Request_Response_ESC8 		= {.id = 480, .header.rtr = 0, .header.length = 8};
tCAN Request_Response_Weapon_status 	= {.id = 488, .header.rtr = 0, .header.length = 8};
tCAN Request_Response_Backplane_Current = {.id = 496, .header.rtr = 0, .header.length = 8};

tCAN Read_Request_SIB_Pressure 		= {.id = 507, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_SIB_Temp 		= {.id = 515, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_HYDRAS 		= {.id = 524, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_HYDRAS_RAW 		= {.id = 532, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_ESC_Vitals 		= {.id = 538, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_ESC1 			= {.id = 546, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_ESC2 			= {.id = 554, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_ESC3 			= {.id = 562, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_ESC4 			= {.id = 570, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_ESC5 			= {.id = 578, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_ESC6 			= {.id = 586, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_ESC7 			= {.id = 594, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_ESC8 			= {.id = 602, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_Weapon_status 	= {.id = 610, .header.rtr = 1, .header.length = 0};
tCAN Read_Request_Backplane_Current 	= {.id = 617, .header.rtr = 1, .header.length = 0};
