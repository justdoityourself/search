/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <map>
#include <string>
#include <filesystem>
#include <fstream>

#include "ascii.hpp"
#include "intake.hpp"

namespace search
{
	namespace frequency
	{
		/*
			In memory analysis of word frequencies
		*/

		class Database
		{
		public:
			template < typename T, typename IS_WORD = decltype(ascii::is_word_character) > void file(const T& source, IS_WORD&& is_word = ascii::is_word_character)
			{
				intake::file(source, [&](auto word)
				{
					auto key = std::string(word);
					std::transform(key.begin(), key.end(), key.begin(), ::tolower);

					auto [i, did_insert] = map.try_emplace(key, 0);
					i->second++;
				}, [&](auto& buffer) { return true;}, is_word);
			}

			template < typename T, typename IS_WORD = decltype(ascii::is_word_character) > void buffer(const T& source, IS_WORD&& is_word = ascii::is_word_character)
			{
				intake::buffer(source, [&](auto word)
				{
					auto key = std::string(word);
					std::transform(key.begin(), key.end(), key.begin(), ::tolower);

					auto [i, did_insert] = map.try_emplace(key, 0);
					i->second++;
				}, is_word);
			}

			template < typename F > void folder(std::string_view path, F && f)
			{
				_folder< std::filesystem::directory_iterator>(path, f);
			}

			template < typename F > void recursive_folder(std::string_view path, F&& f)
			{
				_folder< std::filesystem::recursive_directory_iterator>(path,f);
			}

			const auto& data() { return map; }

			void Print()
			{
				for (auto& i : map)
					std::cout << i.first << " " << i.second << std::endl;
			}

			void Export(std::string_view file)
			{
				std::ofstream handle(file);

				for (auto& i : map)
					handle << i.first << " " << i.second << std::endl;
			}

		private:
			template < typename DITR, typename F > void _folder(std::string_view path, F&& on_file)
			{
				for (auto& e : DITR(path, std::filesystem::directory_options::skip_permission_denied))
				{
					if (e.is_directory())
						continue;

					std::string name;
					uint64_t size = e.file_size();

					try
					{
						name = e.path().string();
					}
					catch (...)
					{
						std::cout << "Skiping file with unicode characters in name..." << std::endl;
						continue;
					}

					on_file(name, size);

					file(name);
				}
			}

			std::map<std::string, size_t> map;
		};

		template < typename T, typename IS_WORD = decltype(ascii::is_word_character)> auto single_file(const T & source, IS_WORD&& is_word = ascii::is_word_character)
		{
			std::map<std::string, size_t> map;

			intake::file(source, [&](auto word)
			{
				auto key = std::string(word);
				std::transform(key.begin(), key.end(), key.begin(), ::tolower);

				auto [i, did_insert] = map.try_emplace(key,0);
				i->second++;
			}, [&](auto& buffer) {return true;},is_word);

			return map;
		}
	}
}