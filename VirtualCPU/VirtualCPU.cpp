#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

enum OpCode : uint8_t
{
	OPCODE_MOV_REG_MEM_TO_FROM_REG = 0x22,
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

static const char* effective_address_calc_table[8] =
{
	"bx + si",
	"bx + di",
	"bp + si",
	"bp + di",
	"si",
	"di",
	"bp",
	"bx",
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

uint8_t GetBitMask(uint8_t bit_size)
{
	return 0xFF >> (8 - bit_size);
}

bool CheckOpCode(OpCode opcode, uint8_t byte, uint8_t opcode_bit_size)
{
	return ((byte >> (8 - opcode_bit_size)) & GetBitMask(opcode_bit_size)) == opcode;
}

void GetEffectiveAddressCalcString(char* buf, size_t buf_size, uint8_t rm, uint16_t disp)
{
	if (disp > 0)
		sprintf_s(buf, buf_size, "[%s + %d]", effective_address_calc_table[rm], disp);
	else
		sprintf_s(buf, buf_size, "[%s]", effective_address_calc_table[rm]);
}

void Disassemble(InstructionStream* stream)
{
	while (stream->inst_at < stream->inst_end)
	{
		uint8_t byte0 = ReadByteFromStream(stream);

		if (CheckOpCode(OPCODE_MOV_REG_MEM_TO_FROM_REG, byte0, 6))
		{
			uint8_t D = (byte0 >> 1) & GetBitMask(1);
			uint8_t W = (byte0 >> 0) & GetBitMask(1);

			uint8_t byte1 = ReadByteFromStream(stream);
			uint8_t mod = (byte1 >> 6) & GetBitMask(2);

			uint8_t reg = (byte1 >> 3) & GetBitMask(3);
			uint8_t rm = (byte1 >> 0) & GetBitMask(3);

			const char* dst_str = 0;
			const char* src_str = 0;

			if (mod == 0x0)
			{
				if (reg == 0x6)
				{
					// 16-bit displacement, special case
					uint16_t disp_lo_hi = ReadWordFromStream(stream);

					char eff_addr_str[32];
					GetEffectiveAddressCalcString(eff_addr_str, 32, rm, disp_lo_hi);
					dst_str = D ? registers[W][reg] : eff_addr_str;
					src_str = D ? eff_addr_str : registers[W][reg];
				}
				else
				{
					// No displacement follows
					char eff_addr_str[32];
					GetEffectiveAddressCalcString(eff_addr_str, 32, rm, 0);
					dst_str = D ? registers[W][reg] : eff_addr_str;
					src_str = D ? eff_addr_str : registers[W][reg];
				}
			}
			else if (mod == 0x1)
			{
				// 8-bit displacement follows
				uint8_t disp_lo = ReadByteFromStream(stream);

				char eff_addr_str[32];
				GetEffectiveAddressCalcString(eff_addr_str, 32, rm, disp_lo);
				dst_str = D ? registers[W][reg] : eff_addr_str;
				src_str = D ? eff_addr_str : registers[W][reg];
			}
			else if (mod == 0x2)
			{
				// 16-bit displacement follows
				uint16_t disp_lo_hi = ReadWordFromStream(stream);

				char eff_addr_str[32];
				GetEffectiveAddressCalcString(eff_addr_str, 32, rm, disp_lo_hi);
				dst_str = D ? registers[W][reg] : eff_addr_str;
				src_str = D ? eff_addr_str : registers[W][reg];
			}
			else if (mod == 0x3)
			{
				// Register to register, no displacement
				uint8_t dst_reg = D ? reg : rm;
				uint8_t src_reg = D ? rm : reg;

				dst_str = registers[W][dst_reg];
				src_str = registers[W][src_reg];
			}

			printf("mov %s, %s\n", dst_str, src_str);
		}
		else if (CheckOpCode(OPCODE_MOV_IMMEDIATE_TO_REG, byte0, 4))
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
