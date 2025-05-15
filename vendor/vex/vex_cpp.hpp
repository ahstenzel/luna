#include "vex.h"
#include <string>
#include <iterator>
#include <limits>
#include <cstddef>

class vex {
public:
	vex(const std::string& name, const std::string& version, const std::string description);
	~vex();

	bool add_arg(const std::string& description, int arg_type, const std::string& long_name, char short_name, int max_count = std::numeric_limits<int>::min());

	bool parse(int argc, char** argv);

	int token_count();

	const vex_arg_token* get_token(int num);

	bool arg_found(const std::string& name);

	std::string get_version();

	std::string get_help();

	struct iterator_type {
		using iterator_category = std::random_access_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = vex_arg_token;
		using pointer           = vex_arg_token*;
		using reference         = vex_arg_token&;

		iterator_type(const vex_ctx* m_ctx, std::size_t idx);

		reference operator*() const;
		pointer operator->();

		iterator_type& operator++();
		iterator_type operator++(int);
		iterator_type& operator--();
		iterator_type operator--(int);

		iterator_type& operator+=(const difference_type& x);
		iterator_type& operator+=(const iterator_type& x);
		iterator_type& operator-=(const difference_type& x);
		iterator_type& operator-=(const iterator_type& x);

		friend iterator_type operator+(iterator_type lhs, const iterator_type& rhs);
		friend iterator_type operator-(iterator_type lhs, const iterator_type& rhs);

		friend bool operator==(const iterator_type& lhs, const iterator_type& rhs);
		friend bool operator!=(const iterator_type& lhs, const iterator_type& rhs);

		friend bool operator<(const iterator_type& lhs, const iterator_type& rhs);
		friend bool operator>(const iterator_type& lhs, const iterator_type& rhs);
		friend bool operator<=(const iterator_type& lhs, const iterator_type& rhs);
		friend bool operator>=(const iterator_type& lhs, const iterator_type& rhs);
	private:
		const vex_ctx* m_ctx;
		std::size_t m_idx;
	};

	using iterator = iterator_type;
	using const_iterator = const iterator_type;
	using riterator = std::reverse_iterator<iterator>;
	using const_riterator = std::reverse_iterator<const_iterator>;

	iterator begin();
	const_iterator begin() const;
	riterator rbegin();
	const_riterator rbegin() const;
	const_iterator cbegin() const;
	const_riterator crbegin() const;
	iterator end();
	const_iterator end() const;
	riterator rend();
	const_riterator rend() const;
	const_iterator cend() const;
	const_riterator crend() const;

private:
	vex_ctx ctx;
};

#ifdef VEX_IMPLEMENTATION

vex::iterator::iterator_type(const vex_ctx* ctx, std::size_t idx) {
	m_ctx = ctx;
	m_idx = idx;
}

vex::iterator::reference vex::iterator::operator*() const {
	return m_ctx->arg_token[m_idx];
}

vex::iterator::pointer vex::iterator::operator->() {
	return &m_ctx->arg_token[m_idx];
}

vex::iterator& vex::iterator::operator++() {
	m_idx++;
	return *this;
}

vex::iterator vex::iterator::operator++(int) {
	vex::iterator tmp(*this);
	operator++();
	return tmp;
}

vex::iterator& vex::iterator::operator--() {
	m_idx--;
	return *this;
}

vex::iterator vex::iterator::operator--(int) {
	vex::iterator tmp(*this);
	operator--();
	return tmp;
}

vex::iterator& vex::iterator::operator+=(const difference_type& x) {
	m_idx += x;
	return *this;
}

vex::iterator& vex::iterator::operator+=(const vex::iterator_type& x) {
	m_idx += x.m_idx;
	return *this;
}

vex::iterator& vex::iterator::operator-=(const difference_type& x) {
	m_idx -= x;
	return *this;
}

vex::iterator& vex::iterator::operator-=(const vex::iterator_type& x) {
	m_idx -= x.m_idx;
	return *this;
}

vex::iterator operator+(vex::iterator_type lhs, const vex::iterator_type& rhs) {
	lhs += rhs;
	return lhs;
}

vex::iterator operator-(vex::iterator_type lhs, const vex::iterator_type& rhs) {
	lhs -= rhs;
	return lhs;
}

bool operator==(const vex::iterator_type& lhs, const vex::iterator_type& rhs) {
	return (lhs.m_ctx == rhs.m_ctx && lhs.m_idx == rhs.m_idx);
}

bool operator!=(const vex::iterator_type& lhs, const vex::iterator_type& rhs) {
	return !(lhs == rhs);
}

bool operator<(const vex::iterator_type& lhs, const vex::iterator_type& rhs) {
	return lhs.m_idx < rhs.m_idx;
}

bool operator>(const vex::iterator_type& lhs, const vex::iterator_type& rhs) {
	return rhs.m_idx < lhs.m_idx;
}

bool operator<=(const vex::iterator_type& lhs, const vex::iterator_type& rhs) {
	return !(lhs.m_idx < rhs.m_idx);
}

bool operator>=(const vex::iterator_type& lhs, const vex::iterator_type& rhs) {
	return !(lhs.m_idx > rhs.m_idx);
}

vex::iterator vex::begin()                { return iterator(&ctx, 0); }

vex::const_iterator vex::begin() const    { return iterator(&ctx, 0); }

vex::riterator vex::rbegin()              { return riterator(end()); }

vex::const_riterator vex::rbegin() const  { return const_riterator(end()); }

vex::const_iterator vex::cbegin() const   { return begin(); }

vex::const_riterator vex::crbegin() const { return rbegin(); }

vex::iterator vex::end()                  { return iterator(&ctx, ctx.num_arg_token); }

vex::const_iterator vex::end() const      { return iterator(&ctx, ctx.num_arg_token); }

vex::riterator vex::rend()                { return riterator(begin()); }

vex::const_riterator vex::rend() const    { return const_riterator(begin()); }

vex::const_iterator vex::cend() const     { return end(); }

vex::const_riterator vex::crend() const   { return rend(); }

vex::vex(const std::string& name, const std::string& version, const std::string description) {
	vex_init_info info = { 0 };
	info.name = name.c_str();
	info.version = version.c_str();
	info.description = description.c_str();
	ctx = vex_init(info);
}

vex::~vex() {
	vex_free(&ctx);
}

bool vex::add_arg(const std::string& description, int arg_type, const std::string& long_name, char short_name, int max_count) {
	vex_arg_desc desc = { 0 };
	desc.arg_type = arg_type;
	desc.description = const_cast<char*>(description.c_str());
	desc.long_name = const_cast<char*>(long_name.c_str());
	desc.short_name = short_name;
	if (max_count == std::numeric_limits<int>::min()) {
		if (arg_type == VEX_ARG_TYPE_FLAG) max_count = 0;
		else max_count = -1;
	}
	desc.max_count = max_count;
	return vex_add_arg(&ctx, desc);
}

bool vex::parse(int argc, char** argv) {
	return vex_parse(&ctx, argc, argv);
}

int vex::token_count() {
	return vex_token_count(&ctx);
}

const vex_arg_token* vex::get_token(int num) {
	return vex_get_token(&ctx, num);
}

bool vex::arg_found(const std::string& name) {
	return vex_arg_found(&ctx, name.c_str());
}

std::string vex::get_version() {
	return std::string(vex_get_version(&ctx));
}

std::string vex::get_help() {
	return std::string(vex_get_help(&ctx));
}

#endif