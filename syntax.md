# Comments

```erwall
# Single line comment
#[
	Multi line comment
	#[
		Nested comment 
	#]
#]
```

# Type declaration

```erwall
type TypeName: Type;

type EntityID: Int64;
type Player: struct 
{
	mut x: Int32;
	mut y: Int32;
	name: Char[];
};
```

# Variable declaration

```erwall
mut name: Type = Type(value);
let name: Type = Type(value);

mut x: Int = Int(sizeof(Float));
let x: Int = Int(sizeof(Float)); # Immutable
```

# Function declaration

```erwall
func name: (arguments) -> return_type {};

func print: (text: Char[]) {};
```

# If statement

```erwall
if(bool) {} elseif(bool){} else {}

if(3 < 10)
{
}
else
{
}
```
