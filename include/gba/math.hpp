#ifndef GBAXX_MATH_HPP
#define GBAXX_MATH_HPP

#include <type_traits>

#include <gba/int.hpp>
#include <gba/fixed_point.hpp>

namespace gba {
namespace math {

template <unsigned Exponent>
struct constants {
	static constexpr auto e = fixed_point<int32, Exponent>( 2.7182818284590452354 );
	static constexpr auto log2e = fixed_point<int32, Exponent>( 1.4426950408889634074 );
	static constexpr auto log10e = fixed_point<int32, Exponent>( 0.43429448190325182765 );
	static constexpr auto ln2 = fixed_point<int32, Exponent>( 0.693147180559945309417 );
	static constexpr auto ln10 = fixed_point<int32, Exponent>( 2.30258509299404568402 );
	static constexpr auto pi = fixed_point<int32, Exponent>( 3.14159265358979323846 );
	static constexpr auto pi_2 = fixed_point<int32, Exponent>( 1.57079632679489661923 );
	static constexpr auto pi_4 = fixed_point<int32, Exponent>( 0.78539816339744830962 );
	static constexpr auto i_pi = fixed_point<int32, Exponent>( 0.31830988618379067154 );
	static constexpr auto i2_pi = fixed_point<int32, Exponent>( 0.63661977236758134308 );
	static constexpr auto i2_sqrtpi = fixed_point<int32, Exponent>( 1.12837916709551257390 );
	static constexpr auto sqrt2 = fixed_point<int32, Exponent>( 1.41421356237309504880 );
	static constexpr auto sqrt1_2 = fixed_point<int32, Exponent>( 0.70710678118654752440 );
};

// Count trailing zeros
template <typename Type>
constexpr typename std::enable_if<std::is_integral<Type>::value && sizeof( Type ) == 8, int>::type
ctz( Type n ) noexcept {
	return __builtin_ctzll( n );
}

template <typename Type>
constexpr typename std::enable_if<std::is_integral<Type>::value && sizeof( Type ) < 8, int>::type
ctz( Type n ) noexcept {
	return __builtin_ctz( n );
}

// Count leading zeros
template <typename Type>
constexpr typename std::enable_if<std::is_integral<Type>::value && sizeof( Type ) == 8, int>::type
clz( Type n ) noexcept {
	return __builtin_clzll( n );
}

template <typename Type>
constexpr typename std::enable_if<std::is_integral<Type>::value && sizeof( Type ) < 8, int>::type
clz( Type n ) noexcept {
	return __builtin_clz( n );
}

template <typename Type>
constexpr typename std::enable_if<std::is_integral<Type>::value && sizeof( Type ) == 8, Type>::type
ceil2( Type n ) noexcept {
	--n;
	n = __builtin_clzll( n );
	return 1 << ( 64 - n );
}

template <typename Type>
constexpr typename std::enable_if<std::is_integral<Type>::value && sizeof( Type ) < 8, Type>::type
ceil2( Type n ) noexcept {
	--n;
	n = __builtin_clz( n );
	return 1 << ( 32 - n );
}

template <typename Type>
constexpr typename std::enable_if<std::is_integral<Type>::value && sizeof( Type ) == 8, Type>::type
floor2( Type n ) noexcept {
	n = __builtin_clzll( n );
	return 1 << ( 63 - n );
}

template <typename Type>
constexpr typename std::enable_if<std::is_integral<Type>::value && sizeof( Type ) < 8, Type>::type
floor2( Type n ) noexcept {
	n = __builtin_clz( n );
	return 1 << ( 31 - n );
}

// Rotate left
template <typename Type>
constexpr typename std::enable_if<std::is_integral<Type>::value, Type>::type
rotl( Type n, unsigned s ) noexcept {
	typename std::make_unsigned<Type>::type un = n;
	return ( un << s ) | ( un >> ( ( sizeof( un ) * 8 ) - s ) );
}

// Rotate right
template <typename Type>
constexpr typename std::enable_if<std::is_integral<Type>::value, Type>::type
rotr( Type n, unsigned s ) noexcept {
	typename std::make_unsigned<Type>::type un = n;
	return ( un >> s ) | ( un << ( ( sizeof( un ) * 8 ) - s ) );
}

namespace detail {

