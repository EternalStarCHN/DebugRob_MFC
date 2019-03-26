#ifndef DEBUG_ROB_H
#define DEBUG_ROB_H
/* exact-width signed integer types */
typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;

/* exact-width unsigned integer types */
typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;

typedef enum pointMode__  : uint8_t
{
  /* Default */
  POINT_DEFAULT = 0,

  /* Servo commands */
  HAND_UP,
  HEAD_MOVE,

  /* Motor Turn Commands */
  TURN_RIGHT,
  TURN_LEFT,
  TURN_BACK,
  TURN_ANYANGLE,

  /* Commands over */
  OVER_,
  QUIT_
} PointMode_t;

namespace Debug
{

class DebugRob
{
public:
	DebugRob() {}

	/* 0xFF 0xhh 0xhh 0xAA */
	static int  ReceiveCommand(void);
	static void UpdateReceiveTag(void);
private:
	static int _receiveTag;
};

}


#endif DEBUG_ROB_H