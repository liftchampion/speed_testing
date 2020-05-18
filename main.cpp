#include <iostream>
#include <cstring>
#include "structs.h"
#include <cmath>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <x86intrin.h>

#define SIZE_OF_STAT 1000000
#define WARM_UP_TIMES 10

void inline start_count(unsigned &cycles_high, unsigned &cycles_low)
{
	asm volatile (
	"CPUID\n\t"
	"RDTSC\n\t"
	"mov %%edx, %0\n\t"
	"mov %%eax, %1\n\t"
	: "=r" (cycles_high), "=r" (cycles_low)::"%rax", "%rbx", "%rcx", "%rdx");

}

void inline stop_count(unsigned &cycles_high, unsigned &cycles_low)
{
	asm volatile (
	"RDTSCP\n\t"
	"mov %%edx, %0\n\t"
	"mov %%eax, %1\n\t"
	"CPUID\n\t"
	: "=r" (cycles_high), "=r" (cycles_low)::"%rax", "%rbx", "%rcx", "%rdx");
}

uint64_t inline convert_res(unsigned cycles_high, unsigned cycles_low)
{
	return ((uint64_t) cycles_high << 32u) | cycles_low;
}

template<typename T, typename F>
void inline
Filltimes(std::vector<uint64_t>& times, void (*f)(F *src, F* dst), T *src, T* dst)
{
	int i;
	uint64_t start, end;
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
	for (i = 0; i < WARM_UP_TIMES; i++)
	{
		start_count(cycles_high, cycles_low);
		stop_count(cycles_high1, cycles_low1);
	}
	for (i = 0; i < SIZE_OF_STAT; i++)
	{
		start_count(cycles_high, cycles_low);
		f((F*)src, (F*)dst);
		stop_count(cycles_high1, cycles_low1);
		start = convert_res(cycles_high, cycles_low);
		end = convert_res(cycles_high1, cycles_low1);
		times[i] = end - start;
	}
}

using NoPackDesc = Test1<std::int64_t, std::int64_t, std::int32_t, char, char>;
using NoPackMix = Test1<char, std::int64_t, std::int32_t, char, std::int64_t>;
using PackDesc = Test2<std::int64_t, std::int64_t, std::int32_t, char, char>;
using PackMix = Test2<char, std::int64_t, std::int32_t, char, std::int64_t>;
using ArrStruct = Test3<std::int64_t, std::int64_t, std::int32_t, char, char>;
using Results = std::unordered_map<std::string, std::unordered_map<std::string, std::pair<double, double>>>;


template<typename S>
void StandartCopyTest(S* src, S* dst)
{
	for (int i = 0; i < 1; i++)
	{
		*dst = *src;
	}
}

template<typename S>
void MemcpyTest(S* src, S* dst)
{
	for (int i = 0; i < 1; i++)
	{
		std::memcpy(src, dst, sizeof(S));
	}
}

template<typename S>
void sseCopyTest(char* src, char* dst){
	for (int i = 0; i < 1; i++){
		char* target = src + sizeof(S);
		for (;src <= target - 16; src += 16, dst += 16){
			__m128i buffer = _mm_load_si128((__m128i*)src);
			_mm_store_si128((__m128i*)dst, buffer);
		}
		for(;src <= target - 8; src += 8, dst += 8){
			__m128i buffer = _mm_loadl_epi64((__m128i_u*)src);
			_mm_storel_epi64((__m128i_u*)dst, buffer);
		}
		for(;src <= target - 4; src += 4, dst += 4){
			*(int*)dst = *(int*)src;
		}
		for(;src != target; src += 1, dst += 1){
			*dst = *src;
		}
	}
}

template<typename S>
inline void avxCopyTest(char* src, char* dst){
	for (int i = 0; i < 1; i++){
		char* target = src + sizeof(S);
		for (;src <= target - 32; src += 32, dst += 32){
			__m256i buffer = _mm256_load_si256((__m256i*)src);
			_mm256_store_si256((__m256i*)dst, buffer);
		}
		for (;src <= target - 16; src += 16, dst += 16){
			__m128i buffer = _mm_load_si128((__m128i*)src);
			_mm_store_si128((__m128i*)dst, buffer);
		}
		for(;src <= target - 8; src += 8, dst += 8){
			__m128i buffer = _mm_loadl_epi64((const __m128i_u*)src);
			_mm_storel_epi64((__m128i_u*)dst, buffer);
		}
		for(;src <= target - 4; src += 4, dst += 4){
			*(int*)dst = *(int*)src;
		}
		for(;src != target; src += 1, dst += 1){
			*dst = *src;
		}
	}
}

