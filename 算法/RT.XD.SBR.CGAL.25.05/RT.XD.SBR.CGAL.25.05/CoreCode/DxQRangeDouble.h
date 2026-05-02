#pragma once


namespace RangeDoubleStd {

	class RangeDouble
	{
	public:
		double min_value;
		double max_value;
		RangeDouble();
		~RangeDouble();

	private:

	};

	class Range1Double
	{
	public:
		RangeDouble x1RangeDouble;
		Range1Double();
		~Range1Double();

	private:

	};


	class Range2Double
	{
	public:
		RangeDouble x1RangeDouble;
		RangeDouble x2RangeDouble;
		Range2Double();
		~Range2Double();

	private:

	};


	class Range3Double
	{
	public:
		RangeDouble x1RangeDouble;
		RangeDouble x2RangeDouble;
		RangeDouble x3RangeDouble;
		Range3Double();
		~Range3Double();

	private:

	};
}
