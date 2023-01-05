# Describers

Describers set restrictions on properties.

- [Scalars](#scalars)
	- [Nil](#nil)
	- [Bool](#bool)
	- [Int](#int)
	- [Num](#num)
	- [Str](#str)
- [Map](#map)
	- [MonoMap of Prop P](#monomap-of-prop-p)
	- [PolyMap](#polymap)
	- [StructuredMap of struct V](#structuredmap-of-struct-v)
- [List](#list)
	- [Array of Prop P with size N](#array-of-prop-p-with-size-n)
	- [MonoList of Prop P](#monolist-of-prop-p)



## Scalars

### Nil

Mapped to `std::nullptr_t`

```cpp
prop<Type::Nil>();
```

No restrictions available.


### Bool

Mapped to `bool`

```cpp
prop<Type::Bool>();
```

No restrictions available.


### Int

Mapped to integer types such as `int`, `long`, `unsigned int`, etc.

```cpp
prop<Type::Int>()
	.multipleOf(3)
	.interval(5 < x <= 42)
	.withClamp();
```

#### `.multipleOf(...)`

Value is restricted to multiples of the given value.

#### `.interval(...)`

Value is restricted to the given interval.

#### `.withClamp(...)`

Validation passes even if the value is not in the specified interval. Instead, when the value is accessed, it is clamped to fit the interval.


### Num

Mapped to floating point types such as `float`, `double`, `long double`, etc.

```cpp
prop<Type::Num>()
	.multipleOf(0.2)
	.interval(2.718 < x <= 3.14)
	.withClamp();
```

#### `.multipleOf(...)`

Value is restricted to multiples of the given value.

#### `.interval(...)`

Value is restricted to the given interval.

#### `.withClamp(...)`

Validation passes even if the value is not in the specified interval. Instead, when the value is accessed, it is clamped to fit the interval.


### Str

Mapped to `std::string`.

```cpp
prop<Type::Str>()
	.oneOf({"foo", "bar", "baz"})
	.length(2 < x <= 7);
```

#### `.oneOf(...)`

Value is restricted to one of the given values.

#### `.length(...)`

Length of the value is restricted to the given interval.



## Map

### MonoMap of Prop P

Mapped to `std::unordered_map<std::string, P::StorageType>`.

```cpp
prop<Type::Map>()
	.of(
		prop<Type::Int>()
			.multipleOf(7)
	)
	.containing({"foo", "bar", "baz"});

// Shorthand
prop<Type::Map>().of<Type::Int>
	.containing({"foo", "bar", "baz"});
```

#### `.containing(...)`

Value is restricted to contain the given keys.


### PolyMap

No type is mapped. Accessible only by Node.

```cpp
Node node(...);
node["foo"].is<Type::Int>().interval(0 < x).withDefault(2);
node[req("bar")].is<Type::Str>().oneOf({"get", "set"});
node["baz"].is<Type::Map>().of<Type::Str>();
```

#### `cray::req(...)`

Value is restricted to contain the given key.


### StructuredMap of struct V

Mapped to struct `V`.

```cpp
struct V {
	int foo;
	std::optional<std::string> bar;
	std::vector<double> baz;
};

prop<Type::Map>().to<V>()
	| field("foo", &V::foo)
	| field("bar", &V::bar)
	| field("baz", &V::baz, prop<Num>().interval(0 < x));
```

#### `cray::field(...)`

Value is restricted to contain the given key with the given type. The key can be omitted if the given type is instance of `std::optional`.



## List

### Array of Prop P with size N

Mapped to `std::array<P::StorageType, N>`.

```cpp
prop<Type::List>()
	.of<3>(
		prop<Type::Str>()
			.length(3 < x < 12)
	);

// Shorthand
prop<Type::List>().of<Type::Int, 3>();
```

No restrictions available.


### MonoList of Prop P

Mapped to `std::vector<P::StorageType>`.

```cpp
prop<Type::List>()
	.of(
		prop<Type::Str>()
			.oneOf({"foo", "bar", "baz"})
	)
	.size(x < 13);

// Shorthand
prop<Type::List>().of<Type::Int>()
	.size(x < 13);
```

#### `.length(...)`

Size of the value is restricted to the given interval.