template<typename S>
inline void avx2CopyTest(char* src, char* dst){
	for (int i = 0; i < 1; i++){
		char* target = src + sizeof(S);
		for (;src <= target - 32; src += 32, dst += 32){
			__m256i buffer = _mm256_load_si256((__m256i*)src);
			_mm256_stream_si256((__m256i*)dst, buffer);
		}
		for (;src <= target - 16; src += 16, dst += 16){
			__m128i buffer = _mm_load_si128((__m128i*)src);
			_mm_store_si128((__m128i*)dst, buffer);
		}
		for(;src <= target - 8; src += 8, dst += 8){
			__m128i buffer = _mm_loadl_epi64((const __m128i_u*)src);
			_mm_storel_epi64((__m128i_u*)dst, buffer);
		}
		for(;src <= target - 4; src += 4, dst += 4){
			*(int*)dst = *(int*)src;
		}
		for(;src != target; src += 1, dst += 1){
			*dst = *src;
		}
	}
}

//template<typename S>
//inline void avx512CopyTest(char* src, char* dst){
//	for (int i = 0; i < 1; i++){
//		char* target = src + sizeof(S);
//
//		for (;src < src + sizeof(S); src += 16, dst += 16){
//			__m512i buffer = _mm512_load_si512((__m512i*)src);
//			_mm512_stream_si512((__m512i*)dst, buffer);
//		}
//		for (;src < src + sizeof(S); src += 8, dst += 8){
//			__m256i buffer = _mm256_load_si256((__m256i*)src);
//			_mm256_stream_si256((__m256i*)dst, buffer);
//		}
//		for (;src < target - 16; src += 16, dst += 16){
//			__m128i buffer = _mm_load_si128((__m128i*)src);
//			_mm_store_si128((__m128i*)dst, buffer);
//		}
//		for(;src < target - 8; src += 8, dst += 8){
//			__m128i buffer = _mm_loadl_epi64(src);
//			_mm_storel_epi64(dst, buffer);
//		}
//		for(;src < target - 4; src += 4, dst += 4){
//			*(int*)dst = *(int*)src;
//		}
//		for(;src != target; src += 1, dst += 1){
//			*dst = *src;
//		}
//	}
//}

inline void Empty(int*, int*){
	for (int i = 0; i < 1; i++) {
	}
}

std::pair<double, double> print_stats(std::vector<uint64_t>& inputs)
{
	std::cout << "Here comes the stats"<<std::endl;
	std::sort(inputs.begin(), inputs.end());
	auto size = (size_t)(SIZE_OF_STAT * .95f);
	size_t i;
	uint64_t min = inputs.front(), max = inputs.back();
	double mean = 0, variance = 0, std;
	for(i = 0; i < size; i++){
		mean += (inputs[i] - min);
	}
	mean = mean / size + min;
	for (i = 0; i < size; i++)
	{
		variance += std::pow(mean - (int64_t)inputs[i], 2);
	}
	variance /= size;
	std = sqrt(variance);
	std::cout << "\tMin value: " << min << "\n";
	std::cout << "\tMax value: " << max << "\n";
	std::cout << "\tMean value: " << mean << "\n";
	std::cout << "\tVariance: " << variance << "\n";
	std::cout << "\tSTD: " << std << std::endl;
	return {mean, std};
}

template<typename T, typename Function>
std::pair<double, double> TestFunction(Function f)
{
	auto src = (T*)_mm_malloc(sizeof(T), 64);
	auto dst = (T*)_mm_malloc(sizeof(T), 64);
	std::vector<uint64_t> times(SIZE_OF_STAT, 0);
	Filltimes<T>(times, f, src, dst);
	_mm_free(src);
	_mm_free(dst);
	return print_stats(times);
}

void print_final_results(Results& map){

	for (const auto& [func_name, res_by_struct] : map){
		double avg = 0, std = 0;
		std::cout << "\033[0;32mResults for " << func_name << "\n";
		for (const auto& [struct_name, res] : res_by_struct){
			avg += res.first;
			std += res.second;
			std::cout << "\t" << struct_name << " mean result is "
			<< res.first << " +/- " << res.second << "\n";
		}
		std::cout << "\t\033[0;34mAverage for all structs is "
		<< avg / res_by_struct.size() << " +/- "
		<< std / res_by_struct.size() << "\n";
	}
	std::cout << "\033[0m";
}

