
#ifndef _MVK_TEXT_HASHTEXT_
#define _MVK_TEXT_HASHTEXT_

#include <algorithm>
#include <cassert>
#include <cstdarg>

namespace mvk
{
	namespace text
	{
		constexpr size_t MVK_FNV_offset_basis = 14695981039346656037ULL;
		constexpr size_t MVK_FNV_prime = 1099511628211ULL;

		inline constexpr size_t fnv1a_append_bytes_char(
			size_t _Val,
			const char* const _First,
			const size_t _Count) noexcept
		{
			for (size_t _Idx = 0; _Idx < _Count; ++_Idx)
			{
				_Val ^= static_cast<size_t>(_First[_Idx]);
				_Val *= MVK_FNV_prime;
			}

			return _Val;
		}

		constexpr size_t fnv1a_Hash_array_representation_char(
			const char* const _First, const size_t _Count) noexcept
		{
			return fnv1a_append_bytes_char(MVK_FNV_offset_basis, _First, _Count);
		}




		template<int LEN>
		class HashText
		{
		private:
			static constexpr int strLenConstexpr(const char* str)
			{
				int l = 0;
				for (; 0 != str[l]; ++l) {}

				return l;
			}


		public:
			constexpr HashText()
				: str_()
				, valid_len_()
				, hash_()
			{
				const int N = strLenConstexpr(s);
				valid_len_ = (LEN < N) ? LEN : N;
				str_[valid_len_] = 0;
				for (auto i = 0; i < valid_len_; ++i)
					str_[i] = s[i];

				hash_ = mvk::text::fnv1a_Hash_array_representation_char(str_, valid_len_);
			}

			void set(const char* s, int len)
			{
				const int N = len;
				valid_len_ = (LEN < N) ? LEN : N;

				for (auto i = 0; i < valid_len_; ++i)
					str_[i] = s[i];
				for (auto i = valid_len_; i < LEN; ++i)
					str_[i] = 0;

				hash_ = mvk::text::fnv1a_Hash_array_representation_char(str, valid_len_);
			}

			constexpr const char* get() const
			{
				return &str_[0];
			}

			constexpr uint64_t hash() const
			{
				return hash_;
			}

			constexpr int length() const
			{
				return valid_len_;
			}

			constexpr int maxLength() const
			{
				return LEN;
			}

		private:
			char str_[LEN + 1] = {};
			int valid_len_ = {};
			uint64_t hash_ = {};
		};

		namespace
		{
			template<int N0, int N1>
			struct CompStr
			{
				constexpr CompStr(const HashText<N0>& v0, const HashText<N1>& v1)
					: result()
				{
					result = true;
					const int n = (v0.length() < v1.length()) ? v0.length() : v1.length();
					for (int i = 0; i < n; i++)
					{
						result &= (v0.get()[i] == v1.get()[i]);
					}
				}

				bool result;
			};

			template<int LEN0, int LEN1>
			static constexpr bool operator == (const HashText<LEN0>& v0, const HashText<LEN1>& v1)
			{
				bool result = (v0.hash() == v1.hash()) && CompStr(v0, v1).result;
				return result;
			}

			template<int LEN0, int LEN1>
			static constexpr bool operator != (const HashText<LEN0>& v0, const HashText<LEN1>& v1)
			{
				return !(v0 == v1);
			}




			template<unsigned int SIZE>
			class FixedString
			{
				static constexpr unsigned int k_size = SIZE;
				static constexpr unsigned int k_buffer_size = k_size + 1;

			public:
				FixedString()
				{
#ifdef _DEBUG
					memset(text_, 0xcd, std::size(text_));
#endif

					valid_len_ = 0;
					std::memset(text_, 0, std::size(text_));
				}
				~FixedString()
				{
				}

				FixedString(const char* str)
				{
					set(str, std::min(static_cast<unsigned int>(std::strlen(str)), k_size));
				}
				FixedString(const FixedString& obj)
				{
					set(obj.text_, obj.valid_len_);
				}
				FixedString(const char* format, ...)
				{
					char buf[k_buffer_size];
					va_list args;
					va_start(args, format);
					const auto n = vsnprintf(buf, sizeof(buf), format, args);
					va_end(args);

					set(buf, n);
				}

				FixedString& operator=(const char* str)
				{
					set(str, std::min(static_cast<unsigned int>(std::strlen(str)), k_size);
				}

				FixedString& operator=(const FixedString& obj)
				{
					set(obj.text_, obj.valid_len_);
					return *this;
				}

				bool operator==(const FixedString& obj) const
				{
					if (obj.valid_len_ != valid_len_)
						return false;

					return 0 == std::memcmp(text_, obj.text_, valid_len_);
				}

				operator const char* () const
				{
					return get();
				}

				void set(const char* str, unsigned int size)
				{
					assert(k_size >= size);

					std::copy_n(str, size, text_);
					if (valid_len_ > size)
					{
						std::memset(text_ + size, 0, valid_len_ - size);
					}

					valid_len_ = size;
				}

				const char* get() const
				{
					return text_;
				}

				unsigned int getValidLen() const
				{
					return valid_len_;
				}

				constexpr unsigned int getMaxLen() const { return k_size; }
				static constexpr unsigned int maxLength() { return k_size; }

			private:
				unsigned int valid_len_ = 0;
				char text_[k_buffer_size] = {};
			};
		}
	}
}

namespace std
{
	template<unsigned int SIZE>
	struct hash<mvk::text::HashText<SIZE>>
	{
		size_t operator()(const mvk::text::HashText<SIZE>>& v) const
		{
			return v.hash();
		}
	};

	template<unsigned int SIZE>
	struct hash<mvk::text::FixedString<SIZE>>
	{
		size_t operator()(const mvk::text::FixedString<SIZE>& v) const
		{
			return mvk::text::fnv1a_Hash_array_representation_char(v.get(), v.getValidLen());
		}
	};
}

#endif
