#ifndef _GPBENCODE_H_
#define _GPBENCODE_H_
#include <string>
#include <sstream>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

#define INL inline
#define field_number int field_number_arg
#define output uint8* target

namespace gpb{

typedef signed char  int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

namespace io{

	static const int kMaxVarintBytes = 10;
	static const int kMaxVarint32Bytes = 5;
	class CodedInputStream
	{
	public:
		explicit CodedInputStream(const uint8* buffer, int size)
			: buffer_(buffer),
			buffer_end_(buffer + size)
		{

		}
		~CodedInputStream(){}

		inline void CodedInputStream::Advance(int amount) {
			buffer_ += amount;
		}
		inline int CodedInputStream::BufferSize() const {
			return buffer_end_ - buffer_;
		}

		bool ReadVarint32(uint32* value){
			if(BufferSize() <= 0)
				return false;
			uint32 first_byte = *buffer_;
			if (first_byte < 0x80) {
				*value = first_byte;
				Advance(1);
				return true;
			}
			const uint8* ptr = buffer_;
			uint32 b;
			uint32 result = first_byte - 0x80;
			++ptr;  // We just processed the first byte.  Move on to the second.
			b = *(ptr++); result += b <<  7; if (!(b & 0x80)) goto done;
			result -= 0x80 << 7;
			b = *(ptr++); result += b << 14; if (!(b & 0x80)) goto done;
			result -= 0x80 << 14;
			b = *(ptr++); result += b << 21; if (!(b & 0x80)) goto done;
			result -= 0x80 << 21;
			b = *(ptr++); result += b << 28; if (!(b & 0x80)) goto done;
			// "result -= 0x80 << 28" is irrevelant.

			// If the input is larger than 32 bits, we still need to read it all
			// and discard the high-order bits.
			for (int i = 0; i < kMaxVarintBytes - kMaxVarint32Bytes; i++) {
				b = *(ptr++); if (!(b & 0x80)) goto done;
			}

			// We have overrun the maximum size of a varint (10 bytes).  Assume
			// the data is corrupt.
			return false;

done:
			*value = result;
			Advance(ptr - buffer_);
			return true;
		}
		bool ReadVarint64(uint64* value){
			if(BufferSize() <= 0)
				return false;
			if (*buffer_ < 0x80) {
				*value = *buffer_;
				Advance(1);
				return true;
			}
			if (BufferSize() >= kMaxVarintBytes ||
				// Optimization:  We're also safe if the buffer is non-empty and it ends
					// with a byte that would terminate a varint.
						(buffer_end_ > buffer_ && !(buffer_end_[-1] & 0x80))) {
							// Fast path:  We have enough bytes left in the buffer to guarantee that
							// this read won't cross the end, so we can skip the checks.

							const uint8* ptr = buffer_;
							uint32 b;

							// Splitting into 32-bit pieces gives better performance on 32-bit
							// processors.
							uint32 part0 = 0, part1 = 0, part2 = 0;

							b = *(ptr++); part0  = b      ; if (!(b & 0x80)) goto done;
							part0 -= 0x80;
							b = *(ptr++); part0 += b <<  7; if (!(b & 0x80)) goto done;
							part0 -= 0x80 << 7;
							b = *(ptr++); part0 += b << 14; if (!(b & 0x80)) goto done;
							part0 -= 0x80 << 14;
							b = *(ptr++); part0 += b << 21; if (!(b & 0x80)) goto done;
							part0 -= 0x80 << 21;
							b = *(ptr++); part1  = b      ; if (!(b & 0x80)) goto done;
							part1 -= 0x80;
							b = *(ptr++); part1 += b <<  7; if (!(b & 0x80)) goto done;
							part1 -= 0x80 << 7;
							b = *(ptr++); part1 += b << 14; if (!(b & 0x80)) goto done;
							part1 -= 0x80 << 14;
							b = *(ptr++); part1 += b << 21; if (!(b & 0x80)) goto done;
							part1 -= 0x80 << 21;
							b = *(ptr++); part2  = b      ; if (!(b & 0x80)) goto done;
							part2 -= 0x80;
							b = *(ptr++); part2 += b <<  7; if (!(b & 0x80)) goto done;
							// "part2 -= 0x80 << 7" is irrelevant because (0x80 << 7) << 56 is 0.

							// We have overrun the maximum size of a varint (10 bytes).  The data
							// must be corrupt.
							return false;

done:
							Advance(ptr - buffer_);
							*value = (static_cast<uint64>(part0)) |
								(static_cast<uint64>(part1) << 28) |
								(static_cast<uint64>(part2) << 56);
							return true;
			} else {
				return false;
			}
		}
		// Read a 32-bit little-endian integer.
		bool ReadLittleEndian32(uint32* value){
			if(BufferSize() < sizeof(*value))
				return false;
#if defined(PROTOBUF_LITTLE_ENDIAN)
			memcpy(value, buffer_, sizeof(*value));
			Advance(sizeof(*value));
			return true;
#else
			*value = (static_cast<uint32>(buffer_[0])      ) |
				(static_cast<uint32>(buffer_[1]) <<  8) |
				(static_cast<uint32>(buffer_[2]) << 16) |
				(static_cast<uint32>(buffer_[3]) << 24);
			Advance(sizeof(*value));
			return true;
#endif
		}
		// Read a 64-bit little-endian integer.
		bool ReadLittleEndian64(uint64* value){
			if(BufferSize() < sizeof(*value))
				return false;
#if defined(PROTOBUF_LITTLE_ENDIAN)
			memcpy(value, buffer_, sizeof(*value));
			Advance(sizeof(*value));
			return true;
#else
			uint32 part0 = (static_cast<uint32>(buffer[0])      ) |
				(static_cast<uint32>(buffer_[1]) <<  8) |
				(static_cast<uint32>(buffer_[2]) << 16) |
				(static_cast<uint32>(buffer_[3]) << 24);
			uint32 part1 = (static_cast<uint32>(buffer_[4])      ) |
				(static_cast<uint32>(buffer_[5]) <<  8) |
				(static_cast<uint32>(buffer_[6]) << 16) |
				(static_cast<uint32>(buffer_[7]) << 24);
			*value = static_cast<uint64>(part0) |
				(static_cast<uint64>(part1) << 32);
			Advance(sizeof(*value));
			return true;
#endif
		}
		bool ReadString(string* buffer, int size){
			if (buffer == NULL || size < 0) return false;  // security: size is often user-supplied
			if(BufferSize() < size)
				return false;
			buffer->resize(size);
			char* data = buffer->empty() ? NULL : &*buffer->begin();
			if(data == NULL) return false;
			memcpy(data, buffer_, size);
			Advance(size);
			return true;
		}
		bool ReadArray(char* buffer, int size){
			if (size < 0) return false;  // security: size is often user-supplied
			if(BufferSize() < size)
				return false;
			buffer = new char[size+1];
			memset(buffer, 0, size+1);
			if(buffer == NULL) return false;
			memcpy(buffer, buffer_, size);
			Advance(size);
			return true;
		}

