/*
	Can Bus Frames as defined in "RoboSub 17 CAN Frames Rev.3.xlsx"
*/
CONST Message_ID {8,16,24,32,40,48,56,64,72,80,88,96,104,
					112,122,130,138,146,154,162,170,178,
					186,194,202,210,218,224,232,240,248,
					256,264,272,280,288,,296,304,312,320,
					330,338,346,354,362,370,378,386,392,
					400,408,416,424,432,440,448,456,464,
					472,480,488,496,507,515,524,532,538,
					546,554,562,570,578,586,594,602,610,
					617};

//Critical Interrupts					
tCAN CI_Kill_Switch;
tCAN CI_Leak;
tCAN CI_Depth;
tCAN CI_SIB_General;
tCAN CI_ESC1;
tCAN CI_ESC2;
tCAN CI_ESC3;
tCAN CI_ESC4;
tCAN CI_ESC5;
tCAN CI_ESC6;
tCAN CI_ESC7;
tCAN CI_ESC8;
tCAN CI_Backplane_Current;
tCAN CI_Auto;

//Weapon Commands
tCAN Weapon1_Command;
tCAN Weapon2_Command;
tCAN Weapon3_Command;
tCAN Weapon4_Command;
tCAN Weapon5_Command;
tCAN Weapon6_Command;
tCAN Weapon7_Command;
tCAN Weapon8_Command;
tCAN Weapon9_Command;
tCAN Weapon10_Command;
tCAN Weapon11_Command;
tCAN Weapon12_Command;
tCAN Weapon13_Command;
tCAN Weapon1_Confirm;
tCAN Weapon2_Confirm;
tCAN Weapon3_Confirm;
tCAN Weapon4_Confirm;
tCAN Weapon5_Confirm;
tCAN Weapon6_Confirm;
tCAN Weapon7_Confirm;
tCAN Weapon8_Confirm;
tCAN Weapon9_Confirm;
tCAN Weapon10_Confirm;
tCAN Weapon11_Confirm;
tCAN Weapon12_Confirm;
tCAN Weapon13_Confirm;

//ESC Control(TEGRA -> WCB)
//	may not need now with TEGRA interface board.
tCAN ESC1;
tCAN ESC2;
tCAN ESC3;
tCAN ESC4;
tCAN ESC5;
tCAN ESC6;
tCAN ESC7;
tCAN ESC8;

tCAN Request_Response_SIB_Pressure;
tCAN Request_Response_SIB_Temp;
tCAN Request_Response_HYDRAS;
tCAN Request_Response_HYDRAS_RAW;
tCAN Request_Response_ESC1;
tCAN Request_Response_ESC2;
tCAN Request_Response_ESC3;
tCAN Request_Response_ESC4;
tCAN Request_Response_ESC5;
tCAN Request_Response_ESC6;
tCAN Request_Response_ESC7;
tCAN Request_Response_ESC8;
tCAN Request_Response_Weapon_status;
tCAN Request_Response_Backplane_Current;

tCAN Read_Request_SIB_Pressure;
tCAN Read_Request_SIB_Temp;
tCAN Read_Request_HYDRAS;
tCAN Read_Request_HYDRAS_RAW;
tCAN Read_Request_ESC1;
tCAN Read_Request_ESC2;
tCAN Read_Request_ESC3;
tCAN Read_Request_ESC4;
tCAN Read_Request_ESC5;
tCAN Read_Request_ESC6;
tCAN Read_Request_ESC7;
tCAN Read_Request_ESC8;
tCAN Read_Request_Weapon_status;
tCAN Read_Request_Backplane_Current;
