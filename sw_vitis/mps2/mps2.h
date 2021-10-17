/* for PMS 2.0 */
/* test coomit 2 */

/* structure of massages */
struct _msg {
	int num;	//number of messages
	int code; 	//code command
	int size;	//size of massive parameters
	int data[];	//parameters
};

/* command message 	[#][Command_Code][size][parameters] */

/* answer message 	[#][Answer_Code][size][parameters]
					[#][ACK][0] - OK set
					[#][ACK][i][return value[i]] - OK get
					[#][ERR][1][err code] - error
*/

/* abbreviation
I			current A/ka
V			voltage V/va
T			temperature C/kc
OUT_MAIN	high current output connector
OUT_SHIM	shim connector sh0 - sh17
PSH			permanent switch heater
R_DS		resistor dissipation stage r0-r3
PM			power module pm0-pm7
PS			power supply ps0-ps3
RELAY		high current relay
CB			control board
SP			set point value
CUR			current value
*/

/* command codes */
typedef enum {
	// main
	GET_STATE,
	SET_REGIME,	 	//[Set_Regime][1][Regime]
	GET_REGIME,		//[GET_REGIME][0]
	GET_OUT_V,		//[GET_OUT_V][0] - output voltages of amphenol connector
	GET_OUT_I,  	//[GET_OUT_I][0] - output current of amphenol connector
	SET_T_MAX_VICHI,
	GET_T_MAX_VICHI,
	SET_T_MAX_VICOR,
	GET_T_MAX_VICOR,
	SET_T_MAX_R_DS,
	GET_T_max_R_DS,
	// remp UP
	SET_V, //[SET_V][1][V]
	GET_V,
	GET_I,
	GET_T, // answer array [12]
	// Power module
	SET_V_PM_SP, 	//[SET_V_PM_SP][7][[i][V0]...[V5]] - i numbers of PM and V0-V5 numbers of vishi ic
	GET_V_PM_SP, 	//[GET_V_PM_SP][1][i] - i numbers of PM 
	GET_V_PM_CUR, 	//[GET_V_PM_CUR][1][i] - i numbers of PM 	
} Command_Codes;

/* regimes */
typedef enum {OFF=0, RAMP_UP=1, RAMP_DOWN=2, SHIM=3, STANDBY=4, EMERGENCY=5, ADVANCED=6} Regime;

/* answer code */
typedef enum  {
	ACK = 777,
	ERR = 666,
} Answer_Code;

/* error code */
typedef enum {
	E_CODE,
	E_DATA,
	PARAM_OUT_RANGES,
	ILLEGAL_CR,
} Error_Code; 	
	