	private:
		const uint8* buffer_;
		const uint8* buffer_end_;     // pointer to the end of the buffer.
	};

class CodedOutputStream
{
public:
	CodedOutputStream(){}
	~CodedOutputStream(){}

	INL static uint8* WriteVarint32ToArray(uint32 value, uint8* target)
	{
		while (value >= 0x80) {
			*target = static_cast<uint8>(value | 0x80);
			value >>= 7;
			++target;
		}
		*target = static_cast<uint8>(value);
		return target + 1;
	}
	INL static uint8* WriteVarint64ToArrayInline(uint64 value, uint8* target)
	{
		// Splitting into 32-bit pieces gives better performance on 32-bit
		// processors.
		uint32 part0 = static_cast<uint32>(value      );
		uint32 part1 = static_cast<uint32>(value >> 28);
		uint32 part2 = static_cast<uint32>(value >> 56);

		int size;

		// Here we can't really optimize for small numbers, since the value is
		// split into three parts.  Cheking for numbers < 128, for instance,
		// would require three comparisons, since you'd have to make sure part1
		// and part2 are zero.  However, if the caller is using 64-bit integers,
		// it is likely that they expect the numbers to often be very large, so
		// we probably don't want to optimize for small numbers anyway.  Thus,
		// we end up with a hardcoded binary search tree...
		if (part2 == 0) {
			if (part1 == 0) {
				if (part0 < (1 << 14)) {
					if (part0 < (1 << 7)) {
						size = 1; goto size1;
					} else {
						size = 2; goto size2;
					}
				} else {
					if (part0 < (1 << 21)) {
						size = 3; goto size3;
					} else {
						size = 4; goto size4;
					}
				}
			} else {
				if (part1 < (1 << 14)) {
					if (part1 < (1 << 7)) {
						size = 5; goto size5;
					} else {
						size = 6; goto size6;
					}
				} else {
					if (part1 < (1 << 21)) {
						size = 7; goto size7;
					} else {
						size = 8; goto size8;
					}
				}
			}
		} else {
			if (part2 < (1 << 7)) {
				size = 9; goto size9;
			} else {
				size = 10; goto size10;
			}
		}

		//GOOGLE_LOG(FATAL) << "Can't get here.";

size10: target[9] = static_cast<uint8>((part2 >>  7) | 0x80);
size9 : target[8] = static_cast<uint8>((part2      ) | 0x80);
size8 : target[7] = static_cast<uint8>((part1 >> 21) | 0x80);
size7 : target[6] = static_cast<uint8>((part1 >> 14) | 0x80);
size6 : target[5] = static_cast<uint8>((part1 >>  7) | 0x80);
size5 : target[4] = static_cast<uint8>((part1      ) | 0x80);
size4 : target[3] = static_cast<uint8>((part0 >> 21) | 0x80);
size3 : target[2] = static_cast<uint8>((part0 >> 14) | 0x80);
size2 : target[1] = static_cast<uint8>((part0 >>  7) | 0x80);
size1 : target[0] = static_cast<uint8>((part0      ) | 0x80);

		target[size-1] &= 0x7F;
		return target + size;
	}
	INL static uint8* WriteVarint32SignExtendedToArray(int32 value, uint8* target)
	{
		if (value < 0) {
			return WriteVarint64ToArrayInline(static_cast<uint64>(value), target);
		} else {
			return WriteVarint32ToArray(static_cast<uint32>(value), target);
		}
	}
	INL static uint8* WriteLittleEndian32ToArray(uint32 value, uint8* target)
	{
#if defined(PROTOBUF_LITTLE_ENDIAN)
		memcpy(target, &value, sizeof(value));
#else
		target[0] = static_cast<uint8>(value);
		target[1] = static_cast<uint8>(value >>  8);
		target[2] = static_cast<uint8>(value >> 16);
		target[3] = static_cast<uint8>(value >> 24);
#endif
		return target + sizeof(value);
	}
	INL static uint8* WriteLittleEndian64ToArray(uint64 value, uint8* target)
	{
#if defined(PROTOBUF_LITTLE_ENDIAN)
		memcpy(target, &value, sizeof(value));
#else
		uint32 part0 = static_cast<uint32>(value);
		uint32 part1 = static_cast<uint32>(value >> 32);

		target[0] = static_cast<uint8>(part0);
		target[1] = static_cast<uint8>(part0 >>  8);
		target[2] = static_cast<uint8>(part0 >> 16);
		target[3] = static_cast<uint8>(part0 >> 24);
		target[4] = static_cast<uint8>(part1);
		target[5] = static_cast<uint8>(part1 >>  8);
		target[6] = static_cast<uint8>(part1 >> 16);
		target[7] = static_cast<uint8>(part1 >> 24);
#endif
		return target + sizeof(value);
	}
	INL static uint8* WriteRawToArray(const void* buffer, int size, uint8* target)
	{
		memcpy(target, buffer, size);
		return target + size;
	}
};
}

#define MakeTag(FIELD_NUMBER, TYPE)                  \
	static_cast<uint32>(                             \
	((FIELD_NUMBER) << 3) \
	| (TYPE))

class WireFormatLite
{
public:
	WireFormatLite(){}
	~WireFormatLite(){}

