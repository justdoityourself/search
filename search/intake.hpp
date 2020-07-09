/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <string_view>

#include "../mio.hpp"

#include "ascii.hpp"

namespace search
{
	namespace intake
	{
		template <typename CHARACTER = char, typename DATA, typename ON_WORD, typename IS_WORD = decltype(ascii::is_word_character)> void buffer(const DATA & data, ON_WORD && on_word, IS_WORD&& is_word = ascii::is_word_character)
		{
			CHARACTER* u = (CHARACTER*) data.data(), *end, *start = nullptr;
			size_t limit = data.size() / sizeof(CHARACTER);
			end = u + limit;

			while(u != end)
			{
				auto [word, inc] = is_word(u);

				if (word && !start)
					start = u;
				else if (!word && start)
				{
					on_word(std::string_view(start, u - start));
					start = nullptr;
				}

				u += inc;
			}
		}

		template <typename CHARACTER = char, typename ON_WORD, typename ON_BUFFER, typename IS_WORD = decltype(ascii::is_word_character) > void file(std::string_view name, ON_WORD&& on_word, ON_BUFFER && on_buffer, IS_WORD&& is_word = ascii::is_word_character)
		{
			auto map = mio::mmap_source(name);

			if(on_buffer(map))
				buffer(map, on_word, is_word);
		}
	}
}