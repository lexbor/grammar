# Grammar

Library for generate compact and fast code for parsing CSS properties.

For example, raw grammar:
```
<a> = <num>* [<a> <x> | <y> | <c> <z> && <b> || <m> <h> | <z>]{1,2} <str>*
```

AST after serialization:
```
<a> = <num>* [[<a> <x>] | <y> | [[<c> <z>] && [<b> || [[<m> <h>] | <z>]]]]{1,2} <str>*
```

Work in progress.

* [x] Tokenizer
* [x] Parser (AST)
* [ ] Tree
* [ ] Code generator by tree

## Dependencies

* [liblexbor-html](https://github.com/lexbor/lexbor) (>=1.4)

Hint: run `cmake` for Lexbor library with `-DLEXBOR_BUILD_SEPARATELY=ON` for build `liblexbor-html`.

## Build and Installation

For build and install Grammar library from source code, use [CMake](https://cmake.org/) (open-source, cross-platform build system).

```bash
cmake . -DLEXBOR_BUILD_TESTS=ON -DLEXBOR_BUILD_EXAMPLES=ON
make
make test
```

## Getting Help

* IRC: [#lexbor on `irc.freenode.net <http://freenode.net>`](http://webchat.freenode.net?channels=%23lexbor)
* E-mail [borisov@lexbor.com](mailto:borisov@lexbor.com)

## AUTHOR

Alexander Borisov <borisov@lexbor.com>

## COPYRIGHT AND LICENSE

   Lexbor.

   Copyright 2018-2020 Alexander Borisov

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


See [LICENSE](https://github.com/lexbor/grammar/blob/master/LICENSE) file.
