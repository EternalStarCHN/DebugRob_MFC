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
  NO_ACTION = 0,                //0x00 无动作

  /* Servo commands */
  LEFT_HAND_UP = 9,             //0x09 举左手
  HEAD_MOVE = 10,               //0x0A 转头
  RIGHT_HAND_UP = 12,           //0x0C 举右手
  HAND_UP = 13,                 //0x0D 举双手
  /* Callback */
  ACK = 8                       //0x08 需要下位机反馈
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