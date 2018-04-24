#pragma once
#include <cinttypes>

static inline uint16_t __local_bswap16(uint16_t x) {
	return ((x << 8) & 0xff00) | ((x >> 8) & 0x00ff);
}


static inline uint32_t __local_bswap32(uint32_t x) {
	return	((x << 24) & 0xff000000 ) |
			((x <<  8) & 0x00ff0000 ) |
			((x >>  8) & 0x0000ff00 ) |
			((x >> 24) & 0x000000ff );
}

static inline uint64_t __local_bswap64(uint64_t x)
{
	return (uint64_t)__local_bswap32(x>>32) |
	      ((uint64_t)__local_bswap32(x&0xFFFFFFFF) << 32);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static inline uint64_t be_dword(uint64_t a) { return __local_bswap64(a); }
static inline uint32_t be_word(uint32_t a) { return __local_bswap32(a); }
static inline uint16_t be_hword(uint16_t a) { return __local_bswap16(a); }
static inline uint64_t le_dword(uint64_t a) { return a; }
static inline uint32_t le_word(uint32_t a) { return a; }
static inline uint16_t le_hword(uint16_t a) { return a; }
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
static inline uint64_t be_dword(uint64_t a) { return a; }
static inline uint32_t be_word(uint32_t a) { return a; }
static inline uint16_t be_hword(uint16_t a) { return a; }
static inline uint64_t le_dword(uint64_t a) { return __local_bswap64(a); }
static inline uint32_t le_word(uint32_t a) { return __local_bswap32(a); }
static inline uint16_t le_hword(uint16_t a) { return __local_bswap16(a); }
#else
#error "What's the endianness of the platform you're targeting?"
#endif

/*
template <class T, static T (*F)(T)>
class ISerialiseablePrimative {
public:
	inline T get() const { return F(mVar);}
	inline void set(T var) { mVar = F(var); }
	inline T operator=(T var) { set(var); return get();}
	inline T operator*() const { return get(); }
private:
	T mVar;
};

typedef ISerialiseablePrimative<uint16_t, le_hword> le_uint16_t;
typedef ISerialiseablePrimative<uint16_t, be_hword> be_uint16_t;
typedef ISerialiseablePrimative<uint32_t, le_word> le_uint32_t;
typedef ISerialiseablePrimative<uint32_t, be_word> be_uint32_t;
typedef ISerialiseablePrimative<uint64_t, le_dword> le_uint64_t;
typedef ISerialiseablePrimative<uint64_t, be_dword> be_uint64_t;
*/

class le_uint16_t {
public:
	inline uint16_t get() const { return le_hword(mVar);}
	inline void set(uint16_t var) { mVar = le_hword(var); }
	inline uint16_t operator=(uint16_t var) { set(var); return get();}
	inline uint16_t operator*() const { return get(); }
private:
	uint16_t mVar;
};

class be_uint16_t {
public:
	inline uint16_t get() const { return be_hword(mVar);}
	inline void set(uint16_t var) { mVar = be_hword(var); }
	inline uint16_t operator=(uint16_t var) { set(var); return get();}
	inline uint16_t operator*() const { return get(); }
private:
	uint16_t mVar;
};

class le_uint32_t {
public:
	inline uint32_t get() const { return le_word(mVar);}
	inline void set(uint32_t var) { mVar = le_word(var); }
	inline uint32_t operator=(uint32_t var) { set(var); return get();}
	inline uint32_t operator*() const { return get(); }
private:
	uint32_t mVar;
};

class be_uint32_t {
public:
	inline uint32_t get() const { return be_word(mVar);}
	inline void set(uint32_t var) { mVar = be_word(var); }
	inline uint32_t operator=(uint32_t var) { set(var); return get();}
	inline uint32_t operator*() const { return get(); }
private:
	uint32_t mVar;
};

class le_uint64_t {
public:
	inline uint64_t get() const { return le_dword(mVar);}
	inline void set(uint64_t var) { mVar = le_dword(var); }
	inline uint64_t operator=(uint64_t var) { set(var); return get();}
	inline uint64_t operator*() const { return get(); }
private:
	uint64_t mVar;
};

class be_uint64_t {
public:
	inline uint64_t get() const { return be_dword(mVar);}
	inline void set(uint64_t var) { mVar = be_dword(var); }
	inline uint64_t operator=(uint64_t var) { set(var); return get();}
	inline uint64_t operator*() const { return get(); }
private:
	uint64_t mVar;
};