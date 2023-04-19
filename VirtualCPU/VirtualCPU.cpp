#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

constexpr uint8_t OPCODE_BITMASK = 0x3F;
constexpr uint8_t D_BITMASK = 0x3;
constexpr uint8_t W_BITMASK = 0x1;
constexpr uint8_t MOD_BITMASK = 0x3;
constexpr uint8_t REG_BITMASK = 0x7;
constexpr uint8_t RM_BITMASK = 0x7;

enum OpCode : uint8_t
{
	OPCODE_MOV = 0x22,
};

static const char* registers[2][8] =
{
	{
		"al", "cl", "dl", "bl",
		"ah", "ch", "dh", "bh",
	},
	{
		"ax", "cx", "dx", "bx",
		"sp", "bp", "si", "di"
	}
};

struct InstructionStream
{
	uint8_t* inst_begin;
	uint8_t* inst_at;
	uint8_t* inst_end;
};

uint8_t ReadByteFromStream(InstructionStream* stream)
{
	uint8_t result = 0;
	if (stream->inst_at < stream->inst_end)
	{
		result = *(stream->inst_at++);
	}

	return result;
}

void Disassemble(InstructionStream* stream)
{
	while (stream->inst_at < stream->inst_end)
	{
		uint8_t byte0 = ReadByteFromStream(stream);
		// Don't really need to bitmask the opcode, since it starts at the very left bit, but whatever
		uint8_t opcode = (byte0 >> 2) & OPCODE_BITMASK;

		switch (opcode)
		{
		case OpCode::OPCODE_MOV:
		{
			uint8_t D = (byte0 >> 1) & D_BITMASK;
			uint8_t W = (byte0 >> 0) & W_BITMASK;

			uint8_t byte1 = ReadByteFromStream(stream);
			uint8_t mod = (byte1 >> 6) & MOD_BITMASK;
			if (mod != 0x3)
			{
				printf("MOV operator other than register to register is currently not supported\n");
			}

			uint8_t reg = (byte1 >> 3) & REG_BITMASK;
			uint8_t rm = (byte1 >> 0) & RM_BITMASK;

			uint8_t dst_reg = D ? reg : rm;
			uint8_t src_reg = D ? rm : reg;

			printf("mov %s, %s\n", registers[W][dst_reg], registers[W][src_reg]);
		} break;
		default:
		{
			printf("Invalid OpCode!\n");
		} break;
		}
	}
}

void ReadByteFile(const char* filepath, uint8_t* dest, size_t dest_byte_size, size_t* bytes_read)
{
	FILE* file_ptr;
	fopen_s(&file_ptr, filepath, "rb");
	if (!file_ptr)
	{
		printf("File could not be opened\n");
		return;
	}

	fseek(file_ptr, 0, SEEK_END);
	long file_size = ftell(file_ptr);
	rewind(file_ptr);
	
	if (dest_byte_size < file_size)
	{
		printf("Destination buffer size is too small to fit the entire file\n");
	}
	else
	{
		*bytes_read = fread(dest, 1, file_size, file_ptr);
		if (*bytes_read != file_size)
		{
			printf("Actual amount of bytes read does not correspond to the total file size\n");
		}
	}

	fclose(file_ptr);
}

int main(int num_args, char** args)
{
	uint8_t buffer[1024];
	size_t bytes_read = 0;

	ReadByteFile("ASM/listing_0037_single_register_mov", buffer, sizeof(buffer), &bytes_read);
	InstructionStream stream = {};
	stream.inst_begin = buffer;
	stream.inst_at = stream.inst_begin;
	stream.inst_end = stream.inst_begin + bytes_read;
	Disassemble(&stream);

	ReadByteFile("ASM/listing_0038_many_register_mov", buffer, sizeof(buffer), &bytes_read);
	stream = {};
	stream.inst_begin = buffer;
	stream.inst_at = stream.inst_begin;
	stream.inst_end = stream.inst_begin + bytes_read;
	Disassemble(&stream);

	system("pause");
	return 0;
}