	enum WireType {
		WIRETYPE_VARINT           = 0,
		WIRETYPE_FIXED64          = 1,
		WIRETYPE_LENGTH_DELIMITED = 2,
		WIRETYPE_START_GROUP      = 3,
		WIRETYPE_END_GROUP        = 4,
		WIRETYPE_FIXED32          = 5,
	};

	// Number of bits in a tag which identify the wire type.
	static const int kTagTypeBits = 3;
	// Mask for those bits.
	static const uint32 kTagTypeMask = (1 << kTagTypeBits) - 1;

	inline static WireType WireFormatLite::GetTagWireType(uint32 tag) {
		return static_cast<WireType>(tag & kTagTypeMask);
	}
	inline static int GetTagFieldNumber(uint32 tag) {
		return static_cast<int>(tag >> kTagTypeBits);
	}

	// ZigZag Transform:  Encodes signed integers so that they can be
	// effectively used with varint encoding.
	//
	// varint operates on unsigned integers, encoding smaller numbers into
	// fewer bytes.  If you try to use it on a signed integer, it will treat
	// this number as a very large unsigned integer, which means that even
	// small signed numbers like -1 will take the maximum number of bytes
	// (10) to encode.  ZigZagEncode() maps signed integers to unsigned
	// in such a way that those with a small absolute value will have smaller
	// encoded values, making them appropriate for encoding using varint.
	//
	//       int32 ->     uint32
	// -------------------------
	//           0 ->          0
	//          -1 ->          1
	//           1 ->          2
	//          -2 ->          3
	//         ... ->        ...
	//  2147483647 -> 4294967294
	// -2147483648 -> 4294967295
	//
	//        >> encode >>
	//        << decode <<

