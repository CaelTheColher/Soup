#pragma once

#include <string>
#include <vector>

#include "cbMeasurement.hpp"

namespace soup
{
	class cbParser
	{
	public:
		std::vector<std::string> words;
		std::vector<std::string>::iterator command_begin;
		std::vector<std::string>::iterator command_end;

		explicit cbParser(const std::string& text);

		[[nodiscard]] bool hasCommand() const noexcept
		{
			return command_begin != words.end();
		}

	public:
		bool checkTrigger(const std::string& trigger);
	private:
		bool checkTriggerWord(const std::string& trigger);
		bool checkTriggerPhrase(const std::vector<std::string>& trigger);

	private:
		[[nodiscard]] std::string word2arg(std::vector<std::string>::iterator i) const noexcept;
	public:
		[[nodiscard]] std::string getArgWord() const noexcept;
		[[nodiscard]] std::string getArgWordLefthand() const noexcept;

		[[nodiscard]] std::string getArgModifier() const noexcept;

		[[nodiscard]] std::string getArgNumeric() const noexcept;
		[[nodiscard]] std::string getArgNumericSecond() const noexcept;
		[[nodiscard]] std::string getArgNumericLefthand() const noexcept;

		[[nodiscard]] cbMeasurement getArgMeasurement() const noexcept;
		[[nodiscard]] cbMeasurement getArgMeasurementLefthand() const noexcept;

		[[nodiscard]] cbUnit getArgUnit() const noexcept;
	};
}