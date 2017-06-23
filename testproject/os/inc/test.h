#ifndef TEST_STR
#define __MAKE_KEYWORD_ENUM__
#define TEST_STR(symbol, name, arg) T_##symbol,
enum {
    TEST_UNKNOWN,
#endif
	TEST_STR(test,"asdf",1)
	TEST_STR(2333,"qwer",3)

#ifdef __MAKE_KEYWORD_ENUM__
    TEST_COUNT,
};
#undef __MAKE_KEYWORD_ENUM__

#undef TEST_STR
#endif