int main()
{
	Results map;
	std::cout << "We are gona meassure this shit!\n";
	std::cout << "\033[0;33mReference value\033[0m\n";
	map["Reference"]["Empty"] = TestFunction<int>(Empty);
	std::cout << "\033[0;33mChecking standard copy operator\033[0m\n";
	map["StandartCopyTest"]["NoPackDesc"] = TestFunction<NoPackDesc>(StandartCopyTest<NoPackDesc>);
	map["StandartCopyTest"]["PackDesc"] = TestFunction<PackDesc>(StandartCopyTest<PackDesc>);
	map["StandartCopyTest"]["NoPackMix"] = TestFunction<NoPackMix>(StandartCopyTest<NoPackMix>);
	map["StandartCopyTest"]["PackMix"] = TestFunction<PackMix>(StandartCopyTest<PackMix>);
	map["StandartCopyTest"]["ArrStruct"] = TestFunction<ArrStruct>(StandartCopyTest<PackMix>);
	std::cout << "\033[0;33mChecking memcpy\033[0m\n";
	map["MemcpyTest"]["NoPackDesc"] = TestFunction<NoPackDesc>(MemcpyTest<NoPackDesc>);
	map["MemcpyTest"]["PackDesc"] = TestFunction<PackDesc>(MemcpyTest<PackDesc>);
	map["MemcpyTest"]["NoPackMix"] = TestFunction<NoPackMix>(MemcpyTest<NoPackMix>);
	map["MemcpyTest"]["PackMix"] = TestFunction<PackMix>(MemcpyTest<PackMix>);
	map["MemcpyTest"]["ArrStruct"] = TestFunction<ArrStruct>(MemcpyTest<PackMix>);
	std::cout << "\033[0;33mChecking SSE\033[0m\n";
	map["sseCopyTest"]["NoPackDesc"] = TestFunction<NoPackDesc>(sseCopyTest<NoPackDesc>);
	map["sseCopyTest"]["PackDesc"] = TestFunction<PackDesc>(sseCopyTest<PackDesc>);
	map["sseCopyTest"]["NoPackMix"] = TestFunction<NoPackMix>(sseCopyTest<NoPackMix>);
	map["sseCopyTest"]["PackMix"] = TestFunction<PackMix>(sseCopyTest<PackMix>);
	map["sseCopyTest"]["ArrStruct"] = TestFunction<ArrStruct>(sseCopyTest<PackMix>);
	std::cout << "\033[0;33mChecking AVX\033[0m\n";
	map["avxCopyTest"]["NoPackDesc"] = TestFunction<NoPackDesc>(avxCopyTest<NoPackDesc>);
	map["avxCopyTest"]["PackDesc"] = TestFunction<PackDesc>(avxCopyTest<PackDesc>);
	map["avxCopyTest"]["NoPackMix"] = TestFunction<NoPackMix>(avxCopyTest<NoPackMix>);
	map["avxCopyTest"]["PackMix"] = TestFunction<PackMix>(avxCopyTest<PackMix>);
	map["avxCopyTest"]["ArrStruct"] = TestFunction<ArrStruct>(avxCopyTest<PackMix>);
	std::cout << "\033[0;33mChecking AVX2\033[0m\n";
	map["avx2CopyTest"]["NoPackDesc"] = TestFunction<NoPackDesc>(avx2CopyTest<NoPackDesc>);
	map["avx2CopyTest"]["PackDesc"] = TestFunction<PackDesc>(avx2CopyTest<PackDesc>);
	map["avx2CopyTest"]["NoPackMix"] = TestFunction<NoPackMix>(avx2CopyTest<NoPackMix>);
	map["avx2CopyTest"]["PackMix"] = TestFunction<PackMix>(avx2CopyTest<PackMix>);
	map["avx2CopyTest"]["ArrStruct"] = TestFunction<ArrStruct>(avx2CopyTest<PackMix>);
//	std::cout << "Checking AVX512\n";
//	map["avx512CopyTest"]["NoPackDesc"] = TestFunction<NoPackDesc>(avx512CopyTest<NoPackDesc>);
//	map["avx512CopyTest"]["PackDesc"] = TestFunction<PackDesc>(avx512CopyTest<PackDesc>);
//	map["avx251CopyTest"]["NoPackMix"] = TestFunction<NoPackMix>(avx512CopyTest<NoPackMix>);
//	map["avx512CopyTest"]["PackMix"] = TestFunction<PackMix>(avx512CopyTest<PackMix>);
//	map["avx512CopyTest"]["ArrStruct"] = TestFunction<ArrStruct>(avx512CopyTest<PackMix>);

	std::cout << "NoPackDesc" << ": " << sizeof(NoPackDesc) << std::endl;
	std::cout << "PackDesc" << ": " << sizeof(PackDesc) << std::endl;
	std::cout << "NoPackMix" << ": " << sizeof(NoPackMix) << std::endl;
	std::cout << "PackMix" << ": " << sizeof(PackMix) << std::endl;
	std::cout << "ArrStruct" << ": " << sizeof(ArrStruct) << std::endl;
	print_final_results(map);
	return 0;
}
