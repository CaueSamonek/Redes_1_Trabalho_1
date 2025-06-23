#ifndef ENUM_MESSAGE_TYPE_HPP
#define ENUM_MESSAGE_TYPE_HPP

namespace cat {

enum EnumMessageType : unsigned char {
	ACK,
	NACK,
	OK,
	HANDSHAKE,
	SIZE,
	DATA,
	TEXT,
	VIDEO,
	IMAGE,
	END_OF_FILE,
	MOVE_RIGHT,
	MOVE_UP,
	MOVE_DOWN,
	MOVE_LEFT,
	INIT,
	ERROR
};

}

#endif //ENUM_MESSAGE_TYPE_HPP