	static int32  ZigZagDecode32(uint32 n){
		return (n >> 1) ^ -static_cast<int32>(n & 1);
	}
	static int64  ZigZagDecode64(uint64 n){
		return (n >> 1) ^ -static_cast<int64>(n & 1);
	}
	static float DecodeFloat(uint32 value){
		union {float f; uint32 i;};
		i = value;
		return f;
	}
	static double DecodeDouble(uint64 value){
		union {double f; uint64 i;};
		i = value;
		return f;
	}

	// Lite alternative to FieldDescriptor::Type.  Must be kept in sync.
	enum FieldType {
		TYPE_DOUBLE         = 1,
		TYPE_FLOAT          = 2,
		TYPE_INT64          = 3,
		TYPE_UINT64         = 4,
		TYPE_INT32          = 5,
		TYPE_FIXED64        = 6,
		TYPE_FIXED32        = 7,
		TYPE_BOOL           = 8,
		TYPE_STRING         = 9,
		TYPE_GROUP          = 10,
		TYPE_MESSAGE        = 11,
		TYPE_BYTES          = 12,
		TYPE_UINT32         = 13,
		TYPE_ENUM           = 14,
		TYPE_SFIXED32       = 15,
		TYPE_SFIXED64       = 16,
		TYPE_SINT32         = 17,
		TYPE_SINT64         = 18,
		MAX_FIELD_TYPE      = 18,
	};

