# Comments

```erw
# Single line comment
#[
	Multi line comment
	#[
		Nested comment 
	#]
#]
```

# Type declaration

```erw
type TypeName: Type;

type EntityID: Int64;
type Player: struct(
	let name: Char[],
	mut x: Int32,
	mut y: Int32
);
```

# Variable declaration

```erw
mut name: Type = value;
let name: Type = value;

mut x: Int = sizeof(Float); 
let x: Int = sizeof(Float); # Immutable
```

# Function declaration

```erw
func name: (arguments) -> return_type {}

func print: (let text: Char[]) {}
func add: (let x: Int32, let y: Int32) -> Int32 {}
```

# If statement

```erw
if(bool) {} elseif(bool) {} else {}

if(3 < 10) {} elseif(3 == 10) {} else {}
```

# C function call

```erw
@c_function();
@c_variable;

@printf("Hello World");
let status: Int32 = @SDL_CreateWindow("Hello World");
let value: Int32 = @c_variable + 4;
```

# Return statement

```erw
return;
return value;

return 12;
```

# Type casting

```erw
cast(Type, value);

let x: Int32 = cast(Int32, 3.14);
```

# Defer statement

```erw
defer {}

defer {@puts("Hello World");}
```

# While statement

```erw
while(bool) {}

while(true) {do_something();}
```

# Assignment

```erw
name = value;
name += value;
name -= value;
name *= value;
name /= value;
name ^= value;
name %= value;

x = 12;
```

# References

```erw
let x: Int32 = 12;
let x_ref: &Int32 = &x;
let y: Int32 = x_ref&;
```
