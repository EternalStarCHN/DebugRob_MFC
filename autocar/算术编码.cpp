#include <iostream>
#include "stdafx.h"

#ifdef TEST
#define M 100
#define N 4
using namespace std;
class suanshu
{
	int count, length;
	char number[N], n;
	long double chance[N], c;
	char code[M];
	long double High, Low, high, low, d;
public:
	suanshu()
	{
		High = 0; Low = 0;
	}
	void get_number();
	void get_code();
	void coding();
	~suanshu() {}
};
void suanshu::get_number()
{
	cout << "please input the number and its chance." << endl;
	for (int i = 0; i < N; i++)
	{
		cin >> n >> c;
		number[i] = n;
		chance[i] = c;
	}
	if (i == 20)
		cout << "the number is full." << endl;
	count = i;
}
void suanshu::get_code()
{
	cout << "please input the code''s length:";
	cin >> length;
	while (length >= M)
	{
		cout << "the length is too larger,please input a smaller one.";
		cin >> length;
	}
	for (int i = 0; i < length; i++)
	{
		cin >> code[i];
	}
}
void suanshu::coding()
{
	int i, j = 0;
	for (i = 0; i < count; i++)
		if (code[0] == number[i]) break;
	while (j < i)
		Low += chance[j++];
	d = chance[j];
	High = Low + d;
	for (i = 1; i < length; i++)
		for (j = 0; j < count; j++)
		{
			if (code[i] == number[j])
			{
				if (j == 0)
				{
					low = Low;
					high = Low + chance[j] * d;
					High = high;
					d *= chance[j];
				}
				else
				{
					float chance_l = 0.0;
					for (int k = 0; k <= j - 1; k++)
						chance_l += chance[k];
					low = Low + d * chance_l;
					high = Low + d * (chance_l + chance[j]);
					Low = low;
					High = high;
					d *= chance[j];
				}
			}
			else continue;
		}
	cout << "the result is:" << Low << endl;
}
int main()
{
	suanshu a;
	a.get_number();
	a.get_code();
	a.coding();
	return 0;
}
#endif