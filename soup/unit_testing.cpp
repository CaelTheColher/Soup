#include "unit_testing.hpp"

#include <iostream>

using namespace soup;

void test(const char* name, void(*test)())
{
	new soup::Test(name, test);
	soup::Test::finishCurrentNode();
}

void assert_impl(bool b, soup::SourceLocation sl)
{
	if (!b)
	{
		Test::currently_running->err = "Assertion in ";
		Test::currently_running->err.append(sl.file_name);
		Test::currently_running->err.append(" on line ");
		Test::currently_running->err.append(std::to_string(sl.line));
		Test::currently_running->err.append(" failed");
		throw AssertionFailed();
	}
}

namespace soup
{
	void TestUnit::onFinishedBuildingTree()
	{
		// Note: Tree should be free'd by this function, but we exit, so who cares
		failed_tests = 0;
		total_tests = 0;
		runTests();
		if (failed_tests == 0)
		{
			std::cout << "All " << total_tests << " tests ran successfully!";
			exit(0);
		}
		std::cout << failed_tests << " / " << total_tests << " tests have failed.\n\n";
		truncateSuccessfulTests();
		printout();
		exit(1);
	}

	void TestUnit::runTests()
	{
		if (is_test)
		{
			try
			{
				Test::currently_running = reinterpret_cast<Test*>(this);
				Test::currently_running->test();
			}
			catch (const AssertionFailed&)
			{
				++failed_tests;
			}
			++total_tests;
		}
		for (const auto& child : children)
		{
			child->runTests();
		}
	}

	void TestUnit::truncateSuccessfulTests()
	{
		for (auto i = children.begin(); i != children.end(); ++i)
		{
			if ((*i)->is_test
				&& reinterpret_cast<Test*>(*i)->err.empty()
				)
			{
				//delete *i; // who cares? we're gonna exit soon anyway
				i = children.erase(i);
			}
		}
	}

	void TestUnit::printout(std::string prefix) const
	{
		std::cout << prefix << name;
		if (is_test)
		{
			std::cout << ": " << reinterpret_cast<const Test*>(this)->err;
		}
		std::cout << "\n";
		prefix.append("  ");
		for (const auto& child : children)
		{
			child->printout(prefix);
		}
	}
}
