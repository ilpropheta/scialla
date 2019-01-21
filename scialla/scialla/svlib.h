/*
Copyright (c) 2019 Marco Arena

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once
#include <string_view>
#include <algorithm>
#include <optional>
#include <charconv>
#include <type_traits>
#include <algorithm>
#include <functional>

namespace svlib
{
	namespace details
	{
		// internal metaprogramming utilities
		namespace meta
		{
			template<typename DelimOrPred>
			constexpr bool IsSplitPredicate = std::is_invocable_r_v<bool, DelimOrPred, char>;

			template<typename DelimOrPred>
			constexpr bool IsSplitTwoArgsPredicate = std::is_invocable_r_v<std::pair<size_t,size_t>, DelimOrPred, std::string_view, size_t>;

			template<typename What>
			struct FnTraits
			{
				template<typename DelimOrPred>
				static size_t find_last_not_of(std::string_view sv, DelimOrPred&& delimOrPred)
				{
					if constexpr (meta::IsSplitPredicate<What>)
					{
						return std::distance(sv.begin(), std::find_if(sv.rbegin(), sv.rend(), std::not_fn(std::forward<DelimOrPred>(delimOrPred))).base()) - 1;
					}
					else
					{
						return sv.find_last_not_of(delimOrPred);
					}
				}

				template<typename DelimOrPred>
				static size_t find_first_not_of(std::string_view sv, DelimOrPred&& delimOrPred)
				{
					if constexpr (meta::IsSplitPredicate<What>)
					{
						return std::distance(sv.begin(), std::find_if(sv.begin(), sv.end(), std::not_fn(std::forward<DelimOrPred>(delimOrPred))));
					}
					else
					{
						return sv.find_first_not_of(delimOrPred);
					}
				}

				template<typename DelimOrPred>
				static size_t find_first_of(std::string_view sv, DelimOrPred&& delimOrPred)
				{
					if constexpr (meta::IsSplitPredicate<What>)
					{
						return std::distance(sv.begin(), std::find_if(sv.begin(), sv.end(), std::forward<DelimOrPred>(delimOrPred)));
					}
					else
					{
						return sv.find_first_of(delimOrPred);
					}
				}

				template<typename DelimOrPred>
				static std::pair<size_t, size_t> find_first_of_from(std::string_view sv, DelimOrPred&& delimOrPred, size_t start)
				{
					if constexpr (meta::IsSplitTwoArgsPredicate<What>)
					{
						return std::forward<DelimOrPred>(delimOrPred)(sv, start);
					}
					else
					{
						auto pos = sv.find_first_of(delimOrPred, start);
						return { pos, pos != std::string_view::npos ? pos + 1 : pos };
					}
				}
			};
		}
		
		auto data(const char* str)
		{
			return str;
		}
	}

	// remove the rightmost characters matching with delimOrPred
	template<typename DelimOrPred = char>
	[[nodiscard]] std::string_view trim_right(std::string_view str, DelimOrPred&& delimOrPred = ' ')
	{
		if (auto lastGood = details::meta::FnTraits<DelimOrPred>::find_last_not_of(str, std::forward<DelimOrPred>(delimOrPred));  lastGood != std::string_view::npos)
		{
			str.remove_suffix(str.size() - lastGood - 1);
			return str;
		}
		return {};
	}

	// remove the leftmost characters matching with delimOrPred
	template<typename DelimOrPred = char>
	[[nodiscard]] std::string_view trim_left(std::string_view str, DelimOrPred delimOrPred = ' ')
	{
		str.remove_prefix(std::min(details::meta::FnTraits<DelimOrPred>::find_first_not_of(str, std::forward<DelimOrPred>(delimOrPred)), str.size()));
		return str;
	}

	// remove the rightmost and leftmost characters matching with delimOrPred
	template<typename DelimOrPred = char>
	[[nodiscard]] std::string_view trim(std::string_view str, DelimOrPred delimOrPred = ' ')
	{
		return trim_right(trim_left(str, std::forward<DelimOrPred>(delimOrPred)), std::forward<DelimOrPred>(delimOrPred));
	}

	// return the substring starting at iFirst and consisting of nCount characters, or the rest of the string if nCount is greater than size
	[[nodiscard]] std::string_view mid(std::string_view str, size_t iFirst, size_t nCount)
	{
		if (iFirst > str.size())
			iFirst = str.size();

		if (iFirst + nCount > str.size())
		{
			nCount = str.size() - iFirst;
		}
		if (iFirst > str.size())
		{
			nCount = 0;
		}

		if ((iFirst == 0) && ((iFirst + nCount) == str.size()))
		{
			return str;
		}

		return str.substr(iFirst, nCount);
	}

	// return the substring starting at iFirst
	[[nodiscard]] std::string_view mid(std::string_view str, size_t iFirst)
	{
		return mid(str, iFirst, str.size() - iFirst);
	}

	// return the substring consisting of the last nCount characters, or the whole string if nCount is greater than size
	[[nodiscard]] std::string_view right(std::string_view str, size_t nCount)
	{
		const auto nLength = str.size();
		if (nCount >= nLength)
		{
			return str;
		}
		return str.substr(nLength - nCount, nCount);
	}

	// return the substring consisting of the first nCount characters, or the whole string if nCount is greater than size
	[[nodiscard]] std::string_view left(std::string_view str, size_t nCount)
	{
		return (nCount >= str.size()) ? str : str.substr(0, nCount);
	}

	// return the string consisting of the first characters not matching delimOrPred
	template<typename DelimOrPred>
	[[nodiscard]] std::string_view span_excluding(std::string_view str, DelimOrPred delimOrPred)
	{
		return str.substr(0, details::meta::FnTraits<DelimOrPred>::find_first_of(str, std::forward<DelimOrPred>(delimOrPred)));
	}

	// return the string consisting of the first characters matching delimOrPred
	template<typename DelimOrPred>
	[[nodiscard]] std::string_view span_including(std::string_view str, DelimOrPred delimOrPred)
	{
		return str.substr(0, details::meta::FnTraits<DelimOrPred>::find_first_not_of(str, std::forward<DelimOrPred>(delimOrPred)));
	}

	// return true if the string begins with the supplied prefix
	[[nodiscard]] bool begins_with(std::string_view str, std::string_view prefix)
	{
		return str.compare(0, prefix.size(), prefix) == 0;
	}

	// return true if the string ends with the supplied suffix
	[[nodiscard]] bool ends_with(std::string_view str, std::string_view suffix)
	{
		return suffix.size() <= str.size() && std::equal(suffix.rbegin(), suffix.rend(), str.rbegin(), str.rend());
	}

	// return true if the string contains the supplied substring
	[[nodiscard]] bool contains(std::string_view str, std::string_view what)
	{
		return std::search(begin(str), end(str), begin(what), end(what)) != end(str);
	}

	// return the iterator pointing to the first occurrence of the supplied substring into the string
	[[nodiscard]] auto search(std::string_view str, std::string_view what)
	{
		return std::search(begin(str), end(str), begin(what), end(what));
	}

	// return the substring consisting of the first nCount characters, or the whole string if nCount is greater than size, and remove such characters from the original string
	std::string_view consume_left(std::string_view& str, size_t nCount = 1u)
	{
		auto firstN = left(str, nCount);
		str.remove_prefix(std::min(str.size(), nCount));
		return firstN;
	}

	// return the substring consisting of the last nCount characters, or the whole string if nCount is greater than size, and remove such characters from the original string
	std::string_view consume_right(std::string_view& str, size_t nCount = 1u)
	{
		auto lastN = right(str, nCount);
		str.remove_suffix(std::min(str.size(), nCount));
		return lastN;
	}

	// return the string consisting of the first characters not matching delimOrPred and remove such characters from the string
	template<typename DelimOrPred>
	std::string_view consume_span_excluding(std::string_view& str, DelimOrPred delimOrPred)
	{
		auto t = span_excluding(str, std::forward<DelimOrPred>(delimOrPred));
		str.remove_prefix(t.size());
		return t;
	}

	// return the string consisting of the first characters matching delimOrPred and remove such characters from the string
	template<typename DelimOrPred>
	std::string_view consume_span_including(std::string_view& str, DelimOrPred delimOrPred)
	{
		auto t = span_including(str, std::forward<DelimOrPred>(delimOrPred));
		str.remove_prefix(t.size());
		return t;
	}

	// return the first token of the string delimited by delimOrPred and remove it, including delimiters, from the string
	template<typename DelimOrPred>
	std::string_view consume(std::string_view& str, DelimOrPred delimOrPred)
	{
		str = trim_left(str, std::forward<DelimOrPred>(delimOrPred));
		auto t = consume_span_excluding(str, std::forward<DelimOrPred>(delimOrPred));
		str = trim_left(str, std::forward<DelimOrPred>(delimOrPred));
		return t;
	}

	namespace split_filters
	{
		struct keep_empties_t
		{
			bool operator()([[maybe_unused]]std::string_view token) const
			{
				return true;
			}
		};

		struct ignore_empties_t
		{
			bool operator()(std::string_view token) const
			{
				return !token.empty();
			}
		};

		inline const keep_empties_t keep_empties;
		inline const ignore_empties_t ignore_empties;
	}

	namespace split_predicates
	{
		class max_length
		{
		public:
			max_length(size_t maxLen)
				: m_maxLen(maxLen)
			{

			}

			std::pair<size_t, size_t> operator()([[maybe_unused]]std::string_view str, size_t startPos) const
			{
				return { startPos + m_maxLen, startPos + m_maxLen };
			}
		private:
			size_t m_maxLen;
		};

		class max_splits
		{
		public:
			max_splits(size_t maxSplits, std::string_view delims)
				: m_maxSplits(maxSplits), m_delimiters(delims)
			{

			}

			max_splits(size_t maxSplits)
				: max_splits(maxSplits, " ")
			{

			}

			std::pair<size_t, size_t> operator()(std::string_view sv, size_t startPos) 
			{
				if (m_maxSplits == 0)
					return { std::string_view::npos, std::string_view::npos };
				--m_maxSplits;
				auto pos = sv.find_first_of(m_delimiters, startPos);
				return { pos, pos + 1 };
			};

		private:
			size_t m_maxSplits;
			std::string_view m_delimiters;
		};
	}

	// calls a function for each token resulting from a split operation
	// delimOrPred can be either a char, const char*, string_view
	// or
	// a 'predicate' function such as pair<size_t, size_t>(str, startPosition)
	// the first size_t is the position of the first "delimiter", the second size_t is the next character to search from
	template<typename DelimOrPred, typename Consumer, typename Filter = split_filters::keep_empties_t>
	void splitf(std::string_view str, DelimOrPred&& delimOrPred, Consumer&& consumer, Filter&& filter = split_filters::keep_empties_t{})
	{
		size_t start = 0;
		auto [pos, next] = details::meta::FnTraits<DelimOrPred>::find_first_of_from(str, std::forward<DelimOrPred>(delimOrPred), start);
		while (pos != std::string_view::npos && pos < str.size())
		{
			auto token = str.substr(start, pos - start);
			if (std::forward<Filter>(filter)(token))
				std::forward<Consumer>(consumer)(token);
			start = next;
			std::tie(pos, next) = details::meta::FnTraits<DelimOrPred>::find_first_of_from(str, std::forward<DelimOrPred>(delimOrPred), start);
		}
		if (start <= str.length())
		{
			auto token = str.substr(start, str.length() - start);
			if (std::forward<Filter>(filter)(token))
				std::forward<Consumer>(consumer)(token);
		}
	}

	// returns a vector<string_view> with the result of the split function applied with the given arguments
	template<typename DelimOrPred = char, typename Filter = split_filters::keep_empties_t>
	[[nodiscard]] std::vector<std::string_view> split(std::string_view str, DelimOrPred&& delimOrPred = ' ', Filter&& filter = split_filters::keep_empties_t{})
	{
		std::vector<std::string_view> res;
		splitf(str, std::forward<DelimOrPred>(delimOrPred), [&](auto token) { res.push_back(token);  }, std::forward<Filter>(filter));
		return res;
	}

	// cfr. str.data()
	[[nodiscard]] auto data_begin(std::string_view str)
	{
		return str.data();
	}

	// cfr. str.data() + str.size()
	[[nodiscard]] auto data_end(std::string_view str)
	{
		return str.data() + str.size();
	}

	// try parsing str as numeric type
	template<typename What>
	[[nodiscard]] std::optional<What> try_parse(std::string_view str)
	{
		What value;
		if (auto res = std::from_chars(data_begin(str), data_end(str), value); res.ec == std::errc{})
			return value;
		return {};
	}

	// try parsing str as a number and, in case of success, remove such number from the beginning of the string
	template<typename What>
	[[nodiscard]] std::optional<What> try_consume_as(std::string_view& str)
	{
		What value;
		if (auto [ptr, ec] = std::from_chars(data_begin(str), data_end(str), value); ec == std::errc{})
		{
			str.remove_prefix(std::distance(data_begin(str), ptr));
			return value;
		}
		return {};
	}

	struct str_cmpi
	{
		int operator()(const char* first, const char* second) const
		{
			return _strcmpi(first, second);
		}
	};
	
	// generic case insensitive and transparent comparator working on any string type
	template<typename CharT = char, typename CmpFunction = str_cmpi>
	class less_ci
	{
		using StrViewT = std::basic_string_view < CharT >;
	public:
		using is_transparent = std::true_type;

		template<typename T1, typename T2, 
			typename = std::enable_if_t<
				!std::is_same_v<T1, StrViewT> && 
				!std::is_same_v<T2, StrViewT>>>
		bool operator()(const T1& first, const T2& second) const
		{
			using details::data;
			return CmpFunction{}(data(first), data(second)) < 0;
		}

		bool operator()(const StrViewT& first, const StrViewT& second) const
		{
			const auto cmp = _memicmp(first.data(), second.data(), sizeof(CharT) * std::min(first.size(), second.size()));
			return cmp == 0 ? (first.size() < second.size()) : cmp < 0;
		}
	};
}
