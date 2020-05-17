#pragma once

#define INSTRUCT_ARR_SIZE 128

template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct Test1 {
	T1 k1;
	T2 k2;
	T3 k3;
	T4 k4;
	T5 k5;
};

template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct Test3 {
	T1 k1[INSTRUCT_ARR_SIZE];
	T2 k2[INSTRUCT_ARR_SIZE];
	T3 k3[INSTRUCT_ARR_SIZE];
	T4 k4[INSTRUCT_ARR_SIZE];
	T5 k5[INSTRUCT_ARR_SIZE];
};

#pragma pack(push, 1)
template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct Test2 {
	T1 k1;
	T2 k2;
	T3 k3;
	T4 k4;
	T5 k5;
};
#pragma pack(pop)
