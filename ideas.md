* Nested comments

```erw
#[
	This too
	#[
		# This is a comment
		Also a comment
	#]
#]
```

* Nested functions

```erw
func test: ()
{
	func nested: ()
	{
		
	}
}
```

* Tagged unions

```erw
type Result: union(&Int32, None);
let result: Result = None();
match(result)
{
	case(&Int32)
	{
		
	}
	else
	{
		
	}
}
```

* Strong typing

```erw
type Int: Int32;
let x: Int32 = 12;
let y: Int = x; # Error
```

* First class arrays

```erw
func test: () -> [5]Int32
{
	return [5]Int32(5, 10, 15, 15, 10);
}

let numbers: [5]Int32 = test();
```

* Sized arrays

```erw
let size: Int32 = len(numbers);
```

* Non-null pointers

```erw
let x: Int32 = 10;
let x_ref: &Int32 = &x;
```

* Sized enums
* Named enums
* Defer block
* Unsafe block
* Switch for all types
* Temorary storage