	template <typename CType, enum FieldType DeclaredType> inline
		static bool ReadPrimitive(io::CodedInputStream* input, CType* value);
	template <>
	inline static bool ReadPrimitive<int32, WireFormatLite::TYPE_INT32>(
		io::CodedInputStream* input,
		int32* value) {
			uint32 temp;
			if (!input->ReadVarint32(&temp)) return false;
			*value = static_cast<int32>(temp);
			return true;
	}
	template <>
	inline static bool ReadPrimitive<int64, WireFormatLite::TYPE_INT64>(
		io::CodedInputStream* input,
		int64* value) {
			uint64 temp;
			if (!input->ReadVarint64(&temp)) return false;
			*value = static_cast<int64>(temp);
			return true;
	}
	template <>
	inline static bool ReadPrimitive<uint32, WireFormatLite::TYPE_UINT32>(
		io::CodedInputStream* input,
		uint32* value) {
			return input->ReadVarint32(value);
	}
	template <>
	inline static bool ReadPrimitive<uint64, WireFormatLite::TYPE_UINT64>(
		io::CodedInputStream* input,
		uint64* value) {
			return input->ReadVarint64(value);
	}
	template <>
	inline static bool ReadPrimitive<int32, WireFormatLite::TYPE_SINT32>(
		io::CodedInputStream* input,
		int32* value) {
			uint32 temp;
			if (!input->ReadVarint32(&temp)) return false;
			*value = ZigZagDecode32(temp);
			return true;
	}
	template <>
	inline static bool ReadPrimitive<int64, WireFormatLite::TYPE_SINT64>(
		io::CodedInputStream* input,
		int64* value) {
			uint64 temp;
			if (!input->ReadVarint64(&temp)) return false;
			*value = ZigZagDecode64(temp);
			return true;
	}
	template <>
	inline static bool ReadPrimitive<uint32, WireFormatLite::TYPE_FIXED32>(
		io::CodedInputStream* input,
		uint32* value) {
			return input->ReadLittleEndian32(value);
	}
	template <>
	inline static bool ReadPrimitive<uint64, WireFormatLite::TYPE_FIXED64>(
		io::CodedInputStream* input,
		uint64* value) {
			return input->ReadLittleEndian64(value);
	}
	template <>
	inline static bool ReadPrimitive<int32, WireFormatLite::TYPE_SFIXED32>(
		io::CodedInputStream* input,
		int32* value) {
			uint32 temp;
			if (!input->ReadLittleEndian32(&temp)) return false;
			*value = static_cast<int32>(temp);
			return true;
	}
	template <>
	inline static bool ReadPrimitive<int64, WireFormatLite::TYPE_SFIXED64>(
		io::CodedInputStream* input,
		int64* value) {
			uint64 temp;
			if (!input->ReadLittleEndian64(&temp)) return false;
			*value = static_cast<int64>(temp);
			return true;
	}
	template <>
	inline static bool ReadPrimitive<float, WireFormatLite::TYPE_FLOAT>(
		io::CodedInputStream* input,
		float* value) {
			uint32 temp;
			if (!input->ReadLittleEndian32(&temp)) return false;
			*value = DecodeFloat(temp);
			return true;
	}
	template <>
	inline static bool ReadPrimitive<double, WireFormatLite::TYPE_DOUBLE>(
		io::CodedInputStream* input,
		double* value) {
			uint64 temp;
			if (!input->ReadLittleEndian64(&temp)) return false;
			*value = DecodeDouble(temp);
			return true;
	}
	template <>
	inline static bool ReadPrimitive<bool, WireFormatLite::TYPE_BOOL>(
		io::CodedInputStream* input,
		bool* value) {
			uint64 temp;
			if (!input->ReadVarint64(&temp)) return false;
			*value = temp != 0;
			return true;
	}

	inline static bool ReadBytesToString(io::CodedInputStream* input,
		string* value) {
			uint32 length;
			return input->ReadVarint32(&length) &&
				input->ReadString(value, length);
	}

	INL static uint32 ZigZagEncode32(int32 n){
		// Note:  the right-shift must be arithmetic
		return (static_cast<uint32>(n) << 1) ^ (n >> 31);
	}
	INL static uint64 ZigZagEncode64(int64 n){
		// Note:  the right-shift must be arithmetic
		return (static_cast<uint64>(n) << 1) ^ (n >> 63);
	}
	INL static uint32 EncodeFloat(float value){
		union {float f; uint32 i;};
		f = value;
		return i;
	}
	INL static uint64 EncodeDouble(double value){
		union {double f; uint64 i;};
		f = value;
		return i;
	}

	INL static uint8* WriteTagToArray(field_number, WireType type, output){
		return io::CodedOutputStream::WriteVarint32ToArray(MakeTag(field_number_arg, type),target);
	}

