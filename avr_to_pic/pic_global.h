/**
Modified from Ryan Young's

GPIO commands that he found to make it more legible,
modified for pic.
**/
#ifndef GLOBAL_H
#define GLOBAL_H

#define true 1
#define false 0

#define True 1
#define False 0

#define SET_L(x)     _XRS(x)
#define SET_H(x)     _XS(x)
//#define TOGGLE(X)    _XT(x)
#define SET_INPUT(x) _XSO(x)
#define SET_OUTPUT(x) _XSI(x)
#define IS_SET(x,y)    _XR(x,y)

#define bit_is_set(x,y)   (x & (1<<y))  //port2? wtf is this shit
#define bit_is_clear(x,y)   !(x & (1<<y))
//#define DDR(x)      _ddr2(x)
//#define PIN(x)      _pin2(x)

#define _bit_is_zero(x, y) LAT##y##bits.LAT##y##x == 0
#define _bit_is_one(x, y) LAT##y##bits.LAT##y##x == 0
#define _XRS(y,x) LAT##y##bits.LAT##y##x = 0
#define _XS(y,x)  LAT##y##bits.LAT##y##x = 1
//#define _XT(x,y)  PIN(x) |= (1<<y) //wtf is this
#define _XSO(y,x) TRIS##y##bits.TRIS##y##x = 0 //set for input
#define _XSI(y,x) TRIS##y##bits.TRIS##y##x = 1 //set for output

#define _XR(x,y) ((PIN(x) & (1<<y)) != 0)

//#define _RPO (x,y,z) RPOR(x)bits.RP(y)R = (z)
//#define _port2(x) PORT ## x
//#define _ddr2(x) DDR ## x
//#define _pin2(x) PIN ## x

#endif
