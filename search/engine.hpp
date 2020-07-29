/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <string_view>
#include <string>
#include <filesystem>

#include "ascii.hpp"
#include "intake.hpp"

#include "tdb/se.hpp"

#include "d8u/util.hpp"
#include "d8u/buffer.hpp"

namespace search
{
	namespace engine
	{
		class LeanLookup
		{
			tdb::search_engine::LeanIndexStream db;

		public:

			LeanLookup(std::string_view database)
				: db(database) { }

			auto& Database() { return db; }



			/*
				Intake
			*/

			template < typename T, typename IS_WORD = decltype(ascii::is_word_character) > void file(const T& source, std::string_view file_name, std::string_view domain, IS_WORD&& is_word = ascii::is_word_character)
			{
				auto keyword_stream = db.Table<0>();
				auto file_screen = db.Table<1>();

				tdb::Key16 key;
				uint32_t index;

				intake::file(source, [&](auto & word)
				{
					keyword_stream.WriteLock(word, gsl::span<uint8_t>((uint8_t*)&index,sizeof(uint32_t)));

				}, [&](auto& buffer,auto & time) 
				{
					key = tdb::Key16(buffer);
					auto [element,found] = file_screen.EmplaceIf(key, (uint32_t) (time/1000), file_name,domain);

					index = element;
					/*if (found)
						*(ref_count)++;*/

					return !found;
				}, is_word);
			}

			template < typename F > void folder(std::string_view path, std::string_view domain, F&& f)
			{
				_folder< std::filesystem::directory_iterator>(path, domain,f);
			}

			template < typename F > void recursive_folder(std::string_view path, std::string_view domain, F&& f)
			{
				_folder< std::filesystem::recursive_directory_iterator>(path, domain,f);
			}



			/*
				Search
			*/

			auto keyword(std::string_view word)
			{
				auto keyword_stream = db.Table<0>();

				auto bytes = keyword_stream.Read(word);
				std::vector<uint32_t> result (bytes.size() / sizeof(uint32_t));

				std::memcpy(result.data(), bytes.data(), bytes.size());

				return result;
			}

			/*
				Comma separated list of keywords with joined results
			*/

			auto query(std::string_view keywords)
			{
				std::map<uint32_t, uint32_t> map;

				d8u::buffer::Helper stream(keywords);

				auto word = stream.GetWord();

				bool inner = false; //Has an inner join occured, Pausing new inserts?

				while (word)
				{
					bool lneg = (word[0] == '-');
					bool linner = (word[0] == '+');

					if (lneg || linner)
						word.Seek(1);

					auto hits = keyword(word);

					if (lneg)
					{
						for (auto hit : hits)
						{
							auto i = map.find(hit);
							if (i != map.end())
								map.erase(i);
						}
					}
					else if (!inner && !linner)
					{
						for (auto hit : hits)
						{
							auto [i,inserted] = map.try_emplace(hit, 1);

							if (!inserted)
								i->second++;
						}
					}
					else if(linner && inner)
					{
						std::map<uint32_t, uint32_t> lmap;

						for (auto hit : hits)
						{
							auto i = map.find(hit);
							if (i != map.end())
								lmap.emplace(hit, i->second+1);
						}

						map = std::move(lmap);
					}
					else if (linner)
					{
						std::map<uint32_t, uint32_t> lmap;

						for (auto hit : hits)
						{
							size_t v = 1;
							auto i = map.find(hit);
							if (i != map.end())
								v += i->second;

							lmap.emplace(hit, v);
						}

						map = std::move(lmap);
					}
					else if (inner)
					{
						for (auto hit : hits)
						{
							auto i = map.find(hit);
							if (i != map.end())
								i->second++;
						}
					}

					word = stream.GetWord();

					if (linner) inner = true;
				}

				std::multimap<uint32_t, uint32_t> sorted;

				for (auto& p : map)
					sorted.emplace(p.second*-1, p.first);

				std::vector<uint32_t> result; result.reserve (sorted.size());

				for (auto& p : sorted)
					result.push_back(p.second);

				return result;
			}	

		private:

			template < typename DITR, typename F > void _folder(std::string_view path, std::string_view domain, F&& on_file)
			{
				for (auto& e : DITR(path, std::filesystem::directory_options::skip_permission_denied))
				{
					if (e.is_directory())
						continue;

					std::string full,tail;
					uint64_t size = e.file_size();

					try
					{
						full = e.path().string();
						tail = e.path().filename().string();
					}
					catch (...)
					{
						std::cout << "Skiping file with unicode characters in name..." << std::endl;
						continue;
					}

					if(on_file(full, size));
						file(full,tail,domain);
				}
			}
		};
	}
}