	INL static uint8* WriteInt32ToArray(field_number, int32 value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_VARINT, target);
		return io::CodedOutputStream::WriteVarint32SignExtendedToArray(value, target);
	}
	INL static uint8* WriteInt64ToArray(field_number, int64 value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_VARINT, target);
		return io::CodedOutputStream::WriteVarint64ToArrayInline(value, target);
	}
	INL static uint8* WriteUInt32ToArray(field_number, uint32 value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_VARINT, target);
		return io::CodedOutputStream::WriteVarint32ToArray(value, target);
	}
	INL static uint8* WriteUInt64ToArray(field_number, uint64 value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_VARINT, target);
		return io::CodedOutputStream::WriteVarint64ToArrayInline(value, target);
	}
	INL static uint8* WriteSInt32ToArray(field_number, int32 value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_VARINT, target);
		return io::CodedOutputStream::WriteVarint32ToArray(ZigZagEncode32(value), target);
	}
	INL static uint8* WriteSInt64ToArray(field_number, int64 value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_VARINT, target);
		return io::CodedOutputStream::WriteVarint64ToArrayInline(ZigZagEncode64(value), target);
	}
	INL static uint8* WriteFixed32ToArray(field_number, uint32 value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_FIXED32, target);
		return io::CodedOutputStream::WriteLittleEndian32ToArray(value, target);
	}
	INL static uint8* WriteFixed64ToArray(field_number, uint64 value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_FIXED64, target);
		return io::CodedOutputStream::WriteLittleEndian64ToArray(value, target);
	}
	INL static uint8* WriteSFixed32ToArray(field_number, int32 value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_FIXED32, target);
		return io::CodedOutputStream::WriteLittleEndian32ToArray(static_cast<uint32>(value), target);
	}
	INL static uint8* WriteSFixed64ToArray(field_number, int64 value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_FIXED64, target);
		return io::CodedOutputStream::WriteLittleEndian64ToArray(static_cast<uint64>(value), target);
	}
	INL static uint8* WriteFloatToArray(field_number, float value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_FIXED32, target);
		return io::CodedOutputStream::WriteLittleEndian32ToArray(EncodeFloat(value), target);
	}
	INL static uint8* WriteDoubleToArray(field_number, double value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_FIXED64, target);
		return io::CodedOutputStream::WriteLittleEndian64ToArray(EncodeDouble(value), target);
	}
	INL static uint8* WriteBoolToArray(field_number, bool value, output){
		target = WriteTagToArray(field_number_arg, WIRETYPE_VARINT, target);
		return io::CodedOutputStream::WriteVarint32ToArray(value ? 1 : 0, target);
	}

	INL static uint8* WriteStringToArray(
		field_number, const string& value, output){
			// String is for UTF-8 text only
			// WARNING:  In wire_format.cc, both strings and bytes are handled by
			//   WriteString() to avoid code duplication.  If the implementations become
			//   different, you will need to update that usage.
			target = WriteTagToArray(field_number_arg, WIRETYPE_LENGTH_DELIMITED, target);
			target = io::CodedOutputStream::WriteVarint32ToArray(value.size(), target);
			return io::CodedOutputStream::WriteRawToArray(value.data(), static_cast<int>(value.size()), target);
	}
	INL static uint8* WriteMessageToArray(
		field_number, const char* value, int size, output){
			target = WriteTagToArray(field_number_arg, WIRETYPE_LENGTH_DELIMITED, target);
			target = io::CodedOutputStream::WriteVarint32ToArray(size, target);
			return io::CodedOutputStream::WriteRawToArray(value, size, target);
	}
};

class gpbmsg
{
public:
	gpbmsg(){}
	virtual ~gpbmsg(){}

public:
	virtual int encode(uint8* buffer)=0;
	virtual bool decode(const uint8* buffer, int len)=0;
};

class gpbdecoder
{
	//only sint32 int64 uint32 uint64 float double bool string
	//no int32 sint64 fixed32 fixed64
public:
	gpbdecoder(){}
	~gpbdecoder(){}

public:
	template <typename CType>
	inline static bool ReadPrimitive(io::CodedInputStream* input, CType* value){
		string msgstr;
		if(!WireFormatLite::ReadBytesToString(input, &msgstr)) return false;
		gpbmsg* pmsg =(gpbmsg*)value;
		return pmsg->decode((const uint8*)msgstr.c_str(), msgstr.size());
	}
	template <>
	inline static bool ReadPrimitive(io::CodedInputStream* input, int32* value){
		//return WireFormatLite::ReadPrimitive<int32, WireFormatLite::TYPE_INT32>(input, value);
		return WireFormatLite::ReadPrimitive<int32, WireFormatLite::TYPE_SINT32>(input, value);
	}
	template <>
	inline static bool ReadPrimitive(io::CodedInputStream* input, int64* value){
		return WireFormatLite::ReadPrimitive<int64, WireFormatLite::TYPE_INT64>(input, value);
	}
	template <>
	inline static bool ReadPrimitive(io::CodedInputStream* input, uint32* value){
		return WireFormatLite::ReadPrimitive<uint32, WireFormatLite::TYPE_UINT32>(input, value);
	}
	template <>
	inline static bool ReadPrimitive(io::CodedInputStream* input, uint64* value){
		return WireFormatLite::ReadPrimitive<uint64, WireFormatLite::TYPE_UINT64>(input, value);
	}
	template <>
	inline static bool ReadPrimitive(io::CodedInputStream* input, float* value){
		return WireFormatLite::ReadPrimitive<float, WireFormatLite::TYPE_FLOAT>(input, value);
	}
	template <>
	inline static bool ReadPrimitive(io::CodedInputStream* input, double* value){
		return WireFormatLite::ReadPrimitive<double, WireFormatLite::TYPE_DOUBLE>(input, value);
	}
	template <>
	inline static bool ReadPrimitive(io::CodedInputStream* input, bool* value){
		return WireFormatLite::ReadPrimitive<bool, WireFormatLite::TYPE_BOOL>(input, value);
	}
	template <>
	inline static bool ReadPrimitive(io::CodedInputStream* input, string* value){
		return WireFormatLite::ReadBytesToString(input, value);
	}

