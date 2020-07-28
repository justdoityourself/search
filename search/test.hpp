/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "frequency.hpp"
#include "engine.hpp"

TEST_CASE("engine", "[search::]")
{
	std::filesystem::remove_all("se.db");

	using namespace search::engine;

	{
		LeanLookup se("se.db");

		size_t count = 0;

		se.recursive_folder("../habitsmasher", [&](auto& name, auto size)
		{
			std::cout << name << " " << size << " " << count ++ << std::endl;
		});
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
