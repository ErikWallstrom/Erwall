//Generated with Erwall

#include <inttypes.h>
#include <stdio.h>
#include <math.h>

enum {erwall_false, erwall_true};

typedef int8_t		erwall_Int8;
typedef int16_t 	erwall_Int16;
typedef int32_t 	erwall_Int32;
typedef int64_t 	erwall_Int64;
typedef uint8_t 	erwall_UInt8;
typedef uint16_t 	erwall_UInt16;
typedef uint32_t 	erwall_UInt32;
typedef uint64_t	erwall_UInt64;
typedef float		erwall_Float32;
typedef double		erwall_Float64;
typedef _Bool		erwall_Bool;

typedef erwall_Int32 erwall_Int;

erwall_Int32 erwall_main()
{
	const erwall_Int32 erwall_x = ((3 + (3 * 4)) - pow(3, 2));
	printf("%i\n", erwall_x);

}

int main(int argc, char* argv[])
{
	return erwall_main();
}
