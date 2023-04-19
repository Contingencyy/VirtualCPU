#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

enum OpCode : uint8_t
{
	OPCODE_MOV_REG_MEM_TO_REG = 0x22,
	OPCODE_MOV_IMMEDIATE_TO_REG = 0x0B
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
	if (stream->inst_at + sizeof(uint8_t) <= stream->inst_end)
	{
		result = *(stream->inst_at++);
	}

	return result;
}

uint16_t ReadWordFromStream(InstructionStream* stream)
{
	uint16_t result = 0;
	if (stream->inst_at + sizeof(uint16_t) <= stream->inst_end)
	{
		result = *((uint16_t*)stream->inst_at);
		stream->inst_at += sizeof(uint16_t);
	}

	return result;
}

void Disassemble(InstructionStream* stream)
{
	while (stream->inst_at < stream->inst_end)
	{
		uint8_t byte0 = ReadByteFromStream(stream);
		if (((byte0 >> 2) & 0x3F) == OpCode::OPCODE_MOV_REG_MEM_TO_REG)
		{
			uint8_t D = (byte0 >> 1) & 0x1;
			uint8_t W = (byte0 >> 0) & 0x1;

			uint8_t byte1 = ReadByteFromStream(stream);
			uint8_t mod = (byte1 >> 6) & 0x3;
			if (mod != 0x3)
			{
				printf("MOV operator other than register to register is currently not supported\n");
			}

			uint8_t reg = (byte1 >> 3) & 0x7;
			uint8_t rm = (byte1 >> 0) & 0x7;

			uint8_t dst_reg = D ? reg : rm;
			uint8_t src_reg = D ? rm : reg;

			printf("mov %s, %s\n", registers[W][dst_reg], registers[W][src_reg]);
		}
		else if (((byte0 >> 4) & 0xF) == OpCode::OPCODE_MOV_IMMEDIATE_TO_REG)
		{
			uint8_t W = (byte0 >> 3) & 0x1;
			uint8_t reg = (byte0 >> 0) & 0x7;
			
			if (W == 0)
			{
				uint8_t data_byte = ReadByteFromStream(stream);
				printf("mov %s, %u\n", registers[W][reg], data_byte);
			}
			else
			{
				uint16_t data_word = ReadWordFromStream(stream);
				printf("mov %s, %u\n", registers[W][reg], data_word);
			}
		}
		else
		{
			printf("Invalid OpCode!\n");
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
	InstructionStream stream = {};

	// ==========================================================
	// Listing 37 & 38

#if 0
	{
		const char* listing_37 = "Listings/listing_0037_single_register_mov";
		printf("====================================================\n");
		printf("Listing %s\n", listing_37);
		ReadByteFile(listing_37, buffer, sizeof(buffer), &bytes_read);
		stream = {};
		stream.inst_begin = buffer;
		stream.inst_at = stream.inst_begin;
		stream.inst_end = stream.inst_begin + bytes_read;
		Disassemble(&stream);
		printf("====================================================\n\n");

		const char* listing_38 = "Listings/listing_0038_many_register_mov";
		printf("====================================================\n");
		printf("Listing %s\n", listing_38);
		ReadByteFile(listing_38, buffer, sizeof(buffer), &bytes_read);
		stream = {};
		stream.inst_begin = buffer;
		stream.inst_at = stream.inst_begin;
		stream.inst_end = stream.inst_begin + bytes_read;
		Disassemble(&stream);
		printf("====================================================\n\n");
	}
#endif

	// ==========================================================
	// Listing 39 & 40

	{
		const char* listing_39 = "Listings/listing_0039_more_movs";
		printf("====================================================\n");
		printf("Listing %s\n", listing_39);
		ReadByteFile(listing_39, buffer, sizeof(buffer), &bytes_read);
		stream = {};
		stream.inst_begin = buffer;
		stream.inst_at = stream.inst_begin;
		stream.inst_end = stream.inst_begin + bytes_read;
		Disassemble(&stream);
		printf("====================================================\n\n");

		const char* listing_40 = "Listings/listing_0040_challenge_movs";
		printf("====================================================\n");
		printf("Listing %s\n", listing_40);
		ReadByteFile(listing_40, buffer, sizeof(buffer), &bytes_read);
		stream = {};
		stream.inst_begin = buffer;
		stream.inst_at = stream.inst_begin;
		stream.inst_end = stream.inst_begin + bytes_read;
		Disassemble(&stream);
		printf("====================================================\n\n");
	}

	system("pause");
	return 0;
}
