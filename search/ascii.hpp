/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <utility>

namespace search
{
	namespace ascii
	{
		std::pair<bool,size_t> is_word_character(char *c)
		{
			return std::make_pair((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z'),1);
		}
	}
}