	template <typename CType>
	inline static bool ReadRepeatedPrimitive(io::CodedInputStream* input, vector<CType>& value){
		uint32 length;
		if(!input->ReadVarint32(&length)) return false;
		for(uint32 i = 0; i < length; ++i){
			CType val;
			if(!ReadPrimitive(input, &val)) return false;
			value.push_back(val);
		}
		return true;
	}
};

class gpbencoder
	//only sint32 int64 uint32 uint64 float double bool string
	//no int32 sint64 fixed32 fixed64
{
public:
	gpbencoder(){}
	~gpbencoder(){}

public:
	template<typename Type>
	INL static uint8* WriteDataToArray(field_number, Type value, output){
		uint8 buf[1024];
		int len = value.encode(buf);
		return WireFormatLite::WriteMessageToArray(field_number_arg, (const char*)buf, len, target);
	}
	template<>
	INL static uint8* WriteDataToArray(field_number, int32 value, output){
		//return WireFormatLite::WriteInt32ToArray(field_number_arg, value, target);
		return WireFormatLite::WriteSInt32ToArray(field_number_arg, value, target);
	}
	template<>
	INL static uint8* WriteDataToArray(field_number, int64 value, output){
		return WireFormatLite::WriteInt64ToArray(field_number_arg, value, target);
	}
	template<>
	INL static uint8* WriteDataToArray(field_number, uint32 value, output){
		return WireFormatLite::WriteUInt32ToArray(field_number_arg, value, target);
	}
	template<>
	INL static uint8* WriteDataToArray(field_number, uint64 value, output){
		return WireFormatLite::WriteUInt64ToArray(field_number_arg, value, target);
	}
	template<>
	INL static uint8* WriteDataToArray(field_number, float value, output){
		return WireFormatLite::WriteFloatToArray(field_number_arg, value, target);
	}
	template<>
	INL static uint8* WriteDataToArray(field_number, double value, output){
		return WireFormatLite::WriteDoubleToArray(field_number_arg, value, target);
	}
	template<>
	INL static uint8* WriteDataToArray(field_number, bool value, output){
		return WireFormatLite::WriteBoolToArray(field_number_arg, value, target);
	}
	template<>
	INL static uint8* WriteDataToArray(field_number, string value, output){
		return WireFormatLite::WriteStringToArray(field_number_arg, value, target);
	}

	template<typename Type>
	INL static uint8* WriteNoTagDataToArray(Type value, output);
	template<>
	INL static uint8* WriteNoTagDataToArray(int32 value, output){
		//return io::CodedOutputStream::WriteVarint32SignExtendedToArray(value, target);
		return io::CodedOutputStream::WriteVarint32ToArray(WireFormatLite::ZigZagEncode32(value), target);
	}
	template<>
	INL static uint8* WriteNoTagDataToArray(int64 value, output){
		return io::CodedOutputStream::WriteVarint64ToArrayInline(value, target);
	}
	template<>
	INL static uint8* WriteNoTagDataToArray(uint32 value, output){
		 return io::CodedOutputStream::WriteVarint32ToArray(value, target);
	}
	template<>
	INL static uint8* WriteNoTagDataToArray(uint64 value, output){
		 return io::CodedOutputStream::WriteVarint64ToArrayInline(value, target);
	}
	template<>
	INL static uint8* WriteNoTagDataToArray(float value, output){
		return io::CodedOutputStream::WriteLittleEndian32ToArray(WireFormatLite::EncodeFloat(value), target);
	}
	template<>
	INL static uint8* WriteNoTagDataToArray(double value, output){
		return io::CodedOutputStream::WriteLittleEndian64ToArray(WireFormatLite::EncodeDouble(value), target);
	}
	template<>
	INL static uint8* WriteNoTagDataToArray(bool value, output){
		return io::CodedOutputStream::WriteVarint32ToArray(value ? 1 : 0, target);
	}
	template<>
	INL static uint8* WriteNoTagDataToArray(string value, output){
		target = io::CodedOutputStream::WriteVarint32ToArray(value.size(), target);
		return io::CodedOutputStream::WriteRawToArray(value.data(), static_cast<int>(value.size()), target);
	}

	template<typename Type>
	INL static uint8* WriteRepeatedToArray(field_number, vector<Type>& value, output){//for simple type, it first write tag(len limit),then write all_data_size(no type tag),finally write the simple data one by one(no tag);
		int count = (int)value.size();
		if (count > 0){
			uint8* tempbuf = new uint8[count*sizeof(Type)*2];
			if(tempbuf == NULL)
				return target;
			uint8* newbuf = tempbuf;
			for(int i = 0; i < count; ++i){
				newbuf = WriteNoTagDataToArray(value[i], newbuf);
			}
			int _lens_cached_byte_size_ = newbuf - tempbuf;
			target = WireFormatLite::WriteTagToArray(field_number_arg, WireFormatLite::WIRETYPE_LENGTH_DELIMITED, target);
			target = io::CodedOutputStream::WriteVarint32ToArray(_lens_cached_byte_size_, target);
			target = io::CodedOutputStream::WriteRawToArray(tempbuf, _lens_cached_byte_size_, target);
			delete[] tempbuf;
		}
		return target;
	}
	template<>//for string or message, it write the string or message one by one directly, not together.
	INL static uint8* WriteRepeatedToArray(field_number, vector<string>& value, output){
		int count = (int)value.size();
		for(int i = 0; i < count; ++i){
			target = WireFormatLite::WriteStringToArray(field_number_arg, value[i], target);
		}
		return target;
	}

	template<typename TKey, typename TValue>
	INL static uint8* WriteMapToArray(field_number, map<TKey, TValue>& value, output){
		for(map<TKey, TValue>::iterator itr = value.begin(); itr != value.end(); ++itr){
			uint8 tempbuf[1024]={0};
			uint8* newbuf = tempbuf;
			newbuf = WriteDataToArray(1, itr->first, newbuf);
			newbuf = WriteDataToArray(2, itr->second, newbuf);
			int _lens_cached_byte_size_ = newbuf - tempbuf;
			target = WireFormatLite::WriteTagToArray(field_number_arg, WireFormatLite::WIRETYPE_LENGTH_DELIMITED, target);
			target = io::CodedOutputStream::WriteVarint32ToArray(_lens_cached_byte_size_, target);
			target = io::CodedOutputStream::WriteRawToArray(tempbuf, _lens_cached_byte_size_, target);
		}
		return target;
	}
};

