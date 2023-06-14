#pragma once
#ifndef UTIL_H
#define UTIL_H

#include <random>
#include <string>
#include <sstream>
#include <iomanip>

namespace Utility {
	static std::string random_string()
	{
		std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

		std::random_device rd;
		std::mt19937 generator(rd());

		std::shuffle(str.begin(), str.end(), generator);

		return str.substr(0, 32);    // assumes 32 < number of characters in str         
	}

	static int random_int() {
		std::string str("123456789");
		std::random_device rd;
		std::mt19937 generator(rd());

		std::shuffle(str.begin(), str.end(), generator);
		str = str.substr(0, 5);
		int out = std::stoi(str);
		return out;
	}

	static std::string uint8_t_to_hexstring(const uint8_t* data, int len)
	{
		std::stringstream ss;
		ss << std::hex;

		for (int i(0); i < len; ++i)
			ss << std::setw(2) << std::setfill('0') << (int)data[i];

		return ss.str();
	}

	static void hexstring_to_uint8_t(uint8_t* out, size_t out_size, const std::string& string) {
		// check that there is enough space in the output array
		if (string.length() / 2 > out_size) {
			// handle error here
			return;
		}
		// convert the hex string to uint8_t values and store them in the output array
		for (size_t i = 0; i < string.length(); i += 2) {
			std::string byte_str = string.substr(i, 2);
			uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
			out[i / 2] = byte;
		}
	}

	static std::string decimal_to_hexstring(long long n)
	{
		std::stringstream hex_stream;
		hex_stream << std::hex << n;
		std::string hex_string = hex_stream.str();
		return hex_string;
	}

	static long long hexstring_to_decimal(std::string in)
	{
		std::size_t pos = 0;
		long long integer = std::stoull(in, std::addressof(pos), 16);
		return integer;
	}

	static void int_to_uint8_t(long long num, uint8_t* bits, size_t len) {
		int a = len - 1;
		for (int i = 0; i < len; i++) {
			bits[i] = (num >> ((a - i) * 8)) & 0xff;
		}
	}

	static long long uint8_t_to_int(uint8_t* bits, size_t len) {
		long long num = 0;
		int a = len - 1;
		for (int i = 0; i < len; i++) {
			num |= ((long long)bits[i] << ((a - i) * 8));
		}
		return num;
	}

	static std::string ascii_to_hexstring(std::string ascii_string) {
		std::stringstream hex_stream;
		for (size_t i = 0; i < ascii_string.length(); ++i) {
			hex_stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ascii_string[i]);
		}
		std::string hex_string = hex_stream.str();
		return hex_string;
	}

	static std::string hexstring_to_ascii(std::string hex_string) {
		std::string ascii_string;
		for (size_t i = 0; i < hex_string.length(); i += 2) {
			std::string byte_string = hex_string.substr(i, 2);
			char byte = static_cast<char>(std::strtol(byte_string.c_str(), nullptr, 16));
			ascii_string += byte;
		}
		return ascii_string;
	}

	static void ascii_to_uint8_t(const std::string& ascii_str, uint8_t* bytes, size_t len) {
		for (size_t i = 0; i < ascii_str.length(); i++) {
			bytes[i] = static_cast<uint8_t>(ascii_str[i]);
		}
	}

	static std::string uint8_t_to_ascii(const uint8_t* bytes, size_t len) {
		std::string result;
		for (size_t i = 0; i < len; i++) {
			result += static_cast<char>(bytes[i]);
		}
		return result;
	}
}



#endif