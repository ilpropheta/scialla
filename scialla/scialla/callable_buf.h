#pragma once

#include <sstream>

namespace scialla
{
	namespace ios
	{
		template<typename Fn, typename CharT = char, typename CharTraits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
		struct callable_buf : public std::basic_stringbuf<CharT, CharTraits, Alloc>
		{
			callable_buf(Fn fn)
				: _sync(std::move(fn))
			{

			}

			~callable_buf()
			{
				const auto curr = str();
				if (!curr.empty())
					sync(curr);
			}

			int sync() override
			{
				return sync(str());
			}

		private:
			int sync(const std::string& curr)
			{
				auto status = _sync(curr);
				str({});
				return status;
			}

			Fn _sync;
		};

		template<typename Fn>
		callable_buf<Fn> create_callable_buf(Fn&& fn)
		{
			return{ (std::forward<Fn>(fn) };
		};
	}
}