#undef MakeTag
#undef output
#undef field_number
#undef INL

class gpbtostr
{
public:
    gpbtostr(){}
    ~gpbtostr(){}

public:
    template<typename T>
    static std::string tostr(const T &t) 
    {   
        std::ostringstream sBuffer;
        sBuffer << t;
        return sBuffer.str();
    }
    template<typename T>
    static std::string tostr(const vector<T> &v) 
    {
        std::ostringstream sBuffer;
        sBuffer<<"vector{("<<v.size()<<")";
        for(vector<T>::const_iterator itr = v.begin(); itr != v.end(); ++itr)
            sBuffer << *itr <<";";
        sBuffer<<"}";
        return sBuffer.str();
    }
    template<typename TK, typename TV>
    static std::string tostr(const map<TK, TV> &m) 
    {
        std::ostringstream sBuffer;
        sBuffer<<"map{("<<m.size()<<")";
        for(map<TK, TV>::const_iterator itr = m.begin(); itr != m.end(); ++itr)
            sBuffer <<itr->first<<":"<<itr->second<<";";
        sBuffer<<"}";
        return sBuffer.str();
    }

    static std::string tostr(gpbmsg* msg) 
    {   
        if(msg == NULL)
            return string("");
        return msg->tostr();
    }
};

}
#endif//_GPBENCODE_H_
