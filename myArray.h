#pragma once

typedef struct intArrayElement
{
	int				count;		// number of elements
	int				mult;		// element multiplier on the right
} intArrayElement;

class myArray
{
public:
	myArray()
	{
		
	}

	void setLength(const int num)
	{
		data.resize(num);
	}

	int size()
	{
		if (data.empty())
			data.resize(1);
		return static_cast<int>(data.size())-1;
	}

	void resize(int num)
	{
		data.resize(num);
	}

	intArrayElement& operator[](const int index)
	{
		return data[index];
	}

	intArrayElement operator[](const int index) const {
		return data[index];
	}

private:
	std::vector< intArrayElement> data;
};