#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "EnumMessageType.hpp"
#include <vector>

namespace cat{

class Message {
  public:
	static constexpr unsigned char MAX_BYTE_LENGTH {131};
	static constexpr unsigned char MIN_BYTE_LENGTH {14};
	static constexpr unsigned char START_CODE {0b01111110}; 
	static constexpr unsigned char DATA_BYTE_MAX_LENGTH {127};
	static constexpr unsigned char DATA_BYTE_OFFSET {4};//8+7+5+4+8 / 8
	static constexpr unsigned char SEQUENCE_BIT_LENGTH {5};

	Message();
	Message(const std::vector<unsigned char> message);
	Message(unsigned char sequence, EnumMessageType type,
										const std::vector<unsigned char> data={});
	virtual ~Message()=default;
	unsigned char getStartCode();
	unsigned char getSize();
	unsigned char getSequence();
	unsigned char getType();
	unsigned char getChecksum();
	std::vector<unsigned char> getData();

	unsigned char calculate_checksum();
	bool verify();//faz checksum e compara com o checksum da mensgem
	void print();

	operator unsigned char*();
	operator std::vector<unsigned char>();
	operator std::vector<unsigned char>&();

	private:
		void setData(const std::vector<unsigned char> data);
		std::vector<unsigned char> message;
};

}

#endif //MESSAGE_HPP
