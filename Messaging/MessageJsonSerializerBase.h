#pragma once
#include <vector>
#include <algorithm>

namespace messaging {
	namespace json_serilization {
		class JsonSerilizationException final : public std::exception {
		public:
			explicit JsonSerilizationException(const char* const str) : std::exception(str) {}
		};
	}
namespace INTERNAL {
	class JsonSerializerBase {
	protected:
		~JsonSerializerBase() noexcept = default;

		inline bool IsValidField(const std::size_t idx) const {
			return !std::binary_search(m_noSerializeableFields.cbegin(), m_noSerializeableFields.cend(), idx);
		}

	public:
		void SetNonSerializeableFields(const std::vector<std::size_t>& fields) {
			m_noSerializeableFields = fields;
			std::sort(m_noSerializeableFields.begin(), m_noSerializeableFields.end());
		}
	private:
		std::vector<std::size_t> m_noSerializeableFields;
	};
}
}