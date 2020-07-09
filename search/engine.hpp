/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <string_view>
#include <string>
#include <filesystem>

#include "ascii.hpp"
#include "intake.hpp"

#include "tdb/se.hpp"

namespace search
{
	namespace engine
	{
		class LeanLookup
		{
			tdb::search_engine::LeanIndexStream index;

			struct Record
			{
				tdb::Key16 file_id;
				uint32_t rank;
			};

		public:
			LeanLookup(std::string_view database)
				: index(database) { }

			/*
				Intake
			*/

			template < typename T, typename IS_WORD = decltype(ascii::is_word_character) > void file(const T& source, IS_WORD&& is_word = ascii::is_word_character)
			{
				auto keyword_stream = index.Table<0>();
				auto file_screen = index.Table<1>();

				tdb::Key16 key;

				intake::file(source, [&](auto word)
				{
					auto key = std::string(word);
					std::transform(key.begin(), key.end(), key.begin(), ::tolower);

					keyword_stream.WriteLock(key, Record{ key,1 });

				}, [&](auto& buffer) 
				{
					key = tdb::Key16(buffer);
					auto [ref_count,found] = file_screen.InsertLock(key, 1);

					if (found)
						*(ref_count)++;

					return !found;
				}, is_word);
			}

			template < typename F > void folder(std::string_view path, F&& f)
			{
				_folder< std::filesystem::directory_iterator>(path, f);
			}

			template < typename F > void recursive_folder(std::string_view path, F&& f)
			{
				_folder< std::filesystem::recursive_directory_iterator>(path, f);
			}

			/*
				Search
			*/


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
		};
	}
}