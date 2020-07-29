/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "frequency.hpp"
#include "engine.hpp"

#include <chrono>

TEST_CASE("engine", "[search::]")
{
	std::filesystem::remove_all("se.db");

	using namespace search::engine;
	using namespace std::chrono;

	{
		LeanLookup se("se.db");

		auto t1 = high_resolution_clock::now();

		size_t count = 0;

		se.recursive_folder("../../", "test",[&](auto& name, auto size)
		{
			std::cout << name << " " << size << " " << count ++ << std::endl;

			return true;
		});

		auto t2 = high_resolution_clock::now();

		std::cout << (t2.time_since_epoch() - t1.time_since_epoch()).count() / 1000 << std::endl;

		auto table0 = se.Database().Table<0>();
		auto population = table0.Index().Population();

		std::cout << "Population: " << population.first << "/" << population.second << std::endl;


		se.recursive_folder("../../", "test", [&](auto& name, auto size)
		{
			auto keyword_stream = se.Database().Table<0>();
			auto file_screen = se.Database().Table<1>();

			tdb::Key16 key;
			uint32_t index;

			search::intake::file(name, [&](auto& word)
			{
				auto result = se.query(word);

				bool found = false;

				for (auto& e : result)
				{
					if (e == index)
					{
						found = true;
						break;
					}
				}

				if (!found)
					throw std::runtime_error("must be found");

			}, [&](auto& buffer, auto & time)
			{
				key = tdb::Key16(buffer);
				auto [element, found] = file_screen.EmplaceIf(key,0,"","");

				if (!found)
					throw std::runtime_error("must be found");

				index = element;

				return true;
			}, search::ascii::is_word_character);

			return false;
		});

		auto res = se.query("+test,+and,-for");

	}

	std::filesystem::remove_all("se.db");

	CHECK(true);
}

TEST_CASE("frequency", "[search::]")
{
	using namespace search::frequency;

	auto map = single_file("search.cpp");

	CHECK(true);
}

TEST_CASE("frequency database", "[search::]")
{
	using namespace search::frequency;

	Database freqdb;

	freqdb.folder("./", [&](auto& name, auto size) 
	{
		std::cout << name << " " << size << std::endl;
	});

	freqdb.Print();

	freqdb.Export("sample.txt");

	CHECK(true);
}