	template <class ReprType>
	constexpr ReprType sqrt_bit( ReprType n, ReprType bit ) noexcept {
		if ( bit > n ) {
			return sqrt_bit<ReprType>( n, bit >> 2 );
		} else {
			return bit;
		}
	}

	template <class ReprType>
	constexpr auto sqrt_bit( ReprType n ) noexcept {
		return sqrt_bit<ReprType>( n, ReprType( 1 ) << ( ( ( sizeof( ReprType ) * 8 - 1 ) + std::is_signed<ReprType>::value ) - 2 ) );
	}

	template <class ReprType>
	constexpr ReprType sqrt_solve3( ReprType n, ReprType bit, ReprType result ) noexcept {
		if ( bit != 0 ) {
			if ( n >= result + bit ) {
				return sqrt_solve3<ReprType>( static_cast< ReprType >( n - ( result + bit ) ), bit >> 2, static_cast< ReprType >( ( result >> 1 ) + bit ) );
			} else {
				return sqrt_solve3<ReprType>( n, bit >> 2, result >> 1 );
			}
		} else {
			return result;
		}
	}

	template <class ReprType>
	constexpr auto sqrt_solve1( ReprType n ) noexcept {
		return sqrt_solve3<ReprType>( n, sqrt_bit<ReprType>( n ), 0 );
	}

	constexpr auto sin_bam16( int16 bam ) {
		auto x = bam << 17;
		if ( ( x ^ ( x << 1 ) ) < 0 ) {
			x = ( 1 << 31 ) - x;
		}
		x = x >> 17;
		return fixed_point<int, 12>::from_data( x * ( 0x18000 - ( ( x * x ) >> 11 ) ) >> 17 );
	}

	template <class ReprType, int Exponent>
	constexpr int16 radian_to_bam16( const fixed_point<ReprType, Exponent>& radian ) {
		constexpr auto radTo16 = make_ufixed<13, 19>( 16384.0 / 3.14159265358979323846264338327950288 );
		return static_cast<int16>( radian * radTo16 );
	}

} // detail

template <class ReprType, int Exponent>
constexpr auto sqrt( const fixed_point<ReprType, Exponent>& x ) noexcept {
	using widened_type = fixed_point<wider_promote<ReprType>, Exponent * 2>;
	return fixed_point<ReprType, Exponent>::from_data( static_cast< ReprType >( detail::sqrt_solve1( widened_type( x ).data() ) ) );
}

template <class ReprType, int Exponent>
constexpr auto sin( const fixed_point<ReprType, Exponent>& radian ) noexcept {
	return detail::sin_bam16( detail::radian_to_bam16( radian ) );
}

template <class S, typename std::enable_if<std::is_floating_point<S>::value, int>::type Dummy = 0>
constexpr auto sin( const S radian ) noexcept {
	return detail::sin_bam16( detail::radian_to_bam16( make_ufixed<13, 19>( radian ) ) );
}

template <class ReprType, int Exponent>
constexpr auto cos( const fixed_point<ReprType, Exponent>& radian ) noexcept {
	return detail::sin_bam16( detail::radian_to_bam16( radian ) + 0x2000 );
}

template <class S, typename std::enable_if<std::is_floating_point<S>::value, int>::type Dummy = 0>
constexpr auto cos( const S radian ) noexcept {
	return detail::sin_bam16( detail::radian_to_bam16( make_ufixed<13, 19>( radian ) ) + 0x2000 );
}

} // math
} // gba

#endif // define GBAXX_MATH_HPP
