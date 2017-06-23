#ifndef __TEST_CPP1_HEADER__
#define __TEST_CPP1_HEADER__
class Time{
	private :
	int hours;
	int minutes;
	public :
	int test;
	Time();
	Time(int h,int m);
	void AddMin(int m);
	void AddHr(int h);
	Time Sum(const Time t) const;
	void Show() const;
};

#endif
