/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <string_view>
#include <string>
#include <set>

#include "../mio.hpp"

#include "ascii.hpp"

namespace search
{
	namespace intake
	{
		template <typename CHARACTER = char, typename DATA, typename ON_WORD, typename IS_WORD = decltype(ascii::is_word_character)> void buffer(const DATA & data, ON_WORD && on_word, IS_WORD&& is_word = ascii::is_word_character)
		{
			std::set<std::string> unique;

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
					std::string word_str(start, u - start);
					std::transform(word_str.begin(), word_str.end(), word_str.begin(), ::tolower);

					auto [i, inserted] = unique.emplace(std::move(word_str));

					if(inserted)
						on_word(*i);

					start = nullptr;
				}

				u += inc;
			}
		}

		template <typename CHARACTER = char, typename ON_WORD, typename ON_BUFFER, typename IS_WORD = decltype(ascii::is_word_character) > void file(std::string_view name, ON_WORD&& on_word, ON_BUFFER && on_buffer, IS_WORD&& is_word = ascii::is_word_character)
		{
			if (!std::filesystem::file_size(name))
				return;

			auto map = mio::mmap_source(name);

			if(on_buffer(map))
				buffer(map, on_word, is_word);
		}
	}
}