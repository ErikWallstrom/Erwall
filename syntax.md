# Comments

```
# Single line comment
#{
	Multi line comment
	#{
		Nested comment 
	#}
#}
```

# Type declaration

```
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

```
mut name: Type(value);
let name: Type(value);

mut x: Int(sizeof(Float));
let x: Int(sizeof(Float)); # Immutable
```

# Function declaration

```
func name: (arguments) -> return_type { }

func print: (text: Char[]) { }
```

